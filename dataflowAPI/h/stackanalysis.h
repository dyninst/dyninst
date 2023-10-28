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

#include <ostream>
#include <sstream>
#include <utility>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <string>

// To define StackAST
#include "DynAST.h"

#include "Absloc.h"
#include "dyntypes.h"
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
   }
   namespace InstructionAPI {
      class Instruction;
      class Expression;
   }

class StackAnalysis {
public:
   typedef boost::shared_ptr<InstructionAPI::Instruction> InstructionPtr;
   typedef boost::shared_ptr<InstructionAPI::Expression> ExpressionPtr;

   // This class represents a stack pointer definition by recording the block
   // and address of the definition, as well as the original absloc that was
   // defined by the definition.
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

   // This class represents offsets on the stack, which we call heights.
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
      bool operator<(const Height &rhs) const noexcept {
         return (height_ < rhs.height_);
      }

      bool operator>(const Height &rhs) const noexcept {
         return (height_ > rhs.height_);
      }

      bool operator<=(const Height &rhs) const noexcept {
         return (height_ <= rhs.height_);
      }

      bool operator>=(const Height &rhs) const noexcept {
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

   // This class represents pairs of Definitions and Heights.  During our stack
   // pointer analysis, we keep track of any stack pointers in registers or
   // memory, as well as the instruction addresses at which those pointers were
   // defined. This is useful for StackMod, where we sometimes want to modify
   // pointer definitions to adjust the locations of variables on the stack.
   // Thus, it makes sense to associate each stack pointer (Height) to the point
   // at which it was defined (Definition).
   class DATAFLOW_EXPORT DefHeight {
   public:
      DefHeight(const Definition &d, const Height &h) : def(d), height(h) {}

      bool operator==(const DefHeight &other) const {
         return def == other.def && height == other.height;
      }

      bool operator<(const DefHeight &other) const {
         return def < other.def;
      }

      Definition def;
      Height height;
   };

   // In some programs, it is possible for a register or memory location to
   // contain different stack pointers depending on the path taken to the
   // current instruction.  When this happens, our stack pointer analysis tries
   // to keep track of the different possible stack pointers, up to a maximum
   // number per instruction (specified by the DEF_LIMIT constant).  As a
   // result, we need a structure to hold sets of DefHeights.  This class fills
   // that role, providing several useful methods to build, modify, and
   // extract information from such sets.
   class DATAFLOW_EXPORT DefHeightSet {
   public:
      bool operator==(const DefHeightSet &other) const {
         return defHeights == other.defHeights;
      }

      // Returns an iterator to the set of DefHeights
      std::set<DefHeight>::iterator begin() {
         return defHeights.begin();
      }

      // Returns a constant iterator to the set of DefHeights
      std::set<DefHeight>::const_iterator begin() const {
         return defHeights.begin();
      }

      // Returns an iterator to the end of the set of DefHeights
      std::set<DefHeight>::iterator end() {
         return defHeights.end();
      }

      // Returns a constant iterator to the end of the set of DefHeights
      std::set<DefHeight>::const_iterator end() const {
         return defHeights.end();
      }

      // Returns the size of this set
      std::set<DefHeight>::size_type size() const {
         return defHeights.size();
      }

      // Inserts a DefHeight into this set
      void insert(const DefHeight &dh) {
         defHeights.insert(dh);
      }

      // Returns true if this DefHeightSet is TOP
      bool isTopSet() const;

      // Returns true if this DefHeightSet is BOTTOM
      bool isBottomSet() const;

      // Sets this DefHeightSet to TOP
      void makeTopSet();

      // Sets this DefHeightSet to BOTTOM
      void makeBottomSet();

      // Populates this DefHeightSet with the corresponding information
      void makeNewSet(ParseAPI::Block *b, Address addr,
         const Absloc &origLoc, const Height &h);

      // Adds to this DefHeightSet a new definition with height h
      void addInitSet(const Height &h);

      // Updates all Heights in this set by the delta amount
      void addDeltaSet(long delta);

      // Returns the result of computing a meet on all Heights in this set
      Height getHeightSet() const;

      // Returns the result of computing a meet on all Definitions in this set
      Definition getDefSet() const;

   private:
      std::set<DefHeight> defHeights;
   };

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
   typedef std::map<Absloc, DefHeightSet> AbslocState;
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

      DefHeightSet apply(const AbslocState &inputs) const;
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

      SummaryFunc() {}

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
   DATAFLOW_EXPORT StackAnalysis(ParseAPI::Function *f,
      const std::map<Address, Address> &crm,
      const std::map<Address, TransferSet> &fs,
      const std::set<Address> &toppable = std::set<Address>());

    DATAFLOW_EXPORT virtual ~StackAnalysis() = default;
    DATAFLOW_EXPORT StackAnalysis& operator=(const Dyninst::StackAnalysis&) = default;

    DATAFLOW_EXPORT Height find(ParseAPI::Block *, Address addr, Absloc loc);
    DATAFLOW_EXPORT DefHeightSet findDefHeight(ParseAPI::Block *block,
        Address addr, Absloc loc);
   DATAFLOW_EXPORT Height findSP(ParseAPI::Block *, Address addr);
   DATAFLOW_EXPORT Height findFP(ParseAPI::Block *, Address addr);
   DATAFLOW_EXPORT void findDefinedHeights(ParseAPI::Block* b, Address addr,
      std::vector<std::pair<Absloc, Height> >& heights);
   // TODO: Update DataflowAPI manual
   DATAFLOW_EXPORT void findDefHeightPairs(ParseAPI::Block *b, Address addr,
      std::vector<std::pair<Absloc, DefHeightSet> > &defHeights);

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
   DefHeightSet meetDefHeights(const DefHeightSet &s1,
      const DefHeightSet &s2);
   void meet(const AbslocState &source, AbslocState &accum);
   void meetSummary(const TransferSet &source, TransferSet &accum);
   AbslocState getSrcOutputLocs(ParseAPI::Edge* e);
   TransferSet getSummarySrcOutputLocs(ParseAPI::Edge *e);
   void computeInsnEffects(ParseAPI::Block *block, InstructionAPI::Instruction insn,
                           const Offset off, TransferFuncs &xferFunc, TransferSet &funcSummary);

   bool isCall(InstructionAPI::Instruction insn);
   bool isJump(InstructionAPI::Instruction insn);
   bool handleNormalCall(InstructionAPI::Instruction insn, ParseAPI::Block *block,
                         Offset off, TransferFuncs &xferFuncs, TransferSet &funcSummary);
   bool handleThunkCall(InstructionAPI::Instruction insn, ParseAPI::Block *block,
                        const Offset off, TransferFuncs &xferFuncs);
   bool handleJump(InstructionAPI::Instruction insn, ParseAPI::Block *block,
                   Offset off, TransferFuncs &xferFuncs, TransferSet &funcSummary);
   void handlePushPop(InstructionAPI::Instruction insn, ParseAPI::Block *block,
                      const Offset off, int sign, TransferFuncs &xferFuncs);
   void handleReturn(InstructionAPI::Instruction insn, TransferFuncs &xferFuncs);
   void handleAddSub(InstructionAPI::Instruction insn, ParseAPI::Block *block,
                     const Offset off, int sign, TransferFuncs &xferFuncs);
   void handleLEA(InstructionAPI::Instruction insn, TransferFuncs &xferFuncs);
   void handleLeave(ParseAPI::Block *block, const Offset off,
      TransferFuncs &xferFuncs);
   void handlePushPopFlags(int sign, TransferFuncs &xferFuncs);
   void handlePushPopRegs(int sign, TransferFuncs &xferFuncs);
   void handlePowerAddSub(InstructionAPI::Instruction insn, ParseAPI::Block *block,
                          const Offset off, int sign, TransferFuncs &xferFuncs);
   void handlePowerStoreUpdate(InstructionAPI::Instruction insn, ParseAPI::Block *block,
                               const Offset off, TransferFuncs &xferFuncs);
   void handleMov(InstructionAPI::Instruction insn, ParseAPI::Block *block,
                  const Offset off, TransferFuncs &xferFuncs);
   void handleZeroExtend(InstructionAPI::Instruction insn, ParseAPI::Block *block,
                         const Offset off, TransferFuncs &xferFuncs);
   void handleSignExtend(InstructionAPI::Instruction insn, ParseAPI::Block *block,
                         const Offset off, TransferFuncs &xferFuncs);
   void handleSpecialSignExtend(InstructionAPI::Instruction insn, TransferFuncs &xferFuncs);
   void handleXor(InstructionAPI::Instruction insn, ParseAPI::Block *block, const Offset off,
                  TransferFuncs &xferFuncs);
   void handleDiv(InstructionAPI::Instruction insn, TransferFuncs &xferFuncs);
   void handleMul(InstructionAPI::Instruction insn, TransferFuncs &xferFuncs);
   void handleSyscall(InstructionAPI::Instruction insn, ParseAPI::Block *block,
                      const Offset off, TransferFuncs &xferFuncs);
   void handleDefault(InstructionAPI::Instruction insn, ParseAPI::Block *block,
                      const Offset off, TransferFuncs &xferFuncs);

   long extractDelta(InstructionAPI::Result deltaRes);
   bool getSubReg(const MachRegister &reg, MachRegister &subreg);
   void retopBaseSubReg(const MachRegister &reg, TransferFuncs &xferFuncs);
   void copyBaseSubReg(const MachRegister &reg, TransferFuncs &xferFuncs);
   void bottomBaseSubReg(const MachRegister &reg, TransferFuncs &xferFuncs);


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
