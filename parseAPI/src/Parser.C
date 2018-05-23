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

#include <vector>
#include <limits>

// For Mutex
#define PROCCONTROL_EXPORTS

#include "dyntypes.h"

#include "CodeObject.h"
#include "CFGFactory.h"
#include "ParseCallback.h"
#include "Parser.h"
#include "CFG.h"
#include "util.h"
#include "debug_parse.h"

#include <boost/tuple/tuple.hpp>

using namespace std;
using namespace Dyninst;
using namespace Dyninst::ParseAPI;
using namespace Dyninst::InstructionAPI;

typedef std::pair< Address, EdgeTypeEnum > edge_pair_t;
typedef vector< edge_pair_t > Edges_t;

#include "common/src/dthread.h"

namespace {
    struct less_cr {
     bool operator()(CodeRegion * x, CodeRegion * y) 
     { 
         return x->offset() < y->offset();
     }
    };
}

Parser::Parser(CodeObject & obj, CFGFactory & fact, ParseCallbackManager & pcb) :
    _obj(obj),
    _cfgfact(fact),
    _pcb(pcb),
    _parse_data(NULL),
    num_delayedFrames(0),
    _sink(NULL),
    _parse_state(UNPARSED),
    _in_parse(false),
    _in_finalize(false)
{
    // cache plt entries for fast lookup
    const map<Address, string> & lm = obj.cs()->linkage();
    map<Address, string>::const_iterator lit = lm.begin();
    for( ; lit != lm.end(); ++lit) {
        parsing_printf("Cached PLT entry %s (%lx)\n",lit->second.c_str(),lit->first);
        plt_entries[lit->first] = lit->second;
    }

    if(obj.cs()->regions().empty()) {
        parsing_printf("[%s:%d] CodeSource provides no CodeRegions"
                       " -- unparesable\n",
            FILE__,__LINE__);
        _parse_state = UNPARSEABLE;
        return;
    }

    // check whether regions overlap
    vector<CodeRegion *> const& regs = obj.cs()->regions();
    vector<CodeRegion *> copy(regs.begin(),regs.end());
    sort(copy.begin(),copy.end(),less_cr());

    // allocate a sink block -- region is arbitrary
    _sink = _cfgfact._mksink(&_obj,copy[0]);

    bool overlap = false;
    CodeRegion * prev = copy[0], *cur = NULL;
    for(unsigned i=1;i<copy.size();++i) {
        cur = copy[i];
        if(cur->offset() < prev->offset() + prev->length()) {
            parsing_printf("Overlapping code regions [%lx,%lx) and [%lx,%lx)\n",
                prev->offset(),prev->offset()+prev->length(),
                cur->offset(),cur->offset()+cur->length());
            overlap = true;
            break;
        }
    }

    if(overlap)
        _parse_data = new OverlappingParseData(this,copy);
    else
        _parse_data = new StandardParseData(this);
}

ParseFrame::~ParseFrame()
{
    cleanup(); // error'd frames still need cleanup
}

Parser::~Parser()
{
    if(_parse_data)
        delete _parse_data;

    vector<ParseFrame *>::iterator fit = frames.begin();
    for( ; fit != frames.end(); ++fit) 
        delete *fit;
    frames.clear();
}

void
Parser::add_hint(Function * f)
{
    if(!_parse_data->findFunc(f->region(),f->addr()))
        record_func(f);
}

void
Parser::parse()
{
    parsing_printf("[%s:%d] parse() called on Parser %p with state %d\n",
                   FILE__,__LINE__,this, _parse_state);

    // For modification: once we've full-parsed once, don't do it again
    if (_parse_state >= COMPLETE) return;

    if(_parse_state == UNPARSEABLE)
        return;

    assert(!_in_parse);
    _in_parse = true;

    parse_vanilla();
    finalize();
    // anything else by default...?

    if(_parse_state < COMPLETE)
        _parse_state = COMPLETE;
    
    _in_parse = false;
    parsing_printf("[%s:%d] parsing complete for Parser %p with state %d\n", FILE__, __LINE__, this, _parse_state);
}

void
Parser::parse_at(
    CodeRegion * region,
    Address target,
    bool recursive,
    FuncSource src)
{
    Function *f;
    ParseFrame *pf;
    vector<ParseFrame *> work;

    parsing_printf("[%s:%d] entered parse_at([%lx,%lx),%lx)\n",
        FILE__,__LINE__,region->low(),region->high(),target);

    if(!region->contains(target)) {
        parsing_printf("\tbad address, bailing\n");
        return;
    }

    if(_parse_state < PARTIAL)
        _parse_state = PARTIAL;

    f = _parse_data->get_func(region,target,src);
    if(!f) {
        parsing_printf("   could not create function at %lx\n",target);
        return;
    }

    ParseFrame::Status exist = _parse_data->frameStatus(region,target);
    if(exist != ParseFrame::BAD_LOOKUP && exist != ParseFrame::UNPARSED) {
        parsing_printf("   function at %lx already parsed, status %d\n",
            target, exist);
        return;
    }

    if(!(pf = _parse_data->findFrame(region,target))) {
        pf = new ParseFrame(f,_parse_data);
        init_frame(*pf);
        frames.push_back(pf);
        _parse_data->record_frame(pf);
    } 

    work.push_back(pf);
    parse_frames(work,recursive);

    // downgrade state if necessary
    if(_parse_state > COMPLETE)
        _parse_state = COMPLETE;

}

void
Parser::parse_at(Address target, bool recursive, FuncSource src)
{
    CodeRegion * region = NULL;

    parsing_printf("[%s:%d] entered parse_at(%lx)\n",FILE__,__LINE__,target);

    if(_parse_state == UNPARSEABLE)
        return;

    StandardParseData * spd = dynamic_cast<StandardParseData *>(_parse_data);
    if(!spd) {
        parsing_printf("   parse_at is invalid on overlapping regions\n");
        return;
    }

    region = spd->reglookup(region,target); // input region ignored for SPD
    if(!region) {
        parsing_printf("   failed region lookup at %lx\n",target);
        return;
    }

    parse_at(region,target,recursive,src);
}

void
Parser::parse_vanilla()
{
    ParseFrame *pf;
    vector<ParseFrame *> work;
    vector<Function *>::iterator fit;

    parsing_printf("[%s:%d] entered parse_vanilla()\n",FILE__,__LINE__);
    parsing_printf("\t%d function hints\n",hint_funcs.size());

    if(_parse_state < PARTIAL)
        _parse_state = PARTIAL;
    else
        parsing_printf("\tparse state is %d, some parsing already done\n",
            _parse_state);

    /* Initialize parse frames from hints */
    for(fit=hint_funcs.begin();fit!=hint_funcs.end();++fit) {
        Function * hf = *fit;
        ParseFrame::Status test = frame_status(hf->region(),hf->addr());
        if(test != ParseFrame::BAD_LOOKUP &&
           test != ParseFrame::UNPARSED)
        {
            parsing_printf("\tskipping repeat parse of %lx [%s]\n",
                hf->addr(),hf->name().c_str());
            continue;
        }

        pf = new ParseFrame(hf,_parse_data);
        init_frame(*pf);
        frames.push_back(pf);
        work.push_back(pf);
        _parse_data->record_frame(pf);
    }

    parse_frames(work,true);
}

void
Parser::parse_edges( vector< ParseWorkElem * > & work_elems )
{
    if(_parse_state == UNPARSEABLE)
        return;

    // build up set of needed parse frames and load them with work elements
    set<ParseFrame*> frameset; // for dup checking
    vector<ParseFrame*> frames;

    for (unsigned idx=0; idx < work_elems.size(); idx++) {

        ParseWorkElem *elem = work_elems[idx];
        Block *src = elem->edge()->src();

        if (elem->order() == ParseWorkElem::call_fallthrough)
        {
            Edge *callEdge = NULL;
            Block::edgelist trgs = src->targets();
            for (Block::edgelist::iterator eit = trgs.begin(); 
                 eit != trgs.end(); 
                 eit++) 
            {
                if ((*eit)->type() == CALL) {
                    callEdge = *eit;
                    if (!(*eit)->sinkEdge()) // if it's a sink edge, look for nonsink CALL
                        break; 
                }
            }
            // create a call work elem so that the bundle is complete
            // and set the target function's return status and 
            // tamper to RETURN and TAMPER_NONE, respectively
            //assert(callEdge);

            // When we have a direct call to unallocated memory or garbage
            // we there is no call edge when the call is first executed. I'm
            // not sure why not, so I'm attempting to continue past here without
            // fixing up the called function...
            // Also, in the case I saw, the callee was _not_ returning. Not directly. It 
            // led into a big case with a longjmp() equivalent. 

            if (callEdge) 
            {
                bool isResolvable = false;
                Address callTarget = 0;
                if ( ! callEdge->sinkEdge() ) 
                {
                    isResolvable = true;
                    callTarget = callEdge->trg()->start();
                    // the call target may be in another Code Object
                    Function *callee = callEdge->trg()->obj()->findFuncByEntry(
                        callEdge->trg()->region(), callTarget);
                    assert(callee);
                    callee->set_retstatus(RETURN);
                    callee->_tamper = TAMPER_NONE;
                }
                elem->bundle()->add(new ParseWorkElem
                    ( elem->bundle(), 
                    callEdge,
                    callTarget,
                    isResolvable,
                    false ));
            }
        }
        ParseFrame *frame = _parse_data->findFrame
            ( src->region(), 
              src->lastInsnAddr() );
        bool isNewFrame = false;
        if (!frame) {
            vector<Function*> funcs;
            src->getFuncs(funcs);
            frame = new ParseFrame(*funcs.begin(),_parse_data);
            for (unsigned fix=1; fix < funcs.size(); fix++) {
                // if the block is shared, all of its funcs need
                // to add the new edge
                funcs[fix]->_cache_valid = false;
            }
            isNewFrame = true;
        }

        // push before frame init so no seed is added
        if (elem->bundle()) {
            frame->work_bundles.push_back(elem->bundle());
        }
        frame->pushWork(elem);
        if (isNewFrame) {
            init_frame(*frame);
        }

        if (frameset.end() == frameset.find(frame)) {
            frameset.insert(frame);
            frames.push_back(frame);
        }
    }

    // now parse
    if(_parse_state < PARTIAL)
        _parse_state = PARTIAL;
    _in_parse = true;

    parse_frames( frames, true );

    if(_parse_state > COMPLETE)
        _parse_state = COMPLETE;
    _in_parse = false;

    finalize();

}

void
Parser::parse_frames(vector<ParseFrame *> & work, bool recursive)
{
    ParseFrame * pf;

    /* Recursive traversal parsing */ 
    while(!work.empty()) {
	
        pf = work.back();
        work.pop_back();

        if(pf->status() == ParseFrame::PARSED)
            continue;

        parse_frame(*pf,recursive);
        switch(pf->status()) {
            case ParseFrame::CALL_BLOCKED: {
                parsing_printf("[%s] frame %lx blocked at %lx\n",
                    FILE__,pf->func->addr(),pf->curAddr);

                assert(pf->call_target);        

                parsing_printf("    call target %lx\n",pf->call_target->addr());
                work.push_back(pf);
    
                CodeRegion * cr = pf->call_target->region();
                Address targ = pf->call_target->addr();

                ParseFrame * tf = _parse_data->findFrame(cr,targ);
                if(!tf)
                {
                    // sanity
                    if(_parse_data->frameStatus(cr,targ) == ParseFrame::PARSED)
                        assert(0);

                    tf = new ParseFrame(pf->call_target,_parse_data);
                    init_frame(*tf);
                    frames.push_back(tf);
                    _parse_data->record_frame(tf);
                }
                if(likely(recursive))
                    work.push_back(tf);
                else {
                    assert(0);
                    // XXX should never get here
                    //parsing_printf("    recursive parsing disabled\n");
                }

                break;
            }
            case ParseFrame::PARSED:
                parsing_printf("[%s] frame %lx complete, return status: %d\n",
                    FILE__,pf->func->addr(),pf->func->_rs);

                if (unlikely( _obj.defensiveMode() && 
                              TAMPER_NONE != pf->func->tampersStack() &&
                              TAMPER_NONZERO != pf->func->tampersStack() ))
                {   // may adjust CALL_FT targets, add a frame to the worklist,
                    // or trigger parsing in a separate target CodeObject
                    tamper_post_processing(work,pf);
                }
                
                /* add waiting frames back onto the worklist */
                resumeFrames(pf->func, work);
                
                pf->cleanup();
                break;
            case ParseFrame::FRAME_ERROR:
                parsing_printf("[%s] frame %lx error at %lx\n",
                    FILE__,pf->func->addr(),pf->curAddr);
                break;
            case ParseFrame::FRAME_DELAYED: {
                parsing_printf("[%s] frame %lx delayed at %lx\n",
                        FILE__,
                        pf->func->addr(),
                       pf->curAddr);
                
                if (pf->delayedWork.size()) {
                    // Add frame to global delayed list
                    map<ParseWorkElem *, Function *>::iterator iter;
                    map<Function *, set<ParseFrame *> >::iterator fIter;
                    for (iter = pf->delayedWork.begin();
                            iter != pf->delayedWork.end();
                            ++iter) {

                        Function * ct = iter->second;
                        parsing_printf("[%s] waiting on %s\n",
                                __FILE__,
                                ct->name().c_str());

                        fIter = delayedFrames.find(ct);
                        if (fIter == delayedFrames.end()) {
                            std::set<ParseFrame *> waiters;
                            waiters.insert(pf);
                            delayedFrames[ct] = waiters;
                        } else {
                            delayedFrames[ct].insert(pf);
                        }
                    }
                } else {
                    // We shouldn't get here
                    assert(0 && "Delayed frame with no delayed work");
                }
                
                /* if the return status of this function has been updated, add
                 * waiting frames back onto the work list */
                resumeFrames(pf->func, work);
                break;
            }
            default:
                assert(0 && "invalid parse frame status");
        }
    }

    // Use fixed-point to ensure we parse frames whose parsing had to be delayed
    if (delayedFrames.size() == 0) {
        // Done!
        parsing_printf("[%s] Fixed point reached (0 funcs with unknown return status)\n)",
                __FILE__);
        num_delayedFrames = 0;
    } else if (delayedFrames.size() == num_delayedFrames) {
        // If we've reached a fixedpoint and have remaining frames, we must
        // have a cyclic dependency
        parsing_printf("[%s] Fixed point reached (%d funcs with unknown return status)\n", 
                __FILE__, 
                delayedFrames.size());

        // Mark UNSET functions in cycle as NORETURN
	// except if we're doing non-recursive parsing.
	// If we're just parsing one function, we want
	// to mark everything RETURN instead.
        map<Function *, set<ParseFrame *> >::iterator iter;
        vector<Function *> updated;
        for (iter = delayedFrames.begin(); iter != delayedFrames.end(); ++iter) {
            Function * func = iter->first;
            if (func->retstatus() == UNSET) {
	      if(recursive) 
	      {
                func->set_retstatus(NORETURN);
                func->obj()->cs()->incrementCounter(PARSE_NORETURN_HEURISTIC);
	      }
	      else
	      {
		func->set_retstatus(RETURN);
	      }
	      updated.push_back(func);
            } 

            set<ParseFrame *> vec = iter->second;
            set<ParseFrame *>::iterator vIter;
            for (vIter = vec.begin(); vIter != vec.end(); ++vIter) {
                Function * delayed = (*vIter)->func;
                if (delayed->retstatus() == UNSET) {
                    delayed->set_retstatus(NORETURN);
                    delayed->obj()->cs()->incrementCounter(PARSE_NORETURN_HEURISTIC);
                    updated.push_back(func);
                }
            }
        }

        // We should have updated the return status of one or more frames; recurse
        if (updated.size()) {
            for (vector<Function *>::iterator uIter = updated.begin();
                    uIter != updated.end();
                    ++uIter) {
                resumeFrames((*uIter), work);
            }

            if (work.size()) {
                parsing_printf("[%s] Updated retstatus of delayed frames, trying again; work.size() = %d\n",
                        __FILE__,
                        work.size());
                parse_frames(work, recursive);
            }
        } else {
            // We shouldn't get here
            parsing_printf("[%s] No more work can be done (%d funcs with unknown return status)\n",
                    __FILE__, 
                    delayedFrames.size());
            assert(0);
        }
    } else {
        // We haven't yet reached a fixedpoint; let's recurse
        parsing_printf("[%s] Fixed point not yet reached (%d funcs with unknown return status)\n", 
                __FILE__,
                delayedFrames.size());

        // Update num_delayedFrames for next iteration 
        num_delayedFrames = delayedFrames.size();

        // Check if we can resume any frames yet
        vector<Function *> updated;
        for (map<Function *, set<ParseFrame *> >::iterator iter = delayedFrames.begin(); 
                iter != delayedFrames.end(); 
                ++iter) {
            if (iter->first->_rs != UNSET) {
                updated.push_back(iter->first);
            }
        }

        if (updated.size()) {
            for (vector<Function *>::iterator uIter = updated.begin();
                    uIter != updated.end();
                    ++uIter) {
                resumeFrames((*uIter), work);
            }
        }

        // Recurse through parse_frames        
        parsing_printf("[%s] Calling parse_frames again, work.size() = %d\n", 
                __FILE__,
                work.size());
        parse_frames(work, recursive);
    }

    for(unsigned i=0;i<frames.size();++i) {
        _parse_data->remove_frame(frames[i]);
        delete frames[i];
    }
    frames.clear();
}

/* Finalizing all functions for consumption:
  
   - Finish delayed parsing
   - Prepare and record FuncExtents for range-based lookup
*/

// Could set cache_valid depending on whether the function is currently
// being parsed somewhere. 

void
Parser::finalize(Function *f)
{
    if(f->_cache_valid) {
        return;
    }

    if(!f->_parsed) {
        parsing_printf("[%s:%d] Parser::finalize(f[%lx]) "
                       "forced parsing\n",
            FILE__,__LINE__,f->addr());
        parse();
    }

	bool cache_value = true;
	/* this is commented out to prevent a failure in tampersStack, but
           this may be an incorrect approach to fixing the problem.
	if(frame_status(f->region(), f->addr()) < ParseFrame::PARSED) {
		// XXX prevent caching of blocks, extents for functions that
		// are actively being parsed. This prevents callbacks and other
		// functions called from within, e.g. parse_frame from setting
		// the caching flag and preventing later updates to the blocks()
		// vector during finalization.
		cache_value = false;
	}*/

    parsing_printf("[%s] finalizing %s (%lx)\n",
        FILE__,f->name().c_str(),f->addr());

    region_data * rd = _parse_data->findRegion(f->region());
    assert(rd);

    // finish delayed parsing and sorting
    Function::blocklist blocks = f->blocks_int();

    // is this the first time we've parsed this function?
    if (unlikely( !f->_extents.empty() )) {
        _parse_data->remove_extents(f->_extents);
        f->_extents.clear();
    }
    

    if(blocks.empty()) {
        f->_cache_valid = cache_value; // see above
        return;
    }
    
    auto bit = blocks.begin();
    FuncExtent * ext = NULL;
    Address ext_s = (*bit)->start();
    Address ext_e = ext_s;

    for( ; bit != blocks.end(); ++bit) {
        Block * b = *bit;
        if(b->start() > ext_e) {
            ext = new FuncExtent(f,ext_s,ext_e);
            parsing_printf("%lx extent [%lx,%lx)\n",f->addr(),ext_s,ext_e);
            f->_extents.push_back(ext);
            rd->funcsByRange.insert(ext);
            ext_s = b->start();
        }
        ext_e = b->end();
    }
    ext = new FuncExtent(f,ext_s,ext_e);
    parsing_printf("%lx extent [%lx,%lx)\n",f->addr(),ext_s,ext_e);
    rd->funcsByRange.insert(ext);
    f->_extents.push_back(ext);

    f->_cache_valid = cache_value; // see comment at function entry

    if (unlikely( f->obj()->defensiveMode())) {
        // add fallthrough edges for calls assumed not to be returning
        // whose fallthrough blocks we parsed anyway (this happens if 
        // the FT block is also a branch target)
        Function::edgelist & edges = f->_call_edge_list;
        for (Function::edgelist::iterator eit = edges.begin();
             eit != edges.end(); 
             eit++)
        {
            if (2 > (*eit)->src()->targets().size()) {
                Block *ft = _parse_data->findBlock((*eit)->src()->region(),
                                                   (*eit)->src()->end());
                if (ft && HASHDEF(f->_bmap,ft->start())) {
                    link((*eit)->src(),ft,CALL_FT,false);
                }
            }
        }
    }
}

void
Parser::finalize()
{
    ScopeLock<> l(finalize_lock);
    if(_parse_state < FINALIZED) {
        finalize_funcs(hint_funcs);
        finalize_funcs(discover_funcs);
        _parse_state = FINALIZED;
    }
}

void
Parser::finalize_funcs(vector<Function *> &funcs)
{
    vector<Function *>::iterator fit = funcs.begin();
    for( ; fit != funcs.end(); ++fit) {
        finalize(*fit);
    } 
}

void
Parser::record_func(Function *f) {
    if(!f) return;

    if(f->src() == HINT)
        hint_funcs.push_back(f);
    else
        discover_funcs.push_back(f);

    sorted_funcs.insert(f);

    _parse_data->record_func(f);
}

void
Parser::init_frame(ParseFrame & frame)
{
   Block * b = NULL;
    Block * split = NULL;

    if ( ! frame.func->_entry ) 
    {
        // Find or create a block
        b = block_at(frame.func, frame.func->addr(),split);
        if(b) {
            frame.leadersToBlock[frame.func->addr()] = b;
            frame.func->_entry = b;
            frame.seed = new ParseWorkElem(NULL,NULL,frame.func->addr(),true,false);
            frame.pushWork(frame.seed);
        } else {
            parsing_printf("[%s] failed to initialize parsing frame\n",
                FILE__);
            return;
        }
        if (split) {
            _pcb.splitBlock(split,b);
        }
    }

    // FIXME these operations should move into the actual parsing
    Address ia_start = frame.func->addr();
    unsigned size = 
     frame.codereg->offset() + frame.codereg->length() - ia_start;
    const unsigned char* bufferBegin =
     (const unsigned char *)(frame.func->isrc()->getPtrToInstruction(ia_start));
    InstructionDecoder dec(bufferBegin,size,frame.codereg->getArch());
    InstructionAdapter_t* ah = InstructionAdapter_t::makePlatformIA_IAPI(b->obj()->cs()->getArch(),
                                                                         dec, ia_start, frame.func->obj(),
									 frame.codereg, frame.func->isrc(), b);
	if(ah->isStackFramePreamble()) {
        frame.func->_no_stack_frame = false;
	}
    frame.func->_saves_fp = ah->savesFP();
    delete ah;
}

void ParseFrame::cleanup()
{
    for(unsigned i=0;i<work_bundles.size();++i)
        delete work_bundles[i];
    work_bundles.clear();
    if(seed)
        delete seed;
    seed = NULL;
}

namespace {
    inline Edge * bundle_call_edge(ParseWorkBundle * b)
    {
        if(!b) return NULL;

        vector<ParseWorkElem*> const& elems = b->elems();
        vector<ParseWorkElem*>::const_iterator it = elems.begin();
        for( ; it != elems.end(); ++it) {
            if((*it)->edge()->type() == CALL)
                return (*it)->edge();
        }
        return NULL;
    }

    /* 
     * Look up the next block for detection of straight-line
     * fallthrough edges into existing blocks.
     */
    inline std::pair<Address, Block*> get_next_block(
        Address addr,
        CodeRegion * codereg, 
        ParseData * _parse_data)
    {
        Block * nextBlock = NULL;
        Address nextBlockAddr;

        nextBlockAddr = numeric_limits<Address>::max();
        region_data * rd = _parse_data->findRegion(codereg);

        if((nextBlock = rd->blocksByRange.successor(addr)) &&
           nextBlock->start() > addr)
        {
            nextBlockAddr = nextBlock->start();   
        }

        return std::pair<Address,Block*>(nextBlockAddr,nextBlock);
    }
}

void
Parser::parse_frame(ParseFrame & frame, bool recursive) {
    /** Persistent intermediate state **/
    InstructionAdapter_t *ahPtr = NULL;
    ParseFrame::worklist_t & worklist = frame.worklist;
    dyn_hash_map<Address, Block *> & leadersToBlock = frame.leadersToBlock;
    Address & curAddr = frame.curAddr;
    Function * func = frame.func;
    dyn_hash_map<Address, bool> & visited = frame.visited;
    unsigned & num_insns = frame.num_insns;
    func->_cache_valid = false;

    /** Non-persistent intermediate state **/
    Address nextBlockAddr;
    Block * nextBlock;

    if (frame.status() == ParseFrame::UNPARSED) {
        parsing_printf("[%s] ==== starting to parse frame %lx ====\n",
            FILE__,frame.func->addr());
        // prevents recursion of parsing
        frame.func->_parsed = true;
    } else {
        parsing_printf("[%s] ==== resuming parse of frame %lx ====\n",
            FILE__,frame.func->addr());
        // Pull work that can be resumed off the delayedWork list
        std::map<ParseWorkElem *, Function *>::iterator iter;
        
        vector<ParseWorkElem *> updated;
        vector<ParseWorkElem *>::iterator uIter;

        for (iter = frame.delayedWork.begin();
                iter != frame.delayedWork.end();
                ++iter) {
            if (iter->second->_rs != UNSET) {
                frame.pushWork(iter->first);
                updated.push_back(iter->first);
            }
        }
        for (uIter = updated.begin(); uIter != updated.end(); ++uIter) {
            frame.delayedWork.erase(*uIter);
        }
    }


    frame.set_status(ParseFrame::PROGRESS);

    while(!worklist.empty()) {
        
        Block * cur = NULL;
        ParseWorkElem * work = frame.popWork();
        if (work->order() == ParseWorkElem::call) {
            Function * ct = NULL;

            if (!work->callproc()) {
	      // If we're not doing recursive traversal, skip *all* of the call edge processing.
	      // We don't want to create a stub. We'll handle the assumption of a fallthrough
	      // target for the fallthrough work element, not the call work element. Catch blocks
	      // will get excluded but that's okay; our heuristic is not reliable enough that I'm willing to deploy
	      // it when we'd only be detecting a non-returning callee by name anyway.
	      // --BW 12/2012
				if (!recursive) {
				    parsing_printf("[%s] non-recursive parse skipping call %lx->%lx\n",
								   FILE__, work->edge()->src()->lastInsnAddr(), work->target());
					continue;
				}
	      
                parsing_printf("[%s] binding call %lx->%lx\n",
                               FILE__,work->edge()->src()->lastInsnAddr(),work->target());
            
                pair<Function*,Edge*> ctp =
                    bind_call(frame,
                              work->target(),
                              work->edge()->src(),
                              work->edge());
                ct = ctp.first;

                work->mark_call();
            } else {
                ct = _parse_data->findFunc(frame.codereg,work->target());
            }

            if (recursive && ct &&
               (frame_status(ct->region(),ct->addr())==ParseFrame::UNPARSED || 
                frame_status(ct->region(),ct->addr())==ParseFrame::BAD_LOOKUP)) {
                // suspend this frame and parse the next
                parsing_printf("    [suspend frame %lx]\n", func->addr()); 
                frame.call_target = ct;
                frame.set_status(ParseFrame::CALL_BLOCKED);
                // need to re-visit this edge
                frame.pushWork(work);
		if (ahPtr) delete ahPtr;
                return;
            } else if (ct && work->tailcall()) {
                // XXX The target has been or is currently being parsed (else
                //     the previous conditional would have been taken),
                //     so if its return status is unset then this
                //     function has to take UNKNOWN
                if (func->_rs != RETURN) {
                    if (ct->_rs > NORETURN)
                      func->set_retstatus(ct->_rs);
                    else if (ct->_rs == UNSET)
                      frame.pushDelayedWork(work, ct);
                }
            }

            continue;
        } else if (work->order() == ParseWorkElem::call_fallthrough) {
            // check associated call edge's return status
            Edge * ce = bundle_call_edge(work->bundle());
            if (!ce) {
                // odd; no call edge in this bundle
                parsing_printf("[%s] unexpected missing call edge at %lx\n",
                        FILE__,work->edge()->src()->lastInsnAddr());
            } else {
                // check for system call FT
                Edge* edge = work->edge();
                Block::Insns blockInsns;
                edge->src()->getInsns(blockInsns);
                auto prev = blockInsns.rbegin();
                InstructionAPI::InstructionPtr prevInsn = prev->second;
                bool is_nonret = false;

                if (prevInsn->getOperation().getID() == e_syscall) {
                    Address src = edge->src()->lastInsnAddr();


                    // Need to determine if system call is non-returning
                    long int syscallNumber;
                    if (!getSyscallNumber(func, edge->src(), src, frame.codereg->getArch(), syscallNumber)) {
                        // If we cannot retrieve a syscall number, assume the syscall returns
                        parsing_printf("[%s] could not retrieve syscall edge, assuming returns\n", FILE__);
                    } else {
		        if (obj().cs()->nonReturningSyscall(syscallNumber)) {
			    is_nonret = true;
			}
		    }

                    if (is_nonret) {
                        parsing_printf("[%s] no fallthrough for non-returning syscall\n",
                                FILE__,
                                work->edge()->src()->lastInsnAddr());

                        // unlink tempsink fallthrough edge
                        Edge * remove = work->edge();
                        remove->src()->removeTarget(remove);
                        factory().destroy_edge(remove);
                        continue;
                    }
                } else {
                    Address target = ce->trg()->start();
                    Function * ct = _parse_data->findFunc(frame.codereg,target);
                    bool is_plt = false;

                    // check if associated call edge's return status is still unknown
                    if (ct && (ct->_rs == UNSET) ) {
                        // Delay parsing until we've finished the corresponding call edge
                        parsing_printf("[%s] Parsing FT edge %lx, corresponding callee (%s) return status unknown; delaying work\n",
                                __FILE__,
                                work->edge()->src()->lastInsnAddr(),
                                ct->name().c_str());

                        // Add work to ParseFrame's delayed list
                        frame.pushDelayedWork(work, ct);

                        // Continue other work for this frame
                        continue; 
                    }

                    is_plt = HASHDEF(plt_entries,target);

                    // CodeSource-defined tests 
                    is_nonret = obj().cs()->nonReturning(target);
                    if (is_nonret) {
                        parsing_printf("\t Disallowing FT edge: CodeSource reports nonreturning\n");
                    }
                    if (!is_nonret && is_plt) {
                        is_nonret |= obj().cs()->nonReturning(plt_entries[target]);
                        if (is_nonret) {
                            parsing_printf("\t Disallowing FT edge: CodeSource reports PLT nonreturning\n");
                        }
                    }
                    // Parsed return status tests
                    if (!is_nonret && !is_plt && ct) {
                        is_nonret |= (ct->retstatus() == NORETURN);
                        if (is_nonret) {
                            parsing_printf("\t Disallowing FT edge: function is non-returning\n");
                        }
                    }
                    // Call-stack tampering tests
                    if (unlikely(!is_nonret && frame.func->obj()->defensiveMode() && ct)) {
                        is_nonret |= (ct->retstatus() == UNKNOWN);
                        if (is_nonret) {
                            parsing_printf("\t Disallowing FT edge: function in "
                                    "defensive binary may not return\n");
                            mal_printf("Disallowing FT edge: function %lx in "
                                    "defensive binary may not return\n", ct->addr());
                        } else {
                            StackTamper ct_tamper = ct->tampersStack();
                            is_nonret |= (TAMPER_NONZERO == ct_tamper);
                            is_nonret |= (TAMPER_ABS == ct_tamper);
                            if (is_nonret) {
                                mal_printf("Disallowing FT edge: function at %lx "
                                        "tampers with its stack\n", ct->addr());
                                parsing_printf("\t Disallowing FT edge: function "
                                        "tampers with its stack\n");
                            }
                        }
                    }
                    if (is_nonret) {
                        parsing_printf("[%s] no fallthrough for non-returning call "
                                "to %lx at %lx\n",FILE__,target,
                                work->edge()->src()->lastInsnAddr());

                        // unlink tempsink fallthrough edge
                        Edge * remove = work->edge();
                        remove->src()->removeTarget(remove);
                        factory().destroy_edge(remove);
			continue;
                    } else
		        // Invalidate cache_valid for all sharing functions
                        invalidateContainingFuncs(func, ce->src());                
                }
            }
        } else if (work->order() == ParseWorkElem::seed_addr) {
            cur = leadersToBlock[work->target()];
        } else if (work->order() == ParseWorkElem::resolve_jump_table) {
	    // resume to resolve jump table 
	    parsing_printf("... continue parse indirect jump at %lx\n", work->ah()->getAddr());
	    Block *nextBlock = work->cur();
	    if (nextBlock->last() != work->ah()->getAddr()) {
	        // The block has been split
	        region_data * rd = _parse_data->findRegion(frame.codereg);
		set<Block*> blocks;
		rd->blocksByRange.find(work->ah()->getAddr(), blocks);
		for (auto bit = blocks.begin(); bit != blocks.end(); ++bit) {
		    if ((*bit)->last() == work->ah()->getAddr()) {
		        nextBlock = *bit;
			break;
		    }
		}

	    }
	    ProcessCFInsn(frame,nextBlock,work->ah());
            continue;
	}
        // call fallthrough case where we have already checked that
        // the target returns. this is used in defensive mode.
        else if (work->order() == ParseWorkElem::checked_call_ft) {
            Edge* ce = bundle_call_edge(work->bundle());
            if (ce != NULL) {
                invalidateContainingFuncs(func, ce->src());
            } else {
                parsing_printf("[%s] unexpected missing call edge at %lx\n",
                        FILE__,work->edge()->src()->lastInsnAddr());
            }
        }
        
        if (NULL == cur) {
            pair<Block*,Edge*> newedge =
                add_edge(frame,
                         frame.func,
                         work->edge()->src(),
                         work->target(),
                         work->edge()->type(),
                         work->edge());
            cur = newedge.first;
        }

        if (HASHDEF(visited,cur->start()))
        {
            parsing_printf("[%s] skipping locally parsed target at %lx\n",
                FILE__,work->target());
            continue;
        } 
        visited[cur->start()] = true;
        leadersToBlock[cur->start()] = cur;

        if (!cur->_parsed)
        {
            parsing_printf("[%s] parsing block %lx\n",
                FILE__,cur->start());
            if (frame.func->obj()->defensiveMode()) {
                mal_printf("new block at %lx (0x%lx)\n",cur->start(), cur);
            }

            cur->_parsed = true;
            curAddr = cur->start();
        } else {
            parsing_printf("[%s] deferring parse of shared block %lx\n",
                FILE__,cur->start());
            if (func->_rs < UNKNOWN) {
                // we've parsed into another function, if we've parsed
                // into it's entry point, set retstatus to match it
                Function * other_func = _parse_data->findFunc(
                    func->region(), cur->start());
                if (other_func && other_func->retstatus() > UNKNOWN) {
                  func->set_retstatus(other_func->retstatus());
                } else {
                  func->set_retstatus(UNKNOWN);
                }
            }
	    // The edge to this shared block is changed from 
	    // "going to sink" to going to this shared block.
	    // This changes the function boundary, so we need to
	    // invalidate the cache.
	    func->_cache_valid = false;
            continue;
        }

        /*
         * External consumers of parsing may have invoked finalizing
         * methods from callback routines. Ensure that the caches
         * are invalidated because a new block is being added to the
         * function's view
         */
        func->_cache_valid = false;

        // NB Using block's region() here because it may differ from the
        //    function's if control flow has jumped to another region
        unsigned size = 
            cur->region()->offset() + cur->region()->length() - curAddr;
        const unsigned char* bufferBegin =
          (const unsigned char *)(func->isrc()->getPtrToInstruction(curAddr));
        InstructionDecoder dec(bufferBegin,size,frame.codereg->getArch());

        if (!ahPtr)
            ahPtr = InstructionAdapter_t::makePlatformIA_IAPI(func->obj()->cs()->getArch(), dec, curAddr, func->obj(), 
                        cur->region(), func->isrc(), cur);
        else
            ahPtr->reset(dec,curAddr,func->obj(),
                         cur->region(), func->isrc(), cur);
       
        InstructionAdapter_t * ah = ahPtr; 

        using boost::tuples::tie;
        tie(nextBlockAddr,nextBlock) = get_next_block(
            frame.curAddr, frame.codereg, _parse_data);

        bool isNopBlock = ah->isNop();

        while(true) {
            curAddr = ah->getAddr();
            /** Check for straight-line fallthrough **/
            if (curAddr == nextBlockAddr) {
                parsing_printf("[%s] straight-line parse into block at %lx\n",
                    FILE__,curAddr);
                if (frame.func->obj()->defensiveMode()) {
                    mal_printf("straight-line parse into block at %lx\n",curAddr);
                }
                ah->retreat();
                curAddr = ah->getAddr();

                end_block(cur,ah);
                pair<Block*,Edge*> newedge =
                    add_edge(frame,frame.func,cur,
                             nextBlockAddr,FALLTHROUGH,NULL);
        
                if (!HASHDEF(visited,nextBlockAddr) &&
                   !HASHDEF(leadersToBlock,nextBlockAddr)) {
                    parsing_printf("[%s:%d] pushing %lx onto worklist\n",
                                   FILE__,__LINE__,nextBlockAddr);

                    frame.pushWork(
                        frame.mkWork(
                            NULL,
                            newedge.second,
                            nextBlockAddr,
                            true,
                            false)
                    );
                    /* preserved as example of leaky code; use mkWork instead
                    frame.pushWork(
                        new ParseWorkElem(
                            NULL,
                            newedge.second,
                            nextBlockAddr,
                            true,
                            false)
                        );
                    */
                    leadersToBlock[nextBlockAddr] = nextBlock;
                }
                break;
            } else if (curAddr > nextBlockAddr) {
                parsing_printf("[%s:%d] inconsistent instruction stream: "
                               "%lx is within [%lx,%lx)\n",
                    FILE__,__LINE__,curAddr,
                    nextBlock->start(),nextBlock->end());
                Address prev_insn; 
                if (nextBlock->consistent(curAddr, prev_insn)) {
                    // The two overlapping blocks aligned.
                    // We need to split the large block, and create new edge to the later block
                    Block* new_block = split_block(frame.func, nextBlock, curAddr, prev_insn);
                    ah->retreat();
                    end_block(cur, ah);
                    add_edge(frame, frame.func, cur, curAddr, FALLTHROUGH, NULL);
                    leadersToBlock[curAddr] = new_block;

                    // We break from this loop because no need more stright-line parsing
                    break;
                }

                // NB "cur" hasn't ended, so its range may
                // not look like it overlaps with nextBlock
                _pcb.overlapping_blocks(cur,nextBlock);

                tie(nextBlockAddr,nextBlock) = 
                    get_next_block(frame.curAddr, frame.codereg, _parse_data);
            }

            // per-instruction callback notification 
            ParseCallback::insn_details insn_det;
            insn_det.insn = ah;
	     
                parsing_printf("[%s:%d] curAddr 0x%lx: %s \n",
                    FILE__,__LINE__,curAddr, insn_det.insn->getInstruction()->format().c_str() );

            if (func->_is_leaf_function) {
                Address ret_addr;
    	        func->_is_leaf_function = !(insn_det.insn->isReturnAddrSave(ret_addr));
                    parsing_printf("[%s:%d] leaf %d funcname %s \n",
                        FILE__,__LINE__,func->_is_leaf_function, func->name().c_str());
                if (!func->_is_leaf_function) func->_ret_addr = ret_addr;	
            }
		
            _pcb.instruction_cb(func,cur,curAddr,&insn_det);

            if (isNopBlock && !ah->isNop()) {
                ah->retreat();

                end_block(cur,ah);
                pair<Block*,Edge*> newedge =
                    add_edge(frame,frame.func,cur,curAddr,FALLTHROUGH,NULL);
                Block * targ = newedge.first;
  
                parsing_printf("[%s:%d] nop-block ended at %lx\n",
                    FILE__,__LINE__,curAddr); 
                if (targ && !HASHDEF(visited,targ->start())) {
                    parsing_printf("[%s:%d] pushing %lx onto worklist\n",
                        FILE__,__LINE__,targ->start());

                    frame.pushWork(
                        frame.mkWork(
                            NULL,
                            newedge.second,
                            targ->start(),
                            true,
                            false)
                        );
                    leadersToBlock[targ->start()] = targ; 
                }
                break;
            }
            
            /** Particular instruction handling (calls, branches, etc) **/
            ++num_insns; 

            if(ah->hasCFT()) {
	       if (ah->isIndirectJump()) {
	           // Create a work element to represent that
		   // we will resolve the jump table later
                   end_block(cur,ah);
                   frame.pushWork( frame.mkWork( work->bundle(), cur, ah));
	       } else {
	           ProcessCFInsn(frame,cur,ah);
	       }
               break;
            } else if (func->_saves_fp &&
                       func->_no_stack_frame &&
                       ah->isFrameSetupInsn()) { // isframeSetup is expensive
               func->_no_stack_frame = false;
            } else if (ah->isLeave()) {
                func->_no_stack_frame = false;
            } else if ( ah->isAbort() ) {
                // 4. `abort-causing' instructions
                end_block(cur,ah);
                //link(cur, _sink, DIRECT, true);
                break; 
            } else if ( ah->isInvalidInsn() ) {
                // 4. Invalid or `abort-causing' instructions
                end_block(cur,ah);
                link(cur, _sink, DIRECT, true);
                break; 
            } else if ( ah->isInterruptOrSyscall() ) {
                // 5. Raising instructions
                end_block(cur,ah);

                pair<Block*,Edge*> newedge =
                    add_edge(frame,frame.func,cur,ah->getNextAddr(),FALLTHROUGH,NULL);
                Block * targ = newedge.first;
   
                if (targ && !HASHDEF(visited,targ->start()) &&
                            !HASHDEF(leadersToBlock,targ->start())) {
                    parsing_printf("[%s:%d] pushing %lx onto worklist\n",
                                   FILE__,__LINE__,targ->start());

                    frame.pushWork(
                        frame.mkWork(
                            NULL,
                            newedge.second,
                            targ->start(),
                            true,
                            false)
                        );
                    leadersToBlock[targ->start()] = targ; 
                }
                if (unlikely(func->obj()->defensiveMode())) {
                    fprintf(stderr,"parsed bluepill insn sysenter or syscall "
                            "in defensive mode at %lx\n",curAddr);
                }
                break;
            } else if (unlikely(func->obj()->defensiveMode())) {
                if (!_pcb.hasWeirdInsns(func) && ah->isGarbageInsn()) {
                    // add instrumentation at this addr so we can
                    // extend the function if this really executes
                    ParseCallback::default_details det(
                        (unsigned char*) cur->region()->getPtrToInstruction(cur->lastInsnAddr()),
                        cur->end() - cur->lastInsnAddr(),
                        true);
                    _pcb.abruptEnd_cf(cur->lastInsnAddr(),cur,&det);
                    _pcb.foundWeirdInsns(func);
                    end_block(cur,ah);
                    // allow invalid instructions to end up as a sink node.
		    link(cur, _sink, DIRECT, true);
                    break;
                } else if (ah->isNopJump()) {
                    // patch the jump to make it a nop, and re-set the 
                    // instruction adapter so we parse the instruction
                    // as a no-op this time, allowing the subsequent
                    // instruction to be parsed correctly
                    mal_printf("Nop jump at %lx, changing it to nop\n",ah->getAddr());
                    _pcb.patch_nop_jump(ah->getAddr());
                    unsigned bufsize = 
                        func->region()->offset() + func->region()->length() - ah->getAddr();
                    const unsigned char* bufferBegin = (const unsigned char *)
                        (func->isrc()->getPtrToInstruction(ah->getAddr()));
                    dec = InstructionDecoder
                        (bufferBegin, bufsize, frame.codereg->getArch());
                    ah->reset(dec, curAddr, func->obj(), 
		              func->region(), func->isrc(), cur);
                } else {
                    entryID id = ah->getInstruction()->getOperation().getID();
                    switch (id) {
                        case e_rdtsc:
                            fprintf(stderr,"parsed bluepill insn rdtsc at %lx\n",curAddr);
                            break;
                        case e_sldt:
                            fprintf(stderr,"parsed bluepill insn sldt at %lx\n",curAddr);
                            break;
                        default:
                            break;
                    }
                }
            } else {
                // default
            }

            /** Check for overruns of valid address space **/
            if (!is_code(func,ah->getNextAddr())) {
                parsing_printf("[%s] next insn %lx is invalid\n",
                               FILE__,ah->getNextAddr());

                end_block(cur,ah);
                // We need to tag the block with a sink edge
                link(cur, _sink, DIRECT, true);
                break;
            } else if (!cur->region()->contains(ah->getNextAddr())) {
                parsing_printf("[%s] next address %lx is outside [%lx,%lx)\n",
                    FILE__,ah->getNextAddr(),
                    cur->region()->offset(),
                    cur->region()->offset()+cur->region()->length());
                end_block(cur,ah);
                // We need to tag the block with a sink edge
                link(cur, _sink, DIRECT, true);
                break;
            }
            ah->advance();
        }
    }
    if (ahPtr) delete ahPtr;
    // Check if parsing is complete
    if (!frame.delayedWork.empty()) {
        frame.set_status(ParseFrame::FRAME_DELAYED);
        return; 
    }

    /** parsing complete **/
    if (HASHDEF(plt_entries,frame.func->addr())) {
//        if (obj().cs()->nonReturning(frame.func->addr())) {
        if (obj().cs()->nonReturning(plt_entries[frame.func->addr()])) {        
            frame.func->set_retstatus(NORETURN);
        } else {
            frame.func->set_retstatus(UNKNOWN);
        }

        // Convenience -- adopt PLT name
        frame.func->_name = plt_entries[frame.func->addr()];
    } else if (frame.func->_rs == UNSET) {
        frame.func->set_retstatus(NORETURN);
    }

    frame.set_status(ParseFrame::PARSED);

    if (unlikely(obj().defensiveMode())) {
       // calculate this after setting the function to PARSED, so that when
       // we finalize the function we'll actually save the results and won't 
       // re-finalize it
       func->tampersStack(); 
       _pcb.newfunction_retstatus( func );
    }
}

void
Parser::end_block(Block * b, InstructionAdapter_t * ah)
{
    b->_lastInsn = ah->getAddr();
    b->updateEnd(ah->getNextAddr());

    record_block(b);
}

void
Parser::record_block(Block *b)
{
    parsing_printf("[%s:%d] recording block [%lx,%lx)\n",
        FILE__,__LINE__,b->start(),b->end());
    _parse_data->record_block(b->region(),b);
}


Block *
Parser::block_at(
    Function * owner,
    Address addr,
    Block * & split)
{
    set<Block *> overlap;
    Block * exist = NULL;
    Block * ret = NULL;
    Block * inconsistent = NULL;
    Address prev_insn = 0;

    split = NULL;

    CodeRegion *cr;
    if(owner->region()->contains(addr))
        cr = owner->region();
    else
        cr = _parse_data->reglookup(owner->region(),addr);

    if(!is_code(owner,addr)) {
        parsing_printf("[%s] block address %lx rejected by isCode()\n",
            FILE__,addr);
        return NULL;
    }

    if(NULL == (exist = _parse_data->findBlock(cr,addr))) {
        _parse_data->findBlocks(cr,addr,overlap);
        if(overlap.size() > 1)
            parsing_printf("[%s] address %lx overlapped by %d blocks\n",
                FILE__,addr,overlap.size());

        /* Platform specific consistency test: 
           generally checking for whether the address is an
           instruction boundary in a block */
        for(set<Block *>::iterator sit=overlap.begin();sit!=overlap.end();++sit)
        {
            Block * b = *sit;

            // consistent will fill in prev_insn with the address of the
            // instruction preceeding addr if addr is consistent
            if(b->consistent(addr,prev_insn)) {
                exist = b;
                break;   
            } else {
                parsing_printf("[%s] %lx is inconsistent with [%lx,%lx)\n", 
                    FILE__,addr,b->start(),b->end());
                if(inconsistent) {
                    parsing_printf("   multiple inconsistent blocks!\n");
                }
                inconsistent = b;
            }
        }
    }

    if(exist) {
        if(exist->start() == addr) {
            parsing_printf("[%s] block %lx exists\n",
                FILE__,addr);
            ret = exist;
        }
        else {
            parsing_printf("[%s] address %lx splits [%lx,%lx) (%p)\n",
               FILE__,addr,exist->start(),exist->end(),exist);
            if (owner->obj()->defensiveMode()) {
                mal_printf("new block at %lx splits [%lx %lx)\n",
                           addr, exist->start(), exist->end());
            }
            split = exist;
            ret = split_block(owner,exist,addr,prev_insn);
        }
    } else {
        ret = factory()._mkblock(owner,cr,addr);
        record_block(ret);
        _pcb.addBlock(owner, ret);
    }

    if(unlikely(inconsistent)) {
       _pcb.overlapping_blocks(ret,inconsistent); 
    }

    return ret;
}

pair<Block *,Edge *>
Parser::add_edge(
    ParseFrame & frame,
    Function * owner,
    Block * src,
    Address dst,
    EdgeTypeEnum et,
    Edge * exist)
{
    Block * split = NULL;
    Block * ret = NULL;
    Edge * newedge = NULL;
    pair<Block *, Edge *> retpair((Block *) NULL, (Edge *) NULL);

    if(!is_code(owner,dst)) {
        parsing_printf("[%s:%d] target address %lx rejected by isCode()\n",
            FILE__, __LINE__, dst);
        return retpair;
    }

    ret = block_at(owner,dst,split);
    retpair.first = ret;

    if(split == src) {
        // special case -- same block
        src = ret;
    }

    if(split && HASHDEF(frame.visited,split->start())) {
        // prevent "delayed parsing" of extant block if 
        // this frame has already visited it
        frame.visited[ret->start()] = true;
        frame.leadersToBlock[ret->start()] = ret;
    }

    if(NULL == exist) {
        newedge = link(src,ret,et,false);
        retpair.second = newedge;
    } else {
        relink(exist,src,ret);
        retpair.second = exist;
    }

    return retpair;
}

Block *
Parser::split_block(
    Function * owner, 
    Block *b, 
    Address addr, 
    Address previnsn)
{
    Block * ret;
    CodeRegion * cr;
    bool isRetBlock = false;
    if(owner->region()->contains(b->start()))
        cr = owner->region();
    else
        cr = _parse_data->reglookup(owner->region(),b->start());
    region_data * rd = _parse_data->findRegion(cr);

    // enable for extra-safe testing, but callers are responsbible
    // assert(b->consistent(addr);

    ret = factory()._mkblock(owner,cr,addr);

    // move out edges
    vector<Edge *> & trgs = b->_trglist;
    vector<Edge *>::iterator tit = trgs.begin(); 
    for(;tit!=trgs.end();++tit) {
        Edge *e = *tit;
        e->_source = ret;
        ret->_trglist.push_back(e);
    }
    if (!trgs.empty() && RET == (*trgs.begin())->type()) {
       isRetBlock = true;
    }
    trgs.clear();
    ret->updateEnd(b->_end);
    ret->_lastInsn = b->_lastInsn;
    ret->_parsed = true;
    link(b,ret,FALLTHROUGH,false); 

    record_block(ret);

    // b's range has changed
    rd->blocksByRange.remove(b);
    b->updateEnd(addr);
    b->_lastInsn = previnsn;
    rd->blocksByRange.insert(b); 
    // Any functions holding b that have already been finalized
    // need to have their caches invalidated so that they will
    // find out that they have this new 'ret' block
    std::set<Function*> prev_owners;
    rd->findFuncs(b->start(),prev_owners);
    for(std::set<Function*>::iterator oit = prev_owners.begin();
        oit != prev_owners.end(); ++oit)
    {
        Function * po = *oit;
        if (po->_cache_valid) {
           po->_cache_valid = false;
           parsing_printf("[%s:%d] split of [%lx,%lx) invalidates cache of "
                   "func at %lx\n",
           FILE__,__LINE__,b->start(),b->end(),po->addr());
        }
        if (isRetBlock) {
           po->_retBL.clear(); //could remove b from the vector instead of clearing it, not sure what's cheaper
        }
    }
    // KEVINTODO: study performance impact of this callback
    _pcb.splitBlock(b,ret);

    return ret;
 }

pair<Function *,Edge*>
Parser::bind_call(ParseFrame & frame, Address target, Block * cur, Edge * exist)
{
    Function * tfunc = NULL;
    Block * tblock = NULL;
    FuncSource how = RT;
    if(frame.func->_src == GAP || frame.func->_src == GAPRT)
        how = GAPRT;

    // look it up
    tfunc = _parse_data->get_func(frame.codereg,target,how);
    if(!tfunc) {
        parsing_printf("[%s:%d] can't bind call to %lx\n",
            FILE__,__LINE__,target);
        return pair<Function*,Edge*>((Function *) NULL,exist);
    }

    // add an edge
    pair<Block*,Edge*> ret = add_edge(frame,tfunc,cur,target,CALL,exist);
    tblock = ret.first;
    if(!tblock) {
        parsing_printf("[%s:%d] can't bind call to %lx\n",
            FILE__,__LINE__,target);
        return pair<Function*,Edge*>((Function *) NULL,exist);
    }

    return pair<Function*,Edge*>(tfunc,ret.second);
}

Function *
Parser::findFuncByEntry(CodeRegion *r, Address entry)
{
    if(_parse_state < PARTIAL) {
        parsing_printf("[%s:%d] Parser::findFuncByEntry([%lx,%lx),%lx) "
                       "forced parsing\n",
            FILE__,__LINE__,r->low(),r->high(),entry);
        parse();
    }
    return _parse_data->findFunc(r,entry);
}

int 
Parser::findFuncs(CodeRegion *r, Address addr, set<Function *> & funcs)
{
    if(_parse_state < COMPLETE) {
        parsing_printf("[%s:%d] Parser::findFuncs([%lx,%lx),%lx,...) "
                       "forced parsing\n",
            FILE__,__LINE__,r->low(),r->high(),addr);
        parse();
    }
    if(_parse_state < FINALIZED) {
        parsing_printf("[%s:%d] Parser::findFuncs([%lx,%lx),%lx,...) "
                       "forced finalization\n",
            FILE__,__LINE__,r->low(),r->high(),addr);
        finalize();
    }
    return _parse_data->findFuncs(r,addr,funcs);
}

int 
Parser::findFuncs(CodeRegion *r, Address start, Address end, set<Function *> & funcs)
{
    if(_parse_state < COMPLETE) {
        parsing_printf("[%s:%d] Parser::findFuncs([%lx,%lx),%lx,%lx) "
                       "forced parsing\n",
            FILE__,__LINE__,r->low(),r->high(),start,end);
        parse();
    }
    if(_parse_state < FINALIZED) {
        parsing_printf("[%s:%d] Parser::findFuncs([%lx,%lx),%lx,%lx) "
                       "forced finalization\n",
            FILE__,__LINE__,r->low(),r->high(),start,end);
        finalize();
    }
    return _parse_data->findFuncs(r,start,end,funcs);
}

Block *
Parser::findBlockByEntry(CodeRegion *r, Address entry)
{
    if(_parse_state < PARTIAL) {
        parsing_printf("[%s:%d] Parser::findBlockByEntry([%lx,%lx),%lx) "
                       "forced parsing\n",
            FILE__,__LINE__,r->low(),r->high(),entry);
        parse();
    }
    return _parse_data->findBlock(r,entry);
}

Block *
Parser::findNextBlock(CodeRegion *r, Address addr)
{
    if(_parse_state < PARTIAL) {
        parsing_printf("[%s:%d] Parser::findBlockByEntry([%lx,%lx),%lx) "
                       "forced parsing\n",
            FILE__,__LINE__,r->low(),r->high(),addr);
        parse();
    }
    return _parse_data->findRegion(r)->get_next_block(addr).second;
}

int
Parser::findBlocks(CodeRegion *r, Address addr, set<Block *> & blocks)
{
    if(_parse_state < COMPLETE) {
        parsing_printf("[%s:%d] Parser::findBlocks([%lx,%lx),%lx,...) "
                       "forced parsing\n",
            FILE__,__LINE__,r->low(),r->high(),addr);
        parse();
    }
    return _parse_data->findBlocks(r,addr,blocks); 
}

// find blocks without parsing.
int Parser::findCurrentBlocks(CodeRegion* cr, Address addr, 
                                std::set<Block*>& blocks) {
    return _parse_data->findBlocks(cr, addr, blocks);
}

int Parser::findCurrentFuncs(CodeRegion * cr, Address addr, std::set<Function*> &funcs) {
    return _parse_data->findFuncs(cr, addr, funcs);
}

Edge*
Parser::link(Block *src, Block *dst, EdgeTypeEnum et, bool sink)
{
    assert(et != NOEDGE);
    Edge * e = factory()._mkedge(src,dst,et);
    e->_type._sink = sink;
    src->_trglist.push_back(e);
    dst->_srclist.push_back(e);
    _pcb.addEdge(src, e, ParseCallback::target);
    _pcb.addEdge(dst, e, ParseCallback::source);
    return e;
}

/* 
 * During parsing, all edges are temporarily linked to the _tempsink
 * block. Adding and then removing edges to and from this block is
 * wasteful, especially given that removal is an O(N) operation with
 * vector storage. This call thus does not record edges into the sink.
 * These edges are guaranteed to have been relinked when parsing is
 * in state COMPLETE.
 *
 * NB This introduces an inconsistency into the CFG invariant that all
 *    targets of an edge have that edge in their source list if the
 *    data structures are queried during parsing. Extenders of 
 *    parsing callbacks are the only ones who can see this state;
 *    they have been warned and should know better.
 */
Edge*
Parser::link_tempsink(Block *src, EdgeTypeEnum et)
{
    Edge * e = factory()._mkedge(src,_sink,et);
    e->_type._sink = true;
    src->_trglist.push_back(e);
    return e;
}

void
Parser::relink(Edge * e, Block *src, Block *dst)
{
    bool addSrcAndDest = true;
    if(src != e->src()) {
        e->src()->removeTarget(e);
        _pcb.removeEdge(e->src(), e, ParseCallback::target);
        e->_source = src;
        src->addTarget(e);
        _pcb.addEdge(src, e, ParseCallback::target);
        addSrcAndDest = false;
    }
    if(dst != e->trg()) { 
        if(e->trg() != _sink) {
            e->trg()->removeSource(e);
            _pcb.removeEdge(e->trg(), e, ParseCallback::source);
            addSrcAndDest = false;
        }
        e->_target = dst;
        dst->addSource(e);
        _pcb.addEdge(dst, e, ParseCallback::source);
        if (addSrcAndDest) {
            // We're re-linking a sinkEdge to be a non-sink edge; since 
            // we don't inform PatchAPI of temporary sinkEdges, we have
            // to add both the source AND target edges
            _pcb.addEdge(src, e, ParseCallback::target);
        }
    }

    e->_type._sink = (dst == _sink);
}

ParseFrame::Status
Parser::frame_status(CodeRegion * cr, Address addr)
{
    // XXX parsing frames may have been cleaned up, but status
    //     is always cached
    return _parse_data->frameStatus(cr,addr);
}

void
Parser::remove_func(Function *func)
{
    if (sorted_funcs.end() != sorted_funcs.find(func)) {
        sorted_funcs.erase(func);
    }
    if (HINT == func->src()) {
        for (unsigned fidx=0; fidx < hint_funcs.size(); fidx++) {
            if (hint_funcs[fidx] == func) {
                hint_funcs[fidx] = hint_funcs[hint_funcs.size()-1];
                hint_funcs.pop_back();
                break;
            }
        }
    }
    else {
        for (unsigned fidx=0; fidx < discover_funcs.size(); fidx++) {
            if (discover_funcs[fidx] == func) {
                discover_funcs[fidx] = discover_funcs[discover_funcs.size()-1];
                discover_funcs.pop_back();
                break;
            }
        }
    }
    
    _parse_data->remove_func(func);
}

void
Parser::remove_block(Dyninst::ParseAPI::Block *block)
{
    _parse_data->remove_block(block);
}

void Parser::move_func(Function *func, Address new_entry, CodeRegion *new_reg)
{
    region_data *reg_data = _parse_data->findRegion(func->region());
    reg_data->funcsByAddr.erase(func->addr());

    reg_data = _parse_data->findRegion(new_reg);
    reg_data->funcsByAddr[new_entry] = func;
}

void Parser::invalidateContainingFuncs(Function *owner, Block *b)
{
    CodeRegion * cr;
    if(owner->region()->contains(b->start()))
        cr = owner->region();
    else
        cr = _parse_data->reglookup(owner->region(),b->start());
    region_data * rd = _parse_data->findRegion(cr);
    

    // Any functions holding b that have already been finalized
    // need to have their caches invalidated so that they will
    // find out that they have this new 'ret' block
    std::set<Function*> prev_owners;
    rd->findFuncs(b->start(),prev_owners);
    for(std::set<Function*>::iterator oit = prev_owners.begin();
        oit != prev_owners.end(); ++oit)
    {
        Function * po = *oit;
        po->_cache_valid = false;
        parsing_printf("[%s:%d] split of [%lx,%lx) invalidates cache of "
                       "func at %lx\n",
                       FILE__,__LINE__,b->start(),b->end(),po->addr());
    }   
}

/* Add ParseFrames waiting on func back to the work queue */
void Parser::resumeFrames(Function * func, vector<ParseFrame *> & work)
{
    // If we do not know the function's return status, don't put its waiters back on the worklist
    if (func->_rs == UNSET) { 
        parsing_printf("[%s] %s return status unknown, cannot resume waiters\n",
                __FILE__,
                func->name().c_str());
        return; 
    }

    // When a function's return status is set, all waiting frames back into the worklist
    map<Function *, set<ParseFrame *> >::iterator iter = delayedFrames.find(func);
    if (iter == delayedFrames.end()) {
        // There were no frames waiting, ignore
        parsing_printf("[%s] %s return status %d, no waiters\n",
                __FILE__,
                func->name().c_str(),
                func->_rs);
        return;
    } else {
        parsing_printf("[%s] %s return status %d, undelaying waiting functions\n",
                __FILE__,
                func->name().c_str(),
                func->_rs);
        // Add each waiting frame back to the worklist
        set<ParseFrame *> vec = iter->second;
        for (set<ParseFrame *>::iterator fIter = vec.begin(); 
                fIter != vec.end();
                ++fIter) {
            work.push_back(*fIter);
        }
        // remove func from delayedFrames map
        delayedFrames.erase(func);
    }
}

bool Parser::getSyscallNumber(Function * /*func*/,
        Block * block,
        Address /*addr*/,
        Architecture arch,
        long int & val)
{
    val = -1;

    // In the common case, the setup of system call number
    // is a mov in the previous instruction. We don't currently look elsewhere.
    // In the future, could use slicing and symeval to find the value of
    // this register at the system call (as unstrip does).
    Block::Insns blockInsns;
    block->getInsns(blockInsns);
    auto prevPair = ++(blockInsns.rbegin());
    InstructionAPI::InstructionPtr prevInsn = prevPair->second;
    if (prevInsn->getOperation().getID() != e_mov) {
        return false;
    }

    MachRegister syscallNumberReg = MachRegister::getSyscallNumberReg(arch);
    if (syscallNumberReg == InvalidReg) {
        return false;
    }
    InstructionAPI::RegisterAST* regAST = new InstructionAPI::RegisterAST(syscallNumberReg);
    InstructionAPI::RegisterAST::Ptr regASTPtr = InstructionAPI::RegisterAST::Ptr(regAST);

    std::vector<InstructionAPI::Operand> operands;
    prevInsn->getOperands(operands);
    for (unsigned i = 0; i < operands.size(); i++) {
        if (!operands[i].isWritten(regASTPtr)) {
            InstructionAPI::Expression::Ptr value = operands[i].getValue();
            InstructionAPI::Result res = value->eval();
            if (res.defined) {
                val = res.convert<Offset>();
            }
        }
    }

    return (val != -1);
}

