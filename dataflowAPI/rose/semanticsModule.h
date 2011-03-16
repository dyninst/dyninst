/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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
#ifndef ROSE_SEMANTICSMODULE_H
#define ROSE_SEMANTICSMODULE_H

#include "SgAsmType.h"

//#include "rose.h"
#if defined(_MSC_VER)
#include "external/stdint-win.h"
#include "external/inttypes-win.h"
#elif defined(os_solaris)
#include <inttypes.h>
#else
#include <stdint.h>
#include <inttypes.h>
#endif

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <stdlib.h>

#include <iostream>

static inline int numBytesInAsmType(SgAsmType* ty) {
  switch (ty->variantT()) {
    case V_SgAsmTypeByte: return 1;
    case V_SgAsmTypeWord: return 2;
    case V_SgAsmTypeDoubleWord: return 4;
    case V_SgAsmTypeQuadWord: return 8;
    default: {std::cerr << "Unhandled type " << ty->class_name() << " in numBytesInAsmType" << std::endl; abort();}
  }
}

#endif // ROSE_SEMANTICSMODULE_H
