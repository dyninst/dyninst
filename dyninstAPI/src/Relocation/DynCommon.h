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
#ifndef PATCHAPI_H_DYNINST_DYNCOMMON_H_
#define PATCHAPI_H_DYNINST_DYNCOMMON_H_

// PatchAPI public interface
#include "PatchCommon.h"
#include "Instrumenter.h"

// Dyninst Internal
#include "dyninstAPI/src/addressSpace.h"

#define DYN_CAST(type, obj)  boost::dynamic_pointer_cast<type>(obj)

// Shortcuts for type casting
#define SCAST_MO(o) static_cast<mapped_object*>(o)
#define SCAST_EI(e) static_cast<edge_instance*>(e)
#define SCAST_BI(b) static_cast<block_instance*>(b)
#define SCAST_PB(b) static_cast<parse_block*>(b)
#define SCAST_PF(f) static_cast<parse_func*>(f)
#define SCAST_FI(f) static_cast<func_instance*>(f)


namespace Dyninst {
namespace PatchAPI {
  class DynAddrSpace;
  class DynObject;
}
}


#endif  // PATCHAPI_H_DYNINST_DYNCOMMON_H_
