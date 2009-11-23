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

#if !defined(IA_INSTRUCITER_H)
#define IA_INSTRUCITER_H

#include "InstructionAdapter.h"
#include "InstrucIter.h"

class IA_InstrucIter : public InstructionAdapter
{
    public:
        instruction getInstruction();
        IA_InstrucIter(InstrucIter from, image_func* f);
        IA_InstrucIter(InstrucIter from, image * im);
        virtual bool hasCFT() const;
        virtual size_t getSize() const;
        virtual bool isFrameSetupInsn() const;
        virtual bool isAbortOrInvalidInsn() const;
        virtual bool isAllocInsn() const;
    // TODO
        virtual void
                getNewEdges(pdvector<std::pair< Address, EdgeTypeEnum> >&
                outEdges, image_basicBlock* currBlk,
                unsigned int num_insns,
                dictionary_hash<Address, std::string> *pltFuncs) const;
        virtual bool isDynamicCall() const;
        virtual bool isAbsoluteCall() const;
        virtual bool simulateJump() const;
        virtual bool isRelocatable(InstrumentableLevel lvl) const;
        virtual void advance();
        virtual bool isNop() const;
        virtual bool isLeave() const;
        virtual bool isDelaySlot() const;
        virtual bool isTailCall(unsigned int num_insns) const;
        virtual bool checkEntry() const;
        virtual Address getCFT() const;
        virtual bool isStackFramePreamble(int& frameSize) const;
        virtual bool savesFP() const;
        virtual bool cleansStack() const;
        virtual bool isConditional() const;
        virtual bool isBranch() const;
        virtual bool isInterruptOrSyscall() const;
   
    private:
        virtual bool isRealCall() const;
        virtual bool isReturn() const;
        virtual bool isCall() const;
        
        mutable InstrucIter ii;
        
};


#endif // !defined(IA_INSTRUCITER_H)
