#pragma once

#include "linkmap/result.hpp"

#include <span>
#include <string>
#include <string_view>

namespace linkmap {
class CheckedReader {
 public:
  explicit CheckedReader(std::span<const std::uint8_t> bytes) : bytes_(bytes) {}
  [[nodiscard]] std::uint64_t size() const noexcept { return bytes_.size(); }
  [[nodiscard]] bool contains(std::uint64_t offset, std::uint64_t length) const noexcept;
  [[nodiscard]] Result<std::uint64_t> readUnsigned(std::uint64_t offset,
                                                   std::uint8_t width,
                                                   bool littleEndian = true) const;
  [[nodiscard]] Result<std::span<const std::uint8_t>> range(std::uint64_t offset,
                                                            std::uint64_t length) const;
  [[nodiscard]] Result<std::string> readString(std::uint64_t tableOffset,
                                               std::uint64_t tableSize,
                                               std::uint64_t stringOffset) const;

 private:
  std::span<const std::uint8_t> bytes_;
};
}
