/*
 * Copyright (c) 1996-2009 Barton P. Miller
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
#include "mutatee_util.h"
#include "pcontrol_mutatee_tools.h"
#include "communication.h"
#include <stdlib.h>
#include <string.h>

#include <sys/syscall.h>

thread_t threads[MAX_POSSIBLE_THREADS];
int thread_results[MAX_POSSIBLE_THREADS];
int num_threads;
int sockfd;

typedef struct {
   thread_t thread_id;
   int (*func)(int, void*);
   void *data;
} datagram;

static testlock_t thread_startup_lock;

void *ThreadTrampoline(void *d)
{
   int (*func)(int, void*);
   datagram *datag;
   thread_t thread_id;
   void *data;
   int func_result;
      
   datag = (datagram *) d;
   thread_id = datag->thread_id;
   func = datag->func;
   data = datag->data;
   free(datag);

   testLock(&thread_startup_lock);
   testUnlock(&thread_startup_lock);

   func_result = func((unsigned long)thread_id, data);
   
   return (void *) (long) func_result;
}

int MultiThreadFinish() {
   int i=0;
   void *result;
   for (i = 0; i < num_threads; i++) 
   {
      result = joinThread(threads[i]);
      thread_results[i] = (int) ((long) result);
   }
   return 0;
}

int MultiThreadInit(int (*init_func)(int, void*), void *thread_data)
{
   int i, j;
   int is_mt = 0;
   num_threads = 0;
   for (i = 0; i < gargc; i++) {
      if ((strcmp(gargv[i], "-mt") == 0) && init_func) {
         is_mt = 1;
         num_threads = atoi(gargv[i+1]);
         break;
      }
   }

   if (is_mt) {
      initLock(&thread_startup_lock);
      testLock(&thread_startup_lock);
      for (j = 0; j < num_threads; j++) {
         datagram *data = (datagram *) malloc(sizeof(datagram));
         data->thread_id = (thread_t)j;
         data->func = init_func;
         data->data = thread_data;
         threads[j] = spawnNewThread((void *) ThreadTrampoline, (void *) data);
      }
   }
   return 0;
}

int handshakeWithServer()
{
   int result;
   send_pid spid;
   spid.code = SEND_PID_CODE;
   spid.pid = getpid();

   result = send_message((unsigned char *) &spid, sizeof(send_pid));
   if (result == -1) {
      fprintf(stderr, "Could not send PID\n");
      return -1;
   }

   handshake shake;
   result = recv_message((unsigned char *) &shake, sizeof(handshake));
   if (result != 0) {
      fprintf(stderr, "Error recieving message\n");
      return -1;
   }
   if (shake.code != HANDSHAKE_CODE) {
      fprintf(stderr, "Recieved unexpected message\n");
      return -1;
   }

   return 0;
}

int releaseThreads()
{
   if (num_threads == 0) {
      return 0;
   }

   testUnlock(&thread_startup_lock);
   return 0;
}

int initProcControlTest(int (*init_func)(int, void*), void *thread_data)
{
   int result = 0;

   if (init_func) {
      result = MultiThreadInit(init_func, thread_data);
   }
   if (result != 0) {
      fprintf(stderr, "Error initializing threads\n");
      return -1;
   }

   result = initMutatorConnection();
   if (result != 0) {
      fprintf(stderr, "Error initializing connection to mutator\n");
      return -1;
   }
   
   result = handshakeWithServer();
   if (result != 0) {
      fprintf(stderr, "Could not handshake with server\n");
      return -1;
   }

   result = releaseThreads();
   if (result != 0) {
      fprintf(stderr, "Could not release threads\n");
      return -1;
   }

   return 0;
}

int finiProcControlTest(int expected_ret_code)
{
   int i, result;
   int has_error = 0;
   if (num_threads == 0)
      return 0;

   result = MultiThreadFinish();
   if (result != 0) {
      fprintf(stderr, "Thread return values were not collected\n");
      return -1;
   }
   for (i = 0; i < num_threads; i++) {
      if (thread_results[i] != expected_ret_code) {
         fprintf(stderr, "Thread returned unexpected return code %d\n", thread_results[i]);
         has_error = 1;
      }
   }
   return has_error ? -1 : 0;
}

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

int initMutatorConnection()
{
   int result;
   char *un_socket = NULL;
   int i;

   for (i = 0; i < gargc; i++) {
      if (strcmp(gargv[i], "-un_socket") == 0) {
         un_socket = gargv[i+1];
         break;
      }
   }
   
   if (un_socket) {
      sockfd = socket(PF_UNIX, SOCK_STREAM, 0);
      if (sockfd == -1) {
         perror("Failed to create socket");
         return -1;
      }
      
      struct sockaddr_un server_addr;
      memset(&server_addr, 0, sizeof(struct sockaddr_un));
      server_addr.sun_family = PF_UNIX;
      strncpy(server_addr.sun_path, un_socket, sizeof(server_addr.sun_path));

      result = connect(sockfd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr_un));
      if (result != 0) {
         perror("Failed to connect to server");
         return -1;
      }
   }

   return 0;
}

int send_message(unsigned char *msg, size_t msg_size)
{
   int result;
   result = send(sockfd, msg, msg_size, 0);
   if (result == -1) {
      perror("Mutatee unable to send message");
      return -1;
   }
   return 0;
}

int recv_message(unsigned char *msg, size_t msg_size)
{
   int result = -1;
   while( result != msg_size && result != 0 ) {
       result = recv(sockfd, msg, msg_size, MSG_WAITALL);

/* Sometimes result will be 29 for some unknown reason
 * The diagnosis is a work in progress
 */
#if defined(os_freebsd_test)
       if( result != msg_size ) {
           long lwp_id;
           syscall(SYS_thr_self, &lwp_id);
           fprintf(stderr, "[%d:%d] Received message of unexpected size %d (expected %d)\n",
                   getpid(), lwp_id, result, msg_size);
       }
#endif

       if (result == -1) {
          perror("Mutatee unable to recieve message");
          return -1;
       }
   }
   return 0;
}

