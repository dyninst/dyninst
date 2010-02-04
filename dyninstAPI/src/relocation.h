// Include LGPL shtuff

#if !defined(_RELOCATION_H_)
#define _RELOCATION_H_

#include "dyn_detail/boost/shared_ptr.hpp" // shared_ptr
#include "common/h/Types.h" // Address
#include "arch.h" // codeGen
#include "Instruction.h" // Instruction::Ptr

#include <list> // stl::list
#include "dyninstAPI/src/function.h" // bblInstance

class baseTramp;
class bblInstance;

namespace Dyninst {
namespace Relocation {

class Transformer;
class Element;
class RelocInsn;
class Inst;
class CFElement;
class Block;


// Wraps an object that can serve as a  control flow target. This
// may include existing code objects (a block or function)
// or something that has been relocated. We wrap them with this
// template class (which will then be specialized as appropriate)
// so we don't pollute the base class with extraneous code (I'm
// looking at _you_, get_address_cr....
//
// Preliminary requirement: T must be persistent during the existence
// of this class so we can use a reference to it. 
class TargetInt {
 public:
  virtual Address addr() const { return 0; }
  virtual bool valid() const { return false; }
  TargetInt() {};
  virtual ~TargetInt() {};
};

template <typename T>
class Target : public TargetInt{
 public:
  Address addr() const;
  bool valid() const;
 Target(const T t) : t_(t) {};
  ~Target() {};

  const T t() { return t_; }

 private:
  const T &t_;
};

class Analyzer {
 public:
  bool apply(Element &);
  bool apply(RelocInsn &);
  bool apply(CFElement &);
  bool apply(Inst &);
};

// Base code generation class
class Element {
  friend class Transformer;
 public:
  typedef dyn_detail::boost::shared_ptr<Element> Ptr;

  Element() {};

  // A default value to make sure things don't go wonky.
  virtual Address addr() const { return 0; }
  virtual InstructionAPI::Instruction::Ptr insn() const {
    return InstructionAPI::Instruction::Ptr();
  }

  bool apply(Analyzer &a) { 
    return a.apply(*this);
  };
  
  // Make binary from the thing
  // Current address (if we need it)
  // is in the codeGen object.
  virtual bool generate(codeGen &gen) = 0;

  virtual std::string format() const = 0;

  virtual ~Element() {};
};

// FIXMEEEEEE
typedef int Sensitivity;

class RelocInsn : public Element {
 public:
  typedef dyn_detail::boost::shared_ptr<RelocInsn> Ptr;

  virtual bool generate(codeGen &gen);

  static Ptr create(InstructionAPI::Instruction::Ptr insn,
		    Address addr);

  const Sensitivity &sens() const { return sens_; }

  virtual ~RelocInsn() {};

  bool apply(Analyzer &a) { 
    return a.apply(*this);
  };

  virtual std::string format() const;

  virtual InstructionAPI::Instruction::Ptr insn() const { return insn_; }
  virtual Address addr() const { return addr_; }

 private:
  RelocInsn(InstructionAPI::Instruction::Ptr insn,
	    Address addr) : 
  insn_(insn), addr_(addr), sens_(0) {};

  // Pointer to the instruction we represent
  InstructionAPI::Instruction::Ptr insn_;

  // Original address of this instruction
  Address addr_;

  // Sensitivity representation
  Sensitivity sens_;
};

 class LocalCFTransformer;

class CFElement : public Element {
  friend class Transformer;
  friend class LocalCFTransformer;
  friend class InstTransformer; // For rewiring edge instrumentation
  friend class PCRelTransformer; // Also
 public:
  static const Address Fallthrough;
  static const Address Taken;

  typedef dyn_detail::boost::shared_ptr<CFElement> Ptr;
  typedef std::map<Address, TargetInt *> DestinationMap;

  bool generate(codeGen &gen);

  // Factory function... we create these first,
  // then fill them in.
  static Ptr create();
		    
  void updateInsn(InstructionAPI::Instruction::Ptr insn);
  void updateAddr(Address addr);

  void setCall() { isCall_ = true; };
  void setConditional() { isConditional_ = true; };
  void setIndirect() { isIndirect_ = true; };

  virtual ~CFElement();

  // Owns the provided *dest parameter
  void addDestination(Address index, TargetInt *dest);

  bool apply(Analyzer &a) { 
    return a.apply(*this);
  };

  virtual std::string format() const;

  virtual Address addr() const { return addr_; }
  virtual InstructionAPI::Instruction::Ptr insn() const { return insn_; }

 private:
  CFElement() :
    isCall_(false),
    isConditional_(false),
    isIndirect_(false),
    addr_(0) {};

  bool isCall_;
  bool isConditional_;
  bool isIndirect_;

  InstructionAPI::Instruction::Ptr insn_;
  Address addr_;

  // An expression that represents how the PC is determined
  // Should be a single register, but who are we to judge?
  InstructionAPI::Expression::Ptr targetExpr_;

  // A map from input values (for some representation of input
  // values) to Targets
  // Used during code generation to determine whether we
  // require some form of address translation. We currently have
  // two cases: conditional and indirect control flow.
  //  Conditional: <true> -> taken target; <false> -> fallthrough target
  //  Indirect: <original address> -> corresponding target
  // TBD: PPC has conditional indirect control flow, so we may want
  // to split these up.
  DestinationMap destMap_;

  //
  // These should move to a CodeGenerator class or something...
  // But for now they can go here
  // The Instruction input allows pulling out ancillary data (e.g.,
  // conditions, prediction, etc.
  static bool generateBranch(codeGen &gen,
			     Address from,
			     TargetInt *to,
			     InstructionAPI::Instruction::Ptr insn,
			     bool fallthrough);
  static bool generateCall(codeGen &gen,
			   Address from,
			   TargetInt *to,
			   InstructionAPI::Instruction::Ptr insn);
  static bool generateConditionalBranch(codeGen &gen,
					Address from,
					TargetInt *to,
					InstructionAPI::Instruction::Ptr insn);
  // The Register holds the translated destination (if any)
  // TODO replace with the register IDs that Bill's building
  typedef unsigned Register;
  static bool generateIndirect(codeGen &gen,
			       Register reg,
			       InstructionAPI::Instruction::Ptr insn);
  static bool generateIndirectCall(codeGen &gen,
				   Register reg,
				   InstructionAPI::Instruction::Ptr insn);
  
  bool generateAddressTranslator(codeGen &gen,
				 Register &reg);  
};


class Inst : public Element {
 public:
  typedef dyn_detail::boost::shared_ptr<Inst> Ptr;

  // I believe I can patch in the current code generation
  // system here...
  static Ptr create();

  Inst() {};

  // This sucks. It seriously sucks. But hey...
  // this points to all the baseTramps with instrumentation
  // at this point. This can be 0, 1, or 2 - 2 if we have
  // post instruction + pre instruction instrumentation.

  void addBaseTramp(baseTramp *b);
  bool empty() const;

  bool generate(codeGen &gen);
  
  virtual ~Inst() {};

  bool apply(Analyzer &a) { 
    return a.apply(*this);
  };

  virtual std::string format() const;

 private:

  std::list<baseTramp *> baseTramps_;

};

// Represents generation for a PC-relative
// memory load/store

class PCRelativeData : public Element {
 public:
   typedef dyn_detail::boost::shared_ptr<PCRelativeData> Ptr;

   virtual bool generate(codeGen &gen);
   static Ptr create(InstructionAPI::Instruction::Ptr insn,
		     Address addr,
		     Address target);

   virtual ~PCRelativeData() {};

   virtual std::string format() const;

 private:
   PCRelativeData(InstructionAPI::Instruction::Ptr insn,
	       Address addr,
	       Address target) : insn_(insn), addr_(addr), target_(target) {};

   InstructionAPI::Instruction::Ptr insn_;
   Address addr_;
   Address target_;
   // Read vs. write doesn't matter now but might
   // in the future.
};

class GetPC : public Element {
 public:
   typedef dyn_detail::boost::shared_ptr<GetPC> Ptr;

   static Ptr create(InstructionAPI::Instruction::Ptr insn,
		     Address addr,
		     bool stackTop,
		     unsigned reg);
   virtual bool generate(codeGen &gen);
   virtual ~GetPC() {};
   virtual std::string format() const;

 private:
   GetPC(InstructionAPI::Instruction::Ptr insn,
	 Address addr,
	 bool isStack,
	 unsigned reg) : 
   insn_(insn), 
     addr_(addr), 
     isStack_(isStack),
     reg_(reg) {};

   bool PCtoStack(codeGen &gen);
   bool PCtoReg(codeGen &gen);

   InstructionAPI::Instruction::Ptr insn_;
   Address addr_;
   bool isStack_;
   unsigned reg_;

};

class Block {
 public:
  friend class Transformer;

  typedef std::list<Element::Ptr> ElementList;
  typedef dyn_detail::boost::shared_ptr<Block> Ptr;

  // Creation via a known bblInstance
  static Ptr create(bblInstance *inst);

  // Creation via a single BaseTramp;
  static Ptr create(baseTramp *base);
  
  // Sensitivity analysis.
  bool analyze(Analyzer &a);

  // Transform the block in some way. 
  // In a slight modification of the typical visitor pattern
  // we hand the transformer the instruction list; it then
  // modifies it to suit itself. This can include substituting
  // instructions or replacing entire groups of instructions with
  // something else (another Element).
  //
  // Also, instrumentation
  bool transform(Transformer &t);

  // Generate code for this block
  // Return: whether generation was successful
  // gen: a codeGen object (AKA wrapper for all sorts
  //      of state we need)
  // changed: whether something in this block changed
  //          necessitating regeneration of the container
  bool generate(codeGen &gen,
		bool &changed);

  Address curAddr() const { return curAddr_; }
  unsigned size() const { return size_; }
  Address origAddr() const { return origAddr_; }

  // Non-const for use by transformer classes
  ElementList &elements() { return elements_; }

  // TODO get a more appropriate estimate...
  unsigned estimateSize() const { return size_; }

  std::string format() const;

  const bblInstance *bbl() const { return bbl_; }

 private:

  Block(bblInstance *bbl) :
  curAddr_(0),
    size_(bbl->getSize()),
    origAddr_(bbl->firstInsnAddr()),
    bbl_(bbl) {};
 Block(baseTramp *) :
  curAddr_(0),
    size_(0), // Should estimate here
    origAddr_(0), // No original address...
    bbl_(NULL) {};


  typedef std::pair<InstructionAPI::Instruction::Ptr, Address> InsnInstance;
  typedef std::vector<InsnInstance> InsnVec;

  // Raw creation method via list of instructions + manually specified
  // post-block control flow:
  static Ptr create(const InsnVec &insns,
		    CFElement::Ptr end);

  // Analyze the block ender and create a logical control flow
  // construct matching it. 
  bool createBlockEnd();

  ElementList elements_;

  Address curAddr_;
  unsigned size_;
  Address origAddr_;

  bblInstance *bbl_;
};


template <>
  class Target<Block::Ptr> : public TargetInt {
 public:
  Address addr() const { 
    return t_->curAddr(); 
  }
  bool valid() const { return t_->curAddr() != 0; }
 Target(Block::Ptr t) : t_(t) {}
  ~Target() {}
  const Block::Ptr &t() const { return t_; };

 private:
  const Block::Ptr t_;
};

template <>
class Target<const bblInstance *> : public TargetInt {
 public:
  Address addr() const { 
    return t_->firstInsnAddr(); 
  }
  bool valid() const { return true; }
 Target(const bblInstance *t) : t_(t) {}
  ~Target() {}

  const bblInstance *t() const { return t_; };

 private:
  const bblInstance *t_;
};


// This is a standin for a "PLT Entry" type
typedef Address PLT_Entry;
template <>
class Target<PLT_Entry> : public TargetInt {
 public:
  Address addr() const { return t_; }
  bool valid() const { return true; }
 Target(PLT_Entry t) : t_(t) {}
  ~Target() {}
  const PLT_Entry &t() const { return t_; }

 private:
  const PLT_Entry t_;
};



};
};
#endif
