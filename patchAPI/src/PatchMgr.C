/* Public Interface */

#include "PatchMgr.h"
#include "PatchObject.h"
#include "PatchCFG.h"

using Dyninst::ParseAPI::CodeSource;
using Dyninst::InstructionAPI::InstructionDecoder;
using Dyninst::InstructionAPI::Instruction;
using Dyninst::PatchAPI::PatchMgr;
using Dyninst::PatchAPI::PatchMgrPtr;
using Dyninst::PatchAPI::PointMaker;
using Dyninst::PatchAPI::AddrSpacePtr;
using Dyninst::PatchAPI::PointMakerPtr;
using Dyninst::PatchAPI::InstrumenterPtr;
using Dyninst::PatchAPI::InstancePtr;
using Dyninst::PatchAPI::Point;
using Dyninst::PatchAPI::PointSet;
using Dyninst::PatchAPI::PatchBlock;
using Dyninst::PatchAPI::PatchEdge;
using Dyninst::PatchAPI::PatchFunction;

bool debug_patchapi_flag = false;

static void
initDebugFlag() {
  if (getenv("PATCHAPI_DEBUG"))
    debug_patchapi_flag = true;
}

PatchMgr::PatchMgr(AddrSpacePtr as, PointMakerPtr pt, InstrumenterPtr inst)
  : point_maker_(pt), as_(as), batch_mode_(0) {
  if (inst == InstrumenterPtr()) {
    instor_ = Instrumenter::create(as);
  } else {
    inst->setAs(as);
    instor_ = inst;
  }
}

PatchMgrPtr
PatchMgr::create(AddrSpacePtr as, PointMakerPtr pf, InstrumenterPtr inst) {
  PatchMgrPtr ret = PatchMgrPtr(new PatchMgr(as, pf, inst));
  if (!ret) return PatchMgrPtr();
  initDebugFlag();
  ret->as_->mgr_ = ret;
  ret->pointMaker()->setMgr(ret);
  patch_cerr << "PatchAPI starts.\n";
  patch_cerr << ws2 << "Glue Instrumenter and Linker ot PatchMgr.\n";
  return ret;
}

bool
PatchMgr::batchStart() {
  patch_cerr << ws2 << "Batch Start.\n";
  if (batch_mode_ != 0) {
    return false;
  }

  batch_mode_++;
  return true;
}

/* Return false if no point is found */
bool
PatchMgr::removeSnippet(InstancePtr instance) {
  if (instance == InstancePtr()) return false;
  return instance->destroy();
}

/* If there's NOT any point in type_pt_map, create one
   otherwise, simply fill those to *points* */
template <class Scope> void
PatchMgr::getPointsByType(TypePtMap& type_pt_map, Point::Type types,
                                Point::Type type, Address addr,
                                Scope* scope, PointSet& points) {
  // If there's NOT a specific *type* in *types*, done.
  if (!Point::TestType(types, type)) {
    return;
  }

  // If there's a specific *type* in *types*:
  PointSet& pts = type_pt_map[type];
  bool should_create = true;
  for (PointSet::iterator i = pts.begin(); i != pts.end(); i++) {
    if ((*i)->address() == addr) {
      should_create = false;
      break;
    }
  }
  if (should_create) {
    Point* point;
    point = point_maker_->createPoint(addr, type, scope);
    pts.insert(point);
    del_pt_set_.insert(point);
  }
  std::copy(pts.begin(), pts.end(), inserter(points, points.begin()));
}

/* Address-level points:
   - Valid Types: INSN_BEFORE, INSN_FT, INSN_TAKEN
   return false if no point is found */
bool
PatchMgr::findPointsByType(Address* addr, Point::Type types,
                           PointSet& points) {
  // Make sure this set contains only points that we find in this method
  points.clear();
  TypePtMap& type_pt_map = addr_type_pt_map_[*addr];

  CodeSource* cs = NULL;
  Address relative_addr = 0;
  Address codeBase = 0;

  for (AddrSpace::ObjSet::iterator ci = as_->objSet().begin();
       ci != as_->objSet().end(); ci++) {
    codeBase = (*ci)->codeBase();
    relative_addr = *addr - codeBase;
    if ((*ci)->cs()->isValidAddress(relative_addr)) {
      cs = (*ci)->cs();
      break;
    } else {
      continue;
    }
  }
  if (cs == NULL) {
    return false;
  }
  InstructionDecoder d(cs->getPtrToInstruction(relative_addr),
                       cs->length(),
                       cs->getArch());

  Instruction::Ptr insn = d.decode();
  if (insn == 0) {
    patch_cerr << "ERROR: instruction at relative addr 0x" << std::hex << relative_addr
               << " is not a valid instruction.\n";
    points.clear();
    return false;
  }
  getPointsByType(type_pt_map, types, Point::PreInsn, *addr, addr, points);
  getPointsByType(type_pt_map, types, Point::PostInsn, *addr, addr, points);
  if (insn->getCategory() == InstructionAPI::c_BranchInsn) {
    getPointsByType(type_pt_map, types, Point::InsnTaken, *addr, addr, points);
  }
  if (points.size() == 0) return false;
  return true;
}

/* Block-level points:
   - Valid Types: BLOCK_ENTRY, BLOCK_EXIT, BLOCK_DURING

   Address-level points (address inside this block):
   - call findPointsByType(Address ...)

   return false if no point is found */
bool
PatchMgr::findPointsByType(PatchBlock* blk, Point::Type types,
                           PointSet& points) {
  // Make sure this set contains only points that we find in this method
  points.clear();
  TypePtMap& type_pt_map = blk_type_pt_map_[blk];

  // Find block specific points, including:
  //  BLOCK_ENTRY, BLOCK_EXIT, BLOCK_DURING
  Address addr = blk->start();
  getPointsByType(type_pt_map, types, Point::BlockEntry, addr, blk, points);

  addr = blk->last();
  getPointsByType(type_pt_map, types, Point::BlockExit, addr, blk, points);

  addr = blk->start();
  getPointsByType(type_pt_map, types, Point::BlockDuring, addr, blk, points);

  // Find instruction (Address) specific points
  Address off = blk->start();
  Address relative_off = off - blk->object()->codeBase();
  InstructionDecoder d(blk->block()->region()->getPtrToInstruction(relative_off),
                       blk->size(),
                       blk->block()->region()->getArch());
  while (off < blk->end()) {
    Address insn_addr = off;
    PointSet insn_points;
    findPointsByType(&insn_addr, types, insn_points);
    std::copy(insn_points.begin(), insn_points.end(),
              inserter(points, points.begin()));
    off += d.decode()->size();
  }

  // Find call points and edge points for target edges
  PatchBlock::edgelist::iterator teit = blk->getTargets().begin();
  for (; teit != blk->getTargets().end(); ++teit) {
    PatchEdge* edge = *teit;
    PointSet edge_points;
    if (edge->type() == ParseAPI::CALL) {
      Address a = blk->last();
      getPointsByType(type_pt_map, types, Point::PreCall, a, blk, points);
      getPointsByType(type_pt_map, types, Point::PostCall, a, blk, points);
    }
    findPointsByType(edge, types, edge_points);
    std::copy(edge_points.begin(), edge_points.end(),
              inserter(points, points.begin()));
  }

  // Find edge points for source edges
  PatchBlock::edgelist::iterator eit = blk->getSources().begin();
  for (; eit != blk->getSources().end(); ++eit) {
    PatchEdge* edge = *eit;
    PointSet edge_points;
    findPointsByType(edge, types, edge_points);
    std::copy(edge_points.begin(), edge_points.end(),
              inserter(points, points.begin()));
  }

  if (points.size() == 0) return false;
  return true;
}

/* Edge-level points:
   - Valid Types: EDGE_DURING
  return false if no point is found */
bool
PatchMgr::findPointsByType(PatchEdge* edge, Point::Type types,
                           PointSet& points) {
  // Make sure this set contains only points that we find in this method
  points.clear();
  TypePtMap& type_pt_map = edge_type_pt_map_[edge];

  // Find edge specific points, including:
  //  EDGE_DURING

  // TODO(wenbin): handle indirect case
  Address addr = 0;
  switch (edge->type()) {
    case ParseAPI::COND_TAKEN:
    case ParseAPI::DIRECT:
      if (edge->source() != NULL) {
        addr = edge->source()->last();
      }
      break;
    case ParseAPI::COND_NOT_TAKEN:
    case ParseAPI::CALL:
    case ParseAPI::FALLTHROUGH:
    case ParseAPI::CATCH:
    case ParseAPI::CALL_FT:
      if (edge->source() != NULL) {
        addr = edge->source()->last();
      }
      break;
    case ParseAPI::RET:
      if (edge->source() != NULL) {
        addr = edge->source()->last();
      }
      break;
    case ParseAPI::INDIRECT:
    case ParseAPI::NOEDGE:
    case ParseAPI::_edgetype_end_:
      return false;
  }
  getPointsByType(type_pt_map, types, Point::EdgeDuring, addr, edge, points);
  if (points.size() == 0) return false;
  return true;
}

/* Function-level points:
   - Valid Types: FUNC_ENTRY, FUNC_EXIT, FUNC_DURING
   Block-level points (blocks inside this func)
   Edge-level points (edges inside this func)
   return false if no point is found */
bool
PatchMgr::findPointsByType(PatchFunction* func,
                           Point::Type types, PointSet& points) {
  // Make sure this set contains only points that we find in this method
  points.clear();
  TypePtMap& type_pt_map = func_type_pt_map_[func];

  // Find function specific points, including:
  //  FUNC_ENTRY, FUNC_EXIT, FUNC_DURING
  Address addr = func->addr();
  getPointsByType(type_pt_map, types, Point::FuncEntry, addr, func, points);

  const  PatchFunction::blocklist& retblks = func->getExitBlocks();
  for (PatchFunction::blocklist::const_iterator bi = retblks.begin();
       bi != retblks.end(); bi++) {
    PatchBlock* blk = *bi;
    addr = blk->last();
    getPointsByType(type_pt_map, types, Point::FuncExit, addr, func, points);
  }
  addr = func->addr();
  getPointsByType(type_pt_map, types, Point::FuncDuring, addr, func, points);


  // Find block specific points, including:
  // BLOCK_ENTRY, BLOCK_EXIT, BLOCK_DURING
  const PatchFunction::blocklist& blks = func->getAllBlocks();
  PatchFunction::blocklist::const_iterator bit = blks.begin();
  for (; bit != func->getAllBlocks().end(); ++bit) {
    PatchBlock* blk = *bit;
    PointSet blk_points;
    findPointsByType(blk, types, blk_points);
    std::copy(blk_points.begin(), blk_points.end(),
              inserter(points, points.begin()));
  }
  if (points.size() == 0) return false;
  return true;
}

/* Start instrumentation */
bool
PatchMgr::patch() {
  patch_cerr << ws4 << "Relocation and Generation Start.\n";

  if (!instor_->process()) {
    std::cerr << "ERROR: instrumenter process failed!\n";
    return false;
  }

  patch_cerr << ws2 << "Batch Finish.\n";
  return true;
}

PatchMgr::~PatchMgr() {
  for (PointSet::iterator pi = del_pt_set_.begin(); pi != del_pt_set_.end(); pi++) {
    delete *pi;
  }
}

