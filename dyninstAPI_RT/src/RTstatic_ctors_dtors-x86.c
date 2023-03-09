/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#if defined(DYNINST_RT_STATIC_LIB)
#include <stdint.h>
void *DYNINSTirel_start;
void *DYNINSTirel_end;

extern void (*DYNINSTctors_begin)(void);
extern void (*DYNINSTdtors_begin)(void);
extern void (*DYNINSTctors_end)(void);
extern void (*DYNINSTdtors_end)(void);

extern void DYNINSTBaseInit(void);

typedef struct {
  long *offset;
  long info;
  long (*ptr)(void);
} rela_t;

typedef struct {
  long *offset;
  long info;
} rel_t;

/*
 * When rewritting a static binary, .ctors and .dtors sections of
 * instrumentation code needs to be combined with the existing .ctors
 * and .dtors sections of the static binary.
 *
 * The following functions process the .ctors and .dtors sections
 * that have been rewritten. The rewriter will relocate the 
 * address of DYNINSTctors_addr and DYNINSTdtors_addr to point to
 * new .ctors and .dtors sections.
 */

void DYNINSTglobal_ctors_handler(void) {
    void (**ctor)(void) = &DYNINSTctors_begin;

    while( ctor != ( &DYNINSTctors_end )) {
	if(*ctor && ((intptr_t)*ctor != -1))
	    (*ctor)();
        ctor++;
    }

    // This ensures that instrumentation cannot execute until all global
    // constructors have run
    DYNINSTBaseInit();
}

void DYNINSTglobal_dtors_handler(void) {
    void (**dtor)(void) = &DYNINSTdtors_begin;

    // Destructors are called in the forward order that they are listed
    while( dtor != (&DYNINSTdtors_end )) {
	if(*dtor && ((intptr_t)*dtor != -1))
	    (*dtor)();
	dtor++;
    }
}

void DYNINSTglobal_irel_handler(void) {
  if (sizeof(long) == 8) {
    rela_t *rel = 0;
    for (rel = (rela_t *)(&DYNINSTirel_start); rel != (rela_t *)(&DYNINSTirel_end); ++rel) {
      long result = 0;
      if (rel->info != 0x25) continue;
      result = (rel->ptr());
      *(rel->offset) = result;
    }
  }
  else {
    rel_t *rel = 0;
    for (rel = (rel_t *)(&DYNINSTirel_start); rel != (rel_t *)(&DYNINSTirel_end); ++rel) {
      typedef long (*funcptr)(void);
      long (*ptr)(void) = 0;
      long result = 0;
      if (rel->info != 0x2a) continue;
      ptr = (funcptr)(*rel->offset);
      result = ptr();
      *(rel->offset) = result;
    }
  }
}

#endif
