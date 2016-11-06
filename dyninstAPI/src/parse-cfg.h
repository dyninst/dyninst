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

// $Id: parse-cfg.h,v 1.37 2008/09/03 06:08:44 jaw Exp $

#ifndef IMAGE_FUNC_H
#define IMAGE_FUNC_H

#include <string>
#include "common/src/Vector.h"
#include "common/src/Types.h"
#include "common/src/Pair.h"
#include "common/src/arch.h"  // instruction
#include "codeRange.h"
#include "parRegion.h"
#include <unordered_map>
#include "symtabAPI/h/Symbol.h"
#include "bitArray.h"
#include "InstructionCache.h"
#include <set>

#include "Parsing.h"

#include "symtabAPI/h/Function.h"

#include <queue>

using namespace std;
using namespace Dyninst;

class pdmodule;

class parse_block;
class image_edge;

class parse_block : public codeRange, public ParseAPI::Block {
  friend class parse_func;
  friend class DynCFGFactory;

 private:
  parse_block(ParseAPI::CodeObject *, ParseAPI::CodeRegion *, Address);

 public:
  parse_block(parse_func *, ParseAPI::CodeRegion *, Address);
  ~parse_block();

  // just pass through to Block
  Address firstInsnOffset() const;
  Address lastInsnOffset() const;
  Address endOffset() const;
  Address getSize() const;

  // cfg access & various predicates
  bool isShared() const { return containingFuncs() > 1; }
  bool isExitBlock();
  bool isCallBlock();
  bool isIndirectTailCallBlock();
  bool isEntryBlock(parse_func *f) const;
  parse_func *getEntryFunc() const;  // func starting with this bock

  bool unresolvedCF() const { return unresolvedCF_; }
  bool abruptEnd() const { return abruptEnd_; }
  void setUnresolvedCF(bool newVal);
  void setAbruptEnd(bool newVal) { abruptEnd_ = newVal; }

  // misc utility
  int id() const { return blockNumber_; }
  void debugPrint();
  image *img();

  // Find callees
  parse_func *getCallee();
  // Returns the address of our callee (if we're a call block, of course)
  std::pair<bool, Address> callTarget();

  // instrumentation-related
  bool needsRelocation() const { return needsRelocation_; }
  void markAsNeedingRelocation() { needsRelocation_ = true; }

  // codeRange implementation
  void *getPtrToInstruction(Address addr) const;
  Address get_address() const { return firstInsnOffset(); }
  unsigned get_size() const { return getSize(); }

  // etc.
  struct compare {
    bool operator()(parse_block *const &b1, parse_block *const &b2) const {
      if (b1->firstInsnOffset() < b2->firstInsnOffset()) return true;
      if (b2->firstInsnOffset() < b1->firstInsnOffset()) return false;

      // XXX the remainder is debugging, and should be removed
      if (b1 != b2)
        fprintf(stderr, "error: two blocks (%p,%p) at 0x%lx\n", b1, b2,
                b1->firstInsnOffset());

      assert(b1 == b2);
      return false;
    }
  };
  typedef std::set<parse_block *, parse_block::compare> blockSet;

  const bitArray &getLivenessIn(parse_func *context);
  // This is copied from the union of all successor blocks
  const bitArray getLivenessOut(parse_func *context);

  typedef std::map<Offset, InstructionAPI::Instruction::Ptr> Insns;
  // The provided parameter is a magic offset to add to each instruction's
  // address; we do this to avoid a copy when getting Insns from block_instances
  void getInsns(Insns &instances, Address offset = 0);

 private:
  bool needsRelocation_;
  int blockNumber_;

  bool unresolvedCF_;
  bool abruptEnd_;

  /* Liveness analysis variables */
  /** gen registers */

  bitArray use;  // Registers used by instructions within the block
  bitArray def;  // Registers defined by instructions within the block
  bitArray in;   // Summarized input liveness; we calculate output on the fly
 public:
  static InstructionCache cachedLivenessInfo;

 private:
  void summarizeBlockLivenessInfo(parse_func *context);
  // Returns true if any information changed; false otherwise
  bool updateBlockLivenessInfo(parse_func *context);
};

inline Address parse_block::firstInsnOffset() const {
  return ParseAPI::Block::start();
}
inline Address parse_block::lastInsnOffset() const {
  return ParseAPI::Block::lastInsnAddr();
}
inline Address parse_block::endOffset() const { return ParseAPI::Block::end(); }
inline Address parse_block::getSize() const { return ParseAPI::Block::size(); }

void checkIfRelocatable(instruction insn, bool &canBeRelocated);

class image_edge : public ParseAPI::Edge {
  friend class parse_block;

 public:
  image_edge(parse_block *source, parse_block *target, EdgeTypeEnum type)
      : ParseAPI::Edge(source, target, type) {}

// MSVC++ 2003 does not properly support covariant return types
// in overloaded methods
#if !defined _MSC_VER || _MSC_VER > 1310
  virtual parse_block *src() const { return (parse_block *)_source; }
  virtual parse_block *trg() const { return (parse_block *)_target; }
#endif

  const char *getTypeString();
};

#include "ast.h"
class parse_func_registers {
 public:
  std::set<Register> generalPurposeRegisters;
  std::set<Register> floatingPointRegisters;
  std::set<Register> specialPurposeRegisters;
};

class parse_func : public ParseAPI::Function {
  enum UnresolvedCF { UNSET_CF, HAS_UNRESOLVED_CF, NO_UNRESOLVED_CF };

  friend class DynCFGFactory;
  friend class DynParseCallback;

 public:
  /* Annotatable requires a default constructor */
  parse_func() {}

 public:
  parse_func(SymtabAPI::Function *func, pdmodule *m, image *i,
             ParseAPI::CodeObject *obj, ParseAPI::CodeRegion *reg,
             InstructionSource *isrc, FuncSource src);

  ~parse_func();

  SymtabAPI::Function *getSymtabFunction() const { return func_; }

  /*** Function naming ***/
  string symTabName() const {
    return func_->getFirstSymbol()->getMangledName();
  }
  string prettyName() const { return func_->getFirstSymbol()->getPrettyName(); }
  string typedName() const { return func_->getFirstSymbol()->getTypedName(); }
  SymtabAPI::Aggregate::name_iter symtab_names_begin() const {
    return func_->mangled_names_begin();
  }
  SymtabAPI::Aggregate::name_iter symtab_names_end() const {
    return func_->mangled_names_end();
  }
  SymtabAPI::Aggregate::name_iter pretty_names_begin() const {
    return func_->pretty_names_begin();
  }
  SymtabAPI::Aggregate::name_iter pretty_names_end() const {
    return func_->pretty_names_end();
  }
  SymtabAPI::Aggregate::name_iter typed_names_begin() const {
    return func_->typed_names_begin();
  }
  SymtabAPI::Aggregate::name_iter typed_names_end() const {
    return func_->typed_names_end();
  }

  /*   vector<string> symTabNameVector() const {
      return func_->getAllMangledNames();
  }
  vector<string> prettyNameVector() const {
      return func_->getAllPrettyNames();
  }
  vector<string> typedNameVector() const {
      return func_->getAllTypedNames();
      }*/
  void copyNames(parse_func *duplicate);
  // return true if the name is new (and therefore added)
  bool addSymTabName(std::string name, bool isPrimary = false);
  bool addPrettyName(std::string name, bool isPrimary = false);
  bool addTypedName(std::string name, bool isPrimary = false);

  /*** Location queries ***/
  Address getOffset() const;
  Address getPtrOffset() const;
  unsigned getSymTabSize() const;
  Address getEndOffset();  // May trigger parsing

  void *getPtrToInstruction(Address addr) const;

  /*** misc. accessors ***/
  pdmodule *pdmod() const { return mod_; }
  image *img() const { return image_; }
  void changeModule(pdmodule *mod);

  ////////////////////////////////////////////////
  // CFG and other function body methods
  ////////////////////////////////////////////////

  bool makesNoCalls();

  ////////////////////////////////////////////////
  // Instpoints!
  ////////////////////////////////////////////////

  // Initiate parsing on this function
  bool parse();

  const pdvector<image_parRegion *> &parRegions();

  bool isInstrumentable();
  bool hasUnresolvedCF();

  // ----------------------------------------------------------------------

  ///////////////////////////////////////////////////
  // Mutable function code, used for hybrid analysis
  ///////////////////////////////////////////////////

  void getReachableBlocks(const std::set<parse_block *> &exceptBlocks,  // input
                          const std::list<parse_block *> &seedBlocks,   // input
                          std::set<parse_block *> &reachableBlocks);  // output
  ParseAPI::FuncReturnStatus init_retstatus()
      const;  // only call on defensive binaries
  void setinit_retstatus(ParseAPI::FuncReturnStatus rs);  // also sets retstatus
  bool hasWeirdInsns() { return hasWeirdInsns_; }  // true if we stopped the
  // parse at a weird instruction (e.g., arpl)
  void setHasWeirdInsns(bool wi);
  void setPrevBlocksUnresolvedCF(size_t newVal) {
    prevBlocksUnresolvedCF_ = newVal;
  };
  size_t getPrevBlocksUnresolvedCF() const { return prevBlocksUnresolvedCF_; };

  // ----------------------------------------------------------------------

  ////////////////////////////////////////////////
  // Misc
  ////////////////////////////////////////////////

  struct compare {
    bool operator()(parse_func *const &f1, parse_func *const &f2) const {
      return (f1->getOffset() < f2->getOffset());
    }
  };

#if defined(arch_x86) || defined(arch_x86_64)
  bool isTrueCallInsn(const instruction insn);
#endif

#if defined(arch_power)
  bool savesReturnAddr() const { return ppc_saves_return_addr_; }
#endif

  bool containsSharedBlocks() const { return containsSharedBlocks_; }

  parse_block *entryBlock();

  /****** OpenMP Parsing Functions *******/
  std::string calcParentFunc(const parse_func *imf,
                             pdvector<image_parRegion *> &pR);
  void parseOMP(image_parRegion *parReg, parse_func *parentFunc,
                int &currentSectionNum);
  void parseOMPSectFunc(parse_func *parentFunc);
  void parseOMPFunc(bool hasLoop);
  bool parseOMPParent(image_parRegion *iPar, int desiredNum,
                      int &currentSectionNum);
  void addRegion(image_parRegion *iPar) { parRegionsList.push_back(iPar); }
  bool OMPparsed() { return OMPparsed_; }
  /****************************************/
  bool isPLTFunction();

  std::set<Register> *usedGPRs() {
    calcUsedRegs();
    return &(usedRegisters->generalPurposeRegisters);
  }
  std::set<Register> *usedFPRs() {
    calcUsedRegs();
    return &(usedRegisters->floatingPointRegisters);
  }

  bool isLeafFunc();

  bool writesFPRs(unsigned level = 0);
  bool writesSPRs(unsigned level = 0);

  void invalidateLiveness() { livenessCalculated_ = false; }
  void calcBlockLevelLiveness();

  const SymtabAPI::Function *func() const { return func_; }

 private:
  void calcUsedRegs(); /* Does one time calculation of registers used in a
                          function, if called again
                          it just refers to the stored values and returns that
                          */

  ///////////////////// Basic func info
  SymtabAPI::Function *func_; /* pointer to the underlying symtab Function */

  pdmodule *mod_; /* pointer to file that defines func. */
  image *image_;
  bool OMPparsed_; /* Set true in parseOMPFunc */

  /////  Variables for liveness Analysis
  enum regUseState { unknown, used, unused };
  parse_func_registers *usedRegisters;
  regUseState containsFPRWrites_;  // floating point registers
  regUseState containsSPRWrites_;  // stack pointer registers

  ///////////////////// CFG and function body
  bool containsSharedBlocks_;  // True if one or more blocks in this
                               // function are shared with another function.

  //  OpenMP (and other parallel language) support
  pdvector<image_parRegion *>
      parRegionsList; /* vector of all parallel regions within function */
  void addParRegion(Address begin, Address end, parRegType t);
  // End OpenMP support

  bool hasWeirdInsns_;             // true if we stopped the parse at a
                                   // weird instruction (e.g., arpl)
  size_t prevBlocksUnresolvedCF_;  // num func blocks when calculated

  // Some functions are known to be unparesable by name
  bool isInstrumentableByFunctionName();
  UnresolvedCF unresolvedCF_;

  ParseAPI::FuncReturnStatus init_retstatus_;

  // Architecture specific data
  bool o7_live;
  bool ppc_saves_return_addr_;

  bool livenessCalculated_;
  bool isPLTFunction_;
};

typedef parse_func *ifuncPtr;

struct ifuncCmp {
  int operator()(const ifuncPtr &f1, const ifuncPtr &f2) const {
    if (f1->getOffset() > f2->getOffset()) return 1;
    if (f1->getOffset() < f2->getOffset()) return -1;
    return 0;
  }
};

inline Address parse_func::getOffset() const {
  return ParseAPI::Function::addr();
}
inline Address parse_func::getPtrOffset() const {
  return func_->getFirstSymbol()->getPtrOffset();
}
inline unsigned parse_func::getSymTabSize() const {
  return func_->getFirstSymbol()->getSize();
}

#endif /* FUNCTION_H */
