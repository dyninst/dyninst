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

#include "IA_IAPI.h"
#include "IA_x86Details.h"

#include "Register.h"
#include "Dereference.h"
#include "Immediate.h"
#include "BinaryFunction.h"
#include "debug_parse.h"

#include <deque>
#include <boost/bind.hpp>

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


bool IA_x86Details::isMovAPSTable(std::vector<std::pair< Address, EdgeTypeEnum > >& outEdges)
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
    parsing_printf("\tChecking for movaps table at 0x%lx...\n", currentBlock->current);
    std::set<Address> found;
    const unsigned char *bufferBegin =
            (const unsigned char *)currentBlock->_isrc->getPtrToInstruction(currentBlock->current);
    if( bufferBegin == NULL ) {
        parsing_printf("%s[%d]: failed to get pointer to instruction by offset\n",
                       FILE__, __LINE__);
        return false;
    }

    unsigned int size = (currentBlock->_cr->offset() + currentBlock->_cr->length()) - currentBlock->current;
    InstructionDecoder d(bufferBegin, size, currentBlock->_isrc->getArch());
    Address cur = currentBlock->current;
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

bool IA_x86Details::isTableInsn(Instruction::Ptr i) 
{
  Expression::Ptr jumpExpr = currentBlock->curInsn()->getControlFlowTarget();
  parsing_printf("jumpExpr for table insn is %s\n", jumpExpr->format().c_str());
  if(i->getOperation().getID() == e_mov && i->readsMemory() && i->isWritten(jumpExpr))
  {
    return true;
  }
  if(i->getOperation().getID() == e_lea && i->isWritten(jumpExpr))
  {
    return true;
  }
  return false;
}
        
IA_IAPI::allInsns_t::const_iterator IA_x86Details::findTableInsn() 
{
    // Check whether the jump is our table insn!
    Expression::Ptr cft = currentBlock->curInsn()->getControlFlowTarget();
    if(cft)
    {
        std::vector<Expression::Ptr> tmp;
        cft->getChildren(tmp);
        if(tmp.size() == 1)
        {
            Expression::Ptr cftAddr = tmp[0];
            zeroAllGPRegisters z(currentBlock->current);
            cftAddr->apply(&z);
            parsing_printf("\tChecking indirect jump %s for table insn\n", currentBlock->curInsn()->format().c_str());
            if(z.isDefined() && z.getResult())
            {
                parsing_printf("\tAddress in jump\n");
                return currentBlock->curInsnIter;
            }
        }
    }
    IA_IAPI::allInsns_t::const_iterator c = currentBlock->curInsnIter;
    while(!isTableInsn(c->second) && c != currentBlock->allInsns.begin())
    {
        --c;
    }
    if(isTableInsn(c->second))
    {
        return c;
    }
    return currentBlock->allInsns.end();
}

namespace {
    IA_IAPI::allInsns_t::const_iterator 
    search_insn_vec(Address a, IA_IAPI::allInsns_t const& v)
    {
        if(v.empty())
            return v.end();
        int first = 0;
        int last = v.size()-1;
        while(last >= first) {
            int c = (first+last)/2;
            if(v[c].first == a)
                return v.begin() + c;
            else if(a < v[c].first)
                last = c-1;
            else
                first = c+1;
        }
        return v.end();
    }
}

        
// This should only be called on a known indirect branch...
bool IA_x86Details::parseJumpTable(Block* currBlk,
                             std::vector<std::pair< Address, EdgeTypeEnum> >& outEdges)
{
    if(currentBlock->isIPRelativeBranch())
    {
        return false;
    }

    if(isMovAPSTable(outEdges))
    {
        return true;
    }
    bool foundJCCAlongTaken = false;
    IA_IAPI::allInsns_t::const_iterator tableLoc = findTableInsn();
    if(tableLoc == currentBlock->allInsns.end())
    {
        parsing_printf("\tunable to find table insn\n");
        return false;
    }
    tableInsn.addrOfInsn = tableLoc->first;
    tableInsn.insn = tableLoc->second;
    Instruction::Ptr maxSwitchInsn, branchInsn;
    boost::tie(maxSwitchInsn, branchInsn, foundJCCAlongTaken) = findMaxSwitchInsn(currBlk);
    if(!maxSwitchInsn || !branchInsn)
    {
        parsing_printf("\tunable to fix max switch size\n");
        return false;
    }
    computeTableAddress();
    
    findThunkAndOffset(currBlk);
    if(thunkInsn.addrOfInsn != 0)
    {
        /*
         * FIXME
         * Noticed 2/8/2011
         *
         * Although findThunkAndOffset looks outside of the current block,
         * this code only looks at the instructions within the current
         * block. One of these things is the wrong thing to do.
         * I don't understand what the goal of this code is; clearly thorough
         * code review is required. --nater
         */

        // XXX this is the only place where an actual search 
        //     through allInsns is required; as per the previous 
        //     comment, I think something is wrong here anyway
        IA_IAPI::allInsns_t::const_iterator thunkLoc =
            search_insn_vec(thunkInsn.addrOfInsn, currentBlock->allInsns);

        if(thunkLoc != currentBlock->allInsns.end())
        {
            if(thunkLoc->second && thunkLoc->second->getOperation().getID() == e_lea)
            {
                tableLoc = thunkLoc;
                tableInsn.addrOfInsn = thunkInsn.addrOfInsn;
                tableInsn.insn = thunkLoc->second;
            }
        }
    }
    parsing_printf("\ttableInsn %s at 0x%lx\n",tableInsn.insn->format().c_str(), tableInsn.addrOfInsn);
    if(thunkInsn.addrFromInsn) {
        parsing_printf("\tThunk-calculated table base address: 0x%lx\n",
                       thunkInsn.addrFromInsn);
    }
    unsigned tableSize = 0, tableStride = 0;
    bool ok = computeTableBounds(maxSwitchInsn, branchInsn, tableInsn.insn, foundJCCAlongTaken,
                                 tableSize, tableStride);
    if(!ok)
    {
        return false;
    }
    IA_IAPI::allInsns_t::const_iterator cur = currentBlock->curInsnIter;
    
    while(tableLoc != cur)
    {
        tableLoc++;
        if(tableLoc->second->getOperation().getID() == e_lea)
        {
            parsing_printf("\tchecking instruction %s at 0x%lx for IP-relative LEA\n", tableLoc->second->format().c_str(),
                           tableLoc->first);
            Expression::Ptr IPRelAddr = tableLoc->second->getOperand(1).getValue();
            IPRelAddr->bind(currentBlock->thePC[currentBlock->_isrc->getArch()].get(),
                            Result(s64, tableLoc->first + tableLoc->second->size()));
            Result iprel = IPRelAddr->eval();
            if(iprel.defined)
            {
                parsing_printf("\trevising tableInsn to %s at 0x%lx\n",tableLoc->second->format().c_str(), tableLoc->first);
                tableInsn.insn = tableLoc->second;
                tableInsn.addrOfInsn = tableLoc->first;
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
    // This first compute() should be unnecessary, as we'll already have done the work above.
    // However, if there turn out to be bugs later (PIC tables where we're revising the table insn
    // and it matters), then recompute here before revision...
    //    computeTableAddress();
    reviseTableAddress();

    IA_IAPI::allInsns_t::const_iterator findSubtract =
        search_insn_vec(tableInsn.addrOfInsn,currentBlock->allInsns);

    int offsetMultiplier = 1;
    while(findSubtract->first < currentBlock->current)
    {
        if(findSubtract->second && findSubtract->second->getOperation().getID() == e_sub)
        {
            parsing_printf("\tsuspect table contains negative offsets, revising\n");
            offsetMultiplier = -1;
            break;
        }
        findSubtract++;
    }
    

    return fillTableEntries(thunkInsn.addrFromInsn, tableInsn.addrFromInsn,
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

bool IA_x86Details::handleCall(IA_IAPI& block)
{
  parsing_printf("\tchecking call at 0x%lx for thunk\n", block.getAddr());
  if(!block.isRealCall())
  {
    parsing_printf("\tthunk found at 0x%lx, checking for add\n", block.getAddr());
    block.advance();
    thunkInsn.addrFromInsn = block.getAddr();
    Instruction::Ptr addInsn = block.getInstruction();
    if(addInsn)
      parsing_printf("\tinsn after thunk: %s\n", addInsn->format().c_str());
    else
      parsing_printf("\tNO INSN after thunk at 0x%lx\n", thunkInsn.addrFromInsn);
    if(addInsn)
    {
      std::set<RegisterAST::Ptr> boundRegs;
      
      if(addInsn->getOperation().getID() == e_pop)
      {
	addInsn->getWriteSet(boundRegs);
	block.advance();
	addInsn = block.getInstruction();
      }
      if(addInsn && ((addInsn->getOperation().getID() == e_add) ||
		     (addInsn->getOperation().getID() == e_lea)))
      {
	Expression::Ptr op0 = addInsn->getOperand(0).getValue();
	Expression::Ptr op1 = addInsn->getOperand(1).getValue();
	for(std::set<RegisterAST::Ptr>::const_iterator curReg = boundRegs.begin();
	    curReg != boundRegs.end();
	    ++curReg)
	{
	  op0->bind(curReg->get(), Result(u64, 0));
	  op1->bind(curReg->get(), Result(u64, 0));
	  
	}
	
	
	Result imm = addInsn->getOperand(1).getValue()->eval();
	Result imm2 = addInsn->getOperand(0).getValue()->eval();
	if(imm.defined)
	{
	  Address thunkDiff = imm.convert<Address>();
	  parsing_printf("\tsetting thunkInsn.addrFromInsn to 0x%lx (0x%lx + 0x%lx)\n",
			 thunkInsn.addrFromInsn+thunkDiff, thunkInsn.addrFromInsn, thunkDiff);
	  thunkInsn.addrOfInsn = block.getPrevAddr();
	  thunkInsn.addrFromInsn = thunkInsn.addrFromInsn + thunkDiff;
	  return true;
	  
	}
	else if(imm2.defined)
	{
	  Address thunkDiff = imm2.convert<Address>();
	  parsing_printf("\tsetting thunkInsn.addrFromInsn to 0x%lx (0x%lx + 0x%lx)\n",
			 thunkInsn.addrFromInsn+thunkDiff, thunkInsn.addrFromInsn, thunkDiff);
	  thunkInsn.addrOfInsn = block.getPrevAddr();
	  thunkInsn.addrFromInsn = thunkInsn.addrFromInsn + thunkDiff;
	  return true;
	}
	else
	{
	  parsing_printf("\tadd insn %s found following thunk at 0x%lx, couldn't bind operands!\n",
			 addInsn->format().c_str(), thunkInsn.addrFromInsn);
	}
      }
    }
    thunkInsn.addrFromInsn = 0;
  }
  thunkInsn.addrFromInsn = 0;
  thunkInsn.addrOfInsn = 0;
  thunkInsn.insn.reset();
  
  return false;
}

bool IA_x86Details::handleAdd(IA_IAPI& block)
{
  parsing_printf("\t found add insn %s without obvious thunk\n",
		 block.getInstruction()->format().c_str());
  // Check table insn and bail if it's not of the mov eax, [eax] form
  // We do this indirectly: if there was already a displacment that we
  // could find in the table insn, we have a "table address"; otherwise, check adds
  if(tableInsn.addrFromInsn != 0) 
  {
    return false;
  }
  
  // Use the add operand as our table base; we're handling tables of the form:
  // <reg = index>
  // add reg, $base
  // mov reg, [reg]
  // jmp *reg
  Expression::Ptr addExpr = block.getInstruction()->getOperand(1).getValue();
  zeroAllGPRegisters z(block.getAddr());
  addExpr->apply(&z);
  thunkInsn.insn = block.getInstruction();
  thunkInsn.addrFromInsn = z.getResult();
  thunkInsn.addrOfInsn = block.getAddr();
  
  parsing_printf("\t setting thunk offset to 0x%lx (EXPERIMENTAL!)\n", thunkInsn.addrFromInsn);
  
  return thunkInsn.addrFromInsn != 0;
}


void IA_x86Details::findThunkInBlock(Block* curBlock) 
{
    const unsigned char* buf =
            (const unsigned char*)(currentBlock->_isrc->getPtrToInstruction(curBlock->start()));
    if( buf == NULL ) {
        parsing_printf("%s[%d]: failed to get pointer to instruction by offset\n",
                       FILE__, __LINE__);
        return;
    }
    
    InstructionDecoder dec(buf,curBlock->size() + InstructionDecoder::maxInstructionLength,
                           currentBlock->_isrc->getArch());
    IA_IAPI block(dec,curBlock->start(),currentBlock->_obj,currentBlock->_cr,
			   currentBlock->_isrc, curBlock);

    parsing_printf("\tchecking block at 0x%lx for thunk\n", curBlock->start());
    while(block.getAddr() < curBlock->end())
    {
        if(block.getInstruction()->getCategory() == c_CallInsn)
        {
	  if(handleCall(block)) return;
        }
        else if(block.getInstruction()->getOperation().getID() == e_lea)
            // Look for an AMD64 IP-relative LEA.  If we find one, it should point to the start of a
        {    // relative jump table.
            parsing_printf("\tchecking instruction %s at 0x%lx for IP-relative LEA\n", block.getInstruction()->format().c_str(),
                           block.getAddr());
            Expression::Ptr IPRelAddr = block.getInstruction()->getOperand(1).getValue();
            IPRelAddr->bind(currentBlock->thePC[currentBlock->_isrc->getArch()].get(), Result(s64, block.getNextAddr()));
            Result iprel = IPRelAddr->eval();
            if(iprel.defined)
            {
                thunkInsn.addrFromInsn = iprel.convert<Address>();
                parsing_printf("\tsetting thunkOffset to 0x%lx at 0x%lx\n",thunkInsn.addrFromInsn, block.getAddr());
                thunkInsn.addrOfInsn = block.getAddr();
		thunkInsn.insn = block.getInstruction();
                return;
            }
        }
	else if(block.getInstruction()->getOperation().getID() == e_add)
	{
	  if(handleAdd(block)) {
	    parsing_printf("handleAdd found thunk candidate, addr is 0x%lx\n", block.getAddr());
	    return;
	  }
	  
	}
        block.advance();
    }
    return;
}

void processPredecessor(Edge* e, std::set<Block*>& visited, std::deque<Block*>& worklist)
{
  parsing_printf("\t\tblock %x, edge type %s\n",
		 e->src()->start(),
		 format(e->type()).c_str());

  // FIXME debugging assert
  assert(detail::isNonCallEdge(e));

  // FIXME check this algorithm... O(log n) lookup in visited
  if(!detail::leadsToVisitedBlock(e, visited))
  {
    worklist.push_back(e->src());
    visited.insert(e->src());
  }  
}


void IA_x86Details::findThunkAndOffset(Block* start) 
{
    std::set<Block*> visited;
    std::deque<Block*> worklist;
    Block* curBlock;
    worklist.insert(worklist.begin(), start);
    visited.insert(start);

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
        findThunkInBlock(curBlock);
        if(thunkInsn.addrOfInsn != 0)
        {
	  parsing_printf("\tSUCCESS, tableInsnAddr = 0x%lx, thunkOffset = 0x%lx\n",
			 thunkInsn.addrOfInsn, thunkInsn.addrFromInsn);
	  return;
        }

        //Block::edgelist::const_iterator sit = curBlock->sources().begin(&epred);
	parsing_printf("\tpredecessors:\n");

	std::for_each(boost::make_filter_iterator(epred, curBlock->sources().begin(), curBlock->sources().end()),
		      boost::make_filter_iterator(epred, curBlock->sources().end(), curBlock->sources().end()),
		      boost::bind(processPredecessor, _1, boost::ref(visited), boost::ref(worklist)));
	
	

    }
    return;

}

boost::tuple<Instruction::Ptr,
 Instruction::Ptr,
 bool> IA_x86Details::findMaxSwitchInsn(Block *start) 
{
    std::set<Block *> visited;
    std::vector<Block *> WL;
    Block *curBlk;
    int depth = 0;

    bool foundMaxSwitch = false;
    bool foundCondBranch = false;

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
                (const unsigned char*)(currentBlock->_isrc->getPtrToInstruction(curBlk->start()));
        if( buf == NULL ) {
            parsing_printf("%s[%d]: failed to get pointer to instruction by offset\n",
                           FILE__, __LINE__);
            return boost::make_tuple(Instruction::Ptr(), Instruction::Ptr(), false);
        }
        InstructionDecoder dec(buf, curBlk->size(), currentBlock->_isrc->getArch());
        Instruction::Ptr i;
        Address curAdr = curBlk->start();
        while((i = dec.decode()))
        {
            if(i->getCategory() == c_CompareInsn)
            // check for cmp
            {
                parsing_printf("\tFound jmp table cmp instruction %s at 0x%lx\n",
                               i->format().c_str(), curAdr);
                compareInsn = i;
                foundMaxSwitch = true;
            }
            if(i->getCategory() == c_BranchInsn &&
               i->allowsFallThrough())
            {
                parsing_printf("\tFound jmp table cond br instruction %s at 0x%lx\n",
                               i->format().c_str(), curAdr);
                condBranchInsn = i;
                foundCondBranch = true;

                Block::edgelist::const_iterator tit = curBlk->targets().begin();
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
        Block::edgelist::const_iterator sit = curBlk->sources().begin();
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

void IA_x86Details::computeTableAddress()
{
    // Extract displacement from table insn
    Expression::Ptr displacementSrc;
    if(tableInsn.insn->getCategory() == c_BranchInsn)
    {
        Expression::Ptr op = tableInsn.insn->getOperand(0).getValue();
        std::vector<Expression::Ptr> tmp;
        op->getChildren(tmp);
        if(tmp.empty())
        {
            displacementSrc = op;
        }
        else
        {
            displacementSrc = tmp[0];
        }
    }
    else
    {
        parsing_printf("\tcracking table instruction %s\n", tableInsn.insn->format().c_str());
        std::vector<Expression::Ptr> tmp;
        Expression::Ptr op = tableInsn.insn->getOperand(1).getValue();
        if(!op)
        {
            parsing_printf("\ttable insn BAD! (no second operand)\n");
            return;
        }
        if(tableInsn.insn->getOperation().getID() != e_lea)
        {
            op->getChildren(tmp);
            if(tmp.empty())
            {
                parsing_printf("\ttable insn BAD! (not LEA, second operand not a deref)\n");
                return;
            }
            displacementSrc = tmp[0];
        }
        else
        {
            displacementSrc = op;
        }
    }
    
    zeroAllGPRegisters z(tableInsn.addrOfInsn + tableInsn.insn->size());
    displacementSrc->apply(&z);
    
    if(!z.isDefined())
    {
      parsing_printf("\ttable insn: %s, displacement %s, bind of all GPRs FAILED\n",
		     tableInsn.insn->format().c_str(), displacementSrc->format().c_str());
      return;
    }
    else
    {
        tableInsn.addrFromInsn = z.getResult();
    }
    parsing_printf("\ttableInsn.addrFromInsn set to 0x%lx\n",tableInsn.addrFromInsn);
}

void IA_x86Details::reviseTableAddress()
{
    if(!tableInsn.addrFromInsn && !thunkInsn.addrFromInsn)
    {
        return;
    }


    // in AMD64 rip-relative LEA case, tableInsn.addrFromInsn and thunkOffset are
    // calculated from the same instruction (FIXME this indicates
    // a flaw in the overall logic which is corrected here)
    if(tableInsn.addrFromInsn != thunkInsn.addrFromInsn) {
        tableInsn.addrFromInsn += thunkInsn.addrFromInsn;
        parsing_printf("\ttableInsn.addrFromInsn revised to 0x%lx\n",tableInsn.addrFromInsn);
    }
    // On Windows, all of our other addresses have had the base address
    // of their module subtracted off before we ever use them.  We need to fix this up
    // to conform to that standard so that Symtab actually believes that there's code
    // at the table's destination.
#if defined(os_windows)
    tableInsn.addrFromInsn -= currentBlock->_obj->cs()->loadAddress();
    parsing_printf("\ttableInsn.addrFromInsn revised to 0x%lx\n",tableInsn.addrFromInsn);
#endif
    if( !currentBlock->_isrc->isValidAddress(tableInsn.addrFromInsn) )
    {
        // If the "jump table" has a start address that is outside
        // of the valid range of the binary, we can say with high
        // probability that we have misinterpreted some other
        // construct (such as a function pointer comparison & tail
        // call, for example) as a jump table. Give up now.
      tableInsn.addrFromInsn = 0;
      parsing_printf("\ttableInsn.addrFromInsn not valid, revised to 0x%lx\n",tableInsn.addrFromInsn);
    }
    return;
}

bool IA_x86Details::fillTableEntries(Address thunkOffset,
                               Address tableBase,
                               unsigned tableSize,
                               unsigned tableStride,
                               int offsetMultiplier,
                               std::vector<std::pair< Address, EdgeTypeEnum> >& outEdges) 
{
    if( !currentBlock->_isrc->isValidAddress(tableBase) )
    {
        parsing_printf("\ttableBase 0x%lx invalid, returning false\n", tableBase);
        return false;
    }
          
    for(unsigned int i=0; i < tableSize; i++)
    {
        Address tableEntry = tableBase + (i * tableStride);
        Address jumpAddress = 0;
                
        if(currentBlock->_isrc->isValidAddress(tableEntry))
        {
            if(tableStride == sizeof(Address)) {
                                        // Unparseable jump table
                jumpAddress = *(const Address *)
                        currentBlock->_isrc->getPtrToInstruction(tableEntry);
                if( 0 == jumpAddress ) {
                    parsing_printf("%s[%d]: failed to get pointer to instruction by offset\n",
                                   FILE__, __LINE__);
                    return false;
                }
            }
            else {
                jumpAddress = *(const int *)
                        currentBlock->_isrc->getPtrToInstruction(tableEntry);
                if( 0 == jumpAddress ) {
                    parsing_printf("%s[%d]: failed to get pointer to "
                            "instruction by offset\n", FILE__, __LINE__);
                    return false;
                }
            }
        }
#if defined(os_windows)
        jumpAddress -= currentBlock->_obj->cs()->loadAddress();
#endif
        if(thunkOffset)
{
    Address candidate = thunkOffset + jumpAddress * offsetMultiplier;
            // XXX We assume that if thunkOffset is set that
            //     entries in the table are relative, but only
            //     if the absolute address would be valid
    if(currentBlock->_isrc->isCode(candidate))
    {
        jumpAddress = candidate;
    }
    else
    {
        parsing_printf("\tcandidate relative entry %d [0x%lx] -> 0x%lx, invalid, skipping\n",
                       i, tableEntry, candidate);
                
    }
}
        else if(!(currentBlock->_isrc->isCode(jumpAddress))) {
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


bool IA_x86Details::computeTableBounds(Instruction::Ptr maxSwitchInsn,
                                 Instruction::Ptr branchInsn,
                                 Instruction::Ptr tableInsn,
                                 bool foundJCCAlongTaken,
                                 unsigned& tableSize,
                                 unsigned& tableStride) 
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
    tableStride = currentBlock->_isrc->getAddressWidth();
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

