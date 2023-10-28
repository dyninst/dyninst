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

#include <stack>
#include <vector>

#include "Architecture.h"
#include "registers/MachRegister.h"
#include "dyntypes.h"
#include "elfutils/libdw.h"
#include "util.h"

namespace Dyninst {

class VariableLocation;
class ProcessReader;

namespace DwarfDyninst {

class DYNDWARF_EXPORT DwarfResult {
    // An interface for building representations of Dwarf expressions.
    // In concrete mode, we have access to process state and
    // can calculate a value. In symbolic mode we lack this information
    // and instead produce a representation. 

public:

    typedef enum {
        Add,
        Sub,
        Mul,
        Div,
        Mod,
        Deref,
        Pick,
        Drop,
        And,
        Or,
        Not,
        Xor,
        Abs,
        GE,
        LE,
        GT,
        LT,
        Eq,
        Neq,
        Shl,
        Shr,
        ShrArith
    } Operator;

    DwarfResult(Architecture a) : arch(a), error(false) {}

    DwarfResult(const DwarfResult &) = default;
    virtual ~DwarfResult() = default;

    virtual void pushReg(Dyninst::MachRegister reg) = 0;
    virtual void readReg(Dyninst::MachRegister reg) = 0;
    virtual void pushUnsignedVal(Dyninst::MachRegisterVal constant) = 0;
    virtual void pushSignedVal(Dyninst::MachRegisterVal constant) = 0;
    virtual void pushOp(Operator op) = 0;
    virtual void pushOp(Operator op, long long ref) = 0;

    // The frame base is the logical top of the stack as reported
    // in the function's debug info
    virtual void pushFrameBase() = 0;

    // And the CFA is the top of the stack as reported in call frame
    // information. 
    virtual void pushCFA() = 0;

    bool err() const { return error; }

    // The conditional branch needs an immediate eval mechanism
    virtual bool eval(MachRegisterVal &val) = 0;

protected:

    // Illegal
    Architecture arch;
    bool error;

};

class DYNDWARF_EXPORT SymbolicDwarfResult : public DwarfResult {

public:
    SymbolicDwarfResult(VariableLocation &v, Architecture a) :
        DwarfResult(a), var(v) {}

    virtual void pushReg(Dyninst::MachRegister reg);
    virtual void readReg(Dyninst::MachRegister reg);
    virtual void pushUnsignedVal(Dyninst::MachRegisterVal constant);
    virtual void pushSignedVal(Dyninst::MachRegisterVal constant);
    virtual void pushOp(Operator op);
    virtual void pushOp(Operator op, long long ref);

    // DWARF logical "frame base", which may be the result of an expression
    // in itself. TODO: figure out what info we need to carry around so we
    // can compute it...
    virtual void pushFrameBase();
    virtual void pushCFA();

    VariableLocation &val();

    virtual bool eval(MachRegisterVal &) { return false; }

private:
    std::stack<MachRegisterVal> operands;

    VariableLocation &var;
};

class DYNDWARF_EXPORT ConcreteDwarfResult : public DwarfResult {

public:
    ConcreteDwarfResult(ProcessReader *r, Architecture a, 
            Address p, Dwarf * d, Elf * e) :
        DwarfResult(a), reader(r), 
        pc(p), dbg(d), dbg_eh_frame(e) {}
    ConcreteDwarfResult() : DwarfResult(Arch_none) {}
    virtual ~ConcreteDwarfResult() {}

    virtual void pushReg(Dyninst::MachRegister reg);
    virtual void readReg(Dyninst::MachRegister reg);
    virtual void pushUnsignedVal(Dyninst::MachRegisterVal constant);
    virtual void pushSignedVal(Dyninst::MachRegisterVal constant);
    virtual void pushOp(Operator op);
    virtual void pushOp(Operator op, long long ref);

    // DWARF logical "frame base", which may be the result of an expression
    // in itself. TODO: figure out what info we need to carry around so we
    // can compute it...
    virtual void pushFrameBase();
    virtual void pushCFA();

    MachRegisterVal val();

    bool eval(MachRegisterVal &v);

private:
    ProcessReader *reader{};

    // For getting access to other expressions
    Address pc{};
    Dwarf * dbg{};
    Elf * dbg_eh_frame{};

    // Dwarf lets you access within the "stack", so we model 
    // it as a vector.
    std::vector<Dyninst::MachRegisterVal> operands;

    MachRegisterVal peek(int index);
    void pop(int num);
    void popRange(int start, int end);
    void push(MachRegisterVal v);

};

}

}
