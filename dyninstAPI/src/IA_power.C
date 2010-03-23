/*
 * Copyright (c) 1996-2009 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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


#include "IA_IAPI.h"

#include "Register.h"
#include "Dereference.h"
#include "Immediate.h"
#include "BinaryFunction.h"
#include "debug.h"
#include "symtab.h"

#if defined(cap_jump_table_slicing)
#error "DYNINST DEPENDENCY ON SYMEVAL"
#include "../../symEval/h/slicing.h"
#include "../../symEval/h/AbslocInterface.h"
#include "Graph.h"
#endif

#include <deque>
#include <iostream>
#include <sstream>
#include <functional>
#include <algorithm>

using namespace Dyninst;
using namespace InstructionAPI;


bool IA_IAPI::isFrameSetupInsn(Instruction::Ptr i) const
{
    return false;
}

bool IA_IAPI::isNop() const
{
    return false;
}

bool IA_IAPI::isMovAPSTable(pdvector<std::pair< Address, EdgeTypeEnum > >&) const
{
    return false;
}

bool IA_IAPI::isTableInsn(Instruction::Ptr) const
{
    return false;
}
        
std::map<Address, Instruction::Ptr>::const_iterator IA_IAPI::findTableInsn() const
{
    return allInsns.end();
}


#if defined(cap_jump_table_slicing)
bool affectsKnownInput(Assignment::Ptr a) {
  Absloc toc(ppc32::r2);
  for(std::vector<AbsRegion>::iterator i = a->inputs().begin();
      i != a->inputs().end();
      ++i)
    {
      if(i->contains(toc)) return true;
    }
  if(a->out().contains(Absloc::Heap)) return true;
  return false;
}

struct isDone
{
  isDone(image_func* f, image_basicBlock* b) : func(f), block(b) {}
  bool operator()(Assignment::Ptr a)
  {
    return (a->addr() <= block->firstInsnOffset()) ||
      (a->addr() > block->lastInsnOffset());
  }
  image_func* func;
  image_basicBlock* block;
};

bool no(Assignment::Ptr) { return false; }
#endif

namespace detail
{
    bool isNonCallEdge(image_edge* e)
    {
        return e->getType() != ET_CALL;
    }
    bool leadsToVisitedBlock(image_edge* e, const std::set<image_basicBlock*>& visited)
    {
        image_basicBlock* src = e->getSource();
        return visited.find(src) != visited.end();
    }

    class TOCandOffsetExtractor : public Dyninst::InstructionAPI::Visitor
    {
        public:
            TOCandOffsetExtractor(Address TOCvalue) : toc_contents(TOCvalue) {}
            virtual ~TOCandOffsetExtractor() {}
            virtual void visit(BinaryFunction* b) {
                Address arg1 = m_stack.front();
                m_stack.pop_front();
                Address arg2 = m_stack.front();
                m_stack.pop_front();
                if(b->isAdd()) {
                    result = arg1 + arg2;
                } else if(b->isMultiply()) {
                    result = arg1 * arg2;
                } else {
                    assert(!"unexpected binary function!");
                    result = 0;
                }
                m_stack.push_front(result);
            }
            virtual void visit(Immediate* i) {
                Address tmp = i->eval().convert<Address>();
                result = tmp;
                m_stack.push_front(tmp);   
            }
            virtual void visit(RegisterAST* r) {
                if(r->getID() == ppc32::r2) {
                    m_stack.push_front(toc_contents);
                } else {
                    m_stack.push_front(0);
                }
            }
            virtual void visit(Dereference*) {}
            void clear() {
                m_stack.clear();
                result = 0;
            }
            std::deque<Address> m_stack;
            Address result;
            Address toc_contents;
    };
};



#if defined(cap_jump_table_slicing)
template <typename _output_iterator>
GraphPtr sliceOnPC(image_basicBlock* block,
	       image_func* const func,
	       const Instruction::Ptr& insn,
	       Address addr,
	       _output_iterator inputs_to_slice)
{
  using namespace std;
  Absloc thePC = Absloc::makePC(func->img()->getArch());
  AssignmentConverter c(true);
  vector<Assignment::Ptr> assignments;
  c.convert(insn, addr, func, assignments);
  Assignment::Ptr sliceStart;
  for(vector<Assignment::Ptr>::iterator curAssign = assignments.begin();
      curAssign != assignments.end();
      ++curAssign) {
    if((*curAssign)->out().contains(thePC)) {
      sliceStart = *curAssign;
    }
  }
  if(sliceStart) {
    cerr << "found assignment to PC: " << sliceStart << " ";
    for(std::vector<AbsRegion>::iterator curInput = sliceStart->inputs().begin();
	curInput != sliceStart->inputs().end();
	++curInput) {
      cerr << *curInput << " ";
    }
    cerr << endl;
  } else {
    cerr << "ERROR IN SEMANTICS: indirect branch didn't assign to PC" << endl;
    return GraphPtr();
  }
  cerr << "slicing within block " << hex << block->firstInsnOffset() << "..." <<
    block->lastInsnOffset() << endl;
  Slicer s(sliceStart, block, func);
  GraphPtr pcSlice = s.backwardSlice(&affectsKnownInput, isDone(func, block));
  NodeIterator entryBegin, entryEnd;
  pcSlice->entryNodes(entryBegin, entryEnd);
  while(entryBegin != entryEnd) {
    Assignment::Ptr currAssign = 
      dyn_detail::boost::dynamic_pointer_cast<AssignNode>(*entryBegin)->assign();
    if(currAssign) {
      for(std::vector<AbsRegion>::const_iterator curInput = currAssign->inputs().begin();
	  curInput != currAssign->inputs().end();
	  ++curInput) {
	*(inputs_to_slice++) = *curInput;
      }
    }
    entryBegin++;
  }
  return pcSlice;
}
#endif

// This should only be called on a known indirect branch...
bool IA_IAPI::parseJumpTable(image_basicBlock* currBlk,
                             pdvector<std::pair< Address, EdgeTypeEnum> >& outEdges) const
{
#if defined(cap_jump_table_slicing)
  // we only try to parse guarded jump tables; this means that we should reach this block via a conditional branch.
  pdvector<image_edge*> srcs;
  currBlk->getSources(srcs);
  if(srcs.size() != 1) {
    cerr << "multiple source blocks for block at " << hex << current << dec << ", skipping" << endl;
    return false;
  }
  static int jumpTableNumber = 0;
  jumpTableNumber++;
  image_basicBlock* predecessor = srcs[0]->getSource();
  cerr << "predecessor block to jump table candidate " << jumpTableNumber << ": " << hex 
       << predecessor->firstInsnOffset() << "..." 
       << predecessor->lastInsnOffset() << dec << endl;

  stringstream filename;
  filename << "jumpTableGraph" << jumpTableNumber;
  std::set<AbsRegion> inputs;
  GraphPtr jumpTableSlice = sliceOnPC(currBlk, context, curInsn(), current, inserter(inputs, inputs.begin()));
  jumpTableSlice->printDOT(filename.str());
  /*  std::map<Address, Instruction::Ptr>::const_iterator found = allInsns.find(predecessor->lastInsnOffset());
  
  if(found != allInsns.end()) {
    cerr << "predecessor block not parsed...WTF?" << endl;
    return false;
  }
  Instruction::Ptr predEnd = found->second;
  assert(predEnd);
  GraphPtr dominatorSlice = sliceOnPC(predecessor, context, predEnd, predecessor->lastInsnOffset(),
  inserter(inputs, inputs.begin()));*/
  fprintf(stderr, "Candidate jump table at 0x%lx, inputs are:\n", current);
  for(std::set<AbsRegion>::iterator curInput = inputs.begin();
      curInput != inputs.end();
      ++curInput) {
    cerr << "\t" << curInput->format() << endl;
  }
  parsing_printf("\tparseJumpTable returning FALSE (not implemented on POWER yet)\n");
#else

  Address initialAddress = current;
  static RegisterAST::Ptr toc_reg(new RegisterAST(ppc32::r2));
  
  Address TOC_address = img->getObject()->getTOCoffset();
  detail::TOCandOffsetExtractor toc_visitor(TOC_address);
    
  // If there are no prior instructions then we can't be looking at a
  // jump through a jump table.
  if(allInsns.size() < 2) {
      return false;
  }  
  
  
  // Check if the previous instruction is a move to CTR or LR;
  // if it is, then this is the pattern we're familiar with.  The
  // register being moved into CTR or LR has the address to jump to.
  std::map<Address, Instruction::Ptr>::const_iterator patternIter = curInsnIter;
  patternIter--;
  RegisterAST::Ptr jumpAddrReg;
  static RegisterAST::Ptr linkReg(new RegisterAST(ppc32::lr)); 
  static RegisterAST::Ptr countReg(new RegisterAST(ppc32::ctr));
  std::set<RegisterAST::Ptr> regs;
  if(patternIter->second->getOperation().getID() == power_op_mtspr &&
     (patternIter->second->isWritten(linkReg) ||
     patternIter->second->isWritten(countReg)))
  {
      patternIter->second->getReadSet(regs);
      assert(regs.size() == 1);
      jumpAddrReg = *(regs.begin());
  }
  else
  {
      return false;
  }
  // In the pattern we've seen, if the instruction previous to this is
  // an add with a result that ends up being used as the jump address,
  // then we're adding a relative value we got from the table to a base
  // address to get the jump address; in other words, the contents of
  // the jump table are relative.
  bool tableIsRelative = false;
  if(patternIter != allInsns.begin())
  {
      patternIter--;
      if(patternIter->second->getOperation().getID() == power_op_add &&
         patternIter->second->isWritten(*(regs.begin())))
      {
        tableIsRelative = true;
      }
  }

  patternIter = curInsnIter;
    
    Address jumpStartAddress = 0;
    Address adjustEntry = 0;
    Address tableStartAddress = 0;
      if(!TOC_address)
      {
          while(patternIter != allInsns.begin())
          {
              patternIter--;
              if(patternIter->second->getOperation().getID() == power_op_addi)
              {
                  regs.clear();
                  patternIter->second->getReadSet(regs);
                  toc_visitor.clear();
                  patternIter->second->getOperand(1).getValue()->apply(&toc_visitor);
                  tableStartAddress = toc_visitor.result;
                  assert(regs.size() == 1);
                  patternIter--;
                  if(patternIter->second->getOperation().getID() == power_op_addis &&
                     patternIter->second->isWritten(*(regs.begin())))
                  {
                      toc_visitor.clear();
                      patternIter->second->getOperand(1).getValue()->apply(&toc_visitor);
                      tableStartAddress += (toc_visitor.result * 0x10000) & 0xFFFF0000;
                      break;
                  }
                  tableStartAddress = 0;
              }
              else if(patternIter->second->getOperation().getID() == power_op_addis)
              {
                  regs.clear();
                  patternIter->second->getReadSet(regs);
                  toc_visitor.clear();
                  patternIter->second->getOperand(1).getValue()->apply(&toc_visitor);
                  tableStartAddress = toc_visitor.result;
                  tableStartAddress *= 10000;
                  tableStartAddress &= 0xFFFF0000;
                  assert(regs.size() == 1);
                  patternIter--;
                  if(patternIter->second->getOperation().getID() == power_op_addi &&
                     patternIter->second->isWritten(*(regs.begin())))
                  {
                      toc_visitor.clear();
                      patternIter->second->getOperand(1).getValue()->apply(&toc_visitor);
                      tableStartAddress += toc_visitor.result;
                      break;
                  }
                  tableStartAddress = 0;
              }
          }
      }
      else if (tableIsRelative) {
          while(patternIter != allInsns.begin())
          {
              patternIter--;
              if((patternIter->second->getOperation().getID() == power_op_lwz ||
                  patternIter->second->getOperation().getID() == power_op_ld) &&
                 patternIter->second->isRead(toc_reg))
              {
                  toc_visitor.clear();
                  patternIter->second->getOperand(1).getValue()->apply(&toc_visitor);
                  jumpStartAddress = toc_visitor.result;
                  break;
              }
          }
    // Anyone know what this does?
          patternIter--;
          if((patternIter->second->getOperation().getID() == power_op_lwz ||
              patternIter->second->getOperation().getID() == power_op_ld))
          {
              toc_visitor.clear();
              patternIter->second->getOperand(1).getValue()->apply(&toc_visitor);
              adjustEntry = toc_visitor.result;
          }
       
          while(patternIter != allInsns.begin()){
              patternIter--;
              if((patternIter->second->getOperation().getID() == power_op_lwz ||
                  patternIter->second->getOperation().getID() == power_op_ld) &&
                  patternIter->second->isRead(toc_reg))
              {
                  toc_visitor.clear();
                  patternIter->second->getOperand(1).getValue()->apply(&toc_visitor);
                  tableStartAddress = toc_visitor.result;
                  break;
              }
          }
      } else {
          bool foundAdjustEntry = false;
          while( patternIter != allInsns.begin() )
          {
              if(patternIter->second->getOperation().getID() == power_op_addi &&
                 patternIter->second->isWritten(jumpAddrReg) &&
                !foundAdjustEntry)
              {
                  foundAdjustEntry = true;
                  toc_visitor.clear();
                  patternIter->second->getOperand(1).getValue()->apply(&toc_visitor);
                  adjustEntry = toc_visitor.result;
                  regs.clear();
                  patternIter->second->getReadSet(regs);
                  jumpAddrReg = *(regs.begin());
              }
              else if(patternIter->second->getOperation().getID() == power_op_lwz &&
                      patternIter->second->isRead(toc_reg) &&
                      patternIter->second->isWritten(jumpAddrReg))
              {
                  toc_visitor.clear();
                  patternIter->second->getOperand(1).getValue()->apply(&toc_visitor);
                  tableStartAddress = toc_visitor.result;
                  break;
              }
              patternIter--;
          }
      }

  // We could also set this = jumpStartAddress...
      if (tableStartAddress == 0)  {
          return false;
      }
      int maxSwitch = 0;
    
      while( patternIter != allInsns.begin() ) {
          if(patternIter->second->getOperation().getID() == power_op_bc) // make this a true cond. branch check
          {
              patternIter--;
              if(patternIter != allInsns.begin() && patternIter->second->getOperation().getID() == power_op_cmpi)
              {
                  maxSwitch = patternIter->second->getOperand(2).getValue()->eval().convert<int>() + 1;
                  break;
              }
          }
          patternIter--;
      }

//fprintf(stderr, "After checking: max switch %d\n", maxSwitch);
      if(!maxSwitch){
          return false;
      }

      Address jumpStart = 0;
      Address tableStart = 0;
      bool is64 = (img->getAddressWidth() == 8);

      if(TOC_address)
      {
              if (tableIsRelative) {
                  void *jumpStartPtr = img->getPtrToData(jumpStartAddress);
        //fprintf(stderr, "jumpStartPtr (0x%lx) = %p\n", jumpStartAddress, jumpStartPtr);
                  if (jumpStartPtr)
                      jumpStart = (is64
                              ? *((Address  *)jumpStartPtr)
                      : *((uint32_t *)jumpStartPtr));
        //fprintf(stderr, "jumpStart 0x%lx, initialAddr 0x%lx\n",
        //      jumpStart, initialAddress);
                  if (jumpStartPtr == NULL) {
                      return false;
                  }
              }
              void *tableStartPtr = img->getPtrToData(tableStartAddress);
      //fprintf(stderr, "tableStartPtr (0x%lx) = %p\n", tableStartAddress, tableStartPtr);
              tableStart = *((Address *)tableStartPtr);
              if (tableStartPtr)
                  tableStart = (is64
                          ? *((Address  *)tableStartPtr)
                  : *((uint32_t *)tableStartPtr));
              else {
                  return false;
              }
      //fprintf(stderr, "... tableStart 0x%lx\n", tableStart);
      
      // We're getting an absolute out of the TOC. Figure out
      // whether we're in code or data.
              const fileDescriptor &desc = img->desc();
              Address textStart = desc.code();
              Address dataStart = desc.data();
      
      // I think this is valid on ppc64 linux.  dataStart and codeStart can be 0.
      // assert(jumpStart < dataStart);
      
              bool tableData = false;
      
                if (tableStart > dataStart) {
                    tableData = true;
                    tableStart -= dataStart;
        //fprintf(stderr, "Table in data, offset 0x%lx\n", tableStart);
                }
                else {
                    tableData = false;
                    tableStart -= textStart;
        //fprintf(stderr, "Table in text, offset 0x%lx\n", tableStart);
                }
      
              for(int i=0;i<maxSwitch;i++){
                  Address tableEntry = adjustEntry + tableStart + (i * instruction::size());
        //fprintf(stderr, "Table entry at 0x%lx\n", tableEntry);
                  if (img->isValidAddress(tableEntry)) {
                      int jumpOffset = *((int *)img->getPtrToData(tableEntry));
          
          //fprintf(stderr, "jumpOffset 0x%lx\n", jumpOffset);
                      Address res = (Address)(jumpStart + jumpOffset);

                      if (img->isCode(res))
                          outEdges.push_back(std::make_pair((Address)(jumpStart+jumpOffset), ET_INDIR));
          //fprintf(stderr, "Entry of 0x%lx\n", (Address)(jumpStart + jumpOffset));
                  }
                  else {
          //fprintf(stderr, "Address not valid!\n");
                  }
              }
      }
  // No TOC, so we're on Power32 Linux.  Do the ELF thing.
      else
      {
          int entriesAdded = 0;
          for(int i = 0; i < maxSwitch; i++)
          {
              void* ptr = NULL;
              Address tableEntry = tableStartAddress + i*instruction::size();
              if(img->isValidAddress(tableEntry))
              {
                  ptr = img->getPtrToInstruction(tableEntry);
              }
              if(ptr)
              {
                  int jumpOffset = *((int *)ptr);
                  outEdges.push_back(std::make_pair((Address)(jumpStartAddress+jumpOffset), ET_INDIR));
                  ++entriesAdded;
              }
          }
          if(!entriesAdded)
          {
              return false;
          }
    //fprintf(stderr, "Found %d entries in jump table, returning success\n", entriesAdded);
      }

  // Sanity check entries in res
      for (pdvector<std::pair<Address, EdgeTypeEnum> >::iterator iter = outEdges.begin();
           iter != outEdges.end(); iter++) {
               if ((iter->first) % 4) {
                   parsing_printf("Warning: found unaligned jump table destination 0x%lx for jump at 0x%lx, disregarding table\n",
                                  iter->first, initialAddress);
                   return false;
               }
           }
           return true;
#endif
  return false;
}


Address IA_IAPI::findThunkInBlock(image_basicBlock* curBlock, Address& thunkOffset) const
{
    return 0;
}


std::pair<Address, Address> IA_IAPI::findThunkAndOffset(image_basicBlock* start) const
{
    return std::make_pair(0, 0);
}

boost::tuple<Instruction::Ptr,
 Instruction::Ptr,
 bool> IA_IAPI::findMaxSwitchInsn(image_basicBlock *start) const
{
    return boost::make_tuple(Instruction::Ptr(), Instruction::Ptr(), false);
}
 
Address IA_IAPI::getTableAddress(Instruction::Ptr tableInsn, Address thunkOffset) const
{
    return 0;
}

bool IA_IAPI::fillTableEntries(Address thunkOffset,
                               Address tableBase,
                               unsigned tableSize,
                               unsigned tableStride,
                               pdvector<std::pair< Address, EdgeTypeEnum> >& outEdges) const
{
    return false;
}


bool IA_IAPI::computeTableBounds(Instruction::Ptr maxSwitchInsn,
                                 Instruction::Ptr branchInsn,
                                 Instruction::Ptr tableInsn,
                                 bool foundJCCAlongTaken,
                                 unsigned& tableSize,
                                 unsigned& tableStride) const
{
    return false;
}

bool IA_IAPI::isThunk() const {
    return false;
}

bool IA_IAPI::isTailCall(unsigned int) const
{
    return false;
}

bool IA_IAPI::checkEntry() const
{
    return true;
}

bool IA_IAPI::savesFP() const
{
    return false;
}

bool IA_IAPI::isStackFramePreamble(int& /*frameSize*/) const
{
    return false;
}

bool IA_IAPI::cleansStack() const
{
    return false;
}
