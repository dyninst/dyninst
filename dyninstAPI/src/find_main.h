#ifndef DYNINST_DYNINSTAPI_FINDMAIN_H
#define DYNINST_DYNINSTAPI_FINDMAIN_H

#include "CodeSource.h"
#include "Symbol.h"
#include "SymEval.h"
#include "Symtab.h"

namespace Dyninst { namespace DyninstAPI {

  namespace st = Dyninst::SymtabAPI;
  namespace pa = Dyninst::ParseAPI;

  struct main_addr_t final {
    constexpr main_addr_t(Dyninst::Address a) : addr{a} {}
    constexpr main_addr_t() = default;

    Dyninst::Address addr{Dyninst::ADDRESS_INVALID};

    // true when the address was found from an existing symbol (i.e., no binary analysis needed).
    bool is_from_symbol{false};

    // The binary has to be executable, so assume it is by default
    bool is_executable{true};

    // Users must explicitly mark it invalid
    bool is_valid{true};

    main_addr_t not_executable() && {
      is_executable = false;
      return *this;
    }

    // This is only used to indicate that there is a fundamental error in parsing the startup
    // sequence before attempting any binary analysis. Architecture-specific implementations of
    // `find_main` should return `unknown()` even if they encounter parsing errors.
    main_addr_t invalid() && {
      is_valid = false;
      return *this;
    }

    main_addr_t from_symbol(Dyninst::Address a) && {
      addr = a;
      is_from_symbol = true;
      return *this;
    }

    // A valid executable, but couldn't find an address
    main_addr_t unknown() && {
      return *this;
    }
  };

  void add_missing_symbols(st::Symtab *, main_addr_t);

  main_addr_t find_main(st::Symtab *);

  namespace aarch64 {
    main_addr_t find_main();
  }

  namespace ppc {
    main_addr_t find_main(st::Symtab *, pa::Block *, pa::SymtabCodeSource const &, pa::Function *);
  }

  namespace riscv64 {
    main_addr_t find_main();
  }

  namespace x86 {
    main_addr_t find_main(pa::Function *);
  }

}}

#endif
