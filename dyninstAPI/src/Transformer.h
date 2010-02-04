// LGPL...

#if !defined(_TRANSFORMER_H_)
#define _TRANSFORMER_H_

#include "dyn_detail/boost/shared_ptr.hpp" // shared_ptr
#include "common/h/Types.h" // Address
#include <list>
#include <map>

#include "image-func.h" // EdgeTypeEnum

class bblInstance;
class baseTramp;

namespace Dyninst {
namespace Relocation {

class Element;
class Block;
class TargetInt;
 class CFElement;

// One of the things a Transformer 'returns' (modifies, really) is 
// a list of where we require patches (branches from original code
// to new code). This list is prioritized - Required, 
// Suggested, and Not Required. Required means we have proof
// that a patch is necessary for correct control flow. Suggested means
// that, assuming correct parsing, no patch is necessary. Not Required
// means that even with incorrect parsing no patch is necessary.
// ... not sure how we can have that, but hey, we might as well
// design it in.

typedef enum {
  NotRequired,
  Suggested,
  Required,
  MaxPriority } Priority;
 
// Address here is typically original address...
typedef std::map<Address, Priority> PriorityMap;

class Transformer {
 public:
  typedef dyn_detail::boost::shared_ptr<Element> ElementPtr;
  typedef std::list<ElementPtr> ElementList;
  typedef dyn_detail::boost::shared_ptr<Block> BlockPtr;
  typedef std::list<BlockPtr> BlockList;

  virtual bool preprocess(BlockList &) { return true;}
  virtual bool postprocess(BlockList &) { return true; }

  virtual bool processBlock(BlockList::iterator &) { return true; }

  virtual ~Transformer() {};
};

// Ensure that each Block ends with a CFElement; 
// if the last instruction in the Block is an explicit
// control transfer we replace it with a CFElement.
// Otherwise we append a new one.
class CFElementTransformer : public Transformer {
 public:
  virtual bool processBlock(BlockList::iterator &);

  CFElementTransformer() {};

 private:

  typedef std::pair<TargetInt *, EdgeTypeEnum> Succ;
  typedef std::vector<Succ> SuccVec;

  // Determine who the successors of a block are
  static void getInterproceduralSuccessors(const bblInstance *inst,
					   SuccVec &succ);

};

// Rewire a set of Blocks so that we do control
// transfers within them rather than back to 
// original code. 
//
// As a side note, track how many edges we rewire
// in this way so we can tell where we'll need
// patch branches.
class LocalCFTransformer : public Transformer {
 public:
  /// Mimics typedefs in CodeMover.h, but I don't want
  // to include that file.
  typedef std::map<Address, BlockPtr> BlockMap;

  virtual bool processBlock(BlockList::iterator &);
  virtual bool postprocess(BlockList &); 

 LocalCFTransformer(const BlockMap &bmap, PriorityMap &p) : 
  bMap_(bmap), pMap_(p) {};

 private:
  int getInEdgeCount(const bblInstance *inst);
  void recordIncomingEdges(const TargetInt *);

  // Borrowed from the CodeMover, we don't change it
  const BlockMap &bMap_;
  // And the priority list that we modify
  PriorityMap &pMap_;

  std::map<Address, int> replacedCount_;
  std::map<Address, int> incomingCount_;
};

class RelocInsn;
// Identify PC-relative memory accesses and replace
// them with a dedicated Element
class PCRelTransformer : public Transformer {
  typedef dyn_detail::boost::shared_ptr<RelocInsn> RelocInsnPtr;
 public:
  virtual bool processBlock(BlockList::iterator &);

 PCRelTransformer(AddressSpace *as) : addrSpace(as) {};

 private:
  bool isPCRelData(ElementPtr ptr,
		   Address &target);
  // Records where PC was stored
  bool isGetPC(ElementPtr ptr,
	       bool &isStack,
	       unsigned &reg);
  // Used for finding call targets
  AddressSpace *addrSpace;
};


class InstTransformer : public Transformer {
 public:
  virtual bool processBlock(BlockList::iterator &);
  virtual bool postprocess(BlockList &l);

  InstTransformer() {};


 private:
  typedef enum {
    Before,
    After } When;
    
  typedef std::pair<BlockPtr, When> InsertPoint;  
  typedef std::map<InsertPoint, BlockList> EdgeBlocks;
  typedef dyn_detail::boost::shared_ptr<CFElement> CFElementPtr;

  EdgeBlocks edgeBlocks_;

  bool addEdgeInstrumentation(baseTramp *tramp,
			      CFElementPtr cf,
			      Address dest,
			      BlockPtr cur);


};

};
};

#endif
