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

// "Policy" class specification for interfacing with the ROSE
// instruction semantics engine. 
//
// Background:
//   The ROSE compiler suite provides a description of x86 instruction
// semantics. This operates in terms of a "policy" that states how each
// ROSE operation should map to your particular use. This policy class
// builds an AST representation of the instruction.
// 
// The instruction semantics is initialized with a copy of this class as
// a template parameter:
// AST_Policy policy;
// X86InstructionSemantics semantics<policy>;
// 
// The engine also takes a datatype template parameter which itself is
// parameterized on the bitsize of the data (e.g., 1, 16, 32, ...), so the
// more proper version is:
// X86InstructionSemantics semantics<policy, policy::Datatype>;
//
// where Datatype must take an int template parameter.

#if !defined(Sym_Eval_Policy_h)
#define Sym_Eval_Policy_h

#include "DynAST.h"
#include "Operation_impl.h"
#include "../h/Absloc.h"

#include <assert.h>
#include <map>
#include <stddef.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include <stdint.h>


#include "../rose/SgAsmx86Instruction.h"
#include "../rose/SgAsmPowerpcInstruction.h"
// Also need ROSE header files... argh. 

// For typedefs
#include "../h/SymEval.h"

namespace Dyninst {

namespace DataflowAPI {

// The ROSE symbolic evaluation engine wants a data type that
// is template parametrized on the number of bits in the data
// type. However, our ASTs don't have this, and a shared_ptr
// to an AST _definitely_ doesn't have it. Instead, we use
// a wrapper class (Handle) that is parametrized appropriately
// and contains a shared pointer. 

// This uses a pointer to a shared pointer. This is ordinarily a really
// bad idea, but stripping the pointer part makes the compiler allocate
// all available memory and crash. No idea why. 



template <size_t Len>
struct Handle {
  AST::Ptr *v_;
  Handle() : v_(NULL) {
    assert(0);
  }
  Handle(AST::Ptr v) {
    assert(v);
    v_ = new AST::Ptr(v);
  }
  Handle(const Handle &rhs) {
    v_ = new AST::Ptr(rhs.var());
  }
  Handle operator=(const Handle &rhs) {
    if (v_) delete v_;
    v_ = new AST::Ptr(rhs.var());
    return  *this;
  }
    ~Handle() { if (v_) delete v_; }
  
  template <size_t Len2>
  bool operator==(const Handle<Len2> &rhs) {
    return ((Len == Len2) && (*(var()) == *(rhs.var())));
  }

  AST::Ptr var() const { 
    assert(v_); return *v_;
  }
  
  //operator AST::Ptr() const { return *var; }
  //AST::Ptr operator->() const { return *var; }
};


 class SymEvalPolicy {
 public:

     SymEvalPolicy(Result_t &r,
                   Address addr,
                   Dyninst::Architecture a,
                   InstructionAPI::Instruction insn);

   ~SymEvalPolicy() {}

   void undefinedInstruction(SgAsmx86Instruction *);
   void undefinedInstruction(SgAsmPowerpcInstruction *);
   void undefinedInstructionCommon();
  
   void startInstruction(SgAsmx86Instruction *);
   void startInstruction(SgAsmPowerpcInstruction *);

   void finishInstruction(SgAsmx86Instruction *);
   void finishInstruction(SgAsmPowerpcInstruction *);
    
   // Policy classes must implement the following methods:
   Handle<32> readGPR(X86GeneralPurposeRegister r) {
     return Handle<32>(wrap(convert(r)));
   }
    
   void writeGPR(X86GeneralPurposeRegister r, Handle<32> value) {
     std::map<Absloc, Assignment::Ptr>::iterator i = aaMap.find(convert(r));
     if (i != aaMap.end()) {
       res[i->second] = value.var();
     } 
     else {
       /*
       std::cerr << "Warning: discarding write to GPR " << convert(r).format() << std::endl;
       for (std::map<Absloc, Assignment::Ptr>::iterator foozle = aaMap.begin();
	    foozle != aaMap.end(); ++foozle) {
	 std::cerr << "\t" << foozle->first.format() << std::endl;
       }
       */
     }
   }
   
   Handle<32> readGPR(unsigned int r) {
       return Handle<32>(wrap(convert(powerpc_regclass_gpr, r)));
   }

   void writeGPR(unsigned int r, Handle<32> value) {
       std::map<Absloc, Assignment::Ptr>::iterator i = aaMap.find(convert(powerpc_regclass_gpr, r));
       if (i != aaMap.end()) {
           res[i->second] = value.var();
       }
   }
   Handle<32> readSPR(unsigned int r) {
        return Handle<32>(wrap(convert(powerpc_regclass_spr, r)));
    }
    void writeSPR(unsigned int r, Handle<32> value) {
        std::map<Absloc, Assignment::Ptr>::iterator i = aaMap.find(convert(powerpc_regclass_spr, r));
        if (i != aaMap.end()) {
            res[i->second] = value.var();
        }
    }
    Handle<4> readCRField(unsigned int field) {
        return Handle<4>(wrap(convert(powerpc_regclass_cr, field)));
    }
    Handle<32> readCR() {
        return Handle<32>(wrap(convert(powerpc_regclass_cr, (unsigned int)-1)));
    }
    void writeCRField(unsigned int field, Handle<4> value) {
        std::map<Absloc, Assignment::Ptr>::iterator i = aaMap.find(convert(powerpc_regclass_cr, field));
        if (i != aaMap.end()) {
            res[i->second] = value.var();
        }
    }
   Handle<16> readSegreg(X86SegmentRegister r) {
     return Handle<16>(wrap(convert(r)));
   }
    void systemCall(unsigned char value)
    {
        fprintf(stderr, "WARNING: syscall %u detected; unhandled by semantics!\n", (unsigned int)(value));
    }
   void writeSegreg(X86SegmentRegister r, Handle<16> value) {
     std::map<Absloc, Assignment::Ptr>::iterator i = aaMap.find(convert(r));
     if (i != aaMap.end()) {
       res[i->second] = value.var();
     }
     // Otherwise we don't care. Annoying that we 
     // had to expand this register...
   }
    
   Handle<32> readIP() { 
     return ip_;
   }
    
   void writeIP(Handle<32> value) {
     // Always care about the IP...
     std::map<Absloc, Assignment::Ptr>::iterator i = aaMap.find(Absloc::makePC(arch));
     if (i != aaMap.end()) {
       res[i->second] = value.var();
     }
     ip_ = value;
     // Otherwise we don't care. Annoying that we 
     // had to expand this register...
   } 
    
   Handle<1> readFlag(X86Flag f) {
     return Handle<1>(wrap(convert(f)));
   }
    
   void writeFlag(X86Flag f, Handle<1> value) {
     std::map<Absloc, Assignment::Ptr>::iterator i = aaMap.find(convert(f));
     if (i != aaMap.end()) {
       res[i->second] = value.var();
     }
     // Otherwise we don't care. Annoying that we 
     // had to expand this register...
   }

   // Len here is the number of bits read, which we'll
   // turn into an argument of the ROSEOperation. 
    
   template <size_t Len>
     Handle<Len> readMemory(X86SegmentRegister /*segreg*/,
			    Handle<32> addr_,
			    Handle<1> cond) {
     if (cond == true_()) {
       return Handle<Len>(getUnaryAST(ROSEOperation::derefOp,
				      addr_.var(),
                                      Len));
     }
     else {
       return Handle<Len>(getBinaryAST(ROSEOperation::derefOp,
				       addr_.var(),
				       cond.var(),
                                       Len));
     }
   }
        
   template <size_t Len>
     Handle<Len> readMemory(Handle<32> addr_,
                            Handle<1> cond) {
    return Handle<Len>(getBinaryAST(ROSEOperation::derefOp,
                                    addr_.var(),
                                    cond.var(),
                                    Len));
     }
     template <size_t Len>
     void writeMemory(X86SegmentRegister,
		      Handle<32> addr_,
		      Handle<Len> data,
		      Handle<32> repeat,
		      Handle<1> cond) {
     // We can only write memory once per instruction. Therefore, 
     // instead of sweating what we're actually being handed (or
     // what AbsRegion we're actually assigning), we build in 
     // a generic "Memory" Absloc to aamap and use that. 

     std::map<Absloc, Assignment::Ptr>::iterator i = aaMap.find(Absloc(0));
     if (i != aaMap.end()) {
       i->second->out().setGenerator(addr_.var());
       i->second->out().setSize(Len);
       
       if (cond == true_()) {
	 res[i->second] = getBinaryAST(ROSEOperation::writeRepOp,
				       data.var(),
				       repeat.var());
       }
       else {
	 res[i->second] = getTernaryAST(ROSEOperation::writeRepOp,
					data.var(),
					cond.var(), 
					repeat.var());
       }
     }
   }

   template <size_t Len>
   void writeMemory(Handle<32> addr_,
                    Handle<Len> data,
                    Handle<1> cond) {
        std::map<Absloc, Assignment::Ptr>::iterator i = aaMap.find(Absloc(0));
        if (i != aaMap.end()) {
            i->second->out().setGenerator(addr_.var());
            i->second->out().setSize(Len);
            if (cond == true_()) {
                // Thinking about it... I think we avoid the "writeOp"
                // because it's implicit in what we're setting; the 
                // dereference goes on the left hand side rather than the
                // right
                res[i->second] = data.var();
            }
            else {
                res[i->second] = getBinaryAST(ROSEOperation::writeOp,
                        data.var(),
                        cond.var());
            }
        }
    }

   
   template <size_t Len>
     void writeMemory(X86SegmentRegister,
		      Handle<32> addr_,
		      Handle<Len> data,
		      Handle<1> cond) {
     std::map<Absloc, Assignment::Ptr>::iterator i = aaMap.find(Absloc(0));
     if (i != aaMap.end()) {
       i->second->out().setGenerator(addr_.var());
       i->second->out().setSize(Len);
       if (cond == true_()) {	 
	 // Thinking about it... I think we avoid the "writeOp"
	 // because it's implicit in what we're setting; the 
	 // dereference goes on the left hand side rather than the
	 // right
	 res[i->second] = data.var();
	 /*
	 res[i->second] = getUnaryAST(ROSEOperation::writeOp,
				      data.var());
	 */
       } 
       else {
	 res[i->second] = getBinaryAST(ROSEOperation::writeOp,
				       data.var(),
				       cond.var());
       }
     }
   }

   // Create a representation for a Len-bit constant
   template <size_t Len>
     Handle<Len> number(uint64_t n) {
     return Handle<Len>(getConstAST(n, Len));
   }

   Handle<1> true_() {
     return Handle<1>(getConstAST(1, 1));
   }

   Handle<1> false_() {
     return Handle<1>(getConstAST(0, 1));
   }

   // TODO FIXME - we need a "bottom".
   Handle<1> undefined_() {
     return Handle<1>(getBottomAST());
   }

   // If-then-else
    
   template <size_t Len>
     Handle<Len> ite(Handle<1> sel, 
		     Handle<Len> ifTrue, 
		     Handle<Len> ifFalse) {
     return Handle<Len>(getTernaryAST(ROSEOperation::ifOp,
				      sel.var(),
				      ifTrue.var(),
				      ifFalse.var()));
   }


   // UNARY OPERATIONS

   // This is actually a ternary operation with
   // the second parameter implicit in the
   // template specifications...
   // From/To specify the range of bits to 
   // extract from the data type.
   template<size_t From, size_t To, size_t Len> 
     Handle<To-From> extract(Handle<Len> a) {
     // return Handle<To-From>(a.var());
     return Handle<To-From>(getTernaryAST(ROSEOperation::extractOp, 
					  a.var(),
					  number<Len>(From).var(),
					  number<Len>(To).var(),
                                          To-From));
   }

   template <size_t Len>
     Handle<Len> invert(Handle<Len> a) {
     return Handle<Len>(getUnaryAST(ROSEOperation::invertOp, a.var()));
   }

   template <size_t Len>
     Handle<Len> negate(Handle<Len> a) {
     return Handle<Len>(getUnaryAST(ROSEOperation::negateOp, a.var()));
   }

   template <size_t From, size_t To> 
     Handle<To> signExtend(Handle<From> a) {
     return Handle<To>(getBinaryAST(ROSEOperation::signExtendOp,
				    a.var(),
				    number<32>(To).var()));
   }

   template <size_t Len>
     Handle<1> equalToZero(Handle<Len> a) {
     return Handle<1>(getUnaryAST(ROSEOperation::equalToZeroOp, a.var()));
   }

   template <size_t Len>
     Handle<Len> leastSignificantSetBit(Handle<Len> a) {
     return Handle<Len>(getUnaryAST(ROSEOperation::LSBSetOp, a.var()));
   }

   template <size_t Len>
     Handle<Len> mostSignificantSetBit(Handle<Len> a) {
     return Handle<Len>(getUnaryAST(ROSEOperation::MSBSetOp, a.var()));
   }

   // BINARY OPERATIONS

   template <size_t Len1, size_t Len2>
     Handle<Len1+Len2> concat(Handle<Len1> a, Handle<Len2> b) {
     return Handle<Len1+Len2>(getBinaryAST(ROSEOperation::concatOp, a.var(), b.var(), Len1+Len2));
   }

   template <size_t Len>
     Handle<Len> and_(Handle<Len> a, Handle<Len> b) {
     return Handle<Len>(getBinaryAST(ROSEOperation::andOp, a.var(), b.var()));
   }

    
   template <size_t Len>
     Handle<Len> or_(Handle<Len> a, Handle<Len> b) {
     return Handle<Len>(getBinaryAST(ROSEOperation::orOp, a.var(), b.var()));
   }

   template <size_t Len>    
     Handle<Len> xor_(Handle<Len> a, Handle<Len> b) {
     return Handle<Len>(getBinaryAST(ROSEOperation::xorOp, a.var(), b.var()));
   }
    
   template <size_t Len>    
     Handle<Len> add(Handle<Len> a, Handle<Len> b) {
     return Handle<Len>(getBinaryAST(ROSEOperation::addOp, a.var(), b.var()));
   }

   template <size_t Len, size_t SALen>
     Handle<Len> rotateLeft(Handle<Len> a, Handle<SALen> b) {
     return Handle<Len>(getBinaryAST(ROSEOperation::rotateLOp, a.var(), b.var()));
   }
    
   template <size_t Len, size_t SALen>
     Handle<Len> rotateRight(Handle<Len> a, Handle<SALen> b) {
     return Handle<Len>(getBinaryAST(ROSEOperation::rotateROp, a.var(), b.var()));
   }

   template <size_t Len, size_t SALen>
     Handle<Len> shiftLeft(Handle<Len> a, Handle<SALen> b) {
     return Handle<Len>(getBinaryAST(ROSEOperation::shiftLOp, a.var(), b.var()));
   }

   template <size_t Len, size_t SALen>
     Handle<Len> shiftRight(Handle<Len> a, Handle<SALen> b) {
     return Handle<Len>(getBinaryAST(ROSEOperation::shiftROp, a.var(), b.var()));
   }

   template <size_t Len, size_t SALen>
     Handle<Len> shiftRightArithmetic(Handle<Len> a, Handle<SALen> b) {
     return Handle<Len>(getBinaryAST(ROSEOperation::shiftRArithOp, a.var(), b.var()));
   }

   template<size_t Len1, size_t Len2>
     Handle<Len1+Len2> signedMultiply(Handle<Len1> a, Handle<Len2> b) {
     return Handle<Len1+Len2>(getBinaryAST(ROSEOperation::sMultOp, a.var(), b.var()));
   }

   template<size_t Len1, size_t Len2>
     Handle<Len1+Len2> unsignedMultiply(Handle<Len1> a, Handle<Len2> b) {
     return Handle<Len1+Len2>(getBinaryAST(ROSEOperation::uMultOp, a.var(), b.var()));
   }
 
   template<size_t Len1, size_t Len2>
     Handle<Len1> signedDivide(Handle<Len1> a, Handle<Len2> b) {
     return Handle<Len1>(getBinaryAST(ROSEOperation::sDivOp, a.var(), b.var()));
   }

   template<size_t Len1, size_t Len2>
     Handle<Len2> signedModulo(Handle<Len1> a, Handle<Len2> b) {
     return Handle<Len2>(getBinaryAST(ROSEOperation::sModOp, a.var(), b.var()));
   }

   template<size_t Len1, size_t Len2>    
     Handle<Len1> unsignedDivide(Handle<Len1> a, Handle<Len2> b) {
     return Handle<Len1>(getBinaryAST(ROSEOperation::uDivOp, a.var(), b.var()));
   }

   template<size_t Len1, size_t Len2>
     Handle<Len2> unsignedModulo(Handle<Len1> a, Handle<Len2> b) {
     return Handle<Len2>(getBinaryAST(ROSEOperation::sModOp, a.var(), b.var()));
   }
    
   // Ternary (??) operations
    
   template<size_t Len>
     Handle<Len> add3(Handle<Len> a, Handle<Len> b, Handle<1> c) {
     return Handle<Len>(getBinaryAST(ROSEOperation::addOp,
				     a.var(),
				     getBinaryAST(ROSEOperation::addOp,
						  b.var(),
						  c.var())));
   }
    
   template<size_t Len>
     Handle<Len> xor3(Handle<Len> a, Handle<Len> b, Handle<Len> c) {
     return Handle<Len>(getBinaryAST(ROSEOperation::xorOp,
				     a.var(),
				     getBinaryAST(ROSEOperation::xorOp,
						  b.var(),
						  c.var())));
   }
    
   template<size_t Len>
     Handle<Len> addWithCarries(Handle<Len> a, 
				Handle<Len> b,
				Handle<1> carryIn,
				Handle<Len>& carries) {
     Handle<Len+1> aa = Handle<Len+1>(extendByMSB<Len, Len+1>(a.var()));
     Handle<Len+1> bb = Handle<Len+1>(extendByMSB<Len, Len+1>(b.var()));
     Handle<Len+1> result = add3(aa, bb, carryIn);
     carries = extract<1,Len+1>(xor3(aa, bb, result));
     return extract<0,Len>(result);
   }

   // Misc
    
   void hlt() {}
   void interrupt(uint8_t) {}
    
   Handle<64> rdtsc() {
     return number<64>(0);
   }

   void startBlock(uint64_t) {}
   void finishBlock(uint64_t) {}

   Handle<32> filterIndirectJumpTarget(Handle<32> x) { return x; }

   Handle<32> filterCallTarget(Handle<32> x) { return x; }

   Handle<32> filterReturnTarget(Handle<32> x) { return x; }

   template <size_t From, size_t To>
     Handle<To> extendByMSB(Handle<From> a) {
     //return Handle<To>(a.var());
     return Handle<To>(getBinaryAST(ROSEOperation::extendMSBOp,
				    a.var(),
				    number<32>(To).var()));
   }

   bool failedTranslate() const { return failedTranslate_; }

 private:
   // Don't use this...
   SymEvalPolicy();

   // This is the thing we fill in ;)
   Result_t &res;

   Architecture arch;
   Address addr;

   Handle<32> ip_;

   bool failedTranslate_;

   // The Dyninst version of the instruction we're translating
   Dyninst::InstructionAPI::Instruction insn_;
    

   // The above is an Assignment::Ptr -> AST::Ptr map
   // But assignments aren't very easy to deal with.
   // So also construct a map from Abslocs
   // to Assignments for the registers; 
   // we use a generic "memory" absloc to handle
   // assignments to memory.

   std::map<Absloc, Assignment::Ptr> aaMap;

   // Conversion functions
   Absloc convert(X86GeneralPurposeRegister r);
   Absloc convert(X86SegmentRegister r);
   Absloc convert(X86Flag r);
   AST::Ptr wrap(Absloc r) {
     return VariableAST::create(Variable(AbsRegion(r), addr));
   }
   Absloc convert(PowerpcRegisterClass c, int n);


    AST::Ptr getConstAST(uint64_t n, size_t s) {
      return ConstantAST::create(Constant(n, s));
    }
                        
    AST::Ptr getBottomAST() {
      return BottomAST::create(false);
    }
                
    AST::Ptr getUnaryAST(ROSEOperation::Op op,
                         AST::Ptr a,
                         size_t s = 0) {
      return RoseAST::create(ROSEOperation(op, s), a);
    }

    AST::Ptr getBinaryAST(ROSEOperation::Op op,
                          AST::Ptr a,
                          AST::Ptr b,
                          size_t s = 0) {
      return RoseAST::create(ROSEOperation(op, s), a, b);
    }
    
    AST::Ptr getTernaryAST(ROSEOperation::Op op,
                           AST::Ptr a,
                           AST::Ptr b,
                           AST::Ptr c,
                           size_t s = 0) {
      return RoseAST::create(ROSEOperation(op, s), a, b, c);
    }    
};




 class SymEvalPolicy_64 {
 public:

     SymEvalPolicy_64(Result_t &r,
                      Address addr,
		      Dyninst::Architecture a,
		      Dyninst::InstructionAPI::Instruction insn);

   ~SymEvalPolicy_64() {}

   void undefinedInstruction(SgAsmx86Instruction *);
   void undefinedInstruction(SgAsmPowerpcInstruction *);
   void undefinedInstructionCommon();
  
   void startInstruction(SgAsmx86Instruction *);
   void startInstruction(SgAsmPowerpcInstruction *);

   void finishInstruction(SgAsmx86Instruction *);
   void finishInstruction(SgAsmPowerpcInstruction *);
    
   // Policy classes must implement the following methods:
   Handle<64> readGPR(X86GeneralPurposeRegister r) {
     return Handle<64>(wrap(convert(r)));
   }
    
   void writeGPR(X86GeneralPurposeRegister r, Handle<64> value) {
     std::map<Absloc, Assignment::Ptr>::iterator i = aaMap.find(convert(r));
     if (i != aaMap.end()) {
       res[i->second] = value.var();
     } 
     else {
       /*
       std::cerr << "Warning: discarding write to GPR " << convert(r).format() << std::endl;
       for (std::map<Absloc, Assignment::Ptr>::iterator foozle = aaMap.begin();
	    foozle != aaMap.end(); ++foozle) {
	 std::cerr << "\t" << foozle->first.format() << std::endl;
       }
       */
     }
   }
   
   Handle<64> readGPR(unsigned int r) {
       return Handle<64>(wrap(convert(powerpc_regclass_gpr, r)));
   }

   void writeGPR(unsigned int r, Handle<64> value) {
       std::map<Absloc, Assignment::Ptr>::iterator i = aaMap.find(convert(powerpc_regclass_gpr, r));
       if (i != aaMap.end()) {
           res[i->second] = value.var();
       }
   }
   Handle<64> readSPR(unsigned int r) {
        return Handle<64>(wrap(convert(powerpc_regclass_spr, r)));
    }
    void writeSPR(unsigned int r, Handle<64> value) {
        std::map<Absloc, Assignment::Ptr>::iterator i = aaMap.find(convert(powerpc_regclass_spr, r));
        if (i != aaMap.end()) {
            res[i->second] = value.var();
        }
    }
    Handle<4> readCRField(unsigned int field) {
        return Handle<4>(wrap(convert(powerpc_regclass_cr, field)));
    }
    Handle<32> readCR() {
        return Handle<32>(wrap(convert(powerpc_regclass_cr, (unsigned int)-1)));
    }
    void writeCRField(unsigned int field, Handle<4> value) {
        std::map<Absloc, Assignment::Ptr>::iterator i = aaMap.find(convert(powerpc_regclass_cr, field));
        if (i != aaMap.end()) {
            res[i->second] = value.var();
        }
    }
   Handle<64> readSegreg(X86SegmentRegister r) {
     return Handle<64>(wrap(convert(r)));
   }
    void systemCall(unsigned char value)
    {
        fprintf(stderr, "WARNING: syscall %u detected; unhandled by semantics!\n", (unsigned int)(value));
    }
   void writeSegreg(X86SegmentRegister r, Handle<16> value) {
     std::map<Absloc, Assignment::Ptr>::iterator i = aaMap.find(convert(r));
     if (i != aaMap.end()) {
       res[i->second] = value.var();
     }
     // Otherwise we don't care. Annoying that we 
     // had to expand this register...
   }
    
   Handle<64> readIP() { 
     return ip_;
   }
    
   void writeIP(Handle<64> value) {
     // Always care about the IP...
     std::map<Absloc, Assignment::Ptr>::iterator i = aaMap.find(Absloc::makePC(arch));
     if (i != aaMap.end()) {
       res[i->second] = value.var();
     }
     ip_ = value;
     // Otherwise we don't care. Annoying that we 
     // had to expand this register...
   } 
    
   Handle<1> readFlag(X86Flag f) {
     return Handle<1>(wrap(convert(f)));
   }
    
   void writeFlag(X86Flag f, Handle<1> value) {
     std::map<Absloc, Assignment::Ptr>::iterator i = aaMap.find(convert(f));
     if (i != aaMap.end()) {
       res[i->second] = value.var();
     }
     // Otherwise we don't care. Annoying that we 
     // had to expand this register...
   }

   // Len here is the number of bits read, which we'll
   // turn into an argument of the ROSEOperation. 
    
   template <size_t Len>
     Handle<Len> readMemory(X86SegmentRegister /*segreg*/,
			    Handle<64> addr_,
			    Handle<1> cond) {
     if (cond == true_()) {
       return Handle<Len>(getUnaryAST(ROSEOperation::derefOp,
				      addr_.var(),
                                      Len));
     }
     else {
       return Handle<Len>(getBinaryAST(ROSEOperation::derefOp,
				       addr_.var(),
				       cond.var(),
                                       Len));
     }
   }
        
   template <size_t Len>
     Handle<Len> readMemory(Handle<64> addr_,
                            Handle<1> cond) {
    return Handle<Len>(getBinaryAST(ROSEOperation::derefOp,
                                    addr_.var(),
                                    cond.var(),
                                    Len));
     }
     template <size_t Len>
     void writeMemory(X86SegmentRegister,
		      Handle<64> addr_,
		      Handle<Len> data,
		      Handle<64> repeat,
		      Handle<1> cond) {
     // We can only write memory once per instruction. Therefore, 
     // instead of sweating what we're actually being handed (or
     // what AbsRegion we're actually assigning), we build in 
     // a generic "Memory" Absloc to aamap and use that. 

     std::map<Absloc, Assignment::Ptr>::iterator i = aaMap.find(Absloc(0));
     if (i != aaMap.end()) {
       i->second->out().setGenerator(addr_.var());
       i->second->out().setSize(Len);
       
       if (cond == true_()) {
	 res[i->second] = getBinaryAST(ROSEOperation::writeRepOp,
				       data.var(),
				       repeat.var());
       }
       else {
	 res[i->second] = getTernaryAST(ROSEOperation::writeRepOp,
					data.var(),
					cond.var(), 
					repeat.var());
       }
     }
   }

   template <size_t Len>
   void writeMemory(Handle<64> addr_,
                    Handle<Len> data,
                    Handle<1> cond) {
        std::map<Absloc, Assignment::Ptr>::iterator i = aaMap.find(Absloc(0));
        if (i != aaMap.end()) {
            i->second->out().setGenerator(addr_.var());
            i->second->out().setSize(Len);
            if (cond == true_()) {
                // Thinking about it... I think we avoid the "writeOp"
                // because it's implicit in what we're setting; the 
                // dereference goes on the left hand side rather than the
                // right
                res[i->second] = data.var();
            }
            else {
                res[i->second] = getBinaryAST(ROSEOperation::writeOp,
                        data.var(),
                        cond.var());
            }
        }
    }

   
   template <size_t Len>
     void writeMemory(X86SegmentRegister,
		      Handle<64> addr_,
		      Handle<Len> data,
		      Handle<1> cond) {
     std::map<Absloc, Assignment::Ptr>::iterator i = aaMap.find(Absloc(0));
     if (i != aaMap.end()) {
       i->second->out().setGenerator(addr_.var());
       i->second->out().setSize(Len);
       if (cond == true_()) {	 
	 // Thinking about it... I think we avoid the "writeOp"
	 // because it's implicit in what we're setting; the 
	 // dereference goes on the left hand side rather than the
	 // right
	 res[i->second] = data.var();
	 /*
	 res[i->second] = getUnaryAST(ROSEOperation::writeOp,
				      data.var());
	 */
       } 
       else {
	 res[i->second] = getBinaryAST(ROSEOperation::writeOp,
				       data.var(),
				       cond.var());
       }
     }
   }

   // Create a representation for a Len-bit constant
   template <size_t Len>
     Handle<Len> number(uint64_t n) {
     return Handle<Len>(getConstAST(n, Len));
   }

   Handle<1> true_() {
     return Handle<1>(getConstAST(1, 1));
   }

   Handle<1> false_() {
     return Handle<1>(getConstAST(0, 1));
   }

   // TODO FIXME - we need a "bottom".
   Handle<1> undefined_() {
     return Handle<1>(getBottomAST());
   }

   // If-then-else
    
   template <size_t Len>
     Handle<Len> ite(Handle<1> sel, 
		     Handle<Len> ifTrue, 
		     Handle<Len> ifFalse) {
     return Handle<Len>(getTernaryAST(ROSEOperation::ifOp,
				      sel.var(),
				      ifTrue.var(),
				      ifFalse.var()));
   }


   // UNARY OPERATIONS

   // This is actually a ternary operation with
   // the second parameter implicit in the
   // template specifications...
   // From/To specify the range of bits to 
   // extract from the data type.
   template<size_t From, size_t To, size_t Len> 
     Handle<To-From> extract(Handle<Len> a) {
     // return Handle<To-From>(a.var());
     return Handle<To-From>(getTernaryAST(ROSEOperation::extractOp, 
					  a.var(),
					  number<Len>(From).var(),
					  number<Len>(To).var(),
                                          To-From));
   }

   template <size_t Len>
     Handle<Len> invert(Handle<Len> a) {
     return Handle<Len>(getUnaryAST(ROSEOperation::invertOp, a.var()));
   }

   template <size_t Len>
     Handle<Len> negate(Handle<Len> a) {
     return Handle<Len>(getUnaryAST(ROSEOperation::negateOp, a.var()));
   }

   template <size_t From, size_t To> 
     Handle<To> signExtend(Handle<From> a) {
     return Handle<To>(getBinaryAST(ROSEOperation::signExtendOp,
				    a.var(),
				    number<32>(To).var()));
   }

   template <size_t Len>
     Handle<1> equalToZero(Handle<Len> a) {
     return Handle<1>(getUnaryAST(ROSEOperation::equalToZeroOp, a.var()));
   }

   template <size_t Len>
     Handle<Len> leastSignificantSetBit(Handle<Len> a) {
     return Handle<Len>(getUnaryAST(ROSEOperation::LSBSetOp, a.var()));
   }

   template <size_t Len>
     Handle<Len> mostSignificantSetBit(Handle<Len> a) {
     return Handle<Len>(getUnaryAST(ROSEOperation::MSBSetOp, a.var()));
   }

   // BINARY OPERATIONS

   template <size_t Len1, size_t Len2>
     Handle<Len1+Len2> concat(Handle<Len1> a, Handle<Len2> b) {
     return Handle<Len1+Len2>(getBinaryAST(ROSEOperation::concatOp, a.var(), b.var(), Len1+Len2));
   }

   template <size_t Len>
     Handle<Len> and_(Handle<Len> a, Handle<Len> b) {
     return Handle<Len>(getBinaryAST(ROSEOperation::andOp, a.var(), b.var()));
   }

    
   template <size_t Len>
     Handle<Len> or_(Handle<Len> a, Handle<Len> b) {
     return Handle<Len>(getBinaryAST(ROSEOperation::orOp, a.var(), b.var()));
   }

   template <size_t Len>    
     Handle<Len> xor_(Handle<Len> a, Handle<Len> b) {
     return Handle<Len>(getBinaryAST(ROSEOperation::xorOp, a.var(), b.var()));
   }
    
   template <size_t Len>    
     Handle<Len> add(Handle<Len> a, Handle<Len> b) {
     return Handle<Len>(getBinaryAST(ROSEOperation::addOp, a.var(), b.var()));
   }

   template <size_t Len, size_t SALen>
     Handle<Len> rotateLeft(Handle<Len> a, Handle<SALen> b) {
     return Handle<Len>(getBinaryAST(ROSEOperation::rotateLOp, a.var(), b.var()));
   }
    
   template <size_t Len, size_t SALen>
     Handle<Len> rotateRight(Handle<Len> a, Handle<SALen> b) {
     return Handle<Len>(getBinaryAST(ROSEOperation::rotateROp, a.var(), b.var()));
   }

   template <size_t Len, size_t SALen>
     Handle<Len> shiftLeft(Handle<Len> a, Handle<SALen> b) {
     return Handle<Len>(getBinaryAST(ROSEOperation::shiftLOp, a.var(), b.var()));
   }

   template <size_t Len, size_t SALen>
     Handle<Len> shiftRight(Handle<Len> a, Handle<SALen> b) {
     return Handle<Len>(getBinaryAST(ROSEOperation::shiftROp, a.var(), b.var()));
   }

   template <size_t Len, size_t SALen>
     Handle<Len> shiftRightArithmetic(Handle<Len> a, Handle<SALen> b) {
     return Handle<Len>(getBinaryAST(ROSEOperation::shiftRArithOp, a.var(), b.var()));
   }

   template<size_t Len1, size_t Len2>
     Handle<Len1+Len2> signedMultiply(Handle<Len1> a, Handle<Len2> b) {
     return Handle<Len1+Len2>(getBinaryAST(ROSEOperation::sMultOp, a.var(), b.var()));
   }

   template<size_t Len1, size_t Len2>
     Handle<Len1+Len2> unsignedMultiply(Handle<Len1> a, Handle<Len2> b) {
     return Handle<Len1+Len2>(getBinaryAST(ROSEOperation::uMultOp, a.var(), b.var()));
   }
 
   template<size_t Len1, size_t Len2>
     Handle<Len1> signedDivide(Handle<Len1> a, Handle<Len2> b) {
     return Handle<Len1>(getBinaryAST(ROSEOperation::sDivOp, a.var(), b.var()));
   }

   template<size_t Len1, size_t Len2>
     Handle<Len2> signedModulo(Handle<Len1> a, Handle<Len2> b) {
     return Handle<Len2>(getBinaryAST(ROSEOperation::sModOp, a.var(), b.var()));
   }

   template<size_t Len1, size_t Len2>    
     Handle<Len1> unsignedDivide(Handle<Len1> a, Handle<Len2> b) {
     return Handle<Len1>(getBinaryAST(ROSEOperation::uDivOp, a.var(), b.var()));
   }

   template<size_t Len1, size_t Len2>
     Handle<Len2> unsignedModulo(Handle<Len1> a, Handle<Len2> b) {
     return Handle<Len2>(getBinaryAST(ROSEOperation::sModOp, a.var(), b.var()));
   }
    
   // Ternary (??) operations
    
   template<size_t Len>
     Handle<Len> add3(Handle<Len> a, Handle<Len> b, Handle<1> c) {
     return Handle<Len>(getBinaryAST(ROSEOperation::addOp,
				     a.var(),
				     getBinaryAST(ROSEOperation::addOp,
						  b.var(),
						  c.var())));
   }
    
   template<size_t Len>
     Handle<Len> xor3(Handle<Len> a, Handle<Len> b, Handle<Len> c) {
     return Handle<Len>(getBinaryAST(ROSEOperation::xorOp,
				     a.var(),
				     getBinaryAST(ROSEOperation::xorOp,
						  b.var(),
						  c.var())));
   }
    
   template<size_t Len>
     Handle<Len> addWithCarries(Handle<Len> a, 
				Handle<Len> b,
				Handle<1> carryIn,
				Handle<Len>& carries) {
     Handle<Len+1> aa = Handle<Len+1>(extendByMSB<Len, Len+1>(a.var()));
     Handle<Len+1> bb = Handle<Len+1>(extendByMSB<Len, Len+1>(b.var()));
     Handle<Len+1> result = add3(aa, bb, carryIn);
     carries = extract<1,Len+1>(xor3(aa, bb, result));
     return extract<0,Len>(result);
   }

   // Misc
    
   void hlt() {}
   void interrupt(uint8_t) {}
    
   Handle<64> rdtsc() {
     return number<64>(0);
   }

   void startBlock(uint64_t) {}
   void finishBlock(uint64_t) {}

   Handle<64> filterIndirectJumpTarget(Handle<64> x) { return x; }

   Handle<64> filterCallTarget(Handle<64> x) { return x; }

   Handle<64> filterReturnTarget(Handle<64> x) { return x; }

   template <size_t From, size_t To>
     Handle<To> extendByMSB(Handle<From> a) {
     //return Handle<To>(a.var());
     return Handle<To>(getBinaryAST(ROSEOperation::extendMSBOp,
				    a.var(),
				    number<64>(To).var()));
   }

   bool failedTranslate() const { return failedTranslate_; }

 private:
   // Don't use this...
   SymEvalPolicy_64();

   // This is the thing we fill in ;)
   Result_t &res;

   Architecture arch;
   Address addr;

   Handle<64> ip_;

   bool failedTranslate_;

   // The Dyninst version of the instruction we're translating
   Dyninst::InstructionAPI::Instruction insn_;
    

   // The above is an Assignment::Ptr -> AST::Ptr map
   // But assignments aren't very easy to deal with.
   // So also construct a map from Abslocs
   // to Assignments for the registers; 
   // we use a generic "memory" absloc to handle
   // assignments to memory.

   std::map<Absloc, Assignment::Ptr> aaMap;

   // Conversion functions
   Absloc convert(X86GeneralPurposeRegister r);
   Absloc convert(X86SegmentRegister r);
   Absloc convert(X86Flag r);
   AST::Ptr wrap(Absloc r) {
     return VariableAST::create(Variable(AbsRegion(r), addr));
   }
   Absloc convert(PowerpcRegisterClass c, int n);


    AST::Ptr getConstAST(uint64_t n, size_t s) {
      return ConstantAST::create(Constant(n, s));
    }
                        
    AST::Ptr getBottomAST() {
      return BottomAST::create(false);
    }
                
    AST::Ptr getUnaryAST(ROSEOperation::Op op,
                         AST::Ptr a,
                         size_t s = 0) {
      return RoseAST::create(ROSEOperation(op, s), a);
    }

    AST::Ptr getBinaryAST(ROSEOperation::Op op,
                          AST::Ptr a,
                          AST::Ptr b,
                          size_t s = 0) {
      return RoseAST::create(ROSEOperation(op, s), a, b);
    }
    
    AST::Ptr getTernaryAST(ROSEOperation::Op op,
                           AST::Ptr a,
                           AST::Ptr b,
                           AST::Ptr c,
                           size_t s = 0) {
      return RoseAST::create(ROSEOperation(op, s), a, b, c);
    }    
};
}
}
#endif
