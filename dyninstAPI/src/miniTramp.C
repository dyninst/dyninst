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

// $Id: miniTramp.C,v 1.41 2008/02/20 22:34:20 legendre Exp $
// Code to install and remove instrumentation from a running process.

#include "miniTramp.h"
#include "baseTramp.h"
#include "instP.h"
#include "instPoint.h"
#include "process.h"
#include "ast.h"
#include "addressSpace.h"
#include "dyninstAPI/h/BPatch.h"

// for AIX
#include "function.h"

int miniTramp::_id = 1;

/*
 * The tramps are chained together left to right, so we need to find the
 *    tramps to the left anf right of the one to delete, and then patch the
 *    call from the left one to the old to call the right one.
 *    Also we need to patch the return from the right one to go to the left
 *    one.
 *
 * New logic: this routine gaps the minitramp out of the execution
 * sequence, but leaves deletion for later. There is a routine in 
 * the process object which maintains a list of elements to be deleted,
 * and the associated data to ensure that deletion is safe. I've
 * added a callback function to the instInstance class which is 
 * called when deletion actually takes place. This allows recordkeeping
 * for any data which may rely on the minitramp (i.e. Paradyn variables)
 *
 */


// returns true if deleted, false if not deleted (because non-existant
// or already deleted
bool miniTramp::uninstrument() {

  // First check: have we started to delete this guy already?
  // This happens when we try to delete an instInstance and GC it
  // We then pause the process, but if the process is exited Paradyn
  // tries to disable all instrumentation... thus causing this one
  // to be deleted again. Sigh. 
  
  // Better fix: figure out why we're double-deleting instrCodeNodes.
	cerr << "Removing miniTramp from " << func()->symTabName() << " @ addr " << hex << instP()->addr() << dec << endl;
    if (instP()->addr() == 0x9bb7b1) {
        DebugBreak();
    }
    if (proc()->proc() &&
        !proc()->proc()->isAttached()) {
        return true;
    }

    if (deleteInProgress) {
        return false;
    }

    
    stats_instru.startTimer(INST_REMOVE_TIMER);
    stats_instru.incrementCounter(INST_REMOVE_COUNTER);

    deleteInProgress = true;

    for (baseTramp::iterator iter = baseT->begin(); iter != baseT->end(); ++iter) {
       if ((*iter) == this) {
          baseT->miniTramps_.erase(iter);
          break;
       }
    }
    
    // DON'T delete the miniTramp. When it is deleted, the callback
    // is made... which should only happen when the memory is freed.
    // Place it on the list to be deleted.
    topDownDelete_ = true;

    // Inform the AddressSpace that we have modified a particular function.
    proc()->addModifiedFunction(func());
    
    stats_instru.stopTimer(INST_REMOVE_TIMER);
    return true;
}

bool miniTramp::generateMT(registerSpace *rs) 
{
    //inst_printf("AST pointer is %p\n", ast_);

    // This can be called multiple times
    if (miniTrampCode_ != NULL) return true;

    miniTrampCode_.allocate(MAX_MINITRAMP_SIZE);
    miniTrampCode_.setAddrSpace(proc());
    miniTrampCode_.setRegisterSpace(rs);
    miniTrampCode_.setPoint(instP());

    /* VG(11/06/01): Added location, needed by effective address AST node */
    returnOffset = ast_->generateTramp(miniTrampCode_, 
                                       cost, false);

    size_ = miniTrampCode_.used();
    miniTrampCode_.finalize();

    return true;
}

bool miniTramp::correctMTJumps() {
    return true;
}

miniTramp::miniTramp(callWhen when_,
                     AstNodePtr ast,
                     baseTramp *base,
                     bool noCost) :
    miniTrampCode_(),
    ID(_id++), 
    returnOffset(0), 
    size_(0),
    baseT(base),
    proc_(NULL),
    when(when_),
    cost(0), 
    noCost_(noCost),
	topDownDelete_(false),
    prev(NULL), next(NULL),
    callback(NULL), callbackData(NULL),
    deleteInProgress(false) {
    ast_ = dyn_detail::boost::dynamic_pointer_cast<AstMiniTrampNode>(AstNode::miniTrampNode(ast));

    assert(baseT);
    proc_ = baseT->proc();
}

miniTramp::miniTramp(const miniTramp *parMini,
                     baseTramp *childT,
                     process *proc) :
    miniTrampCode_(parMini->miniTrampCode_),
    ID(parMini->ID),
    returnOffset(parMini->returnOffset),
    size_(parMini->size_),
    baseT(childT),
    proc_(proc),
    when(parMini->when),
    cost(parMini->cost),
    noCost_(parMini->noCost_),
    topDownDelete_(parMini->topDownDelete_),
    prev(NULL),
    next(NULL),
    callback(NULL),
    callbackData(NULL),
    deleteInProgress(parMini->deleteInProgress)
{
    ast_ = parMini->ast_;

    // Uhh... what about callbacks?
    // Can either set them to null or have them returning 
    // the same void * as their parent...

}    

miniTramp::~miniTramp() {
    if (callback)
        (*callback)(callbackData, this);  
    // FIXME
    //removeAst(ast_);
}

// Given a miniTramp parentMT, find the equivalent in the child
// process (matching by the ID member). Fill in childMT.
  
miniTramp *miniTramp::getInheritedMiniTramp(process *childProc) {
    int_function *childF = childProc->findFuncByInternalFunc(func()->ifunc());
    assert(childF);
    instPoint *childP = childF->findInstPByAddr(instP()->addr());
    
    baseTramp *childB = childP->getBaseTramp(when);

    for (baseTramp::iterator iter = childB->begin(); iter != childB->end(); ++iter) {
       if ((*iter)->ID == ID)
          return *iter;
    }

    return NULL;
}

// Returns true if the "current" is after the "new". Currently does not
// handle in-lining, where you might need catchup even if you're 
// "before".
bool miniTramp::catchupRequired(miniTramp *curMT, miniTramp *newMT, bool) 
{
    // We start at current and iterate. If we see new, stop. If
    // we hit the end, stop.
    miniTramp *iterMT = newMT;

    while (iterMT) {
        if (iterMT == curMT) {
            // Oops, we just hit our current.
            return true;
        }
        iterMT = iterMT->next;
    }

    // We didn't hit it. Optimistically assume that newMT is after
    // curMT, and thus we don't need to worry. 
    
    // TODO INLINE
    return false;
}


instPoint *miniTramp::instP() const {
    return baseT->instP();
}

int_function *miniTramp::func() const {
    return baseT->instP()->func();
}

bool miniTramp::instrumentedViaTrap() const {
   assert(0);
   return false;
}
