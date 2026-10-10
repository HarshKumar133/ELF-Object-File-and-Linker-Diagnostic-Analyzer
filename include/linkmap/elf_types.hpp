#pragma once

#include <cstdint>
#include <string>

namespace linkmap {
struct ElfHeader {
  std::uint8_t elfClass{};
  std::uint8_t dataEncoding{};
  std::uint8_t identVersion{};
  std::uint8_t osAbi{};
  std::uint8_t abiVersion{};
  std::uint16_t type{};
  std::uint16_t machine{};
  std::uint32_t version{};
  std::uint64_t entry{};
  std::uint64_t programHeaderOffset{};
  std::uint64_t sectionHeaderOffset{};
  std::uint32_t flags{};
  std::uint16_t headerSize{};
  std::uint16_t programHeaderEntrySize{};
  std::uint16_t programHeaderCount{};
  std::uint16_t sectionHeaderEntrySize{};
  std::uint16_t sectionHeaderCount{};
  std::uint16_t sectionNameIndex{};
};

[[nodiscard]] std::string elfTypeName(std::uint16_t type);
[[nodiscard]] std::string machineName(std::uint16_t machine);
}
