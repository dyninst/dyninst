#include "parse-cfg.h"
#include "parseAPI/h/CFG.h"
#include "instPoint.h" // For instPoint::Type

#if !defined(_DYN_BLOCK_H_)
#define _DYN_BLOCK_H_

class block_instance;
class func_instance;
class BPatch_edge;

class edge_instance {
   friend class block_instance;
   friend class func_instance;

  public:
   static edge_instance *create(ParseAPI::Edge *, block_instance *src, block_instance *trg);
   static void destroy(edge_instance *);


   ParseAPI::Edge *edge() const { return edge_; }
   block_instance *src();
   block_instance *trg();
   ParseAPI::EdgeTypeEnum type() const { return edge_->type(); }

   bool sinkEdge() const { return edge_->sinkEdge(); }
   bool interproc() const { return edge_->interproc() || 
         (edge_->type() == ParseAPI::CALL) || 
         (edge_->type() == ParseAPI::RET); }

   instPoint *point();
   instPoint *findPoint(instPoint::Type type);

   AddressSpace *proc();
   func_instance *func();

   BPatch_edge *bpedge() { return bpEdge_; }
   void setBPEdge(BPatch_edge *e) { bpEdge_ = e; }

  private:
   edge_instance(ParseAPI::Edge *edge, block_instance *src, block_instance *trg);
   ~edge_instance();

   ParseAPI::Edge *edge_;
   block_instance *src_;
   block_instance *trg_;
   
   BPatch_edge *bpEdge_;

};

// This is somewhat mangled, but allows Dyninst to access the
// iteration predicates of Dyninst without having to go back and
// template that code. Just wrap a ParseAPI predicate in a
// EdgePredicateAdapter and *poof* you're using edge_instances
// instead of ParseAPI edges...

class EdgePredicateAdapter 
   : public ParseAPI::iterator_predicate <
  EdgePredicateAdapter,
  edge_instance *,
  edge_instance * > {
  public:
  EdgePredicateAdapter() : int_(NULL) {};
  EdgePredicateAdapter(ParseAPI::EdgePredicate *intPred) : int_(intPred) {};
   virtual ~EdgePredicateAdapter() {};
   virtual bool pred_impl(edge_instance *e) const { return int_->pred_impl(e->edge()); };

  private:
   ParseAPI::EdgePredicate *int_;
};

class block_instance {
    friend class func_instance;

 public:
    typedef ParseAPI::ContainerWrapper<
       std::vector<edge_instance *>,
       edge_instance *,
       edge_instance *,
       EdgePredicateAdapter> edgelist;


    block_instance(parse_block *ib, func_instance *func);
    block_instance(const block_instance *parent, func_instance *func);
    ~block_instance();

    // "Basic" block stuff
    Address start() const;
    Address end() const;
    Address last() const;
    unsigned size() const;

    // Up-accessors
    func_instance *func() const;
    mapped_object *obj() const;
    AddressSpace *addrSpace() const;
    AddressSpace *proc() const { return addrSpace(); }

    // just because a block is an entry block doesn't mean it is
    // an entry block that this block's function cares about
    bool isEntry() const;
    bool isExit() const { return block_->isExitBlock(); }
    //bool isReturnBlock() const;

    // block_instances are not shared, but their underlying blocks
    // may be
    bool hasSharedBase() const { return block_->isShared(); }

    void triggerModified();

    parse_block * llb() const { return block_; }
    
    struct compare {
        bool operator()(block_instance * const &b1,
                        block_instance * const &b2) const {
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
    block_instance *getTarget();
    block_instance *getFallthrough();
    func_instance *callee();
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
    func_instance *func_;
    parse_block *block_;

    std::vector<edge_instance *> srcs_;
    std::vector<edge_instance *> trgs_;

    edgelist srclist_;
    edgelist trglist_;

    void createInterproceduralEdges(ParseAPI::Edge *, bool forward, 
                                    std::vector<edge_instance *> &edges);
};

#endif
