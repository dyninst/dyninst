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
        
        class StackHeight {
        public:
            typedef signed long stackHeight_t;
            
            static const stackHeight_t uninitialized = MAXLONG;
            static const stackHeight_t notUnique = MINLONG;           
            
            static const StackHeight bottom;
            static const StackHeight top;
            
            StackHeight(const stackHeight_t h) : height_(h) {};
            StackHeight() : height_(uninitialized) {};
            stackHeight_t height() const { return height_; };
            
            static StackHeight join(std::set<StackHeight> &ins) {
                // Useful default argument
                if (ins.empty()) 
                    return bottom;
                
                // If there is a single element in the set return it.
                if (ins.size() == 1) 
                    return *(ins.begin());
                
                // JOIN (..., top) == top
                // Since ins is sorted, if top is in the set it must
                // be the end element...
                const StackHeight &last = *(ins.rbegin());
                if (last == top) return top;
                
                // U (bot, N) == N
                const StackHeight &first = *(ins.begin());
                if ((ins.size() == 2) &&
                    (first == bottom))
                    return *(ins.rbegin());
                
                // There are 2 or more elements; the first one is not
                // bottom; therefore there is a conflict, and we return
                // top.
            return top;
            }
            
            static StackHeight meet (std::set<StackHeight> &ins) {
                // Another useful default.
                if (ins.empty()) {
                    return top;
                }
                
                // If there is a single element in the set return it.
                if (ins.size() == 1) {
                    return *(ins.begin());
                }
                
                // MEET (bottom, ...) == bottom
                // Since ins is sorted, if bottom is in the set it must
                // be the start element...
                
                if ((*(ins.begin())) == bottom)  {
                    return bottom;
                }
                
                // MEET (N, top) == N
                if ((ins.size() == 2) &&
                    ((*(ins.rbegin())) == top)) {
                    return *(ins.begin());
                }

                // There are 2 or more elements; the last one is not
                // top; therefore there is a conflict, and we return
                // bottom.
                return bottom;
            }
            
            // FIX THIS!!! if we stop using TOP == MAXINT and
            // BOT == MININT...
            bool operator<(const StackHeight &comp) const {
                return (height_ < comp.height_);
            }
            
            StackHeight &operator+= (const StackHeight &other) {
                if (isBottom()) return *this;
                if (other.isBottom()) {
                    *this = bottom;
                    return *this;
                }
                if (other.isTop()) {
                    return *this;
                }
                height_ += other.height_;
                return *this;
            }
            const StackHeight operator+(const StackHeight &rhs) const {
                if (isBottom()) return bottom;
                if (rhs.isBottom()) return rhs;
                if (isTop()) return rhs;
                if (rhs.isTop()) return *this;
                return StackHeight(height_ + rhs.height_);
            }
            bool operator==(const StackHeight &other) const {
                return height_ == other.height_;
            }
            bool operator!=(const StackHeight &rhs) const {
                return height_ != rhs.height_;
            }

            std::string getString() const {
                if (isTop()) return "TOP";
                if (isBottom()) return "BOTTOM";
                char buf[256];
                sprintf(buf, "%ld", height_);
                return std::string(buf);
            }                                        

            bool isBottom() const {
                return height_ == notUnique;
            }

            bool isTop() const {
                return height_ == uninitialized; 
            }

        private:
            stackHeight_t height_;
        };

        class StackPresence {
        public:
            typedef enum {
                bottom,
                noFrame,
                frame,
                top} Presence;
            
            StackPresence() : presence_(top) {};
            StackPresence(Presence d) : presence_(d) {};
            Presence presence() const { return presence_; };

            static StackPresence meet(std::set<StackPresence> &ins) {
                // See comments above for optimizations
                if (ins.empty()) return top;
                if (ins.size() == 1) return *(ins.begin());
                if ((*(ins.begin())) == bottom) return bottom;
                if ((ins.size() == 2) && 
                    ((*(ins.rbegin())) == top)) return *(ins.begin());
                return bottom;
            }

            // This is arbitrary and strictly for ordering
            // in set insertion. The only requirement is that 
            // bottom < (ANY) and (ANY) < top
            bool operator<(const StackPresence &rhs) const {
                return (presence_ < rhs.presence_); 
            }
            bool operator==(const StackPresence &other) const {
                return presence_ == other.presence_;
            }
            bool operator!=(const StackPresence &rhs) const {
                return presence_ != rhs.presence_;
            }

            std::string getString() const {
                switch(presence_) {
                case bottom:
                    return "BOTTOM";
                case noFrame:
                    return "NOFRAME";
                case frame:
                    return "FRAME";
                case top:
                    return "TOP";
                default:
                    assert(0);
                    return "UNKNOWN";
                }
            }

            bool isBottom() const {
                return presence_ == bottom;
            }

            bool isTop() const {
                return presence_ == top;
            }

        private:
            Presence presence_;
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
        
        typedef class IntervalTree<Offset, StackHeight> HeightTree;

        typedef class IntervalTree<Offset, StackPresence> PresenceTree;

        typedef image_basicBlock Block;
        typedef image_edge Edge;
        typedef image_func Function;

        typedef std::map<Offset, StackHeight> InsnDeltas;
        typedef std::map<Block *, InsnDeltas> BlockToInsnDeltas;        
        typedef std::map<Block *, StackHeight> BlockHeights;
        typedef std::map<Function *, StackHeight> FuncCleanAmounts;

        typedef std::map<Block *, StackPresence> BlockPresence;
        typedef std::map<Offset, StackPresence> InsnPresence;
        typedef std::map<Block *, InsnPresence> BlockToInsnPresence;
        
        typedef std::map<Block *, bool> BlockStackReset;
        typedef std::map<Offset, bool> InsnStackReset;

        StackAnalysis() : func(NULL), heightIntervals_(NULL), presenceIntervals_(NULL) {};
        StackAnalysis(Function *f);


        const HeightTree *heightIntervals();
        const PresenceTree *presenceIntervals();

        void debugStackHeights();
        void debugStackPresences();

    private:
        
        bool analyze();
        void summarizeBlockDeltas();
        void calculateInterBlockDepth();
        void createIntervals();
        void computeInsnEffects(const Block *block,
                                const InstructionAPI::Instruction &insn,
                                Offset off,
                                StackHeight &height,
                                StackPresence &pres,
                                bool &reset);
        StackHeight getStackCleanAmount(Function *func);

        Block::blockSet blocks;
        Function *func;

        BlockToInsnDeltas blockToInsnDeltas;

        BlockHeights blockHeightDeltas;
        BlockHeights inBlockHeights;
        BlockHeights outBlockHeights;

        BlockToInsnPresence blockToInsnPresence;
        BlockPresence blockPresenceDeltas;
        BlockPresence inBlockPresences;
        BlockPresence outBlockPresences;

        HeightTree *heightIntervals_; // Pointer so we can make it an annotation
        PresenceTree *presenceIntervals_;

        FuncCleanAmounts funcCleanAmounts;

        BlockStackReset blockStackReset;
        InsnStackReset insnStackReset;

    };
};

#endif

