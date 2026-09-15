#include "registers/loongarch64_regs.h"
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

#include <string.h>
#include <iostream>
#include "loongarch64_process.h"
#include "common/src/arch-loongarch64.h"

using namespace Dyninst::loongarch64;
using namespace std;

loongarch64_process::loongarch64_process(Dyninst::PID p, std::string e, std::vector<std::string> a,
                         std::vector<std::string> envp, std::map<int, int> f) :
   int_process(p, e, a, envp, f)
{
}

loongarch64_process::loongarch64_process(Dyninst::PID pid_, int_process *p) :
  int_process(pid_, p)
{
}

loongarch64_process::~loongarch64_process()
{
}

unsigned loongarch64_process::plat_breakpointSize()
{
  // LoongArch64 BREAK instruction is 4 bytes
  return 4;
}

void loongarch64_process::plat_breakpointBytes(unsigned char *buffer)
{
  // LoongArch64 BREAK #0 instruction: 0x002a0000
  buffer[0] = 0x00;
  buffer[1] = 0x00;
  buffer[2] = 0x2a;
  buffer[3] = 0x00;
}

bool loongarch64_process::plat_breakpointAdvancesPC() const
{
  return true;
}

loongarch64_thread::loongarch64_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l) :
   int_thread(p, t, l)
{
}

loongarch64_thread::~loongarch64_thread()
{
}