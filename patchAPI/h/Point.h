/* Public Interface */

#ifndef PATCHAPI_H_POINT_H_
#define PATCHAPI_H_POINT_H_

#include "common.h"
#include "Snippet.h"

namespace Dyninst {
namespace PatchAPI {

   class PatchCallback;

struct EntrySite {
   PatchFunction *func;
   PatchBlock *block;
EntrySite(PatchFunction *f, PatchBlock *b) : func(f), block(b) {};
};
struct CallSite {
   PatchFunction *func;
   PatchBlock *block;
CallSite(PatchFunction *f, PatchBlock *b) : func(f), block(b) {};
};
struct ExitSite {
   PatchFunction *func;
   PatchBlock *block;
ExitSite(PatchFunction *f, PatchBlock *b) : func(f), block(b) {};
};

struct InsnLoc {
   PatchBlock *block;
   Address addr;
   InstructionAPI::Instruction::Ptr insn;
InsnLoc(PatchBlock *b, Address a, InstructionAPI::Instruction::Ptr i) : 
   block(b), addr(a), insn(i) {};
};
      
     
// Uniquely identify the location of a point; this + a type
// uniquely identifies a point.
struct Location {
Location() : func(NULL), block(NULL), addr(0), edge(NULL), untrusted(false), type(Illegal) {};
   // Function
Location(PatchFunction *f) : func(f), block(NULL), addr(0), edge(NULL), untrusted(false), type(Function) {};
Location(EntrySite e) : func(e.func), block(e.block), addr(0), edge(NULL), untrusted(false), type(Entry) {};
Location(CallSite c) : func(c.func), block(c.block), addr(0), edge(NULL), untrusted(false), type(Call) {};
Location(ExitSite e) : func(e.func), block(e.block), addr(0), edge(NULL), untrusted(false), type(Exit) {};
   // A block in a particular function
Location(PatchFunction *f, PatchBlock *b) : func(f), block(b), addr(0), edge(NULL), untrusted(true), type(BlockInstance) {};
   // A trusted instruction (in a particular function)
Location(PatchFunction *f, InsnLoc l) : func(f), block(l.block), addr(l.addr), insn(l.insn), edge(NULL), untrusted(false), type(InstructionInstance) {};
   // An untrusted (raw) instruction (in a particular function)
Location(PatchFunction *f, PatchBlock *b, Address a, InstructionAPI::Instruction::Ptr i) : func(f), block(b), addr(a), insn(i), edge(NULL), untrusted(true), type(InstructionInstance) {};
   // An edge (in a particular function)
Location(PatchFunction *f, PatchEdge *e) : func(f), block(NULL), addr(0), edge(e), untrusted(true), type(Edge) {};
   // A block in general
Location(PatchBlock *b) : func(NULL), block(b), addr(0), edge(NULL), untrusted(false), type(Block) {};
   // A trusted instruction in general
Location(InsnLoc l) : func(NULL), block(l.block), addr(l.addr), insn(l.insn), edge(NULL), untrusted(false), type(Instruction) {};
   // An untrusted (raw) instruction
Location(PatchBlock *b, Address a) : func(NULL), block(b), addr(a), edge(NULL), untrusted(true), type(Instruction) {};
   // An edge
Location(PatchEdge *e) : func(NULL), block(NULL), addr(0), edge(e), untrusted(false), type(Edge) {};

   typedef enum {
      Function,
      Block,
      BlockInstance,
      Instruction,
      InstructionInstance,
      Edge,
      Entry,
      Call,
      Exit,
      Illegal } type_t;

   bool legal(type_t t) { return t == type; }

   InsnLoc insnLoc() { return InsnLoc(block, addr, insn); }
   
   PatchFunction *func;
   PatchBlock *block;
   Address addr;
   InstructionAPI::Instruction::Ptr insn;
   PatchEdge *edge;
   bool untrusted;
   type_t type;
};

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
    Address address() const { return addr_; }
    Type type() const {return type_;}
    bool empty() const { return instanceList_.empty();}

    PATCHAPI_EXPORT PatchFunction* getCallee();

    const ParseAPI::CodeObject* co() const { return co_; }
    const ParseAPI::CodeSource* cs() const { return cs_; }
    const PatchObject* obj() const { return obj_; }
    const InstructionAPI::Instruction::Ptr insn() const { return insn_; }

    PatchFunction* getFunction() const { return the_func_; }
    PatchBlock* getBlock() const { return the_block_; }
    PatchEdge* getEdge() const { return the_edge_; }

    // Point type utilities
    
    // Test whether the type contains a specific type.
    PATCHAPI_EXPORT static bool TestType(Point::Type types, Point::Type trg);
    // Add a specific type to a set of types
    PATCHAPI_EXPORT static void AddType(Point::Type& types, Point::Type trg);
    // Remove a specific type from a set of types
    PATCHAPI_EXPORT static void RemoveType(Point::Type& types, Point::Type trg);

    PATCHAPI_EXPORT PatchCallback *cb() const;
    
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
    ParseAPI::CodeObject* co_;
    ParseAPI::CodeSource* cs_;
    PatchObject* obj_;
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

    // Location bundles what we need to know. 
    PATCHAPI_EXPORT Point *createPoint(Location loc, Point::Type type);

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
   ~BlockPoints();
};

struct EdgePoints {
   Point *during;
EdgePoints() : during(NULL) {};
   ~EdgePoints() { if (during) delete during; };
};

struct FuncPoints {
   Point *entry;
   Point *during;
   std::map<PatchBlock *, Point *> exits;
   std::map<PatchBlock *, Point *> preCalls;
   std::map<PatchBlock *, Point *> postCalls;
FuncPoints() : entry(NULL), during(NULL) {};
   ~FuncPoints();
};


}
}
#endif  // PATCHAPI_H_POINT_H_
