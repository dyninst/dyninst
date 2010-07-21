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
#include "common/h/arch.h" // instruction
#include "codeRange.h"
#include "parRegion.h"
#include "common/h/Dictionary.h"
#include "symtabAPI/h/Symbol.h"
#include "dyninstAPI/src/bitArray.h"
#include "InstructionCache.h"
#include <set>

#include "Parsing.h"

#include "symtabAPI/h/Function.h"

#include <queue>

using namespace Dyninst;

class pdmodule;

class image_basicBlock;
class image_instPoint;
class image_edge;

#if !defined(ESSENTIAL_PARSING_ENUMS)
#define ESSENTIAL_PARSING_ENUMS
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

class image_basicBlock : public codeRange, public ParseAPI::Block  {
    friend class image_func;
    friend class DynCFGFactory;
 private:
    image_basicBlock(ParseAPI::CodeObject *, ParseAPI::CodeRegion*, Address);
 public:
    image_basicBlock(image_func*,ParseAPI::CodeRegion*,Address);

    // just pass through to Block
    Address firstInsnOffset() const;
    Address lastInsnOffset() const;
    Address endOffset() const;
    Address getSize() const;

    // cfg access & various predicates 
    bool isShared() const { return containingFuncs() > 1; }
    bool isExitBlock();
    bool isEntryBlock(image_func * f) const;
    image_func *getEntryFunc() const;  // func starting with this bock

    // misc utility
    int id() const { return blockNumber_; }
    void debugPrint();
    image *img();

    // instrumentation-related
    bool canBeRelocated() const { return canBeRelocated_; }
    bool needsRelocation() const { return needsRelocation_; }
    void markAsNeedingRelocation() { needsRelocation_ = true; }

    // codeRange implementation
    void *getPtrToInstruction(Address addr) const;
    Address get_address() const { return firstInsnOffset(); }
    unsigned get_size() const { return getSize(); }

    // etc.
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

#if defined(cap_liveness)
    const bitArray &getLivenessIn(image_func * context);
    // This is copied from the union of all successor blocks
    const bitArray getLivenessOut(image_func * context);
#endif

#if defined(cap_instruction_api)
    void getInsnInstances(std::vector<std::pair<InstructionAPI::Instruction::Ptr, Offset> > &instances);
#endif

 private:
    bool needsRelocation_;
    int blockNumber_;
    bool canBeRelocated_; // some blocks contain uninstrumentable constructs

#if defined(cap_liveness)
    /* Liveness analysis variables */
    /** gen registers */
    
    bitArray use; // Registers used by instructions within the block
    bitArray def; // Registers defined by instructions within the block
    bitArray in;  // Summarized input liveness; we calculate output on the fly
 public:
    static InstructionCache cachedLivenessInfo;
    
 private:
    void summarizeBlockLivenessInfo(image_func * context);
    // Returns true if any information changed; false otherwise
    bool updateBlockLivenessInfo(image_func * context);
#endif


};

inline Address 
image_basicBlock::firstInsnOffset() const {
    return ParseAPI::Block::start(); 
}
inline Address 
image_basicBlock::lastInsnOffset() const {
    return ParseAPI::Block::lastInsnAddr();
}
inline Address 
image_basicBlock::endOffset() const {
    return ParseAPI::Block::end();
}
inline Address 
image_basicBlock::getSize() const {
    return ParseAPI::Block::size();
}

void checkIfRelocatable (instruction insn, bool &canBeRelocated);

class image_edge : public ParseAPI::Edge {
    friend class image_basicBlock;
 public:
    image_edge(image_basicBlock *source, 
               image_basicBlock *target, 
               EdgeTypeEnum type) :
    ParseAPI::Edge(source,target,type)
   { }

    // MSVC++ 2003 does not properly support covariant return types
    // in overloaded methods
#if !defined _MSC_VER || _MSC_VER > 1310 
   virtual image_basicBlock * src() const { return (image_basicBlock*)_source; }
   virtual image_basicBlock * trg() const { return (image_basicBlock*)_target; }
#endif

   const char * getTypeString();
};

#include "ast.h"
class image_func_registers {
 public:
  std::set<Register> generalPurposeRegisters;
  std::set<Register> floatingPointRegisters;
  std::set<Register> specialPurposeRegisters;
};

class image_func : public ParseAPI::Function
{
  friend class DynCFGFactory;
  friend class DynParseCallback;
  public:
   /* Annotatable requires a default constructor */
   image_func() { }
  public:
   image_func(SymtabAPI::Function *func, 
        pdmodule *m, 
        image *i, 
        ParseAPI::CodeObject * obj,
        ParseAPI::CodeRegion * reg,
        InstructionSource * isrc,
        FuncSource src);

   ~image_func();

   SymtabAPI::Function* getSymtabFunction() const{
      return  func_; 
   }	

   /*** Function naming ***/
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
   // return true if the name is new (and therefore added)
   bool addSymTabName(std::string name, bool isPrimary = false);
   bool addPrettyName(std::string name, bool isPrimary = false);
   bool addTypedName(std::string name, bool isPrimary = false);

   /*** Location queries ***/
   Address getOffset() const;
   Address getPtrOffset() const;
   unsigned getSymTabSize() const;
   Address getEndOffset(); // May trigger parsing

   void *getPtrToInstruction(Address addr) const;

   /*** Debugging output operators ***/
   ostream & operator<<(ostream &s) const;
   friend ostream &operator<<(ostream &os, image_func &f);

   /*** misc. accessors ***/
   pdmodule *pdmod() const { return mod_;}
   image *img() const { return image_; }
   void changeModule(pdmodule *mod);

   ////////////////////////////////////////////////
   // CFG and other function body methods
   ////////////////////////////////////////////////

   bool makesNoCalls();

   ////////////////////////////////////////////////
   // Instpoints!
   ////////////////////////////////////////////////

   void funcEntries(pdvector<image_instPoint*> &);
   void funcExits(pdvector<image_instPoint*> &);
   void funcCalls(pdvector<image_instPoint*> &);
   // statically unresolvable control flow points
   void funcUnresolvedControlFlow(pdvector<image_instPoint*> &);
   void funcAbruptEnds(pdvector<image_instPoint*> &); 
  
   // Initiate parsing on this function
   bool parse();
 
   const pdvector<image_parRegion*> &parRegions();

   bool isInstrumentable() const { return instLevel_ != UNINSTRUMENTABLE; }
   InstrumentableLevel instLevel() const { return instLevel_; }
   void setInstLevel(InstrumentableLevel l) { instLevel_ = l; }

   // ----------------------------------------------------------------------


   ///////////////////////////////////////////////////
   // Mutable function code, used for hybrid analysis
   ///////////////////////////////////////////////////

   static void getUnreachableBlocks( std::set<image_basicBlock*> &deadBlocks, 
                                     std::set<image_basicBlock*> &unreachable );
   ParseAPI::FuncReturnStatus init_retstatus() { return init_retstatus_; }
   void setinit_retstatus(ParseAPI::FuncReturnStatus rs) { init_retstatus_ = rs; }

   // ----------------------------------------------------------------------


   ////////////////////////////////////////////////
   // Misc
   ////////////////////////////////////////////////

    struct compare {
        bool operator()(image_func * const &f1,
                        image_func * const &f2) const {
            return (f1->getOffset() < f2->getOffset());
        }
    };

#if defined(arch_x86) || defined(arch_x86_64)
   bool isTrueCallInsn(const instruction insn);
#endif

#if defined(sparc_sun_solaris2_4)
   bool is_o7_live(){ return o7_live; }
#endif

#if defined(arch_power)
   bool savesReturnAddr() const { return ppc_saves_return_addr_; }
#endif

   // Ifdef relocation... but set at parse time.
   bool canBeRelocated() const { return canBeRelocated_; }

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
   /****************************************/
   bool isPLTFunction();

   std::set<Register> * usedGPRs() { calcUsedRegs(); return &(usedRegisters->generalPurposeRegisters);}
   std::set<Register> * usedFPRs() { calcUsedRegs(); return &(usedRegisters->floatingPointRegisters);}

   bool isLeafFunc();

   bool writesFPRs(unsigned level = 0);
   bool writesSPRs(unsigned level = 0);

#if defined(cap_liveness)
   void calcBlockLevelLiveness();
#endif

   const SymtabAPI::Function *func() const { return func_; }

 private:
   void calcUsedRegs();/* Does one time calculation of registers used in a function, if called again
                          it just refers to the stored values and returns that */

   ///////////////////// Basic func info
   SymtabAPI::Function *func_;			/* pointer to the underlying symtab Function */

   pdmodule *mod_;		/* pointer to file that defines func. */
   image *image_;
   bool OMPparsed_;              /* Set true in parseOMPFunc */

   /////  Variables for liveness Analysis
   enum regUseState { unknown, used, unused };
   image_func_registers * usedRegisters;
   regUseState containsFPRWrites_;   // floating point registers
   regUseState containsSPRWrites_;   // stack pointer registers

   ///////////////////// CFG and function body
   bool containsSharedBlocks_;  // True if one or more blocks in this
                                // function are shared with another function.

   //  OpenMP (and other parallel language) support
   pdvector<image_parRegion*> parRegionsList; /* vector of all parallel regions within function */
    void addParRegion(Address begin, Address end, parRegType t);
   // End OpenMP support

   InstrumentableLevel instLevel_;   // the degree of freedom we have in
                                    // instrumenting the function
   
   bool canBeRelocated_;           // True if nothing prevents us from
                                   // relocating function
   bool needsRelocation_;          // Override -- "relocate this func"

   // Some functions are known to be unparesable by name
   bool isInstrumentableByFunctionName();

   ParseAPI::FuncReturnStatus init_retstatus_;

   // Architecture specific data
   bool o7_live;
   bool ppc_saves_return_addr_;

#if defined(cap_liveness)
   bool livenessCalculated_;
#endif
   bool isPLTFunction_;
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

inline Address 
image_func::getOffset() const {
    return ParseAPI::Function::addr();
}
inline Address 
image_func::getPtrOffset() const {
    return func_->getFirstSymbol()->getPtrOffset();
}
inline unsigned 
image_func::getSymTabSize() const { 
    return func_->getFirstSymbol()->getSize();
}



#endif /* FUNCTION_H */
