#include "linkmap/object_file.hpp"

namespace linkmap {
Result<ObjectFile> parseObject(std::vector<std::uint8_t> bytes, std::filesystem::path path,
                               std::uint32_t id) {
  auto header = parseHeader(std::move(bytes), std::move(path));
  if (!header.hasValue()) return Result<ObjectFile>::failure(header.error());
  auto sections = parseSections(header.value());
  if (!sections.hasValue()) return Result<ObjectFile>::failure(sections.error());
  auto symbols = parseSymbols(header.value(), sections.value());
  if (!symbols.hasValue()) return Result<ObjectFile>::failure(symbols.error());
  auto relocations = parseRelocations(header.value(), sections.value(), symbols.value());
  if (!relocations.hasValue()) return Result<ObjectFile>::failure(relocations.error());
  ObjectFile object;
  object.id = id;
  object.parsed = std::move(header.value());
  object.sections = std::move(sections.value());
  object.symbols = std::move(symbols.value());
  object.relocations = std::move(relocations.value());
  return Result<ObjectFile>::success(std::move(object));
}

Result<ObjectFile> readObject(const std::filesystem::path& path, std::uint32_t id) {
  auto header = readObjectHeader(path);
  if (!header.hasValue()) return Result<ObjectFile>::failure(header.error());
  auto sections = parseSections(header.value());
  if (!sections.hasValue()) return Result<ObjectFile>::failure(sections.error());
  auto symbols = parseSymbols(header.value(), sections.value());
  if (!symbols.hasValue()) return Result<ObjectFile>::failure(symbols.error());
  auto relocations = parseRelocations(header.value(), sections.value(), symbols.value());
  if (!relocations.hasValue()) return Result<ObjectFile>::failure(relocations.error());
  ObjectFile object;
  object.id = id;
  object.parsed = std::move(header.value());
  object.sections = std::move(sections.value());
  object.symbols = std::move(symbols.value());
  object.relocations = std::move(relocations.value());
  return Result<ObjectFile>::success(std::move(object));
}
}
