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


#include "IA_x86.h"
#include "Dereference.h"
#include "Immediate.h"
#include "BinaryFunction.h"
#include "debug_parse.h"
#include "dataflowAPI/h/slicing.h"
#include "dataflowAPI/h/SymEval.h"
#include "ABI.h"
#include "bitArray.h"
//#include "StackTamperVisitor.h"
#include "instructionAPI/h/Visitor.h"

#include "instructionAPI/h/syscalls.h"

#include <deque>
#include "Register.h"

#include <boost/variant2/variant.hpp>
#include <stack>

using namespace Dyninst;
using namespace InstructionAPI;
using namespace Dyninst::InsnAdapter;
using namespace Dyninst::ParseAPI;

IA_x86::IA_x86(Dyninst::InstructionAPI::InstructionDecoder dec_,
               Address start_,
	       Dyninst::ParseAPI::CodeObject* o,
	       Dyninst::ParseAPI::CodeRegion* r,
	       Dyninst::InstructionSource *isrc,
	       Dyninst::ParseAPI::Block * curBlk_):
	           IA_IAPI(dec_, start_, o, r, isrc, curBlk_) {
}

IA_x86::IA_x86(const IA_x86& rhs): IA_IAPI(rhs) {}

IA_x86* IA_x86::clone() const {
    return new IA_x86(*this);
}

bool IA_x86::isFrameSetupInsn(Instruction i) const
{
    if(i.getOperation().getID() == e_mov)
    {
        if(i.readsMemory() || i.writesMemory())
        {
            parsing_printf("%s[%d]: discarding insn %s as stack frame preamble, not a reg-reg move\n",
                           FILE__, __LINE__, i.format().c_str());
            //return false;
        }
        if(i.isRead(stackPtr[_isrc->getArch()]) &&
           i.isWritten(framePtr[_isrc->getArch()]))
        {
            if((unsigned) i.getOperand(0).getValue()->size() == _isrc->getAddressWidth())
            {
                return true;
            }
            else
            {
                parsing_printf("%s[%d]: discarding insn %s as stack frame preamble, size mismatch for %u-byte addr width\n",
                               FILE__, __LINE__, i.format().c_str(), _isrc->getAddressWidth());
            }
        }
    }
    return false;
}


/*
 * Simplify an effective address calulation for 'lea' instruction
 *
 *  A common idiom for a multi-byte nop is to perform an effective address
 *  calculation that results in storing the value of a register into itself.
 *
 * Examples:
 *
 *    lea rax, [rax]
 *    lea rax, [rax*1 + 0]
 *
 * This visitor uses a stack and an RPN-style calculation to simplify instances
 * of multiplicitive (1) and additive (0) identities.  If a binary expression
 * with an identity operand is encountered the result is the other value of the
 * expression. All other expressions result in the original expression.  The
 * final result simplifies to either a register expression or some other
 * expression.
 *
 * After applying the visitor to both operands, A NOP is then determined by
 * testing if each visitor's result is a register and are identical.
 *
 * There are special cases that are handled implicitly.
 *
 * 1) The pseudoregister `riz` in `lea rsi, [rsi+riz*1+0x0]` is an assembly
 * construct to indicate a SIB, is not present in the expression not considered
 * to be read. This reduces the expression to `rsi+0x0`.
 *
 * 2) If the source and destination registers are different sizes, then the
 * instruction is not considered a nop. For example, `lea eax, [rax]`.
 *
 */
class leaSimplifyVisitor : public InstructionAPI::Visitor
{
        using ExprNodeValues = boost::variant2::variant<Expression*, RegisterAST*, int64_t>;
        using ExprStack = std::stack<ExprNodeValues>;
        ExprStack exprStack;

    public:
        void visit(BinaryFunction *bf) override {
            auto op1 = exprStack.top();
            exprStack.pop();
            auto op2 = exprStack.top();
            exprStack.pop();

            auto value = boost::variant2::get_if<int64_t>(&op1);
            if (!value)  {
                std::swap(op1, op2);  // op1 not an immediate, commute and try again
                value = boost::variant2::get_if<int64_t>(&op1);
            }
            if (value && ((*value == 0 && bf->isAdd()) || (*value == 1 && bf->isMultiply())))  {
                exprStack.push(op2);  // additive or multiplicative identity, simplify to op2
            }  else  {
                exprStack.push(bf);   // no simplification, use BinaryFunction expression
            }
        }
        void visit(Immediate *imm) override {
            auto const value = imm->eval().convert<int64_t>();
            exprStack.push(value);
        }
        void visit(RegisterAST *reg) override {
            exprStack.push(reg);
        }
        void visit(Dereference *deref) override {
            parsing_printf("%s[%d]: malformed lea instruction, dereference expression encountered\n",
                           FILE__, __LINE__);
            exprStack.pop();         // pop the Dereference's child expression
            exprStack.push(deref);   // replace with the Dereference exprression
        }

        // return simplified result RegisterAST* or nullptr if not a registerAST*
        const RegisterAST *getRegister() const {
            auto value = boost::variant2::get_if<RegisterAST *>(&exprStack.top());
            return value ? *value : nullptr;
        }
        void reset()  {      // reset the visitor for reuse
            exprStack = {};  // clear the stack
        }
};


bool isNopInsn(Instruction insn) {
    if (insn.getOperation().getID() == e_nop)  {
        return true;
    }  else if (insn.getOperation().getID() == e_lea)  {
        std::vector<Operand> operands;
        insn.getOperands(operands);

        if (operands.size() != 2)  {
            parsing_printf("%s[%d]: malformed lea instruction number of operands (%lu) not 2",
                           FILE__, __LINE__, operands.size());
            return false;
        }

        leaSimplifyVisitor op1Visitor, op2Visitor;
        operands[0].getValue()->apply(&op1Visitor);
        operands[1].getValue()->apply(&op2Visitor);

        auto op1Reg = op1Visitor.getRegister();
        auto op2Reg = op2Visitor.getRegister();

        // both operands simplify to registers and they are the same
        return op1Reg && op2Reg && *op1Reg == *op2Reg;
    }

    return false;
}


bool IA_x86::isNop() const
{
    Instruction ci = curInsn();

    return isNopInsn(ci);
}

/*
 * A `thunk' is a function composed of the following pair of instructions:
 
 thunk:
    mov (%esp), <some register>
    ret
 
 * It has the effect of putting the address following a call to `thunk' into
 * the register, and is used in position independent code.
 */
namespace {
    class ThunkVisitor : public InstructionAPI::Visitor {
     public:
        ThunkVisitor() : offset_(0) { }
        virtual void visit(BinaryFunction *) {
            return;
        }
        virtual void visit(Immediate *i) {
            offset_ = i->eval().convert<Address>();
        }
        virtual void visit(RegisterAST*) {
            return;
        }
        virtual void visit(Dereference*) {
            return;
        }
        Address offset() const { return offset_; }

     private:
        Address offset_;
    };
}
bool IA_x86::isThunk() const {
  // Before we go a-wandering, check the target
   bool valid; Address addr;
   boost::tie(valid, addr) = getCFT();
   if (!valid ||
       !_isrc->isValidAddress(addr)) {
        parsing_printf("... Call to 0x%lx is invalid (outside code or data)\n",
                       addr);
        return false;
    }

    const unsigned char *target =
       (const unsigned char *)_isrc->getPtrToInstruction(addr);
    InstructionDecoder targetChecker(target,
            2*InstructionDecoder::maxInstructionLength, _isrc->getArch());
    Instruction thunkFirst = targetChecker.decode();
    Instruction thunkSecond = targetChecker.decode();
    if((thunkFirst.getOperation().getID() == e_mov) &&
        (thunkSecond.getCategory() == c_ReturnInsn))
    {
        if(thunkFirst.isRead(stackPtr[_isrc->getArch()]))
        {
            // it is not enough that the stack pointer is read; it must
            // be a zero-offset read from the stack pointer
            ThunkVisitor tv;
            Operand op = thunkFirst.getOperand(1);
            op.getValue()->apply(&tv); 
    
            return tv.offset() == 0; 
        }
    }
    return false;
}

bool IA_x86::isTailCall(const Function *context, EdgeTypeEnum type, unsigned int,
                        const set<Address> &knownTargets) const
{
   // Collapse down to "branch" or "fallthrough"
    switch(type) {
       case COND_TAKEN:
       case DIRECT:
       case INDIRECT:
          type = DIRECT;
          break;
       case CALL:
       case RET:
       case COND_NOT_TAKEN:
       case FALLTHROUGH:
       case CALL_FT:
       default:
          return false;
    }

    parsing_printf("Checking for Tail Call for x86\n");
    context->obj()->cs()->incrementCounter(PARSE_TAILCALL_COUNT); 
    if (tailCalls.find(type) != tailCalls.end()) {
        parsing_printf("\tReturning cached tail call check result: %d\n", tailCalls[type]);
        if (tailCalls[type]) {
            context->obj()->cs()->incrementCounter(PARSE_TAILCALL_FAIL);
            return true;
        }
        return false;
    }
    
    bool valid; Address addr;
    boost::tie(valid, addr) = getCFT();

    Function* callee = _obj->findFuncByEntry(_cr, addr);
    Block* target = _obj->findBlockByEntry(_cr, addr);
    if(curInsn().getCategory() == c_BranchInsn &&
       valid &&
       callee && 
       callee != context &&
       // We can only trust entry points from hints
       callee->src() == HINT &&
       /* the target can either be not parsed or not within the current context */
       ((target == NULL) || (target && !context->contains(target)))
       )
    {
      parsing_printf("\tjump to 0x%lx, TAIL CALL\n", addr);
      tailCalls[type] = true;
      return true;
    }

    if (valid && addr > 0 && !context->region()->contains(addr)) {
      parsing_printf("\tjump to 0x%lx in other regions, TAIL CALL\n", addr);
      tailCalls[type] = true;
      return true;
    }    

    if (curInsn().getCategory() == c_BranchInsn &&
            valid &&
            !callee) {
    if (knownTargets.find(addr) != knownTargets.end()) {
	    parsing_printf("\tjump to 0x%lx is known target in this function, NOT TAIL CALL\n", addr);
	    tailCalls[type] = false;
	    return false;
	}
    }

    if(allInsns.size() < 2) {
      if(context->addr() == _curBlk->start() && curInsn().getCategory() == c_BranchInsn)
      {
	parsing_printf("\tjump as only insn in entry block, TAIL CALL\n");
	tailCalls[type] = true;
	return true;
      }
      else
      {
        parsing_printf("\ttoo few insns to detect tail call\n");
        context->obj()->cs()->incrementCounter(PARSE_TAILCALL_FAIL);
        tailCalls[type] = false;
        return false;
      }
    }

    if ((curInsn().getCategory() == c_BranchInsn))
    {
        //std::map<Address, Instruction::Ptr>::const_iterator prevIter =
                //allInsns.find(current);
        
        // Updated: there may be zero or more nops between leave->jmp
       
        allInsns_t::const_iterator prevIter = curInsnIter;
        --prevIter;
        Instruction prevInsn = prevIter->second;
    
        while ( isNopInsn(prevInsn) && (prevIter != allInsns.begin()) ) {
           --prevIter;
           prevInsn = prevIter->second;
        }
	prevInsn = prevIter->second;
        if(prevInsn.getOperation().getID() == e_leave)
        {
           parsing_printf("\tprev insn was leave, TAIL CALL\n");
           tailCalls[type] = true;
           return true;
        }
        else if(prevInsn.getOperation().getID() == e_pop)
        {
            std::set<RegisterAST::Ptr> regsWritten;
            prevInsn.getWriteSet(regsWritten);
            if (regsWritten.size() == 2) {
                MachRegister popReg;
                for (auto rit = regsWritten.begin(); rit != regsWritten.end(); ++rit) {
                    if (MachRegister::getStackPointer(_isrc->getArch()) != (*rit)->getID()) {
                        popReg = (*rit)->getID();
                    }
                }
                int addrWidth = (_isrc->getArch() == Arch_x86_64) ? 8 : 4;
                ABI* abi = ABI::getABI(addrWidth);
                const bitArray& callWritten = abi->getCallWrittenRegisters();
                int index = abi->getIndex(popReg);
                if (index != -1 && callWritten[index] == false) {
                    parsing_printf("\tprev insn was %s, TAIL CALL\n", prevInsn.format().c_str());
                    tailCalls[type] = true;
                    return true;
                }
            }
        }
        else if(prevInsn.getOperation().getID() == e_add)
        {			
            if(prevInsn.isWritten(stackPtr[_isrc->getArch()]))
            {
				bool call_fallthrough = false;
                boost::lock_guard<Block> g(*_curBlk);

				if (_curBlk->start() == prevIter->first) {
					for (auto eit = _curBlk->sources().begin(); eit != _curBlk->sources().end(); ++eit) {						
						if ((*eit)->type() == CALL_FT) {
							call_fallthrough = true;
							break;
						}
					}
				}
				if (call_fallthrough) {
					parsing_printf("\tprev insn was %s, but it is the next instruction of a function call, not a tail call\n",
                                   prevInsn.format().c_str());
				}	else {
					parsing_printf("\tprev insn was %s, TAIL CALL\n", prevInsn.format().c_str());
					tailCalls[type] = true;
					return true;
				}
			} else
				parsing_printf("\tprev insn was %s, not tail call\n", prevInsn.format().c_str());
        }
    }

    tailCalls[type] = false;
    context->obj()->cs()->incrementCounter(PARSE_TAILCALL_FAIL);
    return false;
}

bool IA_x86::savesFP() const
{
	std::vector<Instruction> insns;
	insns.push_back(curInsn());
#if defined(os_windows)
	// Windows functions can start with a noop...
	InstructionDecoder tmp(dec);
	insns.push_back(tmp.decode());
#endif
	for (unsigned i = 0; i < insns.size(); ++i) {
		InstructionAPI::Instruction ci = insns[i];
	    if(ci.getOperation().getID() == e_push)
		{
			if (ci.isRead(framePtr[_isrc->getArch()])) {
				return true;
			}
			else return false;
		}
	}	
	return false;
}

bool IA_x86::isStackFramePreamble() const
{
#if defined(os_windows)
	// Windows pads with a noop
	const int limit = 3;
#else 
	const int limit = 2;
#endif
	if (!savesFP()) return false;
    InstructionDecoder tmp(dec);
    std::vector<Instruction::Ptr> nextTwoInsns;
    for (int i = 0; i < limit; ++i) {
       Instruction insn = tmp.decode();
       if (isFrameSetupInsn(insn)) {
          return true;
       }
    }
	return false;
}

bool IA_x86::cleansStack() const
{
    Instruction ci = curInsn();
	if (ci.getCategory() != c_ReturnInsn) return false;
    std::vector<Operand> ops;
	ci.getOperands(ops);
	return (ops.size() > 1);
}

bool IA_x86::isReturn(Dyninst::ParseAPI::Function * /*context*/,
			Dyninst::ParseAPI::Block* /*currBlk*/) const
{
    // For x86, we check if an instruction is return based on the category. 
    // However, for powerpc, the return instruction BLR can be a return or
    // an indirect jump used for jump tables etc. Hence, we need to function and block
    // to determine if an instruction is a return. But these parameters are unused for x86. 
    return curInsn().getCategory() == c_ReturnInsn;
}

bool IA_x86::isReturnAddrSave(Dyninst::Address&) const
{
    // not implemented on non-power
    return false;
}

bool IA_x86::sliceReturn(ParseAPI::Block* /*bit*/, Address /*ret_addr*/, ParseAPI::Function * /*func*/) const {
   return true;
}

//class ST_Predicates : public Slicer::Predicates {};


/* returns true if the call leads to:
 * -an invalid instruction (or immediately branches/calls to an invalid insn)
 * -a block not ending in a return instruction that pops the return address 
 *  off of the stack
 */
bool IA_x86::isFakeCall() const
{
    assert(_obj->defensiveMode());

    if (isDynamicCall()) {
        return false;
    }

    // get func entry
    bool tampers = false;
    bool valid; Address entry;
    boost::tie(valid, entry) = getCFT();

    if (!valid) return false;

    if (! _cr->contains(entry) ) {
       return false;
    }

    if ( ! _isrc->isCode(entry) ) {
        mal_printf("WARNING: found function call at %lx "
                   "to invalid address %lx %s[%d]\n", current, 
                   entry, FILE__,__LINE__);
        return false;
    }

    // get instruction at func entry
    const unsigned char* bufPtr =
     (const unsigned char *)(_cr->getPtrToInstruction(entry));
    Offset entryOff = entry - _cr->offset();
    InstructionDecoder newdec( bufPtr,
                              _cr->length() - entryOff,
                              _cr->getArch() );
    IA_x86 *ah = new IA_x86(newdec, entry, _obj, _cr, _isrc, _curBlk);
    Instruction insn = ah->curInsn();

    // follow ctrl transfers until you get a block containing non-ctrl 
    // transfer instructions, or hit a return instruction
    while (insn.getCategory() == c_CallInsn ||
           insn.getCategory() == c_BranchInsn)
    {
       boost::tie(valid, entry) = ah->getCFT();
       if ( !valid || ! _cr->contains(entry) || ! _isrc->isCode(entry) ) {
          mal_printf("WARNING: found call to function at %lx that "
                     "leaves to %lx, out of the code region %s[%d]\n", 
                     current, entry, FILE__,__LINE__);
          return false;
       }
        bufPtr = (const unsigned char *)(_cr->getPtrToInstruction(entry));
        entryOff = entry - _cr->offset();
        delete(ah);
        newdec = InstructionDecoder(bufPtr, 
                                    _cr->length() - entryOff, 
                                    _cr->getArch());
        ah = new IA_x86(newdec, entry, _obj, _cr, _isrc, _curBlk);
        insn = ah->curInsn();
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
        if (insn.getCategory() == c_CallInsn ||
            insn.getCategory() == c_ReturnInsn ||
            insn.getCategory() == c_BranchInsn)
        {
            break;
        }

        // calculate instruction delta
        if(insn.isWritten(theStackPtr)) {
            entryID what = insn.getOperation().getID();
            int sign = 1;
            switch(what) 
            {
            case e_push:
                sign = -1;
                //FALLTHROUGH
            case e_pop: {
                int size = insn.getOperand(0).getValue()->size();
                stackDelta += sign * size;
                break;
            }
            case e_pushal:
                sign = -1;
                //FALLTHROUGH
            case e_popal:
            case e_popaw:
                if (1 == sign) {
                    mal_printf("popad ins'n at %lx in func at %lx changes sp "
                               "by %d. %s[%d]\n", ah->getAddr(), 
                               entry, 8 * sign * addrWidth, FILE__, __LINE__);
                }
                stackDelta += sign * 8 * addrWidth;
                break;
            case e_pushf:
                sign = -1;
                //FALLTHROUGH
            case e_popf:
            case e_popfd:
                stackDelta += sign * 4;
                if (1 == sign) {
                    mal_printf("popf ins'n at %lx in func at %lx changes sp "
                               "by %d. %s[%d]\n", ah->getAddr(), entry, 
                               sign * 4, FILE__, __LINE__);
                }
                break;
            case e_enter:
                //mal_printf("Saw enter instruction at %lx in isFakeCall, "
                //           "quitting early, assuming not fake "
                //           "%s[%d]\n",curAddr, FILE__,__LINE__);
                // unhandled case, but not essential for correct analysis
                delete ah;
                return false;
                break;
            case e_leave:
                mal_printf("WARNING: saw leave instruction "
                           "at %lx that is not handled by isFakeCall %s[%d]\n",
                           curAddr, FILE__,__LINE__);
                // unhandled, not essential for correct analysis, would
                // be a red flag if there wasn't an enter ins'n first and 
                // we didn't end in a return instruction
                break;
			case e_and:
				// Rounding off the stack pointer. 
				mal_printf("WARNING: saw and instruction at %lx that is not handled by isFakeCall %s[%d]\n",
					curAddr, FILE__, __LINE__);
				delete ah;
				return false;
				break;

            case e_sub:
                sign = -1;
                //FALLTHROUGH
            case e_add: {
                Operand arg = insn.getOperand(1);
                Result delta = arg.getValue()->eval();
                if(delta.defined) {
                    int delta_int = sign;
                    switch (delta.type) {
                    case u8:
                    case s8:
                        delta_int *= (int)delta.convert<char>();
                        break;
                    case u16:
                    case s16:
                        delta_int *= (int)delta.convert<short>();
                        break;
                    case u32:
                    case s32:
                        delta_int *= delta.convert<int>();
                        break;
                    default:
                        assert(0 && "got add/sub operand of unusual size");
                        break;
                    }
                    stackDelta += delta_int;
                } else if (sign == -1) {
                    delete ah;
                    return false;
                } else {
                    mal_printf("ERROR: in isFakeCall, add ins'n "
                               "at %lx (in first block of function at "
                               "%lx) modifies the sp but failed to evaluate "
                               "its arguments %s[%d]\n", 
                               ah->getAddr(), entry, FILE__, __LINE__);
                    delete ah;
                    return true;
                }
                break;
            }
            default: {
                fprintf(stderr,"WARNING: in isFakeCall non-push/pop "
                        "ins'n at %lx (in first block of function at "
                        "%lx) modifies the sp by an unknown amount. "
                        "%s[%d]\n", ah->getAddr(), entry, 
                        FILE__, __LINE__);
                break;
            } // end default block
            } // end switch
        }

        if (stackDelta > 0) {
            tampers=true;
        }

        // exit condition 2
        ah->advance();
        Instruction next = ah->curInsn();
        if (!next.isValid()) {
            break;
        }
        curAddr += insn.size();
        insn = next;
    } 

    // not a fake call if it ends w/ a return instruction
    if (insn.getCategory() == c_ReturnInsn) {
        delete ah;
        return false;
    }

    // if the stack delta is positive or the return address has been replaced
    // with an absolute value, it's a fake call, since in both cases 
    // the return address is gone and we cannot return to the caller
    if ( 0 < stackDelta || tampers ) {

        delete ah;
        return true;
    }

    delete ah;
    return false;
}

bool IA_x86::isIATcall(std::string &calleeName) const
{
    if (!isDynamicCall()) {
        return false;
    }

    if (!curInsn().readsMemory()) {
        return false;
    }

    std::set<Expression::Ptr> memReads;
    curInsn().getMemoryReadOperands(memReads);
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
    void * asciiPtr = _obj->cs()->getPtrToInstruction(entryAddr);
    if (!asciiPtr) {
        return false;
    }
    Address funcAsciiAddr = 2 + *(Address*) asciiPtr;
    if (!_obj->cs()->isValidAddress(funcAsciiAddr)) {
        return false;
    }

    // see if it's really a string that could be a function name
    char *funcAsciiPtr = (char*) _obj->cs()->getPtrToData(funcAsciiAddr);
    if (!funcAsciiPtr) {
        return false;
    }
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

    mal_printf("found IAT call at %lx to %s\n", current, funcAsciiPtr);
    calleeName = string(funcAsciiPtr);
    return true;
}

bool IA_x86::isNopJump() const
{
    InsnCategory cat = curInsn().getCategory();
    if (c_BranchInsn != cat) {
        return false;
    }
    bool valid; Address addr;
    boost::tie(valid, addr) = getCFT();
    if(valid && current+1 == addr) {
        return true;
    }
    return false;
}

bool IA_x86::isLinkerStub() const
{
    // No need for linker stubs on x86 platforms.
    return false;
}
