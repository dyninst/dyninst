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

// $Id: LocalAlteration.C,v 1.11 2005/01/21 23:44:07 bernat Exp $

#include "dyninstAPI/src/LocalAlteration.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/FunctionExpansionRecord.h"
#include "common/h/Vector.h"
#include "dyninstAPI/src/dyninst.h"
#include "dyninstAPI/src/arch.h"
#include "dyninstAPI/src/util.h"
#include <assert.h>

//#ifndef DEBUG_PA_INST
//#define DEBUG_PA_INST
//#endif

// constructor for LocalAlteration....
LocalAlteration::LocalAlteration(int_function *f, int offset) {
    function = f;
    beginning_offset = offset;
}

// constructor for InsertNops....
InsertNops::InsertNops(int_function *f, int offset, int size):
    LocalAlteration(f, offset) 
{
    // Size should correspond to integer # of nop instructions.... 
    assert((size % sizeOfNop()) == 0);
    sizeNopRegion = size;
}

// update branches :
//  Add extra offset to FunctionExpansionRecord to modify all branches
//  around.  Currently only setup so that footprint has size of 0
//  (in ORIGIONAL CODE) so branches into shouldn't happen, eh???? 
bool InsertNops::UpdateExpansions(FunctionExpansionRecord *fer) {
    fer->AddExpansion(beginning_offset, sizeNopRegion);
    return true;
}

// Update location of inst points in function.  In case of InsertNops, 
//  changes to inst points same as changes to branch targets....
bool InsertNops::UpdateInstPoints(FunctionExpansionRecord *ips) {
    return UpdateExpansions(ips);
}

int InsertNops::getOffset() const {
    return beginning_offset;
}

int InsertNops::getShift() const {
    return sizeNopRegion;
}

LocalAlterationSet::LocalAlterationSet(int_function *f) {
    func = f;
    iterIdx = -1;
    ordered = false;
}

LocalAlterationSet::LocalAlterationSet() 
{
    func = (int_function *)NULL;
    iterIdx = -1;
    ordered = false;
}

LocalAlterationSet::~LocalAlterationSet() {
  //    Flush();
}

LocalAlteration *LocalAlterationSet::getAlterationAtOffset(int byte_offset) {
   for(unsigned i=0; i<alterations.size(); i++) {
      LocalAlteration *cur_alter = alterations[i];
      if(cur_alter->getOffset() == byte_offset)
         return cur_alter;
   }   
   return NULL;
}

void LocalAlterationSet::Flush() {
    iterIdx = -1;
    ordered = false;

    //    fer.DeleteNodes();
    fer.Flush();
    //    ips.DeleteNodes();
    ips.Flush();
    
    for(unsigned i=0;i<alterations.size();i++) {
        alterations[i] = 0;
    }
    alterations.resize(0);
}

void LocalAlterationSet::DeleteAlterations() {
    unsigned i;
    for(i=0;i<alterations.size();i++) {
        delete alterations[i];
    }
    alterations.resize(0);
    ordered = false;
}

void LocalAlterationSet::AddAlteration(LocalAlteration *a) {
    alterations.push_back(a);
    a->UpdateExpansions(&fer);
    a->UpdateInstPoints(&ips);
    ordered = false;
}

int order_peephole_alteration_offsets(const void *a1, const void *a2) {
    const LocalAlteration* la1 = *(const LocalAlteration* const*)a1;
	const LocalAlteration* la2 = *(const LocalAlteration* const*)a2;

    int offset1 = la1->getOffset();
    int offset2 = la2->getOffset();
    return (offset1 - offset2);
}

void LocalAlterationSet::Order() {
    if (ordered == true) return;
    VECTOR_SORT(alterations, order_peephole_alteration_offsets);
    ordered = true;
}

//
// Iterator Code
//
void LocalAlterationSet::iterReset() {
    Order();
    iterIdx = -1;
}

LocalAlteration *LocalAlterationSet::iterNext() {
    Order();
    if ( ++iterIdx < (int)(alterations.size()) ) {
        return alterations[iterIdx];
    }
    return NULL;
}

int LocalAlterationSet::sizeChange() {
    return fer.sizeChange();
}

