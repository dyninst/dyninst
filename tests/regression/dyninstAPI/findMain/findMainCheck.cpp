/*
 * Check that image::findMain recovers main from a stripped aarch64 executable.
 *
 * The expected address is main's symbol in the unstripped file.  The stripped copy has no
 * symbol for main, so the only way Dyninst can name it is findMain's analysis of _start.
 *
 * Usage: findMainCheck <unstripped> <stripped> [--no-main]
 * With --no-main, findMain must not name any main in the stripped file.
 */
#include "BPatch.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_image.h"
#include "BPatch_object.h"
#include "Symbol.h"
#include "Symtab.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace st = Dyninst::SymtabAPI;

static Dyninst::Address main_of(st::Symtab* obj) {
  std::vector<st::Symbol*> syms;
  if(!obj->findSymbol(syms, "main", st::Symbol::ST_UNKNOWN, st::anyName)) {
    return 0;
  }
  for(auto* s : syms) {
    if(s->getOffset() != 0 && s->getRegion() != nullptr) {
      return s->getOffset();
    }
  }
  return 0;
}

int main(int argc, char** argv) {
  if(argc != 3 && !(argc == 4 && std::string{argv[3]} == "--no-main")) {
    std::cerr << "Usage: " << argv[0] << " unstripped stripped [--no-main]\n";
    return EXIT_FAILURE;
  }
  bool const no_main = (argc == 4);

  st::Symtab* orig = nullptr;
  if(!st::Symtab::openFile(orig, argv[1])) {
    std::cerr << "cannot open " << argv[1] << '\n';
    return EXIT_FAILURE;
  }
  Dyninst::Address const expected = main_of(orig);
  if(expected == 0) {
    std::cerr << argv[1] << " has no main symbol\n";
    return EXIT_FAILURE;
  }

  BPatch bpatch;
  BPatch_binaryEdit* edit = bpatch.openBinary(argv[2], false);
  if(!edit) {
    std::cerr << "cannot open " << argv[2] << " for editing\n";
    return EXIT_FAILURE;
  }
  std::vector<BPatch_object*> objs;
  edit->getImage()->getObjects(objs);
  st::Symtab* stripped = nullptr;
  for(auto* o : objs) {
    st::Symtab* s = st::convert(o);
    if(s && s->file() == argv[2]) {
      stripped = s;
    }
  }
  if(!stripped) {
    std::cerr << "no object for " << argv[2] << '\n';
    return EXIT_FAILURE;
  }
  Dyninst::Address const found = main_of(stripped);

  std::cout << std::hex << "main symbol 0x" << expected << ", findMain 0x" << found << '\n';
  if(no_main) {
    return found == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
  }
  return found == expected ? EXIT_SUCCESS : EXIT_FAILURE;
}
