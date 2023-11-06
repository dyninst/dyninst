/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
/* Public Interface */

#ifndef PATCHAPI_H_POINT_H_
#define PATCHAPI_H_POINT_H_

#include <list>
#include <map>
#include <stddef.h>
#include "PatchCommon.h"
#include "Snippet.h"
#include "util.h"
#include "dyntypes.h"

namespace Dyninst {
namespace PatchAPI {


   class PatchCallback;

struct EntrySite_t {
   PatchFunction *func;
   PatchBlock *block;
EntrySite_t(PatchFunction *f, PatchBlock *b) : func(f), block(b) {}
};
struct CallSite_t {
   PatchFunction *func;
   PatchBlock *block;
CallSite_t(PatchFunction *f, PatchBlock *b) : func(f), block(b) {}
};
struct ExitSite_t {
   PatchFunction *func;
   PatchBlock *block;
ExitSite_t(PatchFunction *f, PatchBlock *b) : func(f), block(b) {}
};

struct InsnLoc_t {
   PatchBlock *block;
   Dyninst::Address addr;
   InstructionAPI::Instruction insn;
InsnLoc_t(PatchBlock *b, Dyninst::Address a, InstructionAPI::Instruction i) :
   block(b), addr(a), insn(i) {}
};
      
     
// Uniquely identify the location of a point; this + a type
// uniquely identifies a point.
struct Location {
   static Location Function(PatchFunction *f) { 
      return Location(f, NULL, 0, InstructionAPI::Instruction(), NULL, true, Function_);
   }
   static Location Block(PatchBlock *b) { 
      return Location(NULL, b, 0, InstructionAPI::Instruction(), NULL, true, Block_);
   }
   static Location BlockInstance(PatchFunction *f, PatchBlock *b, bool trusted = false) { 
      return Location(f, b, 0, InstructionAPI::Instruction(), NULL, trusted, BlockInstance_);
   }
   static Location Instruction(InsnLoc_t l) { 
      return Location(NULL, l.block, l.addr, l.insn, NULL, true, Instruction_); 
   }
   static Location Instruction(PatchBlock *b, Dyninst::Address a) {
      return Location(NULL, b, a, InstructionAPI::Instruction(), NULL, false, Instruction_);
   }
   static Location InstructionInstance(PatchFunction *f, InsnLoc_t l, bool trusted = false) { 
      return Location(f, l.block, l.addr, l.insn, NULL, trusted, InstructionInstance_); 
   }
   static Location InstructionInstance(PatchFunction *f, PatchBlock *b, Dyninst::Address a) {
      return Location(f, b, a, InstructionAPI::Instruction(), NULL, false, InstructionInstance_);
   }
   static Location InstructionInstance(PatchFunction *f, PatchBlock *b, Dyninst::Address a,
                                       InstructionAPI::Instruction i, bool trusted = false) {
      return Location(f, b, a, i, NULL, trusted, InstructionInstance_); 
   }
   static Location Edge(PatchEdge *e) {
      return Location(NULL, NULL, 0, InstructionAPI::Instruction(), e, true, Edge_);
   }
   static Location EdgeInstance(PatchFunction *f, PatchEdge *e) { 
      return Location(f, NULL, 0, InstructionAPI::Instruction(), e, false, EdgeInstance_);
   }
   static Location EntrySite(EntrySite_t e) { 
      return Location(e.func, e.block, 0, InstructionAPI::Instruction(), NULL, true, Entry_);
   }
   static Location EntrySite(PatchFunction *f, PatchBlock *b, bool trusted = false) { 
      return Location(f, b, 0, InstructionAPI::Instruction(), NULL, trusted, Entry_);
   }
   static Location CallSite(CallSite_t c) {
      return Location(c.func, c.block, 0, InstructionAPI::Instruction(), NULL, true, Call_);
   }
   static Location CallSite(PatchFunction *f, PatchBlock *b) { 
      return Location(f, b, 0, InstructionAPI::Instruction(), NULL, false, Call_);
   }
   static Location ExitSite(ExitSite_t e) { 
      return Location(e.func, e.block, 0, InstructionAPI::Instruction(), NULL, true, Exit_);
   }
   static Location ExitSite(PatchFunction *f, PatchBlock *b) { 
      return Location(f, b, 0, InstructionAPI::Instruction(), NULL, false, Exit_); }

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

   InsnLoc_t insnLoc() { return InsnLoc_t(block, addr, insn); }
   
   PatchFunction *func;
   PatchBlock *block;
   Dyninst::Address addr;
   InstructionAPI::Instruction insn;
   PatchEdge *edge;
   bool trusted;
   type_t type;

private:
Location(PatchFunction *f, PatchBlock *b, Dyninst::Address a, InstructionAPI::Instruction i, PatchEdge *e, bool u, type_t t) :
   func(f), block(b), addr(a), insn(i), edge(e), trusted(u), type(t) {}

};

// Used in PointType definition
#define type_val(seq) (0x00000001u << seq)


/* A location on the CFG that acts as a container of inserted instances.  Points
   of different types are distinct even the underlying code relocation and
   generation engine happens to put instrumentation from them at the same
   place */

class PATCHAPI_EXPORT Point {
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
    static Point* create(Dyninst::Address addr,
                         Point::Type type,
                         PatchMgrPtr mgr,
                         Scope* scope) {
      Point* ret = new Point(addr, type, mgr, scope);
      return ret;
    }
    Point() {}

    // Block instrumentation /w/ optional function context
    Point(Point::Type t, PatchMgrPtr mgr, PatchBlock *, PatchFunction * = NULL);
    // Insn instrumentation /w/ optional function context
    Point(Point::Type t, PatchMgrPtr mgr, PatchBlock *, Dyninst::Address, InstructionAPI::Instruction, PatchFunction * = NULL);
    // Function entry or during
    Point(Point::Type t, PatchMgrPtr mgr, PatchFunction *);
    // Function call or exit site
    Point(Point::Type t, PatchMgrPtr mgr, PatchFunction *, PatchBlock *);
    // Edge
    Point(Point::Type t, PatchMgrPtr mgr, PatchEdge *, PatchFunction * = NULL);

    virtual ~Point();

    // Point as a snippet container
    typedef std::list<InstancePtr>::iterator instance_iter;
    instance_iter begin() { return instanceList_.begin();}
    instance_iter end() { return instanceList_.end();}
    virtual InstancePtr pushBack(SnippetPtr);
    virtual InstancePtr pushFront(SnippetPtr);
    bool remove(InstancePtr);

    // Remove all snippets in this point
    void clear();

    // Getters
    size_t size();
    Dyninst::Address addr() const { return addr_; }
    Type type() const {return type_;}
    bool empty() const { return instanceList_.empty();}

    PatchFunction* getCallee();

    PatchObject* obj() const;
    const InstructionAPI::Instruction& insn() const { return insn_; }

    PatchFunction *func() const { return the_func_; }
    PatchBlock *block() const { return the_block_; }
    PatchEdge *edge() const { return the_edge_; }
    PatchMgrPtr mgr() const { return mgr_; }

    // Point type utilities
    
    // Test whether the type contains a specific type.
    static bool TestType(Point::Type types, Point::Type trg);
    // Add a specific type to a set of types
    static void AddType(Point::Type& types, Point::Type trg);
    // Remove a specific type from a set of types
    static void RemoveType(Point::Type& types, Point::Type trg);

    PatchCallback *cb() const;

    bool consistency() const;
    
  protected:
    bool destroy();
    void initCodeStructure();
    void changeBlock(PatchBlock *newBlock);


    InstanceList instanceList_;
    Dyninst::Address addr_{};
    Type type_{};
    PatchMgrPtr mgr_;
    PatchBlock* the_block_{};
    PatchEdge* the_edge_{};
    PatchFunction* the_func_{};
    InstructionAPI::Instruction insn_;
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
class PATCHAPI_EXPORT Instance : public boost::enable_shared_from_this<Instance> {
  public:
   typedef boost::shared_ptr<Instance> Ptr;

  Instance(Point* point, SnippetPtr snippet)
     : point_(point), snippet_(snippet), state_(PENDING), type_(SYSTEM), guarded_(true) { }
    virtual ~Instance() {}
    static InstancePtr create(Point*, SnippetPtr,
                        SnippetType type = SYSTEM, SnippetState state = PENDING);

    // Getters and Setters
    SnippetState state() const { return state_;}
    SnippetType type() const {return type_;}
    Point* point() const {return point_;}
    SnippetPtr snippet() const {return snippet_;}
    void set_state(SnippetState state) { state_ = state; }

    void disableRecursiveGuard() { guarded_ = false; }
    bool recursiveGuardEnabled() const { return guarded_; }

    // Destroy itself
    // return false if this snipet instance is not associated with a point
    bool destroy();

  protected:
    Point* point_;
    SnippetPtr snippet_;
    SnippetState state_;
    SnippetType type_;
    bool guarded_;
};

/* Factory class for creating a point that could be either PatchAPI::Point or
   the subclass of PatchAPI::Point.   */
class PATCHAPI_EXPORT PointMaker {
   friend class PatchMgr;
  public:
    PointMaker(PatchMgrPtr mgr) : mgr_(mgr) {}
    PointMaker() {}
    virtual ~PointMaker() {}

    // Location bundles what we need to know. 
    Point *createPoint(Location loc, Point::Type type);

    void setMgr(PatchMgrPtr mgr) { mgr_ = mgr; }
  protected:
    // User override
    virtual Point *mkFuncPoint(Point::Type t, PatchMgrPtr m, PatchFunction *);
    virtual Point *mkFuncSitePoint(Point::Type t, PatchMgrPtr m, PatchFunction *, PatchBlock *);
    virtual Point *mkBlockPoint(Point::Type t, PatchMgrPtr m, PatchBlock *, PatchFunction *context);
    virtual Point *mkInsnPoint(Point::Type t, PatchMgrPtr m, PatchBlock *, Dyninst::Address, InstructionAPI::Instruction,
                               PatchFunction *context);
    virtual Point *mkEdgePoint(Point::Type t, PatchMgrPtr m, PatchEdge *, PatchFunction *context);


    PatchMgrPtr mgr_;
};

// Collection classes
typedef std::map<Dyninst::Address, Point *> InsnPoints;


struct BlockPoints {
   Point *entry{};
   Point *during{};
   Point *exit{};
   InsnPoints preInsn;
   InsnPoints postInsn;
   BlockPoints() = default;
   BlockPoints(const BlockPoints&) = delete;
   BlockPoints(BlockPoints&& other)
      {
         *this = other;
         other.entry = nullptr;
         other.during = nullptr;
         other.exit = nullptr;
      }
   bool consistency(const PatchBlock *block, const PatchFunction *func) const;
   ~BlockPoints();
 private:
   // used by move constructor to default copy members
   BlockPoints& operator=(const BlockPoints&) = default;
};

struct EdgePoints {
   Point *during{};
   EdgePoints() = default;
   EdgePoints(EdgePoints&& other) { during = other.during; other.during = nullptr; }
   ~EdgePoints() { if (during) delete during; }
   bool consistency(const PatchEdge *edge, const PatchFunction *func) const;
   EdgePoints(const EdgePoints&) = delete;
};

struct FuncPoints {
   Point *entry{};
   Point *during{};
   std::map<PatchBlock *, Point *> exits;
   std::map<PatchBlock *, Point *> preCalls;
   std::map<PatchBlock *, Point *> postCalls;
   FuncPoints() = default;
   FuncPoints(const FuncPoints&) = delete;
   ~FuncPoints();
   bool consistency(const PatchFunction *func) const;
};


}
}


#endif  // PATCHAPI_H_POINT_H_
