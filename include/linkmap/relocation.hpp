#pragma once

#include "linkmap/symbol.hpp"

namespace linkmap {
struct Relocation {
  std::uint32_t relocationSection{};
  std::uint32_t targetSection{};
  std::uint32_t symbolTableSection{};
  std::uint32_t symbolIndex{};
  std::uint64_t offset{};
  std::uint32_t rawType{};
  std::string typeName;
  std::string explanation;
  std::string symbolName;
  std::int64_t addend{};
  bool hasAddend{};
  bool descriptionSupported{};
};

[[nodiscard]] Result<std::vector<Relocation>> parseRelocations(
    const ParsedHeader& object, const std::vector<Section>& sections,
    const std::vector<Symbol>& symbols);
}
