/*
 * Copyright (c) 1996 Barton P. Miller
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

#ifndef __FUNCTION_EXPANSION_RECORD_H__
#define __FUNCTION_EXPANSION_RECORD_H__

#include "common/h/headers.h"
#include "common/h/Vector.h"
#include <iostream.h>


// small class for keeping track of information about shifts in code offsets
//  (offset from beginning of function) caused when functions are expanded
//  e.g. when entry or exit instrumentation is inserted....
class FERNode {
    //  the offset (number of instructions from beginning of the function) at
    //   which the extra instructions are inserted.  This offset is called
    //   "original" to exphasze that it is relative to the ORIGINAL function,
    //   not the REWRITTEN function.
    int original_offset;
    //  the number of instructions by which offsets in F() between
    //   original_offset and the original_offset of the next FERNode are
    //   shifted....
    int shift;
  public:
    int OriginalOffset() {return original_offset;}
    int Shift() {return shift;}
    void SetShift(int val) {shift = val;}
    FERNode(int toffset = 0, int tshift = 0);
    friend ostream &operator<<(ostream &os, const FERNode &nd);
};

ostream &operator<<(ostream &os, const FERNode &nd);

class FunctionExpansionRecord {
    // record of info on how instrumenting or otherwise rewriting a function
    //  has expanded logical blocks of it.  This information is needed to patch
    //  jump/branch targets to locations inside the function (e.g. from other
    //  locations inside the function).

    // pdvector holding FERNodes added with AddExpansion()....
    //  assumes sorted in order of original_offset....
    pdvector<FERNode*> expansions;
    // pdvector holding FERNodes representing TOTAL displacements 
    //  derived from individual displacements specified via AddExpansion()....
    pdvector<FERNode> total_expansions;
    // index into expansions which satisfied last GetShift() request.... 
    int index;

    // 0 indicatess that a new FERNode has been added while
    // total_expansions has not been updated to reflect this
    int collapsed;

    // total shift of the record
    int totalShift;

    // sort expansions pdvector (in place)....
    void SortExpansions();
  public:

    // destructor, nuke expansions and total_expansions....
    ~FunctionExpansionRecord();

    // delete the FERNodes in expansions
    void DeleteNodes();
 
    // clear FERNodes out of expansions without deleting the FERNodes
    void Flush();

    // add an additional expansion record....
    void AddExpansion(int original_offset, int shift);
    // compute total expansions for given regions from added expansion
    //  records - use BEFORE GetShift()....
    void Collapse();

    int GetShift(int original_offset);
    FunctionExpansionRecord();

    // returns the sum total of the FERNodes shifts 
    int sizeChange();

// dump state info....
    friend ostream& operator<<(ostream &os, const FunctionExpansionRecord &rc);


    FunctionExpansionRecord& FunctionExpansionRecord::operator=(const FunctionExpansionRecord& f);
};

ostream& operator<<(ostream &os, const FunctionExpansionRecord &rc);


/*  __FUNCTION_EXPANSION_RECORD_H__  */
#endif

