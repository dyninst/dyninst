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

#define INSIDE_INSTRUCTION_API

#include "common/src/vgannotations.h"

#include "Operation_impl.h"
#include "common/src/arch-x86.h"
#include "entryIDs.h"
#include "Register.h"
#include <map>
#include <mutex>
#include "concurrent.h"
#include "common/src/singleton_object_pool.h"
#include "registers/x86_regs.h"
#include "registers/x86_64_regs.h"

using namespace NS_x86;
#include "BinaryFunction.h"
#include "Immediate.h"

namespace Dyninst
{
    namespace InstructionAPI
    {
        RegisterAST::Ptr makeRegFromID(MachRegister regID, unsigned int low, unsigned int high)
        {
            return make_shared(singleton_object_pool<RegisterAST>::construct(regID, low, high));
        }
        RegisterAST::Ptr makeRegFromID(MachRegister regID)
        {
            return make_shared(singleton_object_pool<RegisterAST>::construct(regID, 0, regID.size() * 8));
        }

        Operation::Operation(entryID id, std::string m, Architecture arch)
            : operationID(id), archDecodedFrom(arch), prefixID(prefix_none)
        {
            switch(archDecodedFrom)
            {
                case Arch_x86:
                case Arch_ppc32:
                    addrWidth = u32;
                    break;
                default:
                    addrWidth = u64;
                    break;
            }
            segPrefix = 0;
            isVectorInsn = false;
            mnemonic = m;
        }

        static bool getVectorizationInfo(ia32_entry* e)
        {
            for(int i = 0; i < 3; i++)
            {
                switch(e->operands[i].admet)
                {
                    case am_V:
                    case am_W:
                    case am_P:
                    case am_Q:
                    case am_HK:
                    case am_H:
                    case am_X:
                    case am_XH:
                    case am_XU:
                    case am_XV:
                    case am_XW:
                    case am_Y:
                    case am_YH:
                    case am_YU:
                    case am_YV:
                    case am_YW:
                    case am_VK:
                    case am_WK:

                        return true;
                    default:
                        break;
                }
            }
            return false;
        }

        Operation::Operation(ia32_entry* e, ia32_prefixes* p, ia32_locations* l, Architecture arch) :
            archDecodedFrom(arch), prefixID(prefix_none)
        {
            segPrefix = 0;
            isVectorInsn = getVectorizationInfo(e);
            operationID = e->getID(l);
            // Defaults for no size prefix
            switch(archDecodedFrom)
            {
                case Arch_x86:
                case Arch_ppc32:
                    addrWidth = u32;
                    break;
                default:
                    addrWidth = u64;
                    break;
            }
            if(p && p->getCount())
            {
                if (p->getPrefix(0) == PREFIX_REP) prefixID = prefix_rep;
                if (p->getPrefix(0) == PREFIX_REPNZ) prefixID = prefix_repnz;
                segPrefix = p->getPrefix(1);
                if(p->getAddrSzPrefix())
                {
                    addrWidth = u16;
                }
            }
        }

        Operation::Operation(const Operation& o)
        {
            operationID = o.operationID;
            archDecodedFrom = o.archDecodedFrom;
            prefixID = o.prefixID;
            addrWidth = o.addrWidth;
            segPrefix = o.segPrefix;
            isVectorInsn = o.isVectorInsn;
            mnemonic = o.mnemonic;
        }
        const Operation& Operation::operator=(const Operation& o)
        {
            operationID = o.operationID;
            archDecodedFrom = o.archDecodedFrom;
            prefixID = o.prefixID;
            addrWidth = o.addrWidth;
            segPrefix = o.segPrefix;
            isVectorInsn = o.isVectorInsn;
            mnemonic = o.mnemonic;
            return *this;
        }
        Operation::Operation()
        {
            operationID = e_No_Entry;
            archDecodedFrom = Arch_none;
            prefixID = prefix_none;
            addrWidth = u64;
            segPrefix = 0;
            isVectorInsn = false;
        }

        const Operation::registerSet&  Operation::implicitReads()
        {
            SetUpNonOperandData();

            return otherRead;
        }
        const Operation::registerSet&  Operation::implicitWrites()
        {
            SetUpNonOperandData();

            return otherWritten;
        }
        bool Operation::isRead(Expression::Ptr candidate)
        {

            SetUpNonOperandData();

            for(registerSet::const_iterator r = otherRead.begin();
                    r != otherRead.end();
                    ++r)
            {
                if(*candidate == *(*r))
                {
                    return true;
                }
            }
            for(VCSet::const_iterator e = otherEffAddrsRead.begin();
                    e != otherEffAddrsRead.end();
                    ++e)
            {
                if(*candidate == *(*e))
                {
                    return true;
                }
            }
            return false;
        }
        const Operation::VCSet& Operation::getImplicitMemReads()
        {
            SetUpNonOperandData();
            return otherEffAddrsRead;
        }
        const Operation::VCSet& Operation::getImplicitMemWrites()
        {
            SetUpNonOperandData();
            return otherEffAddrsWritten;
        }

        bool Operation::isWritten(Expression::Ptr candidate)
        {

            SetUpNonOperandData();

            for(registerSet::const_iterator r = otherWritten.begin();
                    r != otherWritten.end();
                    ++r)
            {
                if(*candidate == *(*r))
                {
                    return true;
                }
            }
            for(VCSet::const_iterator e = otherEffAddrsWritten.begin();
                    e != otherEffAddrsWritten.end();
                    ++e)
            {
                if(*candidate == *(*e))
                {
                    return true;
                }
            }
            return false;
        }

        std::string Operation::format() const
        {
            if(mnemonic != "")
            {
                return mnemonic;
            }
            dyn_hash_map<prefixEntryID, std::string>::const_iterator foundPrefix = prefixEntryNames_IAPI.find(prefixID);
            dyn_hash_map<entryID, std::string>::const_iterator found = entryNames_IAPI.find(operationID);
            std::string result;
            if(foundPrefix != prefixEntryNames_IAPI.end())
            {
                result += (foundPrefix->second + " ");
            }
            if(found != entryNames_IAPI.end())
            {
                result += found->second;
            }
            else
            {
                result += "[INVALID]";
            }
            return result;
        }

        entryID Operation::getID() const
        {
            return operationID;
        }

        prefixEntryID Operation::getPrefixID() const
        {
            return prefixID;
        }

        struct OperationMaps
        {
            typedef dyn_c_hash_map<entryID, Operation::registerSet > reg_info_t;
            typedef dyn_c_hash_map<entryID, Operation::VCSet > mem_info_t;
            public:
            OperationMaps(Architecture arch)
            {
                thePC.insert(RegisterAST::Ptr(new RegisterAST(MachRegister::getPC(arch))));
                pcAndSP.insert(RegisterAST::Ptr(new RegisterAST(MachRegister::getPC(arch))));
                pcAndSP.insert(RegisterAST::Ptr(new RegisterAST(MachRegister::getStackPointer(arch))));
                stackPointer.insert(RegisterAST::Ptr(new RegisterAST(MachRegister::getStackPointer(arch))));
                stackPointerAsExpr.insert(RegisterAST::Ptr(new RegisterAST(MachRegister::getStackPointer(arch))));
                framePointer.insert(RegisterAST::Ptr(new RegisterAST(MachRegister::getFramePointer(arch))));
                spAndBP.insert(RegisterAST::Ptr(new RegisterAST(MachRegister::getStackPointer(arch))));
                spAndBP.insert(RegisterAST::Ptr(new RegisterAST(MachRegister::getFramePointer(arch))));
                si.insert(RegisterAST::Ptr(new RegisterAST(arch == Arch_x86_64 ? x86_64::rsi : x86::esi)));
                di.insert(RegisterAST::Ptr(new RegisterAST(arch == Arch_x86_64 ? x86_64::rdi : x86::edi)));
                si_and_di.insert(RegisterAST::Ptr(new RegisterAST(arch == Arch_x86_64 ? x86_64::rsi : x86::esi)));
                si_and_di.insert(RegisterAST::Ptr(new RegisterAST(arch == Arch_x86_64 ? x86_64::rdi : x86::edi)));

                nonOperandRegisterReads.insert(make_pair(e_call, pcAndSP));
                nonOperandRegisterReads.insert(make_pair(e_ret_near, stackPointer));
                nonOperandRegisterReads.insert(make_pair(e_ret_far, stackPointer));
                nonOperandRegisterReads.insert(make_pair(e_leave, framePointer));
                nonOperandRegisterReads.insert(make_pair(e_enter, spAndBP));

                nonOperandRegisterWrites.insert(make_pair(e_call, pcAndSP));
                nonOperandRegisterWrites.insert(make_pair(e_ret_near, pcAndSP));
                nonOperandRegisterWrites.insert(make_pair(e_ret_far, pcAndSP));
                nonOperandRegisterWrites.insert(make_pair(e_leave, spAndBP));
                nonOperandRegisterWrites.insert(make_pair(e_enter, spAndBP));

                nonOperandRegisterWrites.insert(make_pair(e_loop, thePC));
                nonOperandRegisterWrites.insert(make_pair(e_loope, thePC));
                nonOperandRegisterWrites.insert(make_pair(e_loopne, thePC));

                nonOperandRegisterWrites.insert(make_pair(e_jb, thePC));
                nonOperandRegisterWrites.insert(make_pair(e_jb_jnaej_j, thePC));
                nonOperandRegisterWrites.insert(make_pair(e_jbe, thePC));
                nonOperandRegisterWrites.insert(make_pair(e_jcxz_jec, thePC));
                nonOperandRegisterWrites.insert(make_pair(e_jl, thePC));
                nonOperandRegisterWrites.insert(make_pair(e_jle, thePC));
                nonOperandRegisterWrites.insert(make_pair(e_jmp, thePC));
                nonOperandRegisterWrites.insert(make_pair(e_jae, thePC));
                nonOperandRegisterWrites.insert(make_pair(e_jnb_jae_j, thePC));
                nonOperandRegisterWrites.insert(make_pair(e_ja, thePC));
                nonOperandRegisterWrites.insert(make_pair(e_jge, thePC));
                nonOperandRegisterWrites.insert(make_pair(e_jg, thePC));
                nonOperandRegisterWrites.insert(make_pair(e_jno, thePC));
                nonOperandRegisterWrites.insert(make_pair(e_jnp, thePC));
                nonOperandRegisterWrites.insert(make_pair(e_jns, thePC));
                nonOperandRegisterWrites.insert(make_pair(e_jne, thePC));
                nonOperandRegisterWrites.insert(make_pair(e_jo, thePC));
                nonOperandRegisterWrites.insert(make_pair(e_jp, thePC));
                nonOperandRegisterWrites.insert(make_pair(e_js, thePC));
                nonOperandRegisterWrites.insert(make_pair(e_je, thePC));

                nonOperandMemoryReads.insert(make_pair(e_pop, stackPointerAsExpr));
                nonOperandMemoryReads.insert(make_pair(e_popal, stackPointerAsExpr));
                nonOperandMemoryReads.insert(make_pair(e_popaw, stackPointerAsExpr));
                nonOperandMemoryWrites.insert(make_pair(e_push, stackPointerAsExpr));
                nonOperandMemoryWrites.insert(make_pair(e_pushal, stackPointerAsExpr));
                nonOperandMemoryWrites.insert(make_pair(e_call, stackPointerAsExpr));
                nonOperandMemoryReads.insert(make_pair(e_ret_near, stackPointerAsExpr));
                nonOperandMemoryReads.insert(make_pair(e_ret_far, stackPointerAsExpr));
                nonOperandMemoryReads.insert(make_pair(e_leave, stackPointerAsExpr));

                nonOperandRegisterWrites.insert(make_pair(e_cmpsb, si_and_di));
                nonOperandRegisterWrites.insert(make_pair(e_cmpsd, si_and_di));
                nonOperandRegisterWrites.insert(make_pair(e_cmpsw, si_and_di));
                nonOperandRegisterWrites.insert(make_pair(e_movsb, si_and_di));
                nonOperandRegisterWrites.insert(make_pair(e_movsd, si_and_di));
                nonOperandRegisterWrites.insert(make_pair(e_movsw, si_and_di));
                nonOperandRegisterWrites.insert(make_pair(e_insb, di));
                nonOperandRegisterWrites.insert(make_pair(e_insd, di));
                nonOperandRegisterWrites.insert(make_pair(e_insw, di));
                nonOperandRegisterWrites.insert(make_pair(e_stosb, di));
                nonOperandRegisterWrites.insert(make_pair(e_stosd, di));
                nonOperandRegisterWrites.insert(make_pair(e_stosw, di));
                nonOperandRegisterWrites.insert(make_pair(e_scasb, di));
                nonOperandRegisterWrites.insert(make_pair(e_scasd, di));
                nonOperandRegisterWrites.insert(make_pair(e_scasw, di));
                nonOperandRegisterWrites.insert(make_pair(e_lodsb, di));
                nonOperandRegisterWrites.insert(make_pair(e_lodsd, di));
                nonOperandRegisterWrites.insert(make_pair(e_lodsw, di));
                nonOperandRegisterWrites.insert(make_pair(e_outsb, di));
                nonOperandRegisterWrites.insert(make_pair(e_outsd, di));
                nonOperandRegisterWrites.insert(make_pair(e_outsw, di));


            }
            Operation::registerSet thePC;
            Operation::registerSet pcAndSP;
            Operation::registerSet stackPointer;
            Operation::VCSet stackPointerAsExpr;
            Operation::registerSet framePointer;
            Operation::registerSet spAndBP;
            Operation::registerSet si;
            Operation::registerSet di;
            Operation::registerSet si_and_di;

            reg_info_t nonOperandRegisterReads;
            reg_info_t nonOperandRegisterWrites;

            mem_info_t nonOperandMemoryReads;
            mem_info_t nonOperandMemoryWrites;
        };
        OperationMaps op_data_32(Arch_x86);
        OperationMaps op_data_64(Arch_x86_64);
        const OperationMaps& op_data(Architecture arch)
        {
            switch(arch)
            {
                case Arch_x86:
                    return op_data_32;
                case Arch_x86_64:
                    return op_data_64;
                default:
                    return op_data_32;
            }
        }
        void Operation::SetUpNonOperandData()
        {
            if (archDecodedFrom != Arch_x86 && archDecodedFrom != Arch_x86_64) return;
            std::call_once(data_initialized, [&]() {
                    if (prefixID == prefix_rep || prefixID == prefix_repnz) 	{
                    otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::df : x86_64::df));
                    otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::ecx : x86_64::rcx));
                    otherWritten.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::ecx : x86_64::rcx));
                    if(prefixID == prefix_repnz)
                    {
                    otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::zf : x86_64::zf));
                    }
                    }
                    switch(segPrefix)
                    {
                    case PREFIX_SEGCS:
                    otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::cs : x86_64::cs));
                    break;
                    case PREFIX_SEGDS:
                    otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::ds : x86_64::ds));
                    break;
                    case PREFIX_SEGES:
                    otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::es : x86_64::es));
                    break;
                    case PREFIX_SEGFS:
                    otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::fs : x86_64::fs));
                    break;
                    case PREFIX_SEGGS:
                    otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::gs : x86_64::gs));
                    break;
                    case PREFIX_SEGSS:
                    otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::ss : x86_64::ss));
                    break;
                    }

                    OperationMaps::reg_info_t::const_accessor a, b;
                    if (op_data(archDecodedFrom).nonOperandRegisterReads.find(a, operationID)) {
                        otherRead.insert(a->second.begin(), a->second.end());
                    }
                    if (op_data(archDecodedFrom).nonOperandRegisterWrites.find(b, operationID)) {
                        otherWritten.insert(b->second.begin(), b->second.end());
                    }
                    OperationMaps::mem_info_t::const_accessor c, d;
                    if (op_data(archDecodedFrom).nonOperandMemoryReads.find(c, operationID)) {
                        otherEffAddrsRead.insert(c->second.begin(), c->second.end());
                    }
                    if (operationID == e_push) {
                        BinaryFunction::funcT::Ptr adder(new BinaryFunction::addResult());
                        // special case for push: we write at the new value of the SP.
                        Result dummy(addrWidth, 0);
                        Expression::Ptr push_addr(new BinaryFunction(
                                    *(op_data(archDecodedFrom).stackPointerAsExpr.begin()),
                                    Immediate::makeImmediate(Result(s8, -(dummy.size()))),
                                    addrWidth,
                                    adder));

                        otherEffAddrsWritten.insert(push_addr);

                    } else {
                        if (op_data(archDecodedFrom).nonOperandMemoryWrites.find(d, operationID)) {
                            otherEffAddrsWritten.insert(d->second.begin(), d->second.end());
                        }
                    }

                    dyn_hash_map<entryID, flagInfo>::const_iterator found = ia32_instruction::getFlagTable().find(operationID);
                    if (found != ia32_instruction::getFlagTable().end()) {
                        for (unsigned i = 0; i < found->second.readFlags.size(); i++) {
                            switch (found->second.readFlags[i]) {
                                case x86::icf:
                                    otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::cf : x86_64::cf));
                                    break;
                                case x86::ipf:
                                    otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::pf : x86_64::pf));
                                    break;
                                case x86::iaf:
                                    otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::af : x86_64::af));
                                    break;
                                case x86::izf:
                                    otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::zf : x86_64::zf));
                                    break;
                                case x86::isf:
                                    otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::sf : x86_64::sf));
                                    break;
                                case x86::itf:
                                    otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::tf : x86_64::tf));
                                    break;
                                case x86::idf:
                                    otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::df : x86_64::df));
                                    break;
                                case x86::iof:
                                    otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::of : x86_64::of));
                                    break;
                                case x86::int_:
                                    otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::nt_ : x86_64::nt_));
                                    break;
                                case x86::iif_:
                                    otherRead.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::if_ : x86_64::if_));
                                    break;
                                default:
                                    assert(0);
                            }
                        }

                        for (unsigned j = 0; j < found->second.writtenFlags.size(); j++) {
                            switch (found->second.writtenFlags[j]) {
                                case x86::icf:
                                    otherWritten.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::cf : x86_64::cf));
                                    break;
                                case x86::ipf:
                                    otherWritten.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::pf : x86_64::pf));
                                    break;
                                case x86::iaf:
                                    otherWritten.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::af : x86_64::af));
                                    break;
                                case x86::izf:
                                    otherWritten.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::zf : x86_64::zf));
                                    break;
                                case x86::isf:
                                    otherWritten.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::sf : x86_64::sf));
                                    break;
                                case x86::itf:
                                    otherWritten.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::tf : x86_64::tf));
                                    break;
                                case x86::idf:
                                    otherWritten.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::df : x86_64::df));
                                    break;
                                case x86::iof:
                                    otherWritten.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::of : x86_64::of));
                                    break;
                                case x86::int_:
                                    otherWritten.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::nt_ : x86_64::nt_));
                                    break;
                                case x86::iif_:
                                    otherWritten.insert(makeRegFromID((archDecodedFrom == Arch_x86) ? x86::if_ : x86_64::if_));
                                    break;
                                default:
                                    fprintf(stderr, "ERROR: unhandled entry %s\n",
                                            found->second.writtenFlags[j].name().c_str());
                                    assert(0);
                            }
                        }
                    }
            });
            return;
        }
    }

}
