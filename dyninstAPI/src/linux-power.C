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

// $Id: linux-power.C,v 1.19 2008/06/19 19:53:26 legendre Exp $

#include <string>
#include <dlfcn.h>

#include "dyninstAPI/src/linux-power.h"
#include "dyninstAPI/src/addressSpace.h"
#include "dyninstAPI/src/dynProcess.h"
#include "dyninstAPI/src/frame.h"
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/mapped_object.h"
#include "dyninstAPI/src/inst-power.h"
#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/src/registerSpace.h"
#include "dyninstAPI/src/function.h"

#define DLOPEN_MODE (RTLD_NOW | RTLD_GLOBAL)

const char DL_OPEN_FUNC_EXPORTED[] = "dlopen";
const char DL_OPEN_FUNC_INTERNAL[] = "_dl_open";
const char DL_OPEN_FUNC_NAME[] = "do_dlopen";
const char DL_OPEN_LIBC_FUNC_EXPORTED[] = "__libc_dlopen_mode";

Dyninst::Address PCProcess::getLibcStartMainParam(PCThread *) {
    assert(!"This function is unimplemented");
    return 0;
}

Dyninst::Address PCProcess::getTOCoffsetInfo(Dyninst::Address dest) {
    if ( getAddressWidth() == 4 ) return 0;

    // We have an address, and want to find the module the addr is
    // contained in. Given the probabilities, we (probably) want
    // the module dyninst_rt is contained in.
    // I think this is the right func to use

    // Find out which object we're in (by addr).
    mapped_object *mobj = findObject(dest);

    // Very odd case if this is not defined.
    assert(mobj);
    Dyninst::Address TOCOffset = mobj->parse_img()->getObject()->getTOCoffset();
    
    if (!TOCOffset)
       return 0;
    return TOCOffset + mobj->dataBase();
}

Dyninst::Address PCProcess::getTOCoffsetInfo(func_instance *func) {
    if ( getAddressWidth() == 4 ) return 0;

    mapped_object *mobj = func->obj();

    return mobj->parse_img()->getObject()->getTOCoffset() + mobj->dataBase();
}

bool PCProcess::getOPDFunctionAddr(Dyninst::Address &addr) {
    bool result = true;
    if( getAddressWidth() == 8 ) {
        Dyninst::Address resultAddr = 0;
        if( !readDataSpace((const void *)addr, getAddressWidth(),
                    (void *)&resultAddr, false) ) 
        {
            result = false;
        }else{
            addr = resultAddr;
       }
    }
    return result;
}

AstNodePtr PCProcess::createUnprotectStackAST() {
    // This is not necessary on power
    return AstNode::nullNode();
}

bool Frame::setPC(Dyninst::Address newpc) {
   Dyninst::Address pcAddr = getPClocation();
   if (!pcAddr)
   {
       //fprintf(stderr, "[%s:%u] - Frame::setPC aborted", __FILE__, __LINE__);
      return false;
   }

   //fprintf(stderr, "[%s:%u] - Frame::setPC setting %x to %x",
   //__FILE__, __LINE__, pcAddr_, newpc);
   if (getProc()->getAddressWidth() == sizeof(uint64_t)) {
      uint64_t newpc64 = newpc;
      if (!getProc()->writeDataSpace((void*)pcAddr, sizeof(newpc64), &newpc64))
         return false;
      sw_frame_.setRA(newpc64);
   }
   else {
      uint32_t newpc32 = newpc;
      if (!getProc()->writeDataSpace((void*)pcAddr, sizeof(newpc32), &newpc32))
         return false;
      sw_frame_.setRA(newpc32);
   }

   return true;
}

bool AddressSpace::getDyninstRTLibName() {
//full path to libdyninstAPI_RT (used an _m32 suffix for 32-bit version)
    startup_printf("dyninstRT_name: %s\n", dyninstRT_name.c_str());
    if (dyninstRT_name.length() == 0) {
        // Get env variable
        if (getenv("DYNINSTAPI_RT_LIB") != NULL) {
            dyninstRT_name = getenv("DYNINSTAPI_RT_LIB");
        }
        else {
            std::string msg = std::string("Environment variable ") +
                std::string("DYNINSTAPI_RT_LIB") +
               std::string(" has not been defined");
            showErrorCallback(101, msg);
            return false;
        }
    }

    // Automatically choose 32-bit library if necessary.
    const char *modifier = "";
    const char *name = dyninstRT_name.c_str();

    const char *split = P_strrchr(name, '/');
    if ( !split ) split = name;
    split = P_strchr(split, '.');
    if ( !split || P_strlen(split) <= 1 ) {
        // We should probably print some error here.
        // Then, of course, the user will find out soon enough.
        startup_printf("Invalid Dyninst RT lib name: %s\n", 
                dyninstRT_name.c_str());
        return false;
    }

    if (getAddressWidth() == 4 &&
        (sizeof(void *) == 8)) {
       // Need _m32...
       if (P_strstr(name, "_m32") == NULL) {
          modifier = "_m32";
       }
    }

    const char *suffix = split;
    if( getAOut()->isStaticExec() ) {
        suffix = ".a";
    }else{
        if( P_strncmp(suffix, ".a", 2) == 0 ) {
            // This will be incorrect if the RT library's version changes
            suffix = ".so";
        }
    }

    dyninstRT_name = std::string(name, split - name) +
                     std::string(modifier) +
                     std::string(suffix);

    startup_printf("Dyninst RT Library name set to '%s'\n",
            dyninstRT_name.c_str());

    // Check to see if the library given exists.
    if (access(dyninstRT_name.c_str(), R_OK)) {
        std::string msg = std::string("Runtime library ") + dyninstRT_name
        + std::string(" does not exist or cannot be accessed!");
        showErrorCallback(101, msg);
	cerr << msg << endl;
        return false;
    }
    return true;
}

// floor of inferior malloc address range within a single branch of x
// for 32-bit ELF PowerPC mutatees
Dyninst::Address region_lo(const Dyninst::Address x) {
   const Dyninst::Address floor = getpagesize();

   assert(x >= floor);

   if ((x > floor) && (x - floor > getMaxBranch()))
      return x - getMaxBranch();

   return floor;
}


// floor of inferior malloc address range within a single branch of x
// for 64-bit ELF PowerPC mutatees
Dyninst::Address region_lo_64(const Dyninst::Address x) {
   const Dyninst::Address floor = getpagesize();

   assert(x >= floor);

   if ((x > floor) && (x - floor > getMaxBranch()))
      return x - getMaxBranch();

   return floor;
}


// ceiling of inferior malloc address range within a single branch of x
// for 32-bit ELF PowerPC mutatees
Dyninst::Address region_hi(const Dyninst::Address x) {
   const Dyninst::Address ceiling = ~(Dyninst::Address)0 & 0xffffffff;

   assert(x < ceiling);

   if ((x < ceiling) && (ceiling - x > getMaxBranch()))
      return x + getMaxBranch();

   return ceiling;
}


// ceiling of inferior malloc address range within a single branch of x
// for 64-bit ELF PowerPC mutatees
Dyninst::Address region_hi_64(const Dyninst::Address x) {
   const Dyninst::Address ceiling = ~(Dyninst::Address)0;

   assert(x < ceiling);

   if ((x < ceiling) && (ceiling - x > getMaxBranch()))
      return x + getMaxBranch();

   return ceiling;
}
