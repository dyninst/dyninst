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

#ifndef LINUX_SWK_H
#define LINUX_SWK_H

#include "common/h/dyntypes.h"
#include "common/h/SymReader.h"

#include "common/src/Types.h"
#include "common/src/linuxKludges.h"

#define START_THREAD_FUNC_NAME "start_thread"
#define CLONE_FUNC_NAME "__clone"
#define START_FUNC_NAME "_start"

namespace Dyninst {

class Elf_X;

namespace Stackwalker {

struct vsys_info {
  void *vsys_mem;
  Dyninst::Address start;
  Dyninst::Address end;
  Dyninst::SymReader *syms;
  std::string name;
  vsys_info() : vsys_mem(NULL), start(0), end(0), syms(NULL) {}
  ~vsys_info() {
    if (vsys_mem) free(vsys_mem);
  }
};

vsys_info *getVsysInfo(ProcessState *ps);
}
}

#endif
