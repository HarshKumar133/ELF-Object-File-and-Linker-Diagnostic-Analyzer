#include "linkmap/symbol.hpp"

#include <type_traits>

namespace linkmap {
namespace {
constexpr std::uint32_t SHT_SYMTAB = 2;
constexpr std::uint32_t SHT_DYNSYM = 11;
constexpr std::uint32_t SHT_STRTAB = 3;
}

bool Symbol::isCrossObjectCandidate() const noexcept {
  return !name.empty() && (isGlobal() || isWeak()) && (visibility == 0 || visibility == 3) &&
         (type == 0 || type == 1 || type == 2 || type == 6 || type == 10);
}

std::string symbolBindingName(std::uint8_t b) {
  switch (b) { case 0: return "LOCAL"; case 1: return "GLOBAL"; case 2: return "WEAK"; default: return "OTHER"; }
}
std::string symbolTypeName(std::uint8_t t) {
  switch (t) { case 0: return "NOTYPE"; case 1: return "OBJECT"; case 2: return "FUNC"; case 3: return "SECTION"; case 4: return "FILE"; case 6: return "TLS"; case 10: return "IFUNC"; default: return "OTHER"; }
}
std::string symbolVisibilityName(std::uint8_t v) {
  switch (v & 3U) { case 0: return "DEFAULT"; case 1: return "INTERNAL"; case 2: return "HIDDEN"; case 3: return "PROTECTED"; default: return "OTHER"; }
}

Result<std::vector<Symbol>> parseSymbols(const ParsedHeader& object,
                                         const std::vector<Section>& sections) {
  const CheckedReader r(object.bytes);
  std::vector<Symbol> symbols;
  for (const auto& table : sections) {
    if (table.type != SHT_SYMTAB && table.type != SHT_DYNSYM) continue;
    if (table.link >= sections.size())
      return Result<std::vector<Symbol>>::failure({"LM3001", "symbol table links to missing string table", table.offset});
    const auto& strings = sections[table.link];
    if (strings.type != SHT_STRTAB || !strings.hasFilePayload())
      return Result<std::vector<Symbol>>::failure({"LM3001", "symbol table link is not a string table", table.offset});
    if (table.entrySize != 24 || table.size % table.entrySize != 0)
      return Result<std::vector<Symbol>>::failure({"LM3002", "invalid ELF64 symbol table entry size", table.offset});
    if (!r.contains(table.offset, table.size) || !r.contains(strings.offset, strings.size))
      return Result<std::vector<Symbol>>::failure({"LM3003", "symbol or string table exceeds input", table.offset});
    const auto count = table.size / table.entrySize;
    if (count > 10'000'000)
      return Result<std::vector<Symbol>>::failure({"LM3004", "symbol count exceeds resource limit", table.offset});
    if (table.info > count)
      return Result<std::vector<Symbol>>::failure({"LM3002", "symbol table local-symbol boundary exceeds its entry count", table.offset});
    for (std::uint64_t i = 0; i < count; ++i) {
      const auto base = table.offset + i * table.entrySize;
      auto nameOff = r.readUnsigned(base, 4);
      auto info = r.readUnsigned(base + 4, 1);
      auto other = r.readUnsigned(base + 5, 1);
      auto shndx = r.readUnsigned(base + 6, 2);
      auto value = r.readUnsigned(base + 8, 8);
      auto size = r.readUnsigned(base + 16, 8);
      if (!nameOff.hasValue() || !info.hasValue() || !other.hasValue() || !shndx.hasValue() ||
          !value.hasValue() || !size.hasValue())
        return Result<std::vector<Symbol>>::failure({"LM3005", "truncated symbol entry", base});
      Symbol symbol;
      symbol.tableSection = table.index;
      symbol.index = static_cast<std::uint32_t>(i);
      symbol.binding = static_cast<std::uint8_t>(info.value() >> 4U);
      symbol.type = static_cast<std::uint8_t>(info.value() & 0xfU);
      symbol.visibility = static_cast<std::uint8_t>(other.value() & 3U);
      symbol.sectionIndex = static_cast<std::uint16_t>(shndx.value());
      if (symbol.sectionIndex == 0xffffU)
        return Result<std::vector<Symbol>>::failure({"LM3007", "extended symbol section indexes are unsupported", base + 6});
      symbol.value = value.value(); symbol.size = size.value();
      if (nameOff.value() != 0) {
        auto name = r.readString(strings.offset, strings.size, nameOff.value());
        if (!name.hasValue()) return Result<std::vector<Symbol>>::failure(name.error());
        symbol.name = std::move(name.value());
      }
      if (symbol.sectionIndex < 0xff00 && symbol.sectionIndex >= sections.size())
        return Result<std::vector<Symbol>>::failure({"LM3006", "symbol refers to missing section index", base + 6});
      symbols.push_back(std::move(symbol));
    }
  }
  return Result<std::vector<Symbol>>::success(std::move(symbols));
}
}
