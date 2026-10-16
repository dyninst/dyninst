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
#include "common/src/vgannotations.h"
#include <typeinfo>
#include <string.h>
#include "dwarfFrameParser.h"
#include "dwarfExprParser.h"
#include "dwarfResult.h"
#include "VariableLocation.h"
#include "elfutils/libdw.h"
#include <stdio.h>
#include <iostream>
#include "debug_common.h" // dwarf_printf
#include <libelf.h>
#include "registers/abstract_regs.h"
#include "registers/aarch64_regs.h"
#include "dwarf/src/registers/convert.h"

#include <mutex>
#include <map>

using namespace Dyninst;
using namespace DwarfDyninst;
using namespace std;

namespace {
  struct frameParser_key {
    Dwarf *dbg;
    Elf *eh_frame;
    Architecture arch;

    bool operator<(const frameParser_key &rhs) const {
      return (dbg < rhs.dbg) ||
             (dbg == rhs.dbg && eh_frame < rhs.eh_frame) ||
             (dbg == rhs.dbg && eh_frame == rhs.eh_frame && arch < rhs.arch);
    }
  };

  std::mutex frameParsers_mutex;
  std::map<frameParser_key, DwarfFrameParser::Ptr> frameParsers;

  // This only for aarch64
  MachRegister convert_abstract(MachRegister abstract) {
    if(abstract == ReturnAddr)
      return aarch64::x30;
    if(abstract == FrameBase)
      return aarch64::x29;
    if(abstract == StackTop)
      return aarch64::sp;
    if(abstract == CFA)
      dwarf_printf("No aarch64 register for abstract CFA");

    return Dyninst::InvalidReg;
  }
}

DwarfFrameParser::Ptr DwarfFrameParser::create(Dwarf * dbg, Elf * eh_frame, Architecture arch)
{
    if(!dbg && !eh_frame) return NULL;

    frameParser_key k{dbg, eh_frame, arch};

    std::unique_lock<std::mutex> _l(frameParsers_mutex);

    auto iter = frameParsers.find(k);
    if (iter == frameParsers.end()) {
        Ptr newParser = Ptr(new DwarfFrameParser(dbg, eh_frame, arch));
        frameParsers.insert(make_pair(k, newParser));
        return newParser;
    }
    else {
        return iter->second;
    }
}


DwarfFrameParser::DwarfFrameParser(Dwarf * dbg_, Elf * eh_frame, Architecture arch_) :
    dbg(dbg_),
    dbg_eh_frame(eh_frame),
    arch(arch_),
    fde_dwarf_once(BOOST_ONCE_INIT),
    fde_dwarf_status(dwarf_status_uninitialized)
{
}

DwarfFrameParser::~DwarfFrameParser()
{
    if (fde_dwarf_status != dwarf_status_ok)
        return;
    for (unsigned i=0; i<cfi_data.size(); i++)
    {
        // FIXME only do this for dwarf_getcfi_elf!
        // // dwarf_cfi_end(cfi_data[i]);
        //
        // previous code
        //dwarf_fde_cie_list_dealloc(dbg,
        //        cfi_data[i].cfi_data, cfi_data[i].cie_count,
        //        cfi_data[i].cfi_data, cfi_data[i].fde_count);
    }
}

bool DwarfFrameParser::hasFrameDebugInfo()
{
    setupCFIData();
    return fde_dwarf_status == dwarf_status_ok;
}

bool DwarfFrameParser::getRegValueAtFrame(
        Address pc,
        Dyninst::MachRegister reg,
        Dyninst::MachRegisterVal &reg_result,
        ProcessReader *reader,
        FrameErrors_t &err_result)
{
    ConcreteDwarfResult cons(reader, arch, pc, dbg, dbg_eh_frame);

    dwarf_printf("Getting concrete value for %s at 0x%lx\n",
            reg.name().c_str(), pc);
    if (!getRegAtFrame(pc, reg, cons, err_result)) {
        dwarf_printf("\t Returning error from getRegValueAtFrame: %d\n", err_result);
        return false;
    }
    if (cons.err()) {
        dwarf_printf("\t Computed dwarf result to an error\n");
        err_result = FE_Frame_Eval_Error;
        return false;
    }

    reg_result = cons.val();
    dwarf_printf("Returning result 0x%lx for reg %s at 0x%lx\n",
            reg_result, reg.name().c_str(), pc);
    return true;
}

bool DwarfFrameParser::getRegRepAtFrame(
        Address pc,
        Dyninst::MachRegister reg,
        VariableLocation &loc,
        FrameErrors_t &err_result)
{
    SymbolicDwarfResult cons(loc, arch);

    dwarf_printf("Getting symbolic value for %s at 0x%lx\n",
            reg.name().c_str(), pc);
    if (!getRegAtFrame(pc, reg, cons, err_result)) {
        dwarf_printf("\t Returning error from getRegRepAtFrame\n");
        assert(err_result != FE_No_Error);
        return false;
    }

    if (cons.err()) {
        dwarf_printf("\t Computed dwarf result to an error\n");
        err_result = FE_Frame_Eval_Error;
        return false;
    }

    loc = cons.val();

    dwarf_printf("Returning symbolic result for reg %s at 0x%lx\n",
            reg.name().c_str(), pc);

    return true;
}

bool DwarfFrameParser::getRegsForFunction(
        std::pair<Address, Address> range,
        Dyninst::MachRegister reg,
        std::vector<VariableLocation> &locs,
        FrameErrors_t &err_result)
{
    locs.clear();
    dwarf_printf("Entry to getRegsForFunction at 0x%lx, range end 0x%lx, reg %s\n", range.first, range.second, reg.name().c_str());
    err_result = FE_No_Error;

    /**
     * Initialize the FDE and CIE data.  This is only really done once,
     * after which setupCFIData will immediately return.
     **/
    setupCFIData();
    if (!cfi_data.size()) {
        dwarf_printf("\t No FDE data, ret false\n");
        err_result = FE_Bad_Frame_Data;
        return false;
    }

    boost::unique_lock<dyn_mutex> l(cfi_lock);
    for(size_t i=0; i<cfi_data.size(); i++)
    {
        auto next_pc = range.first;
        while(next_pc < range.second)
        {
            Dwarf_Frame * frame = NULL;
            int result = dwarf_cfi_addrframe(cfi_data[i], next_pc, &frame);
            // No FDE covers next_pc: a padding gap between functions. When the
            // caller's range legitimately spans more than one function (e.g.
            // sampling a hot/cold-split function whose partitions bracket other
            // functions), such gaps must be skipped, not treated as the end of
            // the range -- otherwise rows past the first gap are lost. Advancing
            // one byte is fine: inter-function padding is tiny.
            if(result==-1) { next_pc++; continue; }

            Dwarf_Addr start_pc, end_pc;
            dwarf_frame_info(frame, &start_pc, &end_pc, NULL);
            Dwarf_Addr row_next = (end_pc > next_pc) ? end_pc : next_pc + 1;

            Dwarf_Op * ops;
            size_t nops;
            result = dwarf_frame_cfa(frame, &ops, &nops);
            if (result == 0) {
                VariableLocation loc2;
                DwarfDyninst::SymbolicDwarfResult cons(loc2, arch);
                // A row we cannot decode (e.g. an expression CFA in a foreign
                // function within a spanning range) is skipped, not fatal.
                if (DwarfDyninst::decodeDwarfExpression(ops, nops, NULL, cons, arch)) {
                    loc2.lowPC = next_pc;
                    loc2.hiPC = end_pc;
                    locs.push_back(cons.val());
                }
            }
            free(frame);   // dwarf_cfi_addrframe allocates; free every row (the
                           // gap-skipping walk visits many) as getRegRulesForFunction does
            next_pc = row_next;
        }
    }

    return !locs.empty();
}

// convenience wrapper -- map the MachRegister to its DWARF number.
// Callers that need a CFI column with no Dyninst MachRegister (notably ppc64's
// link register, DWARF 65, which Dyninst's register map assigns to 108) must
// use getRegRulesForDwarf directly.
bool DwarfFrameParser::getRegRulesForFunction(
        std::pair<Address, Address> range,
        Dyninst::MachRegister reg,
        std::vector<FrameRegRule> &rules,
        FrameErrors_t &err_result)
{
    return getRegRulesForDwarf(range, DwarfDyninst::register_to_dwarf(reg), rules, err_result);
}

// classify dwarf_frame_register's location description for the DWARF
// column `dwarf_reg` across `range`. Same row-walk as getRegsForFunction, but
// keyed on a raw DWARF register number so any column (e.g. a return-address
// register that has no MachRegister) is reachable.
bool DwarfFrameParser::getRegRulesForDwarf(
        std::pair<Address, Address> range,
        int dwarf_reg,
        std::vector<FrameRegRule> &rules,
        FrameErrors_t &err_result)
{
    rules.clear();
    dwarf_printf("Entry to getRegRulesForDwarf at 0x%lx, range end 0x%lx, dwarf_reg %d\n",
                 range.first, range.second, dwarf_reg);
    err_result = FE_No_Error;

    setupCFIData();
    if (!cfi_data.size()) {
        err_result = FE_Bad_Frame_Data;
        return false;
    }

    boost::unique_lock<dyn_mutex> l(cfi_lock);
    for (size_t i = 0; i < cfi_data.size(); i++)
    {
        auto next_pc = range.first;
        while (next_pc < range.second)
        {
            Dwarf_Frame *frame = NULL;
            // Skip padding gaps between functions (no FDE) rather than ending
            // the walk, so a range spanning a hot/cold-split function's
            // partitions still yields the rows past the gap.
            if (dwarf_cfi_addrframe(cfi_data[i], next_pc, &frame) == -1) { next_pc++; continue; }

            Dwarf_Addr start_pc, end_pc;
            dwarf_frame_info(frame, &start_pc, &end_pc, NULL);
            Dwarf_Addr row_next = (end_pc > next_pc) ? end_pc : next_pc + 1;

            Dwarf_Op ops_mem[3];
            Dwarf_Op *ops = NULL;
            size_t nops = 0;
            int result = dwarf_frame_register(frame, dwarf_reg, ops_mem, &ops, &nops);
            if (result != 0) { free(frame); next_pc = row_next; continue; }

            FrameRegRule r;
            r.lowPC = next_pc;
            r.hiPC = end_pc;
            r.offset = 0;
            r.regnum = 0;
            if (nops == 0) {
                // libdw: *ops == ops_mem means "undefined"; *ops == NULL means
                // "same_value" (this frame does not modify the register).
                r.kind = (ops == ops_mem) ? FrameRegRule::Undefined
                                          : FrameRegRule::SameValue;
            }
            else if (ops[0].atom == DW_OP_call_frame_cfa &&
                     ops[nops-1].atom != DW_OP_stack_value &&
                     (nops == 1 ||
                      (nops == 2 && ops[1].atom == DW_OP_plus_uconst))) {
                r.kind = FrameRegRule::AtCFAOffset;
                r.offset = (nops == 2) ? (long)(int64_t)ops[1].number : 0;
            }
            else if (nops == 1 && ops[0].atom >= DW_OP_reg0 &&
                     ops[0].atom <= DW_OP_reg31) {
                r.kind = FrameRegRule::InRegister;
                r.regnum = (unsigned)(ops[0].atom - DW_OP_reg0);
            }
            else if (nops == 1 && ops[0].atom == DW_OP_regx) {
                r.kind = FrameRegRule::InRegister;
                r.regnum = (unsigned)ops[0].number;
            }
            else {
                // DWARF expression / val_offset rules are not representable in
                // the simple re-emission; report Undefined so callers stay safe.
                r.kind = FrameRegRule::Undefined;
            }
            rules.push_back(r);
            free(frame);
            next_pc = row_next;
        }
    }

    return !rules.empty();
}

bool DwarfFrameParser::getRegAtFrame(
        Address pc,
        Dyninst::MachRegister reg,
        DwarfResult &cons,
        FrameErrors_t &err_result)
{

    err_result = FE_No_Error;

    dwarf_printf("getRegAtFrame for 0x%lx, %s\n", pc, reg.name().c_str());

    /**
     * Initialize the FDE and CIE data.  This is only really done once,
     * after which setupCFIData will immediately return.
     **/
    setupCFIData();
    if (!cfi_data.size()) {
        dwarf_printf("\t No FDE data, ret false\n");
        err_result = FE_Bad_Frame_Data;
        return false;
    }

    int not_found = 0; // if not found FDE covering PC, increment

    // this for goes for each cfi_data to look for the frame at pc
    // the first one it finds, use it and break out of the for
    for(size_t i=0; i<cfi_data.size(); i++)
    {
        Dwarf_Frame * frame = NULL;
        int result = dwarf_cfi_addrframe(cfi_data[i], pc, &frame);
        if (result != 0) // 0 is success, not found FDE covering PC is returned -1
        {
            not_found++; //there can be 2 not found since cfi_data can have side 2
            continue;
        }

        dwarf_printf("Found frame info in cfi_data[%zu], cfi_data.size=%zu \n", i, cfi_data.size());

        // FDE found so make not_found=0 (FALSE), in case it was true because
        // a previous loop
        not_found=0;

        // user can request CFA (same as FrameBase), ReturnAddr, or any register
        // use dwarf_frame_info to get the register number for ReturnAddr
        Dwarf_Addr start_pc, end_pc;
        int dwarf_reg = dwarf_frame_info(frame, &start_pc, &end_pc, NULL);
        if (reg != Dyninst::ReturnAddr &&
                reg != Dyninst::FrameBase &&
                reg != Dyninst::CFA )
            dwarf_reg = DwarfDyninst::register_to_dwarf(reg);

        // now get the rule for the register reg
        // if its CFA (same as FrameBase) use dwarf_frame_cfa
        // else use dwarf_frame_register
        Dwarf_Op * ops;
        size_t nops;
        if (reg == Dyninst::FrameBase || reg == Dyninst::CFA)
        {
            dwarf_printf("\t reg is FrameBase(CFA)\n");

            result = dwarf_frame_cfa(frame, &ops, &nops);
            if (result != 0 || nops == 0)
            {
                err_result = FE_Frame_Read_Error;
                return false;
            }
            dwarf_printf("\t\t nops=%zu\n",nops);

            if (!DwarfDyninst::decodeDwarfExpression(ops, nops, NULL, cons, arch)) {
                err_result = FE_Frame_Eval_Error;
                dwarf_printf("\t Failed to decode dwarf expr, ret false\n");
                return false;
            }
            return true;
        }
        else // get location description for dwarf_reg (which can be RA or reg(n))
        {
            dwarf_printf("\t parameter reg is %s\n", reg.name().c_str());
            dwarf_printf("\t dwarf_reg (or column in CFI table) is %d\n", dwarf_reg);

            Dwarf_Op ops_mem[3];
            result = dwarf_frame_register (frame, dwarf_reg, ops_mem, &ops, &nops);

            if (result != 0)
            {
                err_result = FE_Frame_Read_Error;
                return false;
            }

            // case of undefined
            if(nops == 0 && ops == ops_mem)
            {
                dwarf_printf("\t case of undefined rule, treats as same_value\n");
                if(this->arch == Arch_aarch64) {
                  reg = convert_abstract(reg);
                  dwarf_printf("\t aarch64 converted register reg=%s\n", reg.name().c_str());
                }

                // Dyninst treats as same_value ???
                if (reg != Dyninst::ReturnAddr) {
                    cons.readReg(reg);
                    return true; // true because undefined is a valid output
                } else {
                    return false;
                }
            }

            // case of same_value
            if(nops == 0 && ops == NULL)
            {
                dwarf_printf("\t case of same_value rule\n");
                if(this->arch == Arch_aarch64) {
                  reg = convert_abstract(reg);
                  dwarf_printf("\t aarch64 converted register reg=%s\n", reg.name().c_str());
                }

                if (reg != Dyninst::ReturnAddr) {
                    cons.readReg(reg);
                    return true;
                } else {
                    return false;
                }
            }

            // translate dwarf reg to machine reg
            //Dyninst::MachRegister Register = MachRegister::DwarfEncToReg(dwarf_reg, arch);
            //cons.readReg(Register);

            ConcreteDwarfResult aux_cdr;
            // if is concrete, add Deref as last operation if there isn't DW_OP_stack_value
            if(typeid(cons)==typeid(aux_cdr))
            {
                // if last operation is not DW_OP_stack_value
                if(ops[nops-1].atom != DW_OP_stack_value)
                {
                    // add DW_OP_deref
                    Dwarf_Op * newOps = new Dwarf_Op[nops+1];
                    memcpy(newOps, ops, nops * sizeof(Dwarf_Op));
                    ops = newOps;
                    ops[nops] = {DW_OP_deref, 0, 0, 0};
                    nops++;
                }
            }

            // decode location description, rule dependes on some register
            if (!DwarfDyninst::decodeDwarfExpression(ops, nops, NULL, cons, arch)) {
                err_result = FE_Frame_Eval_Error;
                dwarf_printf("\t Failed to decode dwarf expr, ret false\n");
                return false;
            }

            // Check if cons is Concrete, because there's no need to
            // search CFA again
            if(typeid(cons)==typeid(aux_cdr)) return true;

            // From here cons is SymbolicDwarfResult

            // check case of *ops = {DW_OP_call_frame_cfa, DW_OP_stack_value}
            // this case would produce wrong frameoffset. The correct value
            // of reg should be getting the CFA at the beginning of the FDE range
            // and not at pc. So if this is the case, ignore the subsequent call
            // to getRegAtFrame(pc, CFA).
            if(nops==2)
                if(ops[0].atom==DW_OP_call_frame_cfa &&
                        ops[1].atom== DW_OP_stack_value)
                {
                    auto sdr = dynamic_cast<SymbolicDwarfResult &>(cons);
                    VariableLocation& loc = sdr.val();
                    if(loc.mr_reg == Dyninst::CFA) loc.mr_reg = reg;
                    return true;
                }

            // CFA (or FrameBase) is always associated ???
            // usar outro cons
            if (!getRegAtFrame(pc, Dyninst::CFA, cons, err_result)) {
                assert(err_result != FE_No_Error);
                return false;
            }

            return true;
        }

        break; // if found in the first cfi_data, no need to check in the second
    }

    if(not_found){
        err_result = FE_No_Frame_Entry;
        return false;
    }

    return true;

}

void DwarfFrameParser::setupCFIData()
{
    boost::call_once(fde_dwarf_once, [&]{
        if (!dbg && !dbg_eh_frame) {
            fde_dwarf_status = dwarf_status_error;
            return;
        }

#if defined(dwarf_has_setframe)
        dwarf_set_frame_cfa_value(dbg, DW_FRAME_CFA_COL3);
#endif

        Dwarf_CFI * cfi = nullptr;

        // Try to get dwarf data from .debug_frame
        cfi = dwarf_getcfi(dbg);
        if (dbg && cfi)
        {
            cfi_data.push_back(cfi);
        }

        // Try to get dwarf data from .eh_frame
        cfi = nullptr;
        cfi = dwarf_getcfi_elf(dbg_eh_frame);
        if (dbg_eh_frame && cfi)
        {
            cfi_data.push_back(cfi);
        }

        // Verify if it got any dwarf data
        if (!cfi_data.size()) {
            fde_dwarf_status = dwarf_status_error;
        }
        else{
            fde_dwarf_status = dwarf_status_ok;
        }

        ANNOTATE_HAPPENS_BEFORE(&fde_dwarf_once);
    });
    ANNOTATE_HAPPENS_AFTER(&fde_dwarf_once);
}

