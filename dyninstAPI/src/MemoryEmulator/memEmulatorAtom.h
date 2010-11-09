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

#if !defined (_R_E_MEM_EMULATOR_H_)
#define _R_E_MEM_EMULATOR_H_

#include "dyninstAPI/src/Relocation/Atoms/Atom.h"
#include <stack>
class registerSlot;


namespace Dyninst {
namespace Relocation {

class MemEmulatorTranslator;

struct DecisionTree {
  Register effAddr_;

DecisionTree(Register a) : effAddr_(a) {};
  
  bool generate(codeGen &gen);


  codeBufIndex_t generateSkip(codeGen &gen);
  codeBufIndex_t generateOrig(codeGen &gen);
  codeBufIndex_t generateInst(codeGen &gen);
  codeBufIndex_t generateText(codeGen &gen);
  
  codeBufIndex_t generateCompare(codeGen &gen, Address comp);
  void generateJumps(codeGen &gen,
		     codeBufIndex_t target, 
		     vector<codeBufIndex_t> &sources);
  void generateJCC(codeGen &gen, 
		   codeBufIndex_t target, 
		   vector<codeBufIndex_t> &sources);
  
};

class MemEmulator : public Atom {
  friend class MemEmulatorTranslator;
  typedef std::map<Register, TracePtr> TranslatorMap;
 public:
   typedef dyn_detail::boost::shared_ptr<MemEmulator> Ptr;
   
   static Ptr create(InstructionAPI::Instruction::Ptr insn,
		     Address addr,
		     instPoint *point);

   static void initTranslators(TranslatorMap &t); 

   virtual bool generate(const codeGen &, const Trace *, CodeBuffer &);

   TrackerElement *tracker(int_function *) const;

   virtual ~MemEmulator() {};
   virtual std::string format() const;

   virtual Address addr() const { return addr_; }
   virtual unsigned size() const { return insn_->size(); }
   virtual InstructionAPI::Instruction::Ptr insn() const { return insn_; }

   Register effAddr() const { return effAddr_; }
   
 private:
   // TODO the compare should be a functor of some sort
   MemEmulator(InstructionAPI::Instruction::Ptr insn,
	       Address addr,
	       instPoint *point)
      : insn_(insn), 
      addr_(addr),
      point_(point),
      effAddr_(Null_Register),
      effAddr2_(Null_Register),
      saveFlags_(false),
      saveRAX_(false)
      {};

   bool generateViaModRM(const codeGen &gen, const Trace *, CodeBuffer &buffer);
   bool generateViaOverride(const codeGen &gen, const Trace *, CodeBuffer &buffer);

   bool initialize(codeGen &gen);
   bool checkLiveness(codeGen &gen);   
   bool setupFrame(bool, codeGen &gen);
   bool computeEffectiveAddress(codeGen &gen);
   bool teardownFrame(codeGen &gen);
   bool trailingTeardown(codeGen &gen);

   bool saveFlags(codeGen &gen);

   bool restoreFlags(codeGen &gen);

   bool preCallSave(codeGen &gen);
   bool emitCallToTranslator(CodeBuffer &buffer);
   bool postCallRestore(codeGen &gen);


   bool pushRegIfLive(registerSlot *reg, codeGen &gen);
   bool popRegIfSaved(registerSlot *reg, codeGen &gen);
   Address getTranslatorAddr(codeGen &gen, bool wantShift);
   
   bool generateOrigAccess(codeGen &gen); 

   bool stealEffectiveAddr(Register &ret, codeGen &gen);

   std::pair<bool, bool> getImplicitRegs(codeGen &gen);

   /*
   bool generateJA(codeGen &gen,
		  codeBufIndex_t from,
		  codeBufIndex_t to);
   */

   bool generateImplicit(const codeGen &templ, const Trace *t, CodeBuffer &buffer);

   InstructionAPI::Instruction::Ptr insn_;
   Address addr_;
   instPoint *point_;

   Register effAddr_;
   Register effAddr2_;
   bool saveFlags_;
   bool saveRAX_;
   
   std::stack<Register> externalSaved_;

   static TranslatorMap translators_;
};



// A utility class that packages up the stream of compare/branch/arithmetic
// used above. This lets us outline the code.
class MemEmulatorTranslator : public Atom {
 public:
    typedef dyn_detail::boost::shared_ptr<MemEmulatorTranslator> Ptr;
    static Ptr create(Register r);
    virtual bool generate(const codeGen &, const Trace *, CodeBuffer &); 
    virtual TrackerElement *tracker() const;

    virtual ~MemEmulatorTranslator() {};
    virtual std::string format() const;
    
  private:
  MemEmulatorTranslator(Register r) : reg_(r) {};

    bool generateReturn(codeGen &gen);

    Register reg_;
};

struct MemEmulatorPatch : public Patch {
   // Put in a call to the RTtranslateMemory
   // function
   MemEmulatorPatch(Register r,
                    Address d,
                    instPoint *p)
      : point(p), reg_(r), dest_(d) {};
   virtual bool apply(codeGen &gen, CodeBuffer *buf);
   virtual unsigned estimate(codeGen &) { return 7; };
   virtual ~MemEmulatorPatch() {};

   instPoint *point;
   Register reg_;
   Address dest_;
};

};
};
#endif
