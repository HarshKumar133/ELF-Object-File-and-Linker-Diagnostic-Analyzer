#pragma once

#include "linkmap/object_file.hpp"

namespace linkmap {
struct Diagnostic {
  std::string id;
  std::string severity;
  std::string category;
  std::string symbol;
  std::uint32_t objectId{};
  std::string objectPath;
  std::uint32_t relocationSection{};
  std::uint64_t offset{};
  bool hasLocation{};
  std::string message;
  std::string action;
  std::vector<std::uint32_t> relatedObjects;
  std::string confidence{"supported-subset"};
};
struct DependencyEdge {
  std::uint32_t sourceObject{};
  std::uint32_t targetObject{};
  std::string symbol;
  std::uint32_t relocationSection{};
  std::uint64_t offset{};
};
struct Analysis {
  std::vector<ObjectFile> objects;
  std::vector<Diagnostic> diagnostics;
  std::vector<DependencyEdge> dependencies;
  std::size_t unresolvedReferences{};
  std::size_t duplicateStrongSymbols{};
  std::size_t weakDefinitions{};
};

[[nodiscard]] Analysis analyze(std::vector<ObjectFile> objects);
}
