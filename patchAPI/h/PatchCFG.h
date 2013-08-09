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

#ifndef _PATCHAPI_DYNINST_CFG_H_
#define _PATCHAPI_DYNINST_CFG_H_

#include "CFG.h"
#include "PatchCommon.h"
#include "PatchObject.h"
#include "Point.h"

namespace Dyninst {
namespace PatchAPI {

class PatchParseCallback;

class PatchEdge;
class PatchBlock;
class PatchFunction;
class PatchCallback;

class PATCHAPI_EXPORT PatchEdge {
   friend class PatchBlock;
   friend class PatchFunction;
   friend class PatchObject;
   friend class Callback;
   friend class PatchParseCallback;

  public:
   static PatchEdge *create(ParseAPI::Edge *,
                                            PatchBlock *src,
                                            PatchBlock *trg);
   PatchEdge(ParseAPI::Edge *internalEdge,
             PatchBlock *source,
             PatchBlock *target);
   PatchEdge(const PatchEdge *parent,
             PatchBlock *child_src,
             PatchBlock *child_trg);
   virtual ~PatchEdge();

   // Getters
   ParseAPI::Edge *edge() const;
   PatchBlock *src();
   PatchBlock *trg();
   ParseAPI::EdgeTypeEnum type() const;
   bool sinkEdge() const;
   bool interproc() const;
   bool intraproc() const { return !interproc(); }

   void remove(Point *);
   PatchCallback *cb() const;

   bool consistency() const;

   std::string format() const;

 protected:
    ParseAPI::Edge *edge_;
    PatchBlock *src_;
    PatchBlock *trg_;

    EdgePoints points_;
};

class PATCHAPI_EXPORT PatchBlock {
  friend class PatchEdge;
  friend class PatchFunction;
  friend class PatchObject;
   friend class PatchParseCallback;

  public:
   typedef std::map<Address, InstructionAPI::Instruction::Ptr> Insns;
    typedef std::vector<PatchEdge*> edgelist;

    static PatchBlock *create(ParseAPI::Block *, PatchFunction *);
    PatchBlock(const PatchBlock *parblk, PatchObject *child);
    PatchBlock(ParseAPI::Block *block, PatchObject *obj);
    virtual ~PatchBlock();

    // Getters
    Address start() const;
    Address end() const;
    Address last() const;
    Address size() const;

    PatchFunction* getFunction(ParseAPI::Function*);
    bool isShared();
    int containingFuncs() const;
    void getInsns(Insns &insns) const;
    InstructionAPI::Instruction::Ptr getInsn(Address a) const;
    std::string disassemble() const;
    bool containsCall() const { return 0 < numCallEdges(); };
    bool containsDynamicCall();
    std::string format() const;
    std::string long_format() const;
    PatchFunction* getCallee();

    ParseAPI::Block *block() const;
    PatchObject* object() const;
    PatchObject *obj() const { return object(); }
    const edgelist &sources();
    const edgelist &targets();
    
    PatchEdge *findSource(ParseAPI::EdgeTypeEnum type);
    PatchEdge *findTarget(ParseAPI::EdgeTypeEnum type);

    template <class OutputIterator> 
    void getFuncs(OutputIterator result);

    Point *findPoint(Location loc, Point::Type type, bool create = true);

   void remove(Point *);
   PatchCallback *cb() const;

   bool consistency() const;

   bool wasUserAdded() const;

   virtual void markModified()  {};

  protected:
    typedef enum {
      backwards,
      forwards } Direction;

    void removeSourceEdge(PatchEdge *e);
    void removeTargetEdge(PatchEdge *e);
    void destroyPoints();

    void addSourceEdge(PatchEdge *e, bool addIfEmpty = true);
    void addTargetEdge(PatchEdge *e, bool addIfEmpty = true);

    void splitBlock(PatchBlock *succ);
    int numRetEdges() const;
    int numCallEdges() const;

    ParseAPI::Block *block_;
    edgelist srclist_;
    edgelist trglist_;
    PatchObject* obj_;

    BlockPoints points_;
};



class PATCHAPI_EXPORT PatchFunction {
   friend class PatchEdge;
   friend class PatchBlock;
   friend class PatchObject;
   friend class PatchParseCallback;
   friend class PatchModifier;

   public:
     struct compare {
       bool operator()(PatchBlock * const &b1,
                       PatchBlock * const &b2) const {
         if(b1->start() < b2->start())
           return true;
         else
           return false;
       }
     };
     typedef std::set<PatchBlock *, compare> Blockset;

     static PatchFunction *create(ParseAPI::Function *, PatchObject*);
     PatchFunction(ParseAPI::Function *f, PatchObject* o);
     PatchFunction(const PatchFunction* parFunc, PatchObject* child);
     virtual ~PatchFunction();

     const string &name() const { return func_->name(); }
     Address addr() const { return addr_;  }
     ParseAPI::Function *function() const { return func_; }
     PatchObject *obj() const { return obj_; }

     PatchBlock *entry();
     const Blockset &blocks();
     const Blockset &callBlocks();
     //const Blockset &returnBlocks();
     const Blockset &exitBlocks();

     Point *findPoint(Location loc, Point::Type type, bool create = true);

     // Moved to source code because we treat non-returning calls as exits
     bool verifyExit(PatchBlock *block) { return exitBlocks().find(block) != exitBlocks().end(); }
     bool verifyCall(PatchBlock *block) { return callBlocks().find(block) != callBlocks().end(); }

     // Const : no building the exit/call sets (returns true if set is empty)
     bool verifyExitConst(const PatchBlock *block) const { 
        return exit_blocks_.empty() || 
            exit_blocks_.find(const_cast<PatchBlock *>(block)) != exit_blocks_.end(); 
     }
     bool verifyCallConst(const PatchBlock *block) const { 
        return call_blocks_.empty() || 
            call_blocks_.find(const_cast<PatchBlock *>(block)) != call_blocks_.end(); 
     }

     // Fast access to a range of instruction points
     bool findInsnPoints(Point::Type type, PatchBlock *block,
                                         InsnPoints::const_iterator &start,
                                         InsnPoints::const_iterator &end);

   void remove(Point *);
   PatchCallback *cb() const;

   bool consistency() const;

   virtual void markModified() {};

   protected:
     // For callbacks from ParseAPI to PatchAPI
     void removeBlock(PatchBlock *);
     void addBlock(PatchBlock *);
     void splitBlock(PatchBlock *first, PatchBlock *second);
     void destroyPoints();
     void destroyBlockPoints(PatchBlock *block);
     void invalidateBlocks();

     ParseAPI::Function *func_;
     PatchObject* obj_;
     Address addr_;

     Blockset all_blocks_;
     Blockset call_blocks_;
     Blockset return_blocks_;
     Blockset exit_blocks_;

     FuncPoints points_;
     // For context-specific
     std::map<PatchBlock *, BlockPoints> blockPoints_;
     std::map<PatchEdge *, EdgePoints> edgePoints_;
};

template <class OutputIterator>
void PatchBlock::getFuncs(OutputIterator result) {
  std::vector<ParseAPI::Function *> pFuncs;
  block()->getFuncs(pFuncs);
  for (unsigned i = 0; i < pFuncs.size(); ++i) {
    PatchFunction *func = getFunction(pFuncs[i]);
    *result = func;
    ++result;
  }
}

#define ASSERT_CONSISTENCY_FAILURE 1
#define CONSIST_FAIL {if (ASSERT_CONSISTENCY_FAILURE) assert(0); return false;}

};
};


#endif /* _PATCHAPI_DYNINST_CFG_H_ */
