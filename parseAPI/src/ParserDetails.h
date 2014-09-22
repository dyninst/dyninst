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
    enum parse_work_order {
        seed_addr = 0,
        ret_fallthrough, /* conditional returns */
        cond_taken,
        cond_not_taken,
        br_direct,
        br_indirect,
        catch_block,
        call,
        call_fallthrough,
        __parse_work_end__
    };

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
          _call_processed(false)
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
          _call_processed(false)
    { } 

    ParseWorkBundle *   bundle()        const { return _bundle; }
    Edge *              edge()          const { return _edge; }
    Address             target()        const { return _targ; }
    bool                resolvable()    const { return _can_resolve; }
    parse_work_order    order()         const { return _order; }
    void                setTarget(Address t)  { _targ = t; }

    bool                tailcall()      const { return _tailcall; }
    bool                callproc()      const { return _call_processed; }
    void                mark_call()     { _call_processed = true; }

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
            else {
                if(e1->bundle() == e2->bundle()) 
                    return e1->target() > e2->target();
                else
                    return e1->bundle() > e2->bundle();
            }
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
