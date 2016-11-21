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

#include "instruction_comp.h"
#include <iterator>

using namespace Dyninst;
using namespace InstructionAPI;

#define work_around_broken_gcc
#if !defined(work_around_broken_gcc)
#include <boost/iterator/indirect_iterator.hpp>
using namespace boost;
template <typename I, typename II>
bool indirect_equal(I f1, I l1, II f2)
{
    return std::equal(make_indirect_iterator(f1), make_indirect_iterator(l1),
                      make_indirect_iterator(f2));
}

#else
template <typename I, typename II>
bool indirect_equal(I f1, I l1, II f2)
{
    for(; f1 != l1; ++f1, ++f2)
    {
        if(!(*(*f1) == *(*f2))) return false;
    }
    return true;
}


#endif

InstructionComponent::InstructionComponent()
{
}

InstructionComponent::~InstructionComponent()
{
}

test_results_t InstructionComponent::program_setup(ParameterDict &params)
{
   return PASSED;
}

test_results_t InstructionComponent::program_teardown(ParameterDict &params)
{
   return PASSED;
}

test_results_t InstructionComponent::group_setup(RunGroup *group, ParameterDict &params)
{
   return PASSED;
}

test_results_t InstructionComponent::group_teardown(RunGroup *group, ParameterDict &params)
{
   return PASSED;
}

test_results_t InstructionComponent::test_setup(TestInfo *test, ParameterDict &params)
{
   return PASSED;
}

test_results_t InstructionComponent::test_teardown(TestInfo *test, ParameterDict &params)
{
   return PASSED;
}

test_results_t InstructionMutator::setup(ParameterDict &param)
{
  return PASSED;
}

InstructionMutator::InstructionMutator()
{
}

InstructionMutator::~InstructionMutator()
{
}

std::string InstructionComponent::getLastErrorMsg()
{
   return std::string("");
}

TEST_DLL_EXPORT ComponentTester *componentTesterFactory()
{
   return new InstructionComponent();
}


test_results_t InstructionMutator::verify_read_write_sets(Instruction::Ptr i, const registerSet& expectedRead,
                                      const registerSet& expectedWritten)
{
    set<RegisterAST::Ptr> actualRead_uo;
    set<RegisterAST::Ptr> actualWritten_uo;
    i->getWriteSet(actualWritten_uo);
    i->getReadSet(actualRead_uo);
    registerSet actualRead, actualWritten;
    copy(actualRead_uo.begin(), actualRead_uo.end(), inserter(actualRead, actualRead.begin()));
    copy(actualWritten_uo.begin(), actualWritten_uo.end(), inserter(actualWritten, actualWritten.begin()));
  
    if(actualRead.size() != expectedRead.size() ||
       actualWritten.size() != expectedWritten.size())
    {
        logerror("FAILED: instruction %s, expected %d regs read, %d regs written, actual %d read, %d written\n",
                 i->format().c_str(), expectedRead.size(), expectedWritten.size(), actualRead.size(), actualWritten.size());
        logerror("Expected read:\n");
        for (registerSet::const_iterator iter = expectedRead.begin(); iter != expectedRead.end(); iter++) {
            logerror("\t%s\n", (*iter)->format().c_str());
        }
        logerror("Expected written:\n");
        for (registerSet::const_iterator iter = expectedWritten.begin(); iter != expectedWritten.end(); iter++) {
            logerror("\t%s\n", (*iter)->format().c_str());
        }
        logerror("Actual read:\n");
        for (registerSet::iterator iter = actualRead.begin(); iter != actualRead.end(); iter++) {
            logerror("\t%s\n", (*iter)->format().c_str());
        }
        logerror("Actual written:\n");
        for (registerSet::iterator iter = actualWritten.begin(); iter != actualWritten.end(); iter++) {
            logerror("\t%s\n", (*iter)->format().c_str());
        }

        return FAILED;
    }
    registerSet::const_iterator safety;
    for(safety = expectedRead.begin();
        safety != expectedRead.end();
        ++safety)
    {
        if(!(*safety))
        {
            logerror("ERROR: null shared pointer in expectedRead for instruction %s\n", i->format().c_str());
            return FAILED;
        }
    
    }
    for(safety = actualRead.begin();
        safety != actualRead.end();
        ++safety)
    {
        if(!(*safety))
        {
            logerror("ERROR: null shared pointer in actualRead for instruction %s\n", i->format().c_str());
            return FAILED;
        }
    
    }
  
    if(indirect_equal(actualRead.begin(),
       actualRead.end(),
       expectedRead.begin()))
    {
        
        for(registerSet::const_iterator it = expectedRead.begin();
            it != expectedRead.end();
            ++it)
        {
            if(!i->isRead(*it))
            {
                logerror("%s was in read set, but isRead(%s) was false\n", (*it)->format().c_str(), (*it)->format().c_str());
                return FAILED;
            }
        }
        
    }
    else
    {
        
        logerror("Read set for instruction %s not as expected\n", i->format().c_str());
        logerror("Expected read:\n");
        for (registerSet::const_iterator iter = expectedRead.begin(); iter != expectedRead.end(); iter++) {
            logerror("\t%s\n", (*iter)->format().c_str());
        }
        logerror("Expected written:\n");
        for (registerSet::const_iterator iter = expectedWritten.begin(); iter != expectedWritten.end(); iter++) {
            logerror("\t%s\n", (*iter)->format().c_str());
        }
        logerror("Actual read:\n");
        for (registerSet::iterator iter = actualRead.begin(); iter != actualRead.end(); iter++) {
            logerror("\t%s\n", (*iter)->format().c_str());
        }
        logerror("Actual written:\n");
        for (registerSet::iterator iter = actualWritten.begin(); iter != actualWritten.end(); iter++) {
            logerror("\t%s\n", (*iter)->format().c_str());
        }
        
        return FAILED;
    }
    for(safety = expectedWritten.begin();
        safety != expectedWritten.end();
        ++safety)
    {
        if(!(*safety))
        {
            logerror("ERROR: null shared pointer in expectedWritten for instruction %s\n", i->format().c_str());
            return FAILED;
        }
    
    }
    for(safety = actualWritten.begin();
        safety != actualWritten.end();
        ++safety)
    {
        if(!(*safety))
        {
            logerror("ERROR: null shared pointer in actualWritten for instruction %s\n", i->format().c_str());
            return FAILED;
        }
    
    }
    
    if(indirect_equal(actualWritten.begin(),
       actualWritten.end(),
       expectedWritten.begin()))
    {
        for(registerSet::const_iterator it = expectedWritten.begin();
            it != expectedWritten.end();
            ++it)
        {
            if(!i->isWritten(*it))
            {
                logerror("%s was in write set, but isWritten(%s) was false\n", (*it)->format().c_str(), (*it)->format().c_str());
                return FAILED;
            }
        }
    }
    else
    {
        logerror("Write set for instruction %s not as expected\n", i->format().c_str());
        logerror("Expected read:\n");
        for (registerSet::const_iterator iter = expectedRead.begin(); iter != expectedRead.end(); iter++) {
            logerror("\t%s\n", (*iter)->format().c_str());
        }
        logerror("Expected written:\n");
        for (registerSet::const_iterator iter = expectedWritten.begin(); iter != expectedWritten.end(); iter++) {
            logerror("\t%s\n", (*iter)->format().c_str());
        }
        logerror("Actual read:\n");
        for (registerSet::iterator iter = actualRead.begin(); iter != actualRead.end(); iter++) {
            logerror("\t%s\n", (*iter)->format().c_str());
        }
        logerror("Actual written:\n");
        for (registerSet::iterator iter = actualWritten.begin(); iter != actualWritten.end(); iter++) {
            logerror("\t%s\n", (*iter)->format().c_str());
        }
        return FAILED;
    }
    logerror("PASSED: Instruction %s had read, write sets as expected\n", i->format().c_str());
    return PASSED;
}

test_results_t InstructionMutator::verifyCFT(Expression::Ptr cft, bool expectedDefined, unsigned long expectedValue,
                                             Result_Type expectedType)
{
    Result cftResult = cft->eval();
    if(cftResult.defined != expectedDefined) {
        logerror("FAILED: CFT %s, expected result defined %s, actual %s\n", cft->format().c_str(),
            expectedDefined ? "true" : "false",
            cftResult.defined ? "true" : "false");
        return FAILED;
    }
    if(expectedDefined)
    {
        if(cftResult.type != expectedType)
        {
            logerror("FAILED: CFT %s, expected result type %d, actual %d\n", cft->format().c_str(),
                     expectedType, cftResult.type);
            return FAILED;
        }
        if(cftResult.convert<unsigned long long int>() != expectedValue)
        {
            logerror("FAILED: CFT %s, expected result value 0x%x, actual 0x%x\n", cft->format().c_str(),
                    expectedValue, cftResult.convert<unsigned long>());
            return FAILED;
        }
    }
    return PASSED;
}

test_results_t InstructionMutator::failure_accumulator(test_results_t lhs, test_results_t rhs)
{
    if(lhs == FAILED || rhs == FAILED)
    {
        return FAILED;
    }
    return PASSED;
}
