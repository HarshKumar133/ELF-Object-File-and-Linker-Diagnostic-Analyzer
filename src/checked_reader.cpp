#include "linkmap/checked_reader.hpp"

#include <limits>

namespace linkmap {
bool CheckedReader::contains(std::uint64_t offset, std::uint64_t length) const noexcept {
  return offset <= bytes_.size() && length <= bytes_.size() - offset;
}

Result<std::uint64_t> CheckedReader::readUnsigned(std::uint64_t offset, std::uint8_t width,
                                                 bool littleEndian) const {
  if (width == 0 || width > 8) {
    return Result<std::uint64_t>::failure({"LM0001", "integer width must be between 1 and 8", offset});
  }
  if (!contains(offset, width)) {
    return Result<std::uint64_t>::failure({"LM0002", "integer read extends past input", offset});
  }
  std::uint64_t value = 0;
  for (std::uint8_t i = 0; i < width; ++i) {
    const auto index = static_cast<std::size_t>(offset + i);
    const auto shiftIndex = littleEndian ? i : static_cast<std::uint8_t>(width - i - 1);
    value |= static_cast<std::uint64_t>(bytes_[index]) << (shiftIndex * 8U);
  }
  return Result<std::uint64_t>::success(value);
}

Result<std::span<const std::uint8_t>> CheckedReader::range(std::uint64_t offset,
                                                           std::uint64_t length) const {
  if (!contains(offset, length)) {
    return Result<std::span<const std::uint8_t>>::failure(
        {"LM0002", "requested byte range extends past input", offset});
  }
  return Result<std::span<const std::uint8_t>>::success(
      bytes_.subspan(static_cast<std::size_t>(offset), static_cast<std::size_t>(length)));
}

Result<std::string> CheckedReader::readString(std::uint64_t tableOffset,
                                              std::uint64_t tableSize,
                                              std::uint64_t stringOffset) const {
  if (!contains(tableOffset, tableSize) || stringOffset >= tableSize) {
    return Result<std::string>::failure({"LM0003", "string offset is outside its table", tableOffset});
  }
  const auto begin = tableOffset + stringOffset;
  const auto end = tableOffset + tableSize;
  auto cursor = begin;
  while (cursor < end && bytes_[static_cast<std::size_t>(cursor)] != 0) ++cursor;
  if (cursor == end) {
    return Result<std::string>::failure({"LM0004", "string is not terminated within its table", begin});
  }
  return Result<std::string>::success(std::string(
      reinterpret_cast<const char*>(bytes_.data() + static_cast<std::size_t>(begin)),
      static_cast<std::size_t>(cursor - begin)));
}
}
