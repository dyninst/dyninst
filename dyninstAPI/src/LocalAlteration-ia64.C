/* -*- Mode: C; indent-tabs-mode: true; tab-width: 4 -*- */

/*
 * Copyright (c) 1996-2001 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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

// $Id: LocalAlteration-ia64.C,v 1.3 2003/06/27 20:57:52 tlmiller Exp $

#include "dyninstAPI/src/LocalAlteration.h"
#include "dyninstAPI/src/LocalAlteration-x86.h"

// constructor for ExpandInstruction local alteration....
ExpandInstruction::ExpandInstruction(pd_Function *f, int offset, int size):
    LocalAlteration(f, offset) 
{
    extra_bytes = size;
}

// update branches :
//  Add extra offset to FunctionExpansionRecord to modify all branches
//  around.  Currently only setup so that footprint has size of 0
//  (in ORIGIONAL CODE) so branches into shouldn't happen, eh???? 
bool ExpandInstruction::UpdateExpansions(FunctionExpansionRecord *fer) {
    fer->AddExpansion(beginning_offset, extra_bytes);
    return true;
}

// Update location of inst points in function.  In case of ExpandInstruction, 
//  changes to inst points same as changes to branch targets....
bool ExpandInstruction::UpdateInstPoints(FunctionExpansionRecord* /* ips */) {
    return true;
}

int ExpandInstruction::getOffset() const {
    return beginning_offset;
}
 
int ExpandInstruction::getShift() const {
    return extra_bytes;
}

int ExpandInstruction::numInstrAddedAfter() {
    return 0;
}


// constructor for ExpandInstruction local alteration....
PushEIP::PushEIP(pd_Function *f, int offset):
    LocalAlteration(f, offset) 
{ 

}

// 5 byte call should be replaced with 5 byte push, so no change in branch
// targets
bool PushEIP::UpdateExpansions(FunctionExpansionRecord* /* fer */) {
    return true;
}

// Update location of inst points in function.  In case of PushEIP, 
//  changes to inst points same as changes to branch targets....
bool PushEIP::UpdateInstPoints(FunctionExpansionRecord* /* ips */) {
    return true;
}

int PushEIP::getOffset() const {
    return beginning_offset;
}
 
int PushEIP::getShift() const {
    return 0;
}

int PushEIP::numInstrAddedAfter() {
    return 0;
}


// constructor for ExpandInstruction local alteration....
PushEIPmov::PushEIPmov(pd_Function *f, int offset):
    LocalAlteration(f, offset) 
{
    extra_bytes = 0;
}

// 5 byte call should be replaced with 5 byte mov, so no overall change.
bool PushEIPmov::UpdateExpansions(FunctionExpansionRecord *fer) {
    return true;
}

bool PushEIPmov::UpdateInstPoints(FunctionExpansionRecord* ips) {
    return true;
}

int PushEIPmov::getOffset() const {
    return beginning_offset;
}
 
int PushEIPmov::getShift() const {
    return extra_bytes;
}

int PushEIPmov::numInstrAddedAfter() {
    return 0;
}









