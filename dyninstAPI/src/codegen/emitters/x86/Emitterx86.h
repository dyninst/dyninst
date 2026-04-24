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

#ifndef DYNINST_DYNINSTAPI_CODEGEN_EMITTERS_EMITTERX86_H
#define DYNINST_DYNINSTAPI_CODEGEN_EMITTERS_EMITTERX86_H

/*
 *  Common code for emitting instructions on i386 and x86_64
 */

#include "codegen.h"
#include "dyn_register.h"
#include "dyntypes.h"
#include "emitter.h"

namespace Dyninst { namespace DyninstAPI {

  class Emitterx86 : public Emitter {
  public:
    Emitterx86() = default;

    virtual ~Emitterx86() = default;

    virtual bool emitCallInstruction(codeGen &, func_instance *, Register) = 0;

    void emitAddrSpecLoad(const BPatch_addrSpec_NP *as, Dyninst::Register dest, int stackShift, codeGen &gen) override final;

    void emitCountSpecLoad(const BPatch_countSpec_NP *as, Dyninst::Register dest, codeGen &gen) override;

    virtual void emitLEA(Register base, Register index, unsigned int scale, int disp, Register dest,
                         codeGen &gen) = 0;

    virtual bool emitLoadRelativeSegReg(Register dest, Address offset, Register base, int size,
                                        codeGen &gen) = 0;

    virtual bool emitXorRegImm(Register dest, int imm, codeGen &gen) = 0;

    virtual bool emitXorRegReg(Register dest, Register base, codeGen &gen) = 0;

    virtual bool emitXorRegRM(Register dest, Register base, int disp, codeGen &gen) = 0;

    virtual bool emitXorRegSegReg(Register dest, Register base, int disp, codeGen &gen) = 0;

    Address getInterModuleFuncAddr(func_instance *func, codeGen &gen) override;

    Address getInterModuleVarAddr(const image_variable *var, codeGen &gen) override;
  };

  static constexpr auto IA32_EMULATE = 1000;
  static constexpr auto IA32_ESCAS = 1000;
  static constexpr auto IA32_NESCAS = 1001;
  static constexpr auto IA32_ECMPS = 1002;
  static constexpr auto IA32_NECMPS = 1003;
  static constexpr auto IA32AMDprefetch = 100;
  static constexpr auto IA32AMDprefetchw = 101;

}}

#endif
