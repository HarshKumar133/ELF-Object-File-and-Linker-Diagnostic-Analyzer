#include "linkmap/section.hpp"

#include <limits>
#include <type_traits>

namespace linkmap {
namespace {
constexpr std::uint32_t SHT_STRTAB = 3;
constexpr std::uint32_t SHT_NOBITS = 8;

Result<std::uint64_t> get(const CheckedReader& r, std::uint64_t offset, std::uint8_t width) {
  return r.readUnsigned(offset, width);
}
}

std::string sectionTypeName(std::uint32_t type) {
  switch (type) {
    case 0: return "NULL";
    case 1: return "PROGBITS";
    case 2: return "SYMTAB";
    case 3: return "STRTAB";
    case 4: return "RELA";
    case 5: return "HASH";
    case 6: return "DYNAMIC";
    case 7: return "NOTE";
    case 8: return "NOBITS";
    case 9: return "REL";
    case 11: return "DYNSYM";
    default: return "OTHER";
  }
}

Result<std::vector<Section>> parseSections(const ParsedHeader& object) {
  const CheckedReader r(object.bytes);
  const auto& h = object.header;
  const std::uint64_t tableBytes = static_cast<std::uint64_t>(h.sectionHeaderCount) * 64;
  if (!r.contains(h.sectionHeaderOffset, tableBytes))
    return Result<std::vector<Section>>::failure({"LM2001", "section table exceeds input", h.sectionHeaderOffset});

  std::vector<Section> sections;
  sections.reserve(h.sectionHeaderCount);
  for (std::uint32_t i = 0; i < h.sectionHeaderCount; ++i) {
    const auto base = h.sectionHeaderOffset + static_cast<std::uint64_t>(i) * 64;
    Section s;
    auto assign = [&](std::uint64_t rel, std::uint8_t width, auto& dst) -> bool {
      auto v = get(r, base + rel, width);
      if (!v.hasValue()) return false;
      dst = static_cast<std::remove_reference_t<decltype(dst)>>(v.value());
      return true;
    };
    std::uint32_t nameOffset = 0;
    if (!assign(4, 4, s.type) || !assign(8, 8, s.flags) || !assign(16, 8, s.address) ||
        !assign(24, 8, s.offset) || !assign(32, 8, s.size) || !assign(40, 4, s.link) ||
        !assign(44, 4, s.info) || !assign(48, 8, s.alignment) ||
        !assign(56, 8, s.entrySize) || !assign(0, 4, nameOffset))
      return Result<std::vector<Section>>::failure({"LM2002", "truncated section header", base});
    s.index = i;
    if (s.type != SHT_NOBITS && s.type != 0 && !r.contains(s.offset, s.size))
      return Result<std::vector<Section>>::failure({"LM2003", "section payload exceeds input", s.offset});
    sections.push_back(std::move(s));
    sections.back().name.clear();
    // Temporarily retain the name offset in the parallel vector below.
    (void)nameOffset;
  }

  if (h.sectionNameIndex >= sections.size())
    return Result<std::vector<Section>>::failure({"LM2004", "section-name table index is invalid", h.sectionNameIndex});
  if (h.sectionNameIndex == 0) {
    for (const auto& section : sections) {
      const auto nameOffset = get(r, h.sectionHeaderOffset + static_cast<std::uint64_t>(section.index) * 64, 4);
      if (!nameOffset.hasValue() || nameOffset.value() != 0)
        return Result<std::vector<Section>>::failure({"LM2004", "section name exists but ELF header declares no name table", section.index});
    }
    return Result<std::vector<Section>>::success(std::move(sections));
  }
  const auto& names = sections[h.sectionNameIndex];
  if (names.type != SHT_STRTAB || !r.contains(names.offset, names.size))
    return Result<std::vector<Section>>::failure({"LM2004", "section-name table is not a valid string table", names.offset});

  // Re-read each sh_name field and resolve it strictly inside the linked string table.
  for (auto& section : sections) {
    const auto base = h.sectionHeaderOffset + static_cast<std::uint64_t>(section.index) * 64;
    const auto nameOffset = get(r, base, 4);
    if (!nameOffset.hasValue())
      return Result<std::vector<Section>>::failure({"LM2002", "cannot read section name offset", base});
    if (nameOffset.value() == 0) continue;
    auto name = r.readString(names.offset, names.size, nameOffset.value());
    if (!name.hasValue()) return Result<std::vector<Section>>::failure(name.error());
    section.name = std::move(name.value());
  }
  return Result<std::vector<Section>>::success(std::move(sections));
}
}
