#ifndef _FUNC_RELOC_H_
#define _FUNC_RELOC_H_

#include "common/h/headers.h"

#ifndef BPATCH_LIBRARY
#include "rtinst/h/rtinst.h"
#endif

#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/arch-x86.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/stats.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/showerror.h"
#include "dyninstAPI/src/inst-x86.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/LocalAlteration-x86.h"
#include "dyninstAPI/src/instPoint-x86.h"

bool alreadyExpanded(int offset, int shift, 
                     LocalAlterationSet *alteration_set);

void combineAlterationSets(LocalAlterationSet *alteration_set, 
                           LocalAlterationSet *temp_alteration_set);

void shiftAlterationSet(int type, LocalAlterationSet *alteration_set, 
                        LocalAlterationSet *shifted_alteration_set);

LocalAlteration *fixOverlappingAlterations(LocalAlteration *alteration, 
                                           LocalAlteration *tempAlteration);

#endif





