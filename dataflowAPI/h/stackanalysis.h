/*
 * Copyright (c) 1996-2011 Barton P. Miller
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

#if !defined(STACK_ANALYSIS_H)
#define STACK_ANALYSIS_H

#if !defined(_MSC_VER) && !defined(os_freebsd)
#include <values.h>
#endif

#include "dyntypes.h"
#include <set>
#include <string>
#include "util.h"

#include "common/h/IntervalTree.h"

// for blockSet...
//#include "dyninstAPI/src/image-func.h"

#include "dyn_detail/boost/shared_ptr.hpp"

// To define StackAST
#include "AST.h"

#if defined(os_aix) 
// AIX is missing a MINLONG...
#if defined(arch_64bit)
#define MINLONG INT64_MIN
#else
#define MINLONG INT32_MIN
#endif
#endif

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
  };

  namespace InstructionAPI {
    class Instruction;
  };

 
class StackAnalysis {
  typedef dyn_detail::boost::shared_ptr<InstructionAPI::Instruction> InstructionPtr;

 public:
    
    // The height of a stack is actually a value from a lattice:
    // 
    // <int>: height of the stack at this point
    // BOT/uninitialized: the height of the stack at this point is unknown (uninitialized)
    // TOP/notUnique: the height of the stack at this point is unknown (non-unique)
    
    template <class T> 
        T join(std::set<T> &ins) {
        if (ins.empty()) 
            return T::bottom;
        
        // If there is a single element in the set return it.
        if (ins.size() == 1) 
            return *(ins.begin());
        
        // JOIN (..., top) == top
        // Since ins is sorted, if top is in the set it must
        // be the end element...
        const T &last = *(ins.rbegin());
        if (last == T::top) return T::top;
        
        // U (bot, N) == N
        const T &first = *(ins.begin());
        if ((ins.size() == 2) &&
            (first == T::bottom))
            return *(ins.rbegin());
        
        // There are 2 or more elements; the first one is not
        // bottom; therefore there is a conflict, and we return
        // top.
        return T::top;
    }
    
    
    template <class T> 
        T meet (std::set<T> &ins) {
        // Another useful default.
        if (ins.empty()) {
            return T::top;
        }
        
        // If there is a single element in the set return it.
        if (ins.size() == 1) {
            return *(ins.begin());
        }
        
        // MEET (bottom, ...) == bottom
        // Since ins is sorted, if bottom is in the set it must
        // be the start element...
        
        if ((*(ins.begin())) == T::bottom)  {
            return T::bottom;
        }
        
        // MEET (N, top) == N
        if ((ins.size() == 2) &&
            ((*(ins.rbegin())) == T::top)) {
            return *(ins.begin());
        }
        
        // There are 2 or more elements; the last one is not
        // top; therefore there is a conflict, and we return
        // bottom.
        return T::bottom;
    }

    // A Range represents a single unknown
    // operation on the stack pointer. It
    // consists of an <int, int> pair that
    // represents the upper and lower bounds of
    // the operation on the stack pointer and 
    // an Offset that represents the instruction
    // performing the operation.

    class Range {
    public:
        typedef std::pair<long, long> range_t;
        
        static const range_t infinite;

        Range(range_t range,
              long delta,
              Offset off) : 
            range_(range), delta_(delta), off_(off) {};

        Range(const Range &r, 
              long delta) :
            range_(r.range_),
            delta_(delta),
            off_(r.off_) {};

        Range() : 
            delta_(0), off_(0) {
            
            range_.first = 0;
            range_.second = 0;
        }

        range_t &range() { return range_; }
        Offset &off() { return off_; }

        bool operator== (const Range &rhs) const {
            return ((range_ == rhs.range_) &&
                    (off_ == rhs.off_));
        }

        bool operator< (const Range &rhs) const {
            return (off_ < rhs.off_);
        }

        bool operator> (const Range &rhs) const {
            return (off_ > rhs.off_);
        }

        bool operator!= (const Range &rhs) const { 
            return !((*this) == rhs);
        }
        
        std::string format() const;
    private:
        range_t range_;
        long delta_;
        Offset off_;
        // Value of the stack when the range was applied
    };

    typedef std::vector<Range> Ranges;
    static const Range defaultRange;

    class Region {
    public:
        typedef dyn_detail::boost::shared_ptr<Region> Ptr;

        Region(int n, 
               Range &r,
               Region::Ptr p) : 
            name_(n),
            range_(r),
            prev_(p)
            {};

        Region(int n) : name_(n) {};
        Region() : name_(0) {};
       
        int &name() { return name_; };
        Range &range() { return range_; };
        Ptr prev() { return prev_; };

        std::string format() const;

        bool operator< (const Region &rhs) const {
            return name_ < rhs.name_; 
        }

        bool operator==(const Region &rhs) const {
            return name_ == rhs.name_;
        }
        
        bool operator!= (const Region &rhs) const {
            return !(*this == rhs);
        }

    private:
        int name_;
        Range range_;
        Ptr prev_;
    };

    class Height {
    public:
        typedef signed long Height_t;
        
        static const Height_t uninitialized = MAXLONG;           
        static const Height_t notUnique = MINLONG;           
        
        static const Height bottom;
        static const Height top;
        
        Height(const Height_t h, Region::Ptr r) :
            height_(h), region_(r) {};
        Height(const Height_t h) : height_(h) {};
        Height() : height_(uninitialized) {};

        Height_t height() const { return height_; }
        Region::Ptr region() const { return region_; }
        
        
        // FIX THIS!!! if we stop using TOP == MAXINT and
        // BOT == MININT...
        bool operator<(const Height &rhs) const {
            if (height_ == rhs.height_) 
                return region_ < rhs.region_;
            return (height_ < rhs.height_);
        }
        
        Height &operator+= (const Height &other) {
            if (isBottom()) return *this;
            if (other.isBottom()) {
                *this = bottom;
                return *this;
            }
            if (other.isTop()) {
                return *this;
            }

            if (region_ != other.region_) {
                *this = bottom;
                return *this;
            }

            height_ += other.height_;
            return *this;
        }

        const Height operator+(const Height &rhs) const {
            if (isBottom()) return bottom;
            if (rhs.isBottom()) return rhs;
            if (isTop()) return rhs;
            if (rhs.isTop()) return *this;
            if (region_ != rhs.region_) return bottom;

            return Height(height_ + rhs.height_, region_);
        }

	const Height operator+(const unsigned long &rhs) const {
	  if (isBottom()) return bottom;
	  if (isTop()) {
	    // WTF?
	    return Height(rhs, region_);
	  }
	  return Height(height_ + rhs, region_);
	}

        bool operator==(const Height &rhs) const {
            return ((height_ == rhs.height_) &&
                    (region_ == rhs.region_));
        }
        bool operator!=(const Height &rhs) const {
            return !(*this == rhs);
        }
        
        std::string format() const {
            if (isTop()) return "TOP";
            if (isBottom()) return "BOTTOM";
            
            std::stringstream retVal;
            retVal << height_ << region_->format();
            return retVal.str();
        }                                        
        
        bool isBottom() const {
            return height_ == notUnique;
        }
        
        bool isTop() const {
            return height_ == uninitialized; 
        }
        
    private:
        Height_t height_;
        
        Region::Ptr region_;
    };
    
    // We need to represent the effects of instructions. We do this
    // in terms of transfer functions. We recognize the following 
    // effects on the stack.
    //
    // Offset by known amount: push/pop/etc. 
    // Set to known value: leave
    // Offset by unknown amount expressible in a range [l, h]
    // Set to unknown value expressible in a range [l, h]
    //
    // These map as follows:
    // Offset by known amount: delta transfer function
    // Set to known value: set transfer function
    // Offset by unknown amount: new_region transfer function
    // Set to unknown value: new_region transfer function

    // This gives us three transfer functions. A transfer
    // function is a function T : (height, region, value) -> (height, region)
    // described as follows:
    // Delta (h, r, v) : (h+v, r)
    // Set (h, r, v) : (v, r)
    // New (h, r, vl, vh) : (0, new region(vl, vh))
    // 
    // where 'new region' represents a previously unallocated region, 
    // and (vl,vh) represent the range [l, h]

    typedef std::pair<Ranges, Height> Status;

    class InsnTransferFunc;
    class BlockTransferFunc;

    class InsnTransferFunc {
    public:
        static const InsnTransferFunc top;
        static const InsnTransferFunc bottom;

        static const long uninitialized = MAXLONG;
        static const long notUnique = MINLONG;

        InsnTransferFunc() : delta_(uninitialized), abs_(false), range_(defaultRange) {};
        InsnTransferFunc(long delta, bool reset) :
            delta_(delta), abs_(reset), range_(defaultRange) {};
        InsnTransferFunc(Range &r) :
            delta_(0), abs_(false), range_(r) {};

        void apply(const BlockTransferFunc &in,
                   BlockTransferFunc &out) const;

        void apply(BlockTransferFunc &update) const;

        std::string format() const;

        bool operator==(const InsnTransferFunc &rhs) const {
            return ((delta_ == rhs.delta_) &&
                    (abs_ == rhs.abs_) &&
                    (range_ == rhs.range_));
        }
        
        bool operator!=(const InsnTransferFunc &rhs) const {
            return !(*this == rhs);
        }

        long &delta() { return delta_; }
        bool &abs() { return abs_; }
        Range &range() { return range_; }

    private:
        long delta_;
        bool abs_;
        Range range_;
    };

    // This is a bit of an odd beast. It's a block-level
    // summary of what we've done to the stack, either
    // from the beginning of the block or the beginning
    // of the function. As such, it's a transfer function. 
    // It also needs to satisfy the join/meet requirements 
    // of operator <, operator ==, top, bottom. 

    class BlockTransferFunc {
    public:
        static const BlockTransferFunc bottom;
        static const BlockTransferFunc top;

        static const long uninitialized = MAXLONG;
        static const long notUnique = MINLONG;

        // From meet/join requirements: default constructor
        // is TOP. Represented by uninitialized delta.

        // Members:
        // delta (int) : the height difference from the start
        //               to the finish
        // reset (int) : if true, the stack is reset to 0 and
        //               delta measures from there. 
        // ranges (int, int) : represent unknown modifications
        //                     of the stack (e.g., rounding, adding
        //                     a variable, etc.)

        BlockTransferFunc() : 
            delta_(uninitialized), 
            reset_(false),
            abs_(false)
            {};
        BlockTransferFunc(long d, 
                          bool r,
                          bool a) : 
            delta_(d), 
            reset_(r),
            abs_(a) 
            {};
        
        bool operator <(const BlockTransferFunc &rhs) const {
            if ((*this) == rhs) return false;

            if (rhs == top) return true;
            else if (*this == bottom) return true;
            // Arbitrary unique ordering
            else if (delta_ < rhs.delta_) return true;
            else if (delta_ > rhs.delta_) return false;
            else if (reset_ < rhs.reset_) return true;
            else if (reset_ > rhs.reset_) return false;
            else if (abs_ < rhs.abs_) return true;
            else if (abs_ > rhs.abs_) return false;
            else if (ranges_.size() < rhs.ranges_.size()) return true;
            else if (ranges_.size() > rhs.ranges_.size()) return false;
            // Errr...
            for (unsigned i = 0; i < ranges_.size(); i++) {
                if (ranges_[i] < rhs.ranges_[i]) return true;
                else if (ranges_[i] > rhs.ranges_[i]) return false;
            }
            return false;
        }
        bool operator==(const BlockTransferFunc &rhs) const {
            if ((delta_ == uninitialized) && 
                (rhs.delta_ == uninitialized)) return true;
            if ((delta_ == notUnique) && 
                (rhs.delta_ == notUnique)) return true;

            if (delta_ != rhs.delta_) return false;
            if (reset_ != rhs.reset_) return false;
            if (abs_ != rhs.abs_) return false;
            if (ranges_.size() != rhs.ranges_.size()) return false;
            for (unsigned i = 0; i < ranges_.size(); i++) {
                if (ranges_[i] != rhs.ranges_[i]) return false;
            }
            return true;
        }
        bool operator!=(const BlockTransferFunc &rhs) const {
            return !(*this == rhs);
        }        


        void apply(const BlockTransferFunc &in, BlockTransferFunc &out) const;
        void apply(BlockTransferFunc &out) const;

        std::string format() const;

        long &delta() { return delta_; }
        bool &reset() { return reset_; }
        bool &abs() { return abs_; }
        Ranges &ranges() { return ranges_; }

    private:
        long delta_;
        bool reset_;
        bool abs_;
        Ranges ranges_;
    };

    // We map sets of Ranges to Regions using a prefix tree.
    // Each node in the tree represents a particular Range.
    
    typedef enum { 
      fp_noChange,
      fp_created,
      fp_destroyed } fp_State;
    
    class RangeTree {
    public:
        struct Node {
            Node() {};
            Node(Region::Ptr re) :
                region(re) {};
            ~Node() {
                for (std::map<Range, Node *>::iterator i = children.begin(); 
                     i != children.end(); i++) {
                    if (i->second)
                        delete i->second;
                }
            }

            std::map<Range, Node *> children;
            Region::Ptr region;
        };

        // May expand the tree
        Region::Ptr find(Ranges &str);

        RangeTree(Region::Ptr initial) : curRegion(0) {
            root = new Node(initial);
        };
        ~RangeTree() { 
            delete root; 
        }
        
        int getNewRegionID() { return ++curRegion; }

    private:
        int curRegion;
        Node *root;
    };
        
    // The results of the stack analysis is a series of 
    // intervals. For each interval we have the following
    // information: 
    //   a) Whether the function has a well-defined
    //      stack frame. This is defined as follows:
    //        x86/AMD-64: a frame pointer
    //        POWER: an allocated frame pointed to by GPR1
    //   b) The "depth" of the stack; the distance between
    //      the stack pointer and the caller's stack pointer.
    
    typedef class IntervalTree<Offset, Height> HeightTree;
    
    typedef std::map<Offset, InsnTransferFunc> InsnFuncs;
    typedef std::map<ParseAPI::Block *, InsnFuncs> BlockToInsnFuncs;

    typedef std::map<ParseAPI::Function *, Height> FuncCleanAmounts;
    
    typedef std::map<ParseAPI::Block *, BlockTransferFunc> BlockEffects;

    typedef std::pair<fp_State, Offset> FPChange;
    typedef std::vector<FPChange> FPChangePoints;
    typedef std::map<ParseAPI::Block *, FPChangePoints> BlockToFPChangePoints;
    typedef std::map<ParseAPI::Block *, Height> BlockHeights;

    DATAFLOW_EXPORT StackAnalysis();
    DATAFLOW_EXPORT StackAnalysis(ParseAPI::Function *f);
    
    // Lookup functions; preferred over the above
    DATAFLOW_EXPORT Height findSP(Address addr);
    DATAFLOW_EXPORT Height findFP(Address addr);
    
    DATAFLOW_EXPORT void debug();
    
 private:
    
    bool analyze();
    void summarizeBlocks();

    void sp_fixpoint();
    void sp_createIntervals();

    void fp_fixpoint();
    void fp_createIntervals();

    void computeInsnEffects(ParseAPI::Block *block,
                            const InstructionPtr &insn,
                            const Offset off,
                            InsnTransferFunc &spFunc,
                            fp_State &fpCopied);

    Height getStackCleanAmount(ParseAPI::Function *func);

    Region::Ptr getRegion(Ranges &ranges);
    
    //Block::blockSet blocks;
    ParseAPI::Function *func;

    // SP effect tracking
    BlockToInsnFuncs sp_blockToInsnFuncs;
    BlockEffects sp_blockEffects;
    BlockEffects sp_inBlockEffects;
    BlockEffects sp_outBlockEffects;

    // FP effect tracking. For now, we don't bother
    // integrating the two; instead, we mark where
    // the FP makes a copy of the SP and fill in the 
    // values later
    BlockToFPChangePoints fp_changePoints;
    BlockHeights fp_inBlockHeights;
    BlockHeights fp_outBlockHeights;

    HeightTree *sp_intervals_; // Pointer so we can make it an annotation
    HeightTree *fp_intervals_;
    
    FuncCleanAmounts funcCleanAmounts;
    
    RangeTree rt;
};

};

std::ostream &operator<<(std::ostream &os, const Dyninst::StackAnalysis::Height &h);

namespace Dyninst {
  DEF_AST_LEAF_TYPE(StackAST, Dyninst::StackAnalysis::Height);

};


#endif

