/* Public Interface */

#include "Point.h"
#include "PatchMgr.h"
#include "PatchObject.h"
#include "PatchCFG.h"

using namespace Dyninst;
using namespace Dyninst::PatchAPI;

using Dyninst::PatchAPI::Instance;
using Dyninst::PatchAPI::InstancePtr;
using Dyninst::PatchAPI::Point;
using Dyninst::PatchAPI::PatchFunction;
using Dyninst::PatchAPI::PointMaker;
using Dyninst::ParseAPI::CodeObject;
using Dyninst::ParseAPI::CodeSource;
using Dyninst::ParseAPI::CodeRegion;
using Dyninst::InstructionAPI::InstructionDecoder;
using Dyninst::PatchAPI::SnippetPtr;
using Dyninst::PatchAPI::SnippetType;
using Dyninst::PatchAPI::PatchMgrPtr;
using Dyninst::PatchAPI::SnippetState;
using Dyninst::PatchAPI::PatchBlock;
using Dyninst::PatchAPI::PatchEdge;

InstancePtr
Instance::create(Point* point, SnippetPtr snippet,
    SnippetType type, SnippetState state) {
  InstancePtr ret = InstancePtr(new Instance(point, snippet));
  if (!ret) return InstancePtr();
  ret->state_ = state;
  ret->type_ = type;
  return ret;
}

bool
Instance::destroy() {
  if (point_) {
    bool ret = point_->remove(shared_from_this());
    return ret;
  }
  return false;
}

/* If the Point is PreCall or PostCall */
PatchFunction*
Point::getCallee() {
  if (type() != PreCall && type() != PostCall) return NULL;
  PatchBlock* b = the_block_;
  PatchBlock::edgelist::const_iterator it = b->getTargets().begin();
  for (; it != b->getTargets().end(); ++it) {
    if ((*it)->type() == ParseAPI::CALL) {
      PatchBlock* trg = (*it)->target();
      return trg->function();
    }
  }
  return NULL;
}

/* Associate this point with the block(s) and function(s)
   that contain it */
void
Point::initCodeStructure() {
  assert(mgr_);
  // walk through all code objects
  for (AddrSpace::ObjSet::iterator ci = mgr_->as()->objSet().begin();
       ci != mgr_->as()->objSet().end(); ci++) {
    PatchObject* obj = *ci;
    CodeObject* co = obj->co();
    CodeSource* cs = co->cs();
    Address relative_addr = addr_ - obj->codeBase();
    vector<CodeRegion*> regions = cs->regions();
    for (vector<CodeRegion*>::iterator ri = regions.begin(); ri != regions.end(); ri++) {
      std::set<ParseAPI::Function*> parseapi_funcs;
      co->findFuncs(*ri, relative_addr, parseapi_funcs);

      for (std::set<ParseAPI::Function*>::iterator fi = parseapi_funcs.begin();
           fi != parseapi_funcs.end(); fi++) {
        PatchFunction* func = obj->getFunc(*fi);
        inst_funcs_.insert(func);
      } // Function
    } // Region
  }
  if (!the_func_) the_func_ = *inst_funcs_.begin();
}

/* for single instruction */
Point::Point(Point::Type type, PatchMgrPtr mgr, PatchBlock *b, Address a, InstructionAPI::Instruction::Ptr i, PatchFunction *f)
   :addr_(a), type_(type), mgr_(mgr), the_block_(b), the_edge_(NULL), the_func_(f), insn_(i) {

  initCodeStructure();
}

/* for a block */
Point::Point(Type type, PatchMgrPtr mgr, PatchBlock* blk, PatchFunction *f)
  : addr_(0), type_(type), mgr_(mgr), the_block_(blk), the_edge_(NULL), the_func_(f) {
  initCodeStructure();
}

/* for an edge */
Point::Point(Type type, PatchMgrPtr mgr, PatchEdge* edge, PatchFunction *f)
  : addr_(0), type_(type), mgr_(mgr), the_block_(NULL), the_edge_(edge), the_func_(f) {
  initCodeStructure();
}

/* for a function */
Point::Point(Type type, PatchMgrPtr mgr, PatchFunction* func) : 
   addr_(0), type_(type), mgr_(mgr),
   the_block_(NULL), the_edge_(NULL), the_func_(func) {
  initCodeStructure();
}

/* for a call or exit site */
Point::Point(Type type, PatchMgrPtr mgr, PatchFunction* func, PatchBlock *b) : 
   addr_(0), type_(type), mgr_(mgr),
   the_block_(b), the_edge_(NULL), the_func_(func) {
  initCodeStructure();
}


/* old_instance, old_instance, <---new_instance */
InstancePtr
Point::pushBack(SnippetPtr snippet) {
  InstancePtr instance = Instance::create(this, snippet);
  if (!instance) return instance;
  instanceList_.push_back(instance);
  instance->set_state(INSERTED);
  return instance;
}

/* new_instance--->, old_instance, old_instance */
InstancePtr
Point::pushFront(SnippetPtr snippet) {
  InstancePtr instance = Instance::create(this, snippet);
  if (!instance) return instance;
  instanceList_.push_front(instance);
  instance->set_state(INSERTED);
  return instance;
}

/* Test whether the type contains a specific type. */
bool
Point::TestType(Point::Type types, Point::Type trg) {
  if (types & trg) return true;
  return false;
}

/* Add a specific type to a set of types */
void
Point::AddType(Point::Type& types, Point::Type trg) {
  int trg_int = static_cast<int>(trg);
  int type_int = static_cast<int>(types);
  type_int |= trg_int;
  types = (Point::Type)type_int;
}

/* Remove a specific type from a set of types */
void
Point::RemoveType(Point::Type& types, Point::Type trg) {
  int trg_int = static_cast<int>(trg);
  int type_int = static_cast<int>(types);
  type_int &= (~trg_int);
  types = (Point::Type)type_int;
}

bool
Point::remove(InstancePtr instance) {
  if (instance == InstancePtr()) return false;
  InstanceList::iterator it = std::find(instanceList_.begin(),
                                 instanceList_.end(), instance);
  if (it != instanceList_.end()) {
    instanceList_.erase(it);
    instance->set_state(PENDING);
    return true;
  }
  return false;
}

size_t
Point::size() {
  return instanceList_.size();
}

void
Point::clear() {
  while (size() > 0) {
    InstancePtr i = instanceList_.back();
    i->destroy();
  }
}

/* 1, Clear all snippet instances
   2, Detach from PatchMgr object */
bool
Point::destroy() {
  clear();
  // TODO: this makes a copy of the point map whenever we call this function;
  // that is an enormous amount of copying.  
  // Also, it won't modify the PatchMgr data structure since we first made
  // a copy. Please fix. 
#if 0
  PatchMgr::TypePtMap type_pt_map;
  switch (type_) {
    case Point::PreInsn:
    case Point::PostInsn:
       //case Point::InsnTaken:
    case Point::PreCall:
    case Point::PostCall:
    {
       type_pt_map = mgr_->addr_type_pt_map_[addr_];
       break;
    }
    case Point::BlockEntry:
    case Point::BlockExit:
    case Point::BlockDuring:
    {
      type_pt_map = mgr_->blk_type_pt_map_[the_block_];
      break;
    }
    case Point::FuncEntry:
    case Point::FuncExit:
    case Point::FuncDuring:
    {
      type_pt_map = mgr_->func_type_pt_map_[the_func_];
      break;
    }
    case Point::EdgeDuring:
    {
      type_pt_map = mgr_->edge_type_pt_map_[the_edge_];
      break;
    }
    case Point::LoopStart:
    case Point::LoopEnd:
    case Point::LoopIterStart:
    case Point::LoopIterEnd:
    {
      return false;
    }
    case Point::InsnTypes:
    case Point::LoopTypes:
    case Point::BlockTypes:
    case Point::CallTypes:
    case Point::FuncTypes:
    case Point::OtherPoint:
    case Point::None:
    {
      return false;
    }
  }
  PointSet& points = type_pt_map[type_];
  points.erase(this);
#endif
  return true;
}

Point::~Point() {
  // Clear all instances associated with this point
  clear();
}

void Point::changeBlock(PatchBlock *block) {
   // TODO: callback from here
   the_block_ = block;
}

FuncPoints::~FuncPoints() {
   if (entry) delete entry;
   if (during) delete during;
   for (std::map<PatchBlock *, Point *>::iterator iter = exits.begin();
        iter != exits.end(); ++iter) {
      delete iter->second;
   }
   for (std::map<PatchBlock *, Point *>::iterator iter = preCalls.begin();
        iter != preCalls.end(); ++iter) {
      delete iter->second;
   }
   for (std::map<PatchBlock *, Point *>::iterator iter = postCalls.begin();
        iter != postCalls.end(); ++iter) {
      delete iter->second;
   }
}

BlockPoints::~BlockPoints() {
   if (entry) delete entry;
   if (during) delete during;
   if (exit) delete exit;
   for (InsnPoints::iterator iter = preInsn.begin(); iter != preInsn.end(); ++iter) {
      delete iter->second;
   }
   for (InsnPoints::iterator iter = postInsn.begin(); iter != postInsn.end(); ++iter) {
      delete iter->second;
   }
}
