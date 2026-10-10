#include "linkmap/checked_reader.hpp"
#include "linkmap/diagnostic.hpp"
#include "linkmap/object_parser.hpp"
#include "linkmap/relocation.hpp"
#include "linkmap/section.hpp"
#include "linkmap/symbol.hpp"

#include <array>
#include <cstdint>
#include <iostream>
#include <utility>
#include <vector>

namespace {
int failures = 0;
void check(bool condition, const char* name) {
  if (!condition) { std::cerr << "FAIL: " << name << '\n'; ++failures; }
}
std::vector<std::uint8_t> validHeader() {
  std::vector<std::uint8_t> b(128);
  b[0]=0x7f; b[1]='E'; b[2]='L'; b[3]='F'; b[4]=2; b[5]=1; b[6]=1;
  auto put = [&](std::size_t off, std::uint64_t n, int width) {
    for (int i=0; i<width; ++i) b[off+static_cast<std::size_t>(i)] = static_cast<std::uint8_t>(n >> (i*8));
  };
  put(16,1,2); put(18,62,2); put(20,1,4); put(52,64,2);
  put(40,64,8); put(58,64,2); put(60,1,2); put(62,0,2);
  return b;
}
std::vector<std::uint8_t> validSections() {
  std::vector<std::uint8_t> b(224);
  b[0]=0x7f; b[1]='E'; b[2]='L'; b[3]='F'; b[4]=2; b[5]=1; b[6]=1;
  auto put = [&](std::size_t off, std::uint64_t n, int width) {
    for (int i=0; i<width; ++i) b[off+static_cast<std::size_t>(i)] = static_cast<std::uint8_t>(n >> (i*8));
  };
  put(16,1,2); put(18,62,2); put(20,1,4); put(40,64,8); put(52,64,2);
  put(58,64,2); put(60,2,2); put(62,1,2);
  const std::size_t sec = 64 + 64;
  put(sec,1,4); put(sec+4,3,4); put(sec+24,192,8); put(sec+32,11,8); put(sec+48,1,8);
  const char names[] = "\0.shstrtab\0";
  for (std::size_t i=0; i<sizeof(names); ++i) b[192+i] = static_cast<std::uint8_t>(names[i]);
  return b;
}
std::vector<std::uint8_t> validSymbols() {
  std::vector<std::uint8_t> b(448);
  b[0]=0x7f; b[1]='E'; b[2]='L'; b[3]='F'; b[4]=2; b[5]=1; b[6]=1;
  auto put = [&](std::size_t off, std::uint64_t n, int width) {
    for (int i=0; i<width; ++i) b[off+static_cast<std::size_t>(i)] = static_cast<std::uint8_t>(n >> (i*8));
  };
  put(16,1,2); put(18,62,2); put(20,1,4); put(40,64,8); put(52,64,2);
  put(58,64,2); put(60,4,2); put(62,3,2);
  const std::size_t s1=128, s2=192, s3=256;
  put(s1,1,4); put(s1+4,3,4); put(s1+24,320,8); put(s1+32,9,8); put(s1+48,1,8);
  put(s2,9,4); put(s2+4,2,4); put(s2+24,336,8); put(s2+32,72,8);
  put(s2+40,1,4); put(s2+44,1,4); put(s2+48,8,8); put(s2+56,24,8);
  put(s3,17,4); put(s3+4,3,4); put(s3+24,408,8); put(s3+32,28,8); put(s3+48,1,8);
  const char strings[] = "\0foo\0bar\0";
  for (std::size_t i=0; i<sizeof(strings); ++i) b[320+i]=static_cast<std::uint8_t>(strings[i]);
  // Symbol 1 is an undefined global function named foo.
  put(336+24,1,4); b[336+24+4]=0x12; put(336+24+6,0,2);
  // Symbol 2 is a weak absolute object named bar (a defined symbol at value zero).
  put(336+48,5,4); b[336+48+4]=0x21; put(336+48+6,0xfff1,2);
  const char shnames[] = "\0.strtab\0.symtab\0.shstrtab\0";
  for (std::size_t i=0; i<sizeof(shnames); ++i) b[408+i]=static_cast<std::uint8_t>(shnames[i]);
  return b;
}
std::vector<std::uint8_t> validRelocations() {
  std::vector<std::uint8_t> b(616);
  b[0]=0x7f; b[1]='E'; b[2]='L'; b[3]='F'; b[4]=2; b[5]=1; b[6]=1;
  auto put = [&](std::size_t off, std::uint64_t n, int width) {
    for (int i=0; i<width; ++i) b[off+static_cast<std::size_t>(i)] = static_cast<std::uint8_t>(n >> (i*8));
  };
  put(16,1,2); put(18,62,2); put(20,1,4); put(40,64,8); put(52,64,2);
  put(58,64,2); put(60,6,2); put(62,4,2);
  const std::size_t s1=128, s2=192, s3=256, s4=320, s5=384;
  put(s1,1,4); put(s1+4,1,4); put(s1+24,448,8); put(s1+32,8,8); put(s1+48,1,8);
  put(s2,7,4); put(s2+4,3,4); put(s2+24,456,8); put(s2+32,9,8); put(s2+48,1,8);
  put(s3,15,4); put(s3+4,2,4); put(s3+24,472,8); put(s3+32,72,8);
  put(s3+40,2,4); put(s3+44,1,4); put(s3+48,8,8); put(s3+56,24,8);
  put(s4,23,4); put(s4+4,3,4); put(s4+24,544,8); put(s4+32,44,8); put(s4+48,1,8);
  put(s5,33,4); put(s5+4,4,4); put(s5+24,592,8); put(s5+32,24,8);
  put(s5+40,3,4); put(s5+44,1,4); put(s5+48,8,8); put(s5+56,24,8);
  const char strings[] = "\0foo\0bar\0";
  for (std::size_t i=0; i<sizeof(strings); ++i) b[456+i]=static_cast<std::uint8_t>(strings[i]);
  put(472+24,1,4); b[472+24+4]=0x12;
  put(472+48,5,4); b[472+48+4]=0x21; put(472+48+6,0xfff1,2);
  const char shnames[] = "\0.text\0.strtab\0.symtab\0.shstrtab\0.rela.text\0";
  for (std::size_t i=0; i<sizeof(shnames); ++i) b[544+i]=static_cast<std::uint8_t>(shnames[i]);
  put(592,4,8); put(592+8,(1ULL<<32U)|4U,8); put(592+16,static_cast<std::uint64_t>(-4),8);
  return b;
}
}

int main() {
  const std::array<std::uint8_t, 4> bytes{1, 2, 3, 4};
  linkmap::CheckedReader reader(bytes);
  auto little = reader.readUnsigned(0, 4, true);
  auto big = reader.readUnsigned(0, 4, false);
  check(little.hasValue() && little.value() == 0x04030201, "little endian read");
  check(big.hasValue() && big.value() == 0x01020304, "big endian read");
  check(!reader.readUnsigned(3, 2).hasValue(), "reject out-of-bounds read");
  check(!reader.readUnsigned(0, 0).hasValue(), "reject zero-width read");
  check(!reader.contains(UINT64_MAX, 2), "reject overflowing range");
  check(!reader.readString(0, bytes.size(), 0).hasValue(), "reject unterminated string within table");

  auto valid = linkmap::parseHeader(validHeader());
  check(valid.hasValue(), "parse valid ELF64 ET_REL header");
  if (valid.hasValue()) {
    check(valid.value().header.machine == 62, "read machine field");
    check(linkmap::machineName(62) == "AMD x86-64", "machine name");
  }
  auto truncated = validHeader(); truncated.resize(10);
  check(!linkmap::parseHeader(std::move(truncated)).hasValue(), "reject truncated ident");
  auto badMagic = validHeader(); badMagic[0] = 0;
  check(!linkmap::parseHeader(std::move(badMagic)).hasValue(), "reject invalid magic");
  auto class32 = validHeader(); class32[4] = 1;
  check(!linkmap::parseHeader(std::move(class32)).hasValue(), "reject unsupported ELF class");
  auto bigEndian = validHeader(); bigEndian[5] = 2;
  check(!linkmap::parseHeader(std::move(bigEndian)).hasValue(), "reject unsupported byte order");
  std::uint32_t state=0x91e10da5U;
  for (std::size_t n=0; n<10000; ++n) {
    const auto length=n%160;
    std::vector<std::uint8_t> random(length);
    for (auto& byte : random) {
      state ^= state << 13U; state ^= state >> 17U; state ^= state << 5U;
      byte=static_cast<std::uint8_t>(state & 0xffU);
    }
    (void)linkmap::parseHeader(std::move(random));
  }

  linkmap::ObjectFile mainObject;
  mainObject.id=0; mainObject.parsed.path="main.o";
  linkmap::Symbol ref; ref.tableSection=1; ref.index=1; ref.name="calc"; ref.binding=1; ref.type=2;
  mainObject.symbols.push_back(ref);
  linkmap::Relocation rel; rel.symbolTableSection=1; rel.symbolIndex=1; rel.symbolName="calc";
  rel.offset=12; rel.relocationSection=2;
  mainObject.relocations.push_back(rel);
  linkmap::ObjectFile provider;
  provider.id=1; provider.parsed.path="math.o";
  linkmap::Symbol def; def.tableSection=1; def.index=1; def.name="calc"; def.binding=1; def.type=2; def.sectionIndex=1;
  provider.symbols.push_back(def);
  auto resolved=linkmap::analyze({mainObject,provider});
  check(resolved.unresolvedReferences == 0 && resolved.dependencies.size() == 1, "resolve cross-object symbol");
  auto unresolved=linkmap::analyze({mainObject});
  check(unresolved.unresolvedReferences == 1 && unresolved.diagnostics[0].id == "LM5001", "report unresolved supplied-set reference");
  linkmap::ObjectFile localProvider=provider; localProvider.id=4; localProvider.parsed.path="local.o";
  localProvider.symbols[0].binding=0;
  auto localOnly=linkmap::analyze({mainObject,localProvider});
  check(localOnly.unresolvedReferences == 1 && localOnly.dependencies.empty(), "local same-name symbol cannot satisfy external reference");
  linkmap::ObjectFile secondProvider=provider; secondProvider.id=2; secondProvider.parsed.path="other.o";
  auto duplicate=linkmap::analyze({provider,secondProvider});
  check(duplicate.duplicateStrongSymbols == 1 && duplicate.diagnostics[0].id == "LM5002", "report duplicate strong definitions");
  linkmap::ObjectFile weakProvider=provider; weakProvider.id=3; weakProvider.parsed.path="weak.o";
  weakProvider.symbols[0].binding=2;
  auto weakMatch=linkmap::analyze({mainObject,weakProvider});
  check(weakMatch.dependencies.size() == 1 && weakMatch.diagnostics.size() == 1 &&
        weakMatch.diagnostics[0].id == "LM5003", "report weak-only definition match");
  auto precedence=linkmap::analyze({mainObject,provider,weakProvider});
  check(precedence.dependencies.size() == 1 && precedence.dependencies[0].targetObject == 1,
        "strong definition takes precedence over weak graph candidate");
  linkmap::ObjectFile anotherWeak=weakProvider; anotherWeak.id=5; anotherWeak.parsed.path="weak2.o";
  auto weakDuplicates=linkmap::analyze({weakProvider,anotherWeak});
  check(weakDuplicates.diagnostics.size() == 1 && weakDuplicates.diagnostics[0].id == "LM5004",
        "report duplicate weak definitions with caution");
  auto executable = validHeader(); executable[16] = 2;
  check(!linkmap::parseHeader(std::move(executable)).hasValue(), "reject non-ET_REL");
  auto sectionsObject = linkmap::parseHeader(validSections());
  check(sectionsObject.hasValue(), "parse section fixture header");
  if (sectionsObject.hasValue()) {
    auto sections = linkmap::parseSections(sectionsObject.value());
    check(sections.hasValue() && sections.value().size() == 2, "parse section table");
    if (sections.hasValue()) {
      check(sections.value()[1].name == ".shstrtab", "resolve section name");
      check(sections.value()[1].type == 3 && sections.value()[1].hasFilePayload(), "classify string table");
    }
  }
  auto badSections = validSections(); badSections[64 + 64 + 24] = 0xff;
  auto badObject = linkmap::parseHeader(std::move(badSections));
  check(badObject.hasValue() && !linkmap::parseSections(badObject.value()).hasValue(), "reject out-of-range section payload");
  auto symbolObject = linkmap::parseHeader(validSymbols());
  check(symbolObject.hasValue(), "parse symbol fixture header");
  if (symbolObject.hasValue()) {
    auto sections = linkmap::parseSections(symbolObject.value());
    check(sections.hasValue(), "parse symbol fixture sections");
    if (sections.hasValue()) {
      auto symbols = linkmap::parseSymbols(symbolObject.value(), sections.value());
      check(symbols.hasValue() && symbols.value().size() == 3, "parse symbol table records");
      if (symbols.hasValue()) {
        check(symbols.value()[1].name == "foo" && symbols.value()[1].isUndefined(), "global undefined symbol");
        check(symbols.value()[2].name == "bar" && symbols.value()[2].isWeak() && symbols.value()[2].isAbsolute(), "weak absolute symbol at zero");
      }
    }
  }
  auto relocationObject = linkmap::parseHeader(validRelocations());
  check(relocationObject.hasValue(), "parse relocation fixture header");
  if (relocationObject.hasValue()) {
    auto sections = linkmap::parseSections(relocationObject.value());
    check(sections.hasValue(), "parse relocation fixture sections");
    if (sections.hasValue()) {
      auto symbols = linkmap::parseSymbols(relocationObject.value(), sections.value());
      check(symbols.hasValue(), "parse relocation fixture symbols");
      if (symbols.hasValue()) {
        auto relocs = linkmap::parseRelocations(relocationObject.value(), sections.value(), symbols.value());
        check(relocs.hasValue() && relocs.value().size() == 1, "parse RELA entry");
        if (relocs.hasValue() && !relocs.value().empty()) {
          check(relocs.value()[0].offset == 4 && relocs.value()[0].typeName == "R_X86_64_PLT32", "decode x86-64 relocation");
          check(relocs.value()[0].symbolName == "foo" && relocs.value()[0].hasAddend && relocs.value()[0].addend == -4, "relocation symbol and addend");
        }
      }
    }
  }
  auto unknownBytes=validRelocations(); unknownBytes[600]=0xfe;
  auto unknownObject=linkmap::parseObject(std::move(unknownBytes), "unknown.o");
  check(unknownObject.hasValue() && unknownObject.value().relocations[0].rawType == 254 &&
        !unknownObject.value().relocations[0].descriptionSupported, "preserve unknown relocation type");
  auto badIndexBytes=validRelocations(); badIndexBytes[604]=0xff;
  auto badIndexObject=linkmap::parseObject(std::move(badIndexBytes), "bad-index.o");
  check(!badIndexObject.hasValue() && badIndexObject.error().code == "LM4007", "reject invalid relocation symbol index");
  auto relBytes=validRelocations();
  relBytes[388]=9; relBytes[416]=16; relBytes[440]=16;
  auto relObject=linkmap::parseObject(std::move(relBytes), "rel.o");
  check(relObject.hasValue() && relObject.value().relocations.size() == 1 &&
        !relObject.value().relocations[0].hasAddend, "parse REL entry without explicit addend");
  return failures == 0 ? 0 : 1;
}
