#include "linkmap/report.hpp"
#include "linkmap/version.hpp"

#include <iomanip>
#include <sstream>

namespace linkmap {
namespace {
std::string escapeJson(std::string_view input) {
  std::ostringstream out;
  for (const char raw : input) {
    const auto c = static_cast<unsigned char>(raw);
    switch (c) {
      case '"': out << "\\\""; break;
      case '\\': out << "\\\\"; break;
      case '\b': out << "\\b"; break;
      case '\f': out << "\\f"; break;
      case '\n': out << "\\n"; break;
      case '\r': out << "\\r"; break;
      case '\t': out << "\\t"; break;
      default:
        if (c < 0x20) out << "\\u00" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c) << std::dec;
        else out << static_cast<char>(c);
    }
  }
  return out.str();
}
std::string quote(std::string_view s) { return "\"" + escapeJson(s) + "\""; }
}

std::string renderText(const Analysis& a, std::string_view command, std::string_view verbosity) {
  std::ostringstream out;
  out << "LinkMap Analysis Report\nCommand: " << command << "\n\nInput Objects\n";
  for (const auto& o : a.objects) {
    const auto& h = o.parsed.header;
    out << "  [" << o.id << "] " << o.parsed.path.string() << " — ELF64 little-endian "
        << elfTypeName(h.type) << ", " << machineName(h.machine) << " (machine " << h.machine << ")\n";
    if (verbosity == "verbose")
      out << "      ABI=" << static_cast<unsigned>(h.osAbi) << " ABI-version="
          << static_cast<unsigned>(h.abiVersion) << " entry=0x" << std::hex << h.entry
          << " program-table=0x" << h.programHeaderOffset << " section-table=0x"
          << h.sectionHeaderOffset << std::dec << " phentsize=" << h.programHeaderEntrySize
          << " phnum=" << h.programHeaderCount << " shentsize=" << h.sectionHeaderEntrySize
          << " shnum=" << h.sectionHeaderCount << " shstrndx=" << h.sectionNameIndex << "\n";
    out << "      sections=" << o.sections.size() << " symbols=" << o.symbols.size()
        << " relocations=" << o.relocations.size() << "\n";
    if (command == "sections" || command == "inspect") {
      out << "      Sections:\n";
      for (const auto& s : o.sections)
        out << "        [" << s.index << "] " << (s.name.empty() ? "<unnamed>" : s.name)
            << " type=" << sectionTypeName(s.type) << " size=0x" << std::hex << s.size
            << " offset=0x" << s.offset << std::dec << (s.hasFilePayload() ? " file-backed" : " NOBITS") << "\n";
    }
    if (command == "symbols" || command == "inspect") {
      out << "      Symbols:\n";
      for (const auto& s : o.symbols)
        out << "        table=" << s.tableSection << " index=" << s.index << " "
            << (s.name.empty() ? "<unnamed>" : s.name) << " " << symbolBindingName(s.binding)
            << " " << symbolTypeName(s.type) << " " << symbolVisibilityName(s.visibility)
            << " shndx=" << s.sectionIndex
            << (s.isUndefined() ? " UND" : s.isAbsolute() ? " ABS" : s.isCommon() ? " COMMON" : "") << " value=0x"
            << std::hex << s.value << std::dec << "\n";
    }
    if (command == "relocations" || command == "inspect") {
      out << "      Relocations (offsets are relative to target sections for ET_REL):\n";
      for (const auto& r : o.relocations) {
        const auto& relocationSection = o.sections[r.relocationSection];
        const auto& targetSection = o.sections[r.targetSection];
        out << "        section=" << r.relocationSection << " (" << relocationSection.name
            << ") target=" << r.targetSection << " (" << targetSection.name << ")"
            << " offset=0x" << std::hex << r.offset << std::dec << " type="
            << (r.typeName.empty() ? "unknown" : r.typeName) << " (" << r.rawType << ") symbol="
            << (r.symbolName.empty() ? "<unnamed>" : r.symbolName) << "[" << r.symbolIndex << "]"
            << (r.hasAddend ? " addend=" + std::to_string(r.addend) : "")
            << " interpretation=" << (r.descriptionSupported ? "described" : "unsupported") << "\n";
      }
    }
  }
  if (command == "analyze" || command == "graph") {
    out << "\nCross-Object Resolution\n  Dependencies: " << a.dependencies.size()
        << "  unresolved references: " << a.unresolvedReferences
        << "  duplicate strong symbols: " << a.duplicateStrongSymbols
        << "  weak definitions: " << a.weakDefinitions << "\n";
    for (const auto& edge : a.dependencies)
      out << "  object[" << edge.sourceObject << "] -> object[" << edge.targetObject << "] via " << edge.symbol << "\n";
  }
  out << "\nDiagnostics\n";
  if (verbosity == "quiet") out << "  suppressed by --verbosity quiet\n";
  else if (a.diagnostics.empty()) out << "  none\n";
  for (const auto& d : a.diagnostics) {
    if (verbosity == "quiet") break;
    out << "  " << d.id << " " << d.severity << " " << d.category
        << " confidence=" << d.confidence;
    if (!d.symbol.empty()) out << " symbol=" << d.symbol;
    if (!d.relatedObjects.empty()) {
      out << " related-objects=[";
      for (std::size_t i=0; i<d.relatedObjects.size(); ++i) {
        if (i) out << ',';
        out << d.relatedObjects[i];
      }
      out << ']';
    }
    if (d.hasLocation) out << " object=" << d.objectPath << " relocation-section=" << d.relocationSection << " offset=0x" << std::hex << d.offset << std::dec;
    out << "\n    " << d.message << "\n    Action: " << d.action << "\n";
  }
  out << "\nScope: findings describe only the supplied ET_REL object set; this is not a full linker.\n";
  return out.str();
}

std::string renderJson(const Analysis& a, std::string_view command) {
  std::ostringstream out;
  out << "{\"schema_version\":1,\"tool_version\":" << quote(LINKMAP_VERSION)
      << ",\"command\":" << quote(command) << ",\"analysis_complete\":true,\"input_files\":[";
  for (std::size_t i=0; i<a.objects.size(); ++i) {
    if (i) out << ',';
    const auto& o=a.objects[i]; const auto& h=o.parsed.header;
    out << "{\"id\":" << o.id << ",\"path\":" << quote(o.parsed.path.string())
        << ",\"elf_class\":64,\"byte_order\":\"little\",\"type\":" << h.type
        << ",\"type_name\":" << quote(elfTypeName(h.type)) << ",\"machine\":" << h.machine
        << ",\"machine_name\":" << quote(machineName(h.machine))
        << ",\"elf_version\":" << h.version << ",\"os_abi\":" << static_cast<unsigned>(h.osAbi)
        << ",\"abi_version\":" << static_cast<unsigned>(h.abiVersion)
        << ",\"entry\":" << h.entry << ",\"program_header_offset\":" << h.programHeaderOffset
        << ",\"program_header_entry_size\":" << h.programHeaderEntrySize
        << ",\"program_header_count\":" << h.programHeaderCount
        << ",\"section_header_offset\":" << h.sectionHeaderOffset
        << ",\"section_header_entry_size\":" << h.sectionHeaderEntrySize
        << ",\"section_header_count\":" << h.sectionHeaderCount
        << ",\"section_name_index\":" << h.sectionNameIndex << ",\"sections\":[";
    for (std::size_t j=0; j<o.sections.size(); ++j) {
      if (j) out << ','; const auto& s=o.sections[j];
      out << "{\"index\":" << s.index << ",\"name\":" << quote(s.name)
          << ",\"type\":" << s.type << ",\"type_name\":" << quote(sectionTypeName(s.type))
          << ",\"flags\":" << s.flags << ",\"address\":" << s.address
          << ",\"offset\":" << s.offset << ",\"size\":" << s.size
          << ",\"link\":" << s.link << ",\"info\":" << s.info
          << ",\"alignment\":" << s.alignment << ",\"entry_size\":" << s.entrySize
          << ",\"file_backed\":" << (s.hasFilePayload() ? "true" : "false") << '}';
    }
    out << "],\"symbols\":[";
    for (std::size_t j=0; j<o.symbols.size(); ++j) {
      if (j) out << ','; const auto& s=o.symbols[j];
      out << "{\"table_section\":" << s.tableSection << ",\"index\":" << s.index
          << ",\"name\":" << quote(s.name) << ",\"value\":" << s.value << ",\"size\":" << s.size
          << ",\"binding\":" << quote(symbolBindingName(s.binding)) << ",\"type\":" << quote(symbolTypeName(s.type))
          << ",\"visibility\":" << quote(symbolVisibilityName(s.visibility))
          << ",\"section_index\":" << s.sectionIndex << ",\"undefined\":" << (s.isUndefined()?"true":"false") << '}';
    }
    out << "],\"relocations\":[";
    for (std::size_t j=0; j<o.relocations.size(); ++j) {
      if (j) out << ','; const auto& r=o.relocations[j];
      out << "{\"relocation_section\":" << r.relocationSection << ",\"target_section\":" << r.targetSection
          << ",\"relocation_section_name\":" << quote(o.sections[r.relocationSection].name)
          << ",\"target_section_name\":" << quote(o.sections[r.targetSection].name)
          << ",\"offset\":" << r.offset << ",\"raw_type\":" << r.rawType
          << ",\"type_name\":" << quote(r.typeName) << ",\"symbol_index\":" << r.symbolIndex
          << ",\"symbol\":" << quote(r.symbolName) << ",\"has_addend\":" << (r.hasAddend?"true":"false")
          << ",\"addend\":" << r.addend << ",\"description_supported\":" << (r.descriptionSupported?"true":"false") << '}';
    }
    out << "]}";
  }
  out << "],\"object_dependencies\":[";
  for (std::size_t i=0; i<a.dependencies.size(); ++i) {
    if (i) out << ','; const auto& e=a.dependencies[i];
    out << "{\"source\":" << e.sourceObject << ",\"target\":" << e.targetObject
        << ",\"symbol\":" << quote(e.symbol) << ",\"relocation_section\":" << e.relocationSection
        << ",\"offset\":" << e.offset << '}';
  }
  out << "],\"unresolved_references\":" << a.unresolvedReferences
      << ",\"duplicate_definitions\":" << a.duplicateStrongSymbols << ",\"diagnostics\":[";
  for (std::size_t i=0; i<a.diagnostics.size(); ++i) {
    if (i) out << ','; const auto& d=a.diagnostics[i];
    out << "{\"id\":" << quote(d.id) << ",\"severity\":" << quote(d.severity)
        << ",\"category\":" << quote(d.category) << ",\"symbol\":" << quote(d.symbol)
        << ",\"object\":" << quote(d.objectPath) << ",\"message\":" << quote(d.message)
        << ",\"action\":" << quote(d.action) << ",\"confidence\":" << quote(d.confidence)
        << ",\"has_location\":" << (d.hasLocation?"true":"false")
        << ",\"relocation_section\":" << d.relocationSection << ",\"offset\":" << d.offset
        << ",\"related_objects\":[";
    for (std::size_t j=0; j<d.relatedObjects.size(); ++j) {
      if (j) out << ',';
      out << d.relatedObjects[j];
    }
    out << "]}";
  }
  out << "]}";
  return out.str();
}

std::string renderDot(const Analysis& a) {
  std::ostringstream out;
  out << "digraph LinkMap {\n  rankdir=LR;\n";
  for (const auto& o : a.objects)
    out << "  object_" << o.id << " [label=" << quote(o.parsed.path.filename().string()) << "];\n";
  for (const auto& e : a.dependencies)
    out << "  object_" << e.sourceObject << " -> object_" << e.targetObject
        << " [label=" << quote(e.symbol + " @0x" + [&]{std::ostringstream x; x<<std::hex<<e.offset; return x.str();}()) << "];\n";
  std::size_t unresolvedId = 0;
  for (const auto& d : a.diagnostics) {
    if (d.id != "LM5001") continue;
    const auto node = "unresolved_" + std::to_string(unresolvedId++);
    out << "  " << node << " [shape=diamond,label=" << quote("unresolved: " + d.symbol) << "];\n"
        << "  object_" << d.objectId << " -> " << node << " [label=\"unresolved reference\"];\n";
  }
  out << "}\n";
  return out.str();
}

std::string renderSymbolDot(const Analysis& a, std::string_view symbol) {
  std::ostringstream out;
  out << "digraph LinkMapSymbol {\n  rankdir=LR;\n"
      << "  symbol [shape=ellipse,label=" << quote("symbol: " + std::string(symbol)) << "];\n";
  for (const auto& object : a.objects) {
    bool defines = false;
    bool references = false;
    for (const auto& s : object.symbols)
      if (s.name == symbol && !s.isUndefined() && (s.isGlobal() || s.isWeak())) defines = true;
    for (const auto& r : object.relocations)
      if (r.symbolName == symbol) references = true;
    if (defines) out << "  object_" << object.id << " [label=" << quote(object.parsed.path.filename().string()) << "];\n"
                     << "  object_" << object.id << " -> symbol [label=\"definition\"];\n";
    if (references) out << "  object_" << object.id << " [label=" << quote(object.parsed.path.filename().string()) << "];\n"
                        << "  symbol -> object_" << object.id << " [label=\"relocation reference\"];\n";
  }
  out << "}\n";
  return out.str();
}
}
