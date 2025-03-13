#include "find_main.h"

#include "CFG.h"
#include "debug.h"
#include "dyntypes.h"
#include "image.h"
#include "Symbol.h"
#include "Symtab.h"

#include <string>
#include <vector>

namespace st = Dyninst::SymtabAPI;
namespace pa = Dyninst::ParseAPI;

namespace Dyninst { namespace DyninstAPI {

Dyninst::Address find_main(st::Symtab* linkedFile) {

  startup_printf("find_main: looking for 'main' in %s\n", linkedFile->name().c_str());

  // Only look for 'main' in executables, including PIE, but not
  // other binaries that could be executable (for an example,
  // see ObjectELF::isOnlyExecutable()).
  if(!linkedFile->isExec()) {
    startup_printf("find_main: not an executable\n");
    return Dyninst::ADDR_NULL;
  }

  // It must have at least one code region
  {
    std::vector<st::Region*> regions;
    linkedFile->getCodeRegions(regions);
    if(regions.size() == 0UL) {
      startup_printf("find_main: No main found; no code regions\n");
      return Dyninst::ADDR_NULL;
    }
  }

  // Check for a known symbol name
  for(const char* name : main_function_names()) {
    std::vector<st::Function*> funcs;
    if(linkedFile->findFunctionsByName(funcs, name)) {
      if(dyn_debug_startup) {
        startup_printf("find_main: found ");
        for(auto* f : funcs) {
          startup_printf("{ %s@0x%lx}, ", f->getName().c_str(), f->getOffset());
        }
        startup_printf("\n");
      }
      return funcs[0]->getFirstSymbol()->getOffset();
    }
  }

  // Report a non-stripped binary, but don't fail.
  // This indicates we need to expand our list of possible symbols for 'main'
  if(!linkedFile->isStripped()) {
    startup_printf("find_main: no symbol found, but binary isn't stripped\n");
  }

  // We need to do actual binary analysis from here
  startup_printf("find_main: no symbol found; attempting manual search\n");

  const auto entry_address = static_cast<Dyninst::Address>(linkedFile->getEntryOffset());
  const st::Region* entry_region = linkedFile->findEnclosingRegion(entry_address);

  if(!entry_region) {
    startup_printf("find_main: no region found at entry 0x%lx\n", entry_address);
    return Dyninst::ADDR_NULL;
  }

  pa::SymtabCodeSource scs(linkedFile);

  std::set<pa::CodeRegion*> regions;
  scs.findRegions(entry_address, regions);

  if(regions.empty()) {
    startup_printf("find_main: no region contains 0x%lx\n", entry_address);
    return Dyninst::ADDR_NULL;
  }

  // We should only get one region
  if(regions.size() > 1UL) {
    startup_printf("find_main: found %lu possibly-overlapping regions for 0x%lx\n", regions.size(), entry_address);
    return Dyninst::ADDR_NULL;
  }

  pa::CodeObject co = [&scs]() {
    // To save time, delay the parsing
    pa::CFGFactory* f{};
    pa::ParseCallback* cb{};
    constexpr bool defensive_mode = false;
    constexpr bool delay_parse = true;
    return pa::CodeObject(&scs, f, cb, defensive_mode, delay_parse);
  }();

  pa::Function* entry_point = [&]() {
    pa::CodeRegion* region = *(regions.begin());
    constexpr bool recursive = true;
    co.parse(region, entry_address, recursive);
    return co.findFuncByEntry(region, entry_address);
  }();

  if(!entry_point) {
    startup_printf("find_main: couldn't find function at entry 0x%lx\n", entry_address);
    return Dyninst::ADDR_NULL;
  }

  startup_printf("find_main: found '%s' at entry 0x%lx\n", entry_point->name().c_str(), entry_address);

  const auto& edges = entry_point->callEdges();
  if(edges.empty()) {
    startup_printf("find_main: no call edges\n");
    return Dyninst::ADDR_NULL;
  }

  // In libc, the entry point is _start which only calls __libc_start_main, so
  // assume the first call is the one we want.
  pa::Block* b = (*edges.begin())->src();

  // Try architecture-specific searches
  auto main_addr = [&]() {
    auto file_arch = linkedFile->getArchitecture();

    if(file_arch == Dyninst::Arch_ppc32 || file_arch == Dyninst::Arch_ppc64) {
      return DyninstAPI::ppc::find_main_by_toc(linkedFile, entry_point, b);
    }

    if(file_arch == Dyninst::Arch_x86 || file_arch == Dyninst::Arch_x86_64) {
      return DyninstAPI::x86::find_main(entry_point, b);
    }

    return Dyninst::ADDR_NULL;
  }();

  if(main_addr == Dyninst::ADDR_NULL || !scs.isValidAddress(main_addr)) {
    startup_printf("find_main: unable to find valid entry for 'main'");
    return Dyninst::ADDR_NULL;
  }

  return main_addr;
}

std::vector<st::Symbol*> get_missing_symbols(st::Symtab* linkedFile, Dyninst::Address address_of_main) {

  std::vector<st::Symbol*> new_symbols{};

  const auto entry_address = static_cast<Dyninst::Address>(linkedFile->getEntryOffset());
  st::Region* entry_region = linkedFile->findEnclosingRegion(entry_address);

  // Windows PE has no startup/shutdown (e.g., .init/.fini) so treat it separately
  // NOTE: adds global symbols with default visibility
  if(linkedFile->getFileFormat() == st::FileFormat::PE) {

    auto make_sym_ = [&](std::string const& name, Dyninst::Address addr, unsigned size) {
      // clang-format off
      return new st::Symbol(
        name,
        st::Symbol::ST_FUNCTION,
        st::Symbol::SL_GLOBAL,
        st::Symbol::SV_DEFAULT,
        addr,
        linkedFile->getDefaultModule(),
        entry_region,
        size
      );
      // clang-format on
    };

    std::vector<st::Symbol*> syms{};
    if(!linkedFile->findSymbol(syms, "start", st::Symbol::ST_UNKNOWN, SymtabAPI::mangledName)) {
      // use 'start' for mainCRTStartup.
      new_symbols.push_back(make_sym_("start", entry_address, UINT_MAX));
    } else {
      // add entry point as main given that nothing else was found
      new_symbols.push_back(make_sym_("main", address_of_main, 0U));
    }
    return new_symbols;
  }

  // ELF-like files
  // NOTE: adds local symbols with internal visibility
  auto make_sym_ = [&linkedFile](std::string const& name, Dyninst::Address addr, st::Region *region) {
    // clang-format off
    return new st::Symbol(
      name,
      st::Symbol::ST_FUNCTION,
      st::Symbol::SL_LOCAL,
      st::Symbol::SV_INTERNAL,
      addr,
      linkedFile->getDefaultModule(),
      region,
      0U
    );
    // clang-format on
  };

  // Add either a static or dynamic symbol for 'main'
  {
    st::Region* region{};
    bool const has_plt = linkedFile->findRegion(region, ".plt");
    if(has_plt && region->isOffsetInRegion(address_of_main)) {
      auto* s = make_sym_("DYNINST_pltMain", address_of_main, entry_region);
      new_symbols.push_back(s);
    } else {
      new_symbols.push_back(make_sym_("main", address_of_main, entry_region));
    }
  }

  {
    std::vector<st::Function*> funcs{};
    if(!linkedFile->findFunctionsByName(funcs, "_start")) {
      auto s = make_sym_("_start", entry_region->getMemOffset(), entry_region);
      new_symbols.push_back(s);
    }
  }

  // Add symbols for init/fini
  for(std::string sec : {"init", "fini"}) {
    std::vector<st::Function*> funcs{};
    if(linkedFile->findFunctionsByName(funcs, "_"+sec)) {
      st::Region* reg{};
      if(linkedFile->findRegion(reg, "."+sec)) {
        auto* s = make_sym_("_"+sec, reg->getMemOffset(), reg);
        new_symbols.push_back(s);
      }
    }
  }

  /*
   * System V Application Binary Interface
   * AMD64 Architecture Processor Supplement
   * Version 1.0 - February 22, 2024
   *
   * 5.2 Dynamic Linking
   * Global Offset Table (GOT)
   *
   * This should be safe to use for any SystemV-compliant platform; not just x86.
   */
  st::Region* region{};
  if(linkedFile->findRegion(region, ".dynamic")) {
    std::vector<st::Symbol*> syms{};
    linkedFile->findSymbol(syms, "_DYNAMIC", st::Symbol::ST_UNKNOWN, st::mangledName);
    if(syms.empty()) {
      // clang-format off
      st::Symbol *s = new st::Symbol(
        "_DYNAMIC",
        st::Symbol::ST_OBJECT,
        st::Symbol::SL_LOCAL,
        st::Symbol::SV_INTERNAL,
        region->getMemOffset(),
        linkedFile->getDefaultModule(),
        region,
        0
      );
      // clang-format on
      new_symbols.push_back(s);
    }
  }

  return new_symbols;
}

}}
