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

#include "IA_powerDetails.h"
#include "Visitor.h"
#include "Register.h"
#include "Dereference.h"
#include "Immediate.h"
#include "BinaryFunction.h"
#include "debug_parse.h"
#include <deque>
#include <boost/bind.hpp>
#include <algorithm>
#include <iterator>
#include <boost/iterator/indirect_iterator.hpp>


using namespace Dyninst;
using namespace InstructionAPI;
using namespace Dyninst::InsnAdapter;
using namespace Dyninst::ParseAPI;


namespace Dyninst
{
    namespace InsnAdapter
    {
        namespace detail
        {
            class TOCandOffsetExtractor : public Dyninst::InstructionAPI::Visitor
            {
            public:
                TOCandOffsetExtractor(Address TOCvalue) : result(0), toc_contents(TOCvalue) {}
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
                    if(r->getID() == toc_reg->getID()) {
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
                RegisterAST::Ptr toc_reg;
            };
        }
    }
};


bool IA_powerDetails::findTableAddrNoTOC(const IA_IAPI* blockToCheck)
{
    std::set<RegisterAST::Ptr> regs;
    std::set<RegisterAST::Ptr> writeregs, readregs;
    RegisterAST::Ptr writereg, readreg;
    int dfgreg;
    std::set<RegisterAST::Ptr>::iterator itw, itr;
    std::set<int>::iterator itd;
    toc_visitor->clear();
    bool foundAddis = false;
    bool foundAddi = false;
    bool foundDep = false;
    while(patternIter != blockToCheck->allInsns.begin())
    {
        patternIter--;
        // Do backward dataflow analysis to match the registers that compute the jump table address
        parsing_printf("\tchecking insn %s at 0x%lx\n", patternIter->second->format().c_str(),
                       patternIter->first);
        // Ignore rlwinm instruction since its used for the index and not the table start.
        // Also, remove it from the backwards DFG
        if(patternIter->second->getOperation().getID() == power_op_rlwinm){
            patternIter->second->getWriteSet(writeregs);
            for(itw=writeregs.begin(); itw!= writeregs.end(); itw++){
                writereg=*(itw);
                for(itd=dfgregs.begin(); itd!= dfgregs.end(); itd++){
                    dfgreg = *itd;
                    if (writereg->getID() == dfgreg) {
                        parsing_printf("found Match - erasing rlwinm  \n");
                        dfgregs.erase(*itd);
                    }
                }
            }
            continue;
        }

        writeregs.clear();
        patternIter->second->getWriteSet(writeregs);
        foundDep =false;

        for(itw=writeregs.begin(); itw!= writeregs.end(); itw++){
            writereg=*(itw);
            parsing_printf("Register Written %s \n", writereg->format().c_str());
            for(itd=dfgregs.begin(); itd!= dfgregs.end(); itd++){
                dfgreg = *itd;
                parsing_printf("DFG has %d \n", dfgreg);
                if (writereg->getID() == dfgreg) {
                    parsing_printf("found Match \n");
                    dfgregs.erase(*itd);
                    readregs.clear();
                    patternIter->second->getReadSet(readregs);
                    for(itr=readregs.begin(); itr!= readregs.end(); itr++){
                        readreg=*(itr);
                        dfgregs.insert(readreg->getID());
                        parsing_printf("Reading %s \n", readreg->format().c_str());
                    }
                    foundDep = true;
                    break;
                }
            }
        }
        // We look for addi-addis combination.
        // These instruction can occur in any order and in any block before the indirect branch.
        // Also, there may be more than one addi instruction.
        // Hence, we use adjustTableStartAddress to keep track of immediate values from addi instructions.
        if(foundDep && !foundAddis &&
           (patternIter->second->getOperation().getID() == power_op_addi ||
            patternIter->second->getOperation().getID() == power_op_addic))
        {
            std::set<RegisterAST::Ptr> tmpregs;
            patternIter->second->getReadSet(tmpregs);
            if(tmpregs.size() != 1) {
                continue;
            }
            regs.clear();
            patternIter->second->getReadSet(regs);
            if(regs.size() != 1) {
                continue;
            }
            parsing_printf("\tfound 0x%lx: %s, checking for addis previous\n",
                           patternIter->first,
                           patternIter->second->format().c_str());
            foundAddi = true;
            toc_visitor->clear();
            patternIter->second->getOperand(2).getValue()->apply(toc_visitor.get());
            adjustTableStartAddress += toc_visitor->result;
        }
        else if(foundDep && !foundAddi && patternIter->second->getOperation().getID() == power_op_addis)
        {
            std::set<RegisterAST::Ptr> tmpregs;
            patternIter->second->getReadSet(tmpregs);
            if(tmpregs.size() != 1) {
                continue;
            }
            regs.clear();
            patternIter->second->getReadSet(regs);
            if(regs.size() != 1) {
                continue;
            }
            parsing_printf("\tfound 0x%lx: %s, checking for addi previous\n",
                           patternIter->first,
                           patternIter->second->format().c_str());
            foundAddis = true;
            toc_visitor->clear();
            patternIter->second->getOperand(2).getValue()->apply(toc_visitor.get());
            tableStartAddress = toc_visitor->result;
            tableStartAddress *= 10000;
            tableStartAddress &= 0xFFFF0000;

        } else if( foundDep && foundAddi &&
                   patternIter->second &&
                   (patternIter->second->getOperation().getID() == power_op_addis) &&
                   patternIter->second->isWritten(*(regs.begin())))
        {
            foundAddis = true;
            parsing_printf("\tfound 0x%lx: %s, setting tableStartAddress\n",
                           patternIter->first,
                           patternIter->second->format().c_str());
            toc_visitor->clear();
            patternIter->second->getOperand(2).getValue()->apply(toc_visitor.get());
            tableStartAddress += (toc_visitor->result * 0x10000) & 0xFFFF0000;
            parsing_printf("\ttableStartAddress = 0x%lx\n",
                           tableStartAddress);
            break;
        } else if( foundDep && foundAddis &&
                   patternIter->second &&
                   ((patternIter->second->getOperation().getID() == power_op_addi) ||
                    (patternIter->second->getOperation().getID() == power_op_addic)) &&
                   patternIter->second->isWritten(*(regs.begin())))
        {
            foundAddi = true;
            parsing_printf("\tfound 0x%lx: %s, setting tableStartAddress\n",
                           patternIter->first,
                           patternIter->second->format().c_str());
            toc_visitor->clear();
            patternIter->second->getOperand(2).getValue()->apply(toc_visitor.get());
            tableStartAddress += toc_visitor->result;
            parsing_printf("\ttableStartAddress = 0x%lx\n", tableStartAddress);
            break;
        }
    }
    if (!foundAddi || !foundAddis)
        tableStartAddress = 0;
    else
        tableStartAddress += adjustTableStartAddress;

    parsing_printf(" TABLE START 0x%lx 0x%lx %ld\n", tableStartAddress, adjustTableStartAddress, adjustTableStartAddress);

    // If we've found an addi/addis combination and it's a relative table, look for a mfspr/thunk combination that
    // feeds that...
    if(tableStartAddress && tableIsRelative)
    {
        parsing_printf("\ttableStartAddress non-zero, tableIsRelative true\n");
        bool foundThunk = false;
        bool foundMFSPR = false;
        Address GOTaddress = 0;
        while(patternIter != blockToCheck->allInsns.begin())
        {
            patternIter--;
            if(patternIter->second->getOperation().getID() == power_op_mfspr &&
               patternIter->second->isWritten(*(regs.begin())))
            {
                foundMFSPR = true;
                break;
            }
        }
        while(patternIter != blockToCheck->allInsns.begin())
        {
            patternIter--;
            if(patternIter->second->getCategory() == c_CallInsn) // mid-block call, must be a thunk
            {
                patternIter++;
                parsing_printf("\tfound thunk/mfspr combo, adjusting tableStartAddress by 0x%lx\n", patternIter->first);
                GOTaddress = tableStartAddress + patternIter->first;
                foundThunk = true;
                break;
            }
        }
        if(foundThunk && foundMFSPR)
        {
            toc_visitor->toc_reg = *(regs.begin());
            toc_reg = toc_visitor->toc_reg;
            toc_visitor->toc_contents = GOTaddress;
            tableStartAddress = 0;
            patternIter = currentBlock->curInsnIter;
            parsing_printf("\t calling parseRelativeTableIdiom with toc_reg %s\n", toc_visitor->toc_reg->format().c_str());
            return parseRelativeTableIdiom();
        }
    }
    else
    {
        parsing_printf("\ttableStartAddress = 0x%lx, tableIsRelative = %s\n", tableStartAddress,
                       tableIsRelative ? "true" : "false");
    }
    return tableStartAddress == 0;
}

bool IA_powerDetails::parseRelativeTableIdiom()
{
    bool foundAddress = false;
    while(patternIter != currentBlock->allInsns.begin())
    {
        patternIter--;
        parsing_printf("\t checking 0x%lx: %s for lwz/ld\n", patternIter->first, patternIter->second->format().c_str());
        if((patternIter->second->getOperation().getID() == power_op_lwz ||
            patternIter->second->getOperation().getID() == power_op_ld) &&
           patternIter->second->isRead(toc_reg))
        {
            toc_visitor->clear();
            patternIter->second->getOperand(1).getValue()->apply(toc_visitor.get());
            parsing_printf("%s[%d]: setting jumpStartAddress to 0x%lx, insn %s, TOC 0x%lx\n", FILE__, __LINE__,
                           toc_visitor->result, patternIter->second->format().c_str(), toc_visitor->toc_contents);
            jumpStartAddress = toc_visitor->result;
            foundAddress = true;
            tableStartAddress = jumpStartAddress;
            adjustEntry = 0;
            break;
        }
    }
    if(patternIter == currentBlock->allInsns.begin())
    {
        if (foundAddress) {
            return true;
        } else {

            // If we've already backed up to the beginning, we're not going to find a legit table
            // start address; bail now.
            parsing_printf("%s[%d]: jumpStartAddress insn was first in block w/relative table, ret false\n",
                           FILE__, __LINE__);
            return false;
        }
    }
    // Anyone know what this does?
    patternIter--;
    if((patternIter->second->getOperation().getID() == power_op_lwz ||
        patternIter->second->getOperation().getID() == power_op_ld))
    {
        toc_visitor->clear();
        patternIter->second->getOperand(1).getValue()->apply(toc_visitor.get());
        adjustEntry = toc_visitor->result;
        foundAdjustEntry = true;
        parsing_printf("%s[%d]: setting adjustEntry to 0x%lx, insn %s, TOC 0x%lx\n", FILE__, __LINE__,
                       toc_visitor->result, patternIter->second->format().c_str(), toc_visitor->toc_contents);
    }

    while(patternIter != currentBlock->allInsns.begin()){
        patternIter--;
        if((patternIter->second->getOperation().getID() == power_op_lwz ||
            patternIter->second->getOperation().getID() == power_op_ld) &&
           patternIter->second->isRead(toc_reg))
        {
            toc_visitor->clear();
            patternIter->second->getOperand(1).getValue()->apply(toc_visitor.get());
            tableStartAddress = toc_visitor->result;
            parsing_printf("%s[%d]: setting tableStartAddress to 0x%lx, insn %s, TOC 0x%lx\n", FILE__, __LINE__,
                           toc_visitor->result, patternIter->second->format().c_str(), toc_visitor->toc_contents);
            break;
        }
    }
    return true;
}

namespace detail_ppc
{
    bool isNonCallEdge(ParseAPI::Edge* e)
    {
        return e->type() != CALL;
    }
    bool leadsToVisitedBlock(ParseAPI::Edge* e, const std::set<Block*>& visited)
    {
        Block* src = e->src();
        return visited.find(src) != visited.end();
    }
    void processPredecessor(Edge* e, std::set<Block*>& visited, std::deque<Block*>& worklist)
    {
        parsing_printf("\t\tblock %x, edge type %s\n",
                       e->src()->start(),
                       format(e->type()).c_str());

        // FIXME debugging assert
        assert(isNonCallEdge(e));

        // FIXME check this algorithm... O(log n) lookup in visited
        if(!leadsToVisitedBlock(e, visited))
        {
            worklist.push_back(e->src());
            visited.insert(e->src());
        }
    }
};

bool IA_powerDetails::scanForAdjustOrBase(IA_IAPI::allInsns_t::const_iterator start,
                                          IA_IAPI::allInsns_t::const_iterator end,
                                          RegisterAST::Ptr &jumpAddrReg) {
    std::set<RegisterAST::Ptr> scratchRegs;
    std::set<RegisterAST::Ptr> loadRegs;
    loadRegs.insert(jumpAddrReg);
    for (; start != end; --start) {
        InstructionAPI::Instruction::Ptr insn = start->second;
        parsing_printf("\t\t Examining 0x%lx / %s\n",
                       start->first, start->second->format().c_str());

        if ((insn->getOperation().getID() == power_op_ld ||
             insn->getOperation().getID() == power_op_ldx) &&
            insn->isWritten(jumpAddrReg)) {
            scratchRegs.clear();
            insn->getReadSet(scratchRegs);
            loadRegs.insert(scratchRegs.begin(), scratchRegs.end());
            parsing_printf("Found a load; now have %d load regs\n", loadRegs.size());
        }
        else if(insn->getOperation().getID() == power_op_addi &&
                !foundAdjustEntry) {
            parsing_printf("Found add immediate (%d load regs)...\n", loadRegs.size());
            scratchRegs.clear();
            insn->getWriteSet(scratchRegs);

            bool found = false;
            // This is apparently broken
            for (std::set<RegisterAST::Ptr>::iterator iter = loadRegs.begin(); iter != loadRegs.end(); ++iter) {
                RegisterAST *tmp = (*iter).get();
                RegisterAST *cmp = (*(scratchRegs.begin())).get();
                if (*tmp == *cmp) {
                    found = true;
                    break;
                }
            }
            if (!found) continue;

            parsing_printf("... that adds to a load reg\n");
            foundAdjustEntry = true;
            toc_visitor->clear();
            parsing_printf("... with operand %s\n", insn->getOperand(1).format(insn->getArch(), start->first).c_str());
            insn->getOperand(1).getValue()->apply(toc_visitor.get());
            adjustEntry = toc_visitor->result;
            if (!adjustEntry)
                insn->getOperand(2).getValue()->apply(toc_visitor.get());
            adjustEntry = toc_visitor->result;
        }
        else if((insn->getOperation().getID() == power_op_lwz ||
                 insn->getOperation().getID() == power_op_ld) &&
                insn->isRead(toc_reg) &&
                insn->isWritten(jumpAddrReg))
        {
            parsing_printf("\t found TOC load at %s\n", insn->format().c_str());
            toc_visitor->clear();
            insn->getOperand(1).getValue()->apply(toc_visitor.get());
            tableStartAddress = toc_visitor->result;
            break;
        }
    }
    return true;
}

// Like the above, but a wider net
bool IA_powerDetails::findTableBase(IA_IAPI::allInsns_t::const_iterator start,
                                    IA_IAPI::allInsns_t::const_iterator end) {
    for (; start != end; --start) {
        if(!start->second) continue;
        parsing_printf("\t\t Examining 0x%lx / %s\n",
                       start->first, start->second->format().c_str());
        if((start->second->getOperation().getID() == power_op_lwz ||
            start->second->getOperation().getID() == power_op_ld) &&
           start->second->isRead(toc_reg)) {
            parsing_printf("\t found TOC load at %s\n", start->second->format().c_str());
            toc_visitor->clear();
            start->second->getOperand(1).getValue()->apply(toc_visitor.get());
            tableStartAddress = toc_visitor->result;
            break;
        }
    }
    return true;
}

bool IA_powerDetails::parseJumpTable(Function *,
                                     Block* currBlk,
                                     std::vector<std::pair< Address, EdgeTypeEnum> >& outEdges)
{
    return parseJumpTable(currBlk, outEdges);
}



// This should only be called on a known indirect branch...
bool IA_powerDetails::parseJumpTable(Block* currBlk,
                                     std::vector<std::pair< Address, EdgeTypeEnum> >& outEdges)
{

    Address initialAddress = currentBlock->current;
    toc_reg.reset(new RegisterAST(ppc32::r2));

    TOC_address = currentBlock->_obj->cs()->getTOC();
    toc_visitor.reset(new detail::TOCandOffsetExtractor(TOC_address));
    toc_visitor->toc_reg = toc_reg;

    // If there are no prior instructions then we can't be looking at a
    // jump through a jump table.
    if(currentBlock->allInsns.size() < 2) {
        parsing_printf("%s[%d]: allInsns.size() == %d, ret false", FILE__, __LINE__, currentBlock->allInsns.size());
        return false;
    }


    // Check if the previous instruction is a move to CTR or LR;
    // if it is, then this is the pattern we're familiar with.  The
    // register being moved into CTR or LR has the address to jump to.
    patternIter = currentBlock->curInsnIter;
    patternIter--;
    RegisterAST::Ptr jumpAddrReg;
    std::set<RegisterAST::Ptr> regs;
    if(patternIter->second->getOperation().getID() == power_op_mtspr)
    {
        regs.clear();
        patternIter->second->getReadSet(regs);
        if(regs.size() != 1) {
            parsing_printf("expected mtspr to read 1 register, insn is %s\n", patternIter->second->format().c_str());
            return false;
        }
        jumpAddrReg = *(regs.begin());
        parsing_printf("%s[%d]: JUMPREG %s mtspr at prev insn %s \n", FILE__, __LINE__, jumpAddrReg->format().c_str(), patternIter->second->format().c_str());
        dfgregs.insert(jumpAddrReg->getID());
    }
    else
    {
        parsing_printf("%s[%d]: couldn't find mtspr at prev insn %s, ret false", FILE__, __LINE__,
                       patternIter->second->format().c_str());
        return false;
    }
    assert(jumpAddrReg);
    // In the pattern we've seen, if the instruction previous to this is
    // an add with a result that ends up being used as the jump address,
    // then we're adding a relative value we got from the table to a base
    // address to get the jump address; in other words, the contents of
    // the jump table are relative.
    tableIsRelative = false;
    if(patternIter != currentBlock->allInsns.begin())
    {
        patternIter--;
        if(patternIter->second->getOperation().getID() == power_op_add &&
           patternIter->second->isWritten(*(regs.begin())))
        {
            tableIsRelative = true;
        }
    }
    parsing_printf(" TableIsRelative %d\n", tableIsRelative);

    patternIter = currentBlock->curInsnIter;

    jumpStartAddress = 0;
    adjustEntry = 0;
    tableStartAddress = 0;
    adjustTableStartAddress = 0;
    foundAdjustEntry = false;

    parsing_printf("\t TOC_address 0x%lx\n", TOC_address);
    if(!TOC_address)
    {
        // Find addi-addis instructions to determine the jump table start address.
        // These instructions can be anywhere in the function before the
        // indirect jump.Hence parse through the current block and previous block
        // till we reach the function entry.
        Block* worklistBlock = currBlk;
        std::set <Block*> visited;
        std::deque<Block*> worklist;
        worklist.insert(worklist.begin(), worklistBlock);
        visited.insert(worklistBlock);
        Intraproc epred;
        parsing_printf("Looking for table start address over blocks to function entry\n");
        while(!worklist.empty())
        {
            worklistBlock= worklist.front();
            worklist.pop_front();
            parsing_printf("\tAddress low 0x%lx high 0x%lx current block 0x%lx low 0x%lx high 0x%lx \n", worklistBlock->low(), worklistBlock->high(), currentBlock->current, currBlk->low(), currBlk->high());
            Address blockStart = worklistBlock->start();
            const unsigned char* b = (const unsigned char*)(currentBlock->_isrc->getPtrToInstruction(blockStart));
            parsing_printf(" Block start 0x%lx \n", blockStart);
            InstructionDecoder dec(b, worklistBlock->size(), currentBlock->_isrc->getArch());
            IA_IAPI IABlock(dec, blockStart, currentBlock->_obj, currentBlock->_cr, currentBlock->_isrc, worklistBlock);

            while(IABlock.getInstruction() && !IABlock.hasCFT()) {
                IABlock.advance();
            }

            patternIter = IABlock.curInsnIter;
            findTableAddrNoTOC(&IABlock);
            if(!jumpStartAddress)
            {
                jumpStartAddress = tableStartAddress;
            }
            if (tableStartAddress != 0) {
                jumpStartAddress = tableStartAddress;
                parsing_printf("\t\tjumpStartAddress 0x%lx \n", jumpStartAddress);
                break;
            }
            std::for_each(boost::make_filter_iterator(epred, worklistBlock->sources().begin(), worklistBlock->sources().end()),
                          boost::make_filter_iterator(epred, worklistBlock->sources().end(), worklistBlock->sources().end()),
                          boost::bind(detail_ppc::processPredecessor, _1, boost::ref(visited), boost::ref(worklist)));

        }

    }
    else if (tableIsRelative) {
        if(!parseRelativeTableIdiom())
        {
            return false;
        }
    } else {
        parsing_printf("\t Table is not relative and we know the TOC is 0x%lx, searching for table base\n",
                       TOC_address);
        foundAdjustEntry = false;

        scanForAdjustOrBase(patternIter, currentBlock->allInsns.begin(), jumpAddrReg);

        if (!tableStartAddress) {
            // Keep looking in the immediate predecessor - XLC
            for (Block::edgelist::const_iterator e_iter = currBlk->sources().begin();
                 e_iter != currBlk->sources().end(); ++e_iter) {
                Address blockStart = (*e_iter)->src()->start();
                const unsigned char* b = (const unsigned char*)(currentBlock->_isrc->getPtrToInstruction(blockStart));
                InstructionDecoder dec(b, (*e_iter)->src()->size(), currentBlock->_isrc->getArch());
                IA_IAPI IABlock(dec, blockStart, currentBlock->_obj, currentBlock->_cr, currentBlock->_isrc, (*e_iter)->src());

                // Cache instructions
                while(IABlock.getInstruction() && !IABlock.hasCFT()) {
                    IABlock.advance();
                }

                IA_IAPI::allInsns_t::const_iterator localIter = IABlock.curInsnIter;
                findTableBase(localIter, IABlock.allInsns.begin());
            }
        }
    }

    const Block::edgelist & sourceEdges = currBlk->sources();
    if(sourceEdges.size() != 1 || (*sourceEdges.begin())->type() == CALL) {
        parsing_printf("%s[%d]: jump table not properly guarded, ret false\n", FILE__, __LINE__);
        return false;
    }



    // We could also set this = jumpStartAddress...
    if (tableStartAddress == 0)  {
        parsing_printf("%s[%d]: couldn't find table start addr, ret false\n", FILE__, __LINE__);
        return false;

    }

    parsing_printf("%s[%d]: table start addr is 0x%x\n", FILE__, __LINE__, tableStartAddress);
    int maxSwitch = 0;

    Block* sourceBlock = (*sourceEdges.begin())->src();
    Address blockStart = sourceBlock->start();
    const unsigned char* b = (const unsigned char*)(currentBlock->_isrc->getPtrToInstruction(blockStart));
    InstructionDecoder dec(b, sourceBlock->size(), currentBlock->_isrc->getArch());
    IA_IAPI prevBlock(dec, blockStart,currentBlock->_obj,currentBlock->_cr,currentBlock->_isrc, sourceBlock);
    while(!prevBlock.hasCFT() && prevBlock.getInstruction()) {
        prevBlock.advance();
    }

    parsing_printf("%s[%d]: checking for max switch...\n", FILE__, __LINE__);
    bool foundBranch = false;
    IA_IAPI::allInsns_t::reverse_iterator iter;
    for(iter = prevBlock.allInsns.rbegin(); iter != prevBlock.allInsns.rend(); iter++)

    {
        parsing_printf("\t\tchecking insn 0x%x: %s for cond branch + compare\n", iter->first,
                       iter->second->format().c_str());
        if(iter->second->getOperation().getID() == power_op_bc) // make this a true cond. branch check
        {
            foundBranch = true;
        } else if(foundBranch &&
                  (iter->second->getOperation().getID() == power_op_cmpi ||
                   iter->second->getOperation().getID() == power_op_cmpli))
        {
            maxSwitch = iter->second->getOperand(2).getValue()->eval().convert<int>() + 1;
            break;

        }
    }

    parsing_printf("%s[%d]: After checking: max switch %d\n", FILE__, __LINE__, maxSwitch);
    if(!maxSwitch){
        return false;
    }

    Address jumpStart = 0;
    Address tableStart = 0;
    bool is64 = (currentBlock->_isrc->getAddressWidth() == 8);
    std::vector<std::pair< Address, EdgeTypeEnum> > edges;

    if(TOC_address)
    {
        if (tableIsRelative) {
            void *jumpStartPtr = currentBlock->_isrc->getPtrToData(jumpStartAddress);
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
        void *tableStartPtr = currentBlock->_isrc->getPtrToData(tableStartAddress);
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

        bool tableData = false;
        for(int i=0;i<maxSwitch;i++){
            Address tableEntry = adjustEntry + tableStart + (i * 4 /*instruction::size()*/);
            parsing_printf("\t\tTable entry at 0x%lx\n", tableEntry);
            if (currentBlock->_isrc->isValidAddress(tableEntry)) {
                int jumpOffset;
                if (tableData) {
                    jumpOffset = *((int *)currentBlock->_isrc->getPtrToData(tableEntry));
                }
                else {
                    jumpOffset = *((int *)currentBlock->_isrc->getPtrToInstruction(tableEntry));
                }

                parsing_printf("\t\t\tjumpOffset 0x%lx\n", jumpOffset);
                Address res = (Address)(jumpStart + jumpOffset);

                if (currentBlock->_isrc->isCode(res)) {
                    edges.push_back(std::make_pair((Address)(jumpStart+jumpOffset), INDIRECT));
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
        jumpStart = jumpStartAddress;
        tableStart = tableStartAddress;
        parsing_printf(" jumpStartaddress 0x%lx tableStartAddress 0x%lx \n", jumpStartAddress, tableStartAddress);
        if(toc_visitor->toc_contents)
        {
            void* tmp = NULL;
            if(currentBlock->_isrc->isValidAddress(jumpStartAddress))
            {
                tmp = currentBlock->_isrc->getPtrToData(jumpStartAddress);
                if(!tmp)
                {
                    tmp = currentBlock->_isrc->getPtrToInstruction(jumpStartAddress);
                }
                if(tmp)
                {
                    jumpStart = *((Address*)tmp);
                    parsing_printf("\t\tjumpStart adjusted to 0x%lx\n", jumpStart);
                }
            }
            if(currentBlock->_isrc->isValidAddress(tableStartAddress))
            {
                tmp = currentBlock->_isrc->getPtrToData(tableStartAddress);
                if(!tmp)
                {
                    tmp = currentBlock->_isrc->getPtrToInstruction(tableStartAddress);
                }
                if(tmp)
                {
                    tableStart = *((Address*)tmp);
                    parsing_printf("\t\ttableStart adjusted to 0x%lx\n", jumpStart);
                }
            }
        }


        if (jumpStart == 0) {
#if defined(WITH_SYMTAB_API)
            // If jump table address is a relocation entry, this will be filled by the loader
            // This case is common in shared library where the table address is in the GOT section which is filled by the loader
            // Find the relocation entry for this address and look up its value

            Block* sourceBlock = (*sourceEdges.begin())->src();
            Address blockStart = sourceBlock->start();
            const unsigned char* b = (const unsigned char*)(currentBlock->_isrc->getPtrToInstruction(blockStart));
            InstructionDecoder dec(b, sourceBlock->size(), currentBlock->_isrc->getArch());
            IA_IAPI IABlock(dec, blockStart,currentBlock->_obj,currentBlock->_cr,currentBlock->_isrc, sourceBlock);

            SymtabCodeSource *scs = dynamic_cast<SymtabCodeSource *>(IABlock._obj->cs());
            SymtabAPI::Symtab * symtab = scs->getSymtabObject();
            std::vector<SymtabAPI::Region *> regions;
            symtab->getAllRegions(regions);
            for (unsigned index = 0 ; index < regions.size(); index++) {
                if (regions[index]->getRegionType() == SymtabAPI::Region::RT_RELA ||
                    regions[index]->getRegionType() == SymtabAPI::Region::RT_REL) {
                    std::vector<SymtabAPI::relocationEntry> relocs =
                            regions[index]->getRelocations();
                    parsing_printf(" \t\trelocation size %d looking for 0x%lx\n", relocs.size(), jumpStartAddress);
                    for (unsigned i = 0; i < relocs.size(); ++i) {
                        parsing_printf(" \t 0x%lx => 0x%lx addend 0x%lx \n", relocs[i].rel_addr(),relocs[i].target_addr(), relocs[i].addend());
                        if (relocs[i].rel_addr() == jumpStartAddress) {
                            jumpStart = relocs[i].addend();
                            break;
                        }
                    }
                    break;
                }
            }
#else
            // Can't parse relocation entries without Symtab
        return false;
#endif
        }
        if (tableStart == 0) tableStart = jumpStart;

        if (!tableIsRelative) {
            jumpStart = 0;
        }
        parsing_printf(" jumpStartaddress 0x%lx tableStartAddress 0x%lx \n", jumpStart, tableStart);

        int entriesAdded = 0;
        for(int i = 0; i < maxSwitch; i++)
        {
            void* ptr = NULL;
            Address tableEntry = tableStart + i*4; // instruction::size();
            if(currentBlock->_isrc->isValidAddress(tableEntry))
            {
                ptr = currentBlock->_isrc->getPtrToInstruction(tableEntry);
            }
            if(ptr)
            {
                int jumpOffset = *((int *)ptr);
                edges.push_back(std::make_pair((Address)(jumpStart+jumpOffset), INDIRECT));
                parsing_printf("\t\t\t[0x%lx] -> 0x%lx (0x%lx in table)\n", tableEntry,
                               jumpStart+jumpOffset,
                               jumpOffset);
                ++entriesAdded;
            }
            else
            {
                parsing_printf("\t\t\t[0x%lx] -> [INVALID]\n", tableEntry);
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
    for (std::vector<std::pair<Address, EdgeTypeEnum> >::iterator iter = edges.begin();
         iter != edges.end(); iter++) {
        if ((iter->first) % 4) {
            parsing_printf("Warning: found unaligned jump table destination 0x%lx for jump at 0x%lx, disregarding table\n",
                           iter->first, initialAddress);
            return false;
        }
    }
    // If we have found a jump table, add the targets to outEdges
    for (std::vector<std::pair<Address, EdgeTypeEnum> >::iterator iter = edges.begin();
         iter != edges.end(); iter++) {
        parsing_printf("Adding out edge %d/0x%lx\n", iter->second, iter->first);
        outEdges.push_back(*iter);
    }
    return true;
}
