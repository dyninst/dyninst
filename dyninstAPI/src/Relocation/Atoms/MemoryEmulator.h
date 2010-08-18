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

#include "Atom.h"


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

   virtual bool generate(GenStack &);

   virtual TrackerElement *tracker() const;

   virtual ~MemEmulator() {};
   virtual std::string format() const;

   virtual Address addr() const { return addr_; }
   virtual unsigned size() const { return insn_->size(); }

   Register effAddr() const { return effAddr_; }

 private:
   // TODO the compare should be a functor of some sort
   MemEmulator(InstructionAPI::Instruction::Ptr insn,
	       Address addr,
	       instPoint *point) :
   insn_(insn), 
     addr_(addr),
     point_(point),
     effAddr_(Null_Register),
     saveFlags_(false),
     saveOF_(false),
     saveOthers_(false),
     saveRAX_(false),
     RAXWritten_(false),
     RAXSave_(Null_Register),
     flagSave_(Null_Register)
     {};

   bool initialize(codeGen &gen);

   bool checkLiveFlags(codeGen &gen);
   
   bool allocRegisters(codeGen &gen);

   bool calcWriteSet(pdvector<Register> &excluded, bool);
   
   bool computeEffectiveAddress(codeGen &gen);

   bool saveFlags(codeGen &gen);


   bool restoreFlags(codeGen &gen);

   bool generateOrigAccess(codeGen &gen); 

   /*
   bool generateJA(codeGen &gen,
		  codeBufIndex_t from,
		  codeBufIndex_t to);
   */

   bool createStackFrame(codeGen &gen);
   bool destroyStackFrame(codeGen &gen);
   bool moveRegister(Register from, Register to, codeGen &gen);

   InstructionAPI::Instruction::Ptr insn_;
   Address addr_;
   instPoint *point_;

   Register effAddr_;
   bool saveFlags_;
   bool saveOF_;
   bool saveOthers_;
   bool saveRAX_;
   bool RAXWritten_;
   Register RAXSave_;

   Register flagSave_;

   static TranslatorMap translators_;
};

// A utility class that packages up the stream of compare/branch/arithmetic
// used above. This lets us outline the code.
class MemEmulatorTranslator : public Atom {
 public:
    typedef dyn_detail::boost::shared_ptr<MemEmulatorTranslator> Ptr;
    static Ptr create(Register r);
    virtual bool generate(GenStack &);
    virtual TrackerElement *tracker() const;

    virtual ~MemEmulatorTranslator() {};
    virtual std::string format() const;
    
  private:
  MemEmulatorTranslator(Register r) : reg_(r) {};

    bool generateReturn(codeGen &gen);

    Register reg_;
};


};
};
#endif
