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

#include "../src/mutatee_util.h"
#include "pcontrol_mutatee_tools.h"
#include "communication.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#if defined(os_windows_test)
#include <winsock2.h>
#include <windows.h>
#if !defined(MSG_WAITALL)
#define MSG_WAITALL 8
#endif
#endif

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
#if defined(os_windows_test)
   func_result = func(thread_id.threadid, data);
#else
   func_result = func((unsigned long)thread_id, data);
#endif
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

   //fprintf(stderr, "starting MultiThreadInit\n");

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
#if defined(os_windows_test)
		 data->thread_id.threadid = j;
		data->thread_id.hndl = INVALID_HANDLE;
#else
		 data->thread_id = (thread_t)j;
#endif
		 data->func = init_func;
         data->data = thread_data;
         threads[j] = spawnNewThread((void *) ThreadTrampoline, (void *) data);
      }
   }
   return 0;
}

uint64_t getFunctionPtr(unsigned long *ptr) {
    unsigned long tmpAddr;
#if defined(arch_power_test) && defined(arch_64bit_test)
    /* need to dereference function pointers before sending them to mutator */
    tmpAddr = *ptr;
#else
    tmpAddr = (unsigned long)ptr;
#endif
    return (uint64_t)tmpAddr;
}

uint64_t getTOCValue(unsigned long *ptr) {
    unsigned long tmpAddr;
#if defined(arch_power_test) && defined(arch_64bit_test)
    /* need to get the TOC value from the opd */
    tmpAddr = ptr[1];
#else
    tmpAddr = (unsigned long)ptr;
#endif
    return (uint64_t)tmpAddr;
}

int handshakeWithServer()
{
   int result;
   send_pid spid;
   handshake shake;

  // fprintf(stderr, "starting handshakeWithServer\n");

   spid.code = SEND_PID_CODE;
#if !defined(os_windows_test)
   spid.pid = getpid();
#else
   spid.pid = GetCurrentProcessId();
#endif

   result = send_message((unsigned char *) &spid, sizeof(send_pid));
   if (result == -1) {
      fprintf(stderr, "Could not send PID\n");
      return -1;
   }

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
   WORD wsVer = MAKEWORD(2,0);
   int result = 0;
   WSADATA ignored;
   //fprintf(stderr, "starting initProcControlTest\n");
   result = WSAStartup(wsVer, &ignored);
   if(result)
   {
	   fprintf(stderr, "error in WSAStartup: %d\n", result);
	   return -1;
   }

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

#if defined(os_windows_test)
   WSACleanup();
#endif
   return has_error ? -1 : 0;
}

#if !defined(os_windows_test)
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

       if (result == -1 && errno != EINTR ) {
          perror("Mutatee unable to recieve message");
          return -1;
       }

#if defined(os_freebsd_test)
       /* Sometimes the recv system call is not restarted properly after a
        * signal and an iRPC. TODO a workaround for this bug
        */
       if( result > 0 && result != msg_size ) {
           logerror("Received message of unexpected size %d (expected %d)\n",
                   result, msg_size);
       }
#endif
   }
   return 0;
}
#else // windows


int initMutatorConnection()
{
   int result;
   char *un_socket = NULL;
   int i, pid;
   struct sockaddr_in server_addr;

	//fprintf(stderr, "begin initMutatorConnection()\n");

   for (i = 0; i < gargc; i++) {
      if (strcmp(gargv[i], "-un_socket") == 0) {
         un_socket = gargv[i+1];
		 //fprintf(stderr, "found un_socket: %s\n", un_socket);
         break;
      }
   }
   if(!un_socket)
   {
	   fprintf(stderr, "no -un_socket argument, bailing\n");
	   return 0;
   }
   sscanf(un_socket, "/tmp/pct%d", &pid);
   
   if (un_socket) {
      sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
      if (sockfd == INVALID_SOCKET) {
		  fprintf(stderr, "socket() failed: %d\n", WSAGetLastError());
         perror("Failed to create socket");
         return -1;
      }
      
      memset(&server_addr, 0, sizeof(struct sockaddr_in));
      server_addr.sin_family = AF_INET;
	  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	  server_addr.sin_port = (USHORT)(pid);

      result = connect(sockfd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr_in));
      if (result != 0) {
		  fprintf(stderr, "connect() to 127.0.0.1:%d failed: %d\n", server_addr.sin_port, WSAGetLastError());
         perror("Failed to connect to server");
//		 assert(!"connect failed");
         return -1;
      }
   }

   return 0;
}

int send_message(unsigned char *msg, size_t msg_size)
{
   int result;
   fprintf(stderr, "sending %d bytes...\n", msg_size);
   result = send(sockfd, (char*)msg, msg_size, 0);
   if (result == -1) {
	   fprintf(stderr, "Mutatee unable to send message\n");
      return -1;
   }
   return 0;
}

int recv_message(unsigned char *msg, size_t msg_size)
{
   int result = -1;
   int bytes_remaining = msg_size;
   while( (bytes_remaining > 0) && (result != 0) ) {
	   fprintf(stderr, "mutatee waiting for %d bytes\n", bytes_remaining);
       result = recv(sockfd, (char*)msg, bytes_remaining, 0);

       if (result == -1) {
		   fprintf(stderr, "recv() failed: %d\n", WSAGetLastError());
          fprintf(stderr, "Mutatee unable to recieve message\n");
          return -1;
       }
	   bytes_remaining -= result;
   }
   return 0;
}


#endif
