

#include "dyninstAPI/src/LocalAlteration.h"

// constructor for LocalAlteration....
LocalAlteration::LocalAlteration(pd_Function *f, int beginning_offset, 
				       int ending_offset) {
    function = f;
    beginningOffset = beginning_offset;
    endingOffset = ending_offset;
}


LocalAlterationSet::LocalAlterationSet(pd_Function *f) {
    func = f;
    iterIdx = 0;
    ordered = false;
}

LocalAlterationSet::~LocalAlterationSet() {
    Flush();
}

void LocalAlterationSet::Flush() {
    unsigned i;
    for(i=0;i<alterations.size();i++) {
        delete alterations[i];
    }
    alterations.resize(0);
    ordered = false;
}

void LocalAlterationSet::AddAlteration(LocalAlteration *a) {
    alterations += a;
    a->UpdateExpansions(&fer);
    a->UpdateInstPoints(&ips);
    ordered = false;
}

int order_peephole_alteration_ptrs(const void *a, const void *b) {
    const LocalAlteration *p1, *p2;
    p1 = (const LocalAlteration *)a;
    p2 = (const LocalAlteration *)b;
    return (p1 - p2);
}

void LocalAlterationSet::Order() {
    if (ordered = true) return;
    alterations.sort(order_peephole_alteration_ptrs);
    ordered = true;
}

//
// Iterator Code
//
void LocalAlterationSet::iterReset() {
    Order();
    iterIdx = 0;
}

LocalAlteration *LocalAlterationSet::iterNext() {
    Order();
    if (iterIdx < alterations.size()) {
        return alterations[iterIdx++];
    }
    return NULL;
}


int LocalAlterationSet::sizeChange() {
    return fer.GetShift(func->size());
}

