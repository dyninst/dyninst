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


#include "FunctionExpansionRecord.h"


FERNode::FERNode(int toffset, int tshift) {
    original_offset = toffset;
    shift = tshift;
}

// updated with totalShift for function relocation 
FunctionExpansionRecord::FunctionExpansionRecord() {
    index = 0;
    collapsed = 0;
    totalShift = 0;
}


// replaced old destructor for function relocation
FunctionExpansionRecord::~FunctionExpansionRecord() {
    DeleteNodes();
}


// Added for function relocation
// deletes FERNodes 
void FunctionExpansionRecord::DeleteNodes() {
    unsigned int i;
    for(i=0;i<expansions.size();i++) {
        delete expansions[i];
    }
    expansions.resize(0);
}


// Added for function relocation
// clears record of FERNodes 
void FunctionExpansionRecord::Flush() {
    unsigned int i;
    index = 0;
    collapsed = 0;
    totalShift = 0;
    total_expansions.resize(0);

    for(i=0;i<expansions.size();i++) {
        expansions[i] = 0;
    }
    expansions.resize(0);
}



// updated for function relocation
int FunctionExpansionRecord::GetShift(int original_offset) {
    int stop;

    assert(collapsed);

    // list of expansions empty, shift always 0....
    if (total_expansions.size() <= 0) {
	return 0;
    }

    // original_offset < lowest expansion record offset, offset 0....
    if (original_offset < total_expansions[0].OriginalOffset()) {
	return 0;
    }

    stop = total_expansions.size() - 1;
    // original_offset >= highest expansion record offset, offset of
    //  highest expansion record....
    if (original_offset > total_expansions[stop].OriginalOffset()) {
	return totalShift;
    } 

    // index points too far into expansions, reset it to 0...
    if (original_offset < total_expansions[index].OriginalOffset()) {
	stop = index;
	index = 0;
    }

    while (index < stop && original_offset > \
	    total_expansions[index].OriginalOffset()) {
	index++;
    }
    return total_expansions[index].Shift();
} 

// added for function relocation
int FunctionExpansionRecord::sizeChange() {
    return totalShift;
}


// updated by adding totalShift for function relocation
void FunctionExpansionRecord::AddExpansion(int original_offset, int shift) {
    collapsed = 0;
    expansions.push_back(new FERNode(original_offset, shift));
    totalShift += shift;
}


#ifdef DEBUG
// Dump state info about FunctionExpansionRecord object....
ostream &operator<<(ostream &os, const FunctionExpansionRecord &rc) {
    unsigned int i;

    os << "FunctionExpansionRecord " << &rc << " state info :" << endl;
    os << " index = " << rc.index << endl;
    os << " expansions : size = " << rc.expansions.size() << endl;

    // if vector has iterator class, use that instead....
    for(i=0;i<rc.expansions.size();i++) {
      os << rc.expansions[i];
    }

    for(i=0;i<rc.total_expansions.size();i++) {
      os << rc.total_expansions[i];
    }
    return os;
} 

ostream &operator<<(ostream &os, const FERNode &nd) {
  os << "original_offset : " << nd.original_offset << " , shift : "
     << nd.shift << endl;
  return os;
}
#endif

// function used to sort array of fer nodes....
int sort_fernode_by_offset(const void *a, const void *b) {
    FERNode *f1 = *(FERNode**)(const_cast <void*> (a));
    FERNode *f2 = *(FERNode**)(const_cast <void*> (b));
    int offset1 = f1->OriginalOffset();
    int offset2 = f2->OriginalOffset();
    if (offset1 > offset2) {
        return 1;
    } else if (offset1 < offset2) {
        return -1;
    }
    return 0;
}

void FunctionExpansionRecord::Collapse() {
    int i, total = 0;

    //total_expansions = expansions;

    //for(i=0;i<expansions.size();i++) {
    //    total += expansions[i].Shift();
    //	total_expansions[i].SetShift(total);
    //}

    total_expansions.resize(0);
    VECTOR_SORT(expansions, sort_fernode_by_offset);

    for(i=0;(unsigned)i<expansions.size();i++) {
	total_expansions.push_back(FERNode(expansions[i]->OriginalOffset(), total));
        total += expansions[i]->Shift();
    }

    collapsed = 1;
}


FunctionExpansionRecord& FunctionExpansionRecord::operator=(const FunctionExpansionRecord& f) {

    if (this != &f) {
        expansions = f.expansions;
        total_expansions = f.total_expansions;
        index = f.index;
        collapsed = f.collapsed;
        totalShift = f.totalShift;

        for (unsigned int i = 0; i < expansions.size(); i++) {
            expansions[i] = new FERNode();
            *expansions[i] = *(f.expansions[i]);
        }
    }
    return *this;
}
