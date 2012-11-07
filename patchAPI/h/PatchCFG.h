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

class PatchEdge {
   friend class PatchBlock;
   friend class PatchFunction;
   friend class PatchObject;
   friend class Callback;
   friend class PatchParseCallback;

  public:
   PATCHAPI_EXPORT static PatchEdge *create(ParseAPI::Edge *,
                                            PatchBlock *src,
                                            PatchBlock *trg);
   PATCHAPI_EXPORT PatchEdge(ParseAPI::Edge *internalEdge,
             PatchBlock *source,
             PatchBlock *target);
   PATCHAPI_EXPORT PatchEdge(const PatchEdge *parent,
             PatchBlock *child_src,
             PatchBlock *child_trg);
   PATCHAPI_EXPORT virtual ~PatchEdge();

   // Getters
   PATCHAPI_EXPORT ParseAPI::Edge *edge() const;
   PATCHAPI_EXPORT PatchBlock *src();
   PATCHAPI_EXPORT PatchBlock *trg();
   PATCHAPI_EXPORT ParseAPI::EdgeTypeEnum type() const;
   PATCHAPI_EXPORT bool sinkEdge() const;
   PATCHAPI_EXPORT bool interproc() const;
   PATCHAPI_EXPORT bool intraproc() const { return !interproc(); }

   PATCHAPI_EXPORT void remove(Point *);
   PATCHAPI_EXPORT PatchCallback *cb() const;

   PATCHAPI_EXPORT bool consistency() const;

   PATCHAPI_EXPORT std::string format() const;

 protected:
    ParseAPI::Edge *edge_;
    PatchBlock *src_;
    PatchBlock *trg_;

    EdgePoints points_;
};

class PatchBlock {
  friend class PatchEdge;
  friend class PatchFunction;
  friend class PatchObject;
   friend class PatchParseCallback;

  public:
    typedef std::map<Address, InstructionAPI::Instruction::Ptr> Insns;
    typedef std::vector<PatchEdge*> edgelist;

    PATCHAPI_EXPORT static PatchBlock *create(ParseAPI::Block *, PatchFunction *);
    PATCHAPI_EXPORT PatchBlock(const PatchBlock *parblk, PatchObject *child);
    PATCHAPI_EXPORT PatchBlock(ParseAPI::Block *block, PatchObject *obj);
    PATCHAPI_EXPORT virtual ~PatchBlock();

    // Getters
    PATCHAPI_EXPORT Address start() const;
    PATCHAPI_EXPORT Address end() const;
    PATCHAPI_EXPORT Address last() const;
    PATCHAPI_EXPORT Address size() const;

    PATCHAPI_EXPORT PatchFunction* getFunction(ParseAPI::Function*);
    PATCHAPI_EXPORT bool isShared();
    PATCHAPI_EXPORT int containingFuncs() const;
    PATCHAPI_EXPORT void getInsns(Insns &insns) const;
    PATCHAPI_EXPORT InstructionAPI::Instruction::Ptr getInsn(Address a) const;
    PATCHAPI_EXPORT std::string disassemble() const;
    PATCHAPI_EXPORT bool containsCall() const { return 0 < numCallEdges(); };
    PATCHAPI_EXPORT bool containsDynamicCall();
    PATCHAPI_EXPORT std::string format() const;
    PATCHAPI_EXPORT std::string long_format() const;
    PATCHAPI_EXPORT PatchFunction* getCallee();

    PATCHAPI_EXPORT ParseAPI::Block *block() const;
    PATCHAPI_EXPORT PatchObject* object() const;
    PATCHAPI_EXPORT PatchObject *obj() const { return object(); }
    PATCHAPI_EXPORT const edgelist &sources();
    PATCHAPI_EXPORT const edgelist &targets();
    
    PATCHAPI_EXPORT PatchEdge *findSource(ParseAPI::EdgeTypeEnum type);
    PATCHAPI_EXPORT PatchEdge *findTarget(ParseAPI::EdgeTypeEnum type);

    template <class OutputIterator> 
    void getFuncs(OutputIterator result);

    PATCHAPI_EXPORT Point *findPoint(Location loc, Point::Type type, bool create = true);

   PATCHAPI_EXPORT void remove(Point *);
   PATCHAPI_EXPORT PatchCallback *cb() const;

   PATCHAPI_EXPORT bool consistency() const;

   PATCHAPI_EXPORT bool wasUserAdded() const;

   PATCHAPI_EXPORT virtual void markModified()  {};

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



class PatchFunction {
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

     PATCHAPI_EXPORT static PatchFunction *create(ParseAPI::Function *, PatchObject*);
     PATCHAPI_EXPORT PatchFunction(ParseAPI::Function *f, PatchObject* o);
     PATCHAPI_EXPORT PatchFunction(const PatchFunction* parFunc, PatchObject* child);
     PATCHAPI_EXPORT virtual ~PatchFunction();

     const string &name() const { return func_->name(); }
     Address addr() const { return addr_;  }
     ParseAPI::Function *function() const { return func_; }
     PATCHAPI_EXPORT PatchObject *obj() const { return obj_; }

     PATCHAPI_EXPORT PatchBlock *entry();
     PATCHAPI_EXPORT const Blockset &blocks();
     PATCHAPI_EXPORT const Blockset &callBlocks();
     //PATCHAPI_EXPORT const Blockset &returnBlocks();
     PATCHAPI_EXPORT const Blockset &exitBlocks();

     PATCHAPI_EXPORT Point *findPoint(Location loc, Point::Type type, bool create = true);

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
     PATCHAPI_EXPORT bool findInsnPoints(Point::Type type, PatchBlock *block,
                                         InsnPoints::const_iterator &start,
                                         InsnPoints::const_iterator &end);

   PATCHAPI_EXPORT void remove(Point *);
   PATCHAPI_EXPORT PatchCallback *cb() const;

   PATCHAPI_EXPORT bool consistency() const;

   PATCHAPI_EXPORT virtual void markModified() {};

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
