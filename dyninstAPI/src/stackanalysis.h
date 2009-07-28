/*
 * Copyright (c) 1996-2007 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#if !defined(STACK_ANALYSIS_H)
#define STACK_ANALYSIS_H

#include <values.h>
#include "dyntypes.h"
#include <set>
#include <string>

#include "common/h/IntervalTree.h"

// for blockSet...
#include "dyninstAPI/src/image-func.h"

#include "dyn_detail/boost/shared_ptr.hpp"

// These are _NOT_ in the Dyninst namespace...
class image_func;
class image_basicBlock;
class image_edge;

namespace Dyninst {
class InstructionAPI::Instruction;

class StackAnalysis {
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
              int delta,
              Offset off) : 
            range_(range), delta_(delta), off_(off) {};

        Range(const Range &r, 
              int delta) :
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
        int delta_;
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
    
    class Presence {
    public:
        typedef enum {
            bottom_t,
            noFrame_t,
            frame_t,
            top_t} Presence_t;

        static const Presence bottom;
        static const Presence top;
        
        Presence() : presence_(top_t) {};
        Presence(Presence_t d) : presence_(d) {};
        Presence presence() const { return presence_; };
        
        // This is arbitrary and strictly for ordering
        // in set insertion. The only requirement is that 
        // bottom < (ANY) and (ANY) < top
        bool operator<(const Presence &rhs) const {
            return (presence_ < rhs.presence_); 
        }
        bool operator==(const Presence &other) const {
            return presence_ == other.presence_;
        }
        bool operator!=(const Presence &rhs) const {
            return presence_ != rhs.presence_;
        }
        
        std::string format() const {
            switch(presence_) {
            case bottom_t:
                return "BOTTOM";
            case noFrame_t:
                return "NOFRAME";
            case frame_t:
                return "FRAME";
            case top_t:
                return "TOP";
            default:
                assert(0);
                return "UNKNOWN";
            }
        }

        void apply(const Presence &in,
                   Presence &out) const;
        void apply(Presence &out) const;

        bool isBottom() const {
            return presence_ == bottom_t;
        }
        
        bool isTop() const {
            return presence_ == top_t;
        }
        
    private:
        Presence_t presence_;
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
        InsnTransferFunc(int delta, bool reset) :
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

        int &delta() { return delta_; }
        bool &abs() { return abs_; }
        Range &range() { return range_; }

    private:
        int delta_;
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
        static const BlockTransferFunc initial;

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
        BlockTransferFunc(int d, 
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

        int &delta() { return delta_; }
        bool &reset() { return reset_; }
        bool &abs() { return abs_; }
        Ranges &ranges() { return ranges_; }

    private:
        int delta_;
        bool reset_;
        bool abs_;
        Ranges ranges_;
    };

    // We map sets of Ranges to Regions using a prefix tree.
    // Each node in the tree represents a particular Range.
    
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
    
    typedef class IntervalTree<Offset, Presence> PresenceTree;
    
    typedef image_basicBlock Block;
    typedef image_edge Edge;
    typedef image_func Function;
    
    typedef std::map<Offset, InsnTransferFunc> InsnFuncs;
    typedef std::map<Block *, InsnFuncs> BlockToInsnFuncs;

    typedef std::map<Function *, Height> FuncCleanAmounts;
    
    typedef std::map<Block *, Presence> BlockPresence;
    typedef std::map<Offset, Presence> InsnPresence;
    typedef std::map<Block *, InsnPresence> BlockToInsnPresence;
    
    typedef std::map<Block *, BlockTransferFunc> BlockEffects;

    StackAnalysis();
    StackAnalysis(Function *f);
    
    const HeightTree *heightIntervals();
    const PresenceTree *presenceIntervals();
    
    void debugHeights();
    void debugPresences();
    
 private:
    
    bool analyze();
    void summarizeBlocks();
    void fixpoint();

    void createPresenceIntervals();
    void createHeightIntervals();

    void computeInsnEffects(const Block *block,
                            const InstructionAPI::Instruction &insn,
                            const Offset off,
                            InsnTransferFunc &iFunc,
                            Presence &pres);

    Height getStackCleanAmount(Function *func);

    Region::Ptr getRegion(Ranges &ranges);
    
    Block::blockSet blocks;
    Function *func;
    
    BlockToInsnFuncs blockToInsnFuncs;
    BlockEffects blockEffects;
    BlockEffects inBlockEffects;
    BlockEffects outBlockEffects;
        
    BlockToInsnPresence blockToInsnPresences;
    BlockPresence blockPresences;
    BlockPresence inBlockPresences;
    BlockPresence outBlockPresences;
    
    HeightTree *heightIntervals_; // Pointer so we can make it an annotation
    PresenceTree *presenceIntervals_;
    
    FuncCleanAmounts funcCleanAmounts;
    
    RangeTree rt;
};
};

#endif

