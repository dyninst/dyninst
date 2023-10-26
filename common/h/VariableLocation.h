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

#if !defined(_Variable_Location_h_)
#define _Variable_Location_h_

#include "registers/MachRegister.h"
#include "dyntypes.h"
#include "util.h"

namespace Dyninst {

/*
 * storageClass: Encodes how a variable is stored.
 *
 * storageAddr           - Absolute address of variable.
 * storageReg            - Register which holds variable value.
 * storageRegOffset      - Address of variable = $reg + address.
 */

typedef enum {
   storageUnset,
   storageAddr,
   storageReg,
   storageRegOffset
} storageClass;

COMMON_EXPORT const char *storageClass2Str(storageClass sc);

/*
 * storageRefClass: Encodes if a variable can be accessed through a register/address.
 *
 * storageRef        - There is a pointer to variable.
 * storageNoRef      - No reference. Value can be obtained using storageClass.
 */
typedef enum {
   storageRefUnset,
   storageRef,
   storageNoRef
} storageRefClass;

COMMON_EXPORT const char *storageRefClass2Str(storageRefClass sc);

//location for a variable
//Use mr_reg instead of reg for new code.  reg left in for backwards
// compatibility.

class VariableLocation  {
  public:
	storageClass stClass;
	storageRefClass refClass;
        MachRegister mr_reg;
	long frameOffset;
    long frameOffsetAbs;
	Address lowPC;
	Address hiPC;
   
VariableLocation() :
   stClass(storageUnset),
      refClass(storageRefUnset),
      frameOffset(0),
      frameOffsetAbs(0),
      lowPC(0),
      hiPC(0) {}

   bool operator==(const VariableLocation &rhs) const {
      return ((stClass == rhs.stClass) &&
              (refClass == rhs.refClass) &&
              (mr_reg == rhs.mr_reg) &&
              (frameOffset == rhs.frameOffset) &&
              (frameOffsetAbs == rhs.frameOffsetAbs) &&
              (lowPC == rhs.lowPC) &&
              (hiPC == rhs.hiPC));
   }

};

}

#endif
