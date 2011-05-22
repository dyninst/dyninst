/* Public Interface */

#include "Point.h"
#include "PatchMgr.h"
#include "PatchObject.h"
#include "PatchCFG.h"

using Dyninst::PatchAPI::Instance;
using Dyninst::PatchAPI::InstancePtr;
using Dyninst::PatchAPI::Point;
using Dyninst::PatchAPI::PointPtr;
using Dyninst::PatchAPI::PatchFunction;
using Dyninst::PatchAPI::PointMaker;
using Dyninst::ParseAPI::CodeObject;
using Dyninst::ParseAPI::CodeSource;
using Dyninst::ParseAPI::CodeRegion;
using Dyninst::InstructionAPI::InstructionDecoder;

InstancePtr Instance::create(PointPtr point, SnippetPtr snippet,
    SnippetType type, SnippetState state) {
  InstancePtr ret = InstancePtr(new Instance(point, snippet));
  if (!ret) return InstancePtr();
  ret->state_ = state;
  ret->type_ = type;
  return ret;
}

bool Instance::destroy() {
  if (point_) {
    bool ret = point_->remove(shared_from_this());
    return ret;
  }
  return false;
}

/* If the Point is PreCall or PostCall */
PatchFunction* Point::getCallee() {
  if (type() != PreCall && type() != PostCall) return NULL;
  PatchBlock* b = (*(inst_blks_.begin()));
  PatchBlock::edgelist::iterator it = b->targets().begin();
  for (; it != b->targets().end(); ++it) {
    if ((*it)->type() == ParseAPI::CALL) {
      PatchBlock* trg = (*it)->target();
      return trg->function();
    }
  }
  return NULL;
}

/* Associate this point with the block(s) and function(s)
   that contain it */
void Point::initCodeStructure(Address /*addr*/) {
#if 0
  // walk through all code objects
  for (AddrSpace::CoObjMap::iterator ci = mgr_->as()->getCoobjMap().begin();
       ci != mgr_->as()->getCoobjMap().end(); ci++) {
    PatchObject* mod = (*ci).second;
    CodeObject* co = mod->co();
    CodeSource* cs = mod->cs();
    Address relative_addr = addr_ - mod->codeBase();
    vector<CodeRegion*> regions = cs->regions();
    for (vector<CodeRegion*>::iterator ri = regions.begin(); ri != regions.end(); ri++) {
      std::set<ParseAPI::Function*> parseapi_funcs;
      co->findFuncs(*ri, relative_addr, parseapi_funcs);
      std::set<ParseAPI::Block*> parseapi_blks;
      co->findBlocks(*ri, relative_addr, parseapi_blks);

      for (std::set<ParseAPI::Function*>::iterator fi = parseapi_funcs.begin();
           fi != parseapi_funcs.end(); fi++) {
        PatchFunction* func = mod->getFunc(*fi);
        inst_funcs_.insert(func);
        for (std::set<ParseAPI::Block*>::iterator bi = parseapi_blks.begin(); bi != parseapi_blks.end(); bi++) {
          PatchBlock* blk = func->getBlock(*bi);

          inst_blks_.insert(blk);
        }
      }
    }
    if (inst_blks_.size() > 0) {
      co_ = co;
      cs_ = cs;
      obj_ = mod;
      InstructionDecoder d(cs_->getPtrToInstruction(relative_addr),
                          cs_->length(),
                          cs_->getArch());
      // Get the instruction that contain this point
      instruction_ = d.decode();
      break;
    }
  }
#endif
}

/* for single instruction */
Point::Point(Address addr, Point::Type type, PatchMgrPtr mgr, Address*)
  :addr_(addr), type_(type), mgr_(mgr), blk_(NULL), edge_(NULL), func_(NULL) {
  initCodeStructure(addr);
}

/* for a block */
Point::Point(Address addr, Type type, PatchMgrPtr mgr, PatchBlock* blk)
  : addr_(addr), type_(type), mgr_(mgr), blk_(blk), edge_(NULL), func_(NULL) {
  initCodeStructure(addr);
}

/* for an edge */
Point::Point(Address addr, Type type, PatchMgrPtr mgr, PatchEdge* edge)
  : addr_(addr), type_(type), mgr_(mgr), blk_(NULL), edge_(edge), func_(NULL) {
  initCodeStructure(addr);
}

/* for a function */
Point::Point(Address addr, Type type, PatchMgrPtr mgr,
             PatchFunction* func) : addr_(addr), type_(type), mgr_(mgr),
                                    blk_(NULL), edge_(NULL), func_(func) {
  initCodeStructure(addr);
}

/* old_instance, old_instance, <---new_instance */
InstancePtr Point::push_back(SnippetPtr snippet) {
  InstancePtr instance = Instance::create(shared_from_this(), snippet);
  if (!instance) return instance;
  instanceList_.push_back(instance);
  mgr_->insertion_set_.insert(instance);
  mgr_->current_instances_.insert(instance);
  return instance;
}

/* new_instance--->, old_instance, old_instance */
InstancePtr Point::push_front(SnippetPtr snippet) {
  InstancePtr instance = Instance::create(shared_from_this(), snippet);
  if (!instance) return instance;
  instanceList_.push_front(instance);
  mgr_->insertion_set_.insert(instance);
  mgr_->current_instances_.insert(instance);
  return instance;
}

/* Test whether the type contains a specific type. */
bool Point::TestType(Point::Type types, Point::Type trg) {
  if (types & trg) return true;
  return false;
}

/* Add a specific type to a set of types */
void Point::AddType(Point::Type& types, Point::Type trg) {
  int trg_int = static_cast<int>(trg);
  int type_int = static_cast<int>(types);
  type_int |= trg_int;
  types = (Point::Type)type_int;
}

/* Remove a specific type from a set of types */
void Point::RemoveType(Point::Type& types, Point::Type trg) {
  int trg_int = static_cast<int>(trg);
  int type_int = static_cast<int>(types);
  type_int &= (~trg_int);
  types = (Point::Type)type_int;
}

bool Point::remove(InstancePtr instance) {
  if (instance == InstancePtr()) return false;
  InstanceList::iterator it = std::find(instanceList_.begin(),
                                 instanceList_.end(), instance);
  if (it != instanceList_.end()) {
    instanceList_.erase(it);
    mgr_->insertion_set_.erase(instance);
    mgr_->current_instances_.erase(instance);
    if (instance->state() == INSERTED) {
      mgr_->deletion_set_.insert(instance);
    }
    instance->set_state(PENDING);
    return true;
  }
  return false;
}

size_t Point::size() {
  return instanceList_.size();
}

void Point::clear() {
  while (size() > 0) {
    InstancePtr i = instanceList_.back();
    i->destroy();
  }
}

/* 1, Clear all snippet instances
   2, Detach from PatchMgr object */
bool Point::destroy() {
  clear();
  PatchMgr::TypePtMap type_pt_map;
  switch (type_) {
    case PreInsn:
    case PostInsn:
    case InsnTaken:
    case PreCall:
    case PostCall:
    {
      type_pt_map = mgr_->addr_type_pt_map_[addr_];
      break;
    }
    case BlockEntry:
    case BlockExit:
    case BlockDuring:
    {
      type_pt_map = mgr_->blk_type_pt_map_[blk_];
      break;
    }
    case FuncEntry:
    case FuncExit:
    case FuncDuring:
    {
      type_pt_map = mgr_->func_type_pt_map_[func_];
      break;
    }
    case EdgeDuring:
    {
      type_pt_map = mgr_->edge_type_pt_map_[edge_];
      break;
    }
    case LoopStart:
    case LoopEnd:
    case LoopIterStart:
    case LoopIterEnd:
    {
      return false;
    }
    case InsnTypes:
    case LoopTypes:
    case BlockTypes:
    case CallTypes:
    case FuncTypes:
    case OtherPoint:
    case None:
    {
      return false;
    }
  }
  PointSet& points = type_pt_map[type_];
  points.erase(shared_from_this());
  return true;
}
