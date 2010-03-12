

#if !defined(Absloc_Interface_H)
#define Absloc_Interface_H

#include "Instruction.h"
#include "Register.h"
#include "Expression.h"
#include "Operand.h"
#include "Absloc.h"

class image_func;
class int_function;
class BPatch_function;

namespace Dyninst {

class AbsRegionConverter {
 public:
 AbsRegionConverter(bool cache) : 
  cacheEnabled_(cache) {};

  // Definition: the first AbsRegion represents the expression.
  // If it's a memory reference, any other AbsRegions represent
  // registers used in this expression.

  void convertAll(InstructionAPI::Expression::Ptr expr,
		  Address addr,
		  image_func *func,
		  std::vector<AbsRegion> &regions);

  void convertAll(InstructionAPI::Instruction::Ptr insn,
		  Address addr,
		  image_func *func,
		  std::vector<AbsRegion> &used,
		  std::vector<AbsRegion> &defined);

  // Single converters
  
  AbsRegion convert(InstructionAPI::RegisterAST::Ptr reg);

  AbsRegion convert(InstructionAPI::Expression::Ptr expr,
		    Address addr,
		    image_func *func);

  // Cons up a stack reference at the current addr
  AbsRegion stack(Address addr,
		  image_func *func,
		  bool push);

 private:
    // Returns false if the current height is unknown.
  bool getCurrentStackHeight(image_func *func,
			     Address addr, 
			     long &height, int &region);
  bool getCurrentFrameHeight(image_func *func,
			     Address addr, 
			     long &height, int &region);

  bool convertResultToAddr(const InstructionAPI::Result &res, Address &addr);
  bool convertResultToSlot(const InstructionAPI::Result &res, int &slot);

  bool usedCache(Address, image_func *, std::vector<AbsRegion> &used);
  bool definedCache(Address, image_func *, std::vector<AbsRegion> &defined);

  // Caching mechanism...
  typedef std::vector<AbsRegion> RegionVec;

  typedef std::map<Address, RegionVec> AddrCache;
  typedef std::map<image_func *, AddrCache> FuncCache;

  FuncCache used_cache_;
  FuncCache defined_cache_;
  bool cacheEnabled_;
};

class AssignmentConverter {
 public:  
 AssignmentConverter(bool cache) : cacheEnabled_(cache), aConverter(false) {};

  void convert(InstructionAPI::Instruction::Ptr insn,
	       const Address &addr,
	       image_func *func,
	       std::vector<Assignment::Ptr> &assignments);
  void convert(InstructionAPI::Instruction::Ptr insn,
	       const Address &addr,
	       int_function *func,
	       std::vector<Assignment::Ptr> &assignments);
  void convert(InstructionAPI::Instruction::Ptr insn,
	       const Address &addr,
	       BPatch_function *func,
	       std::vector<Assignment::Ptr> &assignments);


 private:
  void handlePushEquivalent(const InstructionAPI::Instruction::Ptr I,
			    Address addr,
			    image_func *func,
			    std::vector<AbsRegion> &operands,
			    std::vector<Assignment::Ptr> &assignments);
  void handlePopEquivalent(const InstructionAPI::Instruction::Ptr I,
			   Address addr,
			   image_func *func,
			   std::vector<AbsRegion> &operands,
			   std::vector<Assignment::Ptr> &assignments);

  bool cache(image_func *func, Address addr, std::vector<Assignment::Ptr> &assignments);

  typedef std::vector<Assignment::Ptr> AssignmentVec;
  typedef std::map<Address, AssignmentVec> AddrCache;
  typedef std::map<image_func *, AddrCache> FuncCache;

  FuncCache cache_;
  bool cacheEnabled_;

  AbsRegionConverter aConverter;
};

};
#endif

