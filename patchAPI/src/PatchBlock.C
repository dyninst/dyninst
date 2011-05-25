/* Public Interface */

#include "common.h"
#include "PatchCFG.h"

using namespace Dyninst;
using namespace PatchAPI;
using namespace InstructionAPI;

PatchBlock *PatchBlock::create(ParseAPI::Block *ib, PatchFunction *f) {
  return f->object()->getBlock(ib);
}

PatchBlock::PatchBlock(ParseAPI::Block *blk, PatchObject *obj)
  : block_(blk),   obj_(obj) {

  ParseAPI::CodeObject::funclist& all = obj->co()->funcs();
  for (ParseAPI::CodeObject::funclist::iterator fit = all.begin();
       fit != all.end(); ++fit) {
    if ((*fit)->contains(blk)) {
      function_ = obj->getFunc(*fit);
      break;
    }
  }
}

PatchBlock::PatchBlock(const PatchBlock *parent, PatchObject *child)
  : block_(parent->block_), obj_(child) {
  ParseAPI::CodeObject::funclist& all = child->co()->funcs();
  for (ParseAPI::CodeObject::funclist::iterator fit = all.begin();
       fit != all.end(); ++fit) {
    if ((*fit)->contains(block_)) {
      function_ = child->getFunc(*fit);
      break;
    }
  }
}

void PatchBlock::getInsns(Insns &insns) const {
  // Pass through to ParseAPI. They don't have a native interface, so add one.
  Offset off = block_->start();
  const unsigned char *ptr =
    (const unsigned char *)block_->region()->getPtrToInstruction(off);
  if (ptr == NULL) return;
  InstructionDecoder d(ptr, size(), block_->obj()->cs()->getArch());
  while (off < block_->end()) {
    Instruction::Ptr insn = d.decode();
    insns[off + obj_->codeBase()] = insn;
    off += insn->size();
  }
}

void PatchBlock::createInterproceduralEdges(ParseAPI::Edge *iedge,
                                            Direction dir,
                                            std::vector<PatchEdge *> &edges) {
  // Let pT be the target block in the parseAPI
  // Let {f_1, ..., f_n} be the functions T is in
  // We create blocks t_i for each function f_i
  ParseAPI::Block *iblk = (dir == backwards) ? iedge->src() : iedge->trg();
  if (!iblk) {
    assert(dir == forwards); // Can't have sink in-edges
    edges.push_back(object()->getEdge(iedge, this, NULL));
    return;
  }
  std::vector<ParseAPI::Function *> ifuncs;
  iblk->getFuncs(ifuncs);
  for (unsigned i = 0; i < ifuncs.size(); ++i) {
    PatchBlock *pblock = object()->getBlock(iblk);
    assert(pblock);
    PatchEdge *newEdge = NULL;
    if (dir == forwards)
      newEdge = object()->getEdge(iedge, this, pblock);
    else
      newEdge = object()->getEdge(iedge, pblock, this);

    edges.push_back(newEdge);
  }
  return;
}


PatchBlock::edgelist &PatchBlock::getSources() {
  if (srclist_.empty()) {
     for (ParseAPI::Block::edgelist::iterator iter = block_->sources().begin();
           iter != block_->sources().end(); ++iter) {
         PatchEdge *newEdge = obj_->getEdge(*iter, NULL, this);
         srclist_.push_back(newEdge);
      }
  }
  return srclist_;
}

PatchBlock::edgelist &PatchBlock::getTargets() {
  if (trglist_.empty()) {
    for (ParseAPI::Block::edgelist::iterator iter = block_->targets().begin();
         iter != block_->targets().end(); ++iter) {
      PatchEdge *newEdge = obj_->getEdge(*iter, this, NULL);
      trglist_.push_back(newEdge);
    }
  }
  return trglist_;
}

void PatchBlock::removeSourceEdge(PatchEdge *e) {
  // This is called as part of teardown
  // of another Block to remove its edges from our
  // vectors.
  for (std::vector<PatchEdge *>::iterator iter = srclist_.begin();
       iter != srclist_.end(); ++iter) {
    if ((*iter) == e) {
      srclist_.erase(iter);
      return;
    }
  }
}

void PatchBlock::removeTargetEdge(PatchEdge *e) {
  // This is called as part of teardown
  // of another Block to remove its edges from our
  // vectors.
  for (std::vector<PatchEdge *>::iterator iter = trglist_.begin();
       iter != trglist_.end(); ++iter) {
    if ((*iter) == e) {
      trglist_.erase(iter);
      return;
    }
  }
}


bool PatchBlock::isShared() {
  return containingFuncs() > 1;
}

PatchBlock::~PatchBlock() {
}

Address PatchBlock::start() const {
  return object()->codeBase() + block_->start();
}

Address PatchBlock::end() const {
  return object()->codeBase() + block_->end();
}

Address PatchBlock::last() const {
  return object()->codeBase() + block_->lastInsnAddr();
}

Address PatchBlock::size() const {
  return block_->size();
}

int PatchBlock::containingFuncs() const {
  return block_->containingFuncs();
}

bool PatchBlock::containsCall() {
  ParseAPI::Block::edgelist & out_edges = block_->targets();
  ParseAPI::Block::edgelist::iterator eit = out_edges.begin();
  for( ; eit != out_edges.end(); ++eit) {
    if ( ParseAPI::CALL == (*eit)->type() ) {
      return true;
    }
  }
  return false;
}

bool PatchBlock::containsDynamicCall() {
  ParseAPI::Block::edgelist & out_edges = block_->targets();
  ParseAPI::Block::edgelist::iterator eit = out_edges.begin();
   for( ; eit != out_edges.end(); ++eit) {
     if ( ParseAPI::CALL == (*eit)->type() && ((*eit)->sinkEdge())) {
         return true;
      }
   }
   return false;
}

std::string PatchBlock::disassemble() const {
    stringstream ret;
    Insns instances;
    getInsns(instances);
    for (Insns::iterator iter = instances.begin();
         iter != instances.end(); ++iter) {
       ret << "\t" << hex << iter->first << ": " << iter->second->format() << dec << endl;
    }
    return ret.str();
}

InstructionAPI::Instruction::Ptr PatchBlock::getInsn(Address a) const {
   Insns insns;
   getInsns(insns);
   return insns[a];
}

std::string PatchBlock::format() const {
    stringstream ret;
    ret << "BB["
        << hex << start()
        << ","
        << end() << dec
        << "]" << endl;
    return ret.str();
}
