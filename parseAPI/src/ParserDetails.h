/*
 * Copyright (c) 1996-2009 Barton P. Miller
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
#ifndef PARSER_DETAILS_H__
#define PARSER_DETAILS_H__



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
inline bool is_code(Function * f, Address addr)
{
    return f->region()->isCode(addr) ||
           f->isrc()->isCode(addr);
}

}

class ParseWorkBundle;
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
        : bundle_(b),
          edge_(e),
          targ_(target),
          can_resolve_(resolvable),
          tailcall_(tailcall),
          order_(__parse_work_end__),
          call_processed_(false)
    { 
      if(e) {
        switch(e->type()) {
            case CALL:
                order_ = call; break;
            case COND_TAKEN:
                order_ = cond_taken; break;
            case COND_NOT_TAKEN:
                order_ = cond_not_taken; break;
            case INDIRECT:
                order_ = br_indirect; break;
            case DIRECT:
                order_ = br_direct; break;
            case FALLTHROUGH:
                order_ = ret_fallthrough; break;
            case CATCH:
                order_ = catch_block; break;
            case CALL_FT:
                order_ = call_fallthrough; break;
            default:
                fprintf(stderr,"[%s:%d] FATAL: bad edge type %d\n",
                    FILE__,__LINE__,e->type());
                assert(0);
        } 
      } else 
        order_ = seed_addr;
    }

    ParseWorkElem()
        : bundle_(NULL),
          edge_(NULL),
          targ_((Address)-1),
          can_resolve_(false),
          tailcall_(false),
          order_(__parse_work_end__),
          call_processed_(false)
    { } 

    ParseWorkBundle *   bundle()        const { return bundle_; }
    Edge *              edge()          const { return edge_; }
    Address             target()        const { return targ_; }
    bool                resolvable()    const { return can_resolve_; }
    parse_work_order    order()         const { return order_; }

    bool                tailcall()      const { return tailcall_; }
    bool                callproc()      const { return call_processed_; }
    void                mark_call()     { call_processed_ = true; }

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
    ParseWorkBundle * bundle_;
    Edge * edge_;
    Address targ_;
    bool can_resolve_;
    bool tailcall_;
    parse_work_order order_;
    bool call_processed_;
};

class ParseWorkBundle
{
 public:
    ParseWorkBundle() {}
    ~ParseWorkBundle()
    {
        for(unsigned i=0;i<elems_.size();++i)
            delete elems_[i];
    }

    ParseWorkElem* add(ParseWorkElem * e) 
    { 
        elems_.push_back(e);
        return e;
    }
    vector<ParseWorkElem*> const& elems() { return elems_; }
 private:
    vector<ParseWorkElem*> elems_;
};

} // ParseAPI
} // Dyninst

#endif
