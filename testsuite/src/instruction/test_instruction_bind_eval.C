/*
 * Copyright (c) 1996-2008 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

#include "instruction_comp.h"
#include "test_lib.h"

#include "Instruction.h"
#include "InstructionDecoder.h"
#include "Expression.h"
#include "Result.h"

//#include <dyn_detail/boost/assign/list_of.hpp>
//#include <dyn_detail/boost/iterator/indirect_iterator.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/iterator/indirect_iterator.hpp>

using namespace Dyninst;
using namespace InstructionAPI;
using namespace boost::assign;
using namespace boost;

using namespace std;

class test_instruction_bind_eval_Mutator : public InstructionMutator {
public:
   test_instruction_bind_eval_Mutator() { };
   virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* test_instruction_bind_eval_factory()
{
   return new test_instruction_bind_eval_Mutator();
}

template <typename T>
struct shared_ptr_lt
{
  bool operator()(const T& lhs, const T& rhs)
  {
    // Non-nulls precede nulls
    if(rhs.get() == NULL)
    {
      return lhs.get() != NULL;
    }
    if(lhs.get() == NULL)
      return false;
    // Otherwise, dereference and compare
    return *lhs < *rhs;
  }
  
};


typedef std::set<RegisterAST::Ptr, shared_ptr_lt<RegisterAST::Ptr> > registerSet;

test_results_t failure_accumulator(test_results_t lhs, test_results_t rhs)
{
  if(lhs == FAILED || rhs == FAILED)
  {
    return FAILED;
  }
  return PASSED;
}

test_results_t verifyCFT(Expression::Ptr cft, bool expectedDefined, unsigned long expectedValue, Result_Type expectedType)
{
  Result cftResult = cft->eval();
  if(cftResult.defined != expectedDefined) {
    logerror("FAILED: expected result defined %s, actual %s\n", expectedDefined ? "true" : "false", 
	     cftResult.defined ? "true" : "false");
    return FAILED;
  }
  if(expectedDefined)
  {
    if(cftResult.type != expectedType)
    {
      logerror("FAILED: expected result type %d, actual %d\n", expectedType, cftResult.type);
      return FAILED;
    }
    if(cftResult.convert<unsigned long>() != expectedValue)
    {
      logerror("FAILED: expected result value 0x%x, actual 0x%x\n", expectedValue, cftResult.convert<unsigned long>());
      return FAILED;
    }
  }
  return PASSED;
}


test_results_t test_instruction_bind_eval_Mutator::executeTest()
{
  const unsigned char buffer[] = 
  {
    0xFF, 0x94, 0xC1, 0xEF, 0xBE, 0xAD, 0xDE // call [8*EAX + ECX + 0xDEADBEEF]
  };
  unsigned int size = 7;
  unsigned int expectedInsns = 2;
  InstructionDecoder d(buffer, size);
#if defined(arch_x86_64_test)
  d.setMode(true);
#else
  d.setMode(false);
#endif
  std::vector<Instruction> decodedInsns;
  Instruction i;
  do
  {
    i = d.decode();
    decodedInsns.push_back(i);
  }
  while(i.isValid());
  if(decodedInsns.size() != expectedInsns)
  {
    logerror("FAILED: Expected %d instructions, decoded %d\n", expectedInsns, decodedInsns.size());
    for(std::vector<Instruction>::iterator curInsn = decodedInsns.begin();
	curInsn != decodedInsns.end();
	++curInsn)
    {
      logerror("\t%s\n", curInsn->format().c_str());
    }
    
    return FAILED;
  }
  if(decodedInsns.back().isValid())
  {
    logerror("FAILED: Expected instructions to end with an invalid instruction, but they didn't");
    return FAILED;
  }

  Expression::Ptr theCFT = decodedInsns[0].getControlFlowTarget();
  if(!theCFT) {
    logerror("FAILED: instruction %s decoded from call [8*EAX + ECX + 0xDEADBEEF], no CFT found\n", decodedInsns[0].format().c_str());
    return FAILED;
  }
  if(verifyCFT(theCFT, false, 0x1000, u32) == FAILED)
  {
    return FAILED;
  }
  
    
#if defined(arch_x86_64_test)
  RegisterAST* eax = new RegisterAST(r_RAX);
  RegisterAST* ecx = new RegisterAST(r_RCX);
#else
  RegisterAST* eax = new RegisterAST(r_EAX);
  RegisterAST* ecx = new RegisterAST(r_ECX);
#endif
  Result three(u32, 3);
  Result five(u32, 5);
  
  if(!theCFT->bind(eax, three)) {
    logerror("FAILED: bind of EAX failed\n");
    return FAILED;
  }
  if(verifyCFT(theCFT, false, 0x1000, u32) == FAILED)
  {
    return FAILED;
  }
  if(!theCFT->bind(ecx, five)) {
    logerror("FAILED: bind of ECX failed\n");
    return FAILED;
  }
  if(verifyCFT(theCFT, false, 0x1000, u32) == FAILED)
  {
    return FAILED;
  }
  vector<InstructionAST::Ptr> tmp;
  theCFT->getChildren(tmp);
  if(tmp.size() != 1)
  {
    logerror("FAILED: expected dereference with one child, got %d children\n", tmp.size());
    return FAILED;
  }
  Expression::Ptr memRef = dyn_detail::boost::dynamic_pointer_cast<Expression>(tmp[0]);
  if(!memRef) {
    logerror("FAILED: memRef was not an expression\n");
    return FAILED;
  }
  if(verifyCFT(memRef, true, 0xDEADBEEF + (0x03 * 0x08 + 0x05), u32) == FAILED) {
    return FAILED;
  }

  return PASSED;
}

