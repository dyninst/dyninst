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

#include <boost/assign.hpp>
#include <boost/iterator/indirect_iterator.hpp>

using namespace Dyninst;
using namespace InstructionAPI;
using namespace boost;
using namespace std;

class test_instruction_read_write_Mutator : public InstructionMutator {
public:
   test_instruction_read_write_Mutator() { };
   virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* test_instruction_read_write_factory()
{
   return new test_instruction_read_write_Mutator();
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


test_results_t verify_read_write_sets(const Instruction& i, const registerSet& expectedRead,
				      const registerSet& expectedWritten)
{
  set<RegisterAST::Ptr> actualRead_uo;
  set<RegisterAST::Ptr> actualWritten_uo;
  i.getWriteSet(actualWritten_uo);
  i.getReadSet(actualRead_uo);
  registerSet actualRead, actualWritten;
  copy(actualRead_uo.begin(), actualRead_uo.end(), inserter(actualRead, actualRead.begin()));
  copy(actualWritten_uo.begin(), actualWritten_uo.end(), inserter(actualWritten, actualWritten.begin()));
  
  if(actualRead.size() != expectedRead.size() ||
     actualWritten.size() != expectedWritten.size())
  {
    logerror("FAILED: instruction %s, expected %d regs read, %d regs written, actual %d read, %d written\n",
	     i.format().c_str(), expectedRead.size(), expectedWritten.size(), actualRead.size(), actualWritten.size());
    return FAILED;
  }
  
  if(equal(make_indirect_iterator(actualRead.begin()), 
	   make_indirect_iterator(actualRead.end()), 
	   make_indirect_iterator(expectedRead.begin())) && 
     equal(make_indirect_iterator(actualWritten.begin()), 
	   make_indirect_iterator(actualWritten.end()), 
	   make_indirect_iterator(expectedWritten.begin())))
  {
    for(registerSet::const_iterator it = expectedRead.begin();
	it != expectedRead.end();
	++it)
    {
      if(!i.isRead(*it))
      {
	logerror("%s was in read set, but isRead(%s) was false\n", (*it)->format().c_str(), (*it)->format().c_str());
	return FAILED;
      }
    }
    for(registerSet::const_iterator it = expectedWritten.begin();
	it != expectedWritten.end();
	++it)
    {
      if(!i.isWritten(*it))
      {
	logerror("%s was in write set, but isWritten(%s) was false\n", (*it)->format().c_str(), (*it)->format().c_str());
	return FAILED;
      }
    }
    
    return PASSED;
  }
  logerror("Read/write sets for instruction %s not as expected\n", i.format().c_str());
  return FAILED;
}


test_results_t test_instruction_read_write_Mutator::executeTest()
{
  const unsigned char buffer[] = 
  {
    0x40, // INC eAX
    0x50, // PUSH rAX
    0x74, 0x10, // JZ +0x10(8)
    0xE8, 0x20, 0x00, 0x00, 0x00, // CALL +0x10(32)
    0xF8, // CLC
    0x04, 0x30 // ADD AL, 0x30(8)
  };
  unsigned int size = 12;
  unsigned int expectedInsns = 7;
  
  InstructionDecoder d(buffer, size);
  std::vector<Instruction> decodedInsns;
  Instruction i;
  do
  {
    i = d.decode();
    decodedInsns.push_back(i);
  }
  while(i.isValid());
  if(decodedInsns.size() != expectedInsns) // six valid, one invalid
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
  RegisterAST::Ptr eax(new RegisterAST(r_eAX));
  RegisterAST::Ptr adjust(new RegisterAST(r_AF));
  RegisterAST::Ptr zero(new RegisterAST(r_ZF));
  RegisterAST::Ptr overflow(new RegisterAST(r_OF));
  RegisterAST::Ptr parity(new RegisterAST(r_PF));
  RegisterAST::Ptr sign(new RegisterAST(r_SF));
  registerSet expectedRead, expectedWritten;
  expectedRead.insert(expectedRead.begin(), eax);
  expectedWritten = list_of(eax)(adjust)(zero)(overflow)(parity)(sign);
  
  
  return verify_read_write_sets(decodedInsns[0], expectedRead, expectedWritten);
}

