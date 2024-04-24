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

#if !defined(DWARF_SW_H_)
#define DWARF_SW_H_

#include <map>
#include <utility>
#include <stack>
#include <vector>
#include "dyntypes.h"
#include "Architecture.h"
#include "registers/MachRegister.h"
#include "ProcReader.h"
#include "elfutils/libdw.h"
#include "util.h"
#include <boost/thread/once.hpp>
#include "concurrent.h"

namespace Dyninst {

class VariableLocation;

namespace DwarfDyninst {

class DwarfResult;

typedef enum {
    FE_Bad_Frame_Data = 15,
    FE_No_Frame_Entry,
    FE_Frame_Read_Error,
    FE_Frame_Eval_Error,
    FE_No_Error
} FrameErrors_t;

class DYNDWARF_EXPORT DwarfFrameParser {
public:

    typedef boost::shared_ptr<DwarfFrameParser> Ptr;

    static Ptr create(Dwarf * dbg, Elf * eh_frame, Architecture arch);

    DwarfFrameParser(Dwarf * dbg_, Elf * eh_frame, Architecture arch_);

    ~DwarfFrameParser();

    bool hasFrameDebugInfo();

    bool getRegRepAtFrame(Address pc,
            MachRegister reg,
            VariableLocation &loc,
            FrameErrors_t &err_result);

    bool getRegValueAtFrame(Address pc, 
            MachRegister reg, 
            MachRegisterVal &reg_result,
            ProcessReader *reader,
            FrameErrors_t &err_result);

    bool getRegAtFrame(Address pc,
            MachRegister reg,
            DwarfResult &cons,
            FrameErrors_t &err_result);

    bool getRegsForFunction(
            std::pair<Address, Address> range,
            MachRegister reg,
            std::vector<VariableLocation> &locs,
            FrameErrors_t &err_result);


private:

    void setupCFIData();

    struct frameParser_key
    {
        Dwarf * dbg;
        Elf * eh_frame;
        Architecture arch;
        frameParser_key(Dwarf * d, Elf * eh_frame_, Architecture a) : dbg(d), eh_frame(eh_frame_), arch(a)
        {
        }

        bool operator< (const frameParser_key& rhs) const
        {
            return (dbg < rhs.dbg)
                || (dbg == rhs.dbg && eh_frame < rhs.eh_frame)
                || (dbg == rhs.dbg && eh_frame == rhs.eh_frame && arch < rhs.arch);
        }

    };

    typedef enum {
        dwarf_status_uninitialized,
        dwarf_status_error,
        dwarf_status_ok
    } dwarf_status_t;
    
    static std::map<frameParser_key, Ptr> frameParsers;

    Dwarf 	* dbg;
    Elf 	* dbg_eh_frame;
    
    Architecture arch;

    boost::once_flag fde_dwarf_once;
    dwarf_status_t fde_dwarf_status;

    dyn_mutex cfi_lock;
    std::vector<Dwarf_CFI *> cfi_data;

};

}

}
#endif

