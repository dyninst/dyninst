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
 
// $Id: image-func.h,v 1.37 2008/09/03 06:08:44 jaw Exp $

#ifndef IMAGE_FUNC_H
#define IMAGE_FUNC_H

#include <string>
#include "common/h/Vector.h"
#include "common/h/Types.h"
#include "common/h/Pair.h"
#include "codeRange.h"
#include "arch.h" // instruction
#include "parRegion.h"
#include "dyninstAPI/h/BPatch_Set.h"
#include "common/h/Dictionary.h"
#include "symtabAPI/h/Symbol.h"
#include "dyninstAPI/src/bitArray.h"
#include "InstructionCache.h"
#include "InstructionAdapter.h"
#include <set>

#include "symtabAPI/h/Function.h"

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;
using namespace std;

class pdmodule;
class InstrucIter;

// Slight modifications (and specialization) of BPatch_*
class image_basicBlock;
class image_instPoint;

// CFG edges are typed
class image_edge;
   

class InstructionAdapter;
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


// Indicates the source of this function, e.g. whether
// it was listed in a symbol table, discovered with
// recursive traversal, speculatively found in gap parsing, etc
enum FuncSource {
    FS_SYMTAB,
    FS_RT,
    FS_GAP,
    FS_ONDEMAND,
    FS_GAPRT
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

   const char * getTypeString();

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

    struct compare {
        bool operator()(image_basicBlock * const &b1,
                        image_basicBlock * const &b2) const {
            if(b1->firstInsnOffset() < b2->firstInsnOffset())
                return true;
            if(b2->firstInsnOffset() < b1->firstInsnOffset())
                return false;

            // XXX the remainder is debugging, and should be removed
            if(b1 != b2)
                fprintf(stderr,"error: two blocks (%p,%p) at 0x%lx\n",
                    b1,b2,b1->firstInsnOffset());

            assert(b1 == b2);
            return false;
        }
    };

    typedef std::set<image_basicBlock *, image_basicBlock::compare> blockSet;

    void debugPrint();

    void getSources(pdvector<image_edge *> &ins) const;
    void getTargets(pdvector<image_edge *> &outs) const;

    Address get_address() const { return firstInsnOffset(); }
    unsigned get_size() const { return getSize(); }
    void *getPtrToInstruction(Address addr) const;

    // splitting blocks
    image_basicBlock * split(Address loc, image_func *succ_func);
    void split(image_basicBlock * &newBlk);

    bool addTarget(image_edge *edge);
    bool addSource(image_edge *edge);

    void removeTarget(image_edge *edge);
    void removeSource(image_edge *edge);

    int id() const { return blockNumber_; }

    const set<image_func *> & getFuncs() const;

    // convenience method: sometimes any function will do
    image_func * getFirstFunc() const
    {
        if(!funcs_.empty())
            return *funcs_.begin();
        else
            return NULL;
    }

    bool containedIn(image_func * f);

    // add another owning function to this basic block
    void addFunc(image_func *func);

    bool containsRet() { return containsRet_; }

    bool containsCall() { return containsCall_; }

    image_instPoint * getCallInstPoint();
    image_instPoint * getRetInstPoint();

    bool canBeRelocated() const { return canBeRelocated_; }
    bool needsRelocation() const { return needsRelocation_; }
    void markAsNeedingRelocation() { needsRelocation_ = true; }

#if defined(cap_liveness)
    const bitArray &getLivenessIn();
    // This is copied from the union of all successor blocks
    const bitArray getLivenessOut() const;
#endif

#if defined(cap_instruction_api)
    void getInsnInstances(std::vector<std::pair<InstructionAPI::Instruction::Ptr, Offset> > &instances);
#endif

   private:
    // Try to shrink memory usage down.
    void finalize();

    Address firstInsnOffset_;
    Address lastInsnOffset_;
    Address blockEndOffset_;

    bool isEntryBlock_;
    bool isExitBlock_;
    bool needsRelocation_;
    bool isShared_;     // block shared by > 1 functions

    int blockNumber_;

    bool isStub_;       // used in parsing -- if true, has not been parsed
    bool containsRet_; // both of these are tantamount to saying "ends with X"
    bool containsCall_;
    bool canBeRelocated_; // some blocks contain uninstrumentable constructs

    pdvector<image_edge *> targets_;
    pdvector<image_edge *> sources_;

    set<image_func *> funcs_;

#if defined(cap_liveness)
    /* Liveness analysis variables */
    /** gen registers */
    
    bitArray use; // Registers used by instructions within the block
    bitArray def; // Registers defined by instructions within the block
    bitArray in;  // Summarized input liveness; we calculate output on the fly
 public:
    static InstructionCache cachedLivenessInfo;
    
 private:
    void summarizeBlockLivenessInfo();
    // Returns true if any information changed; false otherwise
    bool updateBlockLivenessInfo();
#endif


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

#include "ast.h"

class image_func_registers {
 public:
  std::set<Register> generalPurposeRegisters;
  std::set<Register> floatingPointRegisters;
  std::set<Register> specialPurposeRegisters;
};

// Parse-level function object. Knows about offsets, names, and suchlike; 
// does _not_ do function relocation.
class image_func : public codeRange,
                   public AnnotatableSparse
{
 public:
   static std::string emptyString;

   image_func() {}
   // Almost everything gets filled in later.
   image_func(const std::string &symbol, 
	      Address offset, 
	      const unsigned symTabSize, 
	      pdmodule *m,
	      image *i,
          FuncSource src);
  
   image_func(Function *func, pdmodule *m, image *i, FuncSource src);

   ~image_func();


   ////////////////////////////////////////////////
   // Basic output functions
   ////////////////////////////////////////////////

   Function* getSymtabFunction() const{
      return  func_; 
   }	

   const string &symTabName() const { 
       return func_->getFirstSymbol()->getName();
   }
   const string &prettyName() const {
       return func_->getFirstSymbol()->getPrettyName();
   }
   const string &typedName() const {
       return func_->getFirstSymbol()->getTypedName();
   }

   const vector<string> &symTabNameVector() const {
       return func_->getAllMangledNames();
   }
   const vector<string> &prettyNameVector() const {
       return func_->getAllPrettyNames();
   }
   const vector<string> &typedNameVector() const {
       return func_->getAllTypedNames();
   }
   void copyNames(image_func *duplicate);

   // Bool: returns true if the name is new (and therefore added)
   bool addSymTabName(std::string name, bool isPrimary = false);
   bool addPrettyName(std::string name, bool isPrimary = false);
   bool addTypedName(std::string name, bool isPrimary = false);

   Address getOffset() const {return func_->getFirstSymbol()->getOffset();}
   Address getPtrOffset() const {return func_->getFirstSymbol()->getPtrOffset();}
   Address getEndOffset(); // May trigger parsing
   unsigned getSymTabSize() const { return func_->getFirstSymbol()->getSize(); }

   // coderange needs a get_address...
   Address get_address() const { return getOffset();}
   unsigned get_size() const { return endOffset_ - getOffset(); }

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

   const set<image_basicBlock*, image_basicBlock::compare> &blocks();

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
                Address & funcEnd);
   void parseSharedBlocks(image_basicBlock * firstBlock);

   // Helper function: create a new basic block and add to various data
   // structures (if the new addr is valid)
   bool addBasicBlock(Address newAddr,
                      image_basicBlock *oldBlock,
                      BPatch_Set<Address> &leaders,
                      dictionary_hash<Address, image_basicBlock *> &leadersToBlock,
                      EdgeTypeEnum edgeType,
                      pdvector<Address> &worklist);

   // Add a basic block to the blocklist when this is not the function
   // being parsed. Helps maintain consistency across basic block split
   // operations initiated by another function's parsing.
   void addToBlocklist(image_basicBlock *newBlk);

   // Can we determine whether this function returns?
   FuncReturnStatus returnStatus() { return retStatus_; }

   // Sort basic block list, instrumentation points, and finalize vector
   // data structures to reduce memory
   bool finalize();

   // Check the targets of _call_ instrumentation points and link up
   // dangling targets, if possible
   void checkCallPoints();
   void checkCallPoints(pdvector<image_instPoint *> &points);
    
   Address newCallPoint(Address adr, const instruction code, bool &err);

   ////////////////////////////////////////////////
   // Architecture-dependent parsing support
   ////////////////////////////////////////////////

    bool archIsUnparseable();
    bool archAvoidParsing();
    bool archNoRelocate();
    void archSetFrameSize(int frameSize);
    bool archGetMultipleJumpTargets( 
             BPatch_Set< Address >& targets,
             image_basicBlock *currBlk,
             InstrucIter &ah,
             pdvector< instruction >& allInstructions);
    bool archProcExceptionBlock(Address &catchStart, Address a);
#if !defined(cap_instruction_api)    
    bool archIsATailCall(InstrucIter &ah,
             pdvector< instruction >& allInstructions);
    bool archIsIndirectTailCall(InstrucIter &ah);
    bool archIsRealCall(InstrucIter &ah, bool &validTarget, bool
&simulateJump);
    bool archCheckEntry(InstrucIter &ah, image_func *func );
    bool archIsIPRelativeBranch(InstrucIter& ah);
#endif    
    void archInstructionProc(InstructionAdapter &ah);
    /*void processJump(InstructionAdapter& ah,
		image_basicBlock* currBlk,
		Address& funcBegin,
		Address& funcEnd,
		pdvector<instruction>& allInstructions,
		BPatch_Set<Address>& leaders,
		pdvector<Address>& worklist,
		dictionary_hash<Address, image_basicBlock*>& leadersToBlock,
    dictionary_hash<Address, std::string> *pltFuncs);*/

   
   ////////////////////////////////////////////////
   // Instpoints!
   ////////////////////////////////////////////////

   const pdvector<image_instPoint*> &funcEntries();
   const pdvector<image_instPoint*> &funcExits();
   const pdvector<image_instPoint*> &funcCalls();
  
   // Initiate parsing on this function
   bool parse();
 
   // Do most of the parsing work. Calls architecture-dependent routines
   // defined in image-<arch>.C
   bool buildCFG(pdvector<image_basicBlock *> &funcEntry, Address funcBegin);

   bool isTrapFunc() const {return isTrap;}

   const pdvector<image_parRegion*> &parRegions();

   bool isInstrumentable() const { return instLevel_ != UNINSTRUMENTABLE; }
   InstrumentableLevel instLevel() const { return instLevel_; }

   void addCallInstPoint(image_instPoint *p);
   void addExitInstPoint(image_instPoint *p);

   // ----------------------------------------------------------------------

   ////////////////////////////////////////////////
   // Misc
   ////////////////////////////////////////////////

   codeRange *copy() const;

    struct compare {
        bool operator()(image_func * const &f1,
                        image_func * const &f2) const {
            return (f1->getOffset() < f2->getOffset());
        }
    };

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
   
   // Place to store the results of doFloatingPointStaticAnalysis().
   // This way, if they are ever needed in a mini-tramp, emitFuncJump()
   // for example, the expensive operation doesn't need to happen again.
   bool * usedFPregs;
   
   // Since the IA-64 ABI does not define a frame pointer register,
   // we use DWARF debug records (DW_AT_frame_base entries) to 
   // construct an AST which calculates the frame pointer.
   int getFramePointerCalculator();
#endif

   // Ifdef relocation... but set at parse time.
   bool canBeRelocated() const { return canBeRelocated_; }
   bool needsRelocation() const { return needsRelocation_; }
   void markAsNeedingRelocation(bool foo) { needsRelocation_ = foo; }

   bool containsSharedBlocks() const { return containsSharedBlocks_; }

   image_basicBlock * entryBlock();

   /****** OpenMP Parsing Functions *******/
   std::string calcParentFunc(const image_func * imf, pdvector<image_parRegion *> & pR);
   void parseOMP(image_parRegion * parReg, image_func * parentFunc, int & currentSectionNum);
   void parseOMPSectFunc(image_func * parentFunc);
   void parseOMPFunc(bool hasLoop);
   bool parseOMPParent(image_parRegion * iPar, int desiredNum, int & currentSectionNum);
   void addRegion(image_parRegion * iPar) { parRegionsList.push_back(iPar); }
   bool OMPparsed() { return OMPparsed_; }
   bool isPLTFunction() { return isPLTFunction_; }
   /****************************************/

   bool parsed() { return parsed_; }

   // Not completely implemented, and so commented out.
   std::set<Register> * usedGPRs() { calcUsedRegs(); return &(usedRegisters->generalPurposeRegisters);}
   std::set<Register> * usedFPRs() { calcUsedRegs(); return &(usedRegisters->floatingPointRegisters);}

   bool isLeafFunc();

   bool writesFPRs(unsigned level = 0);
   bool writesSPRs(unsigned level = 0);

#if defined(cap_liveness)
   void calcBlockLevelLiveness();
#endif

   FuncSource howDiscovered() const { return howDiscovered_; }
   
   const Function *func() const { return func_; }

   bool containsBlock(image_basicBlock *);

 private:
     void markBlockEnd(image_basicBlock* curBlock,
                       InstructionAdapter& ah,
                       Address& funcEnd);
     bool isNonReturningCall(image_func* targetFunc,
                             bool isInPLT,
                             std::string pltEntryForTarget,
                             Address currAddr,
                             Address target);
   
   void calcUsedRegs();/* Does one time calculation of registers used in a function, if called again
                          it just refers to the stored values and returns that */

   ///////////////////// Basic func info
   Function *func_;			/* pointer to the underlying symtab Function */

   Address endOffset_;          /* Address of the (next instruction after) the end of the func */
   pdmodule *mod_;		/* pointer to file that defines func. */
   image *image_;
   bool parsed_;                /* Set to true in findInstPoints */
   bool OMPparsed_;              /* Set true in parseOMPFunc */
   bool cleansOwnStack_;

   /////  Variables for liveness Analysis
   image_func_registers * usedRegisters;// container class for all the registers the function uses

   enum regUseState { unknown, used, unused };

   regUseState containsFPRWrites_;   // does this function have floating point write instructions
   regUseState containsSPRWrites_;   // Does this function write to SPRs.


   ///////////////////// CFG and function body
   set<image_basicBlock*, image_basicBlock::compare> blockList;

   bool noStackFrame; // formerly "leaf".  True iff this fn has no stack frame.
   bool makesNoCalls_;
   bool savesFP_;

   bool containsSharedBlocks_;  // True if one or more blocks in this
                                // function are shared with another function.

   FuncReturnStatus retStatus_; // Does this function return or not?

   // Bind a call target to the current basic block
   // May initiate recursive parsing of the target function
   image_func* bindCallTarget(Address target, image_basicBlock * curBlk);

    // Find an existing image_func object or create a new one
   image_func* FindOrCreateFunc(Address target, FuncSource src, bool & created);

   
   ///////////////////// Instpoints 

   pdvector<image_instPoint*> funcEntries_;     /* place to instrument entry 
                                                   (often not addr) */
   pdvector<image_instPoint*> funcReturns;	/* return point(s). */
   pdvector<image_instPoint*> calls;		/* pointer to the calls */

   //  OpenMP (and other parallel language) support
   pdvector<image_parRegion*> parRegionsList; /* vector of all parallel regions within function */
   // End OpenMP support



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

#if defined(cap_liveness)
   bool livenessCalculated_;
#endif
   bool isPLTFunction_;

   // How we discovered this function (e.g., from symbol, RT, etc)
   FuncSource howDiscovered_;
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
