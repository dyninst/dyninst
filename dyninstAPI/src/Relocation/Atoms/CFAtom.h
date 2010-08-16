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

#if !defined (_R_E_CONTROL_FLOW_H_)
#define _R_E_CONTROL_FLOW_H_

#include "Atom.h"

namespace Dyninst {
namespace Relocation {
 class LocalizeCF;
 class TargetInt;
 class Instrumenter;
 class adhocMovementTransformer;
 class Fallthroughs;
 class Modification;

class CFAtom : public Atom {
  friend class Transformer;
  friend class LocalizeCF;
  friend class Instrumenter; // For rewiring edge instrumentation
  friend class adhocMovementTransformer; // Also
  friend class PCSensitiveTransformer;
  friend class Fallthroughs;
  friend class Modification;
 public:
  static const Address Fallthrough;
  static const Address Taken;

  typedef dyn_detail::boost::shared_ptr<CFAtom> Ptr;
  typedef std::map<Address, TargetInt *> DestinationMap;

  bool generate(GenStack &);

  // Factory function... we create these first,
  // then fill them in.
  static Ptr create();
		    
  void updateInsn(InstructionAPI::Instruction::Ptr insn);
  void updateAddr(Address addr);

  void setCall() { isCall_ = true; };
  void setConditional() { isConditional_ = true; };
  void setIndirect() { isIndirect_ = true; };
  void setNeedsFTPadding() { padded_ = true; };

  virtual ~CFAtom();

  // Owns the provided *dest parameter
  void addDestination(Address index, TargetInt *dest);

  virtual std::string format() const;

  virtual Address addr() const { return addr_; }
  virtual InstructionAPI::Instruction::Ptr insn() const { return insn_; }
  virtual unsigned size() const { return insn_->size(); }

 private:
  CFAtom() :
  isCall_(false),
    isConditional_(false),
    isIndirect_(false),
    padded_(false),
    addr_(0) {};

  bool isCall_;
  bool isConditional_;
  bool isIndirect_;
  bool padded_;

  InstructionAPI::Instruction::Ptr insn_;
  Address addr_;

  // An expression that represents how the PC is determined
  // Should be a single register, but who are we to judge?
  InstructionAPI::Expression::Ptr targetExpr_;

  // A map from input values (for some representation of input
  // values) to Targets
  // Used during code generation to determine whether we
  // require some form of address translation. We currently have
  // two cases: conditional and indirect control flow.
  //  Conditional: <true> -> taken target; <false> -> fallthrough target
  //  Indirect: <original address> -> corresponding target
  // TBD: PPC has conditional indirect control flow, so we may want
  // to split these up.
  DestinationMap destMap_;

  //
  // These should move to a CodeGenerator class or something...
  // But for now they can go here
  // The Instruction input allows pulling out ancillary data (e.g.,
  // conditions, prediction, etc.
  bool generateBranch(GenStack &gens,
		      TargetInt *to,
		      InstructionAPI::Instruction::Ptr insn,
		      bool fallthrough);

  bool generateCall(GenStack &gens,
		    TargetInt *to,
		    InstructionAPI::Instruction::Ptr insn); 

  bool generateConditionalBranch(GenStack &gens,
				 TargetInt *to,
				 InstructionAPI::Instruction::Ptr insn); 
  // The Register holds the translated destination (if any)
  // TODO replace with the register IDs that Bill's building
  typedef unsigned Register;
  bool generateIndirect(GenStack &gens,
			Register reg,
			InstructionAPI::Instruction::Ptr insn);
  bool generateIndirectCall(GenStack &gens,
			    Register reg,
			    InstructionAPI::Instruction::Ptr insn,
			    Address origAddr);
  
  bool generateAddressTranslator(codeGen &gen,
				 Register &reg);  
 };

struct CFPatch : public Patch {
  // What type of patch are we?
  typedef enum {
    Jump,
    JCC, 
    Call,
    Data } Type;
  // Data: RIP-relative expression for the destination

 CFPatch(Type a, InstructionAPI::Instruction::Ptr b, TargetInt *c,
	 bool d = false, Address e = 0) :
  type(a), orig_insn(b), target(c),
    postCFPadding_(d), origAddr_(e) {};
  
  virtual bool apply(codeGen &gen, int iteration, int shift);
  virtual bool preapply(codeGen &gen);
  virtual ~CFPatch() {};

  Type type;
  InstructionAPI::Instruction::Ptr orig_insn;
  TargetInt *target;
  bool postCFPadding_;
  Address origAddr_;
  
};

};
};
#endif
