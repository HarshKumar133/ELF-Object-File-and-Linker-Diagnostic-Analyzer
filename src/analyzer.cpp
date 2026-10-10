#include "linkmap/diagnostic.hpp"

#include <algorithm>
#include <map>
#include <tuple>
#include <unordered_map>

namespace linkmap {
namespace {
struct Definition { const ObjectFile* object; const Symbol* symbol; };
bool eligibleDefinition(const Symbol& s) {
  return !s.isUndefined() && !s.name.empty() && (s.isGlobal() || s.isWeak()) &&
         (s.visibility == 0 || s.visibility == 3) && !s.isCommon() &&
         (s.sectionIndex < 0xff00 || s.isAbsolute()) &&
         (s.type == 0 || s.type == 1 || s.type == 2 || s.type == 6 || s.type == 10);
}
bool eligibleReference(const Symbol& s) {
  return s.isUndefined() && !s.name.empty() && (s.isGlobal() || s.isWeak()) &&
         (s.visibility == 0 || s.visibility == 3) &&
         (s.type == 0 || s.type == 1 || s.type == 2 || s.type == 6 || s.type == 10);
}
}

Analysis analyze(std::vector<ObjectFile> objects) {
  Analysis out;
  out.objects = std::move(objects);
  std::map<std::string, std::vector<Definition>> definitions;
  for (const auto& object : out.objects) {
    for (const auto& symbol : object.symbols) {
      if (symbol.isWeak() && !symbol.isUndefined()) ++out.weakDefinitions;
      if (eligibleDefinition(symbol)) definitions[symbol.name].push_back({&object, &symbol});
    }
  }
  for (const auto& [name, candidates] : definitions) {
    std::vector<Definition> strong;
    for (const auto& candidate : candidates) if (candidate.symbol->isGlobal()) strong.push_back(candidate);
    if (strong.size() > 1) {
      Diagnostic d{"LM5002", "error", "duplicate-strong-definition", name,
                   strong.front().object->id, strong.front().object->parsed.path.string(),
                   0, 0, false,
                   "multiple strong global definitions occur in the supplied object set",
                   "keep one strong definition or check which object files are included", {}};
      for (const auto& def : strong) d.relatedObjects.push_back(def.object->id);
      out.diagnostics.push_back(std::move(d));
      ++out.duplicateStrongSymbols;
    } else if (strong.empty() && candidates.size() > 1) {
      Diagnostic d{"LM5004", "note", "duplicate-weak-definitions", name,
                   candidates.front().object->id, candidates.front().object->parsed.path.string(),
                   0, 0, false,
                   "multiple weak definitions occur in the supplied object set; selection is linker-specific",
                   "check whether the weak alternatives are intentional", {}};
      for (const auto& def : candidates) d.relatedObjects.push_back(def.object->id);
      out.diagnostics.push_back(std::move(d));
    }
  }

  for (const auto& object : out.objects) {
    std::unordered_map<std::uint64_t, const Symbol*> symbolLookup;
    for (const auto& symbol : object.symbols)
      symbolLookup.emplace((static_cast<std::uint64_t>(symbol.tableSection) << 32U) | symbol.index,
                           &symbol);
    for (const auto& relocation : object.relocations) {
      const auto foundSymbol = symbolLookup.find(
          (static_cast<std::uint64_t>(relocation.symbolTableSection) << 32U) | relocation.symbolIndex);
      if (foundSymbol == symbolLookup.end() || !eligibleReference(*foundSymbol->second)) continue;
      const Symbol* reference = foundSymbol->second;
      const auto found = definitions.find(reference->name);
      if (found == definitions.end()) {
        if (reference->isWeak()) continue;
        Diagnostic d{"LM5001", "warning", "unresolved-within-input-set", reference->name,
                     object.id, object.parsed.path.string(), relocation.relocationSection,
                     relocation.offset, true,
                     "reference is unresolved within the supplied object set; another input or library may define it",
                     "check the omitted object files and library inputs", {}};
        out.diagnostics.push_back(std::move(d));
        ++out.unresolvedReferences;
        continue;
      }
      bool hasStrong = false;
      for (const auto& def : found->second) if (def.symbol->isGlobal()) hasStrong = true;
      std::vector<Definition> chosen;
      for (const auto& def : found->second)
        if (!hasStrong || def.symbol->isGlobal()) chosen.push_back(def);
      for (const auto& def : chosen) {
        if (def.object->id == object.id) continue;
        out.dependencies.push_back({object.id, def.object->id, reference->name,
                                    relocation.relocationSection, relocation.offset});
      }
      if (!hasStrong && !chosen.empty()) {
        Diagnostic d{"LM5003", "note", "weak-definition-match", reference->name,
                     object.id, object.parsed.path.string(), relocation.relocationSection,
                     relocation.offset, true,
                     "reference has only weak definitions in the supplied object set",
                     "verify that the intended strong definition is included", {}};
        for (const auto& def : chosen) d.relatedObjects.push_back(def.object->id);
        out.diagnostics.push_back(std::move(d));
      }
    }
  }
  std::sort(out.dependencies.begin(), out.dependencies.end(), [](const auto& a, const auto& b) {
    return std::tie(a.sourceObject, a.targetObject, a.symbol, a.relocationSection, a.offset) <
           std::tie(b.sourceObject, b.targetObject, b.symbol, b.relocationSection, b.offset);
  });
  std::sort(out.diagnostics.begin(), out.diagnostics.end(), [](const auto& a, const auto& b) {
    return std::tie(a.id, a.symbol, a.objectId, a.relocationSection, a.offset) <
           std::tie(b.id, b.symbol, b.objectId, b.relocationSection, b.offset);
  });
  return out;
}
}
