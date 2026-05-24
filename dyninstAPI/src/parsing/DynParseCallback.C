#include "DynCommon.h"
#include "DynParseCallback.h"
#include "debug.h"
#include "image.h"
#include "mapped_object.h"
#include "parse_block.h"
#include "parse_func.h"

namespace Dyninst { namespace DyninstAPI {

  void DynParseCallback::abruptEnd_cf(Address, ParseAPI::Block *b, default_details *) {
    static_cast<parse_block *>(b)->setAbruptEnd(true);
  }

  void DynParseCallback::destroy_cb(ParseAPI::Block *b) {
    _img->destroy(b);
  }

  void DynParseCallback::destroy_cb(ParseAPI::Edge *e) {
    _img->destroy(e);
  }

  void DynParseCallback::destroy_cb(ParseAPI::Function *f) {
    _img->destroy(f);
  }

  void DynParseCallback::foundWeirdInsns(ParseAPI::Function *func) {
    static_cast<parse_func *>(func)->setHasWeirdInsns(true);
  }

  bool DynParseCallback::hasWeirdInsns(const ParseAPI::Function *func) const {
    return static_cast<parse_func *>(const_cast<ParseAPI::Function *>(func))
        ->hasWeirdInsns();
  }

  void DynParseCallback::interproc_cf(ParseAPI::Function *f, ParseAPI::Block *b,
                                      Address /*addr*/, interproc_details *det) {
    (void)f; // compiler warning
    if (det->type == ParseCallback::interproc_details::unresolved) {
      static_cast<parse_block *>(b)->setUnresolvedCF(true);
    }
  }

  void DynParseCallback::newfunction_retstatus(ParseAPI::Function *func) {
    dynamic_cast<parse_func *>(func)->setinit_retstatus(func->retstatus());
  }

  void DynParseCallback::overlapping_blocks(ParseAPI::Block *b1, ParseAPI::Block *b2) {
    parsing_printf("[%s:%d] blocks [%lx,%lx) and [%lx,%lx) overlap"
                   "inconsistently\n",
                   FILE__, __LINE__, b1->start(), b1->end(), b2->start(), b2->end());
    static_cast<parse_block *>(b1)->markAsNeedingRelocation();
    static_cast<parse_block *>(b2)->markAsNeedingRelocation();
  }

  void DynParseCallback::patch_nop_jump(Address addr) {
    Architecture arch = _img->codeObject()->cs()->getArch();
    assert(Arch_x86 == arch || Arch_x86_64 == arch);

    unsigned char *ptr =
        (unsigned char *)_img->codeObject()->cs()->getPtrToInstruction(addr);
    ptr[0] = 0x90;
  }

  void DynParseCallback::remove_block_cb(ParseAPI::Function *, ParseAPI::Block *) {
    // we currently do all necessary cleanup during destroy
    // cerr << "Warning: block removal callback unimplemented" << endl;
  }

  void DynParseCallback::remove_edge_cb(ParseAPI::Block *, ParseAPI::Edge *,
                                        edge_type_t) {
    // cerr << "Warning: edge removal callback unimplemented" << endl;
  }

  void DynParseCallback::split_block_cb(ParseAPI::Block *first, ParseAPI::Block *second) {
    if (SCAST_PB(first)->abruptEnd()) {
      SCAST_PB(first)->setAbruptEnd(false);
      SCAST_PB(second)->setAbruptEnd(true);
    }
    if (SCAST_PB(first)->unresolvedCF()) {
      SCAST_PB(first)->setUnresolvedCF(false);
      SCAST_PB(second)->setUnresolvedCF(true);
    }
    if (SCAST_PB(first)->needsRelocation()) {
      SCAST_PB(second)->markAsNeedingRelocation();
    }
    // parse_block::canBeRelocated_ doesn't need to be set, it's only ever
    //  true for the sink block, which is never split
  }

  // calls function that updates bytes if needed
  bool DynParseCallback::updateCodeBytes(Address target_) {
    assert(BPatch_normalMode != _img->hybridMode());
    auto targ = target_ + _img->desc().code();
    auto *obj = reinterpret_cast<mapped_object *>(_img->cb_arg0());
    return obj->updateCodeBytesIfNeeded(targ);
  }

}}
