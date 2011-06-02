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
#include "mutatee_util.h"
#include "pcontrol_mutatee_tools.h"
#include "communication.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

thread_t threads[MAX_POSSIBLE_THREADS];
int thread_results[MAX_POSSIBLE_THREADS];
int num_threads;
int sockfd;
extern int signal_fd;

typedef struct {
   thread_t thread_id;
   int (*func)(int, void*);
   void *data;
} datagram;

#if defined(os_bg_test)
#define USE_MEM_COMM 0
#else
#define USE_MEM_COMM 0
#endif


#define MESSAGE_BUFFER_SIZE 4096
#define MESSAGE_TIMEOUT 30
volatile char recv_buffer[MESSAGE_BUFFER_SIZE];
volatile char send_buffer[MESSAGE_BUFFER_SIZE];
volatile uint32_t recv_buffer_size;
volatile uint32_t send_buffer_size;
volatile uint32_t needs_pc_comm = USE_MEM_COMM;
volatile uint32_t timeout;

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
         assert(num_threads > 1);
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

void pingSignalFD(int sfd)
{
   char c = 'X';
   if (sfd == 0 || sfd == -1) {
      return;
   }
   write(sfd, &c, sizeof(char));
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
   pingSignalFD(signal_fd);
   getSocketInfo();
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

#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

static volatile char MutatorSocket[4096];

static char *socket_type = NULL;
static char *socket_name = NULL;

void getSocketInfo()
{
   int count = 0;


   if (needs_pc_comm)
      return;
   
   count = 0;
   while (MutatorSocket[0] == '\0') {
      usleep(10000); //.01 seconds
      count++;
      if (count >= 60 * 100) { //1 minute
         logerror("Mutatee timeout\n");
         exit(-2);
      }
   }
   socket_type = (char *) MutatorSocket;
   char *space = strchr((char *) MutatorSocket, ' ');
   socket_name = space+1;
   *space = '\0';
}

void sigalarm_handler(int sig)
{
   assert(sig == SIGALRM);
   timeout = 1;
}

void setTimeoutAlarm()
{
   static int set_alarm_handler = 0;
   if (!set_alarm_handler) {
      set_alarm_handler = 1;
      signal(SIGALRM, sigalarm_handler);
   }
   timeout = 0;
   alarm(MESSAGE_TIMEOUT);
}

void resetTimeoutAlarm()
{
   alarm(0);
}

int initMutatorConnection()
{
   int result;

   if (needs_pc_comm)
      return 0;

   if (strcmp(socket_type, "un_socket") == 0) {
      sockfd = socket(PF_UNIX, SOCK_STREAM, 0);
      if (sockfd == -1) {
         perror("Failed to create socket");
         return -1;
      }
      struct sockaddr_un server_addr;
      memset(&server_addr, 0, sizeof(struct sockaddr_un));
      server_addr.sun_family = PF_UNIX;
      strncpy(server_addr.sun_path, socket_name, 108);
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
   if (needs_pc_comm) {
      assert(msg_size < MESSAGE_BUFFER_SIZE);
      assert(!send_buffer_size);
      memcpy((void *) send_buffer, msg, msg_size);
      send_buffer_size = msg_size;
    
      setTimeoutAlarm();
      while (send_buffer_size && !timeout);
      resetTimeoutAlarm();
      if (send_buffer_size) {
         logerror("Timed out in mutatee send_message\n");
         exit(-3);
      }
      return 0;
   }

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
   if (needs_pc_comm) {
      int set_alarm = 0;
      //Sockets won't work now... Communicate by having ProccontrolAPI
      // read and write into the process
      if (!recv_buffer_size) {
         setTimeoutAlarm();
         set_alarm = 1;
      }
      while (!recv_buffer_size && !timeout);
      if (set_alarm) {
         resetTimeoutAlarm();
      }
      if (!recv_buffer_size) {
         logerror("Timed out in mutatee recv_message\n");
         exit(-4);
      }
      assert(msg_size < MESSAGE_BUFFER_SIZE);
      assert(msg_size == recv_buffer_size);
      memcpy((void *) msg, (void *) recv_buffer, msg_size);
      recv_buffer_size = 0;
      return 0;
   }

   int result = -1;
   while( result != (int) msg_size && result != 0 ) {
       fd_set read_set;
       FD_ZERO(&read_set);
       FD_SET(sockfd, &read_set);
       struct timeval s_timeout;
       s_timeout.tv_sec = 0;
       s_timeout.tv_usec = 100000; //.1 sec
       int sresult = select(sockfd+1, &read_set, NULL, NULL, &s_timeout);
       if (sresult == -1 && errno != EINTR) {
          perror("Mutatee unable to receive message during select\n");
          return -1;
       }
       if (sresult <= 0) {
          continue;
       }
       
       
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

