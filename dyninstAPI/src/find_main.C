#include "CFG.h"
#include "debug.h"
#include "dyntypes.h"
#include "find_main.h"
#include "image.h"
#include "Symbol.h"
#include "Symtab.h"

#include <memory>
#include <string>
#include <vector>

namespace st = Dyninst::SymtabAPI;
namespace pa = Dyninst::ParseAPI;

namespace Dyninst { namespace DyninstAPI {

  main_addr_t find_main(st::Symtab *linkedFile) {
    startup_printf("find_main: looking for 'main' in %s\n", linkedFile->name().c_str());

    // Only look for 'main' in executables, including PIE and executable DSOs.
    // see ObjectELF::isOnlyExecutable()
    if(!linkedFile->isExec()) {
      startup_printf("find_main: not an executable\n");
      return main_addr_t{}.not_executable();
    }

    // It must have at least one code region
    {
      std::vector<st::Region *> regions;
      linkedFile->getCodeRegions(regions);
      if(regions.size() == 0UL) {
        startup_printf("find_main: No main found; no code regions\n");
        return main_addr_t{}.invalid();
      }
    }

    // Check for a known symbol name
    for(char const *name : main_function_names()) {
      std::vector<st::Function *> funcs;
      if(linkedFile->findFunctionsByName(funcs, name)) {
        const auto addr = funcs[0]->getFirstSymbol()->getOffset();
        startup_printf("find_main: found 'main' symbol at 0x%lx\n", addr);
        return main_addr_t{addr};
      }
    }

    // Report a non-stripped binary, but don't fail.
    // This indicates we need to expand our list of possible symbols for 'main'
    if(!linkedFile->isStripped()) {
      startup_printf("find_main: no symbol found, but binary isn't stripped\n");
    }

    // We need to do actual binary analysis from here
    startup_printf("find_main: no symbol found; attempting manual search\n");

    auto const entry_address = static_cast<Dyninst::Address>(linkedFile->getEntryOffset());
    st::Region *entry_region = linkedFile->findEnclosingRegion(entry_address);

    if(!entry_region) {
      startup_printf("find_main: no region found at entry 0x%lx\n", entry_address);
      return main_addr_t{}.invalid();
    }

    pa::SymtabCodeSource scs(linkedFile);

    std::set<pa::CodeRegion *> code_regions;
    scs.findRegions(entry_address, code_regions);

    if(code_regions.empty()) {
      startup_printf("find_main: no code region contains 0x%lx\n", entry_address);
      return main_addr_t{}.invalid();
    }

    // We should only get one region
    if(code_regions.size() > 1UL) {
      startup_printf("find_main: found %lu possibly-overlapping code regions for 0x%lx\n",
                     code_regions.size(), entry_address);
      return main_addr_t{}.invalid();
    }

    auto code_object = [&scs]() -> std::unique_ptr<pa::CodeObject> {
      // To save time, delay the parsing
      pa::CFGFactory *f{};
      pa::ParseCallback *cb{};
      constexpr bool defensive_mode = false;
      constexpr bool delay_parse = true;
      return std::unique_ptr<pa::CodeObject>(
          new pa::CodeObject(&scs, f, cb, defensive_mode, delay_parse));
    }();

    pa::Function *entry_point = [&]() {
      constexpr bool recursive = true;
      auto *reg = *(code_regions.begin());
      code_object->parse(reg, entry_address, recursive);
      return code_object->findFuncByEntry(reg, entry_address);
    }();

    if(!entry_point) {
      startup_printf("find_main: couldn't find function at entry 0x%lx\n", entry_address);
      return main_addr_t{}.invalid();
    }

    startup_printf("find_main: found '%s' at entry 0x%lx\n", entry_point->name().c_str(),
                   entry_address);

    // Try architecture-specific searches
    auto main = [&]() {
      switch(linkedFile->getArchitecture()) {
        case Dyninst::Arch_ppc32:
        case Dyninst::Arch_ppc64: {
          auto *reg = *code_regions.begin();
          pa::Block *entryBlock = code_object->findBlockByEntry(reg, entry_address);
          return DyninstAPI::ppc::find_main(linkedFile, entryBlock, scs, entry_point);
        }

        case Dyninst::Arch_x86:
        case Dyninst::Arch_x86_64: {
          return DyninstAPI::x86::find_main(entry_point);
        }

        case Arch_aarch64: {
          return DyninstAPI::aarch64::find_main();
        }

        case Arch_riscv64: {
          return DyninstAPI::riscv64::find_main();
        }

        case Arch_none:
        case Arch_aarch32:
        case Arch_cuda:
        case Arch_amdgpu_gfx908:
        case Arch_amdgpu_gfx90a:
        case Arch_amdgpu_gfx940:
        case Arch_amdgpu_gfx950:
        case Arch_intelGen9:
          break;
      }

      return main_addr_t{}.invalid();
    }();

    if(!main.is_valid) {
      startup_printf("find_main: unable to find valid entry for 'main'\n");
    } else {
      startup_printf("find_main: found 'main' at 0x%lx\n", main.addr);
    }

    return main;
  }

  static st::Symbol* make_pe_symbol(st::Symtab *st, st::Region *reg, std::string name, Dyninst::Address addr, unsigned size) {
    startup_printf("Adding symbol '%s' at 0x%lx\n", name.c_str(), addr);
    // clang-format off
    return new st::Symbol(
      std::move(name),
      st::Symbol::ST_FUNCTION,
      st::Symbol::SL_GLOBAL,
      st::Symbol::SV_DEFAULT,
      addr,
      st->getDefaultModule(),
      reg,
      size
    );
    // clang-format on
  }

  static st::Symbol* make_elf_symbol(st::Symtab *st, st::Region *reg, std::string name, Dyninst::Address addr) {
    startup_printf("Adding symbol '%s' at 0x%lx\n", name.c_str(), addr);
    // clang-format off
    return new st::Symbol(
      name,
      st::Symbol::ST_FUNCTION,
      st::Symbol::SL_LOCAL,
      st::Symbol::SV_INTERNAL,
      addr,
      st->getDefaultModule(),
      reg,
      0U
    );
    // clang-format on
  }

  static void add_main_symbol(st::Symtab *st, Dyninst::Address addr) {
    const auto entry_address = static_cast<Dyninst::Address>(st->getEntryOffset());
    st::Region *entry_region = st->findEnclosingRegion(entry_address);

    if(st->getFileFormat() == st::FileFormat::PE) {
      std::vector<st::Symbol *> syms{};
      auto const has_start = st->findSymbol(syms, "start", st::Symbol::ST_UNKNOWN, SymtabAPI::mangledName);
      if(!has_start) {
        // use 'start' for mainCRTStartup.
        st->addSymbol(make_pe_symbol(st, entry_region, "start", entry_address, UINT_MAX));
      } else {
        // add entry point as main given that nothing else was found
        st->addSymbol(make_pe_symbol(st, entry_region, "main", entry_address, 0U));
      }
      return;
    }

    if(addr != Dyninst::ADDRESS_INVALID) {
      st->addSymbol(make_elf_symbol(st, entry_region, "main", addr));
    }
  }

  void add_missing_symbols(st::Symtab *linkedFile, main_addr_t main) {

    // Don't add a 'main' symbol if there already was one
    if(!main.is_from_symbol) {
      add_main_symbol(linkedFile, main.addr);
    }

    const auto entry_address = static_cast<Dyninst::Address>(linkedFile->getEntryOffset());
    st::Region *entry_region = linkedFile->findEnclosingRegion(entry_address);

    /*  Add helper symbols
     *
     *    ABIs don't require these, but Dyninst uses them during rewriting
     *    (see BinaryEdit::makeInitAndFiniIfNeeded()).
     */
    {
      std::vector<st::Function *> funcs{};
      if(!linkedFile->findFunctionsByName(funcs, "_start")) {
        linkedFile->addSymbol(make_elf_symbol(linkedFile, entry_region, "_start", entry_region->getMemOffset()));
      }

      // Add _init/_fini that points to their section
      for(std::string sec : {"init", "fini"}) {
        if(!linkedFile->findFunctionsByName(funcs, "_" + sec)) {
          st::Region *reg{};
          if(linkedFile->findRegion(reg, "." + sec)) {
            linkedFile->addSymbol(make_elf_symbol(linkedFile, reg, "_" + sec, reg->getMemOffset()));
          }
        }
      }
    }
  }

}}
