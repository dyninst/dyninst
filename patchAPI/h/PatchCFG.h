/* Public Interface */

#ifndef _PATCHAPI_DYNINST_CFG_H_
#define _PATCHAPI_DYNINST_CFG_H_

#include "common.h"
#include "PatchObject.h"

namespace Dyninst {
namespace PatchAPI {

class PatchEdge;
class PatchBlock;
class PatchFunction;

class PatchEdge {
   friend class PatchBlock;
   friend class PatchFunction;
   friend class PatchObject;

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
   PATCHAPI_EXPORT PatchBlock *source();
   PATCHAPI_EXPORT PatchBlock *target();
   PATCHAPI_EXPORT ParseAPI::EdgeTypeEnum type() const;
   PATCHAPI_EXPORT bool sinkEdge() const;
   PATCHAPI_EXPORT bool interproc() const;

 protected:
    ParseAPI::Edge *edge_;
    PatchBlock *src_;
    PatchBlock *trg_;
};

class PatchBlock {
  friend class PatchEdge;
  friend class PatchFunction;
  friend class PatchObject;

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
    PATCHAPI_EXPORT bool containsCall();
    PATCHAPI_EXPORT bool containsDynamicCall();
    PATCHAPI_EXPORT std::string format() const;
    PATCHAPI_EXPORT PatchFunction* getCallee();

    PATCHAPI_EXPORT ParseAPI::Block *block() const;
    PATCHAPI_EXPORT PatchObject* object() const;
    PATCHAPI_EXPORT edgelist &getSources();
    PATCHAPI_EXPORT edgelist &getTargets();

    template <class OutputIterator>
    void getFunctions(OutputIterator result);

  protected:
    typedef enum {
      backwards,
      forwards } Direction;

    void removeSourceEdge(PatchEdge *e);
    void removeTargetEdge(PatchEdge *e);

    ParseAPI::Block *block_;
    edgelist srclist_;
    edgelist trglist_;
    PatchObject* obj_;
};



class PatchFunction {
   friend class PatchEdge;
   friend class PatchBlock;
   friend class PatchObject;

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
     typedef std::set<PatchBlock *, compare> blockset;

     PATCHAPI_EXPORT static PatchFunction *create(ParseAPI::Function *, PatchObject*);
     PATCHAPI_EXPORT PatchFunction(ParseAPI::Function *f, PatchObject* o);
     PATCHAPI_EXPORT PatchFunction(const PatchFunction* parFunc, PatchObject* child);
     PATCHAPI_EXPORT virtual ~PatchFunction();

     const string &name() { return func_->name(); }
     Address addr() const { return addr_;  }
     ParseAPI::Function *function() { return func_; }
     PatchObject* object() { return obj_; }

     PATCHAPI_EXPORT const blockset &getAllBlocks();
     PATCHAPI_EXPORT PatchBlock *getEntryBlock();
     PATCHAPI_EXPORT const blockset &getExitBlocks();
     PATCHAPI_EXPORT const blockset &getCallBlocks();

   protected:
     ParseAPI::Function *func_;
     PatchObject* obj_;
     Address addr_;

     blockset all_blocks_;
     blockset exit_blocks_;
     blockset call_blocks_;
};

template <class OutputIterator>
void PatchBlock::getFunctions(OutputIterator result) {
  std::vector<ParseAPI::Function *> pFuncs;
  block()->getFuncs(pFuncs);
  for (unsigned i = 0; i < pFuncs.size(); ++i) {
    PatchFunction *func = getFunction(pFuncs[i]);
    *result = func;
    ++result;
  }
}

};
};


#endif /* _PATCHAPI_DYNINST_CFG_H_ */
