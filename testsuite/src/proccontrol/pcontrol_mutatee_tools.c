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

#include "../src/mutatee_util.h"
#include "pcontrol_mutatee_tools.h"
#include "communication.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <assert.h>

#if defined(os_windows_test)
    #include <winsock2.h>
    #include <windows.h>
    #if !defined(MSG_WAITALL)
        #define MSG_WAITALL 8
    #endif

#else
    #include <sys/select.h>
    #include <sys/time.h>
    #include <sys/types.h>
    #include <unistd.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#if defined(os_bgq_test)
#include <mpi.h>
#endif

#if !defined(os_windows_test)
#include <poll.h>

static unsigned int gettid(){
    return (unsigned int)pthread_self();
}
#endif

thread_t threads[MAX_POSSIBLE_THREADS];
int thread_results[MAX_POSSIBLE_THREADS];
int num_threads;
int sockfd;
int r_pipe;
int w_pipe;

extern int signal_fd;

typedef struct {
   thread_t thread_id;
   int (*func)(int, void*);
   void *data;
} datagram;

#define MESSAGE_BUFFER_SIZE 4096
#define MESSAGE_TIMEOUT 30
volatile char recv_buffer[MESSAGE_BUFFER_SIZE];
volatile char send_buffer[MESSAGE_BUFFER_SIZE];
volatile uint32_t recv_buffer_size;
volatile uint32_t send_buffer_size;
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


   for (i = 0; i < gargc; i++) {
      if ((strcmp(gargv[i], "-mt") == 0) && init_func) {
         is_mt = 1;
         num_threads = atoi(gargv[i+1]);
         break;
      }
   }

   //fprintf(stdout, "[Mutatee-MultiThreadInit] before lock.(%u/%d)\n", gettid(),getpid());
   if (is_mt && num_threads) {
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
         //fprintf(stdout, "[Mutatee-MultiThreadInit] before spawn thread[%d].(%u/%d)\n", j, gettid(),getpid());
         threads[j] = spawnNewThread((void *) ThreadTrampoline, (void *) data);
      }
   }
   return 0;
}


intptr_t getFunctionPtr(intptr_t *ptr) {
    unsigned long tmpAddr;
#if defined(arch_power_test) && defined(arch_64bit_test) && !defined(arch_ppc_little_endian_test)
    /* need to dereference function pointers before sending them to mutator */
    tmpAddr = *ptr;
#elif defined (os_windows_test)
	/* Windows function pointers may be IAT entries, which are jumps to the actual
	   function */
	unsigned char *opcode = (unsigned char *)ptr;
	/* 4-byte offset branch is 0xe9 <offset>
	   Said offset is from the end of the instruction, or start of the instruction + 5. */
	if (*opcode == 0xe9) {
		int *offsetPtr;
		/* Jump byte */
		opcode++;
		offsetPtr = (int *)opcode;
		tmpAddr = (intptr_t) (((unsigned long) ptr) + 5 + *offsetPtr);
	}
	else {
		tmpAddr = (intptr_t)ptr;
	}
#else
    tmpAddr = (intptr_t)ptr;
#endif
    return (intptr_t)tmpAddr;
}

uint64_t getTOCValue(unsigned long *ptr) {
    unsigned long tmpAddr;
#if defined(arch_power_test) && defined(arch_64bit_test) && !defined(arch_ppc_little_endian_test)
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


   spid.code = SEND_PID_CODE;
#if defined(os_windows_test)
   spid.pid = GetCurrentProcessId();
#elif defined(os_bgq_test)
   MPI_Comm_rank(MPI_COMM_WORLD, &result);
   spid.pid = result;
#else
   spid.pid = getpid();
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
      fprintf(stderr, "Recieved unexpected message.  %lx (%p) is no %lx\n", (unsigned long) shake.code, &shake, (unsigned long) HANDSHAKE_CODE);
      return -1;
   }

   return 0;
}

void pingSignalFD(int sfd)
{
#if !defined(os_windows_test)
	char c = 'X';
   if (sfd == 0 || sfd == -1) {
      return;
   }
   write(sfd, &c, sizeof(char));
#else
	return;
#endif
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
#if defined(os_windows_test)
	WORD wsVer = MAKEWORD(2,0);
   WSADATA ignored;
   result = WSAStartup(wsVer, &ignored);
   if(result)
   {
	   fprintf(stderr, "error in WSAStartup: %d\n", result);
	   return -1;
   }
#endif

   if (init_func) {
      result = MultiThreadInit(init_func, thread_data);
   }
   if (result != 0) {
      fprintf(stderr, "Error initializing threads\n");
      return -1;
   }

   //fprintf(stdout, "[Mutatee-initProcTest] Ping signal FD.(%d/%d)\n",gettid(), getpid() );
   pingSignalFD(signal_fd);
   getSocketInfo();

   //fprintf(stdout, "[Mutatee-initProcTest] init mutator connection.(%d/%d)\n",gettid(), getpid());
   result = initMutatorConnection();
   if (result != 0) {
      fprintf(stderr, "Error initializing connection to mutator\n");
      return -1;
   }

   //fprintf(stdout, "[Mutatee-initProcTest] hand shake with server.(%d/%d)\n", gettid(),getpid());
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
   /*fprintf(stderr, "end finiProcControlTest\n");*/
   return has_error ? -1 : 0;
}

#if defined(__cplusplus)
extern "C" {
#endif

volatile char MutatorSocket[4096];

char *socket_type = NULL;
char *socket_name = NULL;

#if defined(__cplusplus)
}
#endif

void getSocketInfo()
{
   int count = 0;
	char *space = NULL;
   count = 0;
   while (MutatorSocket[0] == '\0') {
#if defined(os_windows_test)
	   Sleep(10);
#else
	   usleep(10000); /*.01 seconds*/
#endif
	   count++;
      if (count >= MESSAGE_TIMEOUT * 100) {
         logerror("Mutatee timeout\n");
         exit(-2);
      }
   }
   MutatorSocket[4095] = '\0';
   socket_type = (char *) MutatorSocket;
   space = strchr((char *) MutatorSocket, ' ');
   socket_name = space+1;
   *space = '\0';
}

#if !defined(os_windows_test)

#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

void sigalarm_handler(int sig)
{
   assert(sig == SIGALRM);
   timeout = 1;
}

void setTimeoutAlarm()
{
   static int set_alarm_handler = 0;
   if (!set_alarm_handler) {
     struct sigaction action;
     action.sa_handler = sigalarm_handler;
     sigemptyset(&action.sa_mask);
     action.sa_flags = 0;

      set_alarm_handler = 1;
      sigaction(SIGALRM, &action, NULL);
   }
   timeout = 0;
   alarm(MESSAGE_TIMEOUT);
}

void resetTimeoutAlarm()
{
   alarm(0);
}

#if defined(os_bgq_test)
static int created_named_pipes = 0;
#endif
static void createNamedPipes()
{
#if defined(os_bgq_test)
   int id, result, size, i;
   uint32_t ready = 0x42;
   if (created_named_pipes)
      return;
   created_named_pipes = 1;

   if (strcmp(socket_type, "named_pipe") != 0)
      return;

   unsigned int len = strlen(socket_name) + 16;
   char *rd_socketname = (char *) malloc(len);
   char *wr_socketname = (char *) malloc(len);

   result = MPI_Comm_rank(MPI_COMM_WORLD, &id);
   if (result != MPI_SUCCESS) {
     fprintf(stderr, "Failed to get MPI_Comm_rank\n");
     return;
   }
   result = MPI_Comm_size(MPI_COMM_WORLD, &size);
   if (result != MPI_SUCCESS) {
     fprintf(stderr, "Failed to get MPI_Comm_size\n");
     return;
   }

   for (i=0; i<size; i++) {
      if (i == id) {
         snprintf(rd_socketname, len, "%s_w.%d", socket_name, id);
         do {
            r_pipe = open(rd_socketname, O_RDONLY | O_NONBLOCK);
         } while (r_pipe == -1 && (errno == ENXIO || errno == EINTR));
         if (r_pipe == -1) {
            int error = errno;
            fprintf(stderr, "Mutatee failed to create read pipe for %s: %s\n", rd_socketname, strerror(error));
            assert(0);
         }
      }
      MPI_Barrier(MPI_COMM_WORLD);
   }

   for (i=0; i<size; i++) {
      if (i == id) {
         snprintf(wr_socketname, len, "%s_r.%d", socket_name, id);
         do {
            w_pipe = open(wr_socketname, O_WRONLY);
         } while (w_pipe == -1 && errno == EINTR);
         if (w_pipe == -1) {
            int error = errno;
            fprintf(stderr, "Mutatee failed to create write pipe for %s: %s\n", wr_socketname, strerror(error));
            assert(0);
         }
      }
      MPI_Barrier(MPI_COMM_WORLD);
   }

   unlink(rd_socketname);
   unlink(wr_socketname);
   free(rd_socketname);
   free(wr_socketname);

   result = send_message((unsigned char *) &ready, 4);
   if (result == -1) {
      int error = errno;
      fprintf(stderr, "Failed to write to pipe during setup: %s (%d)\n", strerror(error), error);
   }

   result = recv_message((unsigned char *) &ready, 4);
   if (result == -1) {
      int error = errno;
      fprintf(stderr, "Failed to read from pipe during setup: %s (%d)\n", strerror(error), error);
   }
#endif
}

int initMutatorConnection()
{
   int result;

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
   if (strcmp(socket_type, "named_pipe") == 0) {
      createNamedPipes();
   }

   return 0;
}

int send_message_socket(unsigned char *msg, size_t msg_size)
{
   return send(sockfd, msg, msg_size, 0);
}

int send_message_pipe(unsigned char *msg, size_t msg_size)
{
   int had_error = 0;
#if defined(os_bgq_test)
   int my_rank;
   int world_size;
   int i;

   assert(created_named_pipes);

   MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
   MPI_Comm_size(MPI_COMM_WORLD, &world_size);

   for (i = 0; i < world_size; i++) {
      if (i == my_rank) {
         had_error = write(w_pipe, msg, msg_size);
      }
      MPI_Barrier(MPI_COMM_WORLD);
   }
#endif
   return had_error;
}

int send_message(unsigned char *msg, size_t msg_size)
{
	int result;
	if (strcmp(socket_type, "un_socket") == 0) {
      result = send_message_socket(msg, msg_size);
	}
   else if (strcmp(socket_type, "named_pipe") == 0) {
      result = send_message_pipe(msg, msg_size);
   }
   else {
      assert(0);
   }
   if (result == -1) {
      perror("Mutatee unable to send message");
      return -1;
   }
   return 0;
}

static int recv_message_socket(unsigned char *msg, size_t msg_size)
{
   assert(strcmp(socket_type, "un_socket") == 0);
   static int warned_syscall_restart = 0;
   int result = -1;
   int timeout = MESSAGE_TIMEOUT * 10;
   int no_select = 0;

#if 0 //make a test on pselect
   sigset_t emptyset, blockset;
    sigemptyset(&blockset);         /* Block SIGINT */
    sigaddset(&blockset, SIGTRAP);
    sigprocmask(SIG_BLOCK, &blockset, &emptyset);
#endif

   while( result != (int) msg_size && result != 0 ) {

      fd_set read_set;
      FD_ZERO(&read_set);
      FD_SET(sockfd, &read_set);

      struct timeval s_timeout;
      s_timeout.tv_sec = 1; /* 1 sec, as needed on bruckner */
      s_timeout.tv_usec = 0;

      //aarch64-debug: this is stuck in aarch64 when testing singlestep
#if 0
      fprintf(stderr, "before select\n");
      struct timespec p_timeout;
        p_timeout.tv_sec = 1;
        p_timeout.tv_nsec = 0;
      sigemptyset(&emptyset);
      int sresult = pselect(sockfd+1, &read_set, NULL, NULL, &p_timeout, &emptyset);
      fprintf(stderr, "after select\n");
#else
      int sresult = select(sockfd+1, &read_set, NULL, NULL, &s_timeout);
#endif
      int error = errno;

      if (sresult == -1)
      {
         if (error == EINVAL || error == EBADF) {
	    fprintf(stderr, "Mutatee unable to receive message during select: %d, %s\n", error, strerror(error));
            return -1;
         }
         else if (error == EINTR) {
            continue;
         }
         else if (error == ENOSYS) {
            no_select = 1;
         }
         else {
            /* Seen as kernels with broken system call restarting during IRPC test. */
            if (error == 514) {
               // No idea what a 514 error is; it shows up on RHEL5 all the time so I'm
               // preventing the error printout.
               continue;
            }

            if (!warned_syscall_restart) {
               fprintf(stderr, "WARNING: Unknown error %d out of select--broken syscall restarting in kernel?\n", error);
               warned_syscall_restart = 1;
            }
            continue;
         }
      }
      if (sresult == 0) {
         timeout--;
         if (timeout > 0)
            continue;
         perror("Timeout waiting for message\n");
         return -1;
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

static int recv_message_pipe(unsigned char *msg, size_t msg_size)
{
   int had_error = 0;
#if defined(os_bgq_test)
   unsigned int bytes_read = 0;
   int my_rank;
   int world_size;
   int i;
   int result;

   assert(created_named_pipes);

   MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
   MPI_Comm_size(MPI_COMM_WORLD, &world_size);

   /**
    * Serialize access to IO system with barriers.
    * Otherwise it's easy to deadlock BlueGene.
    **/
   for (i=0; i < world_size; i++) {
      if (i == my_rank) {
         unsigned int num_retries = 300; /*30 seconds*/
         do {
            struct pollfd fds[1];
            memset(fds, 0, sizeof(struct pollfd));
            fds[0].fd = r_pipe;
            fds[0].events = POLLIN;
            result = poll(fds, 1, 0);
            if (result == -1) {
               had_error = 1;
               perror("Poll failed");
               break;
            }
            if (result == 0) {
               usleep(100000); /*.1 seconds*/
               if (--num_retries == 0) {
                  had_error = 1;
                  fprintf(stderr, "Failed to read message from read pipe\n");
                  break;
               }
            }
         } while (result != 1);
         if (had_error)
            break;

         do {
            result = read(r_pipe, msg + bytes_read, msg_size - bytes_read);
            int error = errno;

            if (result == 0 ||
                (result == -1 && (error == EAGAIN || error == EWOULDBLOCK || error == EIO || error == EINTR)))
            {
               usleep(100000); /*.1 seconds*/
               if (--num_retries <= 0) {
                  fprintf(stderr, "Failed to read message from read pipe\n");
                  had_error = 1;
                  break;
               }
               continue;
            }
            else if (result == -1) {
               fprintf(stderr, "Error: Could not read from read pipe: %s\n", strerror(error));
               had_error = 1;
               break;
            }
            bytes_read += result;
            assert(bytes_read <= msg_size);
         } while (bytes_read < msg_size);
      }
      MPI_Barrier(MPI_COMM_WORLD);
   }
#endif
   return had_error ? -1 : 0;
}


int recv_message(unsigned char *msg, size_t msg_size)
{
   if (strcmp(socket_type, "un_socket") == 0) {
      return recv_message_socket(msg, msg_size);
   }
   else if (strcmp(socket_type, "named_pipe") == 0) {
      return recv_message_pipe(msg, msg_size);
   }
   else {
      assert(0);
   }
}
#else /*  windows  */


int initMutatorConnection()
{
   int result;
   int i, pid;
   struct sockaddr_in server_addr;

   if (!socket_name) {
	   fprintf(stderr, "no socket name set, bailing\n");
	   return 0;
   }
   sscanf(socket_name, "/tmp/pct%d", &pid);

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
         return -1;
      }


   return 0;
}

int send_message(unsigned char *msg, size_t msg_size)
{
   int result;
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
