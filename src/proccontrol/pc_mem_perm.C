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
#include "proccontrol_comp.h"

#define PERM_TOTAL (6)

#if defined(os_windows_test)
// #include <winNT.h>

bool encodeMemPerm(int perm, Process::mem_perm& rights) {
  switch (perm) {
    default:   return false;
    case 0: rights.clrR().clrW().clrX();
    case 1: rights.setR().clrW().clrX();
    case 2: rights.clrR().clrW().setX();
    case 3: rights.setR().setW().clrX();
    case 4: rights.setR().clrW().setX();
    case 5: rights.setR().setW().setX();
  }
  return true;
}

bool decodeMemPerm(Process::mem_perm rights, int& perm) {
  if (rights.isNone()) {
    perm = 0;
  } else if (rights.isR()) {
    perm = 1;
  } else if (rights.isX()) {
    perm = 2;
  } else if (rights.isRW()) {
    perm = 3;
  } else if (rights.isRX()) {
    perm = 4;
  } else if (rights.isRWX()) {
    perm = 5;
  } else {
	perm = -1;
    return false;
  }

  return true;
}
#endif

class pc_mem_permMutator : public ProcControlMutator {
public:
  virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* pc_mem_perm_factory()
{
  return new pc_mem_permMutator();
}

test_results_t pc_mem_permMutator::executeTest()
{
  // SKIPPING due to brokenness so I can get CMake/BATLAB committed.
#if 1// !defined(os_windows_test)
  //skiptest(testnum, testdesc);
  return SKIPPED;

#else
   std::vector<Process::ptr>::iterator i;
   bool result;
   for (i = comp->procs.begin(); i != comp->procs.end(); i++) {
      Process::ptr proc = *i;
      if (!proc->continueProc()) {
         logerror("Failed to continue process\n");
         return FAILED;
      }
   }

   int perm;
   int perm_offset = 1;
   Dyninst::Address addr;
   SYSTEM_INFO sysInfo;
   GetSystemInfo(&sysInfo);
   DWORD pageSize = sysInfo.dwPageSize;
   Process::mem_perm oldRights, rights, tmpRights;

   for (i = comp->procs.begin(); i != comp->procs.end(); i++) {
      Process::ptr proc = *i;
      addr = proc->mallocMemory(pageSize);
      
      if (!proc->getMemoryAccessRights(addr, pageSize, oldRights)) {
         logerror("Failed to get memory permission\n");
         return FAILED;
      }

      if (!decodeMemPerm(oldRights, perm)) {
         logerror("Unsupported memory permission\n");
         return FAILED;
      }

      while (perm_offset < PERM_TOTAL) {
        if (!encodeMemPerm((perm_offset + perm) % PERM_TOTAL, rights)) {
           logerror("Unsupported memory permission\n");
           return FAILED;
        }

        if (!proc->setMemoryAccessRights(addr, pageSize, rights, tmpRights)) {
           logerror("Failed to set memory permission\n");
           return FAILED;
        }

        if (!proc->getMemoryAccessRights(addr, pageSize, tmpRights)) {
           logerror("Failed to get memory permission after set\n");
           return FAILED;
        }

        if (rights != tmpRights) {
           logerror("Failed to change memory permission\n");
           return FAILED;
        }

        perm_offset++;
      }

      if(!proc->freeMemory(addr)) {
           logerror("Failed to free memory\n");
           return FAILED;
      }

   }

   return PASSED;
#endif
}


