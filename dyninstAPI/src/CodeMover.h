// TODO include LGPL

// CodeMover.h: worker classes for block and function relocation

#if !defined(_CODE_MOVER_H_)
#define _CODE_MOVER_H_

#include "dyn_detail/boost/shared_ptr.hpp"
#include "common/h/Types.h"
#include <list>
#include <map>
#include "arch.h" // codeGen structure

#include "Transformer.h"

class int_function;
class int_basicBlock;
class codeGen;

namespace Dyninst {
namespace Relocation {

class Block;
class Transformer;
class Analyzer;
class CodeMover;


class CodeMover {
 public:
  typedef dyn_detail::boost::shared_ptr<CodeMover> Ptr;
  typedef dyn_detail::boost::shared_ptr<Block> BlockPtr;
  typedef std::list<BlockPtr> BlockList;
  typedef std::map<Address, BlockPtr> BlockMap;
  class PatchMap {
    friend class CodeMover;
  public:
    PatchMap() {
      priorities = std::vector<Priority>(MaxPriority);
    }

    typedef std::map<Address, Address> Priority;
    
    const Priority &required() const { return priorities[Required]; }
    const Priority &suggested() const { return priorities[Suggested]; }
    const Priority &notRequired() const { return priorities[NotRequired]; }
    bool empty() const { 
      return required().empty() &&
	suggested().empty() &&
	notRequired().empty();
    }

  private:
    std::vector<Priority> priorities;
  };



  // A generic mover of code; an instruction, a basic block, or
  // a function. This is the algorithm (fixpoint) counterpart
  // of the classes described in relocation.h
  
  // Input: 
  //  A structured description of code in terms of an instruction, a
  //    block, a function, or a set of functions;
  //  A starting address for the moved code 
  //
  // Output: a buffer containing the moved code 

  static Ptr create();

  template<typename BlockIter>
    static Ptr create(BlockIter begin, BlockIter end);
 
  // Needs a different name to get the arguments
  // right.
  template<typename FuncIter>
    static Ptr createFunc(FuncIter begin, FuncIter end);

  static void causeTemplateInstantiations();

  bool analyze(Analyzer &a);

  // Apply the given Transformer to all blocks in the CodeMover
  bool transform(Transformer &t);
  
  // Allocates an internal buffer and relocates the code provided
  // to the constructor. Returns true for success or false for
  // catastrophic failure.
  // The codeGen parameter allows specification of various
  // codeGen-carried information
  bool relocate(Address addr,
		const codeGen &genTemplate);

  // Aaand debugging functionality
  void disassemble() const;


  // Get a map from original addresses to new addresses
  // for all blocks
  typedef std::map<Address, Address> EntryMap;
  const EntryMap &entryMap() { return entryMap_; }

  // Some things (LocalCFTransformer) require the
  // map of (original) addresses to BlockPtrs
  // so they can refer to blocks other than those
  // they are transforming.
  const BlockMap &blockMap() const { return blockMap_; }
  const PatchMap &patchMap();
  // Not const so that Transformers can modify it...
  PriorityMap &priorityMap();

  // Get either an estimate (pre-relocation) or actual
  // size
  unsigned size() const;

  // (void *) to start of code
  void *ptr() const { return gen_.start_ptr(); }

  std::string format() const;

 private:
    

  CodeMover() : addr_(0) {};

  
  void setAddr(Address &addr) { addr_ = addr; }
  template <typename BlockIter>
    bool addBlocks(BlockIter begin, BlockIter end);


  BlockList blocks_;
  // We also want to have a map from a bblInstance
  // to a Block so we can wire together jumps within
  // moved code
  BlockMap blockMap_;


  Address addr_;

  codeGen gen_;

  EntryMap entryMap_;

  PriorityMap priorityMap_;
  
  PatchMap patchMap_;
};

};

};

#endif
