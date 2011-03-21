#include "image-func.h"
#include "parseAPI/h/CFG.h"
#include "instPoint.h" // For instPoint::Type

#if !defined(_DYN_BLOCK_H_)
#define _DYN_BLOCK_H_

class int_block;
class int_function;
class BPatch_edge;

class int_edge {
   friend class int_block;
   friend class int_function;

  public:
   static int_edge *create(ParseAPI::Edge *, int_block *src, int_block *trg);
   static void destroy(int_edge *);
   
   ParseAPI::Edge *edge() const { return edge_; }
   int_block *src();
   int_block *trg();
   ParseAPI::EdgeTypeEnum type() const { return edge_->type(); }

   bool sinkEdge() const { return edge_->sinkEdge(); }
   bool interproc() const { return edge_->interproc() || 
         (edge_->type() == ParseAPI::CALL) || 
         (edge_->type() == ParseAPI::RET); }

   instPoint *point();
   instPoint *findPoint(instPoint::Type type);

   AddressSpace *proc();
   int_function *func();

   BPatch_edge *bpedge() { return bpEdge_; }
   void setBPEdge(BPatch_edge *e) { bpEdge_ = e; }

  private:
   int_edge(ParseAPI::Edge *edge, int_block *src, int_block *trg);
   ~int_edge();

   ParseAPI::Edge *edge_;
   int_block *src_;
   int_block *trg_;
   
   BPatch_edge *bpEdge_;

};

// This is somewhat mangled, but allows Dyninst to access the
// iteration predicates of Dyninst without having to go back and
// template that code. Just wrap a ParseAPI predicate in a
// EdgePredicateAdapter and *poof* you're using int_edges
// instead of ParseAPI edges...

class EdgePredicateAdapter 
   : public ParseAPI::iterator_predicate <
  EdgePredicateAdapter,
  int_edge *,
  int_edge * > {
  public:
  EdgePredicateAdapter() : int_(NULL) {};
  EdgePredicateAdapter(ParseAPI::EdgePredicate *intPred) : int_(intPred) {};
   virtual ~EdgePredicateAdapter() {};
   virtual bool pred_impl(int_edge *e) const { return int_->pred_impl(e->edge()); };

  private:
   ParseAPI::EdgePredicate *int_;
};

class int_block {
    friend class int_function;

 public:
    typedef ParseAPI::ContainerWrapper<
       std::vector<int_edge *>,
       int_edge *,
       int_edge *,
       EdgePredicateAdapter> edgelist;


    int_block(image_basicBlock *ib, int_function *func);
    int_block(const int_block *parent, int_function *func);
    ~int_block();

    // "Basic" block stuff
    Address start() const;
    Address end() const;
    Address last() const;
    unsigned size() const;

    // Up-accessors
    int_function *func() const;
    mapped_object *obj() const;
    AddressSpace *addrSpace() const;
    AddressSpace *proc() const { return addrSpace(); }

    // just because a block is an entry block doesn't mean it is
    // an entry block that this block's function cares about
    bool isEntry() const;
    bool isExit() const { return block_->isExitBlock(); }
    //bool isReturnBlock() const;

    // int_blocks are not shared, but their underlying blocks
    // may be
    bool hasSharedBase() const { return block_->isShared(); }

    void triggerModified();

    image_basicBlock * llb() const { return block_; }
    
    struct compare {
        bool operator()(int_block * const &b1,
                        int_block * const &b2) const {
            if(b1->start() < b2->start()) return true;
            if(b2->start() < b1->start()) return false;
            assert(b1 == b2);
            return false;
        }
    };

    std::string format() const;

    const edgelist &sources();
    const edgelist &targets();

    // Shortcuts
    int_block *getTarget();
    int_block *getFallthrough();
    int_function *callee();
    std::string calleeName(); // 

    // IIIIINSTPOINTS!
    instPoint *entryPoint();
    instPoint *preCallPoint();
    instPoint *postCallPoint();
    instPoint *preInsnPoint(Address addr);
    instPoint *postInsnPoint(Address addr);

    // "Trusted" versions of the above that don't verify the address,
    // for when we've just finished iterating
    instPoint *preInsnPoint(Address addr, InstructionAPI::Instruction::Ptr insn);
    instPoint *postInsnPoint(Address addr, InstructionAPI::Instruction::Ptr insn);

    // Non-creating lookup methods
    instPoint *findPoint(instPoint::Type type);
    const std::map<Address, instPoint *> &findPoints(instPoint::Type type);
    
#if defined(cap_instruction_api)
    // TODO: this should be a map from addr to insn, really
    typedef std::pair<InstructionAPI::Instruction::Ptr, Address> InsnInstance;
    typedef std::vector<InsnInstance> InsnInstances;
    void getInsns(InsnInstances &instances) const;
    std::string disassemble() const;
#endif

    void *getPtrToInstruction(Address addr) const;


    bool containsCall();
    bool containsDynamicCall();

    int id() const;

    void setHighLevelBlock(BPatch_basicBlock *newb);
    BPatch_basicBlock *getHighLevelBlock() const;

 private:
    BPatch_basicBlock *highlevel_block;
    int_function *func_;
    image_basicBlock *block_;

    std::vector<int_edge *> srcs_;
    std::vector<int_edge *> trgs_;

    edgelist srclist_;
    edgelist trglist_;

    void createInterproceduralEdges(ParseAPI::Edge *, bool forward, 
                                    std::vector<int_edge *> &edges);
};

#endif
