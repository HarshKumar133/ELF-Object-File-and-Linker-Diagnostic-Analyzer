#include "linkmap/relocation.hpp"

#include <unordered_map>

namespace linkmap {
namespace {
constexpr std::uint32_t SHT_RELA = 4;
constexpr std::uint32_t SHT_REL = 9;
struct Description { const char* name; const char* explanation; };
Description describeX86_64(std::uint32_t type) {
  switch (type) {
    case 1: return {"R_X86_64_64", "absolute 64-bit address"};
    case 2: return {"R_X86_64_PC32", "32-bit PC-relative reference"};
    case 4: return {"R_X86_64_PLT32", "32-bit PC-relative call or jump through PLT"};
    case 9: return {"R_X86_64_GOTPCREL", "PC-relative reference to a GOT entry"};
    case 10: return {"R_X86_64_32", "absolute 32-bit value"};
    case 11: return {"R_X86_64_32S", "absolute signed 32-bit value"};
    case 41: return {"R_X86_64_GOTPCRELX", "relaxable PC-relative GOT reference"};
    case 42: return {"R_X86_64_REX_GOTPCRELX", "relaxable REX-prefixed GOT reference"};
    default: return {nullptr, nullptr};
  }
}
}

Result<std::vector<Relocation>> parseRelocations(const ParsedHeader& object,
                                                  const std::vector<Section>& sections,
                                                  const std::vector<Symbol>& symbols) {
  const CheckedReader r(object.bytes);
  std::vector<Relocation> relocations;
  for (const auto& section : sections) {
    if (section.type != SHT_RELA && section.type != SHT_REL) continue;
    const std::uint64_t expected = section.type == SHT_RELA ? 24 : 16;
    if (section.entrySize != expected || section.size % expected != 0)
      return Result<std::vector<Relocation>>::failure({"LM4001", "invalid relocation entry size", section.offset});
    if (section.link >= sections.size() ||
        (sections[section.link].type != 2 && sections[section.link].type != 11))
      return Result<std::vector<Relocation>>::failure({"LM4002", "relocation links to a missing or invalid symbol table", section.offset});
    if (section.info >= sections.size())
      return Result<std::vector<Relocation>>::failure({"LM4003", "relocation target section index is invalid", section.offset});
    if (!r.contains(section.offset, section.size))
      return Result<std::vector<Relocation>>::failure({"LM4004", "relocation entries exceed input", section.offset});
    const auto count = section.size / expected;
    if (count > 10'000'000)
      return Result<std::vector<Relocation>>::failure({"LM4005", "relocation count exceeds resource limit", section.offset});
    std::unordered_map<std::uint64_t, const Symbol*> symbolLookup;
    for (const auto& symbol : symbols) {
      if (symbol.tableSection == section.link)
        symbolLookup.emplace((static_cast<std::uint64_t>(symbol.tableSection) << 32U) | symbol.index,
                             &symbol);
    }
    for (std::uint64_t i = 0; i < count; ++i) {
      const auto base = section.offset + i * expected;
      auto offset = r.readUnsigned(base, 8);
      auto info = r.readUnsigned(base + 8, 8);
      if (!offset.hasValue() || !info.hasValue())
        return Result<std::vector<Relocation>>::failure({"LM4006", "truncated relocation entry", base});
      const auto symbolIndex = static_cast<std::uint32_t>(info.value() >> 32U);
      const auto rawType = static_cast<std::uint32_t>(info.value() & 0xffffffffULL);
      const auto found = symbolLookup.find((static_cast<std::uint64_t>(section.link) << 32U) | symbolIndex);
      if (found == symbolLookup.end())
        return Result<std::vector<Relocation>>::failure({"LM4007", "relocation references an invalid symbol index", base + 8});
      const Symbol* referenced = found->second;
      Relocation relocation;
      relocation.relocationSection = section.index;
      relocation.targetSection = section.info;
      relocation.symbolTableSection = section.link;
      relocation.symbolIndex = symbolIndex;
      relocation.offset = offset.value();
      relocation.rawType = rawType;
      relocation.symbolName = referenced->name;
      if (section.type == SHT_RELA) {
        auto addend = r.readUnsigned(base + 16, 8);
        if (!addend.hasValue())
          return Result<std::vector<Relocation>>::failure({"LM4006", "truncated RELA addend", base + 16});
        relocation.addend = static_cast<std::int64_t>(addend.value());
        relocation.hasAddend = true;
      }
      if (object.header.machine == 62) {
        const auto description = describeX86_64(rawType);
        if (description.name != nullptr) {
          relocation.typeName = description.name;
          relocation.explanation = description.explanation;
          relocation.descriptionSupported = true;
        }
      }
      relocations.push_back(std::move(relocation));
    }
  }
  return Result<std::vector<Relocation>>::success(std::move(relocations));
}
}
