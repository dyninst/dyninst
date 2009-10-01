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
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

#if !defined(IA_IAPI_H)
#define IA_IAPI_H

#include "InstructionAdapter.h"
#include "Instruction.h"
#include "InstructionDecoder.h"
#include <boost/tuple/tuple.hpp>

class IA_IAPI : public InstructionAdapter
{
    public:
        IA_IAPI(Dyninst::InstructionAPI::InstructionDecoder dec_,
                Address start_, image_func* f);
        Dyninst::InstructionAPI::Instruction::Ptr getInstruction();
    
        virtual bool hasCFT() const;
        virtual size_t getSize() const;
        virtual bool isFrameSetupInsn() const;
        virtual bool isAbortOrInvalidInsn() const;
        virtual bool isAllocInsn() const;
        virtual void
                getNewEdges(std::vector<std::pair< Address, EdgeTypeEnum> >&
                outEdges, image_basicBlock* currBlk,
                std::vector<instruction>& all_insns,
                dictionary_hash<Address, std::string> *pltFuncs) const;
        virtual bool isDynamicCall() const;
        virtual bool isAbsoluteCall() const;
        virtual bool simulateJump() const;
        virtual void advance();
        virtual bool isNop() const;
        virtual bool isLeave() const;
        virtual bool isDelaySlot() const;
        virtual bool isRelocatable(InstrumentableLevel lvl) const;
        virtual bool isTailCall(std::vector<instruction>&) const;
        virtual bool checkEntry() const;
        virtual Address getCFT() const;
        virtual bool isStackFramePreamble(int& frameSize) const;
        virtual bool savesFP() const;
        virtual bool cleansStack() const;
        virtual bool isConditional() const;
        virtual bool isBranch() const;
    private:
        virtual bool isRealCall() const;
        bool parseJumpTable(image_basicBlock* currBlk,
                            std::vector<std::pair< Address, EdgeTypeEnum > >& outEdges) const;
        bool isIPRelativeBranch() const;
        bool isMovAPSTable(std::vector<std::pair< Address, EdgeTypeEnum > >& outEdges) const;
        Address findThunkAndOffset(image_basicBlock* start) const;
        bool isTableInsn(Dyninst::InstructionAPI::Instruction::Ptr i) const;
        std::pair<Address, Dyninst::InstructionAPI::Instruction::Ptr> findTableInsn() const;
        boost::tuple<Dyninst::InstructionAPI::Instruction::Ptr,
        Dyninst::InstructionAPI::Instruction::Ptr,
        bool> findMaxSwitchInsn(image_basicBlock *start) const;
        bool findThunkInBlock(image_basicBlock* curBlock, Address& thunkOffset) const;
        bool computeTableBounds(Dyninst::InstructionAPI::Instruction::Ptr maxSwitchInsn,
                                Dyninst::InstructionAPI::Instruction::Ptr branchInsn,
                                Dyninst::InstructionAPI::Instruction::Ptr tableInsn,
                                bool foundJCCAlongTaken,
                                unsigned& tableSize,
                                unsigned& tableStride) const;
        bool fillTableEntries(Address thunkOffset,
                              Address tableBase,
                              unsigned tableSize,
                              unsigned tableStride,
                              std::vector<std::pair< Address, EdgeTypeEnum> >& outEdges,
                             bool tableEntriesRelative) const;
        Address getTableAddress(Dyninst::InstructionAPI::Instruction::Ptr tableInsn,
                                Address thunkOffset, bool& tableEntriesRelative) const;
        bool isFrameSetupInsn(Dyninst::InstructionAPI::Instruction::Ptr i) const;
        virtual bool isReturn() const;
        virtual bool isCall() const;




        Dyninst::InstructionAPI::InstructionDecoder dec;
        std::map<Address, Dyninst::InstructionAPI::Instruction::Ptr> allInsns;
        Dyninst::InstructionAPI::Instruction::Ptr curInsn() const;
        std::map<Address, Dyninst::InstructionAPI::Instruction::Ptr>::const_iterator curInsnIter;
        mutable bool validCFT;
        mutable Address cachedCFT;
        mutable std::pair<bool, bool> hascftstatus;
        mutable std::pair<bool, bool> tailCall;
};


#endif // !defined(IA_IAPI_H)
