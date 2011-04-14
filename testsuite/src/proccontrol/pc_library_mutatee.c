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
#include <stdlib.h>
#include <stdio.h>
#include "pcontrol_mutatee_tools.h"


#if !defined(os_windows_test)
#include <dlfcn.h>

#if defined(arch_x86_64_test)
#define LIBTESTA (sizeof(void*) == 8) ? "./libtestA.so" : "./libtestA_m32.so"
#define LIBTESTB (sizeof(void*) == 8) ? "./libtestB.so" : "./libtestB_m32.so"
#else
#define LIBTESTA "./libtestA.so"
#define LIBTESTB "./libtestB.so"
#endif

void *openLib(const char *lib)
{
   void *handle;
   handle = dlopen(lib, RTLD_LAZY);
   return handle;
}

void closeLib(const char *lib, void *handle)
{
   int result;
   result = dlclose(handle);
   lib = NULL;
}

#else
#error Implement windows
#endif

static int threadFunc(int myid, void *data)
{
   myid = 0;
   data = NULL;
   return 0;
}

//Basic test for create/attach and exit.
int pc_library_mutatee()
{
   int result;
   void *handlea, *handleb;
   syncloc msg;

   result = initProcControlTest(threadFunc, NULL);
   if (result != 0) {
      output->log(STDERR, "Initialization failed\n");
      return -1;
   }

   handlea = openLib(LIBTESTA);
   handleb = openLib(LIBTESTB);
   closeLib(LIBTESTB, handleb);
   closeLib(LIBTESTA, handlea);

   msg.code = SYNCLOC_CODE;
   result = send_message((unsigned char *) &msg, sizeof(syncloc));
   if (result == -1) {
      output->log(STDERR, "Failed to send sync message\n");
      return -1;
   }

   result = recv_message((unsigned char *) &msg, sizeof(syncloc));
   if (result == -1) {
      output->log(STDERR, "Failed to recv sync message\n");
      return -1;
   }
   if (msg.code != SYNCLOC_CODE) {
      output->log(STDERR, "Recieved unexpected sync message\n");
      return -1;
   }

   result = finiProcControlTest(0);
   if (result != 0) {
      output->log(STDERR, "Finalization failed\n");
      return -1;
   }

   test_passes(testname);
   return 0;
}
