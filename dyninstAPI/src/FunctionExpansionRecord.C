

#include "FunctionExpansionRecord.h"


FERNode::FERNode(int toffset, int tshift) {
    origional_offset = toffset;
    shift = tshift;
}
 
FunctionExpansionRecord::FunctionExpansionRecord() {
    index = 0;
    collapsed = 0;
}

FunctionExpansionRecord::~FunctionExpansionRecord() {
    int i;
    for(i=0;i<expansions.size();i++) {
        delete expansions[i];
    }
    expansions.resize(0);
}

int FunctionExpansionRecord::GetShift(int origional_offset) {
    int stop;

    assert(collapsed);

    // list of expansions empty, shift always 0....
    if (total_expansions.size() <= 0) {
	return 0;
    }

    // origional_offset < lowest expansion record offset, offset 0....
    if (origional_offset < total_expansions[0].OrigionalOffset()) {
	return 0;
    }

    stop = total_expansions.size() - 1;
    // origional_offset >= highest expansion record offset, offset of
    //  highest expansion record....
    if (origional_offset >= total_expansions[stop].OrigionalOffset()) {
	return total_expansions[stop].Shift();
    } 

    // index points too far into expansions, reset it to 0...
    if (origional_offset < total_expansions[index].OrigionalOffset()) {
	stop = index;
	index = 0;
    }

    while (index < stop && origional_offset >= \
	    total_expansions[index+1].OrigionalOffset()) {
	index++;
    }
    return total_expansions[index].Shift();
}  

void FunctionExpansionRecord::AddExpansion(int origional_offset, int shift) {
    int size = expansions.size();
    collapsed = 0;
    //if (size > 0) {
    //	// assert just to make that expansions is kept sorted.  Currently,
    //	// expansion instances should only be added in increasing order.
    //	// change code here to allow arbitrary inserts here iff necessary.....
    //	assert(origional_offset > expansions[size-1].OrigionalOffset());
    //}
    expansions += new FERNode(origional_offset, shift);
}


// Dump state info about FunctionExpansionRecord object....
ostream &FunctionExpansionRecord::operator<<(ostream &os) {
    unsigned int i;

    os << "FunctionExpansionRecord " << this << " state info :" << endl;
    os << " index = " << index << endl;
    os << " expansions : size = " << expansions.size() << endl;

    // if vector has iterator class, use that instead....
    for(i=0;i<expansions.size();i++) {
        expansions[i]->operator<<(os);
    }

    for(i=0;i<total_expansions.size();i++) {
        total_expansions[i].operator<<(os);
    }
    return os;
} 

ostream &FERNode::operator<<(ostream &os) {
  os << "origional_offset : " << origional_offset << " , shift : " \
     << shift << endl;
  return os;
}

// function used to sort array of fer nodes....
int sort_fernode_by_offset(const void *a, const void *b) {
    FERNode *f1 = (FERNode*)a;
    FERNode *f2 = (FERNode*)b;
    if (f1->OrigionalOffset() > f2->OrigionalOffset()) {
        return 1;
    } else if (f1->OrigionalOffset() < f2->OrigionalOffset()) {
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
    expansions.sort(sort_fernode_by_offset);

    for(i=0;i<expansions.size();i++) {
        total += expansions[i]->Shift();
	total_expansions += FERNode(expansions[i]->OrigionalOffset(), total);
    }

    collapsed = 1;
}

