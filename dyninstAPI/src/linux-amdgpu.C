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

#include <string>

#include "common/src/linuxHeaders.h"
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/dynProcess.h"
#include "dyninstAPI/src/linux-amdgpu.h"

Dyninst::Address PCProcess::getLibcStartMainParam(PCThread *) {
  assert(false && "Not implemented for AMDGPU");
  return 0;
}

Dyninst::Address PCProcess::getTOCoffsetInfo(Dyninst::Address /* dest */) {
  assert(false && "Not implemented for AMDGPU");
  return 0;
}

Dyninst::Address PCProcess::getTOCoffsetInfo(func_instance * /* func */) {
  assert(false && "Not implemented for AMDGPU");
  return 0;
}

bool PCProcess::getOPDFunctionAddr(Dyninst::Address &) {
  assert(false && "Not implemented for AMDGPU");
  return false;
}

AstNodePtr PCProcess::createUnprotectStackAST() {
  assert(false && "Not implemented for AMDGPU");
  return AstNode::nullNode();
}

bool Frame::setPC(Dyninst::Address /* newpc */) {
  assert(false && "Not implemented for AMDGPU");
  return false;
}

bool AddressSpace::getDyninstRTLibName() {
  // full path to libdyninstAPI_RT (used an _m32 suffix for 32-bit version)
  startup_printf("dyninstRT_name: %s\n", dyninstRT_name.c_str());
  if (dyninstRT_name.length() == 0) {
    // Get env variable
    if (getenv("DYNINSTAPI_RT_LIB") != NULL) {
      dyninstRT_name = getenv("DYNINSTAPI_RT_LIB");
    } else {
      std::string msg = std::string("Environment variable ") + std::string("DYNINSTAPI_RT_LIB") +
                        std::string(" has not been defined");
      showErrorCallback(101, msg);
      return false;
    }
  }

  // Automatically choose 32-bit library if necessary.
  const char *modifier = "";
  const char *name = dyninstRT_name.c_str();

  const char *split = P_strrchr(name, '/');
  if (!split)
    split = name;
  split = P_strchr(split, '.');
  if (!split || P_strlen(split) <= 1) {
    // We should probably print some error here.
    // Then, of course, the user will find out soon enough.
    startup_printf("Invalid Dyninst RT lib name: %s\n", dyninstRT_name.c_str());
    return false;
  }

  if (getAddressWidth() == 4 && (sizeof(void *) == 8)) {
    // Need _m32...
    if (P_strstr(name, "_m32") == NULL) {
      modifier = "_m32";
    }
  }

  const char *suffix = split;
  if (getAOut()->isStaticExec()) {
    suffix = ".a";
  } else {
    if (P_strncmp(suffix, ".a", 2) == 0) {
      // This will be incorrect if the RT library's version changes
      suffix = ".so";
    }
  }

  dyninstRT_name = std::string(name, split - name) + std::string(modifier) + std::string(suffix);

  startup_printf("Dyninst RT Library name set to '%s'\n", dyninstRT_name.c_str());

  // Check to see if the library given exists.
  if (access(dyninstRT_name.c_str(), R_OK)) {
    std::string msg = std::string("Runtime library ") + dyninstRT_name +
                      std::string(" does not exist or cannot be accessed!");
    showErrorCallback(101, msg);
    cerr << msg << endl;
    return false;
  }
  return true;
}

Dyninst::Address region_lo(const Dyninst::Address /* x */) {
  assert(false && "Not implemented for AMDGPU");
  return 0;
}

Dyninst::Address region_lo_64(const Dyninst::Address /* x */) {
  assert(false && "Not implemented for AMDGPU");
  return 0;
}

Dyninst::Address region_hi(const Dyninst::Address /* x */) {
  assert(false && "Not implemented for AMDGPU");
  return 0;
}

Dyninst::Address region_hi_64(const Dyninst::Address /* x */) {
  assert(false && "Not implemented for AMDGPU");
  return 0;
}
