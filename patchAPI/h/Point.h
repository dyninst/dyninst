/* Public Interface */

#ifndef PATCHAPI_H_POINT_H_
#define PATCHAPI_H_POINT_H_

#include "common.h"
#include "Snippet.h"

namespace Dyninst {
namespace PatchAPI {

// Used in PointType definition
#define type_val(seq) (0x00000001 << seq)


/* A location on the CFG that acts as a container of inserted instances.  Points
   of different types are distinct even the underlying code relocation and
   generation engine happens to put instrumentation from them at the same
   place */
class Point {
  friend class PatchMgr;

  public:
    // If you want to extend Type, please increment the argument passed
    // to type_val.
    enum Type {
      PreInsn = type_val(0),
      PostInsn = type_val(1),
      InsnTaken = type_val(2),
      BlockEntry = type_val(3),
      BlockExit = type_val(4),
      BlockDuring = type_val(5),
      FuncEntry = type_val(6),
      FuncExit = type_val(7),
      FuncDuring = type_val(8),
      EdgeDuring = type_val(9),
      LoopStart = type_val(10),             // TODO(wenbin)
      LoopEnd = type_val(11),               // TODO(wenbin)
      LoopIterStart = type_val(12),         // TODO(wenbin)
      LoopIterEnd = type_val(13),           // TODO(wenbin)
      PreCall = type_val(14),
      PostCall = type_val(15),
      OtherPoint = type_val(30),
      None = type_val(31),
      InsnTypes = PreInsn | PostInsn | InsnTaken,
      BlockTypes = BlockEntry | BlockExit | BlockDuring,
      FuncTypes = FuncEntry | FuncExit | FuncDuring,
      EdgeTypes = EdgeDuring,
      LoopTypes = LoopStart | LoopEnd | LoopIterStart | LoopIterEnd,  // TODO(wenbin)
      CallTypes = PreCall | PostCall
    };

    template <class Scope>
    static Point* create(Address addr,
                         Point::Type type,
                         PatchMgrPtr mgr,
                         Scope* scope) {
      Point* ret = new Point(addr, type, mgr, scope);
      return ret;
    }
    Point() {}
    Point(Address addr, Point::Type type, PatchMgrPtr mgr, Address*);
    Point(Address addr, Point::Type type, PatchMgrPtr mgr, PatchBlock* blk);
    Point(Address addr, Point::Type type, PatchMgrPtr mgr, PatchEdge* edge);
    Point(Address addr, Point::Type type, PatchMgrPtr mgr, PatchFunction* func);
    virtual ~Point();

    // Point as a snippet container
    typedef std::list<InstancePtr>::iterator instance_iter;
    instance_iter begin() { return instanceList_.begin();}
    instance_iter end() { return instanceList_.end();}
    InstancePtr push_back(SnippetPtr);
    InstancePtr push_front(SnippetPtr);
    bool remove(InstancePtr);

    // Remove all snippets in this point
    void clear();

    // Getters
    size_t size();
    Address address() const { return addr_; }
    Type type() const {return type_;}
    typedef std::set<PatchFunction*> FuncSet;
    typedef std::set<PatchBlock*> BlockSet;
    const Point::FuncSet& getInstFuncs() const { return inst_funcs_; }
    const Point::BlockSet& getInstBlocks() const { return inst_blks_; }
    PatchFunction* getCallee();
    const ParseAPI::CodeObject* co() const { return co_; }
    const ParseAPI::CodeSource* cs() const { return cs_; }
    const PatchObject* obj() const { return obj_; }
    //const InstructionAPI::Instruction::Ptr instruction() const { return instruction_; }
    const InstructionAPI::Instruction::Ptr insn() const { return insn_; }
    PatchFunction* getFunction() const { return the_func_; }
    PatchBlock* getBlock() const { return the_block_; }
    PatchEdge* getEdge() const { return the_edge_; }

    // Point type utilities

    // Test whether the type contains a specific type.
    static bool TestType(Point::Type types, Point::Type trg);
    // Add a specific type to a set of types
    static void AddType(Point::Type& types, Point::Type trg);
    // Remove a specific type from a set of types
    static void RemoveType(Point::Type& types, Point::Type trg);


  protected:
    bool destroy();
    void initCodeStructure();

    InstanceList instanceList_;
    Address addr_;
    Type type_;
    PatchMgrPtr mgr_;
    PatchBlock* the_block_;
    PatchEdge* the_edge_;
    PatchFunction* the_func_;
    InstructionAPI::Instruction::Ptr insn_;
    ParseAPI::CodeObject* co_;
    ParseAPI::CodeSource* cs_;
    PatchObject* obj_;
    Point::FuncSet inst_funcs_;
    Point::BlockSet inst_blks_;
};

inline Point::Type operator|(Point::Type a, Point::Type b) {
  Point::Type types = a;
  Point::AddType(types, b);
  return types;
}

// Type: enum => string
inline const char* type_str(Point::Type type) {
  if (type&Point::PreInsn) return "PreInsn ";
  if (type&Point::PostInsn) return "PostInsn ";
  if (type&Point::InsnTaken) return "InsnTaken ";

  if (type&Point::BlockEntry) return "BlockEntry ";
  if (type&Point::BlockExit) return "BlockExit ";
  if (type&Point::BlockDuring) return "BlockDuring ";

  if (type&Point::FuncEntry) return "FuncEntry ";
  if (type&Point::FuncExit) return "FuncExit ";
  if (type&Point::FuncDuring) return "FuncDuring ";

  if (type&Point::EdgeDuring) return "EdgeDuring ";

  if (type&Point::LoopStart) return "LoopStart ";
  if (type&Point::LoopEnd) return "LoopEnd ";
  if (type&Point::LoopIterStart) return "LoopIterStart ";
  if (type&Point::LoopIterEnd) return "LoopIterEnd ";

  if (type&Point::PreCall) return "PreCall ";
  if (type&Point::PostCall) return "PostCall ";

  return "Unknown";
}

enum SnippetType {
  SYSTEM,
  USER
};

enum SnippetState {
  FAILED,
  PENDING,
  INSERTED
};

/* A representation of a particular snippet inserted at a
   particular point */
class Instance : public dyn_detail::boost::enable_shared_from_this<Instance> {
  public:
    Instance(Point* point, SnippetPtr snippet)
              : point_(point), snippet_(snippet) { }
    virtual ~Instance() {}
    static InstancePtr create(Point*, SnippetPtr,
                    SnippetType type = SYSTEM, SnippetState state = PENDING);

    // Getters and Setters
    SnippetState state() const { return state_;}
    SnippetType type() const {return type_;}
    Point* point() const {return point_;}
    SnippetPtr snippet() const {return snippet_;}
    void set_state(SnippetState state) { state_ = state; }

    // Destroy itself
    // return false if this snipet instance is not associated with a point
    bool destroy();

  protected:
    Point* point_;
    SnippetPtr snippet_;
    SnippetState state_;
    SnippetType type_;
};

/* Factory class for creating a point that could be either PatchAPI::Point or
   the subclass of PatchAPI::Point.   */
class PointMaker {
  public:
    PointMaker(PatchMgrPtr mgr) : mgr_(mgr) {}
    PointMaker() {}
    virtual ~PointMaker() {}

    virtual Point* createPoint(Address addr, Point::Type type,
                               Address* scope);
    virtual Point* createPoint(Address addr, Point::Type type,
                               PatchBlock* scope);
    virtual Point* createPoint(Address addr, Point::Type type,
                               PatchEdge* scope);
    virtual Point* createPoint(Address addr, Point::Type type,
                               PatchFunction* scope);

    void setMgr(PatchMgrPtr mgr) { mgr_ = mgr; }
  protected:
    PatchMgrPtr mgr_;
};

}
}
#endif  // PATCHAPI_H_POINT_H_
