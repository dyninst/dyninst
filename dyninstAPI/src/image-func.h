/*
 * Copyright (c) 1996-2004 Barton P. Miller
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
 
// $Id: image-func.h,v 1.21 2006/06/11 00:35:07 legendre Exp $

#ifndef IMAGE_FUNC_H
#define IMAGE_FUNC_H

#include "common/h/String.h"
#include "common/h/Vector.h"
#include "common/h/Types.h"
#include "common/h/Pair.h"
#include "codeRange.h"
#include "arch.h" // instruction

#if !defined(BPATCH_LIBRARY)
#include "paradynd/src/resource.h"
#endif

#include "dyninstAPI/h/BPatch_Set.h"
#include "common/h/Dictionary.h"

class pdmodule;
class InstrucIter;

// Slight modifications (and specialization) of BPatch_*
class image_basicBlock;
class image_instPoint;

// Added support for typed edges 12.Oct.2005 -- nate
class image_edge;

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

// Function return status. Initially UNSET; after parsing
// all functions will be RETURN, NORETURN, or UNKNOWN.
enum FuncReturnStatus {
    RS_UNSET,
    RS_UNKNOWN,
    RS_RETURN,
    RS_NORETURN
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

class image_edge {
    friend class image_basicBlock;
 public:
   image_edge(image_basicBlock *source, 
              image_basicBlock *target, 
              EdgeTypeEnum type) :
      source_(source),
      target_(target),
      type_(type) {}

   image_basicBlock * getSource() const { return source_; }
   image_basicBlock * getTarget() const { return target_; }
   EdgeTypeEnum getType() const { return type_; }

   void breakEdge();

   char * getTypeString();

 private:
   image_basicBlock *source_;
   image_basicBlock *target_;
   EdgeTypeEnum type_;
};

class image_basicBlock : public codeRange {
    friend class image_func;
 public:
    image_basicBlock(image_func *func, Address firstOffset);

    Address firstInsnOffset() const { return firstInsnOffset_; }
    Address lastInsnOffset() const { return lastInsnOffset_; }
    // Just to be obvious -- this is the end addr of the block
    Address endOffset() const { return blockEndOffset_; }
    Address getSize() const { return blockEndOffset_ - firstInsnOffset_; }
    
    bool isEntryBlock(image_func * f) const;
    bool isExitBlock() const { return isExitBlock_; }

    // Reverse if isEntryBlock; if we're the entry for a function return it.
    image_func *getEntryFunc() const;

    bool isShared() const { return isShared_; }

    static int compare(image_basicBlock *&b1,
                       image_basicBlock *&b2) {
        if (b1->firstInsnOffset() < b2->firstInsnOffset())
            return -1;
        if (b2->firstInsnOffset() < b1->firstInsnOffset())
            return 1;

        if(b1 != b2)
            fprintf(stderr,"oh gnoes, blocks shouldn't match: 0x%p 0x%p are at 0x%lx \n",b1,b2,b1->firstInsnOffset());

        assert(b1 == b2);
        return 0;
    }

    void debugPrint();

    void getSources(pdvector<image_edge *> &ins) const;
    void getTargets(pdvector<image_edge *> &outs) const;

    Address get_address_cr() const { return firstInsnOffset(); }
    unsigned get_size_cr() const { return getSize(); }
    void *getPtrToInstruction(Address addr) const;

    // splitting blocks
    image_basicBlock * split(Address loc, image_func *succ_func);
    void split(image_basicBlock * &newBlk);

    bool addTarget(image_edge *edge);
    bool addSource(image_edge *edge);

    void removeTarget(image_edge *edge);
    void removeSource(image_edge *edge);

    int id() const { return blockNumber_; }

   // blocks may belong to more than one function
   // image_func *func() const { return func_; }
    void getFuncs(pdvector<image_func *> &funcs) const;

    // convenience method: sometimes any function will do
    image_func * getFirstFunc() const
    {
        if(funcs_.size() > 0)
            return funcs_[0];
        else
            return NULL;
    }

    bool containedIn(image_func * f);

    // add another owning function to this basic block
    void addFunc(image_func *func);

    bool containsRet() { return containsRet_; }

    bool containsCall() { return containsCall_; }
    bool callIsOpaque() { return callIsOpaque_; }

    image_instPoint * getCallInstPoint();
    image_instPoint * getRetInstPoint();

    bool canBeRelocated() const { return canBeRelocated_; }

   private:

    // Try to shrink memory usage down.
    void finalize();

    Address firstInsnOffset_;
    Address lastInsnOffset_;
    Address blockEndOffset_;

    bool isEntryBlock_;
    bool isExitBlock_;

    int blockNumber_;

    bool isShared_;     // block shared by > 1 functions

    bool isStub_;       // used in parsing -- if true, has not been parsed

    bool containsRet_; // both of these are tantamount to saying "ends with X"
    bool containsCall_;

    bool callIsOpaque_; // The call site that ends this block is "opaque" with
                        // respect to its return-status (i.e., whether the
                        // the target function returns or not is unknown)

    bool isSpeculative_;    // validity uncertain

    bool canBeRelocated_; // some blocks contain uninstrumentable constructs

    pdvector<image_edge *> targets_;
    pdvector<image_edge *> sources_;

    pdvector<image_func *> funcs_;
};

// Handle creation of edges and insertion of source/target pointers
// in basic block objects
void addEdge(image_basicBlock *source, 
             image_basicBlock *target, 
             EdgeTypeEnum type);

void checkIfRelocatable (instruction insn, bool &canBeRelocated);

bool isRealCall(instruction insn, 
                Address addr, 
                image *owner, 
                bool &validTarget);

// Parse-level function object. Knows about offsets, names, and suchlike; 
// does _not_ do function relocation.
class image_func : public codeRange {
 public:
   static pdstring emptyString;

   // Almost everything gets filled in later.
   image_func(const pdstring &symbol, 
	      Address offset, 
	      const unsigned symTabSize, 
	      pdmodule *m,
	      image *i);
   

   ~image_func();

   ////////////////////////////////////////////////
   // Basic output functions
   ////////////////////////////////////////////////

   const pdstring &symTabName() const { 
       if (symTabNames_.size() > 0) return symTabNames_[0];
       else return emptyString;
   }
   const pdstring &prettyName() const {
       if (prettyNames_.size() > 0) return prettyNames_[0];
       else return emptyString;
   }
   const pdstring &typedName() const {
       if (typedNames_.size() > 0) return typedNames_[0];
       else return emptyString;
   }

   const pdvector<pdstring> &symTabNameVector() const { return symTabNames_; }
   const pdvector<pdstring> &prettyNameVector() const { return prettyNames_; }
   const pdvector<pdstring> &typedNameVector() const { return typedNames_; }
   void copyNames(image_func *duplicate);

   // Bool: returns true if the name is new (and therefore added)
   bool addSymTabName(pdstring name, bool isPrimary = false);
   bool addPrettyName(pdstring name, bool isPrimary = false);
   bool addTypedName(pdstring name, bool isPrimary = false);

   Address getOffset() const {return startOffset_;}
   Address getEndOffset(); // May trigger parsing
   unsigned getSymTabSize() const { return symTabSize_; }

   // coderange needs a get_address...
   Address get_address_cr() const { return getOffset();}
   unsigned get_size_cr() const { return endOffset_ - startOffset_; }

   void *getPtrToInstruction(Address addr) const;


   // extra debuggering info....
   ostream & operator<<(ostream &s) const;
   friend ostream &operator<<(ostream &os, image_func &f);
   pdmodule *pdmod() const { return mod_;}
   image *img() const { return image_; }
   void changeModule(pdmodule *mod);

   ////////////////////////////////////////////////
   // CFG and other function body methods
   ////////////////////////////////////////////////

   const pdvector< image_basicBlock* > &blocks();

   bool hasNoStackFrame();
   bool makesNoCalls();
   bool savesFramePointer();
   bool cleansOwnStack();

   ////////////////////////////////////////////////
   // Parsing support methods
   ////////////////////////////////////////////////

   void parseSharedBlocks(image_basicBlock * firstBlock,
                BPatch_Set< Address > &leaders,
                dictionary_hash< Address, image_basicBlock * > &leadersToBlock,
                BPatch_Set< image_basicBlock* > &parserVisited,
                Address & funcEnd);
   void parseSharedBlocks(image_basicBlock * firstBlock);

   // Helper function: create a new basic block and add to various data
   // structures (if the new addr is valid)
   bool addBasicBlock(Address newAddr,
                      image_basicBlock *oldBlock,
                      BPatch_Set<Address> &leaders,
                      dictionary_hash<Address, image_basicBlock *> &leadersToBlock,
                      EdgeTypeEnum edgeType,
                      pdvector<Address> &worklist,
                      BPatch_Set<image_basicBlock *> &parserVisited); 

   // Add a basic block to the blocklist when this is not the function
   // being parsed. Helps maintain consistency across basic block split
   // operations initiated by another function's parsing.
   void addToBlocklist(image_basicBlock *newBlk);

   // Can we determine whether this function returns?
   FuncReturnStatus returnStatus() { return retStatus_; }

   // Platform-independent sorting/cleaning/fixing of instPoints and
   // basic blocks.
   bool cleanBlockList();
   void checkCallPoints();
    
   void sortBlocklist();
   bool isBLSorted() { return bl_is_sorted; }

   Address newCallPoint(Address adr, const instruction code, bool &err);

   ////////////////////////////////////////////////
   // Architecture-dependent parsing support
   ////////////////////////////////////////////////

    bool archIsUnparseable();
    bool archAvoidParsing();
    void archGetFuncEntryAddr(Address &funcEntryAddr);
    bool archNoRelocate();
    void archSetFrameSize(int frameSize);
    bool archGetMultipleJumpTargets( 
             BPatch_Set< Address >& targets,
             image_basicBlock *currBlk,
             InstrucIter &ah,
             pdvector< instruction >& allInstructions);
    bool archProcExceptionBlock(Address &catchStart, Address a);
    bool archIsATailCall(InstrucIter &ah,
             pdvector< instruction >& allInstructions);
    bool archIsIndirectTailCall(InstrucIter &ah);
    bool archIsAbortOrInvalid(InstrucIter &ah);
    bool archIsRealCall(InstrucIter &ah, bool &validTarget, bool &simulateJump);
    bool archCheckEntry(InstrucIter &ah, image_func *func );
    void archInstructionProc(InstrucIter &ah);


   ////////////////////////////////////////////////
   // Instpoints!
   ////////////////////////////////////////////////

   const pdvector<image_instPoint*> &funcEntries();
   const pdvector<image_instPoint*> &funcExits();
   const pdvector<image_instPoint*> &funcCalls();
   
   // Defined in inst-flowGraph.C
   // The address vector is "possible call targets you might be interested in".
   bool parse( pdvector<Address >&,
                      dictionary_hash< Address, image_func *>& preParseStubs );
   // Do most of the parsing work. Calls architecture-dependent routines
   // defined in image-<arch>.C
   bool buildCFG(pdvector<image_basicBlock *> &funcEntry,
                      Address funcBegin,
                      pdvector< Address >& callTargets,
                      dictionary_hash< Address, image_func *>& preParseStubs);

   bool isTrapFunc() const {return isTrap;}
   bool isInstrumentable() const { return instLevel_ != UNINSTRUMENTABLE; }
   InstrumentableLevel instLevel() const { return instLevel_; }

   void addCallInstPoint(image_instPoint *p);
   void addExitInstPoint(image_instPoint *p);

#if defined(cap_stripped_binaries)
   // Update if symtab is incorrect
   void updateFunctionEnd( Address addr);  
#endif

   // ----------------------------------------------------------------------

   ////////////////////////////////////////////////
   // Misc
   ////////////////////////////////////////////////

   codeRange *copy() const;

#if defined(arch_ia64) || defined(arch_x86) || defined(arch_x86_64)

   bool isTrueCallInsn(const instruction insn);
#endif

#if defined(sparc_sun_solaris2_4)
   bool is_o7_live(){ return o7_live; }
#endif

   // Only defined on alpha right now
   int             frame_size; // stack frame size

#if defined(arch_ia64)
   // We need to know where all the alloc instructions in the
   // function are to do a reasonable job of register allocation
   // in the base tramp.  
   pdvector< Address > allocs;
   
   // Since the IA-64 ABI does not define a frame pointer register,
   // we use DWARF debug records (DW_AT_frame_base entries) to 
   // construct an AST which calculates the frame pointer.
   AstNode * framePointerCalculator;
   
   // Place to store the results of doFloatingPointStaticAnalysis().
   // This way, if they are ever needed in a mini-tramp, emitFuncJump()
   // for example, the expensive operation doesn't need to happen again.
   bool * usedFPregs;
#endif

   // Ifdef relocation... but set at parse time.
   bool canBeRelocated() const { return canBeRelocated_; }
   bool needsRelocation() const { return needsRelocation_; }
   void markAsNeedingRelocation(bool foo) { needsRelocation_ = foo; }

   bool containsSharedBlocks() const { return containsSharedBlocks_; }

   image_basicBlock * entryBlock();

   bool parsed() { return parsed_; }

 private:

   ///////////////////// Basic func info
   pdvector<pdstring> symTabNames_;	/* name as it appears in the symbol table */
   pdvector<pdstring> prettyNames_;	/* user's view of name (i.e. de-mangled) */
   pdvector<pdstring> typedNames_;      /* de-mangled with types */
   Address startOffset_;		/* address of the start of the func */
   Address endOffset_;          /* Address of the (next instruction after) the end of the func */
   unsigned symTabSize_;        /* What we get from the symbol table (if any) */
   pdmodule *mod_;		/* pointer to file that defines func. */
   image *image_;
   bool parsed_;                /* Set to true in findInstPoints */
   bool cleansOwnStack_;


   ///////////////////// CFG and function body
   pdvector< image_basicBlock* > blockList;

   bool noStackFrame; // formerly "leaf".  True iff this fn has no stack frame.
   bool makesNoCalls_;
   bool savesFP_;
   bool call_points_have_been_checked; 
        // true if checkCallPoints has been called.

   bool containsSharedBlocks_;  // True if one or more blocks in this
                                // function are shared with another function.

   FuncReturnStatus retStatus_; // Does this function return or not?

    // Bind a call target to the current basic block
   image_func* bindCallTarget(Address target,
        image_basicBlock * currBlk,
        pdvector< Address >& callTargets,
        dictionary_hash< Address, image_func * >& preParseStubs);

    // Find an existing image_func object or create a new one
    // (inserting it into preParseStubs and pushing it onto 
    // callTargets if it is created)
   image_func * FindOrCreateFunc(Address target,
        pdvector< Address >& callTargets,
        dictionary_hash< Address, image_func * >& preParseStubs);

   
   ///////////////////// Instpoints 

   pdvector<image_instPoint*> funcEntries_;     /* place to instrument entry 
                                                   (often not addr) */
   pdvector<image_instPoint*> funcReturns;	/* return point(s). */
   pdvector<image_instPoint*> calls;		/* pointer to the calls */
   
   bool isTrap; 		// true if function contains a trap instruct
   InstrumentableLevel instLevel_;   // the degree of freedom we have in
                                    // instrumenting the function
   
   bool canBeRelocated_;           // True if nothing prevents us from
                                   // relocating function
   bool needsRelocation_;          // Override -- "relocate this func"

   void *originalCode;   // points to copy of original function

   // Hacky way around parsing things -- we can stick things known to be 
   // unparseable in here.
   bool isInstrumentableByFunctionName();

   // Misc   

   // the vector "calls" should not be accessed until this is true.
   bool o7_live;

   // Functions only have one entry basic block
   image_basicBlock *entryBlock_;

   // Block list must be sorted
   bool bl_is_sorted;
};

typedef image_func *ifuncPtr;

struct ifuncCmp
{
    int operator()( const ifuncPtr &f1, const ifuncPtr &f2 ) const
    {
        if( f1->getOffset() > f2->getOffset() )
            return 1;
        if( f1->getOffset() < f2->getOffset() )
            return -1;
        return 0;
    }
};

#endif /* FUNCTION_H */
