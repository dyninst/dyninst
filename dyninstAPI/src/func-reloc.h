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
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/stats.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/showerror.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/LocalAlteration.h"

#if defined(i386_unknown_solaris2_5) || defined(i386_unknown_nt4_0) || defined(i386_unknown_linux2_0)
#include "dyninstAPI/src/arch-x86.h"
#include "dyninstAPI/src/inst-x86.h"
#include "dyninstAPI/src/instPoint-x86.h"
#include "dyninstAPI/src/LocalAlteration-x86.h"
#endif

#if defined(sparc_sun_solaris2_4)
#include "dyninstAPI/src/arch-sparc.h"
#include "dyninstAPI/src/inst-sparc.h"
#include "dyninstAPI/src/instPoint-sparc.h"
#include "dyninstAPI/src/LocalAlteration-Sparc.h"
#endif

#if defined(i386_unknown_solaris2_5) || defined(i386_unknown_nt4_0) || defined(i386_unknown_linux2_0)

// Check for ExpandInstruction alterations that have already been found.
// Sparc does not use ExpandInstruction alterations
bool alreadyExpanded(int offset, int shift, 
                     LocalAlterationSet *alteration_set);
#endif

bool combineAlterationSets(LocalAlterationSet *alteration_set, 
                           LocalAlterationSet *temp_alteration_set);

LocalAlteration *fixOverlappingAlterations(LocalAlteration *alteration, 
                                           LocalAlteration *tempAlteration);

#endif





