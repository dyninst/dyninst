/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// $Id: trampTemplate.C,v 1.2 2004/03/23 01:12:11 eli Exp $

#include "dyninstAPI/src/trampTemplate.h"
#include "dyninstAPI/src/installed_miniTramps_list.h"
#include "dyninstAPI/src/miniTrampHandle.h"
#include "dyninstAPI/src/process.h"

// Normal constructor
trampTemplate::trampTemplate(const instPoint *l, process *p) : 
        baseAddr(0),
        prevInstru(false), postInstru(false),
        prevBaseCost(0), postBaseCost(0),
        location(l), proc(p),
        pre_minitramps(NULL), post_minitramps(NULL) {
    p->baseMap[l] = this;
};


    // Fork constructor
trampTemplate::trampTemplate(const trampTemplate *t, process *p) : 
        size(t->size),
        trampTemp(t->trampTemp), baseAddr(t->baseAddr),
        localPreOffset(t->localPreOffset), localPreReturnOffset(t->localPreReturnOffset),
        localPostOffset(t->localPostOffset), localPostReturnOffset(t->localPostReturnOffset),
        returnInsOffset(t->returnInsOffset),
        skipPreInsOffset(t->skipPreInsOffset),
        skipPostInsOffset(t->skipPostInsOffset),
        emulateInsOffset(t->emulateInsOffset),
        updateCostOffset(t->updateCostOffset),
        savePreInsOffset(t->savePreInsOffset),
        restorePreInsOffset(t->restorePreInsOffset),
        savePostInsOffset(t->savePostInsOffset),
        restorePostInsOffset(t->restorePostInsOffset),
#if defined(rs6000_ibm_aix4_1)
        recursiveGuardPreJumpOffset(t->recursiveGuardPreJumpOffset),
        recursiveGuardPostJumpOffset(t->recursiveGuardPostJumpOffset),
#elif defined(mips_sgi_irix6_4)
        recursiveGuardPreOnOffset(t->recursiveGuardPreOnOffset),
        recursiveGuardPreOffOffset(t->recursiveGuardPreOffOffset),
        recursiveGuardPostOnOffset(t->recursiveGuardPostOnOffset),
        recursiveGuardPostOffOffset(t->recursiveGuardPostOffOffset),
#endif
        cost(t->cost), costAddr(t->costAddr),
        prevInstru(t->prevInstru), postInstru(t->postInstru),
        prevBaseCost(t->prevBaseCost), postBaseCost(t->postBaseCost),
        location(t->location),
        // IMPORTANT: don't copy process pointer! Needs to be set!
        proc(p),
        // ALSO CLONE MINITRAMP LISTS!
        pre_minitramps(NULL), post_minitramps(NULL) {
    // Clone the minitramp lists
    miniTramps_list *mt = t->pre_minitramps;
    if (mt) {
        pre_minitramps = new miniTramps_list;
        List<miniTrampHandle *>::iterator iter = mt->get_begin_iter();
        List<miniTrampHandle *>::iterator end = mt->get_end_iter();
        for (; iter != end; iter++) {
            miniTrampHandle *mth = new miniTrampHandle(*iter, this);
            pre_minitramps->addMiniTramp(orderLastAtPoint, mth);
        }
    }
    mt = t->post_minitramps;
    if (mt) {
        post_minitramps = new miniTramps_list;
        List<miniTrampHandle *>::iterator iter = mt->get_begin_iter();
        List<miniTrampHandle *>::iterator end = mt->get_end_iter();
        for (; iter != end; iter++) {
            miniTrampHandle *mth = new miniTrampHandle(*iter, this);
            post_minitramps->addMiniTramp(orderLastAtPoint, mth);
        }
    }    
}

// Destructor -- just cleans up data, doesn't actually delete
// things in memory
trampTemplate::~trampTemplate() {
    miniTramps_list *mt = pre_minitramps;
    if (mt) {
        List<miniTrampHandle *>::iterator iter = mt->get_begin_iter();
        List<miniTrampHandle *>::iterator end = mt->get_end_iter();
        for (; iter != end; iter++) {
            delete (*iter);
        }
        delete mt;
    }
    mt = post_minitramps;
    if (mt) {
        List<miniTrampHandle *>::iterator iter = mt->get_begin_iter();
        List<miniTrampHandle *>::iterator end = mt->get_end_iter();
        for (; iter != end; iter++) {
            delete (*iter);
        }
        delete mt;
    }
    // Sometimes we have a "generic" template that gets filled in with
    // offsets and such. It never gets added to the basemap.
    if (proc) proc->baseMap.undef(location);
}

// Assignment
trampTemplate &trampTemplate::operator=(const trampTemplate &t) {
    size = t.size;
    trampTemp = t.trampTemp;
    // baseAddr must be manually set
    localPreOffset = t.localPreOffset;
    localPreReturnOffset = t.localPreReturnOffset;
    localPostOffset = t.localPostOffset;
    localPostReturnOffset = t.localPostReturnOffset;
    returnInsOffset = t.returnInsOffset;
    skipPreInsOffset = t.skipPreInsOffset;
    skipPostInsOffset = t.skipPostInsOffset;
    emulateInsOffset = t.emulateInsOffset;
    updateCostOffset = t.updateCostOffset;
    savePreInsOffset = t.savePreInsOffset;
    restorePreInsOffset = t.restorePreInsOffset;
    savePostInsOffset = t.savePostInsOffset;
    restorePostInsOffset = t.restorePostInsOffset;
#if defined(rs6000_ibm_aix4_1)
    recursiveGuardPreJumpOffset = t.recursiveGuardPreJumpOffset;
    recursiveGuardPostJumpOffset = t.recursiveGuardPostJumpOffset;
#elif defined(mips_sgi_irix6_4)
    recursiveGuardPreOnOffset = t.recursiveGuardPreOnOffset;
    recursiveGuardPreOffOffset = t.recursiveGuardPreOffOffset;
    recursiveGuardPostOnOffset = t.recursiveGuardPostOnOffset;
    recursiveGuardPostOffOffset = t.recursiveGuardPostOffOffset;
#endif
    cost = t.cost;
    costAddr = t.costAddr;
    return *this;
}



    
