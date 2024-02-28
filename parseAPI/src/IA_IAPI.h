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

#if !defined(IA_IAPI_H)
#define IA_IAPI_H

#include <map>
#include <set>
#include <stddef.h>
#include <string>
#include <utility>
#include <vector>
#include <boost/tuple/tuple.hpp>
#include <boost/static_assert.hpp>

#include "InstructionAdapter.h"
#include "InstructionDecoder.h"
#include "Instruction.h"

#include "dyntypes.h"

#include "CFG.h"
using namespace std;

namespace Dyninst {
namespace InsnAdapter {

class IA_IAPI : public InstructionAdapter {
    friend class image_func;
    public:
        InstructionAPI::Instruction  current_instruction() { return curInsnIter->second; }

        IA_IAPI(Dyninst::InstructionAPI::InstructionDecoder dec_,
                Address start_, 
                Dyninst::ParseAPI::CodeObject* o,
                Dyninst::ParseAPI::CodeRegion* r,
                Dyninst::InstructionSource *isrc,
		Dyninst::ParseAPI::Block * curBlk_);
        // We have a iterator, and so can't use the implicit copiers
	IA_IAPI(const IA_IAPI &);
	IA_IAPI &operator=(const IA_IAPI &r);
	~IA_IAPI() { }
	static IA_IAPI* makePlatformIA_IAPI(Dyninst::Architecture arch,
	                                    Dyninst::InstructionAPI::InstructionDecoder dec_,
					    Address start_,
					    Dyninst::ParseAPI::CodeObject* o,
					    Dyninst::ParseAPI::CodeRegion* r,
					    Dyninst::InstructionSource *isrc,
					    Dyninst::ParseAPI::Block * curBlk_);
	virtual IA_IAPI* clone() const = 0;
        virtual void reset(Dyninst::InstructionAPI::InstructionDecoder dec_,
          Address start, ParseAPI::CodeObject *o,
          ParseAPI::CodeRegion *r, InstructionSource *isrc, ParseAPI::Block *);

        virtual const Dyninst::InstructionAPI::Instruction& getInstruction() const;
    
        virtual bool hasCFT() const;
        virtual size_t getSize() const;
        virtual bool isFrameSetupInsn() const;
        virtual bool isAbort() const;
        virtual bool isInvalidInsn() const;
        virtual bool isGarbageInsn() const; //true for insns indicative of bad parse, for defensive mode
        virtual void
                getNewEdges(std::vector<std::pair< Address, 
                                Dyninst::ParseAPI::EdgeTypeEnum> >&outEdges, 
                Dyninst::ParseAPI::Function * context,
                Dyninst::ParseAPI::Block * currBlk,
                unsigned int num_insns,
                dyn_hash_map<Address, std::string> *pltFuncs,
		const set<Address>& knownTargets) const;
        virtual InstrumentableLevel getInstLevel(Dyninst::ParseAPI::Function *, unsigned int num_insns ) const;
        virtual bool isDynamicCall() const;
        virtual bool isAbsoluteCall() const;
        virtual bool simulateJump() const;
        virtual void advance();
        virtual bool retreat();
        virtual bool isNop() const = 0;
        virtual bool isLeave() const;
        virtual bool isDelaySlot() const;
        virtual bool isRelocatable(InstrumentableLevel lvl) const;
        virtual bool isTailCall(const ParseAPI::Function *,
                                Dyninst::ParseAPI::EdgeTypeEnum,
                                unsigned int,
                                const std::set<Address> &) const = 0;
        virtual std::pair<bool, Address> getCFT() const;
        virtual bool isStackFramePreamble() const = 0;
        virtual bool savesFP() const = 0;
        virtual bool cleansStack() const = 0;
        virtual bool isConditional() const;
        virtual bool isBranch() const;
        virtual bool isInterruptOrSyscall() const;
        virtual bool isCall() const;
        virtual bool isReturnAddrSave(Address &ret_addr) const = 0;
        virtual bool isNopJump() const = 0;
        virtual bool sliceReturn(ParseAPI::Block* bit, Address ret_addr, ParseAPI::Function * func) const = 0;
        virtual bool isIATcall(std::string &calleeName) const = 0;
        virtual bool isThunk() const = 0;
	virtual bool isIndirectJump() const;

protected:
        virtual bool isRealCall() const;
        virtual bool parseJumpTable(Dyninst::ParseAPI::Function * currFunc,
	                            Dyninst::ParseAPI::Block* currBlk,
				    std::vector<std::pair< Address, Dyninst::ParseAPI::EdgeTypeEnum > >& outEdges) const;
        virtual bool isIPRelativeBranch() const;
        virtual bool isFrameSetupInsn(Dyninst::InstructionAPI::Instruction i) const = 0;
        virtual bool isReturn(Dyninst::ParseAPI::Function *, Dyninst::ParseAPI::Block* currBlk) const = 0;
        virtual bool isFakeCall() const = 0;
        virtual bool isLinkerStub() const = 0;
	virtual bool isSysEnter() const;
	virtual void parseSyscall(std::vector<std::pair<Address, Dyninst::ParseAPI::EdgeTypeEnum> >& outEdges) const;
	virtual void parseSysEnter(std::vector<std::pair<Address, Dyninst::ParseAPI::EdgeTypeEnum> >& outEdges) const;
        std::pair<bool, Address> getFallthrough() const;

        Dyninst::InstructionAPI::InstructionDecoder dec;

        /*
         * Decoded instruction cache: contains the linear
         * sequence of instructions decoded by the decoder
         * underlying this adapter.
         * 
         * - curInsnIter == *(allInsns.end()-1)
         * - (super)->current = curInsnIter->first
         */
public:
        typedef std::vector< 
            std::pair<Address, 
            Dyninst::InstructionAPI::Instruction> 
        > allInsns_t;
protected:
        allInsns_t allInsns;
        const InstructionAPI::Instruction & curInsn() const;
        allInsns_t::iterator curInsnIter;

        mutable bool validCFT;
        mutable std::pair<bool, Address> cachedCFT;
        mutable bool validLinkerStubState;
        mutable bool cachedLinkerStubState;
        mutable std::pair<bool, bool> hascftstatus;

        mutable std::map<ParseAPI::EdgeTypeEnum, bool> tailCalls;

        static std::once_flag ptrInit;
        static std::map<Architecture, Dyninst::InstructionAPI::RegisterAST::Ptr> framePtr;
        static std::map<Architecture, Dyninst::InstructionAPI::RegisterAST::Ptr> stackPtr;
        static std::map<Architecture, Dyninst::InstructionAPI::RegisterAST::Ptr> thePC;
        static std::map<Address, bool> thunkAtTarget;
        static void initASTs();
};

}
}


#endif // !defined(IA_IAPI_H)
