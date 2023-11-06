/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */


#if !defined(Absloc_Interface_H)
#define Absloc_Interface_H

#include <map>
#include <vector>

#include "Register.h"
#include "Instruction.h"
#include "Expression.h"
#include "Operand.h"
#include "Absloc.h"
#include "util.h"

class int_function;
class BPatch_function;

namespace Dyninst {

  namespace ParseAPI {
    class Function; 
    class Block;
  }

class AbsRegionConverter {
 public:
 DATAFLOW_EXPORT AbsRegionConverter(bool cache, bool stack) :
  cacheEnabled_(cache), stackAnalysisEnabled_(stack) {}

  // Definition: the first AbsRegion represents the expression.
  // If it's a memory reference, any other AbsRegions represent
  // registers used in this expression.

  DATAFLOW_EXPORT void convertAll(InstructionAPI::Expression::Ptr expr,
				  Address addr,
				  ParseAPI::Function *func,
                                  ParseAPI::Block *block,
				  std::vector<AbsRegion> &regions);
  
  DATAFLOW_EXPORT void convertAll(const InstructionAPI::Instruction &insn,
				  Address addr,
				  ParseAPI::Function *func,
                                  ParseAPI::Block *block,
				  std::vector<AbsRegion> &used,
				  std::vector<AbsRegion> &defined);

  // Single converters
  
  DATAFLOW_EXPORT AbsRegion convert(InstructionAPI::RegisterAST::Ptr reg);

  DATAFLOW_EXPORT AbsRegion convert(InstructionAPI::Expression::Ptr expr,
				    Address addr,
				    ParseAPI::Function *func,
                                    ParseAPI::Block *block);

  DATAFLOW_EXPORT AbsRegion convertPredicatedRegister(InstructionAPI::RegisterAST::Ptr r,
          InstructionAPI::RegisterAST::Ptr p,
          bool c);

  // Cons up a stack reference at the current addr
  DATAFLOW_EXPORT AbsRegion stack(Address addr,
				  ParseAPI::Function *func,
                                  ParseAPI::Block *block,
				  bool push);
  
  DATAFLOW_EXPORT AbsRegion frame(Address addr,
				  ParseAPI::Function *func,
                                  ParseAPI::Block *block,
				  bool push);
  
 private:
  // Returns false if the current height is unknown.
  bool getCurrentStackHeight(ParseAPI::Function *func,
                             ParseAPI::Block *block,
			     Address addr, 
			     long &height);
  bool getCurrentFrameHeight(ParseAPI::Function *func,
                             ParseAPI::Block *block,
			     Address addr, 
			     long &height);
  
  bool convertResultToAddr(const InstructionAPI::Result &res, Address &addr);
  bool convertResultToSlot(const InstructionAPI::Result &res, int &slot);
  
  bool usedCache(Address, ParseAPI::Function *, std::vector<AbsRegion> &used);
  bool definedCache(Address, ParseAPI::Function *, std::vector<AbsRegion> &defined);

  // Caching mechanism...
  typedef std::vector<AbsRegion> RegionVec;

  typedef std::map<Address, RegionVec> AddrCache;
  typedef std::map<ParseAPI::Function *, AddrCache> FuncCache;

  FuncCache used_cache_;
  FuncCache defined_cache_;
  bool cacheEnabled_;
  bool stackAnalysisEnabled_;
};

class AssignmentConverter {
 public:  
 DATAFLOW_EXPORT AssignmentConverter(bool cache, bool stack) : cacheEnabled_(cache), aConverter(false, stack) {}

  DATAFLOW_EXPORT void convert(const InstructionAPI::Instruction &insn,
                               const Address &addr,
                               ParseAPI::Function *func,
                               ParseAPI::Block *block,
                               std::vector<Assignment::Ptr> &assignments);


 private:
  void handlePushEquivalent(const InstructionAPI::Instruction I,
			    Address addr,
			    ParseAPI::Function *func,
                            ParseAPI::Block *block,
			    std::vector<AbsRegion> &operands,
			    std::vector<Assignment::Ptr> &assignments);
  void handlePopEquivalent(const InstructionAPI::Instruction I,
			   Address addr,
			   ParseAPI::Function *func,
                           ParseAPI::Block *block,
			   std::vector<AbsRegion> &operands,
			   std::vector<Assignment::Ptr> &assignments);

  bool cache(ParseAPI::Function *func, Address addr, std::vector<Assignment::Ptr> &assignments);

  typedef std::vector<Assignment::Ptr> AssignmentVec;
  typedef std::map<Address, AssignmentVec> AddrCache;
  typedef std::map<ParseAPI::Function *, AddrCache> FuncCache;

  FuncCache cache_;
  bool cacheEnabled_;

  AbsRegionConverter aConverter;
};

}
#endif

