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

/*
 * emit-x86.h - x86 & AMD64 code generators
 * $Id: emit-power.h,v 1.6 2008/03/25 19:24:26 bernat Exp $
 */

#ifndef _EMITTER_POWER_H
#define _EMITTER_POWER_H

#include <assert.h>
#include "ASTs/codeGenAST.h"
#include <vector>
#include "codegen/RegControl.h"
#include "common/src/headers.h"
#include "patching/instPoint.h"
#include "trampolines/baseTramp.h"
#include "dyninstAPI/src/emitter.h"
#include "function_cache.h"
#include "codegen/emitters/PowerPC/EmitterPowerPC.h"

class codeGen;
class registerSpace;
class baseTramp;

class EmitterPOWER32Dyn : public Dyninst::DyninstAPI::EmitterPowerPC
{
  public:
    virtual ~EmitterPOWER32Dyn() {}
};

class EmitterPOWER32Stat : public Dyninst::DyninstAPI::EmitterPowerPC
{
 public:
    virtual ~EmitterPOWER32Stat() {}

    virtual bool emitPLTCall(func_instance *dest, codeGen &gen);
    virtual bool emitPLTJump(func_instance *dest, codeGen &gen);

    virtual bool emitTOCCall(block_instance *dest, codeGen &gen);
    virtual bool emitTOCJump(block_instance *dest, codeGen &gen);
 protected:
    virtual bool emitCallInstruction(codeGen &, func_instance *, bool,
                                     Address);
    virtual Register emitCallReplacement(opCode, codeGen &,
                                         func_instance *) {
        assert(0 && "emitCallReplacement not implemented for binary rewriter");
    }

  private:
    bool emitPLTCommon(func_instance *dest, bool call, codeGen &gen);
    bool emitTOCCommon(block_instance *dest, bool call, codeGen &gen);
};

class EmitterPOWER64Dyn : public Dyninst::DyninstAPI::EmitterPowerPC
{
  public:
  virtual bool emitTOCCall(block_instance *dest, codeGen &gen) { return emitTOCCommon(dest, true, gen); }
  virtual bool emitTOCJump(block_instance *dest, codeGen &gen) { return emitTOCCommon(dest, false, gen); }
    virtual ~EmitterPOWER64Dyn() {}
 private:
    bool emitTOCCommon(block_instance *dest, bool call, codeGen &gen);

};

class EmitterPOWER64Stat : public Dyninst::DyninstAPI::EmitterPowerPC {
  public:
    virtual ~EmitterPOWER64Stat() {}

    virtual bool emitPLTCall(func_instance *dest, codeGen &gen);
    virtual bool emitPLTJump(func_instance *dest, codeGen &gen);

    virtual bool emitTOCCall(block_instance *dest, codeGen &gen);
    virtual bool emitTOCJump(block_instance *dest, codeGen &gen);
 protected:
    virtual bool emitCallInstruction(codeGen &, func_instance *, bool,
                                     Address);
    virtual Register emitCallReplacement(opCode, codeGen &,
                                         func_instance *) {
        assert(0 && "emitCallReplacement not implemented for binary rewriter");
    }

  private:
    bool emitPLTCommon(func_instance *dest, bool call, codeGen &gen);
    bool emitTOCCommon(block_instance *dest, bool call, codeGen &gen);
};

#endif
