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

#if defined(os_linux_test)
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <link.h>
#endif


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "pcontrol_mutatee_tools.h"
#include "solo_mutatee_boilerplate.h"


typedef int (*check_tls_write_t)(unsigned char);
typedef void (*update_tls_reads_t)();

static check_tls_write_t lib_check_tls_write;
static update_tls_reads_t lib_update_tls_reads;
static int myerror = 0;

static testlock_t init_lock;

#if !defined(os_windows_test)
#include <dlfcn.h>

#if defined(arch_x86_64_test)
#define LIBTESTA (sizeof(void*) == 8) ? "./libtestA.so" : "./libtestA_m32.so"
#else
#define LIBTESTA "./libtestA.so"
#endif

//NUM_ITERATIONS needs to match pc_tls
#define NUM_ITERATIONS 8

#if defined(os_linux_test)
static unsigned int lib_count = 0;
int cb(struct dl_phdr_info *info, size_t size, void *v)
{
   (void) info; (void) size; (void) v;
   lib_count++;
   return 0;
}

int am_i_staticlink()
{
   if (lib_count == 0)
      dl_iterate_phdr(cb, NULL);

   return (lib_count == 1);
}
#else
#error Test currently only builds on Linux
#endif

void *openLib(const char *lib)
{
   void *handle;
   handle = dlopen(lib, RTLD_LAZY);
   return handle;
}

void closeLib(const char *lib, void *handle)
{
   dlclose(handle);
}

int setLibFunctions(void *handle)
{
   if (am_i_staticlink())
      return 0;

   lib_check_tls_write = (check_tls_write_t) dlsym(handle, "lib_check_tls_write");
   lib_update_tls_reads = (update_tls_reads_t) dlsym(handle, "lib_update_tls_reads");
   if (!lib_check_tls_write || !lib_update_tls_reads) {
      output->log(STDERR, "Could not find symbols in library\n");
      return -1;
   }
   return 0;
}

#define TLS_SPEC __thread

#else
#error Test does not build on windows
#endif

volatile int dont_optimize = 0;
void breakpoint_func()
{
   dont_optimize++;
}

TLS_SPEC int tls_read_int = 0;
TLS_SPEC unsigned char tls_write_char = 0x40;
TLS_SPEC signed long tls_read_long = 0;

static int exe_check_tls_write(unsigned char expected)
{
   return (tls_write_char == expected);
}

static void exe_update_tls_reads()
{
   tls_read_int++;
   tls_read_long--;
}

static int threadFunc(int myid, void *data)
{
   int i;

   testLock(&init_lock);
   testUnlock(&init_lock);
   for (i = 0; i < NUM_ITERATIONS; i++) {
      if (!exe_check_tls_write(0x40 + i)) {
         output->log(STDERR, "Executable TLS was not the expected value. expected %d was %d\n",
                     (int) (0x40 + i), (int) tls_write_char);
         myerror = 1;
      }
      if (!am_i_staticlink() && !lib_check_tls_write(0x40 + i)) {
         output->log(STDERR, "Library TLS was not the expected value\n");
         myerror = 1;
      }
      if (i != 0) {
         exe_update_tls_reads();
         if (!am_i_staticlink()) {
            lib_update_tls_reads();
         }
      }
      breakpoint_func();
   }
   return 0;
}

//Basic test for create/attach and exit.
int pc_tls_mutatee()
{
   void *handlea;
   send_addr bp_addr_msg;
   syncloc syncloc_msg;
   int result;

   initLock(&init_lock);
   testLock(&init_lock);

   myerror = 0;
   if (!am_i_staticlink()) {
      handlea = openLib(LIBTESTA);
      setLibFunctions(handlea);
   }

   result = initProcControlTest(threadFunc, NULL);
   if (result != 0) {
      output->log(STDERR, "Initialization failed\n");
      return -1;
   }

   bp_addr_msg.code = (uint32_t) SENDADDR_CODE;
   bp_addr_msg.addr = getFunctionPtr((intptr_t *) breakpoint_func);
   result = send_message((unsigned char *) &bp_addr_msg, sizeof(send_addr));
   if (result != 0) {
      output->log(STDERR, "Failed to send breakpoint addresses\n");
      testUnlock(&init_lock);
      return -1;
   }
   
   result = recv_message((unsigned char *) &syncloc_msg, sizeof(syncloc));
   if (result != 0) {
      output->log(STDERR, "Failed to recieve sync message\n");
      testUnlock(&init_lock);
      return -1;
   }
   if (syncloc_msg.code != SYNCLOC_CODE) {
      output->log(STDERR, "Recieved unexpected non-sync message\n");
      testUnlock(&init_lock);
      return -1;
   }

   testUnlock(&init_lock);   
   threadFunc(0, NULL);

   result = finiProcControlTest(0);
   if (result != 0) {
      output->log(STDERR, "Finalization failed\n");
      return -1;
   }

   if (!am_i_staticlink())
      closeLib(LIBTESTA, handlea);

   if (myerror == 0) {
      test_passes(testname);
      return 0;
   }
   return -1;
}
