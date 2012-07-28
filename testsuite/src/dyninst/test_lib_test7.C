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

// $Id: test_lib_test7.C,v 1.1 2008/10/30 19:21:47 legendre Exp $
#include "test_lib_test7.h"
#include "test_lib.h"
#include <sys/msg.h>

bool setupMessaging(int *msgid) {
  key_t msgkey = 1234;

  if((*msgid = msgget(msgkey, 0666|IPC_CREAT)) == -1) {
    perror("Couldn't create messaging");
    return false;
  }

  return true;
}

bool doError(bool *passedTest, bool cond, const char *str) {
   if(cond == true) {
      logerror("%s", str);
      *passedTest = false;
      return true;
   }
   return false;
}

const char *procName[2] = { "parent", "child" };
/*
 * Given a string variable name and an expected value, lookup the varaible
 *    in the process, and verify that the value matches.
 *
 */
bool verifyProcMemory(BPatch_process *appProc, const char *name,
                      int expectedVal, procType proc_type)
{
    BPatch_image *appImage = appProc->getImage();

   if (!appImage) {
       dprintf("unable to locate image for %d\n", appProc->getPid());
      return false;
   }
   
   BPatch_variableExpr *var = appImage->findVariable(name);
   if (!var) {
      dprintf("unable to located variable %s in child\n", name);
      return false;
   }

   int actualVal;
   var->readValue(&actualVal);
   
   if (expectedVal != actualVal) {
      logerror("*** for %s (%s), expected val = %d, but actual was %d\n",
	      name, procName[proc_type], expectedVal, actualVal);
      return false;
   } else {
      dprintf("verified %s (%s) was = %d\n", name, procName[proc_type], 
	      actualVal);
      return true;
   }
}

/*
 * Given a string variable name and an expected value, lookup the varaible
 *    in the process, and verify that the value matches.
 *
 */
bool verifyProcMemory(const char *name, BPatch_variableExpr *var,
                      int expectedVal, procType proc_type)
{
   int actualVal;
   var->readValue(&actualVal);
   
   if (expectedVal != actualVal) {
      logerror("*** for %s (%s), expected val = %d, but actual was %d\n",
	      name, procName[proc_type], expectedVal, actualVal);
      return false;
   } else {
      dprintf("verified %s (%s) was = %d\n", name, procName[proc_type], 
	      actualVal);
      return true;
   }
}

const char *subTestNames[10] = {
    "<no test # 0>",
    "Delete snippet in parent",
    "Delete snippet in child",
    "Delete snippet in both parent and child",
    "Insert snippet in child - wo inherited snippets",
    "Add snippets to parent & child",
    "OneTimeCode in parent & child",
    "Memory allocation in parent & child",
    "Memory deallocate in child",
    "Memory deallocate in parent",
};

// TODO Fix this up so it's less stupid (remove the magic number, make it match
// better with the meaningful test names, etc.
void showFinalResults(bool passedTest, int i) {
   if(passedTest==false) {
      logerror("Failed test #%d (%s)\n", i + 4, subTestNames[i]);
   } else {
      logerror("Passed test #%d (%s)\n", i + 4, subTestNames[i]);
   }
}
