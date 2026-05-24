#include "DynCFGFactory.h"
#include "Function.h"
#include "image.h"
#include "parse_block.h"
#include "parse_func.h"
#include "relocationEntry.h"

#include <boost/thread/lock_guard.hpp>
#include <limits>

namespace {

  class PLTFunction : public Dyninst::SymtabAPI::Function {
    Dyninst::SymtabAPI::relocationEntry r;

  public:
    explicit PLTFunction(Dyninst::SymtabAPI::relocationEntry re)
        : Dyninst::SymtabAPI::Function(re.getDynSym()), r{re} {
    }

    std::string getName() const override {
      return r.name();
    }

    Offset getOffset() const override {
      return r.target_addr();
    }

    unsigned int getSize() const override {
      return 0;
    }

    SymtabAPI::Module *getModule() const override {
      return nullptr;
    }
  };

}

namespace Dyninst { namespace DyninstAPI {

  DynCFGFactory::DynCFGFactory(image *im) : _img(im) {
  }

  ParseAPI::Block *DynCFGFactory::mkblock(ParseAPI::Function *f, ParseAPI::CodeRegion *r,
                                          Address addr) {
    auto *block = new parse_block(static_cast<parse_func *>(f), r, addr);

    if (_img->trackNewBlocks_) {
      _img->newBlocks_.push_back(block);
    }
    return block;
  }

  ParseAPI::Edge *DynCFGFactory::mkedge(ParseAPI::Block *src, ParseAPI::Block *trg,
                                        EdgeTypeEnum type) {
    return new ParseAPI::Edge(src, trg, type);
  }

  ParseAPI::Function *DynCFGFactory::mkfunc(Address addr, FuncSource src,
                                            std::string name, ParseAPI::CodeObject *obj,
                                            ParseAPI::CodeRegion *reg,
                                            InstructionSource *isrc) {

    boost::lock_guard<decltype(_mtx)> _lock{_mtx};

    parse_func *ret;
    SymtabAPI::Symtab *st = _img->getObject();
    SymtabAPI::Function *stf{};
    pdmodule *pdmod = _img->getOrCreateModule(st->getDefaultModule());

    auto found = obj->cs()->linkage().find(addr);
    // PLT stub
    if (found != obj->cs()->linkage().end()) {
      name = found->second;
      std::vector<SymtabAPI::relocationEntry> relocs;
      st->getFuncBindingTable(relocs);
      for (auto i = relocs.begin(); i != relocs.end(); i++) {
        if (i->target_addr() == found->first) {
          stf = new PLTFunction(*i);
          break;
        }
      }
      if (stf && stf->getFirstSymbol()) {
        ret = parse_func::plt_func(stf, pdmod, _img, obj, reg, isrc, src);
        // PLT stubs are typically are undefined symbols in the binary,
        // so there is no corresponding SymtabAPI::Function at Symtab level.
        // PLTFunction is a subclass of SymtabAPI::Function to represent PLT stubs.
        // However, since there is no easy way to add a PLTFunction back to the
        // Symtab object, we need to add PLTFunction to a data structure for
        // future lookup.
        _img->insertPLTParseFuncMap(stf->getName(), ret);
        return ret;
      }
    }
    if (!st->findFuncByEntryOffset(stf, addr)) {
      stf = st->createFunction(name, addr, 0, pdmod->mod());
    } else {
      pdmod = _img->getOrCreateModule(stf->getModule());
    }
    assert(stf);

    ret = new parse_func(stf, pdmod, _img, obj, reg, isrc, src);

    return ret;
  }

  ParseAPI::Block *DynCFGFactory::mksink(ParseAPI::CodeObject *obj,
                                         ParseAPI::CodeRegion *r) {
    parse_block *ret;

    ret = new parse_block(obj, r, std::numeric_limits<Address>::max());
    return ret;
  }

}}
