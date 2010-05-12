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
                parsing_printf("\tTOC visitor visiting binary function, result is 0x%lx\n",
                            result);
                m_stack.push_front(result);
            }
            virtual void visit(Immediate* i) {
                Address tmp = i->eval().convert<Address>();
                result = tmp;
                parsing_printf("\tTOC visitor visiting immediate, result is 0x%lx\n",
                            result);
                m_stack.push_front(tmp);   
            }
            virtual void visit(RegisterAST* r) {
                if(r->getID() == ppc32::r2) {
                    m_stack.push_front(toc_contents);
                } else {
                    m_stack.push_front(0);
                }
                result = m_stack.front();
                parsing_printf("\tTOC visitor visiting register, result is 0x%lx\n",
                            result);
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




// This should only be called on a known indirect branch...
bool IA_IAPI::parseJumpTable(image_basicBlock* currBlk,
                            pdvector<std::pair< Address, EdgeTypeEnum> >& outEdges) const
{

Address initialAddress = current;
static RegisterAST::Ptr toc_reg(new RegisterAST(ppc32::r2));

Address TOC_address = img->getObject()->getTOCoffset();
detail::TOCandOffsetExtractor toc_visitor(TOC_address);
    
// If there are no prior instructions then we can't be looking at a
// jump through a jump table.
if(allInsns.size() < 2) {
    parsing_printf("%s[%d]: allInsns.size() == %d, ret false", FILE__, __LINE__, allInsns.size());
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
    regs.clear();
    patternIter->second->getReadSet(regs);
    if(regs.size() != 1) {
        fprintf(stderr, "expected mtspr to read 1 register, insn is %s\n", patternIter->second->format().c_str());
        return false;
    }
    jumpAddrReg = *(regs.begin());
}
else
{
    parsing_printf("%s[%d]: couldn't find mtspr at prev insn %s, ret false", FILE__, __LINE__,
                    patternIter->second->format().c_str());
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
        if(regs.size() != 1) {
        continue;
        }
        parsing_printf("\tfound 0x%lx: %s, checking for addis previous\n",
                    patternIter->first,
                    patternIter->second->format().c_str());
        toc_visitor.clear();
        patternIter->second->getOperand(2).getValue()->apply(&toc_visitor);
        tableStartAddress = toc_visitor.result;
        patternIter--;
        if(patternIter->second->getOperation().getID() == power_op_addis &&
        patternIter->second->isWritten(*(regs.begin())))
        {
            parsing_printf("\tfound 0x%lx: %s, setting tableStartAddress\n",
                        patternIter->first,
                        patternIter->second->format().c_str());
            toc_visitor.clear();
            patternIter->second->getOperand(2).getValue()->apply(&toc_visitor);
            tableStartAddress += (toc_visitor.result * 0x10000) & 0xFFFF0000;
            parsing_printf("\ttableStartAddress = 0x%lx\n",
                        tableStartAddress);
            break;
        }
        tableStartAddress = 0;
    }
    else if(patternIter->second->getOperation().getID() == power_op_addis)
    {
        regs.clear();
        patternIter->second->getReadSet(regs);
        if(regs.size() != 1) {
            continue;
        }
        parsing_printf("\tfound 0x%lx: %s, checking for addi previous\n",
                        patternIter->first,
                        patternIter->second->format().c_str());
        toc_visitor.clear();
        patternIter->second->getOperand(2).getValue()->apply(&toc_visitor);
        tableStartAddress = toc_visitor.result;
        tableStartAddress *= 10000;
        tableStartAddress &= 0xFFFF0000;
        patternIter--;
        if(patternIter->second->getOperation().getID() == power_op_addi &&
            patternIter->second->isWritten(*(regs.begin())))
        {
            parsing_printf("\tfound 0x%lx: %s, setting tableStartAddress\n",
                            patternIter->first,
                            patternIter->second->format().c_str());
            toc_visitor.clear();
            patternIter->second->getOperand(2).getValue()->apply(&toc_visitor);
            tableStartAddress += toc_visitor.result;
                    parsing_printf("\ttableStartAddress = 0x%lx\n", tableStartAddress);
                    break;
                }
                tableStartAddress = 0;
            }
        }
        jumpStartAddress = tableStartAddress;
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
        if(patternIter == allInsns.begin())
        {
            // If we've already backed up to the beginning, we're not going to find a legit table
            // start address; bail now.
            parsing_printf("%s[%d]: jumpStartAddress insn was first in block w/relative table, ret false\n",
                           FILE__, __LINE__);
            return false;
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
        parsing_printf("%s[%d]: couldn't find table start addr, ret false\n", FILE__, __LINE__);
        return false;
    }
    parsing_printf("%s[%d]: table start addr is 0x%x\n", FILE__, __LINE__, tableStartAddress);
    int maxSwitch = 0;
    
    pdvector<image_edge*> sourceEdges;
    currBlk->getSources(sourceEdges);
    if(sourceEdges.size() != 1) {
        parsing_printf("%s[%d]: jump table not properly guarded, ret false\n", FILE__, __LINE__);
        return false;
    }
    image_basicBlock* sourceBlock = sourceEdges[0]->getSource();
    Address blockStart = sourceBlock->firstInsnOffset();
    const unsigned char* b = (const unsigned char*)(img->getPtrToInstruction(blockStart, context));
    InstructionDecoder dec(b, sourceBlock->getSize(), img->getArch());
    IA_IAPI prevBlock(dec, blockStart, context);
    while(!prevBlock.hasCFT()) {
        prevBlock.advance();
    }

    parsing_printf("%s[%d]: checking for max switch...\n", FILE__, __LINE__);
    patternIter = prevBlock.curInsnIter;
    while( patternIter != prevBlock.allInsns.begin() ) {
        parsing_printf("\t\tchecking insn 0x%x: %s for cond branch\n", patternIter->first,
                            patternIter->second->format().c_str());
        if(patternIter->second->getOperation().getID() == power_op_bc) // make this a true cond. branch check
        {
            if(patternIter != prevBlock.allInsns.begin())
            {
                patternIter--;
                parsing_printf("\t\tchecking insn 0x%x: %s for compare\n", patternIter->first,
                                patternIter->second->format().c_str());
                if(patternIter->second->getOperation().getID() == power_op_cmpi ||
                    patternIter->second->getOperation().getID() == power_op_cmpli)
                {
                    maxSwitch = patternIter->second->getOperand(2).getValue()->eval().convert<int>() + 1;
                    break;
                }
            }
            else
            {
                parsing_printf("\t\t ******** prevBlock.allInsns begins with cond branch!\n");
            }
        }
        patternIter--;
    }

    parsing_printf("%s[%d]: After checking: max switch %d\n", FILE__, __LINE__, maxSwitch);
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
                parsing_printf("%s[%d]: jumpStartPtr (0x%lx) = %p\n", FILE__, __LINE__, jumpStartAddress, jumpStartPtr);
                if (jumpStartPtr)
                    jumpStart = (is64
                            ? *((Address  *)jumpStartPtr)
                    : *((uint32_t *)jumpStartPtr));
                parsing_printf("%s[%d]: jumpStart 0x%lx, initialAddr 0x%lx\n",
                                FILE__, __LINE__, jumpStart, initialAddress);
                if (jumpStartPtr == NULL) {
                    return false;
                }
            }
            void *tableStartPtr = img->getPtrToData(tableStartAddress);
            parsing_printf("%s[%d]: tableStartPtr (0x%lx) = %p\n", FILE__, __LINE__, tableStartAddress, tableStartPtr);
            tableStart = *((Address *)tableStartPtr);
            if (tableStartPtr)
                tableStart = (is64
                        ? *((Address  *)tableStartPtr)
                : *((uint32_t *)tableStartPtr));
            else {
                return false;
            }
            parsing_printf("\t... tableStart 0x%lx\n", tableStart);

    // We're getting an absolute out of the TOC. Figure out
    // whether we're in code or data.
            const fileDescriptor &desc = img->desc();
            Address textStart = desc.code();
            Address dataStart = desc.data();
            parsing_printf("\t... text start 0x%lx, data start 0x%lx\n", textStart, dataStart);

    // I think this is valid on ppc64 linux.  dataStart and codeStart can be 0.
    // assert(jumpStart < dataStart);

            bool tableData = false;

            for(int i=0;i<maxSwitch;i++){
                Address tableEntry = adjustEntry + tableStart + (i * instruction::size());
                parsing_printf("\t\tTable entry at 0x%lx\n", tableEntry);
                if (img->isValidAddress(tableEntry)) {
                    int jumpOffset;
                    if (tableData) {
                        jumpOffset = *((int *)img->getPtrToData(tableEntry));
                    }
                    else {
                        jumpOffset = *((int *)img->getPtrToInstruction(tableEntry));
                    }

                    parsing_printf("\t\t\tjumpOffset 0x%lx\n", jumpOffset);
                    Address res = (Address)(jumpStart + jumpOffset);

                    if (img->isCode(res)) {
                        outEdges.push_back(std::make_pair((Address)(jumpStart+jumpOffset), ET_INDIR));
                        parsing_printf("\t\t\tEntry of 0x%lx\n", (Address)(jumpStart + jumpOffset));
                    }
                }
                else {
                    parsing_printf("\t\tAddress not valid!\n");
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
                parsing_printf("\t\t\t[0x%lx] -> 0x%lx\n", tableEntry, jumpStartAddress+jumpOffset);
                ++entriesAdded;
            }
        }
        if(!entriesAdded)
        {
            parsing_printf("%s[%d]: no entries added from jump table, returning false\n", FILE__, __LINE__);
            return false;
        }
        parsing_printf("%s[%d]: Found %d entries in jump table, returning success\n", FILE__, __LINE__, entriesAdded);
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
    parsing_printf("calling archCheckEntry for 0x%lx, function %s\n", current, context->symTabName().c_str());

#if defined(UNDEF)    
    if (isReturnValueSave())
        context->makesNoCalls_ = false;
    else

    // end cheating

    if (!ah.getInstruction().valid()) return false;

    // We see if we're a procedure linkage table; if so, we are _not_
    // a function (and return false)
#endif
    // We don't consider linkage snippets "functions". 
    dictionary_hash<Address, std::string> *pltFuncs = img->getPltFuncs();
    if (pltFuncs && pltFuncs->defines(current)) {
        return false;
    }
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
