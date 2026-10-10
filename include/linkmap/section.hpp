#pragma once

#include "linkmap/object_parser.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace linkmap {
struct Section {
  std::uint32_t index{};
  std::string name;
  std::uint32_t type{};
  std::uint64_t flags{};
  std::uint64_t address{};
  std::uint64_t offset{};
  std::uint64_t size{};
  std::uint32_t link{};
  std::uint32_t info{};
  std::uint64_t alignment{};
  std::uint64_t entrySize{};
  [[nodiscard]] bool hasFilePayload() const noexcept { return type != 8; }
};

[[nodiscard]] Result<std::vector<Section>> parseSections(const ParsedHeader& object);
[[nodiscard]] std::string sectionTypeName(std::uint32_t type);
}
