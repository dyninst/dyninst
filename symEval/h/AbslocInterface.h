

#if !defined(Absloc_Interface_H)
#define Absloc_Interface_H

#include "Instruction.h"
#include "Register.h"
#include "Expression.h"
#include "Operand.h"
#include "Absloc.h"

class image_func;

namespace Dyninst {

class AbsRegionConverter {
 public:
  // Definition: the first AbsRegion represents the expression.
  // If it's a memory reference, any other AbsRegions represent
  // registers used in this expression.

  static void convertAll(InstructionAPI::Expression::Ptr expr,
			 Address addr,
			 image_func *func,
			 std::vector<AbsRegion> &regions);

  static void convertAll(InstructionAPI::Instruction::Ptr insn,
			 Address addr,
			 image_func *func,
			 std::vector<AbsRegion> &used,
			 std::vector<AbsRegion> &defined);

  // Single converters

  static AbsRegion convert(InstructionAPI::RegisterAST::Ptr reg);

  static AbsRegion convert(InstructionAPI::Expression::Ptr expr,
			   Address addr,
			   image_func *func);

  // Cons up a stack reference at the current addr
  static AbsRegion stack(Address addr,
			 image_func *func);

 private:
    // Returns false if the current height is unknown.
  static bool getCurrentStackHeight(image_func *func,
				    Address addr, 
				    long &height, int &region);
  static bool getCurrentFrameStatus(image_func *func,
				    Address addr);

  static bool convertResultToAddr(const InstructionAPI::Result &res, Address &addr);
  static bool convertResultToSlot(const InstructionAPI::Result &res, int &slot);

};

class AssignmentConverter {
 public:  
  static void convert(InstructionAPI::Instruction::Ptr insn,
		      const Address &addr,
		      image_func *func,
		      std::set<Assignment::Ptr> &assignments);
 private:
  static void handlePushEquivalent(const InstructionAPI::Instruction::Ptr I,
				   Address addr,
				   image_func *func,
				   std::vector<AbsRegion> &operands,
				   std::set<Assignment::Ptr> &assignments);
  static void handlePopEquivalent(const InstructionAPI::Instruction::Ptr I,
				  Address addr,
				  image_func *func,
				  std::vector<AbsRegion> &operands,
				  std::set<Assignment::Ptr> &assignments);
};

};
#endif

