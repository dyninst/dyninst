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

#if !defined(IA_INSTRUCITER_H)
#define IA_INSTRUCITER_H

#include "InstructionAdapter.h"
#include "InstrucIter.h"

class IA_InstrucIter : public InstructionAdapter
{
    public:
        instruction getInstruction();
        IA_InstrucIter(InstrucIter from, image_func* f);
        virtual bool hasCFT() const;
        virtual size_t getSize() const;
        virtual bool isFrameSetupInsn() const;
        virtual bool isAbortOrInvalidInsn() const;
        virtual bool isAllocInsn() const;
    // TODO
        virtual void
                getNewEdges(std::vector<std::pair< Address, EdgeTypeEnum> >&
                outEdges, image_basicBlock* currBlk,
                std::vector<instruction>& all_insns,
                dictionary_hash<Address, std::string> *pltFuncs) const;
        virtual bool isDynamicCall() const;
        virtual bool isAbsoluteCall() const;
        virtual bool simulateJump() const;
        virtual bool isRelocatable(InstrumentableLevel lvl) const;
        virtual void advance();
        virtual bool isNop() const;
        virtual bool isLeave() const;
        virtual bool isDelaySlot() const;
        virtual bool isTailCall(std::vector<instruction>& all_insns) const;
    
    private:
        virtual bool isRealCall() const;
        virtual bool isReturn() const;
        virtual bool isBranch() const;
        virtual bool isCall() const;
        virtual Address getCFT() const;
        
        mutable InstrucIter ii;
        
};


#endif // !defined(IA_INSTRUCITER_H)
