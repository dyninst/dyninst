/*
 * Copyright (c) 1996-2011 Barton P. Miller
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

/*
 * emit-x86.h - x86 & AMD64 code generators
 * $Id: emit-power.h,v 1.6 2008/03/25 19:24:26 bernat Exp $
 */

#ifndef _EMITTER_POWER_H
#define _EMITTER_POWER_H

#include "common/h/headers.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/src/ast.h"

#include "dyninstAPI/src/emitter.h"

class codeGen;
class registerSpace;
class baseTramp;

// class for encapsulating
// platform dependent code generation functions
class EmitterPOWER : public Emitter {

 public:
    virtual ~EmitterPOWER() {};
    virtual codeBufIndex_t emitIf(Register, Register, RegControl, codeGen &) { assert(0); return 0; }
    virtual void emitOp(unsigned, Register, Register, Register, codeGen &) { assert(0); }
    virtual void emitOpImm(unsigned, unsigned, Register, Register, RegValue,
			   codeGen &) { assert(0); }
    virtual void emitRelOp(unsigned, Register, Register, Register, codeGen &) { assert(0); }
    virtual void emitRelOpImm(unsigned, Register, Register, RegValue, codeGen &) { assert(0); }
    virtual void emitDiv(Register, Register, Register, codeGen &) { assert(0); }
    virtual void emitTimesImm(Register, Register, RegValue, codeGen &) { assert(0); }
    virtual void emitDivImm(Register, Register, RegValue, codeGen &) { assert(0); }
    virtual void emitLoad(Register, Address, int, codeGen &) { assert(0); }
    virtual void emitLoadConst(Register, Address, codeGen &) { assert(0); }
    virtual void emitLoadIndir(Register, Register, int, codeGen &) { assert(0); }
    virtual bool emitCallRelative(Register, Address, Register, codeGen &);
    virtual bool emitLoadRelative(Register, Address, Register, int, codeGen &);
    virtual void emitLoadShared(opCode op, Register dest, const image_variable *var, bool is_local, int size, codeGen &gen, Address offset);
    virtual void emitLoadFrameAddr(Register, Address, codeGen &) { assert(0); }

    // These implicitly use the stored original/non-inst value
    virtual void emitLoadOrigFrameRelative(Register, Address, codeGen &) { assert(0); }
    virtual void emitLoadOrigRegRelative(Register, Address, Register, codeGen &, bool) { assert(0); }
    virtual void emitLoadOrigRegister(Address, Register, codeGen &) { assert(0); }

    virtual void emitStore(Address, Register, int, codeGen &) { assert(0); }
    virtual void emitStoreIndir(Register, Register, int, codeGen &) { assert(0); }
    virtual void emitStoreFrameRelative(Address, Register, Register, int, codeGen &) { assert(0); }
    virtual void emitStoreRelative(Register, Address, Register, int, codeGen &);
    virtual void emitStoreShared(Register source, const image_variable *var, bool is_local, int size, codeGen &gen);


    virtual void emitStoreOrigRegister(Address, Register, codeGen &) { assert(0); }

    virtual bool emitMoveRegToReg(Register, Register, codeGen &) { assert(0); return 0;}
    virtual bool emitMoveRegToReg(registerSlot *src, registerSlot *dest, codeGen &gen);

    virtual void emitMovPCToReg(Register, codeGen& gen);

    // This one we actually use now.
    virtual Register emitCall(opCode, codeGen &, const pdvector<AstNodePtr> &,
			      bool, func_instance *);
    //virtual bool emitPIC(codeGen& /*gen*/, Address, Address )=0;

    virtual void emitGetRetVal(Register, bool, codeGen &) { assert(0); }
    virtual void emitGetRetAddr(Register, codeGen &) { assert(0); }
    virtual void emitGetParam(Register, Register, instPoint::Type, opCode, bool, codeGen &) { assert(0); }
    virtual void emitASload(int, int, int, long, Register, int, codeGen &) { assert(0); }
    virtual void emitCSload(int, int, int, long, Register, codeGen &) { assert(0); }
    virtual void emitPushFlags(codeGen &) { assert(0); }
    virtual void emitRestoreFlags(codeGen &, unsigned) { assert(0); }
    // Built-in offset...
    virtual void emitRestoreFlagsFromStackSlot(codeGen &) { assert(0); }
    virtual bool emitBTSaves(baseTramp*, codeGen &) { assert(0); return true;}
    virtual bool emitBTRestores(baseTramp*, codeGen &) { assert(0); return true; }
    virtual void emitStoreImm(Address, int, codeGen &, bool) { assert(0); }
    virtual void emitAddSignedImm(Address, int, codeGen &, bool) { assert(0); }
    virtual int Register_DWARFtoMachineEnc(int) { assert(0); return 0;}
    virtual bool emitPush(codeGen &, Register) { assert(0); return true;}
    virtual bool emitPop(codeGen &, Register) { assert(0); return true;}
    virtual bool emitAdjustStackPointer(int, codeGen &) { assert(0); return true;}
    
    virtual bool clobberAllFuncCall(registerSpace *rs,func_instance *callee);

 protected:
    virtual bool emitCallInstruction(codeGen& /*gen*/, func_instance* /*callee*/, bool /*setTOC*/, Address) = 0;
    virtual Register emitCallReplacement(opCode ocode, codeGen &gen, bool noCost, func_instance *callee) = 0;
};

class EmitterPOWERDyn : public EmitterPOWER {
 public:
    virtual ~EmitterPOWERDyn() {};

 protected:
    virtual bool emitCallInstruction(codeGen& /*gen*/, func_instance* /*callee*/, bool /*setTOC*/, Address);
    virtual Register emitCallReplacement(opCode ocode, codeGen &gen,
                                 bool noCost, func_instance *callee);

};

class EmitterPOWERStat : public EmitterPOWER {
 public:
    virtual ~EmitterPOWERStat() {};

 protected:
    virtual Register emitCallReplacement(opCode ocode, codeGen &gen,
                                 bool noCost, func_instance *callee);
};

class EmitterPOWER32Dyn : public EmitterPOWERDyn {
 public:
    virtual ~EmitterPOWER32Dyn() {}
};

class EmitterPOWER32Stat : public EmitterPOWERStat {
 public:
    virtual ~EmitterPOWER32Stat() {}

    virtual bool emitPLTCall(func_instance *dest, codeGen &gen);
    virtual bool emitPLTJump(func_instance *dest, codeGen &gen);

    bool emitPLTCommon(func_instance *dest, bool call, codeGen &gen);


 protected:
    virtual bool emitCallInstruction(codeGen& /*gen*/, func_instance* /*callee*/, bool /*setTOC*/, Address);
};

class EmitterPOWER64Dyn : public EmitterPOWERDyn {
 public:
    virtual ~EmitterPOWER64Dyn() {}

    virtual bool emitTOCCommon(block_instance *, bool call, codeGen &);

    virtual bool emitTOCJump(block_instance *, codeGen &);
    virtual bool emitTOCCall(block_instance *, codeGen &);

};

class EmitterPOWER64Stat : public EmitterPOWERStat {
 public:
    virtual ~EmitterPOWER64Stat() {}

    virtual bool emitPLTCall(func_instance *dest, codeGen &gen);
    virtual bool emitPLTJump(func_instance *dest, codeGen &gen);

    bool emitPLTCommon(func_instance *dest, bool call, codeGen &gen);


 protected:
    virtual bool emitCallInstruction(codeGen& /*gen*/, func_instance* /*callee*/, bool /*setTOC*/, Address);
    //virtual bool emitPIC(codeGen& /*gen*/, Address, Address ) ;
};

#endif
