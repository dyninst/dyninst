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
void (*DYNINSTctors_addr)(void);
void (*DYNINSTdtors_addr)(void);

#    if defined(MUTATEE64)
static const unsigned long long CTOR_LIST_TERM  = 0x0000000000000000ULL;
static const unsigned long long CTOR_LIST_START = 0xffffffffffffffffULL;
static const unsigned long long DTOR_LIST_TERM  = 0x0000000000000000ULL;
static const unsigned long long DTOR_LIST_START = 0xffffffffffffffffULL;
#    else
static const unsigned CTOR_LIST_TERM  = 0x00000000;
static const unsigned CTOR_LIST_START = 0xffffffff;
static const unsigned DTOR_LIST_TERM  = 0x00000000;
static const unsigned DTOR_LIST_START = 0xffffffff;
#    endif

extern void
DYNINSTBaseInit();

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

void
DYNINSTglobal_ctors_handler()
{
    void (**ctors_array)(void) = &DYNINSTctors_addr;

    // Find end of function pointer list
    void (**tmp_ptr)(void) = ctors_array;
    unsigned size          = 0;
    while(*tmp_ptr != ((void (*)(void)) CTOR_LIST_TERM))
    {
        size++;
        tmp_ptr++;
    }

    // Constructors are called in the reverse order that they are listed
    tmp_ptr = &ctors_array[size - 1];  // skip list end
    while(*tmp_ptr != ((void (*)(void)) CTOR_LIST_START))
    {
        (*tmp_ptr)();
        tmp_ptr--;
    }

    // This ensures that instrumentation cannot execute until all global
    // constructors have run
    DYNINSTBaseInit();
}

void
DYNINSTglobal_dtors_handler()
{
    void (**dtors_array)(void) = &DYNINSTdtors_addr;

    // Destructors are called in the forward order that they are listed
    void (**tmp_ptr)(void) = &dtors_array[1];  // skip list start
    while(*tmp_ptr != ((void (*)(void)) DTOR_LIST_TERM))
    {
        (*tmp_ptr)();
        tmp_ptr++;
    }
}

#endif
