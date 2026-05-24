#include "DynCFGFactory.h"
#include "Function.h"
#include "image.h"
#include "parse_block.h"
#include "parse_func.h"
#include "relocationEntry.h"

#include <limits>

// clang-format off
#if defined(VERBOSE_CFG_FACTORY)
#define record_func_alloc(x) do { _record_func_alloc(x); } while(0)
#define record_edge_alloc(x,s) do { _record_edge_alloc(x,s); } while(0)
#define record_block_alloc(s) do { _record_block_alloc(s); } while(0)
#else
#define record_func_alloc(x) do { } while(0)
#define record_edge_alloc(x,s) do { } while(0)
#define record_block_alloc(s) do { } while(0)
#endif
// clang-format on

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

  DynCFGFactory::DynCFGFactory(image *im)
      : _img(im), _func_allocs(ParseAPI::_funcsource_end_),
        _edge_allocs(ParseAPI::_edgetype_end_), _block_allocs(0), _sink_block_allocs(0) {
  }

  void DynCFGFactory::dump_stats() {
    fprintf(stderr, "===DynCFGFactory for image %p===\n", (void *)_img);
    fprintf(stderr, "   Functions:\n");
    fprintf(stderr, "   %-12s src\n", "cnt");
    for (int i = 0; i < ParseAPI::_funcsource_end_; ++i) {
      fprintf(stderr, "   %-12d %3d\n", _func_allocs[i], i);
    }
    fprintf(stderr, "   Edges:\n");
    fprintf(stderr, "   %-12s type\n", "cnt");
    for (int i = 0; i < ParseAPI::_edgetype_end_; ++i) {
      fprintf(stderr, "   %-12d %4d\n", _edge_allocs[i], i);
    }
    fprintf(stderr, "   Blocks:\n");
    fprintf(stderr, "   %-12d total\n", _block_allocs);
    fprintf(stderr, "   %-12d sink\n", _sink_block_allocs);
  }

  ParseAPI::Block *DynCFGFactory::mkblock(ParseAPI::Function *f, ParseAPI::CodeRegion *r,
                                          Address addr) {
    parse_block *ret;

    record_block_alloc(false);

    ret = new parse_block((parse_func *)f, r, addr);

    if (_img->trackNewBlocks_) {
      _img->newBlocks_.push_back(ret);
    }
    return ret;
  }

  ParseAPI::Edge *DynCFGFactory::mkedge(ParseAPI::Block *src, ParseAPI::Block *trg,
                                        EdgeTypeEnum type) {
    record_edge_alloc(type, false); // FIXME can't tell if it's a sink

    return new ParseAPI::Edge(src, trg, type);
  }

  ParseAPI::Function *DynCFGFactory::mkfunc(Address addr, FuncSource src,
                                            std::string name, ParseAPI::CodeObject *obj,
                                            ParseAPI::CodeRegion *reg,
                                            InstructionSource *isrc) {
    _mtx.lock();
    parse_func *ret;
    SymtabAPI::Symtab *st;
    SymtabAPI::Function *stf = NULL;
    pdmodule *pdmod;
    record_func_alloc(src);

    st = _img->getObject();
    auto found = obj->cs()->linkage().find(addr);
    // PLT stub
    if (found != obj->cs()->linkage().end()) {
      name = found->second;
      pdmod = _img->getOrCreateModule(st->getDefaultModule());
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
        _mtx.unlock();
        return ret;
      }
    }
    if (!st->findFuncByEntryOffset(stf, addr)) {
      pdmod = _img->getOrCreateModule(st->getDefaultModule());
      stf = st->createFunction(name, addr, 0, pdmod->mod());
    } else {
      pdmod = _img->getOrCreateModule(stf->getModule());
    }
    assert(stf);

    ret = new parse_func(stf, pdmod, _img, obj, reg, isrc, src);

    _mtx.unlock();
    return ret;
  }

  ParseAPI::Block *DynCFGFactory::mksink(ParseAPI::CodeObject *obj,
                                         ParseAPI::CodeRegion *r) {
    parse_block *ret;

    record_block_alloc(true);

    ret = new parse_block(obj, r, std::numeric_limits<Address>::max());
    return ret;
  }

}}
