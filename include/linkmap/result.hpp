#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <utility>

namespace linkmap {
struct Error {
  std::string code;
  std::string message;
  std::uint64_t offset{};
};

template <class T> class Result {
 public:
  static Result success(T value) { return Result(std::move(value)); }
  static Result failure(Error error) { return Result(std::move(error)); }
  [[nodiscard]] bool hasValue() const noexcept { return value_.has_value(); }
  [[nodiscard]] const T& value() const { return value_.value(); }
  [[nodiscard]] T& value() { return value_.value(); }
  [[nodiscard]] const Error& error() const { return error_.value(); }

 private:
  explicit Result(T value) : value_(std::move(value)) {}
  explicit Result(Error error) : error_(std::move(error)) {}
  std::optional<T> value_;
  std::optional<Error> error_;
};
}
