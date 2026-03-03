#include "CodeObject.h"
#include "debug.h"
#include "dyntypes.h"
#include "image.h"
#include "InstructionDecoder.h"
#include "parse-cfg.h"
#include "parse_block.h"
#include "Register.h"
#include "registers/MachRegister.h"

namespace parse = Dyninst::ParseAPI;
namespace insn = Dyninst::InstructionAPI;

/*
 * For CFGFactory::mksink only
 */
parse_block::parse_block(parse::CodeObject *obj, parse::CodeRegion *reg, Dyninst::Address addr)
    : Block(obj, reg, addr){}

parse_block::parse_block(parse_func *func, parse::CodeRegion *reg, Dyninst::Address firstOffset)
    : Block(func->obj(), reg, firstOffset, func) {
  // basic block IDs are unique within images.
  blockNumber_ = func->img()->getNextBlockID();
}

void *parse_block::getPtrToInstruction(Dyninst::Address addr) const {
  if(addr < start()) {
    return NULL;
  }
  if(addr >= end()) {
    return NULL;
  }
  // XXX all potential parent functions have the same image
  return region()->getPtrToInstruction(addr);
}

bool parse_block::isEntryBlock(parse_func *f) const {
  return f->entryBlock() == this;
}

/*
 * True if the block has a return edge, or a call that does
 * not return (i.e., a tail call or non-returning call)
 */
bool parse_block::isExitBlock() {
  const Block::edgelist &trgs = targets();
  if(trgs.empty()) {
    return false;
  }

  parse::Edge *e = *trgs.begin();
  if(e->type() == parse::RET) {
    return true;
  }

  if(!e->interproc()) {
    return false;
  }

  if(e->type() == parse::CALL && trgs.size() > 1) {
    // there's a CALL edge and at least one other edge,
    // it's an exit block if there is no CALL_FT edge
    for(Block::edgelist::const_iterator eit = ++(trgs.begin()); eit != trgs.end(); eit++) {
      if((*eit)->type() == parse::CALL_FT && !(*eit)->sinkEdge()) {
        return false;
      }
    }
  }
  return true;
}

bool parse_block::isCallBlock() {
  const Block::edgelist &trgs = targets();
  if(!trgs.empty()) {
    for(Block::edgelist::const_iterator eit = trgs.begin(); eit != trgs.end(); eit++) {
      if((*eit)->type() == parse::CALL) {
        return true;
      }
    }
  }
  return false;
}

bool parse_block::isIndirectTailCallBlock() {
  const Block::edgelist &trgs = targets();
  if(!trgs.empty()) {
    for(Block::edgelist::const_iterator eit = trgs.begin(); eit != trgs.end(); eit++) {
      if((*eit)->type() == parse::INDIRECT && (*eit)->interproc()) {
        return true;
      }
    }
  }
  return false;
}

parse_func *parse_block::getEntryFunc() const {
  parse_func *ret = static_cast<parse_func *>(obj()->findFuncByEntry(region(), start()));

  // sanity check
  if(ret && ret->entryBlock() != this) {
    parsing_printf(
        "[%s:%d] anomaly: block [%lx,%lx) is not entry for "
        "func at %lx\n",
        FILE__, __LINE__, start(), end(), ret->addr());
  }
  return ret;
}

parse_block *parse_func::entryBlock() {
  if(!parsed()) {
    image_->analyzeIfNeeded();
  }
  return static_cast<parse_block *>(entry());
}

void parse_block::setUnresolvedCF(bool newVal) {
  unresolvedCF_ = newVal;
}

std::pair<bool, Address> parse_block::callTarget() {
  using namespace InstructionAPI;
  Offset off = lastInsnAddr();
  const unsigned char *ptr = (const unsigned char *)getPtrToInstruction(off);
  if(ptr == NULL) {
    return std::make_pair(false, 0);
  }
  insn::InstructionDecoder d(ptr, end() - lastInsnAddr(), obj()->cs()->getArch());
  insn::Instruction insn = d.decode();

  // Bind PC to that insn
  // We should build a free function to do this...

  insn::Expression::Ptr cft = insn.getControlFlowTarget();
  if(cft) {
    insn::Expression::Ptr pc(new insn::RegisterAST(MachRegister::getPC(obj()->cs()->getArch())));
    cft->bind(pc.get(), Result(u64, lastInsnAddr()));
    Result res = cft->eval();
    if(!res.defined) {
      return std::make_pair(false, 0);
    }

    return std::make_pair(true, res.convert<Address>());
  }
  return std::make_pair(false, 0);
}
