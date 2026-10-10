#include "linkmap/diagnostic.hpp"
#include "linkmap/report.hpp"
#include "linkmap/version.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace {
void help(std::ostream& out) {
  out << "LinkMap " << LINKMAP_VERSION << "\n"
      << "Inspect ELF64 little-endian relocatable objects and explain selected relationships.\n\n"
      << "Usage: linkmap COMMAND FILE... [--format text|json|dot] [--output FILE]\n\n"
      << "       [--verbosity quiet|normal|verbose]\n"
      << "       graph FILE... --symbol NAME\n"
      << "Commands: inspect, sections, symbols, relocations, analyze, graph, help, version\n"
      << "Exit codes: 0 clean; 1 findings; 2 CLI error; 3 input/parse error; 4 unsupported input; 7 output error.\n"
      << "LinkMap analyzes inputs only; it does not link or execute object files.\n";
}
int errorExit(const linkmap::Error& error) {
  std::cerr << "linkmap: " << error.code << ": " << error.message << " (offset " << error.offset << ")\n";
  if (error.code == "LM1002" || error.code == "LM1003" || error.code == "LM1007") return 4;
  return 3;
}
}

int main(int argc, char** argv) {
  if (argc < 2 || std::string_view(argv[1]) == "help" || std::string_view(argv[1]) == "--help" ||
      std::string_view(argv[1]) == "-h") {
    if (argc > 2) { std::cerr << "linkmap: help accepts no additional arguments\n"; return 2; }
    help(std::cout); return 0;
  }
  if (std::string_view(argv[1]) == "version" || std::string_view(argv[1]) == "--version") {
    if (argc > 2) { std::cerr << "linkmap: version accepts no additional arguments\n"; return 2; }
    std::cout << "linkmap " << LINKMAP_VERSION << '\n'; return 0;
  }
  const std::string command = argv[1];
  const std::vector<std::string> commands{"inspect", "sections", "symbols", "relocations", "analyze", "graph"};
  if (std::find(commands.begin(), commands.end(), command) == commands.end()) {
    std::cerr << "linkmap: unknown command: " << command << "\n";
    return 2;
  }
  std::string format = command == "graph" ? "dot" : "text";
  std::string verbosity = "normal";
  std::filesystem::path outputPath;
  std::string symbolFilter;
  std::vector<std::filesystem::path> inputs;
  for (int i=2; i<argc; ++i) {
    const std::string arg=argv[i];
    if (arg == "--format") {
      if (++i >= argc) { std::cerr << "linkmap: --format requires a value\n"; return 2; }
      format=argv[i];
    } else if (arg == "--output") {
      if (++i >= argc) { std::cerr << "linkmap: --output requires a path\n"; return 2; }
      outputPath=argv[i];
    } else if (arg == "--symbol") {
      if (++i >= argc) { std::cerr << "linkmap: --symbol requires a name\n"; return 2; }
      symbolFilter=argv[i];
    } else if (arg == "--verbosity") {
      if (++i >= argc) { std::cerr << "linkmap: --verbosity requires a value\n"; return 2; }
      verbosity=argv[i];
    } else if (arg.starts_with('-')) {
      std::cerr << "linkmap: unknown option: " << arg << "\n"; return 2;
    } else inputs.emplace_back(arg);
  }
  if (inputs.empty()) { std::cerr << "linkmap: command requires at least one input file\n"; return 2; }
  if (format != "text" && format != "json" && format != "dot") {
    std::cerr << "linkmap: unsupported format: " << format << "\n"; return 2;
  }
  if (verbosity != "quiet" && verbosity != "normal" && verbosity != "verbose") {
    std::cerr << "linkmap: unsupported verbosity: " << verbosity << "\n"; return 2;
  }
  if (format == "dot" && command != "graph") {
    std::cerr << "linkmap: DOT format is available only for graph\n"; return 2;
  }
  if (!symbolFilter.empty() && command != "graph") {
    std::cerr << "linkmap: --symbol is available only for graph\n"; return 2;
  }
  if ((command == "analyze" || command == "graph") && inputs.size() < 2) {
    std::cerr << "linkmap: " << command << " requires at least two object files\n"; return 2;
  }

  std::vector<linkmap::ObjectFile> objects;
  objects.reserve(inputs.size());
  for (std::size_t i=0; i<inputs.size(); ++i) {
    auto parsed=linkmap::readObject(inputs[i], static_cast<std::uint32_t>(i));
    if (!parsed.hasValue()) {
      std::cerr << "linkmap: " << inputs[i].string() << ": ";
      return errorExit(parsed.error());
    }
    objects.push_back(std::move(parsed.value()));
  }
  auto analysis=linkmap::analyze(std::move(objects));
  std::string rendered;
  if (format == "json") rendered=linkmap::renderJson(analysis, command);
  else if (format == "dot" && !symbolFilter.empty()) rendered=linkmap::renderSymbolDot(analysis, symbolFilter);
  else if (format == "dot") rendered=linkmap::renderDot(analysis);
  else rendered=linkmap::renderText(analysis, command, verbosity);
  if (!outputPath.empty()) {
    std::error_code filesystemError;
    const bool exists = std::filesystem::exists(outputPath, filesystemError);
    if (filesystemError) {
      std::cerr << "linkmap: could not inspect output path: " << outputPath.string() << "\n";
      return 7;
    }
    if (exists) {
      std::cerr << "linkmap: refusing to overwrite existing output file: " << outputPath.string() << "\n";
      return 7;
    }
    std::ofstream out(outputPath, std::ios::binary);
    if (!out || !(out << rendered)) {
      std::cerr << "linkmap: could not write output file: " << outputPath.string() << "\n";
      return 7;
    }
  } else std::cout << rendered;
  return analysis.diagnostics.empty() ? 0 : 1;
}
