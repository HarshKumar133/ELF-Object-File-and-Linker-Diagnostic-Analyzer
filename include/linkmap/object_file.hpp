#pragma once

#include "linkmap/relocation.hpp"

namespace linkmap {
struct ObjectFile {
  std::uint32_t id{};
  ParsedHeader parsed;
  std::vector<Section> sections;
  std::vector<Symbol> symbols;
  std::vector<Relocation> relocations;
};

[[nodiscard]] Result<ObjectFile> parseObject(std::vector<std::uint8_t> bytes,
                                             std::filesystem::path path,
                                             std::uint32_t id = 0);
[[nodiscard]] Result<ObjectFile> readObject(const std::filesystem::path& path,
                                            std::uint32_t id = 0);
}
