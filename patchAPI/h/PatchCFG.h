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
class PatchObject;

/* PatchAPI Edge */
class PatchEdge {
   friend class PatchBlock;
   friend class PatchFunction;
   friend class PatchObject;

  public:
   static PatchEdge *create(ParseAPI::Edge *, PatchBlock *src, PatchBlock *trg);
   ~PatchEdge();

   // Getters
   ParseAPI::Edge *edge() const;
   PatchBlock *source();
   PatchBlock *target();
   ParseAPI::EdgeTypeEnum type() const;
   bool sinkEdge() const;
   bool interproc() const;

  protected:
    PatchEdge(ParseAPI::Edge *internalEdge,
              PatchBlock *source,
              PatchBlock *target);

    PatchEdge(const PatchEdge *parent,
              PatchBlock *child_src,
              PatchBlock *child_trg);

    ParseAPI::Edge *edge_;
    PatchBlock *src_;
    PatchBlock *trg_;
};

/* This is somewhat mangled, but allows PatchAPI to access the
   iteration predicates of ParseAPI without having to go back and
   template that code. Just wrap a ParseAPI predicate in a
   EdgePredicateAdapter and *poof* you're using PatchAPI edges
   instead of ParseAPI edges... */
class EdgePredicateAdapter
   : public ParseAPI::iterator_predicate <
  EdgePredicateAdapter,
  PatchEdge *,
  PatchEdge * > {
  public:
    EdgePredicateAdapter() : int_(NULL) {};
    EdgePredicateAdapter(ParseAPI::EdgePredicate *intPred) : int_(intPred) {};
    virtual ~EdgePredicateAdapter() {};
     virtual bool pred_impl(PatchEdge *e) const { return int_->pred_impl(e->edge()); };

  private:
    ParseAPI::EdgePredicate *int_;
};

/* PatchAPI Block */
class PatchBlock {
  friend class PatchEdge;
  friend class PatchFunction;
  friend class PatchObject;

  public:
    typedef std::map<Address, InstructionAPI::Instruction::Ptr> Insns;
    //typedef std::pair<Address, InstructionAPI::Instruction::Ptr> InsnInstance;
    //typedef std::vector<InsnInstance> InsnInstances;
    typedef ParseAPI::ContainerWrapper<
      std::vector<PatchEdge *>,
      PatchEdge *,
      PatchEdge *,
      EdgePredicateAdapter> edgelist;

    static PatchBlock *create(ParseAPI::Block *, PatchFunction *);
    virtual ~PatchBlock();

    // Getters
    Address start() const;
    Address end() const;
    Address last() const;
    Address size() const;

    bool isShared();
    int containingFuncs() const;
    void getInsns(Insns &insns) const;
    InstructionAPI::Instruction::Ptr getInsn(Address a) const;
    std::string disassemble() const;
    bool containsCall();
    bool containsDynamicCall();
    std::string format() const;

    // Difference between this layer and ParseAPI: per-function blocks.
    PatchFunction *function() const { return function_; }
    ParseAPI::Block *block() const { return block_; }
    PatchObject* object() const { return obj_; }
    edgelist &sources();
    edgelist &targets();

  protected:
    PatchBlock(ParseAPI::Block *block, PatchFunction *func);
    PatchBlock(const PatchBlock *parblk, PatchObject *child);
    PatchBlock(ParseAPI::Block *block, PatchObject *obj);

    typedef enum {
      backwards,
      forwards } Direction;

    void removeSourceEdge(PatchEdge *e);
    void removeTargetEdge(PatchEdge *e);
    void createInterproceduralEdges(ParseAPI::Edge *, Direction dir,
                                    std::vector<PatchEdge *> &);

    ParseAPI::Block *block_;
    PatchFunction *function_;
    std::vector<PatchEdge *> srcs_;
    std::vector<PatchEdge *> trgs_;
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
   typedef PatchBlock::edgelist edgelist;

   static PatchFunction *create(ParseAPI::Function *, PatchObject*);
   virtual ~PatchFunction();

   const string &name() { return func_->name(); }
   Address addr() const { return addr_;  }
   ParseAPI::Function *function() { return func_; }
   PatchBlock *entry() { return getBlock(func_->entry()); }
   PatchObject* object() { return obj_; }

   const blocklist &blocks();
   //   const edgelist &callEdges();
   const blocklist &returnBlocks();

   //   bool entries(PointSet& pts);
   //   bool exits(PointSet& pts);

   PatchBlock *getBlock(ParseAPI::Block *);
   void addBlock(PatchBlock*);
   PatchEdge *getEdge(ParseAPI::Edge *, PatchBlock *src, PatchBlock *trg);

   PatchFunction(ParseAPI::Function *f, PatchObject* o);
   PatchFunction(const PatchFunction* parFunc, PatchObject* child);

 protected:
   void removeEdge(PatchEdge *e);

   ParseAPI::Function *func_;
   PatchObject* obj_;
   Address addr_;

   std::vector<PatchBlock *> blocks_;
   std::vector<PatchEdge *> callEdges_;
   std::vector<PatchBlock *> returnBlocks_;

   //edgelist callEdgeList_;

   typedef std::map<ParseAPI::Block *, PatchBlock *> BlockMap;
   BlockMap blockMap_;

   typedef std::map<ParseAPI::Edge *, PatchEdge *> EdgeMap;
   EdgeMap edgeMap_;

};

};
};


#endif /* _PATCHAPI_DYNINST_CFG_H_ */
