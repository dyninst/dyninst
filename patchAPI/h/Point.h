/* Public Interface */

#ifndef PATCHAPI_H_POINT_H_
#define PATCHAPI_H_POINT_H_

#include "common.h"
#include "Snippet.h"

namespace Dyninst {
namespace PatchAPI {

   class PatchCallback;


struct PatchEntrySite_t {
   PatchFunction *func;
   PatchBlock *block;
PatchEntrySite_t(PatchFunction *f, PatchBlock *b) : func(f), block(b) {};
};
struct PatchCallSite_t {
   PatchFunction *func;
   PatchBlock *block;
PatchCallSite_t(PatchFunction *f, PatchBlock *b) : func(f), block(b) {};
};
struct PatchExitSite_t {
   PatchFunction *func;
   PatchBlock *block;
PatchExitSite_t(PatchFunction *f, PatchBlock *b) : func(f), block(b) {};
};

struct PatchInsnLoc_t {
   PatchBlock *block;
   Address addr;
   InstructionAPI::Instruction::Ptr insn;
PatchInsnLoc_t(PatchBlock *b, Address a, InstructionAPI::Instruction::Ptr i) : 
   block(b), addr(a), insn(i) {};
};
      
     
// Uniquely identify the PatchLocation of a point; this + a type
// uniquely identifies a point.
struct PatchLocation {
   static PatchLocation Function(PatchFunction *f) { 
      return PatchLocation(f, NULL, 0, InstructionAPI::Instruction::Ptr(), NULL, true, Function_); 
   }
   static PatchLocation Block(PatchBlock *b) { 
      return PatchLocation(NULL, b, 0, InstructionAPI::Instruction::Ptr(), NULL, true, Block_);
   }
   static PatchLocation BlockInstance(PatchFunction *f, PatchBlock *b, bool trusted = false) { 
      return PatchLocation(f, b, 0, InstructionAPI::Instruction::Ptr(), NULL, trusted, BlockInstance_); 
   }
   static PatchLocation Instruction(PatchInsnLoc_t l) { 
      return PatchLocation(NULL, l.block, l.addr, l.insn, NULL, true, Instruction_); 
   }
   static PatchLocation Instruction(PatchBlock *b, Address a) { 
      return PatchLocation(NULL, b, a, InstructionAPI::Instruction::Ptr(), NULL, false, Instruction_); 
   }
   static PatchLocation InstructionInstance(PatchFunction *f, PatchInsnLoc_t l, bool trusted = false) { 
      return PatchLocation(f, l.block, l.addr, l.insn, NULL, trusted, InstructionInstance_); 
   }
   static PatchLocation InstructionInstance(PatchFunction *f, PatchBlock *b, Address a) { 
      return PatchLocation(f, b, a, InstructionAPI::Instruction::Ptr(), NULL, false, InstructionInstance_); 
   }
   static PatchLocation InstructionInstance(PatchFunction *f, PatchBlock *b, Address a, InstructionAPI::Instruction::Ptr i, bool trusted = false) { 
      return PatchLocation(f, b, a, i, NULL, trusted, InstructionInstance_); 
   }
   static PatchLocation Edge(PatchEdge *e) {
      return PatchLocation(NULL, NULL, 0, InstructionAPI::Instruction::Ptr(), e, true, Edge_); 
   }
   static PatchLocation EdgeInstance(PatchFunction *f, PatchEdge *e) { 
      return PatchLocation(f, NULL, 0, InstructionAPI::Instruction::Ptr(), e, false, EdgeInstance_);
   }
   static PatchLocation EntrySite(PatchEntrySite_t e) { 
      return PatchLocation(e.func, e.block, 0, InstructionAPI::Instruction::Ptr(), NULL, true, Entry_); 
   }
   static PatchLocation EntrySite(PatchFunction *f, PatchBlock *b, bool trusted = false) { 
      return PatchLocation(f, b, 0, InstructionAPI::Instruction::Ptr(), NULL, trusted, Entry_); 
   }
   static PatchLocation CallSite(PatchCallSite_t c) {
      return PatchLocation(c.func, c.block, 0, InstructionAPI::Instruction::Ptr(), NULL, true, Call_); 
   }
   static PatchLocation CallSite(PatchFunction *f, PatchBlock *b) { 
      return PatchLocation(f, b, 0, InstructionAPI::Instruction::Ptr(), NULL, false, Call_);
   }
   static PatchLocation ExitSite(PatchExitSite_t e) { 
      return PatchLocation(e.func, e.block, 0, InstructionAPI::Instruction::Ptr(), NULL, true, Exit_);
   }
   static PatchLocation ExitSite(PatchFunction *f, PatchBlock *b) { 
      return PatchLocation(f, b, 0, InstructionAPI::Instruction::Ptr(), NULL, false, Exit_); }

   typedef enum {
      Function_,
      Block_,
      BlockInstance_,
      Instruction_,
      InstructionInstance_,
      Edge_,
      EdgeInstance_,
      Entry_,
      Call_,
      Exit_,
      Illegal_ } type_t;

   bool legal(type_t t) { return t == type; }

   PatchInsnLoc_t insnLoc() { return PatchInsnLoc_t(block, addr, insn); }
   
   PatchFunction *func;
   PatchBlock *block;
   Address addr;
   InstructionAPI::Instruction::Ptr insn;
   PatchEdge *edge;
   bool trusted;
   type_t type;

private:
PatchLocation(PatchFunction *f, PatchBlock *b, Address a, InstructionAPI::Instruction::Ptr i, PatchEdge *e, bool u, type_t t) :
   func(f), block(b), addr(a), insn(i), edge(e), trusted(u), type(t) {};

};

// Used in PointType definition
#define type_val(seq) (0x00000001 << seq)


/* A PatchLocation on the CFG that acts as a container of inserted instances.  Points
   of different types are distinct even the underlying code rePatchLocation and
   generation engine happens to put instrumentation from them at the same
   place */

class Point {
  friend class PatchMgr;
  friend class PatchBlock;
  friend class PatchFunction;

  public:
    // If you want to extend Type, please increment the argument passed
    // to type_val.
    enum Type {
      PreInsn = type_val(0),
      PostInsn = type_val(1),
      //InsnTaken = type_val(2), // This should be edge instrumentation? bernat, 25JUN11
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
      InsnTypes = PreInsn | PostInsn,
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

    // Block instrumentation /w/ optional function context
    PATCHAPI_EXPORT Point(Point::Type t, PatchMgrPtr mgr, PatchBlock *, PatchFunction * = NULL);
    // Insn instrumentation /w/ optional function context
    PATCHAPI_EXPORT Point(Point::Type t, PatchMgrPtr mgr, PatchBlock *, Address, InstructionAPI::Instruction::Ptr, PatchFunction * = NULL);
    // Function entry or during
    PATCHAPI_EXPORT Point(Point::Type t, PatchMgrPtr mgr, PatchFunction *);
    // Function call or exit site
    PATCHAPI_EXPORT Point(Point::Type t, PatchMgrPtr mgr, PatchFunction *, PatchBlock *);
    // Edge
    PATCHAPI_EXPORT Point(Point::Type t, PatchMgrPtr mgr, PatchEdge *, PatchFunction * = NULL);

    PATCHAPI_EXPORT virtual ~Point();

    // Point as a snippet container
    typedef std::list<InstancePtr>::iterator instance_iter;
    instance_iter begin() { return instanceList_.begin();}
    instance_iter end() { return instanceList_.end();}
    PATCHAPI_EXPORT InstancePtr pushBack(SnippetPtr);
    PATCHAPI_EXPORT InstancePtr pushFront(SnippetPtr);
    PATCHAPI_EXPORT bool remove(InstancePtr);

    // Remove all snippets in this point
    PATCHAPI_EXPORT void clear();

    // Getters
    PATCHAPI_EXPORT size_t size();
    Address addr() const { return addr_; }
    PATCHAPI_EXPORT Type type() const {return type_;}
    bool empty() const { return instanceList_.empty();}

    PATCHAPI_EXPORT PatchFunction* getCallee();

    PATCHAPI_EXPORT PatchObject* obj() const;
    const InstructionAPI::Instruction::Ptr insn() const { return insn_; }

    PATCHAPI_EXPORT PatchFunction *func() const { return the_func_; }
    PATCHAPI_EXPORT PatchBlock *block() const { return the_block_; }
    PATCHAPI_EXPORT PatchEdge *edge() const { return the_edge_; }

    // Point type utilities
    
    // Test whether the type contains a specific type.
    PATCHAPI_EXPORT static bool TestType(Point::Type types, Point::Type trg);
    // Add a specific type to a set of types
    PATCHAPI_EXPORT static void AddType(Point::Type& types, Point::Type trg);
    // Remove a specific type from a set of types
    PATCHAPI_EXPORT static void RemoveType(Point::Type& types, Point::Type trg);

    PATCHAPI_EXPORT PatchCallback *cb() const;

    bool consistency() const;
    
  protected:
    bool destroy();
    void initCodeStructure();
    void changeBlock(PatchBlock *newBlock);


    InstanceList instanceList_;
    Address addr_;
    Type type_;
    PatchMgrPtr mgr_;
    PatchBlock* the_block_;
    PatchEdge* the_edge_;
    PatchFunction* the_func_;
    InstructionAPI::Instruction::Ptr insn_;
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
  //if (type&Point::InsnTaken) return "InsnTaken ";

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
   friend class PatchMgr;
  public:
    PointMaker(PatchMgrPtr mgr) : mgr_(mgr) {}
    PointMaker() {}
    virtual ~PointMaker() {}

    // PatchLocation bundles what we need to know. 
    PATCHAPI_EXPORT Point *createPoint(PatchLocation loc, Point::Type type);

    void setMgr(PatchMgrPtr mgr) { mgr_ = mgr; }
  protected:
    // User override
    PATCHAPI_EXPORT virtual Point *mkFuncPoint(Point::Type t, PatchMgrPtr m, PatchFunction *);
    PATCHAPI_EXPORT virtual Point *mkFuncSitePoint(Point::Type t, PatchMgrPtr m, PatchFunction *, PatchBlock *);
    PATCHAPI_EXPORT virtual Point *mkBlockPoint(Point::Type t, PatchMgrPtr m, PatchBlock *, PatchFunction *context);
    PATCHAPI_EXPORT virtual Point *mkInsnPoint(Point::Type t, PatchMgrPtr m, PatchBlock *, Address, InstructionAPI::Instruction::Ptr, PatchFunction *context);
    PATCHAPI_EXPORT virtual Point *mkEdgePoint(Point::Type t, PatchMgrPtr m, PatchEdge *, PatchFunction *context);


    PatchMgrPtr mgr_;
};

// Collection classes
typedef std::map<Address, Point *> InsnPoints;


struct BlockPoints {
   Point *entry;
   Point *during;
   Point *exit;
   InsnPoints preInsn;
   InsnPoints postInsn;
BlockPoints() : entry(NULL), during(NULL), exit(NULL) {};
   bool consistency(const PatchBlock *block, const PatchFunction *func) const;
   ~BlockPoints();
};

struct EdgePoints {
   Point *during;
EdgePoints() : during(NULL) {};
   ~EdgePoints() { if (during) delete during; };
   bool consistency(const PatchEdge *edge, const PatchFunction *func) const;
};

struct FuncPoints {
   Point *entry;
   Point *during;
   std::map<PatchBlock *, Point *> exits;
   std::map<PatchBlock *, Point *> preCalls;
   std::map<PatchBlock *, Point *> postCalls;
FuncPoints() : entry(NULL), during(NULL) {};
   ~FuncPoints();
   bool consistency(const PatchFunction *func) const;
};


}
}
#endif  // PATCHAPI_H_POINT_H_
