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

/*
 * Support for speculative parsing of code regions, such as scanning
 * for function preambles in gaps between known code regions.
 */

#include "ParseData.h"

#include "common/src/arch.h"

#include "parseAPI/h/CodeObject.h"
#include "parseAPI/h/CodeSource.h"
#include "parseAPI/h/CFG.h"
#include "parseAPI/h/InstructionAdapter.h"

#include "Parser.h"
#include "debug_parse.h"
#include "util.h"

using namespace std;
using namespace Dyninst;
using namespace Dyninst::ParseAPI;

#if defined(cap_stripped_binaries)

#include "ProbabilisticParser.h"

namespace hd {
    Address calc_end(Function * f) {
        Address ret = f->addr() + 1;
        if(!f->extents().empty()) {
            ret = f->extents().back()->end();
        }
        return ret;
    }

    bool compute_gap(
        CodeRegion * cr,
        Address addr,
        set<Function *,Function::less> const& funcs,
        set<Function *,Function::less>::const_iterator & fit,
        Address & gapStart,
        Address & gapEnd)
    {
        Function * cur = NULL;
        Function * next = NULL;
        long gapsize = 0;

        long MIN_GAP_SIZE = 5;      // probably too small, really

        Address lowerBound = cr->offset();
        Address upperBound = cr->offset() + cr->length();

        // special case for the first gap
        if(fit == funcs.begin()) {
            gapStart = lowerBound;
            if(fit == funcs.end())
                gapEnd = upperBound;
            else
                gapEnd = (*fit)->addr();
            gapsize = (long)(gapEnd - gapStart);
        } else {
            gapStart = 0;
            gapEnd = 0;
        }

        parsing_printf("addr: %lx gs: %lx ge: %lx\n",addr,gapStart,gapEnd);

        while(addr >= gapEnd ||
              gapsize <= MIN_GAP_SIZE)
        {
            if(fit == funcs.end() || (*fit)->addr() > upperBound) {
                return false;
            }
            
            cur = *fit;
            gapStart = calc_end(cur); 

			set<Function*,Function::less>::const_iterator fit2(fit);
            ++fit2;
    
            if(fit2 == funcs.end() || (*fit2)->addr() > upperBound)
                gapEnd = upperBound;
            else {
                next = *fit2;
                gapEnd = next->addr();
            } 
           
            gapsize = (long)(gapEnd - gapStart);
            if(addr >= gapEnd || gapsize <= MIN_GAP_SIZE)
                ++fit; 
        }

        parsing_printf("[%s] found code gap [%lx,%lx) (%ld bytes)\n",
            FILE__,gapStart,gapEnd,gapsize);

        return true;
    }
    bool compute_gap_new(
        CodeRegion * cr,
        Address addr,
        set<Function *,Function::less> const& funcs,
        set<Function *,Function::less>::const_iterator & beforeGap,
        Address & gapStart,
        Address & gapEnd,
	bool &reset_iterator)
    {
        long MIN_GAP_SIZE = 15;    
        Address lowerBound = cr->offset();
        Address upperBound = cr->offset() + cr->length();


        if (funcs.empty()) {
	    if (addr >= upperBound) return false;
	    gapStart = addr + 1;
	    if (gapStart < lowerBound) gapStart = lowerBound;
	    gapEnd = upperBound;
	    reset_iterator = true;
	    return true;
	} else if (addr < (*funcs.begin())->addr()) {
	    gapStart = addr + 1;
	    if (gapStart < lowerBound) gapStart = lowerBound;
	    gapEnd = (*funcs.begin())->addr();
	    reset_iterator = true;
	    return true;
	} else {
	    reset_iterator = false;
	    set<Function *,Function::less>::const_iterator afterGap(beforeGap);
	    ++afterGap;
	    while (true) {
	        gapStart = calc_end(*beforeGap);
		if (afterGap == funcs.end() || (*afterGap)->addr() > upperBound)
		    gapEnd = upperBound;
		else
		    gapEnd = (*afterGap)->addr();
		if (addr >= gapEnd || (long)(gapEnd - gapStart) <= MIN_GAP_SIZE) {
		    if (afterGap == funcs.end()) return false;
		    beforeGap = afterGap;
		    ++afterGap;
		} else {
		    if (gapStart < addr + 1) gapStart = addr + 1;
		    break;
		}
	    }
	    return true;
	}
    }
    bool gap_heuristic_GCC(CodeObject *co,CodeRegion *cr,Address addr)
    {
        using namespace Dyninst::InstructionAPI;

        // adjust this if we look before the current address
        const unsigned char* bufferBegin = 
            (const unsigned char*)(cr->getPtrToInstruction(addr));
        if(!bufferBegin)
            return false;
        if (!isStackFramePrecheck_gcc(bufferBegin))
            return false;

        InstructionDecoder dec(bufferBegin, 
            cr->offset() + cr->length() - addr, 
            cr->getArch());
	Block * blk = NULL;
	InstructionAdapter_t* ah = InstructionAdapter_t::makePlatformIA_IAPI(co->cs()->getArch(),dec, addr, co, cr, cr, blk);
	bool ret = ah->isStackFramePreamble();
	delete ah;
        return ret;
    }

    bool gap_heuristic_MSVS(CodeObject *co, CodeRegion *cr, Address addr)
    {
        using namespace Dyninst::InstructionAPI;
    
        const unsigned char* bufferBegin = 
            (const unsigned char*)(cr->getPtrToInstruction(addr));
        if(!bufferBegin)
            return false;
        if (!isStackFramePrecheck_msvs(bufferBegin))
            return false;
    
        InstructionDecoder dec(bufferBegin, 
            cr->offset()+cr->length()-addr,
            cr->getArch());
	Block * blk = NULL;
    	InstructionAdapter_t* ah = InstructionAdapter_t::makePlatformIA_IAPI(co->cs()->getArch(),dec, addr, co, cr, cr, blk);
	bool ret = ah->isStackFramePreamble();
	delete ah;
        return ret;
    }

    bool gap_heuristics(CodeObject *co,CodeRegion *cr,Address addr)
    {
        bool ret = false;
#if defined(arch_x86) || defined(arch_x86_64) || defined(i386_unknown_nt4_0)

  #if defined(os_windows)
        ret = gap_heuristic_MSVS(co,cr,addr);
  #else
        ret = gap_heuristic_GCC(co,cr,addr);
  #endif
#endif  
        return ret;
    }

    bool IsNop(CodeObject *co, CodeRegion *cr, Address addr) {
        using namespace Dyninst::InstructionAPI;
    
        const unsigned char* bufferBegin = 
            (const unsigned char*)(cr->getPtrToInstruction(addr));
        if(!bufferBegin)
            return false;
 
        InstructionDecoder dec(bufferBegin, 
            cr->offset() + cr->length() - addr, 
            cr->getArch());
	Block * blk = NULL;
    	InstructionAdapter_t* ah = InstructionAdapter_t::makePlatformIA_IAPI(co->cs()->getArch(),dec, addr, co, cr, cr, blk);
	bool ret = ah->isNop();
	delete ah;
        return ret;
    }
}

/*
 * Uses platform-specific function preamble patterns to
 * scan between known functions within a code region.
 * A typical stripped ELF binary might look something like
 * this:

        _______     <-- .text begin
       |       |
       |-------|
       |       |
       |  PLT  |
       |       |
       |-------| 
       |       |    <-- gap
       |-------|
       | code  |
       |       |
       |-------|
       |       |    <-- gap
       |-------|
       .       .
       . code  .
       .       .
       |       |
       |-------|
       |       |    <-- gap
       |       |
       |-------|    <-- .text end
       |       |
       .       .
       .       .

 * parse_gap_heuristic() will look for functions
 * in the `gap' subregions
 */
void Parser::parse_gap_heuristic(CodeRegion * cr)
{
    // ensure that we've parsed and finalized
    // all vanilla parsing
    if(_parse_state < COMPLETE)
        parse();
    finalize();

    Address gapStart = 0;
    Address gapEnd = 0;
    Address curAddr = 0;

    int match = 0;

    // don't touch this iterator, except when it starts out empty
    bool reset_iterator = sorted_funcs.empty();
    set<Function *,Function::less>::const_iterator fit = sorted_funcs.begin();
    while(hd::compute_gap(cr,curAddr,sorted_funcs,fit,gapStart,gapEnd)) {
        parsing_printf("[%s] scanning for prologues in [%lx,%lx)\n",
            FILE__,gapStart,gapEnd);
        for(curAddr=gapStart; curAddr < gapEnd; ++curAddr) {
            if(cr->isCode(curAddr) && hd::gap_heuristics(&_obj,cr,curAddr)) {
                assert(!findFuncByEntry(cr,curAddr));
                ++match;
                parse_at(cr,curAddr,true,GAP);

                if(reset_iterator) {
                    fit = sorted_funcs.begin();
                    reset_iterator = false;
                }

                break;
            }
        }
    }

    parsing_printf("[%s] gap parsing matched %d prologues\n",
        FILE__,match);

    // refinalize
    finalize();
}

bool Parser::getGapRange(CodeRegion* cr, Address curAddr, Address& gapStart, Address& gapEnd) {
    std::set< std::pair<Address, Address> > func_range;
    for (auto fit = sorted_funcs.begin(); fit != sorted_funcs.end(); ++fit) {
        Function * f = *fit;
	for (auto eit = f->extents().begin(); eit != f->extents().end(); ++eit) {
	    FuncExtent *fe = *eit;
	    func_range.insert(make_pair(fe->start(), fe->end()));
	}
    }
    auto iter = func_range.upper_bound(make_pair(curAddr, std::numeric_limits<Address>::max() ));
    if (iter == func_range.end()) {
        gapEnd = cr->offset() + cr->length();
    } else {
        gapEnd = iter->first;
    }
    if (iter == func_range.begin()) {
        gapStart = curAddr;
    } else {
	--iter;
        if (iter->second > curAddr) {
	    gapStart = iter->second;
        } else {
	    gapStart = curAddr;
        }
    }
    return gapStart < gapEnd;
}

void Parser::probabilistic_gap_parsing(CodeRegion *cr) {
    // 0. ensure that we've parsed and finalized all vanilla parsing.
    // We also locate all the gaps
    
    if (_parse_state < COMPLETE)
        parse();
    finalize();

    string model_spec;
    if (obj().cs()->getAddressWidth() == 8) 
        model_spec = "64-bit";
    else
        model_spec = "32-bit";

    // Load the pre-trained idiom model:
    hd::ProbabilityCalculator pc(cr, obj().cs(), this, model_spec);
    Address gapStart;
    Address gapEnd;
    Address curAddr = cr->offset();
    while (getGapRange(cr, curAddr, gapStart, gapEnd)) {
        parsing_printf("[%s] scanning for FEP in [%lx,%lx)\n",
            FILE__,gapStart,gapEnd);
        for(curAddr=gapStart; curAddr < gapEnd; ++curAddr) {
            if(cr->isCode(curAddr)) {
	        pc.calcProbByMatchingIdioms(curAddr);
		if (!pc.isFEP(curAddr)) continue;
		if (hd::IsNop(&_obj,cr, curAddr)) continue;
		Block* parsed = _obj.findBlockByEntry(cr, curAddr);
		if (parsed) continue;
                parse_at(cr,curAddr,true,GAP);
                break;
            }
        }
        finalize();
    }
}

#else // cap_stripped binaries
void Parser::parse_gap_heuristic(CodeRegion*)
{

}
void Parser::probabilistic_gap_parsing(CodeRegion *) {
}
#endif
