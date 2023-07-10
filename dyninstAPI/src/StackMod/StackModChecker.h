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

#ifndef _StackModChecker_h_
#define _StackModChecker_h_

#include <map>
#include <set>
#include <string>
#include <vector>
#include "BPatch_function.h"

#include "Instruction.h"

#include "dyninstAPI/src/function.h"
#include "StackMod.h"
#include "StackModExpr.h"

class StackModChecker
{
    public:
        StackModChecker(BPatch_function* b, func_instance* f) :
            bfunc(b),
            func(f),
            _unsafeStackGrowth(false) {}
        ~StackModChecker();

        bool addModsInternal(std::set<StackMod*>);

    private:
        BPatch_function* bfunc;
        func_instance* func;

    private:
        typedef map<int, pair<int, StackMod::MType> > rangeMap;
        typedef rangeMap::iterator rangeIterator;

        std::string getName() const;

        bool accumulateStackRanges(StackMod* m);
        bool alignStackRanges(int alignment, StackMod::MOrder order, std::vector<StackMod*>& mods);
        void findContiguousRange(rangeIterator iter, rangeIterator& end);
        
        bool checkStackGrowth(Dyninst::StackAnalysis& sa); 
        void processBlock(Dyninst::StackAnalysis& sa, Dyninst::ParseAPI::Block* block);
        bool checkAllPaths(Dyninst::StackAnalysis& sa);
        bool checkAllPathsInternal(Dyninst::ParseAPI::Block* block, 
                std::set<Dyninst::ParseAPI::Block*>& state,
                std::vector<Dyninst::ParseAPI::Block*>& path,
                std::set<Dyninst::ParseAPI::Block*>& exitBlocks,
                Dyninst::StackAnalysis& sa);
        bool checkPath(std::vector<Dyninst::ParseAPI::Block*>& path);
        
        bool findInsertOrRemovePoints(Dyninst::StackAnalysis& sa, StackMod* m, std::vector<BPatch_point*>*& points, long& dispFromRSP);
        bool checkInsn(Dyninst::ParseAPI::Block* block, Dyninst::Offset off, int loc, Dyninst::StackAnalysis& sa, BPatch_point*& point, long& dispFromRSP);
        bool findCanaryPoints(std::vector<BPatch_point*>* insertPoints,
            std::vector<BPatch_point*>* checkPoints);
        
        bool areModificationsSafe();
        bool isAccessSafe(InstructionAPI::Instruction insn, StackAccess *access);
        
        bool _unsafeStackGrowth;
        
        std::map<Dyninst::ParseAPI::Block*, std::vector<Dyninst::StackAnalysis::Height>* > blockHeights;

        rangeMap _stackRanges;
        rangeMap _stackRangesCleanup;
};

#endif
