/*
 * Check the DT_SONAME an object declares, which is NOT always the name of the file
 * holding it.  ProcControl depends on this to identify the thread library: libthread_db
 * asks for objects by SONAME, while the link map reports whatever path the loader
 * opened, and the two diverge whenever a dependency is redirected (glibc-hwcaps, an
 * ld.so.cache entry, an audit library).
 *
 * Usage: getSOName <file> <expected-soname>
 * An empty <expected-soname> means the object must declare no SONAME at all.
 */
#include "SymReader.h"
#include "Symtab.h"
#include "SymtabReader.h"

#include <cstdlib>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
  if(argc != 3) {
    std::cerr << "Usage: " << argv[0] << " file expected-soname\n";
    return EXIT_FAILURE;
  }

  std::string const file{argv[1]};
  std::string const expected{argv[2]};

  namespace st = Dyninst::SymtabAPI;

  // (1) the SymtabAPI accessor
  st::Symtab* s{};
  if(!st::Symtab::openFile(s, file)) {
    std::cerr << "Unable to open file '" << file << "'\n";
    return EXIT_FAILURE;
  }

  char const* raw = s->getSOName();
  std::string const from_symtab = raw ? raw : std::string{};

  if(from_symtab != expected) {
    std::cerr << "Symtab::getSOName() gave '" << from_symtab << "', expected '" << expected
              << "'\n";
    return EXIT_FAILURE;
  }

  // (2) the SymReader interface, which is the one ProcControl goes through
  Dyninst::SymbolReaderFactory* factory = st::getSymtabReaderFactory();
  if(!factory) {
    std::cerr << "No symbol reader factory\n";
    return EXIT_FAILURE;
  }

  Dyninst::SymReader* reader = factory->openSymbolReader(file);
  if(!reader) {
    std::cerr << "Unable to open symbol reader for '" << file << "'\n";
    return EXIT_FAILURE;
  }

  std::string const from_reader = reader->getSOName();
  factory->closeSymbolReader(reader);

  if(from_reader != expected) {
    std::cerr << "SymReader::getSOName() gave '" << from_reader << "', expected '" << expected
              << "'\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
