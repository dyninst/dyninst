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
#include <set>
#include <vector>
#include <map>
#include <unordered_map>
#include <utility>
#include "dataflowAPI/h/Absloc.h"
#include "dataflowAPI/h/AbslocInterface.h"
#include "Instruction.h"

#include "dataflowAPI/h/stackanalysis.h"
#include "dataflowAPI/h/slicing.h"
#include "ABI.h"
#include "bitArray.h"
#include "registers/x86_regs.h"
#include "registers/x86_64_regs.h"

#include "common/h/Graph.h"
#include "instructionAPI/h/Instruction.h"
#include "instructionAPI/h/InstructionDecoder.h"

#include "debug_dataflow.h"

#include "parseAPI/h/CFG.h"
#include "parseAPI/h/CodeSource.h"
#include "parseAPI/h/CodeObject.h"

#include <ctime>

using namespace Dyninst;
using namespace InstructionAPI;
using namespace std;
using namespace ParseAPI;

bool containsCall(ParseAPI::Block *);
bool containsRet(ParseAPI::Block *);
ParseAPI::Function *getEntryFunc(ParseAPI::Block *);

/* An algorithm to generate a slice graph.
 
The slice graph is a directed graph that consists of nodes
corresponding to assignments from a set of inputs to an
output, where each input or output is an `abstract region'
(AbsRegion class) describing a register or a stack or heap
location. Edges in the slice graph indicate flow from the
output AbsRegion of the source node to an input AbsRegion of
the target node. Edges are typed with an AbsRegion
corresponding to the input region the flow describes; this
edge typing is necessary because two AbsRegions in different
assignments may be refer to equivalent locations without
being identical---consider, for example, the transformation
of stack locations across function calls in interprocedural
slices.

Implementation details:

The slicing algorithm searches either forward or backward
from an initial assignment in the CFG, with the search
termination controlled in part by a set of user-provided
predicates (indicating, e.g., whether to follow call edges).
At each step, the slicer maintains an `active' set of
AbsRegions for which we are searching for related
assignments (uses, in the forward case; definitions, in the
backward case); this set is updated as the search
progresses. The graph is linked up on the way "down" the
slice recursion.

To avoid redundantly revisiting "down-slice" instructions
due to forks in the CFG, AbsRegion assignments are cached as
the search completes recursion down a particular branch.
Because CFGs are loopy directeg graphs, however, this does
not lead to an optimimal search ordering; it is possible to
construct pathological cases in which down-slice edges are
visited multiple times due to novel AbsRegions arising on
different paths. The caching of down-slice AbsRegions only 
guarantees that a particular AbsRegion is only searched for
once along a given path---the loopy nature of the graph
prevents optimal search stragegies such as a topologically
sorted ordering that would be possible in a DAG. 

The algorithm goes more or less like this:

   A_0 <- initial assignment 
   F <- initialize frame from active AbsRegions in A_0
   // F contains `active' set of AbsRegions
   
   sliceInternalAux( F ) :
     
      // find assignments in the current instruction,
      // add them to the graph if appropriate, update
      // the active set:
      // active <- active \ killed U matches
      updateAndLink(F)
  
      // `successor' is direction-appropriate 
      foreach successor NF of F
         if visited(F->NF) // edge visited    
            
            // try to add assignments cached from down-slice
            updateAndLinkFromCache(NF)
            
            // remove AbsRegions that have been visited
            // along this edge from the active set
            removeBlocked(NF) 

            // active is empty unless this instruction
            // introduced new active regions (in
            // updateAndLinkFromCache)

         visited(F->NF) <- true
         // recurse
         sliceInternalAux( NF )
         // merge cached definitions, except those generated
         // in F
         cache[F] <- cache[F] U (cache[NF] \ defs[F]

   Clearly the `find successors' bit is quite complicated
   and involves user-defined predicates and various CFG
   traversal rules, updating of AbsRegions, etc. Refer to
   comments in the code for more details.
*/
Graph::Ptr
Slicer::sliceInternal(
    Direction dir,
    Predicates &p)
{
    Graph::Ptr ret;
    SliceNode::Ptr aP;
    SliceFrame initFrame;
    map<CacheEdge, set<AbsRegion> > visited;

    // this is the unified cache aka the cache that will hold 
    // the merged set of 'defs'.
    unordered_map<Address,DefCache> cache;

    // this is the single cache aka the cache that holds
    // only the 'defs' from a single instruction. 
    unordered_map<Address, DefCache> singleCache; 
    
    ret = Graph::createGraph();

    // set up a slicing frame describing with the
    // relevant context
    constructInitialFrame(dir,initFrame);

    // note that the AbsRegion in this Element *does not matter*;
    // we just need the block, function and assignment
    aP = createNode(Element(b_,f_,a_->out(),a_));

    if(dir == forward) {
        slicing_printf("Inserting entry node %p/%s\n",
            static_cast<void*>(aP.get()),aP->format().c_str());
    } else {
        slicing_printf("Inserting exit node %p/%s\n",
            static_cast<void*>(aP.get()),aP->format().c_str());
    }

    // add to graph
    insertInitialNode(ret, dir, aP);
    if (p.addNodeCallback(a_,visitedEdges) && p.modifyCurrentFrame(initFrame, ret, this)) {
        // initialize slice stack and set for loop detection.
        // the set may be redundant, but speeds up the loopless case.
        addrStack.push_back(initFrame.addr());
        addrSet.insert(initFrame.addr());
	
	slicing_printf("Starting recursive slicing\n");
	sliceInternalAux(ret,dir,p,initFrame,true,visited, singleCache, cache);
	slicing_printf("Finished recursive slicing\n");
    }


    // promote any remaining plausible nodes.
    promotePlausibleNodes(ret, dir); 

    cleanGraph(ret);
    return ret;
}

// main slicing routine. creates any new edges if they are part of the 
// slice, and recursively slices on the next isntruction(s).
void Slicer::sliceInternalAux(
    Graph::Ptr g,
    Direction dir,
    Predicates &p,
    SliceFrame &cand,
    bool skip,              // skip linking this frame; for bootstrapping
    map<CacheEdge,set<AbsRegion> > & visited,
    unordered_map<Address, DefCache>& singleCache, 
    unordered_map<Address,DefCache> & cache)
{
    vector<SliceFrame> nextCands;
    DefCache& mydefs = singleCache[cand.addr()];

    slicing_printf("\tslicing from %lx, currently watching %lu regions\n",
        cand.addr(),cand.active.size());

    // Find assignments at this point that affect the active
    // region set (if any) and link them into the graph; update
    // the active set. Returns `true' if any changes are made,
    // `false' otherwise.

    if (!skip) {
        if (!updateAndLink(g,dir,cand, mydefs, p)) return;
	    slicing_printf("\t\tfinished udpateAndLink, active.size: %lu\n",
                       cand.active.size());
        // If the analysis that uses the slicing can stop for 
	// analysis specifc reasons on a path, the cache
	// may or may not contain the complete dependence for a
	// visited edge. The analysis should decide whether to use
	// the cache or not.

        if (p.performCacheClear()) cache.clear();
    }

    if (cand.active.empty()) {
        promotePlausibleNodes(g, dir);
        return;
    }

    // Find the next search candidates along the control
    // flow (for appropriate direction)
    bool success = getNextCandidates(dir,p,cand,nextCands);
    if (!success) {
        widenAll(g,dir,cand);
    }

    slicing_printf("\t\tgetNextCandidates returned %lu, success: %d\n",
                   nextCands.size(),success);

    for (unsigned i=0; i < nextCands.size(); ++i) {
        SliceFrame & f = nextCands[i];
        if (!f.valid) {
            widenAll(g,dir,cand);
            continue;
        }

        CacheEdge e(cand.addr(),f.addr());

        slicing_printf("\t\t candidate %u is at %lx, %lu active\n",
                       i,f.addr(),f.active.size());

        if (visited.find(e) != visited.end()) {
            // attempt to resolve the current active set
            // via cached values from down-slice, eliminating
            // those elements of the active set that can be
            // so resolved

            // check if in loop, if so, merge single caches into unified.
            if (addrSet.find(f.addr()) != addrSet.end()) {
                mergeRecursiveCaches(singleCache, cache, f.addr());
            }

            updateAndLinkFromCache(g,dir,f,cache[f.addr()]);
            removeBlocked(f,visited[e]);

            // the only way this is not true is if the current
            // search path has introduced new AbsRegions of interest
            if (f.active.empty()) {
                continue;
            }
        }

        markVisited(visited,e,f.active);

        // If the control flow search has run
        // off the rails somehow, widen;
        // otherwise search down this new path
        if(!f.valid || (p.slicingSizeLimitFactor() > 0 && visited.size() > p.slicingSizeLimitFactor() * g->size())) {
            widenAll(g,dir,cand);
	    }
        else {

            // update stacks
            addrStack.push_back(f.addr());
            addrSet.insert(f.addr());

            sliceInternalAux(g,dir,p,f,false,visited, singleCache, cache);

            // clean up stacks
            addrStack.pop_back();
            addrSet.erase(f.addr());

            // absorb the down-slice cache into this node's cache
	        cache[cand.addr()].merge(cache[f.addr()]);
        }
    }
    
    // promote plausible entry/exit nodes if this is end of recursion.
    if (nextCands.size() == 0) {
        promotePlausibleNodes(g, dir);
    }

    // Replace any definitions from down-slice with
    // those created by this instruction
    //
    // XXX if this instruction has definitions that
    //     were not of interest when it was visited
    //     (empty mydefs for that absregion), then
    //     do not cache down-slice information; if
    //     a different path leads back to this node,
    //     we need to create the real definitions
    cache[cand.addr()].replace(mydefs);
}

void
Slicer::removeBlocked(
    SliceFrame & f,
    set<AbsRegion> const& block)
{
    SliceFrame::ActiveMap::iterator ait = f.active.begin();
    for( ; ait != f.active.end(); ) {
        if(block.find((*ait).first) != block.end()) {
            SliceFrame::ActiveMap::iterator del = ait;
            ++ait;
            f.active.erase(del);
        } else {
            ++ait;
        }
    }
}

void
Slicer::markVisited(
    map<CacheEdge, set<AbsRegion> > & visited,
    CacheEdge const& e,
    SliceFrame::ActiveMap const& active)
{
    set<AbsRegion> & v = visited[e];
    SliceFrame::ActiveMap::const_iterator ait = active.begin();
    for( ; ait != active.end(); ++ait) {
        v.insert((*ait).first);
    }
}

// converts the current instruction into assignments and looks for matching 
// elements in the active map. if any are found, graph nodes and edges are
// created. this function also updates the active map to be contain only the
// elements that are valid after the above linking (killed defs are removed).
bool Slicer::updateAndLink(
    Graph::Ptr g,
    Direction dir,
    SliceFrame & cand,
    DefCache& cache,
    Predicates &p)
{

    vector<Assignment::Ptr> assns;
    vector<bool> killed;
    vector<Element> matches;
    vector<Element> newactive;

    bool change = false;

    killed.resize(cand.active.size(),false);

    if(dir == forward)
        convertInstruction(cand.loc.current->first,cand.addr(),cand.loc.func, cand.loc.block, assns);
    else
        convertInstruction(cand.loc.rcurrent->first,cand.addr(),cand.loc.func, cand.loc.block, assns);

    // iterate over assignments and link matching elements.
    for(unsigned i=0; i<assns.size(); ++i) {
        SliceFrame::ActiveMap::iterator ait = cand.active.begin();
        unsigned j=0;
        for( ; ait != cand.active.end(); ++ait,++j) {
            if (findMatch(g,dir,cand,(*ait).first,assns[i],matches, cache)) { // links	  
	        if (!p.addNodeCallback(assns[i], visitedEdges)) return false;
	    }
	    killed[j] = killed[j] || kills((*ait).first,assns[i]);
            change = change || killed[j];
        }
        // Record the *potential* of this instruction to interact
        // with all possible abstract regions
        cachePotential(dir,assns[i],cache);
    }

    if(!change && matches.empty()) {// no change -- nothing killed, nothing added
        return true;
    }

    // update of active set -- matches + anything not killed
    SliceFrame::ActiveMap::iterator ait = cand.active.begin();
    unsigned j=0;
    for( ; ait != cand.active.end(); ) {
        if(killed[j]) {
            // remove killed nodes from plausible exit set.
            // this need only be done in the forward case,
            // because backward slice semantics properly
            // handle the plausible entry set.
            if (dir == forward) {
                for (auto vf = ait->second.begin(), vl = ait->second.end();
                        vf != vl; ++vf) {
                    plausibleNodes.erase(createNode(*vf));
                }
                
            }
            SliceFrame::ActiveMap::iterator del = ait;
            ++ait;
            cand.active.erase(del);
        } else {
            ++ait;
        }
        ++j;
    }

    for(unsigned i=0;i<matches.size();++i) {
       // Check our predicates
       if (p.widenAtPoint(matches[i].ptr)) {
          widen(g, dir, matches[i]);
       }
       else if (p.endAtPoint(matches[i].ptr)) {
          // Do nothing...
       }
       else {
          cand.active[matches[i].reg].push_back(matches[i]);
       }
    }
    return p.modifyCurrentFrame(cand, g, this);
}

// similar to updateAndLink, but this version only looks at the
// unified cache. it then inserts edges for matching elements.
void Slicer::updateAndLinkFromCache(
    Graph::Ptr g,
    Direction dir,
    SliceFrame & f, 
    DefCache & cache)
{
    SliceFrame::ActiveMap::iterator ait = f.active.begin();

    // if the abstract region of interest is in the defcache,
    // update it and link it

    for( ; ait != f.active.end(); ) {
        AbsRegion const& r = (*ait).first;
        if(!cache.defines(r)) {
            ++ait;
            continue;
        }

        // Link them up 
        vector<Element> const& eles = (*ait).second;
        set<Def> const& defs = cache.get(r);
        set<Def>::const_iterator dit = defs.begin();
        for( ; dit != defs.end(); ++dit) {
            for(unsigned i=0;i<eles.size();++i) {
                // don't create self-loops on assignments
                if (eles[i].ptr != (*dit).ele.ptr)
                    insertPair(g,dir,eles[i],(*dit).ele,(*dit).data);
            }
        }

        // Stop caring about this region
        SliceFrame::ActiveMap::iterator del = ait;
        ++ait;
        f.active.erase(del);
    }
}

void
Slicer::cachePotential(
    Direction dir,
    Assignment::Ptr assn,
    DefCache & cache)
{
    if(dir == forward) {
        vector<AbsRegion> const& inputs = assn->inputs();
        for(unsigned i=0;i<inputs.size();++i) {
            (void)cache.get(inputs[i]);
        }
    } else {
        (void)cache.get(assn->out());
    }
}

/*
 * Compare the assignment `assn' to the abstract region `cur'
 * and see whether they match, for the direction-appropriate
 * definition of "match". If so, generate new slice elements
 * and return them in the `match' vector, after linking them
 * to the elements associated with the region `cur'.
 * Return true if these exists at least a match.
 */
bool
Slicer::findMatch(
    Graph::Ptr g,
    Direction dir,
    SliceFrame const& cand,
    AbsRegion const& reg,
    Assignment::Ptr assn,
    vector<Element> & matches,
    DefCache& cache)
{
    bool hadmatch = false;
    if(dir == forward) {
		slicing_cerr << "\t\tComparing candidate assignment " << assn->format() << " to input region " << reg.format() << endl;
        vector<AbsRegion> const& inputs = assn->inputs();
        for(unsigned i=0;i<inputs.size();++i) {
            if(reg.contains(inputs[i])) {
                hadmatch = true;    
				slicing_cerr << "\t\t\t Match!" << endl;
                // Link the assignments associated with this
                // abstract region (may be > 1)
                Element ne(cand.loc.block,cand.loc.func,reg,assn);

                // Cache
                cache.get(reg).insert( Def(ne,inputs[i]) );
                
                vector<Element> const& eles = cand.active.find(reg)->second;
                for(unsigned j=0;j<eles.size();++j) {
                    insertPair(g,dir,eles[j],ne,inputs[i]);

                }
            }
        }
        if(hadmatch) {
            // In this case, we are now interested in
            // the outputs of the assignment
            matches.push_back(
                Element(cand.loc.block,cand.loc.func,assn->out(),assn));
        }
    } else {
        slicing_printf("\t\t\t\t\tComparing current %s to candidate %s\n",
            reg.format().c_str(),assn->out().format().c_str());
        if(reg.contains(assn->out()) || assn->out().contains(reg)) {
	    hadmatch = true;
            slicing_printf("\t\t\t\t\t\tMatch!\n");

            // Link the assignments associated with this
            // abstract region (may be > 1)
            Element ne(cand.loc.block,cand.loc.func,reg,assn); 

            // Cache
            cache.get(reg).insert( Def(ne,reg) );
            slicing_printf("\t\t\t cached [%s] -> <%s,%s>\n",
               reg.format().c_str(),
                ne.ptr->format().c_str(),reg.format().c_str());

            vector<Element> const& eles = cand.active.find(reg)->second;
            for(unsigned i=0;i<eles.size();++i) {
                // N.B. using the AbsRegion from the Element, which
                //      may differ from the `reg' parameter to this
                //      method because of transformation though
                //      call or return edges. This `true' AbsRegion
                //      is used to associate two different AbsRegions
                //      during symbolic evaluation
                // if (eles[i].ptr != ne.ptr)
                if (eles[i].ptr->addr() != ne.ptr->addr())
                    insertPair(g,dir,eles[i],ne,eles[i].reg);
            }

            // In this case, we are now interested in the 
            // _inputs_ to the assignment
            vector<AbsRegion> const& inputs = assn->inputs();
            for(unsigned i=0; i< inputs.size(); ++i) {
                ne.reg = inputs[i];
                matches.push_back(ne);
            }
            if (cand.loc.block->obj()->cs()->getArch() == Arch_cuda) {
                if (reg.contains(assn->out()) && !assn->out().contains(reg)) {
                    ne.reg = assn->out();
                    ne.reg.flipPredicateCondition();
                    slicing_printf("\t\t\t Handle predicate: search for %s, find %s, generate %s\n", reg.format().c_str(), assn->out().format().c_str(), ne.reg.format().c_str());
                    matches.push_back(ne);
                }
            }
        }
    }

    return hadmatch;
}


bool 
Slicer::getNextCandidates(
    Direction dir,
    Predicates & p,
    SliceFrame const& cand,
    vector<SliceFrame> & newCands)
{
    if(dir == forward) {
        return getSuccessors(p,cand,newCands);
    }
    else {
        return getPredecessors(p,cand,newCands);
    }
}

/*
 * Given the location (instruction) in `cand', find zero or more
 * control flow successors from this location and create new slicing
 * frames for them. Certain types of control flow require mutation of
 * the SliceFrame (modification of context, e.g.) AND mutate the 
 * abstract regions in the frame's `active' list (e.g. modifying
 * stack locations).
 */
bool
Slicer::getSuccessors(
    Predicates &p,
    SliceFrame const& cand,
    vector<SliceFrame> & newCands)
{
    InsnVec::iterator next = cand.loc.current;
    ++next;

    // Case 1: just one intra-block successor
    if(next != cand.loc.end) {
        SliceFrame nf = cand;
        nf.loc.current = next;
        assert(nf.loc.block);
        newCands.push_back(nf);

        slicing_printf("\t\t\t\t added intra-block successor\n");
        return true;
    }

    // Case 2: end of the block. Subcases are calls, returns, and other
    bool err = false;

    if(containsCall(cand.loc.block)) {
        slicing_printf("\t\t Handling call... ");
        SliceFrame nf = cand;

        // nf may be transformed
        if(handleCall(p,nf,err)) {
            slicing_printf("success, err: %d\n",err);
            assert(nf.loc.block);
            newCands.push_back(nf);
        } else {
            slicing_printf("failed, err: %d\n",err);
        }
    }
    else if(containsRet(cand.loc.block)) {
        slicing_printf("\t\t Handling return... ");
        SliceFrame nf = cand;
    
        // nf may be transformed
        if(handleReturn(p,nf,err)) {
            slicing_printf("success, err: %d\n",err);
            assert(nf.loc.block);
            newCands.push_back(nf);
        } else {
            slicing_printf("failed, err: %d\n",err);
        }
    }
    else {
        // Default intraprocedural control flow case; this
        // case may produce multiple new frames, but it
        // will not transform any of them (besides changing
        // the location)

        const Block::edgelist & targets = cand.loc.block->targets();
        Block::edgelist::const_iterator eit = targets.begin();
        for( ; eit != targets.end(); ++eit) {
            if((*eit)->sinkEdge()) {
                // will force widening
                newCands.push_back(SliceFrame(false));
            } 
            else {
                SliceFrame nf = cand;
                slicing_printf("\t\t Handling default edge type %d... ",
                    (*eit)->type());
                if(handleDefault(forward,p,*eit,nf,err)) {
                    slicing_printf("success, err: %d\n",err);
                    assert(nf.loc.block);
                    newCands.push_back(nf);
                } else {
                    slicing_printf("failed, err: %d\n",err);
                }
            }
        }
    }
    return !err;
}

void Slicer::handlePredecessorEdge(ParseAPI::Edge* e,
				   Predicates& p,
				   SliceFrame const& cand,
				   vector<SliceFrame> & newCands,
				   bool& err,
				   SliceFrame& nf)
{
  visitedEdges.insert(e);
  if (p.ignoreEdge(e)) {
      slicing_printf("ignore edge from %lx to %lx, type %d according to predicate\n", e->src()->last(), e->trg()->start(), e->type()); 
      return ;
  }
  switch(e->type()) 
  {
  case CALL:
    slicing_printf("\t\t Handling call... ");
    if(handleCallBackward(p,cand,newCands,e,err)) {
      slicing_printf("succeess, err: %d\n",err);
    } else {
      slicing_printf("failed, err: %d\n",err);
    }
    break;
  case RET:
    slicing_printf("\t\t Handling return... ");
    nf = cand;
    if(handleReturnBackward(p,cand,nf,e,err)) {
      slicing_printf("succeess, err: %d\n",err);
    } else {
      slicing_printf("failed, err: %d\n",err);
    }
    break;
  case CATCH:
    slicing_printf("\t\t Ignore catch edges ... ");
    break;
  default:
    if (e->interproc()) {
      slicing_printf("\t\t Handling tail call... ");
      if(handleCallBackward(p,cand,newCands,e,err)) {
        slicing_printf("succeess, err: %d\n",err);
      } else {
        slicing_printf("failed, err: %d\n",err);
      }
      return;
    }

    newCands.emplace_back(cand);
    slicing_printf("\t\t Handling default edge type %d... ",
		   e->type());
    if(handleDefault(backward,p,e,newCands.back(),err)) {
      slicing_printf("success, err: %d\n",err);
    } else {
      newCands.pop_back();
      slicing_printf("failed, err: %d\n",err);
    }
  }
}


  

/*
 * Same as successors, only backwards
 */
bool
Slicer::getPredecessors(
    Predicates &p,
    SliceFrame const& cand,
    vector<SliceFrame> & newCands)
{
    InsnVec::reverse_iterator prev = cand.loc.rcurrent;
    ++prev;

    // Case 1: intra-block
    if(prev != cand.loc.rend) {
      SliceFrame *nf = NULL;

      //Slightly more complicated than the forward case; check
      //a predicate for each active abstract region to see whether
      //we should continue
      bool cont = false;
      for (SliceFrame::ActiveMap::const_iterator ait = cand.active.begin(); ait != cand.active.end(); ++ait) {
        bool add = p.addPredecessor((*ait).first);
        if(add) {
          if (nf == NULL) {
            newCands.emplace_back(cand.loc, cand.con);
            nf = &newCands.back();
            nf->loc.rcurrent = prev;
          }
          nf->active.insert(*ait);
        }
        cont = cont || add;
      }

      if(cont) {
        slicing_printf("\t\t\t\t Adding intra-block predecessor %lx\n",
          nf->loc.addr());
        slicing_printf("\t\t\t\t Current regions are:\n");
        if(df_debug_slicing_on()) {
          for (SliceFrame::ActiveMap::const_iterator ait = cand.active.begin(); ait != cand.active.end(); ++ait) {
            slicing_printf("\t\t\t\t%s\n",
              (*ait).first.format().c_str());

            vector<Element> const& eles = (*ait).second;
            for(unsigned i=0;i<eles.size();++i) {
              slicing_printf("\t\t\t\t\t [%s] : %s\n",
                eles[i].reg.format().c_str(),eles[i].ptr->format().c_str());
            }
          }
        }
      }
      return true;
    }

    // Case 2: inter-block
    bool err = false;
    SliceFrame nf;
    
    Block::edgelist sources;
    cand.loc.block->copy_sources(sources);
    map< pair<Address, int> , ParseAPI::Edge* > sources_edges;
    for (auto eit = sources.begin(); eit != sources.end(); ++eit) {
        sources_edges.insert(make_pair( make_pair( (*eit)->src()->start(), (int)(*eit)->type() ), *eit));
    }
    for (auto eit = sources_edges.begin(); eit != sources_edges.end(); eit++) {
        handlePredecessorEdge(eit->second, p, cand, newCands, err, nf);
    }
    return !err; 
}

/*
 * Process a call instruction, determining whether to follow the
 * call edge (with the help of the predicates) or the fallthrough
 * edge (coloquially referred to as `funlink' thanks to our 
 * departed Arizona alum --- much respect M.L.)
 */
bool
Slicer::handleCall(
    Predicates & p,
    SliceFrame & cur,
    bool & err)
{
    ParseAPI::Block * callee = NULL;
    ParseAPI::Edge * funlink = NULL;
    bool widen = false;

    const Block::edgelist & targets = cur.loc.block->targets();
    Block::edgelist::const_iterator eit = targets.begin();
    for( ; eit != targets.end(); ++eit) {
        ParseAPI::Edge * e = *eit;
        if (e->sinkEdge()) widen = true;
        else if(e->type() == CALL) {
            if (callee && callee != e->trg()) {
                // Oops
                widen = true;
            }
            callee = e->trg();
        } else if(e->type() == CALL_FT) {
           funlink = e;
        }
    }
    
    if(followCall(p, callee, cur)) {
       if (widen) {
          // Indirect call that they wanted us to follow, so widen.
          err = true;
          return true;
       }

       ParseAPI::Block * caller_block = cur.loc.block;
       
       cur.loc.block = callee;
       cur.loc.func = getEntryFunc(callee);
       getInsns(cur.loc);
       
       // Update the context of the slicing frame
       // and modify the abstract regions in its
       // active vector
       if(!handleCallDetails(cur,caller_block)) {
          err = true;
          return false;
       }
    }
    else {
        // Use the funlink
        if(!funlink) {
            // FIXME I am preserving a comment and semantics that
            // I do not understand here, again as of 06697df. Quote:

            // ???
            return false;
        }
        if(!handleDefault(forward,p,funlink,cur,err)) {
            err = true;
            return false;
        }
    }
    return true;
}

/*
 * Builds up a call stack and callee function, and ask
 * the predicate whether we should follow the call (or,
 * implicitly, follow its fallthrough edge instead).
 */
bool
Slicer::followCall(
    Predicates & p,
    ParseAPI::Block * target,
    SliceFrame & cur)
{
    // FIXME quote:
   // A NULL callee indicates an indirect call.
   // TODO on that one...
   
    ParseAPI::Function * callee = (target ? getEntryFunc(target) : NULL);
    
    // cons up a call stack
    stack< pair<ParseAPI::Function *, int> > callStack;
    for(Context::reverse_iterator calls = cur.con.rbegin();
        calls != cur.con.rend(); ++calls)
    {
        if(NULL != calls->func) {
            callStack.push(make_pair(calls->func,calls->stackDepth));
        }
    } 
    // Quote:
        // FIXME: assuming that this is not a PLT function, since I have no
        // idea at present.  -- BW, April 2010

    // Give the predicate an opportunity to accept this call for each
    // of the abstract regions in the active set
    //
    // XXX There is an interesting concern here. What if the predicate
    // would indicate `follow' for one active AbsRegion and `do not
    // follow' for another? What you'd want to do in that case is
    // fork into multiple SliceFrames for absregions that should go
    // one way or the other. This is something that could be done by
    // moving the handleCallDetails() call into this method and
    // modifying the nextCands vector here, as opposed to in
    // handleCall(). 
    // 
    // This issue needs review by a person involved in slicer design.
    // FIXME

    bool ret = false;

    SliceFrame::ActiveMap::iterator ait = cur.active.begin();
    for( ; ait != cur.active.end(); ++ait) {
        ret = ret || p.followCall(callee, callStack, (*ait).first);
    }
    
    return ret;
}

void 
Slicer::shiftAllAbsRegions(
    SliceFrame & cur,
    long stack_depth,
    ParseAPI::Function *callee)
{
    SliceFrame::ActiveMap newMap;

    // fix all of the abstract regions
    SliceFrame::ActiveMap::iterator ait = cur.active.begin();
    for( ; ait != cur.active.end(); ++ait) {
        AbsRegion const& reg = (*ait).first;

        // shortcut -- do nothing if no adjustment is necessary
        if(reg.absloc() == Absloc()) {
            // N.B. doing this the hard way (rather than map.insert()
            //      in case a previous or later iteration transforms
            //      a different AbsRegion to the same as (*ait).first
            vector<Element> & e = newMap[(*ait).first];
            e.insert(e.end(),(*ait).second.begin(),(*ait).second.end());
            continue;
        }

        // Adjust the mapping region, but do not adjust the regions of the
        // elements --- these are maintained to their old values for
        // annotating the slicing graph edges to facilitate substitution
        // in symbolic expansion
        AbsRegion newReg;
        shiftAbsRegion(reg,newReg,stack_depth,callee);

        // just copy the elements
        vector<Element> & e = newMap[newReg];
        e.insert(e.end(),(*ait).second.begin(),(*ait).second.end());
    }
    // and replace
    cur.active = newMap;    
}

/*
 * Adjust the slice frame's context and translates the abstract
 * regions in the active list from caller to callee
 */
bool
Slicer::handleCallDetails(
    SliceFrame & cur,
    ParseAPI::Block * caller_block)
{ 
    ParseAPI::Function * caller = cur.con.front().func;
    ParseAPI::Function * callee = cur.loc.func;

    long stack_depth = 0;
    if(!getStackDepth(caller, caller_block, caller_block->end(), stack_depth))
        return false;

    // Increment the context
    pushContext(cur.con, callee, caller_block, stack_depth);

    // Transform the active abstract regions
    shiftAllAbsRegions(cur,stack_depth,callee);

    return true;
}

/*
 * Properly adjusts the location & context of the slice frame and the
 * AbsRegions of its active elements
 */
bool 
Slicer::handleReturn(
    Predicates & /* p */,
    SliceFrame & cur,
    bool & err)
{
    // Sanity check -- when we pop (in handleReturnDetails),
    // it should not result in context being empty
    //
    // FIXME I duplicated this from the old version; I don't
    //       understand why this doesn't set err = false.
    if(cur.con.size() <= 1)
        return false;

    // Find successor
    ParseAPI::Block * retBlock = NULL;
    
    const Block::edgelist & targets = cur.loc.block->targets();
    Block::edgelist::const_iterator eit = targets.begin();
    for(; eit != targets.end(); ++eit) {
        if((*eit)->type() == CALL_FT) {
            retBlock = (*eit)->trg();
            if ((*eit)->sinkEdge()) {
                cerr << "Weird!" << endl;
            }
            break;
        }
    }
    if(!retBlock) {
        err = true;
        return false;
    }

    // Pops a context and adjusts the abstract regions in `active'
    handleReturnDetails(cur);

    // Fix location given new context
    cur.loc.func = cur.con.front().func;
    cur.loc.block = retBlock;
    getInsns(cur.loc);

    return true;
}

/*
 * Do the actual context popping and active AbsRegion translation
 */
void
Slicer::handleReturnDetails(
    SliceFrame & cur)
{
    long stack_depth = cur.con.front().stackDepth;
    popContext(cur.con);

    assert(!cur.con.empty());

    slicing_printf("\t%s (%d), \n",
        (cur.con.front().func ? cur.con.front().func->name().c_str() : "NULL"),
        cur.con.front().stackDepth);

    // Transform the active abstract regions
    shiftAllAbsRegions(cur,-1*stack_depth,cur.con.front().func);
}

static bool EndsWithConditionalJump(ParseAPI::Block *b) {
    bool cond = false;
    for (auto eit = b->targets().begin(); eit != b->targets().end(); ++eit)
        if ((*eit)->type() == COND_TAKEN) cond = true;
    return cond;
}


bool
Slicer::handleDefault(
    Direction dir,
    Predicates & p,
    ParseAPI::Edge * e,
    SliceFrame & cur,
    bool & /* err */)
{
    if(dir == forward) {
        cur.loc.block = e->trg();
        getInsns(cur.loc);
    } else {
        cur.loc.block = e->src();
        getInsnsBackward(cur.loc);
	// We only track control flow dependencies when the user wants to
	if (p.searchForControlFlowDep() && EndsWithConditionalJump(e->src())) {
	    vector<Element> newE;	 
	    for (auto ait = cur.active.begin(); ait != cur.active.end(); ++ait) {	        
	        newE.insert(newE.end(), ait->second.begin(), ait->second.end());
	    }	
	    cur.active[AbsRegion(Absloc(MachRegister::getPC(cur.loc.block->obj()->cs()->getArch())))] = newE;
  	    slicing_printf("\tadding tracking ip for control flow information ");
	}
    }
    return true;
}

/* ----------------- backwards slicing implementations ------------------ */

bool
Slicer::handleCallBackward(
    Predicates & p,
    SliceFrame const& cand,
    vector<SliceFrame> & newCands,
    ParseAPI::Edge * e,
    bool & /* err */)
{
    // We don't know which function the caller block belongs to,
    // so check each possibility against the predicate.
    //
    // XXX   this suffers the same problem as followCall in the forward
    //       case; the predicates test against only a single abstract
    //       region. What we do here is to build up mappings from
    //       function paths (that will be followed) to sets of abstract
    //       regions, then create SliceFrames with these sets.

    map<ParseAPI::Function *, SliceFrame::ActiveMap > fmap;

    SliceFrame::ActiveMap::const_iterator ait = cand.active.begin();
    for( ; ait != cand.active.end(); ++ait) {
        vector<ParseAPI::Function *> follow = 
            followCallBackward(p,cand,(*ait).first,e->src());
        for(unsigned j=0;j<follow.size();++j) {
            fmap[follow[j]].insert(*ait);
        }
    }

    map<ParseAPI::Function *, SliceFrame::ActiveMap >::iterator mit = 
        fmap.begin();
    for( ; mit != fmap.end(); ++mit) {
        ParseAPI::Function * f = (*mit).first;
        SliceFrame::ActiveMap & act = (*mit).second;
    
        SliceFrame nf(cand.loc,cand.con);
        nf.active = act;

        nf.con.push_back(ContextElement(f));
        nf.loc.block = e->src();
        nf.loc.func = f;

        // pop context & adjust AbsRegions
        if(!handleCallDetailsBackward(nf)) {
            // FIXME I have preserved this behavior (returning false if
            //       the current frame can't be adjusted). It seems to
            //       me that we ought to set err = true and continue
            //       processing the rest of the call edges.
            //
            //       This issue needs review by somebody knowledgeable
            //       about the slicer.
            return false;
        }

        getInsnsBackward(nf.loc);
        newCands.push_back(nf);
    } 
    return true;
}

/*
 * FIXME egregious copying
 */
vector<ParseAPI::Function *>
Slicer::followCallBackward(
    Predicates & p,
    SliceFrame const& cand,
    AbsRegion const& reg,
    ParseAPI::Block * caller_block)
{
    stack< pair<ParseAPI::Function *, int> > callStack;  
    for(Context::const_reverse_iterator calls = cand.con.rbegin();
        calls != cand.con.rend(); ++calls)
    {
        if(calls->func) {
            callStack.push(
               std::make_pair(calls->func, calls->stackDepth));
        }
    }
    return p.followCallBackward(caller_block, callStack, reg);
}

bool
Slicer::handleCallDetailsBackward(
    SliceFrame & cur)
{
    ParseAPI::Block * callBlock = cur.loc.block;
    ParseAPI::Function * caller = cur.loc.func;

    Address callBlockLastInsn = callBlock->lastInsnAddr();

    long stack_depth;
    if(!getStackDepth(caller, callBlock, callBlockLastInsn,stack_depth)) {
        return false;
    }

    popContext(cur.con);
    assert(!cur.con.empty());


    slicing_printf("\t%s, %d\n",
        (cur.con.front().func ? cur.con.front().func->name().c_str() : "NULL"),
        cur.con.front().stackDepth);

    // Transform the active abstract regions
    shiftAllAbsRegions(cur,-1*stack_depth,caller);

    return true;
}
    
bool
Slicer::handleReturnBackward(
    Predicates & p,
    SliceFrame const& cand,
    SliceFrame & newCand,
    ParseAPI::Edge * e,
    bool & err)
{
    ParseAPI::Block * callee = e->src();

    // cons up a call stack for the call associated
    // with this return and ask the predicates whether
    // they would have come down this call.
    //
    // FIXME the issue of predicates evaluating only a
    //       single abstract region applies here as well;
    //       see comments in handleCall and handleCallBackward

    if(followReturn(p,cand,callee)) {
        // XXX it is not clear to me why a null callee is an error here
        //     but not if followReturn returns false FIXME
        if(!callee) {
            err = true;
            return false;
        }

        newCand = cand;
        newCand.loc.block = callee;
        newCand.loc.func = getEntryFunc(callee);
        getInsnsBackward(newCand.loc);

        if(!handleReturnDetailsBackward(newCand,cand.loc.block)) {
            err = true;
            return false;
        }
        return true;
    } 
    return false;
}

bool
Slicer::handleReturnDetailsBackward(
    SliceFrame & cur,
    ParseAPI::Block * caller_block)
{
    ParseAPI::Function * caller = cur.con.front().func;
    ParseAPI::Function * callee = cur.loc.func;

    long stack_depth;
    if(!getStackDepth(caller,caller_block, caller_block->end(),stack_depth)) {
        return false;
    }
    
    pushContext(cur.con, callee, caller_block, stack_depth);

    // Transform the active abstract regions
    shiftAllAbsRegions(cur,stack_depth,caller);

    return true; 
}

bool
Slicer::followReturn(
    Predicates & p,
    SliceFrame const& cand,
    ParseAPI::Block * source)
{
    ParseAPI::Function * callee = (source ? getEntryFunc(source) : NULL);
    
    stack< pair<ParseAPI::Function *, int> > callStack;
    for(Context::const_reverse_iterator calls = cand.con.rbegin();
        calls != cand.con.rend(); ++calls)
    {
        if(calls->func) {
            callStack.push(
               std::make_pair(calls->func, calls->stackDepth));
        }
    }

    bool ret = false;
    SliceFrame::ActiveMap::const_iterator ait = cand.active.begin();
    for( ; ait != cand.active.end(); ++ait) {
        ret = ret || p.followCall(callee,callStack,(*ait).first);
    }
    return ret;
}
    


/* ------------------------------------------- */

Address SliceNode::addr() const { 
  if (a_)
    return a_->addr();
  return 0;
}

bool containsCall(ParseAPI::Block *block) {
  // We contain a call if the out-edges include
  // either a CALL or a CALL_FT edge
  const Block::edgelist &targets = block->targets();
  Block::edgelist::const_iterator eit = targets.begin();
  for (; eit != targets.end(); ++eit) {
    ParseAPI::Edge *edge = *eit;
    if (edge->type() == CALL) return true;
  }
  return false;
}

bool containsRet(ParseAPI::Block *block) {
  // We contain a call if the out-edges include
  // either a CALL or a CALL_FT edge
  const Block::edgelist &targets = block->targets();
  Block::edgelist::const_iterator eit = targets.begin();
  for (; eit != targets.end(); ++eit) {
    ParseAPI::Edge *edge = *eit;
    if (edge->type() == RET) return true;
  }
  return false;
}

static void getInsnInstances(ParseAPI::Block *block,
		      Slicer::InsnVec &insns) {
  ParseAPI::Block::Insns bi;
  block->getInsns(bi);
  for (auto iit = bi.begin(); iit != bi.end(); ++iit) {
    insns.emplace_back(std::make_pair(iit->second, iit->first));
  }
}

ParseAPI::Function *getEntryFunc(ParseAPI::Block *block) {
  return block->obj()->findFuncByEntry(block->region(), block->start());
}

// Constructor. Takes the initial point we slice from. 

// TODO: make this function-less interprocedural. That would throw the
// stack analysis for a loop, but is generally doable...
Slicer::Slicer(Assignment::Ptr a,
               ParseAPI::Block *block,
               ParseAPI::Function *func,
	       bool cache,
	       bool stackAnalysis) : 
  insnCache_(new InsnCache()),
  own_insnCache(true),
  a_(a),
  b_(block),
  f_(func),
  converter(new AssignmentConverter(cache, stackAnalysis)),
  own_converter(true)
{
}

Slicer::Slicer(Assignment::Ptr a,
               ParseAPI::Block *block,
               ParseAPI::Function *func,
               AssignmentConverter* ac):
  insnCache_(new InsnCache()),
  own_insnCache(true),
  a_(a),
  b_(block),
  f_(func),
  converter(ac),
  own_converter(false)
{
}

Slicer::Slicer(Assignment::Ptr a,
               ParseAPI::Block *block,
               ParseAPI::Function *func,
               AssignmentConverter* ac,
               InsnCache* c):
  insnCache_(c),
  own_insnCache(false),
  a_(a),
  b_(block),
  f_(func),
  converter(ac),
  own_converter(false)
{
}


Slicer::~Slicer()
{
  if (own_converter)
    delete converter;
  if (own_insnCache)
    delete insnCache_;
}


Graph::Ptr Slicer::forwardSlice(Predicates &predicates) {

	// delete cache state
  unique_edges_.clear(); 

  return sliceInternal(forward, predicates);
}

Graph::Ptr Slicer::backwardSlice(Predicates &predicates) {
  // delete cache state
  unique_edges_.clear(); 

  return sliceInternal(backward, predicates);
}

bool Slicer::getStackDepth(ParseAPI::Function *func, ParseAPI::Block *block, Address callAddr, long &height) {
  StackAnalysis sA(func);

  StackAnalysis::Height heightSA = sA.findSP(block, callAddr);

  // Ensure that analysis has been performed.

  assert(!heightSA.isTop());
  
  if (heightSA.isBottom()) {
    return false;
  }
  
  height = heightSA.height();
  
  // The height will include the effects of the call
  // Should check the region... 

  //slicing_cerr << "Get stack depth at " << std::hex << callAddr
  //<< std::dec << " " << (int) height << endl;

  return true;
}

void Slicer::pushContext(Context &context,
			 ParseAPI::Function *callee,
			 ParseAPI::Block *callBlock,
			 long stackDepth) {
  slicing_cerr << "pushContext with " << context.size() << " elements" << endl;
  assert(context.front().block == NULL);
  context.front().block = callBlock;

  slicing_cerr << "\t" 
	       << (context.front().func ? context.front().func->name() : "NULL")
	       << ", " 
	       << context.front().stackDepth 
	       << endl;

    context.push_front(ContextElement(callee, stackDepth));
}

void Slicer::popContext(Context &context) {
  context.pop_front();

  context.front().block = NULL;
}

void Slicer::shiftAbsRegion(AbsRegion const&callerReg,
			    AbsRegion &calleeReg,
			    long stack_depth,
			    ParseAPI::Function *callee) {
  if (callerReg.absloc() == Absloc()) {
    // Typed, so just keep the same type and call it a day
    calleeReg = callerReg;
    return;
  }
  else {
    assert(callerReg.type() == Absloc::Unknown);
    
    const Absloc &callerAloc = callerReg.absloc();
    if (callerAloc.type() != Absloc::Stack) {
      calleeReg = AbsRegion(callerAloc);
    }
    else {
      if (stack_depth == -1) {
        // Oops
	calleeReg = AbsRegion(Absloc::Stack);
	return;
      }
      else {
        //slicing_cerr << "*** Shifting caller absloc " << callerAloc.off()
        //<< " by stack depth " << stack_depth 
        //<< " and setting to function " << callee->name() << endl;
	calleeReg = AbsRegion(Absloc(callerAloc.off() - stack_depth,
				     0, // Entry point has region 0 by definition
				     callee));
      }
    }
  }
}

bool Slicer::kills(AbsRegion const&reg, Assignment::Ptr &assign) {
  // Did we find a definition of the same abstract region?
  // TBD: overlaps ins't quite the right thing here. "contained
  // by" would be better, but we need to get the AND/OR
  // of abslocs hammered out.
  
  if (assign->out().type() != Absloc::Unknown) {
    // A region assignment can never kill
    return false; 
  }

  if (assign->insn().getOperation().getID() == e_call && reg.absloc().type() == Absloc::Register) {
      MachRegister r = reg.absloc().reg();
      ABI* abi = ABI::getABI(b_->obj()->cs()->getAddressWidth());
      int index = abi->getIndex(r);
      if (index >= 0)
          if (abi->getCallWrittenRegisters()[abi->getIndex(r)] && r != x86_64::r11) return true;
  }
  return reg.contains(assign->out()) || assign->out().contains(reg);
}

// creates a new node from an element if that node does not yet exist.
// otherwise, it returns the pre-existing node.
SliceNode::Ptr Slicer::createNode(Element const&elem) {
  if (created_.find(elem.ptr) != created_.end()) {
    return created_[elem.ptr];
  }
  SliceNode::Ptr newNode = SliceNode::create(elem.ptr, elem.block, elem.func);
  created_[elem.ptr] = newNode;

  // mark this node as plausibly being entry/exit.
  plausibleNodes.insert(newNode); 
  return newNode;
}

std::string SliceNode::format() const {
  if (!a_) {
    return "<NULL>";
  }

  stringstream ret;
  ret << "(" << a_->format() << "@" <<
    f_->name() << ")";
  return ret.str();
}

// converts an instruction to a vector of assignments. if this slicer has
// already converted this instruction, this function returns the same
// assignments.
// Note that we CANNOT use a global cache based on the address
// of the instruction to convert because the block that contains
// the instructino may change during parsing.
void Slicer::convertInstruction(const Instruction &insn,
                                Address addr,
                                ParseAPI::Function *func,
                                ParseAPI::Block *block,
                                std::vector<Assignment::Ptr> &ret) {
  converter->convert(insn,
		    addr,
		    func,
                    block,
		    ret);
  return;
}

void Slicer::getInsns(Location &loc) {


  InsnCache::iterator iter = insnCache_->find(loc.block->start());
  if (iter == insnCache_->end()) {
    getInsnInstances(loc.block, (*insnCache_)[loc.block->start()]);
  }
  
  loc.current = (*insnCache_)[loc.block->start()].begin();
  loc.end = (*insnCache_)[loc.block->start()].end();
}

void Slicer::getInsnsBackward(Location &loc) {
    assert(loc.block->start() != (Address) -1); 
    InsnCache::iterator iter = insnCache_->find(loc.block->start());
    if (iter == insnCache_->end()) {
      getInsnInstances(loc.block, (*insnCache_)[loc.block->start()]);
    }

    loc.rcurrent = (*insnCache_)[loc.block->start()].rbegin();
    loc.rend = (*insnCache_)[loc.block->start()].rend();
}

// inserts an edge from source to target (forward) or target to source
// (backward) if the edge does not yet exist. this is done by converting
// source and target to graph nodes (creating them if they do not exist).
void Slicer::insertPair(Graph::Ptr ret,
			Direction dir,
			Element const&source,
			Element const&target,
			AbsRegion const& data) 
{
    SliceNode::Ptr s = createNode(source);
    SliceNode::Ptr t = createNode(target);

    insertPair(ret, dir, s, t, data);
}

// inserts an edge from source to target (forward) or target to source
// (backward) if the edge does not yet exist.
void Slicer::insertPair(Graph::Ptr ret,
			Direction dir,
			SliceNode::Ptr& s,
			SliceNode::Ptr& t,
			AbsRegion const& data) 
{

  EdgeTuple et(s,t,data);
  if(unique_edges_.find(et) != unique_edges_.end()) {
    unique_edges_[et] += 1;
    return;
  }
  unique_edges_[et] = 1;  

  if (dir == forward) {
     SliceEdge::Ptr e = SliceEdge::create(s, t, data);
     ret->insertPair(s, t, e);

     // this node is clearly not entry/exit.
     plausibleNodes.erase(s);
  } else {
     SliceEdge::Ptr e = SliceEdge::create(t, s, data);
     ret->insertPair(t, s, e);

     // this node is clearly not entry/exit.
     plausibleNodes.erase(s); 
  }
}

void
Slicer::widenAll(
    Graph::Ptr g,
    Direction dir,
    SliceFrame const& cand)
{
    SliceFrame::ActiveMap::const_iterator ait = cand.active.begin();
    for( ; ait != cand.active.end(); ++ait) {
        vector<Element> const& eles = (*ait).second;
        for(unsigned i=0;i<eles.size();++i)
            widen(g,dir,eles[i]);
    }
}

void Slicer::widen(Graph::Ptr ret,
		   Direction dir,
		   Element const&e) {
  if (dir == forward) {
    ret->insertPair(createNode(e),
		    widenNode());
    ret->insertExitNode(widenNode());
  }
  else {
    ret->insertPair(widenNode(), createNode(e));
    ret->insertEntryNode(widenNode());
  }
}

SliceNode::Ptr Slicer::widenNode() {
  if (widen_) {
    return widen_;
  }

  widen_ = SliceNode::create(Assignment::Ptr(),
			      NULL, NULL);
  return widen_;
}

void Slicer::markAsEndNode(Graph::Ptr ret, Direction dir, Element &e) {
  if (dir == forward) {    
    ret->insertExitNode(createNode(e));
  }
  else {
    ret->insertEntryNode(createNode(e));
  }
}

void Slicer::fastForward(Location &loc, Address
			 addr) {
    while ((loc.current != loc.end) && (loc.addr() < addr)) {
        loc.current++;
    }

    if (loc.current == loc.end || loc.addr() != addr) {
        slicing_cerr << "Cannot find addr " << std::hex << addr 
            << "in block [" << loc.block->start() << "," << loc.block->end() << ")" 
            << ", function " << loc.func->name() << " at " << loc.func->addr()
            << std::dec << endl;
    }
}

void Slicer::fastBackward(Location &loc, Address addr) {
    while ((loc.rcurrent != loc.rend) && (loc.addr() > addr)) {
        loc.rcurrent++;
    }

    if (loc.rcurrent == loc.rend || loc.addr() != addr) {
        slicing_cerr << "Cannot find addr " << std::hex << addr 
            << "in block [" << loc.block->start() << "," << loc.block->end() << ")" 
            << ", function " << loc.func->name() << " at " << loc.func->addr()
            << std::dec << endl;
    }
}

// removes unnecessary nodes from the slice graph. this is
// currently mostly ia32/amd64 flags that are written but
// never read. 
void Slicer::cleanGraph(Graph::Ptr ret) {
  slicing_cerr << "Cleaning up the graph..." << endl;
  // Clean the graph up
  
  // TODO: make this more efficient by backwards traversing.
  // For now, I know that we're generating graphs with tons of
  // unnecessary flag sets (which are immediately killed) and so
  // we don't have long non-exiting chains, we have "fuzz"
  
  NodeIterator nbegin, nend;
  ret->allNodes(nbegin, nend);
  
  std::list<Node::Ptr> toDelete;
  unsigned numNodes = 0;
  for (; nbegin != nend; ++nbegin) {
	  expand_cerr << "NodeIterator in cleanGraph: " << (*nbegin)->format() << endl;
      numNodes++;
      SliceNode::Ptr foozle =
          boost::dynamic_pointer_cast<SliceNode>(*nbegin);
      //cerr << "Checking " << foozle << "/" << foozle->format() << endl;
      if ((*nbegin)->hasOutEdges()) {
          slicing_cerr << "\t has out edges, leaving in" << endl;
          continue;
      }

      // don't remove entry/exit nodes.
      if (ret->isEntryNode(*nbegin) || ret->isExitNode(*nbegin)) {
          continue;
      }

      // mark non-flags nodes as exit nodes. these nodes are likely
      // relevant to the slice and should not be deleted. only delete 
      // flag nodes. this should stop the over-deleting by this
      // function, but this is a fairly ugly way of doing it. 
      const Absloc& abs = foozle->a_->out().absloc();
      if (abs.type() == Absloc::Register && 
                        (abs.reg().getArchitecture() == Arch_x86 
                         || abs.reg().getArchitecture() == Arch_x86_64) && 
                        (abs.reg().getBaseRegister() & x86::FLAG) == x86::FLAG) {
          slicing_cerr << "\t deleting" << endl;
          toDelete.push_back(*nbegin);
      } else {
          ret->markAsExitNode(foozle);
      }
  }

  for (std::list<Node::Ptr>::iterator tmp =
	   toDelete.begin(); tmp != toDelete.end(); ++tmp) {
      ret->deleteNode(*tmp);
  }
  slicing_cerr << "\t Slice has " << numNodes << " nodes" << endl;
}

// promotes nodes in the slice graph to termination nodes.
// essentially, the slicer maintains a set of nodes that 
// may be entry/exit nodes for the backwards/fowards case.
// this function removes the nodes from the set, and 
// marks them in the graph as true entry/exit nodes.
// in the forward case, the entry node is a single node,
// the assignment from which the slice began. in the backward
// case, this node is the single exit node. exit nodes in the
// forward case are definitions that are still live at function
// exit. entry nodes in the backward case are uses for which the
// definition lies outside the function (before entry) or move
// instructions where one operand is a literal.
void Slicer::promotePlausibleNodes(GraphPtr g, Direction d) {
    // note: it would be better to reuse some other
    // functions here, but none of them quite use 
    // the interface we need here.
    if (d == forward) {
        for (auto first = plausibleNodes.begin(), last = plausibleNodes.end();
             first != last; ++first) {
            g->markAsExitNode(*first);
        }
    } else {
        for (auto first = plausibleNodes.begin(), last = plausibleNodes.end();
             first != last; ++first) {
            g->markAsEntryNode(*first);
        }
    }
    plausibleNodes.clear();
}

ParseAPI::Block *Slicer::getBlock(ParseAPI::Edge *e,
				   Direction dir) {
  return ((dir == forward) ? e->trg() : e->src());
}

bool Slicer::isWidenNode(Node::Ptr n) {
  SliceNode::Ptr foozle =
    boost::dynamic_pointer_cast<SliceNode>(n);
  if (!foozle) return false;
  if (!foozle->assign()) return true;
  return false;
}

void Slicer::insertInitialNode(GraphPtr ret, Direction dir, SliceNode::Ptr aP) {
  if (dir == forward) {
    // Entry node
    ret->insertEntryNode(aP);
  }
  else {
    ret->insertExitNode(aP);
  }
}

// creates the initial slice frame and initializes instance variables.
void Slicer::constructInitialFrame(
    Direction dir,
    SliceFrame & initFrame)
{
    Instruction init_instruction;
    initFrame.con.push_front(ContextElement(f_));
    initFrame.loc = Location(f_,b_);

    // increment iterators to initial instruction. 
    if(dir == forward) {
        initFrame.loc.fwd = true;
        getInsns(initFrame.loc);
        fastForward(initFrame.loc, a_->addr());
        init_instruction = initFrame.loc.current->first;
    } else {
        initFrame.loc.fwd = false;
        getInsnsBackward(initFrame.loc);
        fastBackward(initFrame.loc, a_->addr());
        init_instruction = initFrame.loc.rcurrent->first;
    }

    // reconstruct initial assignment. the initial assignment was created
    // by a different converter, and thus if the instruction is visited
    // more than once by the slicer, the assignment will be recreated
    // instead of coming out of cache. this results in duplicated graph
    // nodes. 
    std::vector<Assignment::Ptr> assigns;
    convertInstruction(init_instruction, a_->addr(), f_, b_, assigns);
    for (auto first = assigns.begin(), last = assigns.end(); first != last; ++first) {
        if ((*first)->out() == a_->out()) { a_ = *first; break; }
    }

    if(dir == forward) {
        Element oe(b_,f_,a_->out(),a_);
        initFrame.active[a_->out()].push_back(oe);
    } else {
        vector<AbsRegion> & inputs = a_->inputs();
        vector<AbsRegion>::iterator iit = inputs.begin();
        for ( ; iit != inputs.end(); ++iit) {
            Element ie(b_,f_,*iit,a_);
            initFrame.active[*iit].push_back(ie);
        }
    }
}

void
Slicer::DefCache::merge(Slicer::DefCache const& o)
{
    map<AbsRegion, set<Def> >::const_iterator oit = o.defmap.begin();
    for( ; oit != o.defmap.end(); ++oit) {
        AbsRegion const& r = oit->first;
        set<Def> const& s = oit->second;
        defmap[r].insert(s.begin(),s.end());
    }
}

void
Slicer::DefCache::replace(Slicer::DefCache const& o)
{   
    // XXX if o.defmap[region] is empty set, remove that entry
    map<AbsRegion, set<Def> >::const_iterator oit = o.defmap.begin();
    for( ; oit != o.defmap.end(); ++oit) {
        if(!(*oit).second.empty())
            defmap[(*oit).first] = (*oit).second;
        else
            defmap.erase((*oit).first);
    }
}

void
Slicer::DefCache::print() const {
    map<AbsRegion, set<Def> >::const_iterator it = defmap.begin();
    for( ; it !=defmap.end(); ++it) {
        slicing_printf("\t\t%s ->\n",(*it).first.format().c_str());
        set<Def> const& defs = (*it).second;
        set<Def>::const_iterator dit = defs.begin();
        for( ; dit != defs.end(); ++dit) {
            slicing_printf("\t\t\t<%s,%s>\n",
                (*dit).ele.ptr->format().c_str(),
                (*dit).data.format().c_str());
        }
    }
}

// merges all single caches that have occured single addr in the
// recursion into the appropriate unified caches.
void Slicer::mergeRecursiveCaches(std::unordered_map<Address, DefCache>& single, 
                                  std::unordered_map<Address, DefCache>& unified, Address) {


    for (auto first = addrStack.rbegin(), last = addrStack.rend();
            first != last; ++first) {
        unified[*first].replace(single[*first]);
        auto next = first + 1;
        if (next != last) {
            unified[*next].merge(unified[*first]);
        }
    }
}

