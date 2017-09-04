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
#ifndef _PARSER_DETAILS_H_
#define _PARSER_DETAILS_H_

#include "IA_IAPI.h"

namespace Dyninst {
namespace ParseAPI {

namespace {
/*
 * The isCode queries into CodeSource objects with
 * disjoint regions involve expensive range lookups.
 * Because most often isCode is called on addresses
 * within the current function's region, this code
 * short circuits the expensive case.
 *
 * NB for overlapping region CodeSources, the two
 * cases in this function are identical. We'll pay
 * extra in this (uncommon) case.
 */
static inline bool is_code(Function * f, Address addr)
{
    return f->region()->isCode(addr) ||
           f->isrc()->isCode(addr);
}

}

class ParseWorkBundle;

// Seems to be an edge whose target (?) needs parsing

class ParseWorkElem
{
 public:
    /*
     * NOTE: The order of elements in this enum is critical to parsing order.
     * The earier an element appear in the enum, the sooner the corresponding 
     * edges are going to be parsed.
     *
     * 1. Our current implementation of non-returning function analysis 
     * WORK IFF call is prioritized over call_fallthrough.
     *
     * 2. We have a tail call heuristics that a jump to its own block is not a tail call.
     * For this heuristics to be more effective, we want to traverse 
     * certain intraprocedural edges such as call_fallthrough and cond_not_taken
     * over potential tail call edges such as cond_taken, br_direct, and br_indirect.
     *
     * 3. Jump table analysis would like to have as much intraprocedural control flow
     * as possible to resolve an indirect jump. So resolve_jump_table is delayed.
     *
     * Please make sure to update this comment 
     * if you change the order of the things appearing in the enum
     *
     */
    enum parse_work_order {
        seed_addr = 0,
        ret_fallthrough, /* conditional returns */
        call,
        call_fallthrough,
        cond_taken,
        cond_not_taken,
        br_direct,
        br_indirect,
        catch_block,
        checked_call_ft,
	resolve_jump_table, // We want to finish all possible parsing work before parsing jump tables
        __parse_work_end__
    };

    // allow direct access to setting order/frame type..
    ParseWorkElem(
            ParseWorkBundle *b, 
            parse_work_order o,
            Edge *e, 
            Address target, 
            bool resolvable,
            bool tailcall)
        : _bundle(b),
          _edge(e),
          _targ(target),
          _can_resolve(resolvable),
          _tailcall(tailcall),
          _order(o),
          _call_processed(false) { }

    ParseWorkElem(
            ParseWorkBundle *b, 
            Edge *e, 
            Address target, 
            bool resolvable,
            bool tailcall)
        : _bundle(b),
          _edge(e),
          _targ(target),
          _can_resolve(resolvable),
          _tailcall(tailcall),
          _order(__parse_work_end__),
          _call_processed(false),
	  _cur(NULL),
	  _ah(NULL)

    { 
      if(e) {
        switch(e->type()) {
            case CALL:
                _order = call; break;
            case COND_TAKEN:
                _order = cond_taken; break;
            case COND_NOT_TAKEN:
                _order = cond_not_taken; break;
            case INDIRECT:
                _order = br_indirect; break;
            case DIRECT:
                {
                    if (tailcall) {
                        _order = call;
                    } else {
                        _order = br_direct; 
                    }
                    break;
                }
            case FALLTHROUGH:
                _order = ret_fallthrough; break;
            case CATCH:
                _order = catch_block; break;
            case CALL_FT:
                _order = call_fallthrough; break;
            default:
                fprintf(stderr,"[%s:%d] FATAL: bad edge type %d\n",
                    FILE__,__LINE__,e->type());
                assert(0);
        } 
      } else 
        _order = seed_addr;
    }

    ParseWorkElem()
        : _bundle(NULL),
          _edge(NULL),
          _targ((Address)-1),
          _can_resolve(false),
          _tailcall(false),
          _order(__parse_work_end__),
          _call_processed(false),
	  _cur(NULL),
	  _ah(NULL)
    { } 

    // This work element is a continuation of
    // parsing jump tables
    ParseWorkElem(ParseWorkBundle *bundle, Block *b, const InsnAdapter::IA_IAPI* ah)
         : _bundle(bundle),
          _edge(NULL),
          _targ((Address)-1),
          _can_resolve(false),
          _tailcall(false),
          _order(resolve_jump_table),
          _call_processed(false),
	  _cur(b) {	      
	      _ah = ah->clone();
	  }

    ~ParseWorkElem() {
        if (_ah != NULL) delete _ah;
    }

      

    ParseWorkBundle *   bundle()        const { return _bundle; }
    Edge *              edge()          const { return _edge; }
    Address             target()        const { return _targ; }
    bool                resolvable()    const { return _can_resolve; }
    parse_work_order    order()         const { return _order; }
    void                setTarget(Address t)  { _targ = t; }

    bool                tailcall()      const { return _tailcall; }
    bool                callproc()      const { return _call_processed; }
    void                mark_call()     { _call_processed = true; }

    Block *             cur()           const { return _cur; }
    InsnAdapter::IA_IAPI *  ah()        const { return _ah; }

    /* 
     * Note that compare treats the parse_work_order as `lowest is
     * highest priority'.
     *
     * Sorts by parse_work_order, then bundle, then address
    */
    struct compare {
        bool operator()(const ParseWorkElem * e1, const ParseWorkElem * e2)
        {
            parse_work_order o1 = e1->order();
            parse_work_order o2 = e2->order();   

            if(o1 > o2)
                return true;
            else if(o1 < o2)
                return false;
            else 
	        return e1->target() > e2->target();
        }
    };

 private:
    ParseWorkBundle * _bundle;
    Edge * _edge;
    Address _targ;
    bool _can_resolve;
    bool _tailcall;
    parse_work_order _order;
    bool _call_processed;

    // Data for continuing parsing jump tables
    Block * _cur;
    InsnAdapter::IA_IAPI * _ah;
};

// ParseWorkElem container

class ParseWorkBundle
{
 public:
    ParseWorkBundle() {}
    ~ParseWorkBundle()
    {
        for(unsigned i=0;i<_elems.size();++i)
            delete _elems[i];
    }

    ParseWorkElem* add(ParseWorkElem * e) 
    { 
        _elems.push_back(e);
        return e;
    }
    vector<ParseWorkElem*> const& elems() { return _elems; }
 private:
    vector<ParseWorkElem*> _elems;
};

} // ParseAPI
} // Dyninst

#endif
