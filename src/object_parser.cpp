#include "linkmap/object_parser.hpp"

#include <fstream>
#include <limits>
#include <type_traits>

namespace linkmap {
namespace {
Result<std::uint64_t> field(const CheckedReader& r, std::uint64_t off, std::uint8_t width) {
  return r.readUnsigned(off, width, true);
}
}

std::string elfTypeName(std::uint16_t type) {
  switch (type) {
    case 0: return "ET_NONE";
    case 1: return "ET_REL";
    case 2: return "ET_EXEC";
    case 3: return "ET_DYN";
    case 4: return "ET_CORE";
    default: return "unknown";
  }
}

std::string machineName(std::uint16_t machine) {
  switch (machine) {
    case 3: return "Intel 80386";
    case 40: return "AArch32";
    case 62: return "AMD x86-64";
    case 183: return "AArch64";
    case 243: return "RISC-V";
    default: return "unknown";
  }
}

Result<ParsedHeader> parseHeader(std::vector<std::uint8_t> bytes, std::filesystem::path path) {
  CheckedReader r(bytes);
  if (r.size() < 16) return Result<ParsedHeader>::failure({"LM1001", "truncated ELF identification", r.size()});
  const std::uint8_t magic[] = {0x7f, 'E', 'L', 'F'};
  for (std::uint64_t i = 0; i < 4; ++i) {
    if (bytes[static_cast<std::size_t>(i)] != magic[i])
      return Result<ParsedHeader>::failure({"LM1001", "input does not have ELF magic", i});
  }
  if (bytes[4] != 2) return Result<ParsedHeader>::failure({"LM1002", "only ELF64 is supported", 4});
  if (bytes[5] != 1) return Result<ParsedHeader>::failure({"LM1003", "only little-endian ELF is supported", 5});
  if (bytes[6] != 1) return Result<ParsedHeader>::failure({"LM1004", "unsupported ELF identification version", 6});
  if (r.size() < 64) return Result<ParsedHeader>::failure({"LM1005", "truncated ELF64 header", r.size()});

  ElfHeader h;
  h.elfClass = bytes[4]; h.dataEncoding = bytes[5]; h.identVersion = bytes[6];
  h.osAbi = bytes[7]; h.abiVersion = bytes[8];
  auto assign = [&](std::uint64_t off, std::uint8_t width, auto& dst) -> bool {
    auto value = field(r, off, width);
    if (!value.hasValue()) return false;
    dst = static_cast<std::remove_reference_t<decltype(dst)>>(value.value());
    return true;
  };
  if (!assign(16, 2, h.type) || !assign(18, 2, h.machine) || !assign(20, 4, h.version) ||
      !assign(24, 8, h.entry) || !assign(32, 8, h.programHeaderOffset) ||
      !assign(40, 8, h.sectionHeaderOffset) || !assign(48, 4, h.flags) ||
      !assign(52, 2, h.headerSize) || !assign(54, 2, h.programHeaderEntrySize) ||
      !assign(56, 2, h.programHeaderCount) || !assign(58, 2, h.sectionHeaderEntrySize) ||
      !assign(60, 2, h.sectionHeaderCount) || !assign(62, 2, h.sectionNameIndex))
    return Result<ParsedHeader>::failure({"LM1005", "could not read ELF header fields", 16});
  if (h.version != 1 || h.headerSize != 64)
    return Result<ParsedHeader>::failure({"LM1006", "invalid ELF64 header version or size", 20});
  if (h.type != 1)
    return Result<ParsedHeader>::failure({"LM1007", "only relocatable ELF files (ET_REL) are supported", 16});
  if (h.programHeaderCount == 0xffffU)
    return Result<ParsedHeader>::failure({"LM1013", "extended program-header numbering is unsupported", 56});
  if (h.programHeaderCount != 0) {
    if (h.programHeaderEntrySize != 56 ||
        !r.contains(h.programHeaderOffset,
                    static_cast<std::uint64_t>(h.programHeaderCount) * h.programHeaderEntrySize))
      return Result<ParsedHeader>::failure({"LM1014", "invalid or out-of-range program-header table", h.programHeaderOffset});
  }
  if (h.sectionHeaderCount == 0 || h.sectionHeaderEntrySize != 64)
    return Result<ParsedHeader>::failure({"LM1008", "missing, extended, or invalid section-header table", 58});
  if (!r.contains(h.sectionHeaderOffset,
                  static_cast<std::uint64_t>(h.sectionHeaderCount) * h.sectionHeaderEntrySize))
    return Result<ParsedHeader>::failure({"LM1009", "section-header table exceeds input", h.sectionHeaderOffset});
  if (h.sectionNameIndex == 0xffffU || h.sectionNameIndex >= h.sectionHeaderCount)
    return Result<ParsedHeader>::failure({"LM1010", "unsupported or invalid section-name table index", 62});
  return Result<ParsedHeader>::success({h, std::move(bytes), std::move(path)});
}

Result<ParsedHeader> readObjectHeader(const std::filesystem::path& path) {
  std::ifstream input(path, std::ios::binary | std::ios::ate);
  if (!input) return Result<ParsedHeader>::failure({"LM1011", "could not open input file", 0});
  const auto end = input.tellg();
  if (end < 0) return Result<ParsedHeader>::failure({"LM1011", "could not determine input size", 0});
  const auto size = static_cast<std::uint64_t>(end);
  constexpr std::uint64_t maxInput = 512ULL * 1024ULL * 1024ULL;
  if (size > maxInput) return Result<ParsedHeader>::failure({"LM1012", "input exceeds 512 MiB resource limit", 0});
  std::vector<std::uint8_t> bytes(static_cast<std::size_t>(size));
  input.seekg(0);
  if (!bytes.empty() && !input.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size())))
    return Result<ParsedHeader>::failure({"LM1011", "failed while reading input file", 0});
  return parseHeader(std::move(bytes), path);
}
}
