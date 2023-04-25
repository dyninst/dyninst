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

#ifndef _BPatch_instruction_h_
#define _BPatch_instruction_h_

#include "BPatch_dll.h"
#include "dyntypes.h"
#include <string>

class BPatch_basicBlock;
class BPatch_point;

class internal_instruction;

#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_instruction

class BPATCH_DLL_EXPORT BPatch_instruction {
  friend class BPatch_basicBlock;

 public:
  // maximum number of memory accesses per instruction; platform dependent
   static const unsigned int nmaxacc_NP;

 protected:

  unsigned int nacc;
  internal_instruction *insn_;
  bool *isLoad;
  bool *isStore;
  int *preFcn;       // prefetch function (-1 = none)
  int *condition;    // -1 means no condition, all other values are machine specific
                                // conditions, currently (8/13/02) the tttn field on x86
  bool *nonTemporal; // non-temporal (cache non-polluting) write on x86

  BPatch_basicBlock *parent;
  long unsigned int addr;
 public:

  BPatch_instruction(internal_instruction *insn,
	  Dyninst::Address _addr);
  virtual ~BPatch_instruction();

  void getInstruction(const unsigned char *&_buffer, unsigned char &_length);

  internal_instruction *insn();

  // Not yet implemented
  char *getMnemonic() const { return NULL; }

  BPatch_point * getInstPoint();

  BPatch_basicBlock * getParent();
  void * getAddress();
 public:

  bool equals(const BPatch_instruction* mp) const { return mp ? equals(*mp) : false; }

  bool equals(const BPatch_instruction& rp) const
  {
    bool res;

    res = 
      (isLoad == rp.isLoad) &&
      (isStore == rp.isStore) &&
      (preFcn == rp.preFcn) &&
      (condition == rp.condition) && 
      (nonTemporal == rp.nonTemporal);

    return res;
  }

  bool hasALoad() const { return nacc == 1 ? isLoad[0] : (isLoad[0] || isLoad[1]); }
  bool hasAStore() const { return nacc == 1 ? isStore[0] : (isStore[0] || isStore[1]); }
  bool hasAPrefetch_NP() const { return preFcn[0] >= 0; }

  unsigned int getNumberOfAccesses() const { return nacc; }
  bool isALoad(int which = 0) const { return isLoad[which]; }
  bool isAStore(int which = 0) const { return isStore[which]; }
  bool isAPrefetch_NP(int which = 0) const { return preFcn[which] >= 0; }
  bool isConditional_NP(int which = 0) const { return condition[which] >= 0; }
  bool isNonTemporal_NP(int which = 0) const { return nonTemporal[which]; }
  int  prefetchType_NP(int which = 0) const { return preFcn[which]; }
  int  conditionCode_NP(int which = 0) const { return condition[which]; }
};

class BPATCH_DLL_EXPORT BPatch_branchInstruction : public BPatch_instruction{
    friend class BPatch_basicBlock;

 public:
    BPatch_branchInstruction(internal_instruction *insn,
                             long unsigned int _addr,
                             void *target) :
        BPatch_instruction(insn, _addr),
        target_(target) {}

    ~BPatch_branchInstruction() {}

    void *getTarget() { return target_; }

 protected:

    void *target_;
};


class BPatch_registerExpr;
class BPatch_addressSpace;
class BPatch_point;

class BPATCH_DLL_EXPORT BPatch_register {
    friend class BPatch_registerExpr;
    friend class BPatch_addressSpace;
    friend class BPatch_point;

 public:
    std::string name() const;
    
 private:
    BPatch_register(std::string n,
                    int e) :
        name_(n),
        number_(e) {}
    
    std::string name_;
    int number_;
};

#endif
