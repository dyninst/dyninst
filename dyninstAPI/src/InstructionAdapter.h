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

#if !defined(INSTRUCTION_ADAPTER_H)
#define INSTRUCTION_ADAPTER_H

#include "image-func.h"
#include "instPoint.h"

#if !defined(ESSENTIAL_PARSING_ENUMS)
#define ESSENTIAL_PARSING_ENUMS
enum EdgeTypeEnum {
   ET_CALL,
   ET_COND_TAKEN,
   ET_COND_NOT_TAKEN,
   ET_INDIR,
   ET_DIRECT,
   ET_FALLTHROUGH,
   ET_CATCH,
   ET_FUNLINK,  // connect block ended by call instruction with next block
               // (see BIT)
   ET_NOEDGE
};

// Function return status. We initially assume that all functions
// do not return. Unresolvable control flow edges change the status
// to UNKNOWN, and return instructions (or the equivalent) change
// status to RETURN
enum FuncReturnStatus {
    RS_UNSET,       // convenience for parsing
    RS_NORETURN,
    RS_UNKNOWN,
    RS_RETURN
};
// There are three levels of function-level "instrumentability":
// 1) The function can be instrumented normally with no problems (normal case)
// 2) The function contains unresolved indirect branches; we have to assume
//    these can go anywhere in the function to be safe, so we must instrument
//    safely (e.g., with traps)
// 3) The function is flatly uninstrumentable and must not be touched.
enum InstrumentableLevel {
    NORMAL,
    HAS_BR_INDIR,
    UNINSTRUMENTABLE
};
#endif //!defined(ESSENTIAL_PARSING_ENUMS)

class InstructionAdapter
{
    public:
        InstructionAdapter(Address start, image_func* f);
        InstructionAdapter(Address start, image* im);
        virtual ~InstructionAdapter();
    // Implemented
    virtual bool hasCFT() const = 0;
    virtual size_t getSize() const = 0;
    virtual bool isFrameSetupInsn() const = 0;
    virtual bool isAbortOrInvalidInsn() const = 0;
    virtual bool isAllocInsn() const = 0;
    // TODO
    virtual void
            getNewEdges(pdvector<std::pair< Address, EdgeTypeEnum> >&
            outEdges, image_basicBlock* currBlk,
            unsigned int num_insns,
            dictionary_hash<Address, std::string> *pltFuncs) const =
0;
    virtual bool isDynamicCall() const = 0;
    virtual bool isAbsoluteCall() const = 0;
    virtual InstrumentableLevel getInstLevel(unsigned int num_insns) const;
    virtual FuncReturnStatus getReturnStatus(image_basicBlock* currBlk, unsigned int num_insns) const ;
    virtual instPointType_t getPointType(unsigned int num_insns,
                                         dictionary_hash<Address, std::string>
*pltFuncs) const;
    virtual bool hasUnresolvedControlFlow(image_basicBlock* currBlk, unsigned int num_insns)
const;
            virtual bool simulateJump() const= 0;
    virtual void advance() = 0;
    virtual bool isNop() const = 0;
    virtual bool isLeave() const = 0;
    virtual bool isDelaySlot() const = 0;
    virtual bool isRelocatable(InstrumentableLevel lvl) const = 0;
    virtual Address getAddr() const;
    virtual Address getPrevAddr() const;
    virtual Address getNextAddr() const;
    virtual bool checkEntry() const = 0;
    virtual Address getCFT() const = 0;
    virtual bool isStackFramePreamble(int& frameSize) const = 0;
    virtual bool savesFP() const = 0;
    virtual bool cleansStack() const = 0;
    virtual bool isConditional() const = 0;
    virtual bool isBranch() const = 0;
    virtual bool isInterruptOrSyscall() const = 0;
    protected:
        virtual bool isReturn() const = 0;
        virtual bool isCall() const = 0;
        virtual bool isTailCall(unsigned int num_insns) const = 0;
        virtual bool isRealCall() const = 0;
        Address current;
    Address previous;
    mutable bool parsedJumpTable;
    mutable bool successfullyParsedJumpTable;
    mutable bool isDynamicCall_;
    mutable bool checkedDynamicCall_;
    mutable bool isInvalidCallTarget_;
    mutable bool checkedInvalidCallTarget_;
    image_func* context;
    image* img;
};


#endif // !defined(INSTRUCTION_ADAPTER_H)
