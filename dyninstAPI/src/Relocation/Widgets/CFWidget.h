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

#if !defined (_R_E_CONTROL_FLOW_H_)
#define _R_E_CONTROL_FLOW_H_

#include <map>
#include <string>
#include "Widget.h"

class block_instance;
class func_instance;
class instPoint;

namespace NS_x86 {
   class instruction;
}

namespace NS_power {
  class instruction;
}

namespace NS_aarch64 {
  class instruction;
}

#if defined(arch_x86) || defined(arch_x86_64)
typedef NS_x86::instruction arch_insn;
#elif defined (arch_power) 
typedef NS_power::instruction arch_insn;
#elif defined (arch_aarch64)
typedef NS_aarch64::instruction arch_insn;
#else
#error "Unknown architecture"
#endif

namespace Dyninst {
namespace Relocation {
 class LocalizeCF;
 class TargetInt;
 class Instrumenter;
 class adhocMovementTransformer;
 class Fallthroughs;
 class Modification;
 class RelocBlock;

class CFWidget : public Widget {
  friend class Transformer;
  friend class LocalizeCF;
  friend class Instrumenter; // For rewiring edge instrumentation
  //friend class adhocMovementTransformer; // Also
  //friend class PCSensitiveTransformer;
  friend class Modification;
  friend class RelocBlock;
 public:
  static const Address Fallthrough;
  static const Address Taken;

  typedef boost::shared_ptr<CFWidget> Ptr;
  typedef std::map<Address, TargetInt *> DestinationMap;

  static Ptr create(Address addr);
  static Ptr create(const Widget::Ptr info);

  bool generate(const codeGen &templ,
                const RelocBlock *,
                CodeBuffer &buffer);

  virtual ~CFWidget();

  // Owns the provided *dest parameter
  void addDestination(Address index, TargetInt *dest);
  TargetInt *getDestination(Address dest) const;
  const DestinationMap &destinations() const { return destMap_; }

  virtual std::string format() const;

  virtual Address addr() const { return addr_; }
  virtual InstructionAPI::Instruction insn() const { return insn_; }

  void setGap(unsigned gap) { gap_ = gap; }
  void setOrigTarget(Address a) { origTarget_ = a; }
  unsigned gap() const { return gap_; }
  void clearIsCall() { isCall_ = false; }
  void clearIsIndirect() { isIndirect_ = false; }
  void clearIsConditional() { isConditional_ = false; }
  bool isCall() const { return isCall_; }
  bool isIndirect() const { return isIndirect_; }
  bool isConditional() const { return isConditional_; }

 private:
   CFWidget(Address a)
      : isCall_(false),
     isConditional_(false),
     isIndirect_(false),
     gap_(0),
     addr_(a), 
     origTarget_(0) {}

   CFWidget(InstructionAPI::Instruction insn,
          Address addr);

   TrackerElement *tracker(const RelocBlock *) const;
   TrackerElement *destTracker(TargetInt *dest, const RelocBlock *) const;
   TrackerElement *addrTracker(Address addr, const RelocBlock *) const;
   TrackerElement *padTracker(Address addr, unsigned size, const RelocBlock *) const;
   

  // These are not necessarily mutually exclusive. See also:
  // PPC conditional linking indirect branch, oy. 
  bool isCall_;
  bool isConditional_;
  bool isIndirect_;

  unsigned gap_;

  InstructionAPI::Instruction insn_;
  Address addr_;

  // If we were a PC-relative indirect store that data here
  Address origTarget_;

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

  // These should move to a CodeGenerator class or something...
  // But for now they can go here
  // The Instruction input allows pulling out ancillary data (e.g.,
  // conditions, prediction, etc.
  bool generateBranch(CodeBuffer &gens,
					  TargetInt *to,
					  InstructionAPI::Instruction insn,
					  const RelocBlock *trace,
					  bool fallthrough);

  bool generateCall(CodeBuffer &gens,
					TargetInt *to,
					const RelocBlock *trace,
					InstructionAPI::Instruction insn);

  bool generateConditionalBranch(CodeBuffer &gens,
								 TargetInt *to,
								 const RelocBlock *trace,
								 InstructionAPI::Instruction insn);
  // The Register holds the translated destination (if any)
  // TODO replace with the register IDs that Bill's building
  typedef unsigned Register;
  bool generateIndirect(CodeBuffer &gens,
						Register reg,
						const RelocBlock *trace,
						InstructionAPI::Instruction insn);
  bool generateIndirectCall(CodeBuffer &gens,
							Register reg,
							InstructionAPI::Instruction insn,
							const RelocBlock *trace,
							Address origAddr);
};

struct CFPatch : public Patch {
  // What type of patch are we?
  typedef enum {
    Jump,
    JCC, 
    Call,
    Data } Type;
  // Data: RIP-relative expression for the destination

 CFPatch(Type a,
		 InstructionAPI::Instruction b,
		 TargetInt *c,
		 const func_instance *d,
		 Address e = 0);
  
  virtual bool apply(codeGen &gen, CodeBuffer *buf);
  virtual unsigned estimate(codeGen &templ);
  virtual ~CFPatch();

  Type type;
  InstructionAPI::Instruction orig_insn;
  TargetInt *target;
  const func_instance *func;
  Address origAddr_;  
  arch_insn *ugly_insn;
  unsigned char* insn_ptr;


#if defined(arch_power)
  // 64-bit PPC/Linux has a TOC register we need
  // to maintain. That puts it in "special case"
  // territory...
  bool needsTOCUpdate();
  bool handleTOCUpdate(codeGen &gen);
#endif

  private:
  bool isPLT(codeGen &gen);
  bool applyPLT(codeGen &gen, CodeBuffer *buf);



};

struct PaddingPatch : public Patch {
  // For Kevin's defensive Dyninst, we want to append a
  // padding area past the return point of calls that don't
  // necessarily return to the normal places. This requires
  // both a) an empty space in code gen and b) tracking that
  // address in the process. The first is easy enough to
  // do statically, but the second requires a patch so that
  // we get notified of address finickiness.

   PaddingPatch(unsigned size, bool registerDefensive, bool noop, block_instance *b);
   virtual bool apply(codeGen &gen, CodeBuffer *buf);
   virtual unsigned estimate(codeGen &templ);
   virtual ~PaddingPatch() {}
   
   unsigned size_;
   bool registerDefensive_;
   bool noop_;
   block_instance *block_;
};

}
}
#endif
