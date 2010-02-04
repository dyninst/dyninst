// LGPL...

#include "CodeMover.h"
#include "relocation.h"
#include "Transformer.h"
#include "function.h" // bblInstance, int_basicBlock, int_function...

#include "InstructionDecoder.h" // for debug
#include "addressSpace.h" // Also for debug

#include "debug.h"

using namespace std;
using namespace Dyninst;
using namespace Relocation;
using namespace InstructionAPI;

CodeMover::Ptr CodeMover::create() {
  // Make a CodeMover
  Ptr ret = Ptr(new CodeMover());
  if (!ret) 
    return Ptr();

  return ret;
}  

template <typename BlockIter>
CodeMover::Ptr CodeMover::create(BlockIter begin,
				 BlockIter end) {
  Ptr ret = create();
  if (!ret) return Ptr();
  
  // Now for the hard part. We need to turn the vector of
  // int_basicBlocks into a vector of Relocation::Blocks.  Those take
  // bblInstances as inputs. I'm going to assume no multiple
  // relocation, and so we can just use the primary bblInstance for
  // each int_basicBlock.
  
  if (!ret->addBlocks(begin, end))
    return Ptr();
  
  return ret;
};


template <typename FuncIter>
CodeMover::Ptr CodeMover::createFunc(FuncIter begin, FuncIter end) {
  Ptr ret = create();
  if (!ret) return Ptr();

  // A vector of Functions is just an extended vector of basic blocks...
  for (; begin != end; ++begin) {
    const vector<int_basicBlock *> &blocks = (*begin)->blocks();
    if (!ret->addBlocks(blocks.begin(), blocks.end())) 
      return Ptr();
  }
  return ret;
}

template <typename BlockIter>
bool CodeMover::addBlocks(BlockIter begin, BlockIter end) {
  for (; begin != end; ++begin) {
    bblInstance *bbl = (*begin)->origInstance();
    relocation_cerr << "Creating Block for bbl at " 
		    << std::hex << bbl->firstInsnAddr() << std::dec
		    << endl;
    Block::Ptr block = Block::create(bbl);
    if (!block)
      return false;
    blocks_.push_back(block);
    blockMap_[bbl->firstInsnAddr()] = block;
    relocation_cerr << "  Updated block map: " 
		    << std::hex << bbl->firstInsnAddr() << std::dec
		    << "->" << block.get() << endl;

  }
  return true;
}

///////////////////////


bool CodeMover::transform(Transformer &t) {
  bool ret = true; 

  t.preprocess(blocks_);
  for (BlockList::iterator i = blocks_.begin(); 
       i != blocks_.end(); ++i) {
    if (!t.processBlock(i))
      ret = false;
  }
  t.postprocess(blocks_);

  return ret;
}

// And now the fun begins
// 
// We wish to minimize the space required by the relocated code. Since some platforms
// may have varying space requirements for certain instructions (e.g., branches) this
// requires a fixpoint calculation. We start with the original size and increase from
// there. 
// 
// Reasons for size increase:
//   1) Instrumentation. It tends to use room. Odd.
//   2) Transformed instructions. We may need to replace a single instruction with a
//      sequence to emulate its original behavior
//   3) Variable-sized instructions. If we increase branch displacements we may need
//      to increase the corresponding branch instruction sizes.

bool CodeMover::relocate(Address addr, const codeGen &t) {
  addr_ = addr;
  
  // Accumulate initial size guess
  // We'll expand this as necessary...
  unsigned size = 0;
  for (BlockList::iterator i = blocks_.begin(); 
       i != blocks_.end(); ++i) {
    size += (*i)->size();
  }

  gen_.setAddr(addr);
  gen_.applyTemplate(t);

  relocation_cerr << "Relocation: set addr to " 
		  << std::hex << addr << std::dec << " and applied template" << endl;

  relocation_cerr << "Verification: address is " << gen_.currAddr() << endl;

  int globalChanged = 0;
  int pass = 1;
  do {
    // Reset the flag...
    globalChanged = 0;

    cerr << "Pass" << pass;

    gen_.allocate(size);

    for (BlockList::iterator i = blocks_.begin(); 
	 i != blocks_.end(); ++i) {
      bool blockChanged = false;
      if (!(*i)->generate(gen_, blockChanged))
	return false; // Catastrophic failure
      if (blockChanged) {
	globalChanged++;
      }
      relocation_cerr << "    ... post generation, local " 
		      << blockChanged << " and global " 
		      << globalChanged << endl;
    }

    cerr << ", " << globalChanged << " blocks changed" << endl;
    
    pass++;
  } 
  while (globalChanged);

  // Update the entry map
  for (BlockList::iterator i = blocks_.begin(); 
       i != blocks_.end(); ++i) {
    entryMap_[(*i)->origAddr()] = (*i)->curAddr();
  }

  return true;
}

void CodeMover::disassemble() const {
  // InstructionAPI to the rescue!!!
  InstructionDecoder decoder((const unsigned char *)gen_.start_ptr(), 
			     gen_.used());
  Address addr = gen_.startAddr();

  decoder.setMode(gen_.addrSpace()->getAddressWidth() == 8);
  Instruction::Ptr cur = decoder.decode();
  while (cur && cur->isValid()) {
    cerr << std::hex << addr << std::dec << " " << cur->format() << endl;
    addr += cur->size();
    cur = decoder.decode();
  }
}

unsigned CodeMover::size() const {
  if (gen_.used()) return gen_.used();

  // Try and get an estimate of the block sizes.... ugh.
  unsigned size = 0;

  for (BlockList::const_iterator i = blocks_.begin(); 
       i != blocks_.end(); ++i) {
    size += (*i)->estimateSize();
  }
  return size;
}

///////////////////////

PriorityMap &CodeMover::priorityMap() {
  if (priorityMap_.empty()) {
    for (BlockList::iterator i = blocks_.begin(); 
	 i != blocks_.end(); ++i) {
      priorityMap_[(*i)->origAddr()] = Required;
    }
  }
  return priorityMap_;
}

///////////////////////

const CodeMover::PatchMap &CodeMover::patchMap() {
  // Take the current PriorityMap, digest it,
  // and return a sorted list of where we need 
  // patches (from and to)

  if (patchMap_.empty()) {
    for (PriorityMap::const_iterator iter = priorityMap_.begin();
	 iter != priorityMap_.end(); ++iter) {
      const Address &from = iter->first;
      const Priority &p = iter->second;
      // the priority map may include things not in the block
      // map...
      BlockMap::const_iterator b_iter = blockMap_.find(from);
      if (b_iter != blockMap_.end()) {
	const Address &to = b_iter->second->curAddr();
	patchMap_.priorities[p][from] = to;
      }
    }
  }

  return patchMap_;
}

string CodeMover::format() const {
  stringstream ret;
  
  ret << "CodeMover() {" << endl;

  for (BlockList::const_iterator iter = blocks_.begin();
       iter != blocks_.end(); ++iter) {
    ret << (*iter)->format();
  }
  ret << "}" << endl;
  return ret.str();

}


///////////////////
// STUUUUUPIDITY!!!!
///////////////////

void CodeMover::causeTemplateInstantiations() {
  set<int_basicBlock *> bset;
  vector<int_basicBlock *> bvec;

  create(bset.begin(), bset.end());
  create(bvec.begin(), bvec.end());

  set<int_function *> fset;
  vector<int_function *> fvec;
  
  createFunc(fset.begin(), fset.end());
  createFunc(fvec.begin(), fvec.end());
}
