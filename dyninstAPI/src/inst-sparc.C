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

// $Id: inst-sparc.C,v 1.204 2008/05/08 21:52:24 legendre Exp $

#include "dyninstAPI/src/inst-sparc.h"

#include "dyninstAPI/src/rpcMgr.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/miniTramp.h"
#include "dyninstAPI/src/baseTramp.h"

#include "dyninstAPI/src/registerSpace.h"

#include "dyninstAPI/src/dyn_thread.h" // get_index

#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/h/BPatch.h"

#include "dyninstAPI/src/addressSpace.h"
#include "dyninstAPI/src/function.h"

#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

static dictionary_hash<std::string, unsigned> funcFrequencyTable(::Dyninst::stringhash);


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
#if 0
Register
emitOptReturn(instruction i, Register src, char *insn, Address &base, 
              bool noCost, const instPoint *location,
              bool for_multithreaded) {
    
    unsigned instr = i.raw;
    registerSpace *rs = NULL;

    cout << "Handling a special case for optimized return value." << endl;

    assert(((instr&0x3e000000)>>25) <= 16);

    if ((instr&0x02000)>>13)
        emitImm(plusOp, (instr&0x07c000)>>14, instr&0x01fff,
                ((instr&0x3e000000)>>25)+16, insn, base, noCost, rs);
    else
        (void) emitV(plusOp, (instr&0x07c000)>>14, instr&0x01fff,
             ((instr&0x3e000000)>>25)+16, insn, base, noCost);
    
    return emitR(getSysRetValOp, 0, 0, src, insn, base, noCost, location,
                 for_multithreaded);
}
#endif
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

//
// initDefaultPointFrequencyTable - define the expected call frequency of
//    procedures.  Currently we just define several one shots with a
//    frequency of one, and provide a hook to read a file with more accurate
//    information.
//
void initDefaultPointFrequencyTable()
{
    FILE *fp;
    float value;
    char name[512];

    funcFrequencyTable["main"] = 1;
    funcFrequencyTable["DYNINSTsampleValues"] = 1;
    funcFrequencyTable[EXIT_NAME] = 1;

    // try to read file.
    fp = fopen("freq.input", "r");
    if (!fp) {
        return;
    } else {
        bperr("found freq.input file\n");
    }
    while (!feof(fp)) {
        fscanf(fp, "%s %f\n", name, &value);
        funcFrequencyTable[name] = (int) value;
        bperr("adding %s %f\n", name, value);
    }
    fclose(fp);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

//
// return cost in cycles of executing at this point.  This is the cost
//   of the base tramp if it is the first at this point or 0 otherwise.
//
int instPoint::getPointCost()
{
   unsigned worstCost = 0;
   for (unsigned i = 0; i < instances.size(); i++) {
       if (instances[i]->multi()) {
           if (instances[i]->multi()->usesTrap()) {
              // Stop right here
              // Actually, probably don't want this if the "always
              // delivered" instrumentation happens
              return 9000; // Estimated trap cost
          }
          else {
              // Base tramp cost if we're first at point, otherwise
              // free (someone else paid)
              // Which makes no sense, since we're talking an entire instPoint.
              // So if there is a multitramp hooked up we use the base tramp cost.
              worstCost = 70; // Magic constant from before time
          }
      }
      else {
          // No multiTramp, so still free (we're not instrumenting here).
      }
  }
  return worstCost;
}

unsigned baseTramp::getBTCost() {
    return 70; // Need to dynamically configure!
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void registerSpace::initialize32() { 
    static bool done = false;
    if (done) return;
    done = true;
    // registers 8 to 15: out registers 
    // registers 16 to 22: local registers
    Register deadList[10] = { 16, 17, 18, 19, 20, 21, 22, 23};
    unsigned dead_reg_count = 8;

    pdvector<registerSlot *> registers;
    for (unsigned i = 0; i < dead_reg_count; i++) {
        char buf[128];
        sprintf(buf, "r%d", deadList[i]);
        registers.push_back(new registerSlot(deadList[i],
                                             buf,
                                             false,
                                             registerSlot::deadAlways,
                                             registerSlot::GPR));
    }
    registerSpace::createRegisterSpace(registers);
}

void registerSpace::initialize() {
    initialize32();
}


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void emitImm(opCode op, Register src1, RegValue src2imm, Register dest, 
             codeGen &gen, bool noCost, registerSpace *)
{
    RegValue op3 = -1;
    int result = -1;
    switch (op) {
        // integer ops
    case plusOp:
        op3 = ADDop3;
        insnCodeGen::generateImm(gen, op3, src1, src2imm, dest);
        break;
        
    case minusOp:
        op3 = SUBop3;
        insnCodeGen::generateImm(gen, op3, src1, src2imm, dest);
        break;
        
    case timesOp:
        op3 = SMULop3;
        if (isPowerOf2(src2imm,result) && (result<32))
                    insnCodeGen::generateLShift(gen, src1, (Register)result, dest);           
                else 
                    insnCodeGen::generateImm(gen, op3, src1, src2imm, dest);
                break;
                
            case divOp:
                op3 = SDIVop3;
                if (isPowerOf2(src2imm,result) && (result<32))
                    insnCodeGen::generateRShift(gen, src1, (Register)result, 
                                                dest);           
                else { // needs to set the Y register to zero first
                    // Set the Y register to zero: Zhichen
                    insnCodeGen::generateImm(gen, WRYop3, REG_G(0), 0, 0);
                    insnCodeGen::generateImm(gen, op3, src1, src2imm, dest);
                }
                break;

            // Bool ops
            case orOp:
                op3 = ORop3;
                insnCodeGen::generateImm(gen, op3, src1, src2imm, dest);
                break;

            case andOp:
                op3 = ANDop3;
                insnCodeGen::generateImm(gen, op3, src1, src2imm, dest);
                break;

            // rel ops
            // For a particular condition (e.g. <=) we need to use the
            // the opposite in order to get the right value (e.g. for >=
            // we need BLTcond) - naim
            case eqOp:
                insnCodeGen::generateImmRelOp(gen, BNEcond, src1, src2imm, dest);
                break;

            case neOp:
                insnCodeGen::generateImmRelOp(gen, BEcond, src1, src2imm, dest);
                break;

            case lessOp:
                insnCodeGen::generateImmRelOp(gen, BGEcond, src1, src2imm, dest);
                break;

            case leOp:
                insnCodeGen::generateImmRelOp(gen, BGTcond, src1, src2imm, dest);
                break;

            case greaterOp:
                insnCodeGen::generateImmRelOp(gen, BLEcond, src1, src2imm, dest);
                break;

            case geOp:
                insnCodeGen::generateImmRelOp(gen, BLTcond, src1, src2imm, dest);
                break;

            default:
                Register dest2 = gen.rs()->getScratchRegister(gen, noCost);
                emitV(loadConstOp, src2imm, dest2, dest2, gen, noCost);
                emitV(op, src1, dest2, dest, gen, noCost);
                break;
        }
        return;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

//
// All values based on Cypress0 && Cypress1 implementations as documented in
//   SPARC v.8 manual p. 291
//
int getInsnCost(opCode op)
{
    /* XXX Need to add branchOp */
    if (op == loadConstOp) {
        return(1);
    } else if (op ==  loadOp) {
        // sethi + load single
        return(1+1);
    } else if (op ==  loadIndirOp) {
        return(1);
    } else if (op ==  storeOp) {
        // sethi + store single
        // return(1+3); 
        // for SS-5 ?
        return(1+2); 
    } else if (op ==  storeIndirOp) {
        return(2); 
    } else if (op ==  ifOp) {
        // subcc
        // be
        // nop
        return(1+1+1);
    } else if (op ==  callOp) {
        int count = 0;

        // mov src1, %o0
        count += 1;

        // mov src2, %o1
        count += 1;

        // clr i2
        count += 1;

        // clr i3
        count += 1;

        // sethi
        count += 1;

        // jmpl
        count += 1;

        // noop
        count += 1;

        return(count);
    } else if (op ==  updateCostOp) {
        // sethi %hi(obsCost), %l0
        // ld [%lo + %lo(obsCost)], %l1
        // add %l1, <cost>, %l1
        // st %l1, [%lo + %lo(obsCost)]
        return(1+2+1+3);
    } else if (op ==  trampPreamble) {
        return(0);
    } else if (op == noOp) {
        // noop
        return(1);
    } else if (op == getParamOp) {
        return(0);
    } else {
        switch (op) {
            // rel ops
            case eqOp:
            case neOp:
            case lessOp:
            case leOp:
            case greaterOp:
            case geOp:
                // bne -- assume taken
                return(2);
                break;
            default:
                return(1);
                break;
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

bool isReturnInsn(instruction instr, Address addr, std::string name) {
    if (instr.isInsnType(RETmask, RETmatch) ||
        instr.isInsnType(RETLmask, RETLmatch)) {
        //  Why 8 or 12?
        //  According to the sparc arch manual (289), ret or retl are
        //   synthetic instructions for jmpl %i7+8, %g0 or jmpl %o7+8, %go.
        //  Apparently, the +8 is not really a hard limit though, as 
        //   sometimes some extra space is allocated after the jump 
        //   instruction for various reasons.
        //  1 possible reason is to include information on the size of
        //   returned structure (4 bytes).
        //  So, 8 or 12 here is a heuristic, but doesn't seem to 
        //   absolutely have to be true.
        //  -matt
        if (((*instr).resti.simm13 != 8) && ((*instr).resti.simm13 != 12) 
                    && ((*instr).resti.simm13 != 16)) {
          sprintf(errorLine,"WARNING: unsupported return at address 0x%lx"
                        " in function %s - appears to be return to PC + %i", 
                  addr, name.c_str(), (int)(*instr).resti.simm13);
          showErrorCallback(55, errorLine);
        } else { 
          return true;
        }
    }
    return false;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

bool isBranchInsn(instruction instr) {
    if ((*instr).branch.op == 0 
                && ((*instr).branch.op2 == 2 || (*instr).branch.op2 == 6)) 
          return true;
    return false;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

bool doNotOverflow(int value)
{
  // we are assuming that we have 13 bits to store the immediate operand.
  //if ( (value <= 16383) && (value >= -16384) ) return(true);
  if ( (value <= MAX_IMM13) && (value >= MIN_IMM13) ) return(true);
  else return(false);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

bool AddressSpace::getDynamicCallSiteArgs(instPoint *callSite,
                                     pdvector<AstNodePtr> &args) {
    const instruction &insn = callSite->insn();
    if (insn.isJmplInsn()) {
        //this instruction is a jmpl with i == 1, meaning it
        //calling function register rs1+simm13
        if((*insn).rest.i == 1){
            
            AstNodePtr base =  AstNode::operandNode(AstNode::origRegister,
                                                  (void *) (*insn).rest.rs1);
            AstNodePtr offset = AstNode::operandNode(AstNode::Constant,
                                          (void *) (*insn).resti.simm13);
            args.push_back( AstNode::operatorNode(plusOp, base, offset));
        }
        
        //This instruction is a jmpl with i == 0, meaning its
        //two operands are registers
        else if((*insn).rest.i == 0){
            //Calculate the byte offset from the contents of the %fp reg
            //that the registers from the previous stack frame
            //specified by rs1 and rs2 are stored on the stack
            args.push_back( AstNode::operatorNode(plusOp, 
                                                  AstNode::operandNode(AstNode::origRegister,
                                                                       (void *) (*insn).rest.rs1),
                                                  AstNode::operandNode(AstNode::origRegister,
                                                                       (void *) (*insn).rest.rs2)));
        }
        else assert(0);
        
        args.push_back( AstNode::operandNode(AstNode::Constant,
                                             (void *) callSite->addr()));
    }
    else if(insn.isTrueCallInsn()){
        //True call destinations are always statically determinable.
        //return true;
        fprintf(stderr, "%s[%d]:  dynamic call is statically determinable, FIXME\n",
                __FILE__, __LINE__);
        return false; //  but we don't generate any args here??
    }
    else return false;
    
    return true;
}


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

// Emit code to jump to function CALLEE without linking.  (I.e., when
// CALLEE returns, it returns to the current caller.)  On SPARC, we do
// this by ensuring that the register context upon entry to CALLEE is
// the register context of function we are instrumenting, popped once.
void emitFuncJump(opCode op, codeGen &gen, 
                  int_function *callee, AddressSpace * /*proc*/,
                  const instPoint *, bool)
{
    assert(op == funcJumpOp);
    Address addr = callee->getAddress();

    // This must mimic the generateRestores baseTramp method. 
    // TODO: make this work better :)

    for (unsigned g_iter = 0; g_iter <= 6; g_iter += 2) {
        // ldd [%fp + -8], %g0
        // ldd [%fp + -16], %g2
        // ldd [%fp + -24], %g4
        // ldd [%fp + -32], %g6
        
        insnCodeGen::generateLoadD(gen, REG_FPTR, 
                                   -(8+(g_iter*4)), REG_G(g_iter));
    }

    // And pop the stack
    // This is left here so that we don't put it back in accidentally.
    // The jmpl instruction stores the jump address; we don't want that.
    // So we "invisi-jump" by making the jump, _then_ executing the
    // restore.
    //insnCodeGen::generateSimple(i, RESTOREop3, 0, 0, 0);
    
    insnCodeGen::generateSetHi(gen, addr, 13);
    // don't want the return address to be used
    insnCodeGen::generateImm(gen, JMPLop3, 13, LOW10(addr), 0);
    insnCodeGen::generateSimple(gen, RESTOREop3, 0, 0, 0);
}

#include <sys/systeminfo.h>

// VG(4/24/2002) It seems a good idea to cache the result.
// This is not thread safe, but should be okay since the 
// result shoud be the same for all threads...
/*
 * function which check whether the architecture is 
 * sparcv8plus or not. For the earlier architectures 
 * it is not possible to support random instrumentation 
 */
bool isV8plusISA()
{
  static bool result;
  static bool gotresult = false;

  if(gotresult)
    return result;
  else {
    char isaOptions[256];

    if (sysinfo(SI_ISALIST, isaOptions, 256) < 0)
      return false;
    if (strstr(isaOptions, "sparcv8plus"))
      return true;
    return false;
  }
}

/*
 * function which check whether the architecture is 
 * sparcv9 or later. For the earlier architectures 
 * it is not possible to support ajacent arbitrary 
 * instrumentation points
 */
bool isV9ISA()
{
  static bool result;
  static bool gotresult = false;

  if(gotresult)
    return result;
  else { 
    char isaOptions[256];

    if (sysinfo(SI_ISALIST, isaOptions, 256) < 0)
      return false;
    if (strstr(isaOptions, "sparcv9"))
      return true;
    return false;
  }
}

extern bool relocateFunction(process *proc, instPoint *&location);
extern bool branchInsideRange(instruction insn, Address branchAddress, 
                              Address firstAddress, Address lastAddress); 
extern instPoint* find_overlap(pdvector<instPoint*> v, Address targetAddress);
extern void sorted_ips_vector(pdvector<instPoint*>&fill_in);

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

// Determine the maximum amount of space required for relocating
// an instruction. Must assume the instruction will be relocated
// anywhere in the address space.

unsigned relocatedInstruction::maxSizeRequired() {
    return insn->spaceToRelocate();
}


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


bool baseTramp::generateSaves(codeGen &gen,
                              registerSpace*, baseTrampInstance *) {
    if (isConservative()) {
        // Yadda
    }

    int frameShift = 0;
    if (isConservative()) 
        frameShift = -256;
    else 
        frameShift = -120;

    // save %sp, -120, %sp
    insnCodeGen::generateImm(gen, SAVEop3, REG_SPTR, frameShift, REG_SPTR);

    for (unsigned g_iter = 0; g_iter <= 6; g_iter += 2) {
        // std %g[iter], [ %fp + -(8 + (g_iter*4))]
        // Otherwise known as:
        // std %g0, [ %fp + -8]
        // std %g2, [ %fp + -16]
        // std %g4, [ %fp + -24]
        // std %g6, [ %fp + -32]

        insnCodeGen::generateStoreD(gen, REG_G(g_iter), REG_FPTR, 
                                    -(8+(g_iter*4)));
    }


    if (BPatch::bpatch->isForceSaveFPROn() || 
       (isConservative() && BPatch::bpatch->isSaveFPROn()) ) {
        for (unsigned f_iter = 0; f_iter <= 30; f_iter += 2) {
            // std %f[iter], [%fp + -(40 + iter*4)]
            insnCodeGen::generateStoreFD(gen, f_iter, REG_FPTR, 
                                         -(40 + (f_iter*4)));
        }
        
        // I hate constants... anyone know what this pattern means?
        // save codes to %g1?
        instruction saveConditionCodes(0x83408000);
        insnCodeGen::generate(gen,saveConditionCodes);
        
        // And store them.
        insnCodeGen::generateStore(gen, REG_G(1), REG_FPTR,
                                   -164);
    }
    
    return true;
}

bool baseTramp::generateRestores(codeGen &gen,
                                 registerSpace *,
                                 baseTrampInstance *) {
    if (BPatch::bpatch->isForceSaveFPROn() || 
       (isConservative() && BPatch::bpatch->isSaveFPROn())) {
        for (unsigned f_iter = 0; f_iter <= 30; f_iter += 2) {
            // std %f[iter], [%fp + -(40 + iter*4)]
            insnCodeGen::generateLoadFD(gen, REG_FPTR, 
                                        -(40+(f_iter*4)), f_iter);
        }
        
        // load them...
        insnCodeGen::generateLoad(gen, REG_FPTR,
                                  -164, REG_G(1));

        // I hate constants... anyone know what this pattern means?
        // restore codes from g1?
        instruction restoreConditionCodes(0x85806000);
        insnCodeGen::generate(gen,restoreConditionCodes);
    }


    for (unsigned g_iter = 0; g_iter <= 6; g_iter += 2) {
        // ldd [%fp + -8], %g0
        // ldd [%fp + -16], %g2
        // ldd [%fp + -24], %g4
        // ldd [%fp + -32], %g6
        
        insnCodeGen::generateLoadD(gen, REG_FPTR, 
                                   -(8+(g_iter*4)), REG_G(g_iter));
    }

    // And pop the stack

    insnCodeGen::generateSimple(gen, RESTOREop3, 0, 0, 0);

    return true;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

//This function returns true if the processor on which the daemon is running
//is an ultra SPARC, otherwise returns false.
bool isUltraSparc(){
  struct utsname u;
  if(uname(&u) < 0){
    cerr <<"Trouble in uname(), inst-sparc-solaris.C\n";
    return false;
  }
  if(!strcmp(u.machine, "sun4u")){
    return 1;
  }
  return false;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void emitLoadPreviousStackFrameRegister(Address register_num,
					Register dest,
					codeGen &gen,
					int size,
					bool noCost){
  if(register_num > 31)
    assert(0);
  else if(register_num > 15){
    /*Need to find it on the stack*/
    unsigned frame_offset = (register_num-16) * 4;
    /*generate a FLUSHW instruction, in order to make sure that
      the registers from the caller are on the caller's stack
      frame*/

    if(isUltraSparc())
        insnCodeGen::generateFlushw(gen);
    else 
        insnCodeGen::generateTrapRegisterSpill(gen);
    
    if(frame_offset == 0){
        emitV(loadIndirOp, 30, 0, dest, gen,  noCost, NULL, size);
    }	    
    else {
        emitImm(plusOp,(Register) 30,(RegValue)frame_offset, 
                dest, gen, noCost);
        emitV(loadIndirOp, dest, 0, dest, gen, noCost, NULL, size);
    }
  }	  
  else if(register_num > 7) { 
      //out registers become in registers, so we add 16 to the register
      //number to find it's value this stack frame. We move it's value
      //into the destination register
      emitV(orOp, (Register) register_num + 16, 0,  dest, gen, false);
  }
  else /* if(register_num >= 0) */ {
      int frame_offset;
      if(register_num % 2 == 0) 
          frame_offset = (register_num * -4) - 8;
      else 
          frame_offset = (register_num * -4);
      //read globals from the stack, they were saved in tramp-sparc.S
      emitImm(plusOp,(Register) 30,(RegValue)frame_offset, 
              dest, gen, noCost);
      emitV(loadIndirOp, dest, 0, dest, gen, noCost, NULL, size);
  }
  /* else assert(0); */
  
}

void emitStorePreviousStackFrameRegister(Address,
                                         Register,
                                         codeGen &,
                                         int,
                                         bool) {
    assert(0);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

Register emitFuncCall(opCode op, 
		      codeGen &gen, 
		      pdvector<AstNodePtr> &operands, 
		      bool noCost, int_function *callee)
{
   assert(op == callOp || op == funcJumpOp);
   pdvector <Register> srcs;
   void cleanUpAndExit(int status);
   
   // sanity check for NULL address argument
   if (!callee) {
     char msg[256];
     sprintf(msg, "%s[%d]:  internal error:  emitFuncCall called w/out"
             "callee argument", __FILE__, __LINE__);
     showErrorCallback(80, msg);
     assert(0);
   }
 
   for (unsigned u = 0; u < operands.size(); u++) {
       Register src = REG_NULL;
       Address unused = ADDR_NULL;
       if (!operands[u]->generateCode_phase2( gen, false, unused, src)) assert(0);
       assert(src != REG_NULL);
       srcs.push_back(src);
   }
   
   for (unsigned u=0; u<srcs.size(); u++){
      if (u >= 5) {
         std::string msg = "Too many arguments to function call in instrumentation code: only 5 arguments can be passed on the sparc architecture.\n";
         bperr( msg.c_str());
         showErrorCallback(94,msg);
         cleanUpAndExit(-1);
      }
      insnCodeGen::generateSimple(gen, ORop3, 0, srcs[u], u+8);
      gen.rs()->freeRegister(srcs[u]);
   }
   
   // As Ling pointed out to me, the following is rather inefficient.  It does:
   //   sethi %hi(addr), %o5
   //   jmpl %o5 + %lo(addr), %o7   ('call' pseudo-instr)
   //   nop
   // We can do better:
   //   call <addr>    (but note that the call true-instr is pc-relative jump)
   //   nop
   insnCodeGen::generateSetHi(gen, callee->getAddress(), 13);
   insnCodeGen::generateImm(gen, JMPLop3, 13, LOW10(callee->getAddress()), 15); 
   insnCodeGen::generateNOOP(gen);
   
   // return value is the register with the return value from the function.
   // This needs to be %o0 since it is back in the caller's scope.

   Register retReg = gen.rs()->allocateRegister(gen, noCost);
   
   // Move tmp to dest
   emitImm(orOp, REG_O(0), 0, retReg, gen, noCost, gen.rs());

   return retReg;
}

Register emitFuncCall(opCode op, 
		      codeGen &gen, 
		      pdvector<AstNodePtr> &operands, 
		      bool noCost, Address callee_addr_) {
    // Argh... our dlopen installation uses an address version :/
    
    assert(op == callOp);
    pdvector <Register> srcs;
    void cleanUpAndExit(int status);
    
   // sanity check for NULL address argument
   if (!callee_addr_) {
     char msg[256];
     sprintf(msg, "%s[%d]:  internal error:  emitFuncCall called w/out"
             "callee address argument", __FILE__, __LINE__);
     showErrorCallback(80, msg);
     assert(0);
   }
 
   for (unsigned u = 0; u < operands.size(); u++) {
       Register src = REG_NULL;
       Address unused = ADDR_NULL;
       if (!operands[u]->generateCode_phase2( gen, false, unused, src)) assert(0);
       assert(src != REG_NULL);
       srcs.push_back(src);
   }
   
   for (unsigned u=0; u<srcs.size(); u++){
      if (u >= 5) {
         std::string msg = "Too many arguments to function call in instrumentation code: only 5 arguments can be passed on the sparc architecture.\n";
         bperr( msg.c_str());
         showErrorCallback(94,msg);
         cleanUpAndExit(-1);
      }
      insnCodeGen::generateSimple(gen, ORop3, 0, srcs[u], u+8);
      gen.rs()->freeRegister(srcs[u]);
   }
   
   // As Ling pointed out to me, the following is rather inefficient.  It does:
   //   sethi %hi(addr), %o5
   //   jmpl %o5 + %lo(addr), %o7   ('call' pseudo-instr)
   //   nop
   // We can do better:
   //   call <addr>    (but note that the call true-instr is pc-relative jump)
   //   nop
   insnCodeGen::generateSetHi(gen, callee_addr_, 13);
   insnCodeGen::generateImm(gen, JMPLop3, 13, LOW10(callee_addr_), 15); 
   insnCodeGen::generateNOOP(gen);
   
   // return value is the register with the return value from the function.
   // This needs to be %o0 since it is back in the caller's scope.

   Register retReg = gen.rs()->allocateRegister(gen, noCost);
   
   // Move tmp to dest
   emitImm(orOp, retReg, 0, REG_O(0), gen, noCost, gen.rs());

   return retReg;
}

 
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


codeBufIndex_t emitA(opCode op, Register src1, Register /*src2*/, Register dest, 
              codeGen &gen, RegControl, bool /*noCost*/)
{
    //bperr("emitA(op=%d,src1=%d,src2=XX,dest=%d)\n",op,src1,dest);
    codeBufIndex_t retval;
    switch (op) {
    case ifOp: 
        // cmp src1,0
        insnCodeGen::generateImm(gen, SUBop3cc, src1, 0, 0);
	//genSimpleInsn(gen, SUBop3cc, src1, 0, 0); insn++;
        retval = gen.getIndex();
        insnCodeGen::generateCondBranch(gen, dest, BEcond, false);

	insnCodeGen::generateNOOP(gen);
        break;
    case branchOp: 
	// Unconditional branch
        retval = gen.getIndex();
        insnCodeGen::generateBranch(gen, dest);
        insnCodeGen::generateNOOP(gen);
        break;
    case trampPreamble: {
        return(0);      // let's hope this is expected!
        }
    default:
        abort();        // unexpected op for this emit!
    }
    return retval;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

Register emitR(opCode op, Register src1, Register src2, Register dest, 
               codeGen &gen, bool /*noCost*/,
               const instPoint *location, bool for_multithreaded)
{
    //bperr("emitR(op=%d,src1=%d,src2=XX,dest=XX)\n",op,src1);

    // Here is what a "well-named" astFlag seems to mean.
    // astFlag is true iff the function does not begin with a save
    // or our instumentation is placed before the save instruction.
    // Notice that we do emit a save in the basetramp in any case,
    // so if (astFlag) then arguments are in the previous frame's
    // O registers (this frame's I registers). If (!astFlag) then
    // arguments are two frames above us.
    // For callsite instrumentation, callee's arguments are always
    // in I registers (remember, we did a save)
    
    // From ast.C:
#if 0
    if (astFlag || location->getPointType() == callSite)
        src = emitR(getSysParamOp, (Register)oValue, Null_Register, 
                    dest, insn, base, noCost, location,
                    proc->multithread_capable());
    else 
        src = emitR(getParamOp, (Address)oValue, Null_Register,
                    dest, insn, base, noCost, location,
                    proc->multithread_capable());
    
#endif
    
#if 0
    if (loperand) {
        instruction instr;
        instr.raw = (unsigned)(loperand->oValue);
        src = emitOptReturn(instr, dest, insn, base, noCost, location,
                            proc->multithread_capable());
    }
    else if (astFlag)
        src = emitR(getSysRetValOp, 0, 0, dest, insn, base, noCost, location,
                    proc->multithread_capable());
#endif

    // Suppose we can check the instPoint info; for now assume that registers are
    // requested at function entry, and return at function exit.
    
    opCode opToUse;
    if (op == getParamOp) {
        if (location->getPointType() == functionEntry) {
            // Function entry: no save, parameters in input registers.
            opToUse = getSysParamOp;
        }
        else if (location->getPointType() == callSite) {
            // Call site parameters: in input registers.
            opToUse = getSysParamOp;
        }
        else {
            // Odd... not entry, not call site. We'll assume a save has
            // been executed. Really, don't do this.
            opToUse = getParamOp;
        }
    }
    else if (op == getRetValOp) {
        if (location->getPointType() == functionExit)
            opToUse = getRetValOp;
        else if (location->getPointType() == callSite)
            opToUse = getSysRetValOp;
        else {
            fprintf(stderr, "%s[%d]: Incorrect use of getRetValOp.\n",
                    __FILE__, __LINE__);
            assert(0);
        }
    }
    else {
        assert(0);
        return 0;
    }

    switch(opToUse) {
    case getParamOp: {
        if(for_multithreaded) {
            // saving CT/vector address on the stack
            insnCodeGen::generateStore(gen, REG_MT_POS, REG_FPTR, -40);
        }
        // We have managed to emit two saves since the entry point of
        // the function, so the first 6 parameters are in the previous
        // frame's I registers, and we need to pick them off the stack
        
        // generate the FLUSHW instruction to make sure that previous
        // windows are written to the register save area on the stack
        if (isUltraSparc()) {
            insnCodeGen::generateFlushw(gen);
        }
        else {
            insnCodeGen::generateTrapRegisterSpill(gen);
        }

        if (src1 < 6) {
            // The arg is in a previous frame's I register. Get it from
            // the register save area
            if (src2 != REG_NULL)
                insnCodeGen::generateStore(gen, src2, REG_FPTR, 4*8 + 4*src1);
            insnCodeGen::generateLoad(gen, REG_FPTR, 4*8 + 4*src1, dest);
        }
        else {
            // The arg is on the stack, two frames above us. Get the previous
            // FP (i6) from the register save area
            insnCodeGen::generateLoad(gen, REG_FPTR, 4*8 + 4*6, dest);
            // old fp is in dest now

            // Write new value, if any
            if (src2 != REG_NULL)
                insnCodeGen::generateStore(gen, src2, dest, 92 + 4 * (src1 - 6));

            // Finally, load the arg from the stack
            insnCodeGen::generateLoad(gen, dest, 92 + 4 * (src1 - 6), dest);
        }

        if(for_multithreaded) {
            // restoring CT/vector address back in REG_MT_POS
            insnCodeGen::generateLoad(gen, REG_FPTR, -40, REG_MT_POS);
        }

        return dest;
    }
    case getSysParamOp: {
        // We have emitted one save since the entry point of
        // the function, so the first 6 parameters are in this
        // frame's I registers
        if (src1 < 6) {
            // Param is in an I register
            if (src2 != REG_NULL)
                insnCodeGen::generateSimple(gen, ORop3, 0, src2, REG_I(src1));
            return(REG_I(src1));
        }
        else {
            // Param is on the stack
            if (src2 != REG_NULL)
                insnCodeGen::generateStore(gen, src2, REG_FPTR, 92 + 4 * (src1 - 6));
            insnCodeGen::generateLoad(gen, REG_FPTR, 92 + 4 * (src1 - 6), dest);
            return dest;
        }
    }
    case getRetValOp: {
        // We have emitted one save since the exit point of
        // the function, so the return value is in the previous frame's
        // I0, and we need to pick it from the stack

        // generate the FLUSHW instruction to make sure that previous
        // windows are written to the register save area on the stack
        if (isUltraSparc()) {
            insnCodeGen::generateFlushw(gen);
        }
        else {
            insnCodeGen::generateTrapRegisterSpill(gen);
        }

        if (src2 != REG_NULL)
            insnCodeGen::generateStore(gen, src2, REG_FPTR, 4*8);
        insnCodeGen::generateLoad(gen, REG_FPTR, 4*8, dest);
        return dest;
    }
    case getSysRetValOp:
        if (src2 != REG_NULL)
            insnCodeGen::generateSimple(gen, ORop3, 0, src2, REG_I(0));
        return(REG_I(0));

    default:
        abort();        // unexpected op for this emit!
    }
}

void emitJmpMC(int /*condition*/, int /*offset*/, codeGen & /*baseInsn*/)
{
    // Not needed for memory instrumentation, otherwise TBD
}

// VG(12/02/01): Emit code to add the original value of a register to
// another. The original value may need to be restored somehow...
static inline void emitAddOriginal(Register src, Register acc, 
                                   codeGen &gen, bool noCost)
{
    // Plan:
    // ignore g0 (r0), since we would be adding 0.
    // get    g1-g4 (r1-r4) from stack (where the base tramp saved them)
    // get    g5-g7 (r5-r7) also from stack, but issue an warning because
    // the SPARC psABI cleary states that G5-G7 SHOULD NOT BE EVER WRITTEN
    // in userland, as they may be changed arbitrarily by signal handlers!
    // So AFAICT it doesn't make much sense to read them...
    // get o0-o7 (r8-r15) by "shifting" the window back (i.e. get r24-r31)
    // get l0-l7 (r16-23) by flushing the register window to stack
    //                    and geeting them from there
    // get i0-i7 (r24-31) by --"--
    // Note: The last two may seem slow, but they're async. trap safe this way
    
    // All this is coded as a binary search.
    
    bool mustFree = false;
    Register temp;
    
    if (src >= 16) {
        // VG(12/06/01): Currently saving registers on demand is NOT
        // implemented on SPARC (dumps assert), so we can safely ignore it
        temp = gen.rs()->getScratchRegister(gen, noCost);
        mustFree = true;
        
        // Cause repeated spills, till all windows but current are clear
        insnCodeGen::generateFlushw(gen); // only ultraSPARC supported here
        
        // the spill trap puts these at offset from the previous %sp now %fp (r30)
        unsigned offset = (src-16) * sizeof(long); // FIXME for 64bit mutator/32bit mutatee
        insnCodeGen::generateLoad(gen, 30, offset, temp);
    }
    else if (src >= 8)
        temp = src + 16;
    else if (src >= 1) {
        // VG(12/06/01): Currently saving registers on demand is NOT
        // implemented on SPARC (dumps assert), so we can safely ignore it
        temp = gen.rs()->getScratchRegister(gen, noCost);
        mustFree = true;
        
        // the base tramp puts these at offset from %fp (r30)
        unsigned offset = ((src%2) ? src : (src+2)) * -sizeof(long); // FIXME too
        insnCodeGen::generateLoad(gen, 30, offset, temp);
        
        if (src >= 5)
            logLine("SPARC WARNING: Restoring original value of g5-g7 is unreliable!");
    }
    else // src == 0
        return;
    
    // add temp to dest;
    emitV(plusOp, temp, acc, acc, gen, noCost, 0);
    
}

// VG(11/30/01): Load in destination the effective address given
// by the address descriptor. Used for memory access stuff.
void emitASload(const BPatch_addrSpec_NP *as, Register dest, codeGen &gen,
		bool noCost)
{
    int imm = as->getImm();
    int ra  = as->getReg(0);
    int rb  = as->getReg(1);
    
    // TODO: optimize this to generate the minimum number of
    // instructions; think about schedule
    
    // emit code to load the immediate (constant offset) into dest; this
    // writes at baseInsn+base and updates base, we must update insn...
    emitVload(loadConstOp, (Address)imm, dest, dest, gen, noCost);
    
    // If ra is used in the address spec, allocate a temp register and
    // get the value of ra from stack into it
    if(ra > -1)
        emitAddOriginal(ra, dest, gen, noCost);
    
    // If rb is used in the address spec, allocate a temp register and
    // get the value of ra from stack into it
    if(rb > -1)
        emitAddOriginal(rb, dest, gen, noCost);
}

void emitCSload(const BPatch_addrSpec_NP *as, Register dest, codeGen &gen,
		bool noCost)
{
  emitASload(as, dest, gen, noCost);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

//
// load the original FP (before the dyninst saves) into register dest
//
int getFP(codeGen &gen, Register dest)
{
    insnCodeGen::generateSimple(gen, RESTOREop3, 0, 0, 0);
    insnCodeGen::generateStore(gen, REG_FPTR, REG_SPTR, 68);
	  
    insnCodeGen::generateImm(gen, SAVEop3, REG_SPTR, -112, REG_SPTR);
    insnCodeGen::generateLoad(gen, REG_SPTR, 112+68, dest); 


    return(4*instruction::size());
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void emitVload(opCode op, Address src1, Register src2, Register dest, 
               codeGen &gen, bool /*noCost*/, 
               registerSpace * /*rs*/, int size,
               const instPoint * /* location */, AddressSpace * /* proc */) 
{
    if (op == loadConstOp) {
        // dest = src1:imm    TODO
        
        if ((src1) > ( unsigned )MAX_IMM13 || (src1) < ( unsigned )MIN_IMM13) {
            // src1 is out of range of imm13, so we need an extra instruction
            insnCodeGen::generateSetHi(gen, src1, dest);
            
	    // or regd,imm,regd

            // Chance for optimization: we should check for LOW10(src1)==0,
            // and if so, don't generate the following bitwise-or instruction,
            // since in that case nothing would be done.

	    insnCodeGen::generateImm(gen, ORop3, dest, LOW10(src1), dest);
	} else {
	    // really or %g0,imm,regd
	    insnCodeGen::generateImm(gen, ORop3, 0, src1, dest);
	}
    } else if (op ==  loadOp) {
	// dest = [src1]   TODO
	insnCodeGen::generateSetHi(gen, src1, dest);

        if (size == 1)
            insnCodeGen::generateLoadB(gen, dest, LOW10(src1), dest);
        else if (size == 2)
            insnCodeGen::generateLoadH(gen, dest, LOW10(src1), dest);
        else
            insnCodeGen::generateLoad(gen, dest, LOW10(src1), dest);
    } else if (op ==  loadFrameRelativeOp) {
	// return the value that is FP offset from the original fp
	//   need to restore old fp and save it on the stack to get at it.
        
	getFP(gen, dest);
	if (((int) src1 < MIN_IMM13) || ((int) src1 > MAX_IMM13)) {
	    // offsets are signed!
	    int offset = (int) src1;

	    // emit sethi src2, offset
	    insnCodeGen::generateSetHi(gen, offset, src2);

	    // or src2, offset, src2
	    insnCodeGen::generateImm(gen, ORop3, src2, LOW10(offset), src2);

	    // add dest, src2, dest
	    insnCodeGen::generateSimple(gen, ADDop3, dest, src2, src2);

	    insnCodeGen::generateLoad(gen, src2, 0, dest);
	}  else {
	    insnCodeGen::generateLoad(gen, dest, src1, dest);
	}
    } else if (op == loadFrameAddr) {
	// offsets are signed!
	int offset = (int) src1;
        
	getFP(gen, dest);

	if (((int) offset < MIN_IMM13) || ((int) offset > MAX_IMM13)) {
	    // emit sethi src2, offset
	    insnCodeGen::generateSetHi(gen, offset, src2);

	    // or src2, offset, src2
	    insnCodeGen::generateImm(gen, ORop3, src2, LOW10(offset), src2);

	    // add dest, src2, dest
	    insnCodeGen::generateSimple(gen, ADDop3, dest, src2, dest);
	}  else {
	    // fp is in dest, just add the offset
	    insnCodeGen::generateImm(gen, ADDop3, dest, offset, dest);
	}
    } else if(op == loadRegRelativeAddr) {
      int offset = (int) src1;
      insnCodeGen::generateLoad(gen, src2, offset, dest);
      
    } else {
        abort();       // unexpected op for this emit!
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void emitVstore(opCode op, Register src1, Register src2, Address dest, 
                codeGen &gen, bool /*noCost*/, 
                registerSpace * /*rs*/, int /* size */,
                const instPoint * /* location */, AddressSpace * /* proc */)
{
    
    if (op == storeOp) {
        instruction insn;
	(*insn).sethi.op = FMT2op;
	(*insn).sethi.rd = src2;
	(*insn).sethi.op2 = SETHIop2;
	(*insn).sethi.imm22 = HIGH22(dest);
        insnCodeGen::generate(gen,insn);

	insnCodeGen::generateStore(gen, src1, src2, LOW10(dest));
    } else if (op == storeFrameRelativeOp) {
	// offsets are signed!
	int offset = (int) dest;

	getFP(gen, src2);

	if ((offset < MIN_IMM13) || (offset > MAX_IMM13)) {
	    // We are really one regsiter short here, so we put the
	    //   value to store onto the stack for part of the sequence
	    insnCodeGen::generateStore(gen, src1, REG_SPTR, 112+68);

	    insnCodeGen::generateSetHi(gen, offset, src1);

	    insnCodeGen::generateImm(gen, ORop3, src1, LOW10(offset), src1);

	    insnCodeGen::generateSimple(gen, ADDop3, src1, src2, src2);

	    insnCodeGen::generateLoad(gen, REG_SPTR, 112+68, src1); 

	    insnCodeGen::generateStore(gen, src1, src2, 0);
	} else {
	    insnCodeGen::generateStore(gen, src1, src2, dest);
	}
    } else {
        abort();       // unexpected op for this emit!
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void emitV(opCode op, Register src1, Register src2, Register dest, 
              codeGen &gen, bool /*noCost*/, 
           registerSpace * /*rs*/, int /* size */,
           const instPoint * /* location */, AddressSpace * /* proc */)
           
{
    //bperr("emitV(op=%d,src1=%d,src2=%d,dest=%d)\n",op,src1,src2,dest);
    
    assert ((op!=branchOp) && (op!=ifOp) && 
            (op!=trampPreamble));         // !emitA
    assert ((op!=getRetValOp) && (op!=getSysRetValOp) &&
            (op!=getParamOp) && (op!=getSysParamOp));           // !emitR
    assert ((op!=loadOp) && (op!=loadConstOp));                 // !emitVload
    assert ((op!=storeOp));                                     // !emitVstore
    assert ((op!=updateCostOp));                                // !emitVupdate
    
    
    if (op == loadIndirOp) {
	insnCodeGen::generateLoad(gen, src1, 0, dest);
    } else if (op == storeIndirOp) {
	insnCodeGen::generateStore(gen, src1, dest, 0);
    } else if (op == noOp) {
	insnCodeGen::generateNOOP(gen);
    } else if (op == saveRegOp) {
	// should never be called for this platform.
        // VG(12/02/01): Unfortunately allocateRegister *may* call this
	abort();
    } else {
        int op3=-1;
	switch (op) {
	    // integer ops
        case plusOp:
            op3 = ADDop3;
            break;
            
        case minusOp:
            op3 = SUBop3;
            break;
            
        case timesOp:
            op3 = SMULop3;
            break;
            
        case divOp:
            op3 = SDIVop3;
            //need to set the Y register to Zero, Zhichen
            insnCodeGen::generateImm(gen, WRYop3, REG_G(0), 0, 0);
            break;
            
	    // Bool ops
        case orOp:
            op3 = ORop3;
            break;
            
        case andOp:
            op3 = ANDop3;
            break;
            
	    // rel ops
	    // For a particular condition (e.g. <=) we need to use the
            // the opposite in order to get the right value (e.g. for >=
            // we need BLTcond) - naim
        case eqOp:
            insnCodeGen::generateRelOp(gen, BNEcond, src1, src2, dest);
            break;
            
        case neOp:
            insnCodeGen::generateRelOp(gen, BEcond, src1, src2, dest);
            break;
            
        case lessOp:
            insnCodeGen::generateRelOp(gen, BGEcond, src1, src2, dest);
            break;
            
        case leOp:
            insnCodeGen::generateRelOp(gen, BGTcond, src1, src2, dest);
            break;
            
        case greaterOp:
            insnCodeGen::generateRelOp(gen, BLEcond, src1, src2, dest);
            break;

        case geOp:
            insnCodeGen::generateRelOp(gen, BLTcond, src1, src2, dest);
            break;
            
        default:
            abort();
            break;
	}
        if (op3 != -1) {
            // I.E. was set above...
            insnCodeGen::generateSimple(gen, op3, src1, src2, dest);
        }
      }
   return;
}



/****************************************************************************/
/****************************************************************************/

int instPoint::liveRegSize()
{
  /* Stub function */
  return 0;
}

/**
 * Fills in an indirect function pointer at 'addr' to point to 'f'.
 **/
bool writeFunctionPtr(AddressSpace *p, Address addr, int_function *f)
{
   Address val_to_write = f->getAddress();
   return p->writeDataSpace((void *) addr, sizeof(Address), &val_to_write);   
}

Emitter *AddressSpace::getEmitter() {
    return NULL;
}
