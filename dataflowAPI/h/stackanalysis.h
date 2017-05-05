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

#if !defined(STACK_ANALYSIS_H)
#define STACK_ANALYSIS_H

#if !defined(_MSC_VER) && !defined(os_freebsd)
#include <values.h>
#endif

#include <list>
#include <map>
#include <set>
#include <string>

// To define StackAST
#include "DynAST.h"

#include "Absloc.h"
#include "dyntypes.h"
#include "dyn_regs.h"
#include "util.h"

// FreeBSD is missing a MINLONG and MAXLONG
#if defined(os_freebsd) 
#if defined(arch_64bit)
#define MINLONG INT64_MIN
#define MAXLONG INT64_MAX
#else
#define MINLONG INT32_MIN
#define MAXLONG INT32_MAX
#endif
#endif

// These are _NOT_ in the Dyninst namespace...
namespace Dyninst {
   namespace ParseAPI {
      class Function;
      class Block;
      class Edge;
   };
   namespace InstructionAPI {
      class Instruction;
      class Expression;
   };

class StackAnalysis {
public:
   typedef boost::shared_ptr<InstructionAPI::Instruction> InstructionPtr;
   typedef boost::shared_ptr<InstructionAPI::Expression> ExpressionPtr;

   class DATAFLOW_EXPORT Definition {
   public:
      typedef enum {TOP, BOTTOM, DEF} Type;
      Address addr;
      ParseAPI::Block *block;
      Absloc origLoc;
      Type type;

      Definition(ParseAPI::Block *b, Address a, Absloc l) : addr(a),
         block(b), origLoc(l), type(DEF) {}
      Definition(ParseAPI::Block *b, Absloc l) : addr(0), block(b),
         origLoc(l), type(DEF) {}
      Definition(Address a, Absloc l) : addr(a), block(NULL), origLoc(l),
         type(DEF) {}
      Definition() : addr(0), block(NULL), type(TOP) {}

      bool operator==(const Definition &other) const {
         // FIXME: To pass checks in StackAnalysis::summarize(), we consider
         // definitions equivalent as long as one is not BOTTOM and the other
         // something else.  This is not proper.
         //return type == other.type && block == other.block;
         if (type == BOTTOM && other.type != BOTTOM) return false;
         if (type != BOTTOM && other.type == BOTTOM) return false;
         return true;
      }

      bool operator<(const Definition &rhs) const {
         if (type == TOP) return false;
         if (type == BOTTOM && rhs.type == BOTTOM) return false;
         if (type == BOTTOM) return true;

         if (rhs.type == TOP) return true;
         if (rhs.type == BOTTOM) return false;

         // At this point we know both are DEFs
         return block < rhs.block;
      }

      std::string format() const;

      static Definition meet(const Definition &lhs, const Definition &rhs) {
         if (lhs.type == TOP) return rhs;
         if (rhs.type == TOP) return lhs;
         if (lhs == rhs) return rhs;
         Definition bottom;
         bottom.type = BOTTOM;
         return bottom;
      }
   };

   class DATAFLOW_EXPORT Height {
   public:
      typedef signed long Height_t;
      typedef enum {TOP, BOTTOM, HEIGHT} Type;

      static const Height_t uninitialized = MAXLONG;
      static const Height_t notUnique = MINLONG;
      static const Height bottom;
      static const Height top;

      Height(const Height_t h, const Type t = HEIGHT) : height_(h), type_(t) {}
      Height() : height_(uninitialized), type_(TOP) {}
        
      Height_t height() const { return height_; }
        
      // FIXME if we stop using TOP == MAXINT and BOT == MININT...
      bool operator<(const Height &rhs) const {
         return (height_ < rhs.height_);
      }

      bool operator>(const Height &rhs) const {
         return (height_ > rhs.height_);
      }

      bool operator<=(const Height &rhs) const {
         return (height_ <= rhs.height_);
      }

      bool operator>=(const Height &rhs) const {
         return (height_ >= rhs.height_);
      }

      Height &operator+= (const Height &other);
      Height &operator+=(const signed long &rhs);
      const Height operator+(const Height &rhs) const;
      const Height operator+(const signed long &rhs) const;
      const Height operator-(const Height &rhs) const;

      bool operator==(const Height &rhs) const {
         return type_ == rhs.type_ && height_ == rhs.height_;
      }

      bool operator!=(const Height &rhs) const {
         return !(*this == rhs);
      }
        
      std::string format() const {
         if (isTop()) return "TOP";
         if (isBottom()) return "BOTTOM";

         std::stringstream retVal;
         retVal << height_;
         return retVal.str();
      }
       friend std::ostream& operator<<(std::ostream& stream, const Height& c)
       {
          stream << c.format() << std::endl;
          return stream;
       }

      bool isBottom() const {
         return type_ == BOTTOM && height_ == notUnique;
      }

      bool isTop() const {
         return type_ == TOP && height_ == uninitialized;
      }

      static Height meet(const Height &lhs, const Height &rhs) {
         if (rhs == lhs) return rhs;
         if (rhs == top) return lhs;
         if (lhs == top) return rhs;
         return bottom;
      }

      static Height meet(std::set<Height> &ins) {
         if (ins.empty()) return top;

         // If there is a single element in the set return it.
         if (ins.size() == 1) return *ins.begin();

         // MEET (bottom, ...) == bottom
         // Since ins is sorted, if bottom is in the set it must
         // be the start element...
         if (*ins.begin() == bottom) return bottom;

         // MEET (N, top) == N
         if (ins.size() == 2 && *ins.rbegin() == top) {
            return *ins.begin();
         }

         // There are 2 or more elements; the last one is not top; therefore
         // there is a conflict, and we return bottom.
         return bottom;
      }

   private:
      Height_t height_;
      Type type_;
   };

   typedef std::pair<Definition, Height> DefHeight;
   DATAFLOW_EXPORT static bool isTopSet(const std::set<DefHeight> &s);
   DATAFLOW_EXPORT static bool isBottomSet(const std::set<DefHeight> &s);

   // We need to represent the effects of instructions. We do this in terms of
   // transfer functions. We recognize the following effects on the stack.
   //   * Offset by known amount: push/pop/etc.
   //   * Set to known value: leave
   //   * Copy the stack pointer to/from some Absloc.
   //
   // There are also:
   //   * Offset by unknown amount expressible in a range [l, h]
   //   * Set to unknown value expressible in a range [l, h]
   // which we don't handle yet.
   //
   // This gives us the following transfer functions.
   //   * Delta(RV, f, t, v) -> RV[f] += v;
   //   * Abs(RV, f, t, v) -> RV[f] = v;
   //   * Copy(RV, f, t, v) -> RV[t] = RV[f];
   //
   // In the implementations below, we provide f, t, v at construction time (as
   // they are fixed) and RV as a parameter. Note that a transfer function is a
   // function T : (RegisterVector, RegisterID, RegisterID, value) ->
   // (RegisterVector).
   typedef std::map<Absloc, std::set<DefHeight> > AbslocState;
   class DATAFLOW_EXPORT TransferFunc {
   public:
      typedef enum {TOP, BOTTOM, OTHER} Type;

      static const long uninitialized = MAXLONG;
      static const long notUnique = MINLONG;
      static const TransferFunc top;
      static const TransferFunc bottom;

      TransferFunc() :
         from(Absloc()), target(Absloc()), delta(0), abs(uninitialized),
         retop(false), topBottom(false), type_(TOP) {}
      TransferFunc(long a, long d, Absloc f, Absloc t, bool i = false,
         bool rt = false, Type type = OTHER) : from(f), target(t), delta(d),
         abs(a), retop(rt), topBottom(i), type_(type) {}
      TransferFunc(std::map<Absloc,std::pair<long,bool> > f, long d, Absloc t) :
         from(Absloc()), target(t), delta(d), abs(uninitialized), retop(false),
         topBottom(false), fromRegs(f), type_(OTHER) {}

      static TransferFunc identityFunc(Absloc r);
      static TransferFunc deltaFunc(Absloc r, long d);
      static TransferFunc absFunc(Absloc r, long a, bool i = false);
      static TransferFunc copyFunc(Absloc f, Absloc t, bool i = false);
      static TransferFunc bottomFunc(Absloc r);
      static TransferFunc retopFunc(Absloc r);
      static TransferFunc sibFunc(std::map<Absloc, std::pair<long,bool> > f,
         long d, Absloc t);

      static TransferFunc meet(const TransferFunc &lhs,
         const TransferFunc &rhs);

      bool isBaseRegCopy() const;
      bool isBaseRegSIB() const;
      bool isIdentity() const;
      bool isBottom() const;
      bool isTop() const;
      bool isRetop() const;
      bool isAbs() const;
      bool isCopy() const;
      bool isDelta() const;
      bool isSIB() const;
      bool isTopBottom() const {
         return topBottom;
      }

      bool operator==(const TransferFunc &rhs) const {
         return from == rhs.from && target == rhs.target &&
            delta == rhs.delta && abs == rhs.abs && retop == rhs.retop &&
            topBottom == rhs.topBottom && fromRegs == rhs.fromRegs;
      }

      bool operator!=(const TransferFunc &rhs) const {
         return !(*this == rhs);
      }

      std::set<DefHeight> apply(const AbslocState &inputs) const;
      void accumulate(std::map<Absloc, TransferFunc> &inputs);
      TransferFunc summaryAccumulate(
         const std::map<Absloc, TransferFunc> &inputs) const;

      std::string format() const;
      Type type() const;

      Absloc from;
      Absloc target;
      long delta;
      long abs;

      // Distinguish between default-constructed transfer functions and
      // explicitly-retopped transfer functions.
      bool retop;

      // Annotate transfer functions that have the following characteristic:
      // if target is TOP, keep as TOP
      // else, target must be set to BOTTOM
      // E.g., sign-extending a register:
      //   if the register had an uninitialized stack height (TOP),
      //       the sign-extension has no effect
      //   if the register had a valid or notunique (BOTTOM) stack height,
      //       the sign-extension must result in a BOTTOM stack height
      bool topBottom;

      // Handle complex math from SIB functions
      std::map<Absloc, std::pair<long, bool> > fromRegs;

   private:
      Type type_;
   };

   typedef std::list<TransferFunc> TransferFuncs;
   typedef std::map<Absloc, TransferFunc> TransferSet;

   // Summarize the effects of a series (list!) of transfer functions.
   // Intended to summarize a block. We may want to do a better job of
   // summarizing, but this works...
   class SummaryFunc {
   public:
      static const long uninitialized = MAXLONG;
      static const long notUnique = MINLONG;

      SummaryFunc() {};

      void apply(ParseAPI::Block *block, const AbslocState &in,
         AbslocState &out) const;
      void accumulate(const TransferSet &in, TransferSet &out) const;

      std::string format() const;
      void validate() const;

      void add(TransferFuncs &f);
      void addSummary(const TransferSet &summary);

      TransferSet accumFuncs;
   };

   // The results of the stack analysis is a series of intervals. For each
   // interval we have the following information:
   //   a) Whether the function has a well-defined
   //      stack frame. This is defined as follows:
   //        * x86/AMD-64: a frame pointer
   //        * POWER: an allocated frame pointed to by GPR1
   //   b) The "depth" of the stack; the distance between
   //      the stack pointer and the caller's stack pointer.
   //   c) The "depth" of any copies of the stack pointer.

   typedef std::map<Offset, AbslocState> StateIntervals;
   typedef std::map<ParseAPI::Block *, StateIntervals> Intervals;

   typedef std::map<ParseAPI::Function *, Height> FuncCleanAmounts;

   typedef std::map<ParseAPI::Block *, SummaryFunc> BlockEffects;
   typedef std::map<ParseAPI::Block *, AbslocState> BlockState;
   typedef std::map<ParseAPI::Block *, TransferSet> BlockSummaryState;

   // To build intervals, we must replay the effect of each instruction.
   // To avoid sucking enormous time, we keep those transfer functions around...
   typedef std::map<ParseAPI::Block *, std::map<Offset, TransferFuncs> >
      InstructionEffects;
   typedef std::map<ParseAPI::Block *, std::map<Offset, TransferSet> >
      CallEffects;

   DATAFLOW_EXPORT StackAnalysis();
   DATAFLOW_EXPORT StackAnalysis(ParseAPI::Function *f);
   // TODO: Update DataflowAPI manual
   DATAFLOW_EXPORT StackAnalysis(ParseAPI::Function *f,
      const std::map<Address, Address> &crm,
      const std::map<Address, TransferSet> &fs,
      const std::set<Address> &toppable = std::set<Address>());

    DATAFLOW_EXPORT virtual ~StackAnalysis();

    DATAFLOW_EXPORT Height find(ParseAPI::Block *, Address addr, Absloc loc);
    DATAFLOW_EXPORT std::set<DefHeight> findDefHeight(ParseAPI::Block *block,
        Address addr, Absloc loc);
   DATAFLOW_EXPORT Height findSP(ParseAPI::Block *, Address addr);
   DATAFLOW_EXPORT Height findFP(ParseAPI::Block *, Address addr);
   DATAFLOW_EXPORT void findDefinedHeights(ParseAPI::Block* b, Address addr,
      std::vector<std::pair<Absloc, Height> >& heights);
   // TODO: Update DataflowAPI manual
   DATAFLOW_EXPORT void findDefHeightPairs(ParseAPI::Block *b, Address addr,
      std::vector<std::pair<Absloc, std::set<DefHeight> > > &defHeights);

   // TODO: Update DataflowAPI manual
   DATAFLOW_EXPORT bool canGetFunctionSummary();
   DATAFLOW_EXPORT bool getFunctionSummary(TransferSet &summary);

   DATAFLOW_EXPORT void debug();

private:
   std::string format(const AbslocState &input) const;
   std::string format(const TransferSet &input) const;

   MachRegister sp();
   MachRegister fp();

   bool analyze();
   bool genInsnEffects();
   void summarizeBlocks(bool verbose = false);
   void summarize();

   void fixpoint(bool verbose = false);
   void summaryFixpoint();

   void createIntervals();

   void createEntryInput(AbslocState &input);
   void createSummaryEntryInput(TransferSet &input);
   void meetInputs(ParseAPI::Block *b, AbslocState& blockInput,
      AbslocState &input);
   void meetSummaryInputs(ParseAPI::Block *b, TransferSet &blockInput,
      TransferSet &input);
   DefHeight meetDefHeight(const DefHeight &dh1, const DefHeight &dh2);
   std::set<DefHeight> meetDefHeights(const std::set<DefHeight> &s1,
      const std::set<DefHeight> &s2);
   void meet(const AbslocState &source, AbslocState &accum);
   void meetSummary(const TransferSet &source, TransferSet &accum);
   AbslocState getSrcOutputLocs(ParseAPI::Edge* e);
   TransferSet getSummarySrcOutputLocs(ParseAPI::Edge *e);
   void computeInsnEffects(ParseAPI::Block *block, InstructionPtr insn,
      const Offset off, TransferFuncs &xferFunc, TransferSet &funcSummary);

   bool isCall(InstructionPtr insn);
   bool isJump(InstructionPtr insn);
   bool handleNormalCall(InstructionPtr insn, ParseAPI::Block *block,
      Offset off, TransferFuncs &xferFuncs, TransferSet &funcSummary);
   bool handleThunkCall(InstructionPtr insn, ParseAPI::Block *block,
      const Offset off, TransferFuncs &xferFuncs);
   bool handleJump(InstructionPtr insn, ParseAPI::Block *block,
      Offset off, TransferFuncs &xferFuncs, TransferSet &funcSummary);
   void handlePushPop(InstructionPtr insn, ParseAPI::Block *block,
      const Offset off, int sign, TransferFuncs &xferFuncs);
   void handleReturn(InstructionPtr insn, TransferFuncs &xferFuncs);
   void handleAddSub(InstructionPtr insn, ParseAPI::Block *block,
      const Offset off, int sign, TransferFuncs &xferFuncs);
   void handleLEA(InstructionPtr insn, TransferFuncs &xferFuncs);
   void handleLeave(ParseAPI::Block *block, const Offset off,
      TransferFuncs &xferFuncs);
   void handlePushPopFlags(int sign, TransferFuncs &xferFuncs);
   void handlePushPopRegs(int sign, TransferFuncs &xferFuncs);
   void handlePowerAddSub(InstructionPtr insn, ParseAPI::Block *block,
      const Offset off, int sign, TransferFuncs &xferFuncs);
   void handlePowerStoreUpdate(InstructionPtr insn, ParseAPI::Block *block,
      const Offset off, TransferFuncs &xferFuncs);
   void handleMov(InstructionPtr insn, ParseAPI::Block *block,
      const Offset off, TransferFuncs &xferFuncs);
   void handleZeroExtend(InstructionPtr insn, ParseAPI::Block *block,
      const Offset off, TransferFuncs &xferFuncs);
   void handleSignExtend(InstructionPtr insn, ParseAPI::Block *block,
      const Offset off, TransferFuncs &xferFuncs);
   void handleSpecialSignExtend(InstructionPtr insn, TransferFuncs &xferFuncs);
   void handleXor(InstructionPtr insn, ParseAPI::Block *block, const Offset off,
      TransferFuncs &xferFuncs);
   void handleDiv(InstructionPtr insn, TransferFuncs &xferFuncs);
   void handleMul(InstructionPtr insn, TransferFuncs &xferFuncs);
   void handleSyscall(InstructionPtr insn, ParseAPI::Block *block,
      const Offset off, TransferFuncs &xferFuncs);
   void handleDefault(InstructionPtr insn, ParseAPI::Block *block,
      const Offset off, TransferFuncs &xferFuncs);

   long extractDelta(InstructionAPI::Result deltaRes);
   bool getSubReg(const MachRegister &reg, MachRegister &subreg);
   void retopBaseSubReg(const MachRegister &reg, TransferFuncs &xferFuncs);
   void copyBaseSubReg(const MachRegister &reg, TransferFuncs &xferFuncs);
   void bottomBaseSubReg(const MachRegister &reg, TransferFuncs &xferFuncs);

   static void makeTopSet(std::set<DefHeight> &s);
   static void makeBottomSet(std::set<DefHeight> &s);
   static void makeNewSet(ParseAPI::Block *b, Address addr,
      const Absloc &origLoc, const Height &h, std::set<DefHeight> &s);
   static void addInitSet(const Height &h, std::set<DefHeight> &s);
   static void addDeltaSet(long delta, std::set<DefHeight> &s);
   static Height getHeightSet(const std::set<DefHeight> &s);
   static Definition getDefSet(const std::set<DefHeight> &s);


   Height getStackCleanAmount(ParseAPI::Function *func);

   // This constant limits the number of definitions we track per register. If
   // more than this many definitions are found, the register is considered to
   // be BOTTOM, and the definitions tracked so far are dropped.
   static const unsigned DEF_LIMIT = 2;

   ParseAPI::Function *func;

   // Map from call sites to PLT-resolved addresses
   std::map<Address, Address> callResolutionMap;

   // Function summaries to utilize during analysis
   std::map<Address, TransferSet> functionSummaries;

   // Functions whose return values should be topped rather than bottomed.  This
   // is used when evaluating a cycle in the call graph via fixed-point
   // analysis.  Note that if functionSummaries contains a summary for the
   // function, it will be used instead.
   std::set<Address> toppableFunctions;

   // SP effect tracking
   BlockEffects *blockEffects;  // Pointer so we can make it an annotation
   InstructionEffects *insnEffects;  // Pointer so we can make it an annotation
   CallEffects *callEffects;  // Pointer so we can make it an annotation

   BlockState blockInputs;
   BlockState blockOutputs;

   // Like blockInputs and blockOutputs, but used for function summaries.
   // Instead of tracking Heights, we track transfer functions.
   BlockSummaryState blockSummaryInputs;
   BlockSummaryState blockSummaryOutputs;

   Intervals *intervals_; // Pointer so we can make it an annotation

   FuncCleanAmounts funcCleanAmounts;
   int word_size;
   ExpressionPtr theStackPtr;
   ExpressionPtr thePC;
};

} // namespace Dyninst


namespace Dyninst {
   DEF_AST_LEAF_TYPE(StackAST, Dyninst::StackAnalysis::Height);
}
#endif
