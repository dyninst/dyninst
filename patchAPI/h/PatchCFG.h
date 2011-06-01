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
//class PatchObject;

class PatchEdge {
   friend class PatchBlock;
   friend class PatchFunction;
   friend class PatchObject;

  public:
   PATCHAPI_EXPORT static PatchEdge *create(ParseAPI::Edge *,
                                            PatchBlock *src,
                                            PatchBlock *trg);
   PatchEdge(ParseAPI::Edge *internalEdge,
             PatchBlock *source,
             PatchBlock *target);
   PatchEdge(const PatchEdge *parent,
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
    PatchBlock(const PatchBlock *parblk, PatchObject *child);
    PatchBlock(ParseAPI::Block *block, PatchObject *obj);
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

    // Difference between this layer and ParseAPI: per-function blocks.
    PATCHAPI_EXPORT PatchFunction *function() const { return function_; }
    PATCHAPI_EXPORT ParseAPI::Block *block() const { return block_; }
    PATCHAPI_EXPORT PatchObject* object() const { return obj_; }
    PATCHAPI_EXPORT edgelist &getSources();
    PATCHAPI_EXPORT edgelist &getTargets();

    template <class OutputIterator>
    PATCHAPI_EXPORT void getFunctions(OutputIterator result);

  protected:
    typedef enum {
      backwards,
      forwards } Direction;

    void removeSourceEdge(PatchEdge *e);
    void removeTargetEdge(PatchEdge *e);

    ParseAPI::Block *block_;
    PatchFunction *function_;
    edgelist srclist_;
    edgelist trglist_;
    PatchObject* obj_;
};


/* PatchAPI Function */
class PatchFunction {
   friend class PatchEdge;
   friend class PatchBlock;
   friend class PatchObject;

   public:
     typedef std::vector<PatchBlock *> blocklist;

     static PatchFunction *create(ParseAPI::Function *, PatchObject*);
     PatchFunction(ParseAPI::Function *f, PatchObject* o);
     PatchFunction(const PatchFunction* parFunc, PatchObject* child);
     virtual ~PatchFunction();

     const string &name() { return func_->name(); }
     Address addr() const { return addr_;  }
     ParseAPI::Function *function() { return func_; }
     PatchObject* object() { return obj_; }

     const blocklist &getAllBlocks();
     PatchBlock *getEntryBlock();
     const blocklist &getExitBlocks();
     const blocklist &getCallBlocks();

   protected:
     ParseAPI::Function *func_;
     PatchObject* obj_;
     Address addr_;

     blocklist all_blocks_;
     blocklist exit_blocks_;
     blocklist call_blocks_;
};

template <class OutputIterator>
void PatchBlock::getFunctions(OutputIterator result) {
  std::vector<ParseAPI::Function *> pFuncs;
  block()->getFuncs(pFuncs);
  for (unsigned i = 0; i < pFuncs.size(); ++i) {
    //PatchFunction *func = function()->object()->getFunction(pFuncs[i]);
    PatchFunction *func = getFunction(pFuncs[i]);
    *result = func;
    ++result;
  }
}

};
};


#endif /* _PATCHAPI_DYNINST_CFG_H_ */
