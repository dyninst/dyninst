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

#include "Parser.h"

#if defined(_OPENMP)
#include <omp.h>
#endif

#include <vector>
#include <limits>
#include <algorithm>
// For Mutex
#define PROCCONTROL_EXPORTS

#include "dyntypes.h"

#include "CodeObject.h"
#include "CFGFactory.h"
#include "ParseCallback.h"
#include "CFG.h"
#include "util.h"
#include "debug_parse.h"
#include "IndirectAnalyzer.h"

#include <boost/timer/timer.hpp>
#include <fstream>

#include "tbb/concurrent_vector.h"

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
    _parse_state(UNPARSED)
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
    }

    if (_parse_state != UNPARSEABLE) {
        // check whether regions overlap
        vector<CodeRegion *> const& regs = obj.cs()->regions();
        vector<CodeRegion *> copy(regs.begin(),regs.end());
        sort(copy.begin(),copy.end(),less_cr());

        // allocate a sink block -- region is arbitrary
        _sink = new Block(&_obj, regs[0], std::numeric_limits<Address>::max());

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

        if(overlap) {
            _parse_data = new OverlappingParseData(this,copy);
            assert(_sink == record_block(_sink));
            return;
        }
    }
    _parse_data = new StandardParseData(this);    
    if (_parse_state != UNPARSEABLE) {
        assert(_sink == record_block(_sink));
    }
}

ParseFrame::~ParseFrame()
{
    cleanup(); // error'd frames still need cleanup
}

Parser::~Parser()
{
    if(_parse_data)
        delete _parse_data;

    for(auto fit = frames.begin() ; fit != frames.end(); ++fit)
        delete *fit;

    frames.clear();
}

    void
Parser::add_hint(Function * f)
{
    _parse_data->record_func(f);
    record_func(f);
}

    template <typename T>
std::string pair_to_string(T pr)
{
    std::stringstream s;
    s << pr.first << ", " << pr.second;
    return s.str();
}


    void
Parser::parse()
{
    parsing_printf("[%s:%d] parse() called on Parser %p with state %d\n",
            FILE__,__LINE__,this, _parse_state);

    if(_parse_state == UNPARSEABLE) {
        record_hint_functions();
        return;
    }

    // For modification: once we've full-parsed once, don't do it again
    if (_parse_state >= COMPLETE) return;

    ScopeLock<Mutex<true> > L(parse_mutex);
    parse_vanilla();
    finalize();
    // anything else by default...?

    if(_parse_state < COMPLETE)
        _parse_state = COMPLETE;

    parsing_printf("[%s:%d] parsing complete for Parser %p with state %d\n", FILE__, __LINE__, this, _parse_state);
#ifdef ADD_PARSE_FRAME_TIMERS
    std::ofstream stat_log("functions.csv");
    stat_log << "Results for " << time_histogram.size() << " buckets\n";
    stat_log << "usecs,count\n";
    std::transform(time_histogram.begin(), time_histogram.end(),
            std::ostream_iterator<std::string >(stat_log, "\n"),
            pair_to_string<std::pair<unsigned int, unsigned int> >);
#endif
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
    LockFreeQueue<ParseFrame *> work;

    parsing_printf("[%s:%d] entered parse_at([%lx,%lx),%lx)\n",
            FILE__,__LINE__,region->low(),region->high(),target);

    if(!region->contains(target)) {
        parsing_printf("\tbad address, bailing\n");
        return;
    }

    // Reset parser status 
    _parse_state = PARTIAL;
    hint_funcs.clear();
    discover_funcs.clear();
    deleted_func.clear();    
    f = _parse_data->createAndRecordFunc(region, target, src);
    if (f == NULL)
        f = _parse_data->findFunc(region,target);
    if(!f) {
        parsing_printf("   could not create function at %lx\n",target);
        return;
    }

    ParseFrame::Status exist = _parse_data->frameStatus(region,target);
    if(exist != ParseFrame::BAD_LOOKUP) {
        parsing_printf("   frame at %lx already exists, status %d\n",
                target, exist);
        return;
    }
    pf = _parse_data->createAndRecordFrame(f);
    if (pf != NULL) {
        frames.insert(pf);
    } else {
        pf = _parse_data->findFrame(region, target);
    }
    if (pf->func->entry())
        work.insert(pf);
    parse_frames(work,recursive);
    finalize();

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
    LockFreeQueue<ParseFrame *> work;

    parsing_printf("[%s:%d] entered parse_vanilla()\n",FILE__,__LINE__);
    parsing_printf("\t%d function hints\n",hint_funcs.size());

    if(_parse_state < PARTIAL)
        _parse_state = PARTIAL;
    else
        parsing_printf("\tparse state is %d, some parsing already done\n",
                _parse_state);

    dyn_c_vector< std::pair<Address, ParseFrame*> > fvec;
    /* Initialize parse frames from hints */

    // Note: there is no fundamental obstacle to parallelizing this loop. However,
    // race conditions need to be resolved in supporting laysrs first.
#pragma omp parallel for schedule(dynamic)
    for (unsigned int i = 0; i < hint_funcs.size(); i++) {
        Function * hf = hint_funcs[i];
        ParseFrame::Status test = frame_status(hf->region(),hf->addr());
        if(test != ParseFrame::BAD_LOOKUP)
        {
            parsing_printf("\tskipping repeat parse of %lx [%s]\n",
                    hf->addr(),hf->name().c_str());
            continue;
        }

        ParseFrame *pf = _parse_data->createAndRecordFrame(hf);
        if (pf == NULL) {
            pf = _parse_data->findFrame(hf->region(), hf->addr());
        } else {
            frames.insert(pf);
        }
        fvec.push_back( make_pair(hf->addr(), pf) );        
    }

    vector<std::pair<Address, ParseFrame*> > svec;
    for (auto it = fvec.begin(); it != fvec.end(); ++it)
        svec.push_back(make_pair(it->first, it->second));
    sort(svec.begin(), svec.end());
    vector<std::pair<long, ParseFrame*> > size_vec;

    for (size_t i = 1; i < svec.size(); ++i)
        size_vec.push_back(make_pair(svec[i].first - svec[i-1].first, svec[i-1].second));
    if (svec.size() > 0)
        size_vec.push_back(make_pair(0, svec[svec.size()-1].second));
    sort(size_vec.begin(), size_vec.end());
    for (size_t i = 0; i < size_vec.size(); ++i)
        work.insert(size_vec[i].second);

    parse_frames(work,true);
}

    void
Parser::parse_edges( vector< ParseWorkElem * > & work_elems )
{
    if(_parse_state == UNPARSEABLE)
        return;

    // build up set of needed parse frames and load them with work elements
    set<ParseFrame*> frameset; // for dup checking
    LockFreeQueue<ParseFrame*> frames;

    for (unsigned idx=0; idx < work_elems.size(); idx++) {

        ParseWorkElem *elem = work_elems[idx];
        Block *src = elem->edge()->src();

        if (elem->order() == ParseWorkElem::call_fallthrough)
        {
            ParseAPI::Edge *callEdge = NULL;
            boost::lock_guard<Block> g(*src);
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
                    callTarget = callEdge->trg_addr();
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
                          callEdge->src()->last(),
                          callTarget,
                          isResolvable,
                          false ));
            }
        }

        vector<Function*> funcs;
        src->getFuncs(funcs);
        for (unsigned fix=1; fix < funcs.size(); fix++) {
            // if the block is shared, all of its funcs need
            // to add the new edge
            funcs[fix]->_cache_valid = false;
        }
        Function * f = NULL;
        if (funcs.size() >  0) {
            // Choosing any function is fine
            f = funcs[0];
        } else {
            f = _parse_data->createAndRecordFunc(src->region(), src->start(), RT);
        }

        ParseFrame *frame = _parse_data->createAndRecordFrame(f);
        if (frame == NULL) {
            frame = _parse_data->findFrame(src->region(), f->addr());
        } 
        if (elem->bundle()) {
            frame->work_bundles.push_back(elem->bundle());
        }
        // push before frame init so no seed is added
        frame->pushWork(elem);
        if (frameset.end() == frameset.find(frame)) {
            frameset.insert(frame);
            frames.insert(frame);
        }
    }
    ScopeLock<Mutex<true> > L(parse_mutex);
    // now parse
    if(_parse_state < PARTIAL)
        _parse_state = PARTIAL;

    parse_frames( frames, true );

    if(_parse_state > COMPLETE)
        _parse_state = COMPLETE;

    finalize();

}


LockFreeQueueItem<ParseFrame*> *
Parser::ProcessOneFrame(ParseFrame* pf, bool recursive) {
    LockFreeQueueItem<ParseFrame*> *frame_list = 0;
    assert(pf->func);
    {
        boost::lock_guard<ParseFrame> g(*pf);
#ifdef ADD_PARSE_FRAME_TIMERS
        boost::timer::cpu_timer t;
        t.start();
#endif
        parse_frame(*pf,recursive);
#ifdef ADD_PARSE_FRAME_TIMERS
        t.stop();
        unsigned int msecs = floor(t.elapsed().wall / 1000000.0);
        {
            dyn_c_hash_map<unsigned int, unsigned int>::accessor a;
            time_histogram.insert(a, msecs);
            ++(a->second);
        }
#endif
        frame_list = postProcessFrame(pf, recursive);
    }
    return frame_list;
}

LockFreeQueueItem<ParseFrame *> *Parser::postProcessFrame(ParseFrame *pf, bool recursive) {

    LockFreeQueue<ParseFrame*> work;
    switch(pf->status()) {
        case ParseFrame::CALL_BLOCKED: {
            parsing_printf("[%s] frame %lx blocked at %lx\n",
                           FILE__,pf->func->addr(),pf->curAddr);
            {
                work.insert(pf);
                assert(pf->call_target);
                parsing_printf("    call target %lx\n",pf->call_target->addr());
                ParseFrame * tf = _parse_data->createAndRecordFrame(pf->call_target);
                if (tf) {
                    // a new frame
                    frames.insert(tf);
                    assert(tf);
                    assert(tf->func->entry());
                    if(recursive) work.insert(tf);
                } 
            }
            break;
        }
        case ParseFrame::RETURN_SET: {
                                         parsing_printf("[%s] frame %lx's function's return status set to RETURN. Chance to resume functions", FILE__, pf->func->addr());
                                         work.insert(pf);
                                         resumeFrames(pf->func, work);
                                         break;
                                     }

        case ParseFrame::PARSED:{
                                    parsing_printf("[%s] frame %lx complete, return status: %d\n",
                                            FILE__,pf->func->addr(),pf->func->retstatus());

                                    if (unlikely(_obj.defensiveMode() &&
                                                TAMPER_NONE != pf->func->tampersStack() &&
                                                TAMPER_NONZERO != pf->func->tampersStack() ))
                                    {   // may adjust CALL_FT targets, add a frame to the worklist,
                                        // or trigger parsing in a separate target CodeObject
                                        tamper_post_processing(work,pf);
                                    }

                                    /* Since we are done with parsing this function,
                                     * we should be able to resume frames that are
                                     * waiting for this function. 
                                     *
                                     * This is important to reduce the idle time between imbalanced threads.
                                     * Instead of waiting for the master thread to resume every frames,
                                     * we immediately resume frames that are waitfing for the current function
                                     *
                                     * Besides resuming frames when a frame is finsihed, it is also important
                                     * to resume a frame itself when a frame is put into DELAYED status.
                                     * See the comments in ParseFrame::FRAME_DELAYED case for more details.
                                     */
                                    resumeFrames(pf->func, work);

                                    pf->cleanup();
                                    _parse_data->remove_frame(pf);
                                    break;
                                }
        case ParseFrame::FRAME_ERROR:
                                parsing_printf("[%s] frame %lx error at %lx\n",
                                        FILE__,pf->func->addr(),pf->curAddr);
                                break;
        case ParseFrame::FRAME_DELAYED:
                                {
                                    parsing_printf("[%s] frame %lx delayed at %lx\n",
                                            FILE__,
                                            pf->func->addr(),
                                            pf->curAddr);

                                    /* It is possible that a callee that caused this frame to be in delayed status
                                     * has already been parsed. Here we check if any callee has been parsed, and if
                                     * it is the case, we can immedately resume the current frame.
                                     *
                                     * Note that this resume paired with the resumeFrames in ParseFrame::PARSED is
                                     * necessary for correctness. 
                                     *
                                     * Suppose thread 1 is parsing function A, and thread 2 is parsing function B.
                                     * And A calls B. Thread 1 is first delayed by B and puts A into the delayed frames.
                                     * Then thread 2 finishes B and resumes A, and thread 1 is still working on other
                                     * parts of A. If thread 3 picks up the resumed work item to parse A, thread 3 will 
                                     * find that thread 1 is still working on A, and gives this work item up.
                                     * However, thread 1 still believes that itself is delayed by B, causing delayed edges
                                     * never parsed.
                                     *
                                     * Therefore, it is crucial to resume the current frame if we find some of the 
                                     * callees have been finished.
                                     */
                                    bool immediatelyResume = false;
                                    if (pf->delayedWork.size()) {
                                        // Add frame to global delayed list

                                        for (auto iter = pf->delayedWork.begin();
                                                iter != pf->delayedWork.end();
                                                ++iter) {

                                            Function * ct = iter->second;
                                            parsing_printf("[%s] waiting on %s\n",
                                                    __FILE__,
                                                    ct->name().c_str());
                                            {
                                                dyn_c_hash_map<Function*, std::set<ParseFrame*> >::accessor a;
                                                set<ParseFrame *> waiters;
                                                // This acquires the lock for ct, regardless we get a new set of not
                                                bool new_set = delayed_frames.insert(a, std::make_pair(ct, waiters));
                                                if (ct->retstatus() != UNSET) {
                                                    if (new_set) delayed_frames.erase(a);
                                                    immediatelyResume = true;
                                                    continue;
                                                }
                                                a->second.insert(pf);
                                                delayed_frames_changed.exchange(true, boost::memory_order_relaxed);
                                            }
                                        }
                                    } else {
                                        // We shouldn't get here
                                        assert(0 && "Delayed frame with no delayed work");
                                    }

                                    if (immediatelyResume) {
                                        work.insert(pf);
                                    }

                                    break;
                                }
        case ParseFrame::PROGRESS:
                                // another thread is working on this already; possibly something else unblocked.
                                break;

        default:
                                assert(0 && "invalid parse frame status");
    }
    return work.steal();
}


void
Parser::LaunchWork
(
 LockFreeQueueItem<ParseFrame*> *frame_list,
 bool recursive
 )
{
    LockFreeQueue<ParseFrame *> private_queue(frame_list);
    for(;;) {
        LockFreeQueueItem<ParseFrame *> *first = private_queue.pop();
        if (first == 0) break;
        ParseFrame *frame = first->value();
        delete first;
#pragma omp task firstprivate(frame, recursive)
        SpawnProcessFrame(frame, recursive);
    }
}


void
Parser::SpawnProcessFrame
(
 ParseFrame *pf,
 bool recursive
 )
{
    LockFreeQueueItem<ParseFrame*> *new_frames = ProcessOneFrame(pf, recursive);
    LaunchWork(new_frames, recursive);
}

void print_work_queue(LockFreeQueue<ParseFrame *> *work_queue)
{
    LockFreeQueueItem<ParseFrame *> *current = work_queue->peek();

    std::cout << "Work Queue" << std::endl;
    while (current) {
        std::cout << "  parse frame " << std::hex << current->value()
            << std::dec << std::endl;
        current = current->next();
    }
}


void
    Parser::ProcessFrames
(
 LockFreeQueue<ParseFrame *> *work_queue,
 bool recursive
 )
{
#pragma omp parallel shared(work_queue)
    {
#pragma omp master
        LaunchWork(work_queue->steal(), recursive);
    }
}


    void
Parser::parse_frames(LockFreeQueue<ParseFrame *> &work, bool recursive)
{
    delayed_frames_changed.store(false);
    ProcessFrames(&work, recursive);
    bool done = false, cycle = false;
    {
        // Check if we can resume any frames yet
        vector<Function *> updated;
        for (auto iter = delayed_frames.begin();
                iter != delayed_frames.end();
                ++iter) {
            if (iter->first->retstatus() != UNSET) {
                updated.push_back(iter->first);
            }
        }
        if (updated.size()) {
            for (auto uIter = updated.begin();
                    uIter != updated.end();
                    ++uIter) {
                resumeFrames((*uIter), work);
            }
        }

        if(delayed_frames.size() == 0 && updated.empty()) {
            parsing_printf("[%s] Fixed point reached (0 funcs with unknown return status)\n)",
                    __FILE__);
            done = true;
        } else if(delayed_frames.size() > 0 && !delayed_frames_changed.load()) {
            cycle = true;
        }
    }

    // Use fixed-point to ensure we parse frames whose parsing had to be delayed
    if (!done) {
        if(cycle) {
            processCycle(work,recursive);
        } else {
            processFixedPoint(work, recursive);
        }
    }

    cleanup_frames();
}

void Parser::processFixedPoint(LockFreeQueue<ParseFrame *> &work, bool recursive) {
    // We haven't yet reached a fixedpoint; let's recurse
    parsing_printf("[%s] Fixed point not yet reached (%d funcs with unknown return status)\n",
            __FILE__,
            delayed_frames.size());

    // Recurse through parse_frames
    parsing_printf("[%s] Calling parse_frames again... \n", __FILE__);

    parse_frames(work, recursive);
}

void Parser::processCycle(LockFreeQueue<ParseFrame *> &work, bool recursive) {// If we've reached a fixedpoint and have remaining frames, we must
    // have a cyclic dependency
    vector<Function *> updated;

    parsing_printf("[%s] Fixed point reached (%d funcs with unknown return status)\n",
            __FILE__,
            delayed_frames.size());

    // Mark UNSET functions in cycle as NORETURN
    // except if we're doing non-recursive parsing.
    // If we're just parsing one function, we want
    // to mark everything RETURN instead.
    for (auto iter = delayed_frames.begin(); iter != delayed_frames.end(); ++iter) {
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
        for (auto vIter = vec.begin(); vIter != vec.end(); ++vIter) {
            Function * delayed = (*vIter)->func;
            if (delayed->retstatus() == UNSET) {
                delayed->set_retstatus(NORETURN);
                delayed->obj()->cs()->incrementCounter(PARSE_NORETURN_HEURISTIC);
                updated.push_back(delayed);
            }
        }
    }


    // We should have updated the return status of one or more frames; recurse
    if (updated.size()) {
        for (auto uIter = updated.begin();
                uIter != updated.end();
                ++uIter) {
            resumeFrames((*uIter), work);
        }

        if (work.peek()) {
            parsing_printf("[%s] Updated retstatus of delayed frames, trying again...\n", __FILE__);
            parse_frames(work, recursive);
        }
    } else {
        // We shouldn't get here
        parsing_printf("[%s] No more work can be done (%d funcs with unknown return status)\n",
                __FILE__,
                delayed_frames.size());
        assert(0);
    }
}

void Parser::cleanup_frames()  {
    vector <ParseFrame *> pfv;
    std::copy(frames.begin(), frames.end(), std::back_inserter(pfv));
#pragma omp parallel for schedule(dynamic)
    for (unsigned int i = 0; i < pfv.size(); i++) {
        ParseFrame *pf = pfv[i];
        if (pf) {
            delete pf;
        }
    }
    frames.clear();
}

/* Finalizing all functions for consumption:

   - Finish delayed parsing
   - Prepare and record FuncExtents for range-based lookup
   */

// This function returns false if it flips tail call determination of an edge,
// and thus needs to re-finalize the function boundary.
//
// Returns true if it is done and finalized the function
    bool
Parser::finalize(Function *f)
{
    boost::lock_guard<Function> g(*f);
    if(f->_cache_valid) {
        return true;
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


    // Determine function boundary
    Function::blocklist blocks = f->blocks_int();

    // Check whether there are tail calls to blocks within the same function
    dyn_hash_map<Block*, bool> visited;
    for (auto bit = blocks.begin(); bit != blocks.end(); ++bit) {
        Block * b = *bit;
        visited[b] = true;
    }

    int block_cnt = 0;
    for (auto bit = blocks.begin(); bit != blocks.end(); ++bit) {
        Block * b = *bit;
        block_cnt++;
        for (auto eit = b->targets().begin(); eit != b->targets().end(); ++eit) {
            ParseAPI::Edge *e = *eit;
            // Tail call corretions are only relevant to non-sink COND_TAKEN or non-sink DIRECT edges
            if (e->sinkEdge()) continue;
            if (e->type() != COND_TAKEN && e->type() != DIRECT) continue;

            Block* trg_block = e->trg();
            bool trg_has_call_edge = false;
            {
                boost::unique_lock<Block> l(*trg_block);
                for (auto eit2 = trg_block->sources().begin(); eit2 != trg_block->sources().end(); ++eit2)
                    if ((*eit2)->type() == CALL) {
                        trg_has_call_edge = true;
                        break;
                    }
            }

            // Rule 1:
            // If an edge is currently not a tail call, but the edge target has a CALL incoming edge,
            // the current edge should be marked as a tail call
            if (trg_has_call_edge && !e->interproc()) {
                e->_type._interproc = true;
                parsing_printf("from %lx to %lx, marked as tail call (target has a CALL incoming edge), re-finalize\n", b->last(), e->trg()->start());
                return false;
            }

            // Now let's deal with fliping tail-call to not-tail-call
            // If the target has a CALL incoming edge, or the current edge
            // is currently not marked as tail call, then we can skip this edge
            if (trg_has_call_edge || !e->interproc()) continue;

            // Look up the tail call target.
            // If the tail call target is created from a symbol, then keep it as a tail call
            Function *trg_func = findFuncByEntry(trg_block->region(), trg_block->start());
            if (trg_func && trg_func->src() == HINT) continue;

            // If the edge source and the edge target are in different sections,
            // the edge is a tail call.
            // For example, if a jump from .text to .plt (different sections), then
            // no matter what, this edge is a tail call.
            if (trg_func->region() != b->region()) continue;

            // Rule 2:
            // Find a tail call targeting a block within the same function.
            // If the jump target is the function entry,
            // it is a recursive tail call.
            // Otherwise,  this edge is not a tail call
            if (visited.find(e->trg()) != visited.end() && e->trg() != f->entry()) {
                e->_type._interproc = false;
                parsing_printf("from %lx to %lx, marked as not tail call (part of the function), re-finalize\n", b->last(), e->trg()->start());
                return false;
            }     

            // Rule 3:
            // If an edge is currently a tail call, but the edge target has only the current edge as incoming edges,
            // we treat this as not tail call.        
            bool only_incoming = true;
            for (auto seit = trg_block->sources().begin(); seit != trg_block->sources().end(); ++seit)
                if (*seit != e) {
                    only_incoming = false;
                    break;
                }
            if (only_incoming) {
                e->_type._interproc = false;
                parsing_printf("from %lx to %lx, marked as not tail call (single entry), re-finalize\n", b->last(), e->trg()->start());
                return false;
            }
        }
    }


    // Check whether the function contains only one block,
    // and the block contains only an unresolve indirect jump.
    // If it is the case, change the edge to tail call if necessary.
    //
    // This is part of the tail call heuristics.
    // However, during parsing, the entry block may be created by
    // the function, or may be created by another function sharing code.
    // If the block is created by a larger function, the heuristic will
    // not mark the edge as tail call
    if (block_cnt == 1) {
        Block *b = f->entry();
        Block::Insns insns;
        b->getInsns(insns);
        if (insns.size() == 1 && insns.begin()->second.getCategory() == c_BranchInsn) {
            for (auto eit = b->targets().begin(); eit != b->targets().end(); ++eit) {
                ParseAPI::Edge *e = *eit;
                if (!e->interproc() && (e->type() == INDIRECT || e->type() == DIRECT)) {
                    e->_type._interproc = true;
                    parsing_printf("from %lx to %lx, marked as tail call (jump at entry), re-finalize\n", b->last(), e->trg()->start());
                    return false;
                }
            }
        }
    }

    // is this the first time we've parsed this function?
    if (unlikely( !f->_extents.empty() )) {
        _parse_data->remove_extents(f->_extents);
        f->_extents.clear();
    }


    if(blocks.empty()) {
        f->_cache_valid = cache_value; // see above
        return true;
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
            ext_s = b->start();
        }
        ext_e = b->end();
    }
    ext = new FuncExtent(f,ext_s,ext_e);

    parsing_printf("%lx extent [%lx,%lx)\n",f->addr(),ext_s,ext_e);
    f->_extents.push_back(ext);

    f->_cache_valid = cache_value; // see comment at function entry

    // Update funcsByBlockMap
    for (auto bit = blocks.begin(); bit != blocks.end(); ++bit) {
        Block *b = *bit;
        dyn_c_hash_map<Block*, std::set<Function*> >::accessor a;
        std::set<Function*> new_value;
        new_value.insert(f);
        if (!funcsByBlockMap.insert(a, make_pair(b, new_value))) {
            a->second.insert(f);
        }
    }

    if (unlikely( f->obj()->defensiveMode())) {
        // add fallthrough edges for calls assumed not to be returning
        // whose fallthrough blocks we parsed anyway (this happens if 
        // the FT block is also a branch target)
        Function::edgelist & edges = f->_call_edge_list;
        for (Function::edgelist::iterator eit = edges.begin();
                eit != edges.end();
                eit++)
        {
            boost::lock_guard<Block> g(*(*eit)->src());
            if (2 > (*eit)->src()->targets().size()) {
                Block *ft = _parse_data->findBlock((*eit)->src()->region(),
                        (*eit)->src()->end());
                if (ft && HASHDEF(f->_bmap,ft->start())) {
                    link_block((*eit)->src(),ft,CALL_FT,false);
                }
            }
        }
    }
    return true;
}

    void
Parser::finalize()
{
    if(_parse_state < FINALIZED) {
        finalize_jump_tables();
        std::vector<region_data*> rd;
        _parse_data->getAllRegionData(rd);
        int totalBlock = 0;
        for (auto rit = rd.begin(); rit != rd.end(); ++rit)
            totalBlock += (*rit)->getTotalNumOfBlocks();
        funcsByBlockMap.rehash(2 * totalBlock);
        finalize_funcs(hint_funcs);
        finalize_funcs(discover_funcs);
        clean_bogus_funcs(discover_funcs);

        for (auto it = hint_funcs.begin(); it != hint_funcs.end(); ++it)
            if (deleted_func.find(*it) == deleted_func.end()) {
                sorted_funcs.insert(*it);
                funcs_to_ranges.push_back(*it);
            }

        for (auto it = discover_funcs.begin(); it != discover_funcs.end(); ++it)
            if (deleted_func.find(*it) == deleted_func.end()) {
                sorted_funcs.insert(*it);
                funcs_to_ranges.push_back(*it);
            }

        _parse_state = FINALIZED;
    }
}

/* The goal of finalizing jump tables is to remove bogus control flow
 * edges caused by over-approximating jump table size. During jump table
 * analysis, we do not use any information about other jump tables.
 *
 * Here, we know all the jump table starts and we assume that "no jump 
 * tables share any entries". Therefore, if a jump table overruns into
 * another jump table, we trim the overrun entries.
 */

    void
Parser::finalize_jump_tables()
{
    set<Address> jumpTableStart;
    map<Block*, Function::JumpTableInstance*> jumpTables;

    // Step 1: get all jump tables
    for (auto fit = hint_funcs.begin(); fit != hint_funcs.end(); ++fit) {
        Function* f = *fit;
        for (auto jit = f->getJumpTables().begin(); jit != f->getJumpTables().end(); ++jit) {
            jumpTableStart.insert(jit->second.tableStart);
            jumpTables.emplace(jit->second.block, &(jit->second));
        }
    }

    for (auto fit = discover_funcs.begin(); fit != discover_funcs.end(); ++fit) {
        Function* f = *fit;
        for (auto jit = f->getJumpTables().begin(); jit != f->getJumpTables().end(); ++jit) {
            jumpTableStart.insert(jit->second.tableStart);
            jumpTables.emplace(jit->second.block, &(jit->second));
        }
    }

    for (auto ait = jumpTableStart.begin(); ait != jumpTableStart.end(); ++ait)
        parsing_printf("Jump table at %lx\n", *ait);

    vector<Function::JumpTableInstance*> jumpTableVector;
    for (auto jti = jumpTables.begin(); jti != jumpTables.end(); ++jti)
        jumpTableVector.push_back(jti->second);

    // Step 2: concurrently searching for overrun jump table entries
#pragma omp parallel for schedule(dynamic)
    for (size_t i = 0; i < jumpTableVector.size(); ++i) {
        Function::JumpTableInstance* jti = jumpTableVector[i];
        parsing_printf("Inspect jump table at %lx\n", jti->block->last()); 
        auto start_it = jumpTableStart.find(jti->tableStart);
        ++start_it;

        Block::edgelist targets;
        jti->block->copy_targets(targets);
        for (auto eit = targets.begin(); eit != targets.end(); ++eit) {
            if ((*eit)->sinkEdge()) {
                parsing_printf("Remove sink edge for jump table at %lx\n", jti->block->last());
                jti->block->removeTarget(*eit);
                (*eit)->trg()->removeSource(*eit);
                break;
            }
        }

        if (start_it == jumpTableStart.end()) continue;
        if (*start_it < jti->tableEnd) {
            std::set<Address> validTargets;
            // Non-overlapping entries are valid targets.
            // We record these valid targets and do not remove target edges
            // even if overlapping entries lead to valid targets.
            for (Address addr = jti->tableStart; addr < *start_it; addr += jti->indexStride) {
                validTargets.insert(jti->tableEntryMap[addr]);
            }

            // Build a target address to ParseAPI::Edge* map
            std::map<Address, Edge*> edgeMap;
            jti->block->copy_targets(targets);
            for (auto eit = targets.begin(); eit != targets.end(); ++eit) {
                if ((*eit)->type() != INDIRECT || (*eit)->sinkEdge()) continue;
                edgeMap.insert(make_pair((*eit)->trg_addr(), *eit));
            }

            // Enumerate every overlapping entries and attempt to delete bogus edges
            for (Address addr = *start_it; addr < jti->tableEnd; addr += jti->indexStride) {
                if (validTargets.find(jti->tableEntryMap[addr]) != validTargets.end()) continue;
                if (edgeMap.find(jti->tableEntryMap[addr]) == edgeMap.end()) continue;
                Edge * e = edgeMap[jti->tableEntryMap[addr]];
                delete_bogus_blocks(e);
            }

            // Adjust jump table end
            jti->tableEnd = *start_it;
        }
    }
}

/* Removed indirect jump edges may lead to other 
 * blocks and edges that should be removed
 */
    void
Parser::delete_bogus_blocks(Edge* e)
{
    Block* cur = e->trg();

    parsing_printf("Remove an edge from %p[%lx, %lx) to %p[%lx, %lx), type %d\n",
            e->src(), e->src()->start(), e->src()->end(),
            e->trg(), e->trg()->start(), e->trg()->end(),
            e->type());
    e->src()->removeTarget(e);
    // Bogus control flow may lead to invalid instructions,
    // which will cause a sink edge. So, the sink block may
    // have a large number of incoming edges. 
    if (e->sinkEdge()) return;
    cur->removeSource(e);

    // If the target block has other incoming edges,
    // then we do not remove the block at this point.
    // It is possible that all incoming edges are bogus
    // indirect edges, then the last removed edge will
    // trigger the deletion of the block.
    Block::edgelist sources;
    cur->copy_sources(sources);
    for (auto eit = sources.begin(); eit != sources.end(); ++eit)
        if ((*eit)->type() != INDIRECT && (*eit)->src() != e->src()) 
            return;

    // If an indirect edge points a function entry,
    // and the entry block does not have other incoming edges,
    // then this function must be from the symbol talbe.
    // No need to perform cascading deletion.
    Function* func = findFuncByEntry(cur->region(), cur->start());
    if (func != NULL) return;

    // The target block is created by the bogus indirect edge,
    // we need to continue deleting edges
    Block::edgelist targets;
    cur->copy_targets(targets);
    for (auto eit = targets.begin(); eit != targets.end(); ++eit) {
        delete_bogus_blocks(*eit);
    }
}

    void
Parser::finalize_funcs(dyn_c_vector<Function *> &funcs)
{
    int size = funcs.size();
#pragma omp parallel for schedule(dynamic)
    for(int i = 0; i < size; ++i) {
        Function *f = funcs[i];
        f->finalize();
    }
}

/* This function should be run only with a single thread.
 *
 * If range data is changed to use a concurrent data structure
 * that supports concurrent writes.
 *
 * Finalizing ranges should then be moved back to normal finalization
 */

    void
Parser::finalize_ranges()
{
    for (size_t i = 0; i < funcs_to_ranges.size(); ++i) {
        Function *f = funcs_to_ranges[i];
        region_data * rd = _parse_data->findRegion(f->region());
        for (auto eit = f->extents().begin(); eit != f->extents().end(); ++eit)
            rd->funcsByRange.insert(*eit);
        for (auto bit = f->blocks().begin(); bit != f->blocks().end(); ++bit)
            rd->insertBlockByRange(*bit);
    }
    funcs_to_ranges.clear();
}

    void
Parser::clean_bogus_funcs(dyn_c_vector<Function*> &funcs)
{
    for (auto fit = funcs.begin(); fit != funcs.end(); fit++) {
        Function *f = *fit;
        // HINT functions are from symbol table;
        // GAP functions are from gap parsing;
        // ONDEMAND functions are from user specified parsing location
        // MODIFICATION functions are generated by instrumentation
        // These functions should stay
        if (f->src() == HINT || f->src() == GAP || f->src() == ONDEMAND || f->src() == MODIFICATION) continue;  
        bool interprocEdge = false;
        // Here we do not need locking because we
        // are single-threaded during finalizing
        for (auto eit = f->entry()->sources().begin();
                eit != f->entry()->sources().end(); ++eit)
            if ((*eit)->interproc()) { interprocEdge = true; break; }
        if (!interprocEdge) {
            parsing_printf("Removing function %lx with name %s\n", f->addr(), f->name().c_str());
            // This is a discovered function that has no inter-procedural entry edge.
            // This function should be created because tail call heuristic makes a mistake
            // We have already fixed such bogos tail calls in the previous step of finalizing,
            // so now we should remove such bogus function
            remove_func(f);

            // Also need to decrement the block reference count
            for (auto blk = f->blocks().begin(); blk != f->blocks().end(); blk++) {
                (*blk)->_func_cnt.fetch_add(-1);
            }
        }
    }
}

    void
Parser::record_func(Function *f)
{
    if(!f) return;

    if(f->src() == HINT)
        hint_funcs.push_back(f);
    else
        discover_funcs.push_back(f);    
}

    void
Parser::init_frame(ParseFrame & frame)
{
    Block * b = NULL;
    Block * split = NULL;

    // Find or create a block
    b = block_at(frame, frame.func, frame.func->addr(),split);
    if(b) {
        frame.leadersToBlock[frame.func->addr()] = b;
        frame.func->_entry = b;
        frame.seed = new ParseWorkElem(NULL,NULL,0, frame.func->addr(),true,false);
        frame.pushWork(frame.seed);
    } else {
        parsing_printf("[%s] failed to initialize parsing frame\n",
                       FILE__);
        return;
    }
    if (split) {
        _pcb.splitBlock(split,b);
    }

    // FIXME these operations should move into the actual parsing
    Address ia_start = frame.func->addr();
    unsigned size =
        frame.codereg->offset() + frame.codereg->length() - ia_start;
    const unsigned char* bufferBegin =
        (const unsigned char *)(frame.func->region()->getPtrToInstruction(ia_start));
    InstructionDecoder dec(bufferBegin,size,frame.codereg->getArch());
    InstructionAdapter_t* ah = InstructionAdapter_t::makePlatformIA_IAPI(obj().cs()->getArch(),
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
    inline ParseAPI::Edge * bundle_call_edge(ParseWorkBundle * b)
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
    inline ParseWorkElem * bundle_call_elem(ParseWorkBundle * b)
    {
        if(!b) return NULL;

        vector<ParseWorkElem*> const& elems = b->elems();
        vector<ParseWorkElem*>::const_iterator it = elems.begin();
        for( ; it != elems.end(); ++it) {
            if((*it)->edge()->type() == CALL)
                return (*it);
        }
        return NULL;
    }

    /* 
     * Look up the next block for detection of straight-line
     * fallthrough edges into existing blocks.
     */
}

void
Parser::parse_frame(ParseFrame & frame, bool recursive) {
    frame.func->_cache_valid = false;
    if (frame.status() == ParseFrame::PARSED) {
        return;
    } else if (frame.status() == ParseFrame::UNPARSED) {
        parsing_printf("[%s] ==== starting to parse frame %lx ====\n",
                FILE__,frame.func->addr());
        // prevents recursion of parsing
        frame.func->_parsed = true;
        if (HASHDEF(plt_entries,frame.func->addr()))
            frame.func->_name = plt_entries[frame.func->addr()];
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
            if (iter->second->retstatus() != UNSET) {
                frame.pushWork(iter->first);
                updated.push_back(iter->first);
            }
        }
        for (uIter = updated.begin(); uIter != updated.end(); ++uIter) {
            frame.delayedWork.erase(*uIter);
        }
    }

    frame.set_status(ParseFrame::PROGRESS);
    // The resolving jump tables depend on the current shape of CFG.
    // We use a fix-point analysis, where we we-analyze jump tables
    // that may need re-analysis, which may find new out-going edges
    int count = 0;
    do {
        count++;
        parsing_printf("Iteration %d for function %s at %lx\n", count, frame.func->name().c_str(), frame.func->addr());
        bool ret = parse_frame_one_iteration(frame, recursive);
        if (ret) return;
    } while (inspect_value_driven_jump_tables(frame));

    /** parsing complete **/
    if (frame.func->retstatus() == UNSET && HASHDEF(plt_entries,frame.func->addr())) {
        // For PLT entries, they are either NORETURN or RETURN.
        // They should not be in any cyclic dependency
        if (obj().cs()->nonReturning(plt_entries[frame.func->addr()])) {
            frame.func->set_retstatus(NORETURN);
        } else {
            frame.func->set_retstatus(RETURN);
        }
    } else if (frame.func->retstatus() == UNSET) {
        frame.func->set_retstatus(NORETURN);
    }

    frame.set_status(ParseFrame::PARSED);

    if (unlikely(obj().defensiveMode())) {
        // calculate this after setting the function to PARSED, so that when
        // we finalize the function we'll actually save the results and won't
        // re-finalize it
        frame.func->tampersStack();
    }
    _pcb.newfunction_retstatus( frame.func );
}

bool
Parser::parse_frame_one_iteration(ParseFrame &frame, bool recursive) {
    InstructionAdapter_t *ahPtr = NULL;
    ParseFrame::worklist_t & worklist = frame.worklist;
    map<Address, Block *> & leadersToBlock = frame.leadersToBlock;
    Address & curAddr = frame.curAddr;
    Function * func = frame.func;
    dyn_hash_map<Address, bool> & visited = frame.visited;
    unsigned & num_insns = frame.num_insns;

    /** Non-persistent intermediate state **/
    Address nextBlockAddr;

    while(!worklist.empty()) {

        Block * cur = NULL;
        ParseWorkElem * work = frame.popWork();
        if (work->order() == ParseWorkElem::call) {
            region_data::edge_data_map::accessor a;
            region_data::edge_data_map* edm = _parse_data->get_edge_data_map(func->region());
            assert(edm->find(a, work->source()));
            parsing_printf("Handling call work element\n");
            cur = a->second.b; 
            Function * ct = NULL;

            // If we're not doing recursive traversal, skip *all* of the call edge processing.
            // We don't want to create a stub. We'll handle the assumption of a fallthrough
            // target for the fallthrough work element, not the call work element. Catch blocks
            // will get excluded but that's okay; our heuristic is not reliable enough that I'm willing to deploy
            // it when we'd only be detecting a non-returning callee by name anyway.
            // --BW 12/2012
            if (!recursive) {
                parsing_printf("[%s] non-recursive parse skipping call %lx->%lx\n",
                        FILE__, cur->lastInsnAddr(), work->target());
                continue;
            }

            FuncSource how = RT;
            if(frame.func->_src == GAP || frame.func->_src == GAPRT)
                how = GAPRT;

            ct = _parse_data->createAndRecordFunc(frame.codereg, work->target(), how);
            if (ct) {
                _pcb.discover_function(ct);
            } else {
                ct = _parse_data->findFunc(frame.codereg, work->target());
            }
            bool frame_not_created = (frame_status(ct->region(),ct->addr())==ParseFrame::BAD_LOOKUP);
            parsing_printf("\tframe %lx, not created %d\n", ct->addr(), frame_not_created);

            if (!frame_not_created && !work->callproc()) {
                parsing_printf("[%s] binding call (call target should have been created) %lx->%lx\n",
                        FILE__,cur->lastInsnAddr(),work->target());
                bind_call(frame, work->target(), cur, work->edge());
                work->mark_call();
            }

            if (recursive && ct && frame_not_created) {
                // suspend this frame and parse the next
                parsing_printf("    [suspend frame %lx]\n", func->addr());
                frame.call_target = ct;
                frame.set_status(ParseFrame::CALL_BLOCKED);
                // need to re-visit this edge
                frame.pushWork(work);
                if (ahPtr) delete ahPtr;
                return true;
            } else if (ct && work->tailcall()) {
                // If func's return status is RETURN, 
                // then this tail callee does not impact the func's return status
                if (func->retstatus() != RETURN) {
                    update_function_ret_status(frame, ct, work);
                }
            }

            continue;
        } else if (work->order() == ParseWorkElem::call_fallthrough) {
            // check associated call edge's return status
            ParseWorkElem * call_elem = bundle_call_elem(work->bundle());
            if (!call_elem) {
                // odd; no call edge in this bundle
                parsing_printf("[%s] unexpected missing call edge at %lx\n",
                        FILE__,work->edge()->src()->lastInsnAddr());
            } else {
                // check for system call FT
                ParseAPI::Edge* edge = work->edge();
                Block::Insns blockInsns;
                //                boost::lock_guard<Block> src_guard(*edge->src());
                edge->src()->getInsns(blockInsns);
                auto prev = blockInsns.rbegin();
                InstructionAPI::Instruction prevInsn = prev->second;
                bool is_nonret = false;

                if (prevInsn.getOperation().getID() == e_syscall) {
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
                        ParseAPI::Edge * remove = work->edge();
                        remove->src()->removeTarget(remove);
                        factory().destroy_edge(remove, destroyed_noreturn);
                        continue;
                    }
                } else if (func->obj()->cs()->isCode(call_elem->target())) {
                    // For indirect calls, since we do not know the callee,
                    // the call fallthrough edges are assumed to exist
                    Address target = call_elem->target();
                    Function * ct = _parse_data->findFunc(frame.codereg,target);
                    assert(ct);
                    bool is_plt = false;

                    // check if associated call edge's return status is still unknown
                    if (ct && (ct->retstatus() == UNSET) ) {
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
                        ParseAPI::Edge * remove = work->edge();
                        //remove->src()->removeTarget(remove);
                        factory().destroy_edge(remove, destroyed_noreturn);
                        continue;
                    } else
                        // Invalidate cache_valid for all sharing functions
                        invalidateContainingFuncs(func, work->edge()->src());
                }
            }
        } else if (work->order() == ParseWorkElem::seed_addr) {
            cur = leadersToBlock[work->target()];
        } else if (work->order() == ParseWorkElem::resolve_jump_table) {
            // resume to resolve jump table
            auto work_ah = work->ah();
            parsing_printf("... continue parse indirect jump at %lx\n", work_ah->getAddr());
            ProcessCFInsn(frame,NULL,work->ah());
            // We only re-parse jump tables
            if (!work_ah->isTailCall(frame.func, INDIRECT, frame.num_insns, frame.knownTargets))
                frame.value_driven_jump_tables.insert(work_ah->getAddr());
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
        } else if (work->order() == ParseWorkElem::func_shared_code) {
            if (func->retstatus() != RETURN) {
                // The current function shares code with another function.
                // current function on this control flow path is the same
                // as the shared function.
                //
                update_function_ret_status(frame, work->shared_func(), work);
            }
            func->_cache_valid = false;
            continue;

        }

        if (NULL == cur) {
            pair<Block*,ParseAPI::Edge*> newedge =
                add_edge(frame,
                        frame.func,
                        work->edge()->src(),
                        work->source(),
                        work->target(),
                        work->edge()->type(),
                        work->edge());
            cur = newedge.first;
        }

        if (HASHDEF(visited,cur->start()))
        {
            parsing_printf("[%s] skipping locally parsed target at %lx, [%lx, %lx)\n",
                    FILE__,work->target(), cur->start(), cur->end());
            continue;
        }

        // Multiple functions can get accesses to a block,
        // but only the function that creates the block should
        // parse the block
        if (cur->createdByFunc() == frame.func && !cur->_parsed)
        {
            parsing_printf("[%s] parsing block %lx\n",
                    FILE__,cur->start());
            if (frame.func->obj()->defensiveMode()) {
                mal_printf("new block at %lx (0x%lx)\n",cur->start(), cur);
            }

            cur->_parsed = true;
            curAddr = cur->start();
            visited[cur->start()] = true;
            leadersToBlock[cur->start()] = cur;
        } else if (cur->createdByFunc() != frame.func) {
            parsing_printf("[%s] deferring parse of shared block %lx\n",
                    FILE__,cur->start());
            if (func->retstatus() < UNKNOWN) {
                // The current function shares code with the function
                // that created the block. So, the return status of the
                // current function on this control flow path is the same
                // as the shared function.
                //
                // This code is designed for the new POWER ABI,
                // where each function has two entries. But this code
                // also works if the functions that share the code
                // have the same return blocks.
                Function * other_func = cur->createdByFunc(); 
                update_function_ret_status(frame, other_func, frame.mkWork(NULL, other_func));
            }
            // The edge to this shared block is changed from
            // "going to sink" to going to this shared block.
            // This changes the function boundary, so we need to
            // invalidate the cache.
            func->_cache_valid = false;
            continue;
        } else {
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
            (const unsigned char *)(func->region()->getPtrToInstruction(curAddr));
        InstructionDecoder dec(bufferBegin,size,frame.codereg->getArch());

        if (!ahPtr)
            ahPtr = InstructionAdapter_t::makePlatformIA_IAPI(func->obj()->cs()->getArch(), dec, curAddr, func->obj(),
                    cur->region(), func->isrc(), cur);
        else
            ahPtr->reset(dec,curAddr,func->obj(),
                    cur->region(), func->isrc(), cur);

        InstructionAdapter_t * ah = ahPtr;

        nextBlockAddr = std::numeric_limits<Address>::max();
        auto nextBlockIter = frame.leadersToBlock.upper_bound(frame.curAddr);
        if (nextBlockIter != frame.leadersToBlock.end()) {
            nextBlockAddr = nextBlockIter->first;
        }
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

                end_block(cur,ahPtr);
                if (!set_edge_parsing_status(frame, cur->last(), cur)) break;
                ParseAPI::Edge* newedge = link_tempsink(cur, FALLTHROUGH);
                parsing_printf("[%s:%d] pushing %lx onto worklist\n",
                        FILE__,__LINE__,nextBlockAddr);
                frame.pushWork(
                        frame.mkWork(
                            NULL,
                            newedge,
                            ahPtr->getAddr(),
                            nextBlockAddr,
                            true,
                            false)
                        );
                break;
            } else if(ah->getInstruction().getCategory()==c_GPUKernelExitInsn) {
                // this is special treatment for non-returning instruction
                // examples are amdgpu_op_s_endpgm and amddgpu_op_s_endpgm_saved
                //cout << "calling endblock for non-returning instruction " << std::hex <<ah->getAddr() << endl; 
                end_block(cur,ahPtr);
                break;
            }
            // per-instruction callback notification
            ParseCallback::insn_details insn_det;
            insn_det.insn = ah;

            parsing_printf("[%s:%d] curAddr 0x%lx: %s \n",
                    FILE__,__LINE__,curAddr, insn_det.insn->getInstruction().format().c_str() );

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

                end_block(cur,ahPtr);
                if (!set_edge_parsing_status(frame,cur->last(), cur)) break; 
                ParseAPI::Edge* newedge = link_tempsink(cur, FALLTHROUGH);

                parsing_printf("[%s:%d] nop-block ended at %lx\n",
                        FILE__,__LINE__,curAddr);
                parsing_printf("[%s:%d] pushing %lx onto worklist\n",
                        FILE__,__LINE__,curAddr);
                frame.pushWork(
                        frame.mkWork(
                            NULL,
                            newedge,
                            ah->getAddr(),
                            curAddr,
                            true,
                            false)
                        );
                break;
            }

            /** Particular instruction handling (calls, branches, etc) **/
            ++num_insns;

            if(ah->hasCFT()) {
                if (ah->isIndirectJump()) {
                    // Create a work element to represent that
                    // we will resolve the jump table later
                    end_block(cur,ahPtr);
                    set_edge_parsing_status(frame,cur->last(), cur);
                    frame.pushWork( frame.mkWork( work->bundle(), cur, ahPtr));
                } else {
                    end_block(cur,ahPtr);
                    if (!set_edge_parsing_status(frame,cur->last(), cur)) break; 
                    if (ProcessCFInsn(frame,cur,ahPtr)) {
                        // This is the first time to set the current function to be RETURN.
                        // We may be able to resume some delayed functions
                        frame.set_status(ParseFrame::RETURN_SET); 
                        return true;
                    }
                }
                break;
            }else if (func->_saves_fp &&
                    func->_no_stack_frame &&
                    ah->isFrameSetupInsn()) { // isframeSetup is expensive
                func->_no_stack_frame = false;
            } else if (ah->isLeave()) {
                func->_no_stack_frame = false;
            } else if ( ah->isAbort() ) {
                // 4. `abort-causing' instructions
                end_block(cur,ahPtr);
                set_edge_parsing_status(frame, cur->last(), cur);
                //link(cur, sink_block, DIRECT, true);
                break;
            } else if ( ah->isInvalidInsn() ) {
                // 4. Invalid or `abort-causing' instructions
                end_block(cur,ahPtr);
                if (!set_edge_parsing_status(frame,cur->last(), cur)) break;
                link_addr(ahPtr->getAddr(), _sink, DIRECT, true, func);
                break;
            } else if ( ah->isInterruptOrSyscall() ) {
                // 5. Raising instructions
                end_block(cur,ahPtr);
                if (!set_edge_parsing_status(frame,cur->last(), cur)) break; 
                ParseAPI::Edge* newedge = link_tempsink(cur, FALLTHROUGH);
                parsing_printf("[%s:%d] pushing %lx onto worklist\n",
                        FILE__,__LINE__,curAddr);
                frame.pushWork(
                        frame.mkWork(
                            NULL,
                            newedge,
                            ahPtr->getAddr(),
                            ahPtr->getNextAddr(),
                            true,
                            false)
                        );
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
                    end_block(cur,ahPtr);
                    if (!set_edge_parsing_status(frame,cur->last(), cur)) break; 
                    // allow invalid instructions to end up as a sink node.
                    link_addr(ahPtr->getAddr(), _sink, DIRECT, true, func);
                    break;
                } else if (ah->isNopJump()) {
                    // patch the jump to make it a nop, and re-set the 
                    // instruction adapter so we parse the instruction
                    // as a no-op this time, allowing the subsequent
                    // instruction to be parsed correctly
                    mal_printf("Nop jump at %lx, changing it to nop\n",ah->getAddr());
                    _pcb.patch_nop_jump(ah->getAddr());
                    unsigned bufsize = func->region()->offset() + func->region()->length() - ah->getAddr();
                    const unsigned char* bufferBegin = (const unsigned char *)
                        (func->region()->getPtrToInstruction(ah->getAddr()));
                    dec = InstructionDecoder
                        (bufferBegin, bufsize, frame.codereg->getArch());
                    ah->reset(dec, curAddr, func->obj(),
                            func->region(), func->isrc(), cur);
                } else {
                    entryID id = ah->getInstruction().getOperation().getID();
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

                end_block(cur,ahPtr);
                if (!set_edge_parsing_status(frame, cur->last(), cur)) break;
                // We need to tag the block with a sink edge
                link_addr(ahPtr->getAddr(), _sink, DIRECT, true, func);

                break;
            } else if (!cur->region()->contains(ah->getNextAddr())) {
                parsing_printf("[%s] next address %lx is outside [%lx,%lx)\n",
                        FILE__,ah->getNextAddr(),
                        cur->region()->offset(),
                        cur->region()->offset()+cur->region()->length());
                end_block(cur,ahPtr);
                if (!set_edge_parsing_status(frame, cur->last(), cur)) break;
                // We need to tag the block with a sink edge
                link_addr(ahPtr->getAddr(), _sink, DIRECT, true, func);
                break;
            }
            ah->advance();
        }
    }
    if (ahPtr) delete ahPtr;
    // Check if parsing is complete
    if (!frame.delayedWork.empty()) {
        frame.set_status(ParseFrame::FRAME_DELAYED);
        return true;
    }
    return false;
}

    void
Parser::end_block(Block * b, InstructionAdapter_t * ah)
{
    b->updateEnd(ah->getNextAddr());
    b->_lastInsn = ah->getAddr();
}

    Block*
Parser::record_block(Block *b)
{
    parsing_printf("[%s:%d] recording block [%lx,%lx)\n",
            FILE__,__LINE__,b->start(),b->end());
    return _parse_data->record_block(b->region(),b);
}


// block_at should only be called when
// we know the we want to create a block within the function.
// So, we should not call this function to create the entry block
// of a callee function.
    Block *
Parser::block_at(ParseFrame &frame,
        Function * owner,
        Address addr,
        Block * & split)
{
    Block * ret = NULL;

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


    {
#ifdef ENABLE_RACE_DETECTION
        // this lock causes deadlock when running in parallel, but it is
        // useful for suppressing unimportant races on the iterator
        boost::lock_guard<Function> g(*owner);
#endif
        // An already existing block
        auto iter = frame.leadersToBlock.find(addr);
        if (iter != frame.leadersToBlock.end()) {
            return iter->second;
        }
        ret = _cfgfact._mkblock(owner, cr, addr);
        ret = record_block(ret);
        return ret;
    }
}

    pair<Block *,ParseAPI::Edge *>
Parser::add_edge(
        ParseFrame & frame,
        Function * owner,
        Block * src,
        Address src_addr,
        Address dst,
        EdgeTypeEnum et,
        ParseAPI::Edge * exist)
{
    Block * split = NULL;
    Block * ret = NULL;
    ParseAPI::Edge * newedge = NULL;
    pair<Block *, ParseAPI::Edge *> retpair((Block *) NULL, (ParseAPI::Edge *) NULL);

    if(!is_code(owner,dst)) {
        parsing_printf("[%s:%d] target address %lx rejected by isCode()\n",
                FILE__, __LINE__, dst);
        return retpair;
    }
    region_data::edge_data_map::accessor a;
    region_data::edge_data_map* edm = _parse_data->get_edge_data_map(owner->region());
    assert(edm->find(a, src_addr));
    src = a->second.b; 
    parsing_printf("\t add_edge called, src_addr %lx, dst %lx, edge type %d, looked up block [%lx, %lx)\n",
            src_addr, dst, et, src->start(), src->end());

    assert(src->last() == src_addr); 

    // The source block of the edge may have been split
    // since adding into the worklist. We use the edge
    // source address and follow fall-througth edge
    // to find the correct block object
    ret = block_at(frame, owner,dst, split);
    retpair.first = ret;

    if(split == src) {
        // special case -- same block
        src = ret;
    }

    if(et != CALL && frame.leadersToBlock.find(ret->start()) == frame.leadersToBlock.end()) {
        // If we created a new block, and this is not a function call,
        // then the new block is part of the current function.
        // We need to mark it
        frame.leadersToBlock[ret->start()] = ret;
    }

    if(NULL == exist) {
        newedge = link_block(src,ret,et,false);
        retpair.second = newedge;
    } else {
        assert(src->last() == src_addr);
        if (exist->type() == FALLTHROUGH || exist->type() == COND_NOT_TAKEN || exist->type() == CALL_FT) {
            if (src->end() != ret->start()) {
                fprintf(stderr, "src_addr %lx, src: [%lx, %lx), dst [%lx, %lx), target %lx, edge type %d\n", src_addr, src->start(), src->end(), ret->start(), src->end(), dst, et);
            }
        }

        relink(exist,src,ret);
        retpair.second = exist;
    }

    return retpair;
}

    pair<Function *,ParseAPI::Edge*>
Parser::bind_call(ParseFrame & frame, Address target, Block * cur, ParseAPI::Edge * exist)
{
    Function * tfunc = NULL;

    // look it up
    tfunc = _parse_data->findFunc(frame.codereg,target);
    if(!tfunc) {
        parsing_printf("[%s:%d] can't bind call to %lx\n",
                FILE__,__LINE__,target);
        return pair<Function*,ParseAPI::Edge*>((Function *) NULL,exist);
    }
    assert(tfunc->entry());

    relink(exist,cur,tfunc->entry());
    return pair<Function*,ParseAPI::Edge*>(tfunc,exist);
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
Parser::findFuncsByBlock(CodeRegion *r, Block *b, set<Function*> &funcs)
{
    if(_parse_state < COMPLETE) {
        parsing_printf("[%s:%d] Parser::findFuncsByBlock([%lx,%lx),%lx,...) "
                "forced parsing\n",
                FILE__,__LINE__,r->low(),r->high(),b->start());
        parse();
    }
    if(_parse_state < FINALIZED) {
        parsing_printf("[%s:%d] Parser::findFuncsByBlock([%lx,%lx),%lx,...) "
                "forced finalization\n",
                FILE__,__LINE__,r->low(),r->high(),b->start());
        finalize();
    }
    dyn_c_hash_map<Block*, std::set<Function*> >::const_accessor a;
    if (funcsByBlockMap.find(a, b)) {
        funcs = a->second;
        return funcs.size();
    }
    return 0;
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
    if (!funcs_to_ranges.empty()) finalize_ranges();
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
    if (!funcs_to_ranges.empty()) finalize_ranges();
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
    if (!funcs_to_ranges.empty()) finalize_ranges();
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

/* This function creates an edge by specifying
 * the source address. This function will look
 * up the source address in the block end map.
 * So, any function that calls this function should
 * not acquire an accessor to the source address
 */

    ParseAPI::Edge*
Parser::link_addr(Address src_addr, Block *dst, EdgeTypeEnum et, bool sink, Function * func)
{
    region_data::edge_data_map::accessor a;
    region_data::edge_data_map* edm = _parse_data->get_edge_data_map(func->region());
    assert(edm->find(a, src_addr));
    Block * src = a->second.b;
    return link_block(src, dst, et, sink);
}

/* This function creates an edge by specifying
 * the source Block. Any function that calls this 
 * function should first acquire an accessor to 
 * the source address and lookup the block object.
 * 
 * Otherwise, the source block may be split and cause
 * wrong edges.
 */
    ParseAPI::Edge*
Parser::link_block(Block* src, Block *dst, EdgeTypeEnum et, bool sink)
{
    if (et == FALLTHROUGH)
        assert(src->end() == dst->start());
    assert(et != NOEDGE);
    ParseAPI::Edge * e = factory()._mkedge(src,dst,et);
    e->_type._sink = sink;
    src->addTarget(e);
    dst->addSource(e);
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
    ParseAPI::Edge*
Parser::link_tempsink(Block *src, EdgeTypeEnum et)
{
    // Do not put the edge into block target list at this moment,
    // because the source block is likely to be split.
    Block* tmpsink = _sink;
    ParseAPI::Edge * e = factory()._mkedge(src, tmpsink,et);
    e->_type._sink = true;
    return e;
}

    void
Parser::relink(ParseAPI::Edge * e, Block *src, Block *dst)
{
    if (e->type() == FALLTHROUGH || e->type() == COND_NOT_TAKEN || e->type() == CALL_FT) {
        if (src->end() != dst->start()) {
            fprintf(stderr, "In relink : src [%lx, %lx) dst [%lx, %lx)\n", src->start(), src->end(), dst->start(), dst->end());
            assert(src->end() == dst->start());
        }
    }
    Block* oldDst = NULL;
    if(e->trg() && e->trg_addr() != std::numeric_limits<Address>::max()) {
        oldDst = e->trg();
    }
    bool addSrcAndDest = true;
    if(dst != e->trg()) {
        if(oldDst) // old edge was not sink
        {
            oldDst->removeSource(e);
            _pcb.removeEdge(e->trg(), e, ParseCallback::source);
            addSrcAndDest = false;
        }
        e->_target_off = dst->start();
        e->_target = dst;
        dst->addSource(e);
        _pcb.addEdge(dst, e, ParseCallback::source);
    }

    // Add the edge into the block's target list
    // Writes should be sequenced, so this should work.
    // Helgrind gets confused, so we use a cmp&swap to hide the write.
    Block * old = e->src();
    assert(e->_source.compare_exchange_strong(old, src));
    src->addTarget(e);
    _pcb.addEdge(src, e, ParseCallback::target);

    if (addSrcAndDest) {
        // We're re-linking a sinkEdge to be a non-sink edge; since
        // we don't inform PatchAPI of temporary sinkEdges, we have
        // to add both the source AND target edges
        _pcb.addEdge(src, e, ParseCallback::target);
    }
    e->_type._sink = (dst->start() == std::numeric_limits<Address>::max());
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
    deleted_func.insert(func);
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

    {
        dyn_c_hash_map<Address, Function*>::accessor a;
        if(reg_data->funcsByAddr.find(a, func->addr()))
        {
            reg_data->funcsByAddr.erase(a);
        }
        reg_data = _parse_data->findRegion(new_reg);
        reg_data->funcsByAddr.insert(a, make_pair(new_entry, func));
    }
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
void Parser::resumeFrames(Function * func, LockFreeQueue<ParseFrame *> & work)
{
    // If we do not know the function's return status, don't put its waiters back on the worklist
    if (func->retstatus() == UNSET) {
        parsing_printf("[%s] %s return status unknown, cannot resume waiters\n",
                __FILE__,
                func->name().c_str());
        return;
    }

    // When a function's return status is set, all waiting frames back into the worklist
    dyn_c_hash_map<Function*, std::set<ParseFrame*> >::accessor a;
    if (!delayed_frames.find(a, func)) {
        // There were no frames waiting, ignore
        parsing_printf("[%s] %s return status %d, no waiters\n",
                __FILE__,
                func->name().c_str(),
                func->retstatus());
        return;
    } else {
        parsing_printf("[%s] %s return status %d, undelaying waiting functions\n",
                __FILE__,
                func->name().c_str(),
                func->retstatus());
        // Add each waiting frame back to the worklist
        set<ParseFrame *> vec = a->second;
        for (set<ParseFrame *>::iterator fIter = vec.begin();
                fIter != vec.end();
                ++fIter) {
            work.insert(*fIter);
            Function *f = (*fIter)->func;
            parsing_printf("\t undelay function %s at %lx, frame delay work size %d\n", f->name().c_str(), f->addr(), (*fIter)->delayedWork.size()); 
        }
        // remove func from delayedFrames map
        delayed_frames.erase(a);
        delayed_frames_changed.exchange(true, boost::memory_order_relaxed);
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
    InstructionAPI::Instruction prevInsn = prevPair->second;
    if (prevInsn.getOperation().getID() != e_mov) {
        return false;
    }

    MachRegister syscallNumberReg = MachRegister::getSyscallNumberReg(arch);
    if (syscallNumberReg == InvalidReg) {
        return false;
    }
    InstructionAPI::RegisterAST* regAST = new InstructionAPI::RegisterAST(syscallNumberReg);
    InstructionAPI::RegisterAST::Ptr regASTPtr = InstructionAPI::RegisterAST::Ptr(regAST);

    std::vector<InstructionAPI::Operand> operands;
    prevInsn.getOperands(operands);
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

bool Parser::set_edge_parsing_status(ParseFrame& frame, Address addr, Block* b) {
    Function *f = frame.func;
    parsing_printf("Function %s tries to set parsing edge at %lx\n", frame.func->name().c_str(), addr);
    region_data::edge_data_map * edm = _parse_data->get_edge_data_map(b->region());
    region_data::edge_data_map::accessor a1;
    if (edm->insert(a1, addr)) {
        // This is the first time that a block ends at this address.
        // We record this block and function
        a1->second.f = f;
        a1->second.b = b;
        return true;
    } else {
        parsing_printf("[%s:%d] parsing edge at %lx has started by another thread, function %s at %lx\n",FILE__, __LINE__, addr, a1->second.f->name().c_str(), a1->second.f->addr()); 
        // the same function may have created edges before,
        // due to overlapping instructions
        if (a1->second.f != f) {
            // If another function has been here, the current function
            // must share code with that function. And the return status
            // of the current function depends on the shared function
            frame.pushWork(frame.mkWork(NULL, a1->second.f));
        }    

        // The current block and the recorded block overlap.
        // We attempt to split the blocks.
        Block *A, *B;
        Function *fA, *fB;
        if (b->start() < a1->second.b->start()) {
            A = b;
            B = a1->second.b;
            fA = f;
            fB = a1->second.f;
        } else {
            A = a1->second.b;
            B = b;
            fA = a1->second.f;
            fB = f;
        }
        if (A == B) return false;
        assert(A->end() == B->end());
        Address prev_insn;
        bool inconsistent = false;
        region_data::edge_data_map::accessor a2;
        if (A->consistent(B->start(), prev_insn)) {
            // The edge should stay with the shorter block
            A->moveTargetEdges(B);
            a1->second.f = fB;
            a1->second.b = B;
            A->updateEnd(B->start());
            A->_lastInsn = prev_insn;
            bool cont = true;
            // Iteratively split the block
            while (!edm->insert(a2, A->last())) {
                cont = false;
                B = a2->second.b;
                if (A->start() < B->start()) {
                    if (A->consistent(B->start(), prev_insn)) {
                        A->updateEnd(B->start());
                        A->_lastInsn = prev_insn;
                        cont = true;
                    }
                } else {
                    if (B->consistent(A->start(), prev_insn)) {
                        Block * tmp = A;
                        Function * tmpF = fA;
                        A = B;
                        B = tmp;
                        A->moveTargetEdges(B);
                        fA = a2->second.f;
                        a2->second.b = tmp;
                        a2->second.f = tmpF;
                        A->updateEnd(B->start());
                        A->_lastInsn = prev_insn;
                        cont = true;
                    }
                }
                if (!cont) {
                    inconsistent = true;
                    break;
                }
            }
            if (cont) {
                assert(A->end() == B->start());
                link_block(A,B,FALLTHROUGH,false);
                a2->second.f = fA;
                a2->second.b = A;
            }
        } else {
            inconsistent = true;
        }
        if (inconsistent) {
            Block::Insns A_insns, B_insns;
            A->getInsns(A_insns);
            B->getInsns(B_insns);
            for (auto iit = B_insns.begin(); iit != B_insns.end(); ++iit) {
                auto ait = A_insns.find(iit->first);
                if (ait != A_insns.end()) {
                    Address addr = iit->first;
                    --ait;
                    --iit;

                    Block * ret = factory()._mkblock(fA, b->region(),addr);
                    Block * exist = record_block(ret);
                    if (exist != ret) {
                        ret = exist;
                    }
                    ret->updateEnd(B->end());
                    ret->_lastInsn = B->_lastInsn;
                    ret->_parsed = true;

                    A->moveTargetEdges(ret);
                    B->moveTargetEdges(ret);
                    A->updateEnd(addr);
                    A->_lastInsn = ait->first;
                    B->updateEnd(addr);                    
                    B->_lastInsn = iit->first;

                    if (a2.empty()) {
                        a1->second.f = fA;
                        a1->second.b = ret;
                    } else {
                        a2->second.f = fA;
                        a2->second.b = ret;
                    }

                    link_block(A,ret,FALLTHROUGH,false);
                    link_block(B,ret,FALLTHROUGH,false);

                    region_data::edge_data_map::accessor a3;
                    if (!edm->insert(a3, A->last()))  {
                        assert(!"edm->insert(a3, A->last())");
                    }
                    a3->second.f = fA;
                    a3->second.b = A;
                    region_data::edge_data_map::accessor a4;
                    if (!edm->insert(a4, B->last()))  {
                        assert(!"edm->insert(a4, B->last())");
                    }
                    a4->second.f = fB;
                    a4->second.b = B;
                    break;
                }
            }
        }
        return false;
    }
}

bool Parser::inspect_value_driven_jump_tables(ParseFrame &frame) {
    bool ret = false;
    /* Right now, we just re-calculate jump table targets for 
     * every jump tables. An optimization is to improve the jump
     * table analysis to record which indirect jump is value
     * driven, and then only re-calculated value driven tables
     */
    for (auto bit = frame.value_driven_jump_tables.begin();
            bit != frame.value_driven_jump_tables.end();
            ++bit) {
        Address addr = *bit;
        region_data::edge_data_map::accessor a;
        region_data::edge_data_map* edm = _parse_data->get_edge_data_map(frame.func->region());
        assert(edm->find(a, addr));
        Block * block = a->second.b;
        std::vector<std::pair< Address, Dyninst::ParseAPI::EdgeTypeEnum > > outEdges;
        IndirectControlFlowAnalyzer icfa(frame.func, block);
        icfa.NewJumpTableAnalysis(outEdges);

        // Collect original targets
        set<Address> existing;
        for (auto eit = block->targets().begin(); eit != block->targets().end(); ++eit) {
            existing.insert((*eit)->trg_addr());
        }
        for (auto oit = outEdges.begin(); oit != outEdges.end(); ++oit) {
            if (existing.find(oit->first) != existing.end()) continue;
            // Find a new target and push it into work list
            ret = true;
            parsing_printf("Finding new target from block [%lx, %lx) to %lx\n", block->start(), block->end(), oit->first);
            ParseAPI::Edge* newedge = link_tempsink(block, oit->second);
            frame.knownTargets.insert(oit->first);

            frame.pushWork(frame.mkWork(
                        NULL,
                        newedge,
                        block->last(),
                        oit->first,
                        true,
                        false)
                    );

        }
    }
    return ret;
}


void
Parser::update_function_ret_status(ParseFrame &frame, Function * other_func, ParseWorkElem *work) {
    /* The return status starts with UNSET, and increases to RETURN, and maybe NORETURN
     * Once it is RETURN or NORETURN, it will not go back to UNSET.
     *
     * Therefore, it is crucial for the following if statements to be the right order:
     * First check the smaller values, and then check the larger values.
     *
     * Consider that if we reverse the order. So the code looks like
     * 1) if (other_func->retstatus() == RETURN) {
     *       ....
     * 2) }  else if (other_func->retstatus() == UNSET) {
     *       ....
     *    }
     * In such code structure, at line 1), the other_func can be in UNSET, so the check fails.
     * Concurrently, the other_func can be immediately set to RETURN, making the check at 
     * line 2) failing. So, the frame.func is neither delayed, nor updates its return status
     * to RETURN, which can lead to wrong NORETURN status. 
     */
    parsing_printf("Function %s at %lx share code with function %s at %lx\n", frame.func->name().c_str(), frame.func->addr(), other_func->name().c_str(), other_func->addr());
    if (other_func->retstatus() == UNSET) {
        parsing_printf("\t other_func is UNSET, create delayed work\n");
        frame.pushDelayedWork(work, other_func);
    } else if (other_func->retstatus() == RETURN) {
        parsing_printf("\t other_func is RETURN, set this function to RETURN\n");
        frame.func->set_retstatus(RETURN);
    } else {
        parsing_printf("\t other_func is NORETURN, this path does not impact the return status of this function\n");
    }

}

void
Parser::record_hint_functions() {
    for (auto f : hint_funcs) {
        sorted_funcs.insert(f);
    }
}
