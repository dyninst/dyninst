

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


