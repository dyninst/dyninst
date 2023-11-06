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

// A quick header file defining externally visible types from the PatchAPI
// namespace. This allows us to reduce cross-pollination of header files.


#if !defined(_RELOCATION_H_)
#define _RELOCATION_H_

#include <list>
#include <map>
#include "dyninstAPI/src/codegen.h" // codeGen structure
#include "boost/shared_ptr.hpp"

namespace Dyninst {
namespace Relocation {

class CodeMover;
class SpringboardMap;
struct SpringboardReq;
class SpringboardBuilder;

typedef boost::shared_ptr<Dyninst::Relocation::CodeMover> CodeMoverPtr;
typedef boost::shared_ptr<Dyninst::Relocation::SpringboardBuilder> SpringboardBuilderPtr;

}
}

// This avoids the requirement to include CodeMover.h
#endif
