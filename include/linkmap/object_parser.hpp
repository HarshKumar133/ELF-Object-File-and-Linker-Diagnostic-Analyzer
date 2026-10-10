#pragma once

#include "linkmap/checked_reader.hpp"
#include "linkmap/elf_types.hpp"

#include <filesystem>
#include <vector>

namespace linkmap {
struct ParsedHeader {
  ElfHeader header;
  std::vector<std::uint8_t> bytes;
  std::filesystem::path path;
};

[[nodiscard]] Result<ParsedHeader> parseHeader(std::vector<std::uint8_t> bytes,
                                               std::filesystem::path path = {});
[[nodiscard]] Result<ParsedHeader> readObjectHeader(const std::filesystem::path& path);
}
