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
#include "debug_parse.h"

#include <deque>

using namespace Dyninst;
using namespace InstructionAPI;
using namespace Dyninst::InsnAdapter;
using namespace Dyninst::ParseAPI;

class zeroAllGPRegisters : public InstructionAPI::Visitor
{
    public:
    zeroAllGPRegisters(Address ip) : defined(true), m_ip(ip) {}
    virtual ~zeroAllGPRegisters() {}
    bool defined;
    std::deque<long> results;
    Address m_ip;
    long getResult() {
        if(results.empty()) return 0;
        return results.front();
    }
    bool isDefined() {
        return defined && (results.size() == 1);
    }
    virtual void visit(BinaryFunction* b)
    {
        if(!defined) return;
        long arg1 = results.back();
        results.pop_back();
        long arg2 = results.back();
        results.pop_back();
        if(b->isAdd())
        {
            results.push_back(arg1+arg2);
        }
        else if(b->isMultiply())
        {
            results.push_back(arg1*arg2);
        }
        else
        {
            defined = false;
        }
    }
    virtual void visit(Immediate* i)
    {
        if(!defined) return;
        results.push_back(i->eval().convert<long>());
    }
    virtual void visit(RegisterAST* r)
    {
        if(!defined) return;
        if(r->getID() == x86::eip ||
           r->getID() == x86_64::eip ||
           r->getID() == x86_64::rip)
        {
            results.push_back(m_ip);
            return;
        }
        results.push_back(0);
    }
    virtual void visit(Dereference* )
    {
        defined = false;
    }
    
};

bool IA_IAPI::isFrameSetupInsn(Instruction::Ptr i) const
{
    if(i->getOperation().getID() == e_mov)
    {
        if(i->isRead(stackPtr[_isrc->getArch()]) &&
           i->isWritten(framePtr[_isrc->getArch()]))
        {
            return true;
        }
    }
    return false;
}

bool IA_IAPI::isNop() const
{
    Instruction::Ptr ci = curInsn();

    // TODO: add LEA no-ops
    assert(ci);
    if(ci->getOperation().getID() == e_nop)
        return true;
    if(ci->getOperation().getID() == e_lea)
    {
        std::set<Expression::Ptr> memReadAddr;
        ci->getMemoryReadOperands(memReadAddr);
        std::set<RegisterAST::Ptr> writtenRegs;
        ci->getWriteSet(writtenRegs);
        
        if(memReadAddr.size() == 1 && writtenRegs.size() == 1)
        {
            if(**(memReadAddr.begin()) == **(writtenRegs.begin()))
            {
                return true;
            }
            // TODO: check for zero displacement--do we want to bind here?
        }
    }
    return false;
}

bool IA_IAPI::isMovAPSTable(std::vector<std::pair< Address, EdgeTypeEnum > >& outEdges) const
{
    /**
    * AMD-64 gcc emits a re-accuring idiom of:
     *         jmpq   *%r8
     *         movaps %xmm7,-0xf(%rax)
     *         movaps %xmm6,-0x1f(%rax)
     *         movaps %xmm5,-0x2f(%rax)
     *         movaps %xmm4,-0x3f(%rax)
     *         movaps %xmm3,-0x4f(%rax)
     *         movaps %xmm2,-0x5f(%rax)
     *         movaps %xmm1,-0x6f(%rax)
     *         movaps %xmm0,-0x7f(%rax)
     *         <other>
     *
     * The jump register is calculated in such a way that it'll be difficult
     * for our analysis to figure it out.  Instead we'll recognize the pattern
     * of the 'movaps'.  Note that the following instruction is also a valid jump
     * target
     **/
    parsing_printf("\tChecking for movaps table at 0x%lx...\n", current);
    std::set<Address> found;
    const unsigned char *bufferBegin =
            (const unsigned char *)_isrc->getPtrToInstruction(current);
    if( bufferBegin == NULL ) {
        parsing_printf("%s[%d]: failed to get pointer to instruction by offset\n",
                       FILE__, __LINE__);
        return false;
    }

    unsigned int size = (_cr->offset() + _cr->length()) - current;
    InstructionDecoder d(bufferBegin, size, _isrc->getArch());
    Address cur = current;
    unsigned last_insn_size = 0;
    InstructionAPI::Instruction::Ptr i = d.decode();
    cur += i->size();
    InstructionAPI::Instruction::Ptr insn;
    while (NULL != (insn = d.decode())) {
        //All insns in sequence are movaps
        parsing_printf("\t\tChecking instruction %s\n", insn->format().c_str());
        if (insn->getOperation().getID() != e_movapd &&
            insn->getOperation().getID() != e_movaps)
        {
            break;
        }
        //All insns are same size
        if (last_insn_size == 0)
            last_insn_size = insn->size();
        else if (last_insn_size != insn->size())
            break;

        found.insert(cur);

        cur += insn->size();
    }
    if (found.size() == 8) {
        found.insert(cur);
        //It's a match
        for (std::set<Address>::iterator i=found.begin(); i != found.end(); i++) {
            outEdges.push_back(std::make_pair(*i, INDIRECT));
        }
        parsing_printf("\tfound\n");
        return true;
    }
    parsing_printf("\tnot found (%d insns)\n", found.size());
    return false;
}

bool IA_IAPI::isTableInsn(Instruction::Ptr i) const
{
    if(i->getOperation().getID() == e_mov && i->readsMemory() && !i->writesMemory())
    {
        return true;
    }
    if(i->getOperation().getID() == e_lea)
    {
        return true;
    }
    return false;
}
        
std::map<Address, Instruction::Ptr>::const_iterator IA_IAPI::findTableInsn() const
{
    // Check whether the jump is our table insn!
    Expression::Ptr cft = curInsn()->getControlFlowTarget();
    if(cft)
    {
        std::vector<InstructionAST::Ptr> tmp;
        cft->getChildren(tmp);
        if(tmp.size() == 1)
        {
            Expression::Ptr cftAddr = dyn_detail::boost::dynamic_pointer_cast<Expression>(tmp[0]);
            zeroAllGPRegisters z(current);
            cftAddr->apply(&z);
            parsing_printf("\tChecking indirect jump %s for table insn\n", curInsn()->format().c_str());
            if(z.isDefined() && z.getResult())
            {
                parsing_printf("\tAddress in jump\n");
                return allInsns.find(current);
            }
        }
    }
    std::map<Address, Instruction::Ptr>::const_iterator c =
            allInsns.find(current);
    while(!isTableInsn(c->second) && c != allInsns.begin())
    {
        --c;
    }
    if(isTableInsn(c->second))
    {
        return c;
    }
    return allInsns.end();
}
        
// This should only be called on a known indirect branch...
bool IA_IAPI::parseJumpTable(Block* currBlk,
    std::vector<std::pair< Address, EdgeTypeEnum> >& outEdges) const
{
    if(isIPRelativeBranch())
    {
        return false;
    }

    if(isMovAPSTable(outEdges))
    {
        return true;
    }
    bool foundJCCAlongTaken = false;
    Instruction::Ptr tableInsn;
    Address tableInsnAddr;
    std::map<Address, Instruction::Ptr>::const_iterator tableLoc = findTableInsn();
    if(tableLoc == allInsns.end())
    {
        parsing_printf("\tunable to find table insn\n");
        return false;
    }
    tableInsnAddr = tableLoc->first;
    tableInsn = tableLoc->second;
    Instruction::Ptr maxSwitchInsn, branchInsn;
    boost::tie(maxSwitchInsn, branchInsn, foundJCCAlongTaken) = findMaxSwitchInsn(currBlk);
    if(!maxSwitchInsn || !branchInsn)
    {
        parsing_printf("\tunable to fix max switch size\n");
        return false;
    }
    Address thunkOffset;
    Address thunkInsnAddr;
    boost::tie(thunkInsnAddr, thunkOffset) = findThunkAndOffset(currBlk);
    if(thunkInsnAddr != 0)
    {
        std::map<Address, Instruction::Ptr>::const_iterator thunkLoc =
                allInsns.find(thunkInsnAddr);
        if(thunkLoc != allInsns.end())
        {
            if(thunkLoc->second && thunkLoc->second->getOperation().getID() == e_lea)
            {
                tableLoc = thunkLoc;
                tableInsnAddr = thunkInsnAddr;
                tableInsn = thunkLoc->second;
            }
        }
    }
    parsing_printf("\ttableInsn %s at 0x%lx\n",tableInsn->format().c_str(), tableInsnAddr);
    if(thunkOffset) {
        parsing_printf("\tThunk-calculated table base address: 0x%lx\n",
                       thunkOffset);
    }
    unsigned tableSize = 0, tableStride = 0;
    bool ok = computeTableBounds(maxSwitchInsn, branchInsn, tableInsn, foundJCCAlongTaken,
                                 tableSize, tableStride);
    if(!ok)
    {
        return false;
    }
    std::map<Address, Instruction::Ptr>::const_iterator cur = allInsns.find(current);
    
    while(tableLoc != cur)
    {
        tableLoc++;
        if(tableLoc->second->getOperation().getID() == e_lea)
        {
            parsing_printf("\tchecking instruction %s at 0x%lx for IP-relative LEA\n", tableLoc->second->format().c_str(),
                           tableLoc->first);
            Expression::Ptr IPRelAddr = tableLoc->second->getOperand(1).getValue();
            IPRelAddr->bind(thePC[_isrc->getArch()].get(), Result(s64, tableLoc->first + tableLoc->second->size()));
            Result iprel = IPRelAddr->eval();
            if(iprel.defined)
            {
                parsing_printf("\trevising tableInsn to %s at 0x%lx\n",tableLoc->second->format().c_str(), tableLoc->first);
                tableInsn = tableLoc->second;
                tableInsnAddr = tableLoc->first;
            }

        }
        else
        {
            parsing_printf("\tChecking for sign-extending mov at 0x%lx...\n", tableLoc->first);
            if(tableLoc->second->getOperation().getID() == e_movsxd ||
               tableLoc->second->getOperation().getID() == e_movsx)
            {
                std::set<Expression::Ptr> movsxReadAddr;
                tableLoc->second->getMemoryReadOperands(movsxReadAddr);

                if(movsxReadAddr.empty()) {
                    // should be a register-register movsx[d]
                    // from either 16- or 32-bit source operand
                    Expression::Ptr op1 = 
                        tableLoc->second->getOperand(0).getValue(); 
                    Expression::Ptr op2 = 
                        tableLoc->second->getOperand(1).getValue();
            
                    if(op1 && op2) {
                        int sz1 = op1->eval().size();
                        int sz2 = op2->eval().size();
                        parsing_printf("\t\tfound %d-byte to %d-byte move, revised stride to %d\n",
                        
                            sz2,sz1,sz2);
                        tableStride = sz2;
                    }
                }

                static Immediate::Ptr four(new Immediate(Result(u8, 4)));
                static Expression::Ptr dummy4(new DummyExpr());
                static Expression::Ptr dummy2(new DummyExpr());
                static Immediate::Ptr two(new Immediate(Result(u8, 2)));
                static BinaryFunction::funcT::Ptr multiplier(new BinaryFunction::multResult());
                static BinaryFunction::Ptr scaleCheck4(new BinaryFunction(four, dummy4, (tableStride == 8) ? u64: u32,
                        multiplier));
                static BinaryFunction::Ptr scaleCheck2(new BinaryFunction(two, dummy2, (tableStride == 8) ? u64: u32,
                        multiplier));
                for(std::set<Expression::Ptr>::const_iterator readExp =
                    movsxReadAddr.begin();
                    readExp != movsxReadAddr.end();
                    ++readExp)
                {
                    if((*readExp)->isUsed(scaleCheck4))
                    {
                        parsing_printf("\tFound sign-extending mov, revising table stride to scale (4)\n");
                        tableStride = 4;
                    }
                    else if((*readExp)->isUsed(scaleCheck2))
                    {
                        parsing_printf("\tFound sign-extending mov, revising table stride to scale (2)\n");
                        tableStride = 2;
                    }
                    else
                    {
                        parsing_printf("\tFound sign-extending mov insn %s, readExp %s\n",
                                       tableLoc->second->format().c_str(), (*readExp)->format().c_str());
                        parsing_printf("\tcouldn't match stride expression %s or %s--HELP!\n",
                                       scaleCheck2->format().c_str(), scaleCheck4->format().c_str());
                    }
                }
                break;
            }
        }
    }
    Address tableBase = getTableAddress(tableInsn, tableInsnAddr, thunkOffset);
    std::map<Address, Instruction::Ptr>::const_iterator findSubtract =
            allInsns.find(tableInsnAddr);
    int offsetMultiplier = 1;
    while(findSubtract->first < current)
    {
        if(findSubtract->second && findSubtract->second->getOperation().getID() == e_sub)
        {
            parsing_printf("\tsuspect table contains negative offsets, revising\n");
            offsetMultiplier = -1;
            break;
        }
        findSubtract++;
    }
    

    return fillTableEntries(thunkOffset, tableBase,
                            tableSize, tableStride, offsetMultiplier, outEdges);
}
namespace detail
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
};


Address IA_IAPI::findThunkInBlock(Block* curBlock, Address& thunkOffset) const
{
    const unsigned char* buf =
            (const unsigned char*)(_isrc->getPtrToInstruction(curBlock->start()));
    if( buf == NULL ) {
        parsing_printf("%s[%d]: failed to get pointer to instruction by offset\n",
                       FILE__, __LINE__);
        return false;
    }
    
    InstructionDecoder dec(buf,curBlock->size() + InstructionDecoder::maxInstructionLength,
                           _isrc->getArch());
    IA_IAPI * blockptr = NULL;
    blockptr = new IA_IAPI(dec,curBlock->start(),_obj,_cr,_isrc);
    IA_IAPI & block = *blockptr;

    parsing_printf("\tchecking block at 0x%lx for thunk\n", curBlock->start());
    while(block.getAddr() < curBlock->end())
    {
        if(block.getInstruction()->getCategory() == c_CallInsn)
        {
            parsing_printf("\tchecking call at 0x%lx for thunk\n", block.getAddr());
            if(!block.isRealCall())
            {
                parsing_printf("\tthunk found at 0x%lx, checking for add\n", block.getAddr());
                block.advance();
                thunkOffset = block.getAddr();
                Instruction::Ptr addInsn = block.getInstruction();
                if(addInsn)
                    parsing_printf("\tinsn after thunk: %s\n", addInsn->format().c_str());
                else
                    parsing_printf("\tNO INSN after thunk at 0x%lx\n", thunkOffset);
                if(addInsn)
                {
                    if(addInsn->getOperation().getID() == e_pop)
                    {
                        block.advance();
                        addInsn = block.getInstruction();
                    }
                    if(addInsn && addInsn->getOperation().getID() == e_add)
                    {
                        Result imm = addInsn->getOperand(1).getValue()->eval();
                        Result imm2 = addInsn->getOperand(0).getValue()->eval();
                        if(imm.defined)
                        {
                            Address thunkDiff = imm.convert<Address>();
                            parsing_printf("\tsetting thunkOffset to 0x%lx (0x%lx + 0x%lx)\n",
                                           thunkOffset+thunkDiff, thunkOffset, thunkDiff);
                            Address ret = block.getPrevAddr();
                            thunkOffset = thunkOffset + thunkDiff;
                            delete blockptr;
                            return ret;
                        }
                        else if(imm2.defined)
                        {
                            Address thunkDiff = imm2.convert<Address>();
                            parsing_printf("\tsetting thunkOffset to 0x%lx (0x%lx + 0x%lx)\n",
                                           thunkOffset+thunkDiff, thunkOffset, thunkDiff);
                            Address ret = block.getPrevAddr();
                            thunkOffset = thunkOffset + thunkDiff;
                            delete blockptr;
                            return ret;
                        }
                        else
                        {
                            parsing_printf("\tadd insn %s found following thunk at 0x%lx, couldn't bind operands!\n",
                                           addInsn->format().c_str(), thunkOffset);
                        }
                    }
                }
                thunkOffset = 0;
            }
        }
        else if(block.getInstruction()->getOperation().getID() == e_lea)
            // Look for an AMD64 IP-relative LEA.  If we find one, it should point to the start of a
        {    // relative jump table.
            parsing_printf("\tchecking instruction %s at 0x%lx for IP-relative LEA\n", block.getInstruction()->format().c_str(),
                           block.getAddr());
            Expression::Ptr IPRelAddr = block.getInstruction()->getOperand(1).getValue();
            IPRelAddr->bind(thePC[_isrc->getArch()].get(), Result(s64, block.getNextAddr()));
            Result iprel = IPRelAddr->eval();
            if(iprel.defined)
            {
                thunkOffset = iprel.convert<Address>();
                parsing_printf("\tsetting thunkOffset to 0x%lx at 0x%lx\n",thunkOffset, block.getAddr());
                Address ret = block.getAddr();
                delete blockptr;
                return ret;
            }
        }
        block.advance();
    }
    delete blockptr;
    return 0;
}


std::pair<Address, Address> IA_IAPI::findThunkAndOffset(Block* start) const
{
    std::set<Block*> visited;
    std::deque<Block*> worklist;
    Block* curBlock;
    worklist.insert(worklist.begin(), start);
    visited.insert(start);
    Address thunkOffset = 0;

    // traverse only intraprocedural edges
    Intraproc epred;

    while(!worklist.empty())
    {
        curBlock = worklist.front();
        worklist.pop_front();
        // If the only thunk we can find within this function that precedes the
        // indirect jump in control flow (we think) is after it in the address space,
        // it may be a jump table but it's not one our heuristics should expect to
        // handle well.  Better to bail out than try to parse something that will be garbage.
        if(curBlock->start() > start->start()) continue;
        Address tableInsnAddr = findThunkInBlock(curBlock, thunkOffset);
        if(tableInsnAddr != 0)
        {
            return std::make_pair(tableInsnAddr, thunkOffset);
        }

        Block::edgelist::iterator sit = curBlock->sources().begin(&epred);
        for( ; sit != curBlock->sources().end(); ++sit) {
            ParseAPI::Edge *e = *sit;

            // FIXME debugging assert
            assert(detail::isNonCallEdge(e));

            // FIXME check this algorithm... O(log n) lookup in visited
            if(!detail::leadsToVisitedBlock(e, visited))
            {
                worklist.push_back(e->src());
                visited.insert(e->src());
            }
        }
    }
    return std::make_pair(0, 0);

}

boost::tuple<Instruction::Ptr,
 Instruction::Ptr,
 bool> IA_IAPI::findMaxSwitchInsn(Block *start) const
{
    std::set<Block *> visited;
    std::vector<Block *> WL;
    Block *curBlk;
    int depth = 0;

    bool foundMaxSwitch = false;
    bool foundCondBranch = false;
    Address maxSwitchAddr = 0;

    WL.push_back(start);
    Instruction::Ptr compareInsn, condBranchInsn;
    bool compareOnTakenBranch = false;
    for(unsigned j=0;j < WL.size(); j++)
    {
        curBlk = WL[j];
        visited.insert(curBlk);

        foundMaxSwitch = false;
        foundCondBranch = false;
        const unsigned char* buf =
                (const unsigned char*)(_isrc->getPtrToInstruction(curBlk->start()));
        if( buf == NULL ) {
            parsing_printf("%s[%d]: failed to get pointer to instruction by offset\n",
                           FILE__, __LINE__);
            return boost::make_tuple(Instruction::Ptr(), Instruction::Ptr(), false);
        }
        InstructionDecoder dec(buf, curBlk->size(), _isrc->getArch());
        Instruction::Ptr i;
        Address curAdr = curBlk->start();
        while(i = dec.decode())
        {
            if(i->getCategory() == c_CompareInsn)
            // check for cmp
            {
                parsing_printf("\tFound jmp table cmp instruction %s at 0x%lx\n",
                               i->format().c_str(), curAdr);
                compareInsn = i;
                maxSwitchAddr = curAdr;
                foundMaxSwitch = true;
            }
            if(i->getCategory() == c_BranchInsn &&
               i->allowsFallThrough())
            {
                parsing_printf("\tFound jmp table cond br instruction %s at 0x%lx\n",
                               i->format().c_str(), curAdr);
                condBranchInsn = i;
                foundCondBranch = true;

                Block::edgelist::iterator tit = curBlk->targets().begin();
                bool taken_hit = false;
                bool fallthrough_hit = false;
                for ( ; tit != curBlk->targets().end(); ++tit) {
                    ParseAPI::Edge *t = *tit;
                    if (t->type() == COND_TAKEN &&
                        (visited.find(t->trg()) != visited.end()))
                    {
                        taken_hit = true;
                    }
                    if ((t->type() == COND_NOT_TAKEN ||
                         t->type() == FALLTHROUGH) &&
                         (visited.find(t->trg()) != visited.end()))
                    {
                        fallthrough_hit = true;
                    }
                }
                parsing_printf("\tfindMaxSwitchInsn: taken_hit: %d, fallthrough_hit: %d\n", taken_hit, fallthrough_hit);
                compareOnTakenBranch = taken_hit && !fallthrough_hit;
                break;
            }
            curAdr += i->size();
        }

        if(foundMaxSwitch && foundCondBranch)
            break; // done

            // look further back
        Block::edgelist::iterator sit = curBlk->sources().begin();
        depth++;
            // We've seen depth 2 in libc et al
        if(depth > 2) return boost::make_tuple(Instruction::Ptr(), Instruction::Ptr(), false);
           
        for( ; sit != curBlk->sources().end(); ++sit) 
        {
            ParseAPI::Edge * s = *sit;

            // ignore return edges
            if(s->type() == RET)
                continue;

            if(s->type() == CALL)
                return boost::make_tuple(Instruction::Ptr(), Instruction::Ptr(), false);

            Block * src = s->src();
            if( (visited.find( src ) == visited.end())) {
                WL.push_back(src);
            }
        }
    }
    WL.clear();
    parsing_printf("\tfindMaxSwitchInsn: table on taken branch: %d, returning: %d\n", compareOnTakenBranch, foundMaxSwitch &&
            foundCondBranch);
    
    return boost::make_tuple(compareInsn, condBranchInsn, compareOnTakenBranch);
}
 
Address IA_IAPI::getTableAddress(Instruction::Ptr tableInsn, 
    Address tableInsnAddr,
    Address thunkOffset) const
{
    // Extract displacement from table insn
    Expression::Ptr displacementSrc;
    if(tableInsn->getCategory() == c_BranchInsn)
    {
        Expression::Ptr op = tableInsn->getOperand(0).getValue();
        std::vector<InstructionAST::Ptr> tmp;
        op->getChildren(tmp);
        if(tmp.empty())
        {
            displacementSrc = op;
        }
        else
        {
            displacementSrc = dyn_detail::boost::dynamic_pointer_cast<Expression>(tmp[0]);
        }
    }
    else
    {
        parsing_printf("\tcracking table instruction %s\n", tableInsn->format().c_str());
        std::vector<InstructionAST::Ptr> tmp;
        Expression::Ptr op = tableInsn->getOperand(1).getValue();
        if(!op)
        {
            parsing_printf("\ttable insn BAD! (no second operand)\n");
            return 0;
        }
        if(tableInsn->getOperation().getID() != e_lea)
        {
            op->getChildren(tmp);
            if(tmp.empty())
            {
                parsing_printf("\ttable insn BAD! (not LEA, second operand not a deref)\n");
                return 0;
            }
            displacementSrc = dyn_detail::boost::dynamic_pointer_cast<Expression>(tmp[0]);
        }
        else
        {
            displacementSrc = op;
        }
    }
    
    zeroAllGPRegisters z(tableInsnAddr + tableInsn->size());
    displacementSrc->apply(&z);
    
    Address jumpTable = 0;
    //if(!disp.defined)
    if(!z.isDefined())
    {
        if(!thunkOffset)
        {
            parsing_printf("\ttable insn: %s, displacement %s, bind of all GPRs FAILED\n",
                           tableInsn->format().c_str(), displacementSrc->format().c_str());
            return 0;
        }
    }
    else
    {
//        jumpTable = disp.convert<Address>();
        jumpTable = z.getResult();
    }
    parsing_printf("\tjumpTable set to 0x%lx\n",jumpTable);

    if(!jumpTable && !thunkOffset)
    {
        return 0;
    }


    // in AMD64 rip-relative LEA case, jumpTable and thunkOffset are
    // calculated from the same instruction (FIXME this indicates
    // a flaw in the overall logic which is corrected here)
    if(jumpTable != thunkOffset) {
        jumpTable += thunkOffset;
        parsing_printf("\tjumpTable revised to 0x%lx\n",jumpTable);
    }
    // On Windows, all of our other addresses have had the base address
    // of their module subtracted off before we ever use them.  We need to fix this up
    // to conform to that standard so that Symtab actually believes that there's code
    // at the table's destination.
#if defined(os_windows)
	jumpTable -= _obj->cs()->loadAddress();
#endif
    if( !_isrc->isValidAddress(jumpTable) )
{
        // If the "jump table" has a start address that is outside
        // of the valid range of the binary, we can say with high
        // probability that we have misinterpreted some other
        // construct (such as a function pointer comparison & tail
        // call, for example) as a jump table. Give up now.
    return 0;
}
    return jumpTable;
}

bool IA_IAPI::fillTableEntries(Address thunkOffset,
                               Address tableBase,
                               unsigned tableSize,
                               unsigned tableStride,
                               int offsetMultiplier,
                               std::vector<std::pair< Address, EdgeTypeEnum> >& outEdges) const
{
    if( !_isrc->isValidAddress(tableBase) )
    {
        parsing_printf("\ttableBase 0x%lx invalid, returning false\n", tableBase);
        return false;
    }
          
    for(unsigned int i=0; i < tableSize; i++)
    {
        Address tableEntry = tableBase + (i * tableStride);
        Address jumpAddress = 0;
                
        if(_isrc->isValidAddress(tableEntry))
        {
            if(tableStride == sizeof(Address)) {
                                        // Unparseable jump table
                jumpAddress = *(const Address *)
                    _isrc->getPtrToInstruction(tableEntry);
                if( 0 == jumpAddress ) {
                    parsing_printf("%s[%d]: failed to get pointer to instruction by offset\n",
                                   FILE__, __LINE__);
                    return false;
                }
            }
            else {
                jumpAddress = *(const int *)
                    _isrc->getPtrToInstruction(tableEntry);
                if( 0 == jumpAddress ) {
                    parsing_printf("%s[%d]: failed to get pointer to "
                        "instruction by offset\n", FILE__, __LINE__);
                    return false;
                }
            }
        }
#if defined(os_windows)
        jumpAddress -= _obj->cs()->loadAddress();
#endif
        if(thunkOffset)
        {
            Address candidate = thunkOffset + jumpAddress * offsetMultiplier;
            // XXX We assume that if thunkOffset is set that
            //     entries in the table are relative, but only
            //     if the absolute address would be valid
            if(_isrc->isCode(candidate))
            {
                jumpAddress = candidate;
            }
            else
            {
                parsing_printf("\tcandidate relative entry %d [0x%lx] -> 0x%lx, invalid, skipping\n",
                               i, tableEntry, candidate);
                
            }
        }
        else if(!(_isrc->isCode(jumpAddress))) {
            parsing_printf("\tentry %d [0x%lx] -> 0x%lx, invalid, skipping\n",
                                   i, tableEntry, jumpAddress);
                    continue;
        }
        parsing_printf("\tentry %d [0x%lx] -> 0x%lx\n",i,tableEntry,jumpAddress);
                outEdges.push_back(std::make_pair(jumpAddress, INDIRECT));
                
    }
    if(outEdges.empty()) parsing_printf("\tno valid table entries, ret false\n");
    return !outEdges.empty();
    
}


bool IA_IAPI::computeTableBounds(Instruction::Ptr maxSwitchInsn,
                                 Instruction::Ptr branchInsn,
                                 Instruction::Ptr tableInsn,
                                 bool foundJCCAlongTaken,
                                 unsigned& tableSize,
                                 unsigned& tableStride) const
{
    assert(maxSwitchInsn && branchInsn);
    Result compareBound = maxSwitchInsn->getOperand(1).getValue()->eval();
    if(!compareBound.defined) return false;
    tableSize = compareBound.convert<unsigned>();
    // Sanity check the bounds; 32k tables would be an oddity, and larger is almost certainly
    // a misparse
    static const unsigned int maxTableSize = 32768;
    if(tableSize > maxTableSize)
    {
        parsing_printf("\tmaxSwitch of %d above %d, BAILING OUT\n", tableSize, maxTableSize);
        return false;
    }
    if(foundJCCAlongTaken)
    {
        if(branchInsn->getOperation().getID() == e_jbe ||
           branchInsn->getOperation().getID() == e_jle)
        {
            tableSize++;
        }
    }
    else
    {
        if(branchInsn->getOperation().getID() == e_jnbe ||
           branchInsn->getOperation().getID() == e_jnle)
        {
            tableSize++;
        }
    }
    parsing_printf("\tmaxSwitch set to %d\n", tableSize);
    tableStride = _isrc->getAddressWidth();
    std::set<Expression::Ptr> tableInsnReadAddr;
    tableInsn->getMemoryReadOperands(tableInsnReadAddr);
    if(tableStride == 8)
    {
        static Immediate::Ptr four(new Immediate(Result(u8, 4)));
        static BinaryFunction::funcT::Ptr multiplier(new BinaryFunction::multResult());
        static Expression::Ptr dummy(new DummyExpr());
        static BinaryFunction::Ptr scaleCheck(new BinaryFunction(four, dummy, u64, multiplier));
        for(std::set<Expression::Ptr>::const_iterator curExpr = tableInsnReadAddr.begin();
            curExpr != tableInsnReadAddr.end();
            ++curExpr)
        {
            if((*curExpr)->isUsed(scaleCheck))
            {
                tableSize = tableSize >> 1;
                parsing_printf("\tmaxSwitch revised to %d\n",tableSize);
            }
        }
    }
    return true;
}

bool IA_IAPI::isThunk() const {
  // Before we go a-wandering, check the target
    if (!_isrc->isValidAddress(getCFT()))
    {
        parsing_printf("... Call to 0x%lx is invalid (outside code or data)\n",
                       getCFT());
        return false;
    }

    const unsigned char *target =
            (const unsigned char *)_isrc->getPtrToInstruction(getCFT());
  // We're decoding two instructions: possible move and possible return.
  // Check for move from the stack pointer followed by a return.
    InstructionDecoder targetChecker(target, 
            2*InstructionDecoder::maxInstructionLength, _isrc->getArch());
    Instruction::Ptr thunkFirst = targetChecker.decode();
    Instruction::Ptr thunkSecond = targetChecker.decode();
    parsing_printf("... checking call target for thunk, insns are %s, %s\n", thunkFirst->format().c_str(),
                   thunkSecond->format().c_str());
    if(thunkFirst && (thunkFirst->getOperation().getID() == e_mov))
    {
        if(thunkFirst->isRead(stackPtr[_isrc->getArch()]))
        {
            parsing_printf("... checking second insn\n");
            if(!thunkSecond) {
                parsing_printf("...no second insn\n");
                return false;
            }
            if(thunkSecond->getCategory() != c_ReturnInsn)
            {
                parsing_printf("...insn %s not a return\n", thunkSecond->format().c_str());
                return false;
            }
            return true;
        }
    }
    parsing_printf("... real call found\n");
    return false;
}

bool IA_IAPI::isTailCall(Function * /*context*/,unsigned int) const
{
    if(tailCall.first) {
        parsing_printf("\tReturning cached tail call check result: %d\n", tailCall.second);
        return tailCall.second;
    }
    tailCall.first = true;

    if(curInsn()->getCategory() == c_BranchInsn &&
       _obj->findFuncByEntry(_cr,getCFT()))
    {
        parsing_printf("\tjump to 0x%lx, TAIL CALL\n", getCFT());
        tailCall.second = true;
        return tailCall.second;
    }

    if(allInsns.size() < 2) {
        tailCall.second = false;
        parsing_printf("\ttoo few insns to detect tail call\n");
        return tailCall.second;
    }

    if(curInsn()->getCategory() == c_BranchInsn ||
       curInsn()->getCategory() == c_CallInsn)
    {
        std::map<Address, Instruction::Ptr>::const_iterator prevIter =
                allInsns.find(current);
        --prevIter;
        Instruction::Ptr prevInsn = prevIter->second;
        if(prevInsn->getOperation().getID() == e_leave)
        {
            parsing_printf("\tprev insn was leave, TAIL CALL\n");
            tailCall.second = true;
            return tailCall.second;
        }
        if(prevInsn->getOperation().getID() == e_pop)
        {
            if(prevInsn->isWritten(framePtr[_isrc->getArch()]))
            {
                parsing_printf("\tprev insn was %s, TAIL CALL\n", prevInsn->format().c_str());
                tailCall.second = true;
                return tailCall.second;
            }
            parsing_printf("\tprev insn was %s, not tail call\n", prevInsn->format().c_str());
        }
    }
    tailCall.second = false;
    return tailCall.second;
}

bool IA_IAPI::savesFP() const
{
    Instruction::Ptr ci = curInsn();
    if(ci->getOperation().getID() == e_push)
    {
        return(ci->isRead(framePtr[_isrc->getArch()]));
    }
    return false;
}

bool IA_IAPI::isStackFramePreamble() const
{
    if(savesFP())
    {
        InstructionDecoder tmp(dec);
        std::vector<Instruction::Ptr> nextTwoInsns;
        nextTwoInsns.push_back(tmp.decode());
        nextTwoInsns.push_back(tmp.decode());
        if(isFrameSetupInsn(nextTwoInsns[0]) ||
           isFrameSetupInsn(nextTwoInsns[1]))
        {
            return true;
        }
    }
    return false;
}

bool IA_IAPI::cleansStack() const
{
    Instruction::Ptr ci = curInsn();
    return (ci->getCategory() == c_ReturnInsn) &&
            ci->getOperand(0).getValue();

}

bool IA_IAPI::isReturnAddrSave() const
{
    // not implemented on non-power
    return false;
}


//class ST_Predicates : public Slicer::Predicates {};

// returns stackTamper, which is false if parsing should not resume 
// after call instructions to this function.  
// The function recommends parsing at an alternative address if the stack 
// delta is a known absolute or relative value, otherwise we will instrument
// this function's return instructions to see if the function returns
StackTamper IA_IAPI::tampersStack(ParseAPI::Function *, 
                                  Address &) const
{
    return TAMPER_NONE;

#if 0

    using namespace DataflowAPI;
    if (TAMPER_UNSET != func->stackTamper()) {
        return func->stackTamper();
    }

    if ( ! _obj->defensiveMode() ) { 
        return TAMPER_NONE;
    }

    Function::blocklist & retblks = func->returnBlocks();
    if ( retblks.begin() == retblks.end() ) {
        return TAMPER_NONE;
    }

    AssignmentConverter converter(true);
    vector<Assignment::Ptr> assgns;
    ST_Predicates preds;
    StackTamper tamper = TAMPER_UNSET;
    //Absloc stkLoc (MachRegister::getStackPointer(_isrc->getArch()));
    Function::blocklist::iterator bit;
    for (bit = retblks.begin(); retblks.end() != bit; bit++) {
        Address retnAddr = (*bit)->lastInsnAddr();
        InstructionDecoder retdec( _isrc->getPtrToInstruction( retnAddr ), 
                                  InstructionDecoder::maxInstructionLength, 
                                  _cr->getArch() );
        Instruction::Ptr retn = retdec.decode();
        converter.convert(retn, retnAddr, func, assgns);
        vector<Assignment::Ptr>::iterator ait;
        AST::Ptr sliceAtRet;

        for (ait = assgns.begin(); assgns.end() != ait; ait++) {
            AbsRegion & outReg = (*ait)->out();
            if ( outReg.absloc().isPC() ) {
                Slicer slicer(*ait,*bit,func);
                Graph::Ptr slGraph = slicer.backwardSlice(preds);
                SymEval::Result_t slRes;
                SymEval::expand(slGraph,slRes);
                if (dyn_debug_malware) {
                    stringstream graphDump;
                    graphDump << "sliceDump_" << func->name() 
                              << "_" << retnAddr << ".dot";
                    slGraph->printDOT(graphDump.str());
                }
                sliceAtRet = slRes[*ait];
                break;
            }
        }
        assert(sliceAtRet != NULL);
        StackTamperVisitor vis((*ait)->out());
        tamper = vis.tampersStack(sliceAtRet, tamperAddr);
        assgns.clear();
    }
    return tamper;
#endif
}

/* returns true if the call leads to:
 * -an invalid instruction (or immediately branches/calls to an invalid insn)
 * -a block not ending in a return instruction that pops the return address 
 *  off of the stack
 */
bool IA_IAPI::isFakeCall() const
{
    assert(_obj->defensiveMode());

    // get instruction at entry of new func
    bool tampers = false;
    Address entry = getCFT();
    if ( ! _cr->contains(entry) || ! _isrc->isCode(entry) ) {
        mal_printf("WARNING: found call to function at %lx that "
                "redirects to invalid address %lx %s[%d]\n", current, 
                entry, FILE__,__LINE__);
        return false;
    }
    const unsigned char* bufPtr =
     (const unsigned char *)(_isrc->getPtrToInstruction(entry));
    InstructionDecoder newdec( bufPtr,
                              _isrc->offset() + _isrc->length() - entry,
                              _cr->getArch() );
    IA_IAPI ah(newdec, entry, _obj, _cr, _isrc);
    Instruction::Ptr insn = ah.curInsn();

    // follow ctrl transfers until you get a block containing non-ctrl 
    // transfer instructions, or hit a return instruction
    while (insn->getCategory() == c_CallInsn ||
           insn->getCategory() == c_BranchInsn) 
    {
        Address entry = ah.getCFT();
        if ( ! _cr->contains(entry) || ! _isrc->isCode(entry) ) {
            mal_printf("WARNING: found call to function at %lx that "
                    "redirects to invalid address %lx %s[%d]\n", current, 
                    entry, FILE__,__LINE__);
            return false;
        }
        bufPtr = (const unsigned char *)(_isrc->getPtrToInstruction(entry));
        newdec = InstructionDecoder(bufPtr, 
                                    _isrc->offset() + _isrc->length() - entry, 
                                    _cr->getArch());
        ah = IA_IAPI(newdec, entry, _obj, _cr, _isrc);
        insn = ah.curInsn();
    }

    // calculate instruction stack deltas for the block, leaving the iterator
    // at the last ins'n if it's a control transfer, or after calculating the 
    // last instruction's delta if we run off the end of initialized memory
    int stackDelta = 0;
    int addrWidth = _isrc->getAddressWidth();
    static Expression::Ptr theStackPtr
        (new RegisterAST(MachRegister::getStackPointer(_isrc->getArch())));
    Address curAddr = entry;

    while(true) {

        // exit condition 1
        if (insn->getCategory() == c_CallInsn ||
            insn->getCategory() == c_ReturnInsn ||
            insn->getCategory() == c_BranchInsn) 
        {
            break;
        }

        // calculate instruction delta
        if(insn->isWritten(theStackPtr)) {
            entryID what = insn->getOperation().getID();
            int sign = 1;
            switch(what) 
            {
            case e_push:
                sign = -1;
            case e_pop: {
                Operand arg = insn->getOperand(0);
                if (arg.getValue()->eval().defined) {
                    stackDelta += sign * addrWidth;
                } else {
                    assert(0);
                }
                break;
            }
            case e_pusha:
            case e_pushad:
                sign = -1;
            case e_popa:
            case e_popad:
                stackDelta += sign * 8 * addrWidth;
                break;

            case e_pushf:
            case e_pushfd:
                sign = -1;
            case e_popf:
            case e_popfd:
                stackDelta += sign * 4;
                break;

            case e_leave:
            case e_enter:
                fprintf(stderr, "WARNING: saw leave or enter instruction "
                        "at %lx that is not handled by isFakeCall %s[%d]\n",
                        curAddr, FILE__,__LINE__);//KEVINTODO: unhandled case
            default:
                assert(0);//what stack-writing instruction is this?
            }
        }

        if (stackDelta > 0) {
            tampers=true;
        }

        // exit condition 2
        ah.advance();
        Instruction::Ptr next = ah.curInsn();
        if (NULL == next) {
            break;
        }
        curAddr += insn->size();
        insn = next;
    } 

    // not a fake call if it ends w/ a return instruction
    if (insn->getCategory() == c_ReturnInsn) {
        return false;
    }

    // if the stack delta is positive or the return address has been replaced
    // with an absolute value, it's a fake call, since in both cases 
    // the return address is gone and we cannot return to the caller
    if ( 0 < stackDelta || tampers ) {
        return true;
    }

    return tampers;
}

bool IA_IAPI::isIATcall() const
{
    if (!isDynamicCall()) {
        return false;
    }

    if (!curInsn()->readsMemory()) {
        return false;
    }

    std::set<Expression::Ptr> memReads;
    curInsn()->getMemoryReadOperands(memReads);
    if (memReads.size() != 1) {
        return false;
    }

    Result memref = (*memReads.begin())->eval();
    if (!memref.defined) {
        return false;
    }
    Address entryAddr = memref.convert<Address>();

    // convert to a relative address
    if (_obj->cs()->loadAddress() < entryAddr) {
        entryAddr -= _obj->cs()->loadAddress();
    }
    
    if (!_obj->cs()->isValidAddress(entryAddr)) {
        return false;
    }

    // calculate the address of the ASCII string pointer, 
    // skip over the IAT entry's two-byte hint
    Address funcAsciiAddr = 2 + *(Address*) (_obj->cs()->getPtrToData(entryAddr));
    if (!_obj->cs()->isValidAddress(funcAsciiAddr)) {
        return false;
    }

    // see if it's really a string that could be a function name
    char *funcAsciiPtr = (char*) _obj->cs()->getPtrToData(funcAsciiAddr);
    char cur = 'a';
    int count=0;
    do {
        cur = funcAsciiPtr[count];
        count++;
    }while (count < 100 && 
            _obj->cs()->isValidAddress(funcAsciiAddr+count) &&
            ((cur >= 'A' && cur <= 'z') ||
             (cur >= '0' && cur <= '9')));
    if (cur != 0 || count <= 1) 
        return false;

    return true;
}
