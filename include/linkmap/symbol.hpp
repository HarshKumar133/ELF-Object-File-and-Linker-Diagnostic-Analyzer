#pragma once

#include "linkmap/section.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace linkmap {
struct Symbol {
  std::uint32_t tableSection{};
  std::uint32_t index{};
  std::string name;
  std::uint64_t value{};
  std::uint64_t size{};
  std::uint8_t binding{};
  std::uint8_t type{};
  std::uint8_t visibility{};
  std::uint16_t sectionIndex{};
  [[nodiscard]] bool isUndefined() const noexcept { return sectionIndex == 0; }
  [[nodiscard]] bool isAbsolute() const noexcept { return sectionIndex == 0xfff1; }
  [[nodiscard]] bool isCommon() const noexcept { return sectionIndex == 0xfff2; }
  [[nodiscard]] bool isWeak() const noexcept { return binding == 2; }
  [[nodiscard]] bool isGlobal() const noexcept { return binding == 1; }
  [[nodiscard]] bool isLocal() const noexcept { return binding == 0; }
  [[nodiscard]] bool isCrossObjectCandidate() const noexcept;
};

[[nodiscard]] Result<std::vector<Symbol>> parseSymbols(const ParsedHeader& object,
                                                       const std::vector<Section>& sections);
[[nodiscard]] std::string symbolBindingName(std::uint8_t binding);
[[nodiscard]] std::string symbolTypeName(std::uint8_t type);
[[nodiscard]] std::string symbolVisibilityName(std::uint8_t visibility);
}
