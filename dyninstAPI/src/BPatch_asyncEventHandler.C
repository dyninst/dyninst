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


#include "util.h"
#include "BPatch_asyncEventHandler.h"
#include "EventHandler.h"
#include "mailbox.h"
#include "BPatch_libInfo.h"
#include "signalhandler.h"
#include "signalgenerator.h"
#include "mapped_object.h"
#include "rpcMgr.h"
#include <stdio.h>

#if defined (os_windows)
#include <process.h>
#else
#include <signal.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/socket.h>
#define ASYNC_SOCKET_PATH_LEN 128
#endif

#include "BPatch.h"
#include "BPatch_point.h"
#include "BPatch_eventLock.h"
#include "mailbox.h"
#include "callbacks.h"
#include "EventHandler.h"
#include "util.h"
#include "process.h"

using namespace Dyninst;

extern unsigned long primary_thread_id;

BPatch_asyncEventHandler *global_async_event_handler = NULL;
BPatch_asyncEventHandler *getAsync() 
{
  if (!global_async_event_handler) {
    // BPatch creates and initializes, so just...

    abort();
  }
  return global_async_event_handler;
}

void makeThreadDeleteCB(process *p, int index);

//  A wrapper for pthread_create, or its equivalent.

inline THREAD_RETURN  asyncHandlerWrapper(void *h)
{
  ((BPatch_asyncEventHandler * )h)->main();
  DO_THREAD_RETURN;
}

bool BPatch_asyncEventHandler::connectToProcess(process *p)
{
   async_printf("%s[%d][%s]:  enter ConnectToProcess %d\n", 
         FILE__, __LINE__,getThreadStr(getExecThreadID()), p->getPid());

   //  All we do here is add the process to the list of connected processes
   //  with a fd equal to -1, indicating the not-yet-connected state.
   //
   //  Then remotely execute code in the mutatee to initiate the connection.

   //  make sure that this process is not already known

   for (int i = (int) process_fds.size() -1 ; i >= 0; i--) 
   {
      if ((p == process_fds[i].proc) || 
            (p->getPid() == process_fds[i].proc->getPid()))
      {
         //  If it is, delete the old record to prepare for the new one.
         //  This case can be encountered in the case of multiple process management
         //  when processes are created and terminated rapidly.

		  //  Do we need to close/disconnect socket here??

         fprintf(stderr,"%s[%d]:  duplicate request to connect to process %d\n",
               FILE__, __LINE__, p->getPid());

         VECTOR_ERASE(process_fds,i,i);
         //return false;
      }
   } 

   process_record newp;

#if !defined (os_windows)

   std::string sock_fname;

   PDSOCKET newsock =  setup_socket(p->getPid(), sock_fname);

   if (INVALID_PDSOCKET == newsock)
   {
	   fprintf(stderr, "%s[%d]:  failed to setup socket for new proc\n", FILE__, __LINE__);
	   return false;
   }

  async_printf("%s[%d]:  new socket %s\n", FILE__, __LINE__, sock_fname.c_str());

   newp.sock = newsock;

#endif
   //  add process to list
   newp.proc = p;
   newp.fd = -1;

   process_fds.push_back(newp);

   process *llproc = p;

#if defined (os_windows)
   //  find the variable to set with the port number to connect to
   int_variable *res = NULL;

   set<mapped_object *> &rtlib = llproc->runtime_lib;
   set<mapped_object *>::iterator rtlib_it;
   for(rtlib_it = rtlib.begin(); rtlib_it != rtlib.end(); ++rtlib_it) {
       if( !res ) res = const_cast<int_variable *>((*rtlib_it)->getVariable("connect_port"));
       else break;
   }

   if (!res) 
   {
      fprintf(stderr, "%s[%d]:  cannot find var connect_port in rt lib\n",
            FILE__, __LINE__);
      return false;
   }

   int_variable *portVar = res;

   bool result = llproc->writeDataSpace((void *) portVar->getAddress(), 
         sizeof(listen_port), &listen_port);

   if (!result) 
   {
      fprintf(stderr, "%s[%d]:  cannot write var connect_port in rt lib\n",
            FILE__, __LINE__);
      return false;
   }
#endif

#if !defined (os_windows)
   //  tell async handler to expect a new connection:
   int buf = p->getPid();

   if (sizeof(int) != write(control_pipe_write, & buf, sizeof(int)))
   {
	   fprintf(stderr, "%s[%d]:  failed to signal async thread\n", FILE__, __LINE__);
   }

   //  get mutatee to initiate connection

#if 1 
   while (llproc->sh->isActivelyProcessing()) 
   {
	   inferiorrpc_printf("%s[%d]:  waiting before doing user stop for process %d\n", FILE__,
			   __LINE__, llproc->getPid());
	   llproc->sh->waitForEvent(evtAnyEvent);
   }

   if (p->hasExited()) 
   {
	   fprintf(stderr, "%s[%d]:  oneTimeCode failing because process is terminated\n", FILE__, __LINE__);
	   return false;
   }


   long mutator_pid = getpid();
   pdvector<AstNodePtr> the_args;
   the_args.push_back(AstNode::operandNode(AstNode::Constant, (void*)mutator_pid));
   AstNodePtr dynInit = AstNode::funcCallNode("DYNINSTasyncConnect", the_args);
   llproc->getRpcMgr()->postRPCtoDo(dynInit,
                                    true, // Don't update cost
                                    NULL /*no callback*/,
                                    NULL, // No user data
                                    false, // Don't run when done
                                    true, // Use reserved memory
                                    NULL, NULL);// No particular thread or LWP


   llproc->sh->overrideSyncContinueState(ignoreRequest);

   async_printf("%s[%d]:  about to launch RPC for connect\n", FILE__, __LINE__);

   bool rpcNeedsContinue = false;
   //bool rpcNeedsContinue = true;
   llproc->getRpcMgr()->launchRPCs(rpcNeedsContinue,
		   false); // false: not running
   assert(rpcNeedsContinue);

   async_printf("%s[%d]:  continued proc to run RPC -- wait for RPCSignal\n", FILE__, __LINE__);

   if (p->hasExited()) 
   {
	   fprintf(stderr, "%s[%d]:  oneTimeCode failing because process is terminated\n", FILE__, __LINE__);
	   return false;
   }

   eventType evt = llproc->sh->waitForEvent(evtRPCSignal, llproc, NULL /*lwp*/, statusRPCDone);

   if (p->hasExited()) 
   {
	   fprintf(stderr, "%s[%d]:  oneTimeCode failing because process is terminated\n", FILE__, __LINE__);
	   return false;
   }

   if (evt == evtProcessExit)
   {
	   fprintf(stderr, "%s[%d]:  process terminated during async attachl\n", FILE__, __LINE__);
	   return false;
   }

   async_printf("%s[%d]:  after waitForEvent(evtRPCSignal\n", FILE__, __LINE__);

   //  is this needed?
   getMailbox()->executeCallbacks(FILE__, __LINE__);
#else
   BPatch_Vector<BPatch_function *> funcs;

   if (!p->getImage()->findFunction("DYNINSTasyncConnect", funcs)
         || ! funcs.size() ) 
   {
      bpfatal("%s[%d]:  could not find function: DYNINSTasyncConnect\n",
            FILE__, __LINE__);
      return false;
   }

   if (funcs.size() > 1) 
   {
      bperr("%s[%d]:  found %d varieties of function: DYNINSTasyncConnect\n",
            FILE__, __LINE__, funcs.size());
   }

   BPatch_Vector<BPatch_snippet *> args;
   long mutator_pid = getpid();
   args.push_back(new BPatch_constExpr((void*)mutator_pid));
   BPatch_funcCallExpr connectcall(*funcs[0], args);

   //  Run the connect call as oneTimeCode

   void *expected_result = (void *) 1;
   if ( p->oneTimeCodeInt(connectcall) != expected_result ) 
   {
      bpfatal("%s[%d]:  failed to connect mutatee to async handler\n", 
            FILE__, __LINE__);
      return false;
   }

#endif

   if (llproc->hasExited()) 
   {
	   fprintf(stderr, "%s[%d]:  unexpected process exit!\n", FILE__, __LINE__);
	   return false;
   }

   async_printf("%s[%d]:  got new connection\n", FILE__, __LINE__);
#endif


   return true;
}


bool BPatch_asyncEventHandler::detachFromProcess(process *p)
{
	//  find the fd for this process 
	//  (reformat process vector while we're at it)

	// We can call this if the process has already exited; it then
	// just cleans up state without executing any events.

#if ! defined( cap_async_events )
	return true;
#endif

   async_printf("%s[%d]:  welcome to detachFromProcess\n", FILE__, __LINE__);

	int targetfd = -2;
	for (unsigned int i = 0; i < process_fds.size(); ++i) 
	{
		if (process_fds[i].proc == p) 
		{
			//fprintf(stderr, "%s[%d]:  removing process %d\n", FILE__, __LINE__, p->getPid());
			targetfd  = process_fds[i].fd;
			VECTOR_ERASE(process_fds,i,i);
			break;
		}
	} 


	if (targetfd == -2) 
	{
		//  if we have no record of this process. must already be detached
		//bperr("%s[%d]:  detachFromProcess(%d) could not find process record\n",
		//      FILE__, __LINE__, p->getPid());
		async_printf("%s[%d]:  detachFromProcess: could not find process\n", FILE__, __LINE__);
		return true;
	}

	//  if we never managed to fully attach, targetfd might still be -1.
	//  not sure if this could happen, but just return in this case.
	if (targetfd == -1) return true;

	//  get the mutatee to close the comms file desc.

	//  wake up async handler to adjust set of wait fds:
	int buf = -1;

#if !defined(os_windows)
	async_printf("%s[%d]:  detachFromProcess: signalling async thread\n", FILE__, __LINE__);

	if (sizeof(int) != write(control_pipe_write, & buf, sizeof(int)))
	{
		fprintf(stderr, "%s[%d]:  failed to signal async thread\n", FILE__, __LINE__);
	}
#endif

	if (!mutateeDetach(p)) 
	{
		async_printf("%s[%d]:  detachFromProcess: mutateeDetach failed\n", FILE__, __LINE__);
		//bperr("%s[%d]:  detachFromProcess(%d) could not clean up mutatee\n",
		//      FILE__, __LINE__, p->getPid());
	}

	//  close our own file desc for this process.
	P_close(targetfd);

	return true; // true
}

BPatch_asyncEventHandler::BPatch_asyncEventHandler() :
   EventHandler<EventRecord>(BPatch_eventLock::getLock(), "ASYNC",false /*create thread*/),
   monitored_points(addrHash) 
{
   //  prefer to do socket init in the initialize() function so that we can
   //  return errors.
}
#if defined(os_windows)
static
   void
cleanupSockets( void )
{
   WSACleanup();
}

#endif

#if !defined (os_windows)
char *generate_socket_name(char *buf, int mutator_pid, int mutatee_pid)
{
	uid_t euid = geteuid();
	struct passwd *passwd_info = getpwuid(euid);
	assert(passwd_info);
	snprintf(buf, 128, "%s/dyninstAsync.%s.%d.%d", P_tmpdir, 
			passwd_info->pw_name, mutator_pid, mutatee_pid);
	return buf;
}
#endif

#if defined (os_windows)
PDSOCKET BPatch_asyncEventHandler::setup_socket(int , std::string &)
#else
PDSOCKET BPatch_asyncEventHandler::setup_socket(int mutatee_pid, std::string &sock_fname)
#endif
{
	PDSOCKET sock = INVALID_PDSOCKET;

#if defined(os_windows)
   WSADATA data;
   bool wsaok = false;

   // request WinSock 2.0
   if ( WSAStartup( MAKEWORD(2,0), &data ) == 0 )
   {
      // verify that the version that was provided is one we can use

      if ( (LOBYTE(data.wVersion) == 2) && (HIBYTE(data.wVersion) == 0) )
      {
         wsaok = true;
      }
   }

   assert(wsaok);

   //  set up socket to accept connections from mutatees (on demand)
   sock = P_socket(PF_INET, SOCK_STREAM, 0);

   if (INVALID_PDSOCKET == sock) 
   {
      bperr("%s[%d]:  new socket failed, sock = %d, lasterror = %d\n", 
			  FILE__, __LINE__, (unsigned int) sock, WSAGetLastError());
      return INVALID_PDSOCKET;
   }

   struct sockaddr_in saddr;
  struct in_addr *inadr;
  struct hostent *hostptr;

  hostptr = gethostbyname("localhost");
  inadr = (struct in_addr *) ((void*) hostptr->h_addr_list[0]);
  memset((void*) &saddr, 0, sizeof(saddr));
  saddr.sin_family = PF_INET;
  saddr.sin_port = htons(0); // ask system to assign
  saddr.sin_addr = *inadr;
  
  const char *path = "windows-socket";
#else
  //  set up socket to accept connections from mutatees (on demand)
  sock = P_socket(SOCKET_TYPE, SOCK_STREAM, 0);

  if (INVALID_PDSOCKET == sock) 
  {
    bperr("%s[%d]:  new socket failed\n", FILE__, __LINE__);
    return INVALID_PDSOCKET;
  }

  char path[ASYNC_SOCKET_PATH_LEN];
  generate_socket_name(path, getpid(), mutatee_pid);
  sock_fname = std::string(path);

  struct sockaddr_un saddr;
  saddr.sun_family = AF_UNIX;
  strcpy(saddr.sun_path, path);

  //  make sure this file does not exist already.
  if ( 0 != unlink(path) && (errno != ENOENT)) 
  {
     bperr("%s[%d]:  unlink failed [%d: %s]\n", FILE__, __LINE__, errno, 
            strerror(errno));
  }
#endif

  //  bind socket to port (windows) or temp file in the /tmp dir (unix)

  if (PDSOCKET_ERROR == ::bind(sock, (struct sockaddr *) &saddr, 
                             sizeof(saddr))) 
  { 
    bperr("%s[%d]:  bind socket to %s failed\n", FILE__, __LINE__, path);
	return INVALID_PDSOCKET;
  }

#if defined(os_windows)
  //  get the port number that was assigned to us
  int length = sizeof(saddr);
  if (PDSOCKET_ERROR == getsockname(sock, (struct sockaddr *) &saddr,
                                    &length)) 
  {
    bperr("%s[%d]:  getsockname failed\n", FILE__, __LINE__);
	return INVALID_PDSOCKET;
  }

  listen_port = ntohs (saddr.sin_port);
#endif

  // set socket to listen for connections  
  // (we will explicitly accept in the main event loop)

  if (PDSOCKET_ERROR == listen(sock, 32)) 
  {  //  this is the number of simultaneous connects we can handle
    bperr("%s[%d]:  listen to %s failed\n", FILE__, __LINE__, path);
    return INVALID_PDSOCKET;
  }

  return sock;
}

bool BPatch_asyncEventHandler::initialize()
{
  //  Finally, create the event handling thread
#if defined (os_windows)
   std::string sock_fname;

   PDSOCKET newsock =  setup_socket(0, sock_fname);

   if (INVALID_PDSOCKET == newsock)
   {
	   fprintf(stderr, "%s[%d]:  failed to setup socket for new proc\n", FILE__, __LINE__);
	   return false;
   }

   windows_sock = newsock;

#else
	int pipe_desc[2];
	control_pipe_read = -1;
	control_pipe_write = -1;

	int res = pipe(pipe_desc);

	if (-1 == res)
	{
		fprintf(stderr, "%s[%d]:  failed to setup control pipe\n", FILE__, __LINE__);
		return false;
	}
	
	control_pipe_read = pipe_desc[0];
	control_pipe_write = pipe_desc[1];

	if ((control_pipe_read == -1) || (control_pipe_write == -1))
	{
		fprintf(stderr, "%s[%d]:  failed to setup control pipe\n", FILE__, __LINE__);
		return false;
	}
#endif

  if (!createThread()) 
  {
    bperr("%s[%d]:  could not create event handling thread\n", 
          FILE__, __LINE__);
    return false;
  }


  startup_printf("%s[%d]:  Created async thread\n", FILE__ , __LINE__);
  return true;
}

BPatch_asyncEventHandler::~BPatch_asyncEventHandler()
{
  if (isRunning()) 
    if (!shutDown()) 
	{
      bperr("%s[%d]:  shut down async event handler failed\n", FILE__, __LINE__);
    }

#if defined (os_windows)
  WSACleanup();
#endif
}

bool BPatch_asyncEventHandler::shutDown()
{
  if (!isRunning()) return true;

#if defined(os_windows)
  shutDownFlag = true;
#else
  int killres;

  killres = pthread_kill(handler_thread, 9);

  if (killres) 
  {
     fprintf(stderr, "%s[%d]:  pthread_kill: %s[%d]\n", FILE__, __LINE__,
             strerror(killres), killres);
     return false;
  }

  fprintf(stderr, "%s[%d]:  \t\t..... killed.\n", FILE__, __LINE__);

#endif

  return true;
}

bool BPatch_asyncEventHandler::waitNextEvent(EventRecord &ev)
{

	async_printf("%s[%d]:  welcome to waitNextEvent\n", FILE__, __LINE__);

  //  Since this function is part of the main event loop, __most__ of
  //  it is under lock. This is necessary to protect data in this class
  //  (process-fd mappings for ex) from race conditions.
  // 
  //  The basic lock structure:
  //     Lock
  //       do set up for select
  //     Unlock
  //     select();
  // 
  //     Lock
  //       analyze results of select
  //     Unlock
  //     return
  __LOCK;

  //  keep a static list of events in case we get several simultaneous
  //  events from select()...  just in case.

  if (event_queue.size()) 
  {
     // we already have one (from last call of this func)
     //
     //  this might result in an event reordering, not sure if important
     //   (since we are removing from the end of the list)
     //ev = event_queue[event_queue.size() - 1];
     //event_queue.pop_back();
     ev = event_queue[0];
     VECTOR_ERASE(event_queue,0,0);
     bool found = false;
     for (unsigned i=0; i<process_fds.size(); i++) 
     {
        if (process_fds[i].proc &&
            process_fds[i].proc->getPid() == ev.proc->getPid()) 
        {
           found = true;
           break;
        }
        
     }
     if (found) 
     {
        __UNLOCK;
        return true;
     }
     event_queue.push_back(ev);
  }

  int width = 0;
  fd_set readSet;
  fd_set errSet;

  FD_ZERO(&readSet);
  FD_ZERO(&errSet);

  //  start off with a NULL event:
  ev.type = evtNullEvent;

  cleanUpTerminatedProcs();

#if defined (os_windows)
  FD_SET(windows_sock, &readSet);
  FD_SET(windows_sock, &errSet);
  if (windows_sock > (unsigned)width)
      width = windows_sock;
#else
  //  build the set of fds we want to wait on, one fd per process
  FD_SET(control_pipe_read, &readSet);
  FD_SET(control_pipe_read, &errSet);
  if (control_pipe_read > width)
      width = control_pipe_read;
#endif

  for (unsigned int i = 0; i < process_fds.size(); ++i) 
  {
    if (process_fds[i].fd == -1) 
	{
		assert(process_fds[i].sock != INVALID_PDSOCKET);
		FD_SET(process_fds[i].sock, &readSet);
		FD_SET(process_fds[i].sock, &errSet);
		if (process_fds[i].sock > (unsigned)width)
			width = process_fds[i].sock;
	}
	else
	{
		FD_SET(process_fds[i].fd, &readSet);
		FD_SET(process_fds[i].fd, &errSet);
		if ((unsigned)process_fds[i].fd > (unsigned) width)
			width = process_fds[i].fd;
	}
  }

  // "width" is computed but ignored on Windows NT, where sockets
  // are not represented by nice little file descriptors.

  async_printf("%s[%d]:  before select:  width =  %d\n",  FILE__, __LINE__, 
                  width);

  __UNLOCK;

  ////////////////////////////////////////
  //  WARNING:  THIS SECTION IS UNLOCKED -- don't access any non local vars here
  ////////////////////////////////////////

  int result = 0;
  errno = 0;

  do 
  {
    result = P_select(width+1, &readSet, NULL, &errSet, NULL);
  } 
  while ((result == -1) && (errno == EINTR));

  __LOCK;

  async_printf("%s[%d]:  after select:  res =  %d -- %s\n",  FILE__, __LINE__, 
                  result, strerror(errno));

  if (-1 == result) {
    if (errno == EBADF) {
      if (!cleanUpTerminatedProcs()) 
	  {
        //fprintf(stderr, "%s[%d]:  FIXME:  select got EBADF, but no procs "
        // "terminated\n", FILE__, __LINE__);
        __UNLOCK;
        return false;
      }
      else 
	  {
        __UNLOCK;
        return true;  
      }
    }

    async_printf("%s[%d]:  select returned -1: %s\n", FILE__, __LINE__, strerror(errno));
    bperr("%s[%d]:  select returned -1\n", FILE__, __LINE__);
    __UNLOCK;
	return false;
  }


#if defined(os_windows)
  if (FD_ISSET(windows_sock, &readSet)) 
  {
	  struct sockaddr cli_addr;
	  SOCKLEN_T clilen = sizeof(cli_addr);

	  int new_fd = P_accept(windows_sock, (struct sockaddr *) &cli_addr, &clilen);

	  if (-1 == new_fd) 
	  {
		  bperr("%s[%d]:  accept failed\n", FILE__, __LINE__);
		  return false;
	  }

	  async_printf("%s[%d]:  about to read new connection\n", FILE__, __LINE__);

	  //  do a (blocking) read so that we can get the pid associated with
	  //  this connection.
	  EventRecord pid_ev;
	  readReturnValue_t result = readEvent(new_fd, pid_ev);

	  if (result != RRVsuccess) 
	  {
		  async_printf("%s[%d]:  READ ERROR\n", FILE__, __LINE__);
		  return false;
	  }

	  assert(pid_ev.type == evtNewConnection);
	  ev = pid_ev;
	  async_printf("%s[%d]:  new connection to %d\n",  FILE__, __LINE__,
			  ev.proc->getPid());
	  ev.what = new_fd;

  }

#else

  if (FD_ISSET(control_pipe_read, &readSet)) 
  {
	  //  must have a new process to pay attention to
	  int newpid = 0;
	  readReturnValue_t retval = P_socketRead<int>(control_pipe_read, newpid);
	  if (retval != RRVsuccess) 
	  {
		  async_printf("%s[%d]:  read failed\n", FILE__, __LINE__);
		  __UNLOCK;
	  }

	  if (newpid == -1) 
	  {
		  async_printf("%s[%d]:  drop connection request\n",  FILE__, __LINE__, newpid);
		  //  This case happens when we want to tell the async handler that
		  //  we removed a fd from the set of fds to listen to
		  ev.type = evtNullEvent;
		  ev.proc = NULL;
                  __UNLOCK;
		  return true;
	  }

	  async_printf("%s[%d]:  new connection to %d\n",  FILE__, __LINE__, newpid);
	  ev.type = evtNewConnection;
	  ev.proc = NULL;

	  for (unsigned int i = 0; i < process_fds.size(); ++i) 
	  {
		  if (process_fds[i].proc->getPid() == newpid)
			  ev.proc = process_fds[i].proc;
	  }

	  if (ev.proc == NULL)
		  fprintf(stderr, "%s[%d]:  could not find process %d\n", FILE__, __LINE__, newpid);
  }
#endif

  for (unsigned int i = 0; i < process_fds.size(); ++i)
  {
	  PDSOCKET sock = process_fds[i].sock;

	  if (sock == INVALID_PDSOCKET)
		  continue;

	  //  See if we have any new connections (accept):
	  if (FD_ISSET(sock, &readSet)) 
	  {

#if !defined(os_windows)
		  char sock_name[256];
		  generate_socket_name(sock_name, getpid(), process_fds[i].proc->getPid());
#endif

		  struct sockaddr cli_addr;
		  SOCKLEN_T clilen = sizeof(cli_addr);

		  async_printf("%s[%d]:  about to accept new connection\n", FILE__, __LINE__); 

		  int new_fd = P_accept(sock, (struct sockaddr *) &cli_addr, &clilen);

		  if (-1 == new_fd) 
		  {
			  bperr("%s[%d]:  accept failed\n", FILE__, __LINE__);
			  __UNLOCK;
			  abort();
			  return false;
		  }

		  async_printf("%s[%d]:  accepted new connection fd %d\n", FILE__, __LINE__, new_fd); 
		  process_fds[i].fd = new_fd;
		  process_fds[i].sock = INVALID_PDSOCKET;

#if !defined(os_windows)
		  async_printf("%s[%d]:  unlinking socket %s\n", FILE__, __LINE__, sock_name);
		  unlink(sock_name);
#endif

		  async_printf("%s[%d]:  new connection to %d\n",  FILE__, __LINE__, 
				  ev.proc->getPid());

		  ev.what = new_fd;
		  ev.type = evtNewConnection;
		  ev.proc = process_fds[i].proc;
	  }
  }

  ////////////////////////////////////////
  ////////////////////////////////////////
  ////////////////////////////////////////

  //__LOCK;
  //  See if we have any processes reporting events:

  for (unsigned int j = 0; j < process_fds.size(); ++j) 
  {
    if (-1 == process_fds[j].fd) continue;

    //  Possible race here, if mutator removes fd from set, but events
    //  are pending??

    if (!FD_ISSET(process_fds[j].fd, &readSet)) 
       continue;

    async_printf("%s[%d]:  select got event on fd %d, process = %d\n",  FILE__, __LINE__, 
			process_fds[j].fd, process_fds[j].proc->getPid());

    // Read event
    EventRecord new_ev;
    
    readReturnValue_t result = readEvent(process_fds[j].fd, new_ev);
    if (result != RRVsuccess) 
	{
        switch (result) 
		{
        //case RRVillegalProcess:
        case RRVinsufficientData:
        case RRVreadError:
        case RRVerror:
            async_printf("%s[%d]: READ ERROR readEvent returned error code %d\n",
                         FILE__, __LINE__, result);
            continue;
            break;
        case RRVnoData:
            //  This read can fail if the mutatee has exited.  Just note that this
            //  fd is no longer valid, and keep quiet.
            //if (process_fds[j].proc->isTerminated()) {
            async_printf("%s[%d]:  READ ERROR read event failed\n", FILE__, __LINE__);
            //  remove this process/fd from our vector
            async_printf("%s[%d]:  readEvent failed due to process termination\n", 
                         FILE__, __LINE__);

            for (unsigned int k = j+1; k < process_fds.size(); ++k) 
			{
                process_fds[j] = process_fds[k];
            }

            process_fds.pop_back();
            // and decrement counter so we examine this element (j) again
            j--;
            continue;
            break;
        default:
            assert(0 && "Illegal value returned by readEvent");
            break;
        }
    }

    if (new_ev.type == evtNullEvent) 
	{
       continue;
    }

    new_ev.what = process_fds[j].fd;

    if (ev.type == evtNullEvent) 
	{
       //If ev is unset, then set ev to new_ev
       ev = new_ev;
    }
	else 
	{
       // If ev is set, then queue up new_ev as we got more than one.
       event_queue.push_back(new_ev);
    }
  }
  
  __UNLOCK;
  return true;
}

//  threadExitWrapper exists to ensure that callbacks are called before
//  the thread is deleted.  Maybe there's a better way....

void threadDeleteWrapper(BPatch_process *p, BPatch_thread *t)
{
   p->deleteBPThread(t);
}

void threadExitWrapper(BPatch_process *p, BPatch_thread *t, 
                       pdvector<AsyncThreadEventCallback *> *cbs_ptr)
{
  pdvector<AsyncThreadEventCallback *> &cbs = *cbs_ptr;

  for (unsigned int i = 0; i < cbs.size(); ++i) 
  {
      AsyncThreadEventCallback *cb = dynamic_cast<AsyncThreadEventCallback *>(cbs[i]);
      if (cb)
          (*cb)(p,t);
  }

  threadDeleteWrapper(p,t);
}

bool handleThreadCreate(process *p, EventRecord &ev, unsigned index, int lwpid, 
      dynthread_t tid, unsigned long stack_addr, unsigned long start_pc)
{
   //Create the new BPatch_thread object
   async_printf("%s[%d]:  before createOrUpdateBPThread: pid = %d, " \
         "start_pc = %p, addr = %p, tid = %lu, index = %d, " \
         "lwp = %d\n", 
         FILE__, __LINE__, ev.proc->getPid(), (void *) start_pc, 
         (void *) stack_addr, tid, index, lwpid);

   int pid = p->getPid();

   BPatch_process *bpprocess = BPatch::bpatch->getProcessByPid(pid);
   if (!bpprocess)
   {
	   fprintf(stderr, "%s[%d]:  ERROR:  cannot find relevant bpatch process\n", FILE__, __LINE__);
	   return false;
   }

   BPatch_thread *thr = bpprocess->handleThreadCreate(index, lwpid, tid, stack_addr, start_pc);

   if (!thr) 
   {
      async_printf("%s[%d]: handleThreadCreate failed!\n", FILE__, __LINE__);
   }
   else 
   {
      if (thr->getTid() != tid) 
	  {
         fprintf(stderr, "%s[%d]:  thr->getTid(): %lu, tid %lu\n", 
				 FILE__, __LINE__, thr->getTid(), tid);
      }
   }

   async_printf("%s[%d]:  leaving handleThreadCreate\n", FILE__, __LINE__);

   return (NULL != thr);
}

#if defined(x86_64_unknown_linux2_4)
bool readDynamicCallInfo (PDSOCKET fd, Address &callsite_addr, Address &func_addr, unsigned  address_width)
#else
bool readDynamicCallInfo (PDSOCKET fd, Address &callsite_addr, Address &func_addr, unsigned  /*address_width*/)
#endif
{
   BPatch_dynamicCallRecord call_rec;
   readReturnValue_t retval ;
   //is the mutatee 32 or 64 bit?
#if defined(x86_64_unknown_linux2_4)
   if ( address_width == 4 )
   {
      BPatch_dynamicCallRecord32 call_rec_32;

      retval = P_socketRead<BPatch_dynamicCallRecord32>(fd, call_rec_32);
      call_rec.call_site_addr = (void*)call_rec_32.call_site_addr;
      call_rec.call_target = (void*)call_rec_32.call_target;
   } 
   else
   {
      retval = P_socketRead<BPatch_dynamicCallRecord>(fd, call_rec);
   }
#else
   retval = P_socketRead<BPatch_dynamicCallRecord>(fd, call_rec);
#endif

   if (retval != RRVsuccess) 
   {
      fprintf(stderr, "%s[%d]:  failed to read dynamic call record\n",
            FILE__, __LINE__);
      return false;
   }

   callsite_addr = (Address) call_rec.call_site_addr;
   func_addr = (Address) call_rec.call_target;

   return true;
}

bool handleDynamicCall(process *llproc,
      dictionary_hash<Address, BPatch_point *> &monitored_points,
      Address callsite_addr, Address func_addr)
{
   //  find the point that triggered this event
   int pid = llproc->getPid();

   BPatch_process *bpprocess = BPatch::bpatch->getProcessByPid(pid);
   if (!bpprocess)
   {
	   fprintf(stderr, "%s[%d]:  ERROR:  cannot find relevant bpatch process\n", FILE__, __LINE__);
	   return false;
   }

   if (!monitored_points.defines(callsite_addr)) 
   {
      fprintf(stderr, "%s[%d]:  could not find point for address %lu\n", 
            FILE__, __LINE__, (unsigned long) callsite_addr);
      return false;
   }

   BPatch_point *pt = monitored_points[callsite_addr];

   //  found the record(s), now find the function that was called
   int_function *f = llproc->findOneFuncByAddr(func_addr);

   if (!f) 
   {
      fprintf(stderr, "%s[%d]:  failed to find BPatch_function\n",
            FILE__, __LINE__);
      return false;
   }

   //  find the BPatch_function...

   BPatch_function *bpf = NULL;

   if (NULL == (bpf = bpprocess->findOrCreateBPFunc(f, NULL))) 
   {
      fprintf(stderr, "%s[%d]:  failed to find BPatch_function\n",
            FILE__, __LINE__);
      return false;
   }
   
   //  issue the callback(s) and we're done:

   pdvector<CallbackBase *> cbs;

   getCBManager()->dispenseCallbacksMatching(evtDynamicCall, cbs);

   for (unsigned int i = 0; i < cbs.size(); ++i) 
   {
      DynamicCallsiteCallback &cb = * ((DynamicCallsiteCallback *) cbs[i]);
      cb(pt, bpf);
   }

   return true;
}

#if defined(x86_64_unknown_linux2_4)
bool readNewThreadEventInfo(PDSOCKET fd, unsigned long &start_pc, unsigned long &stack_addr, 
      unsigned &index, int &lwpid, dynthread_t &tid, unsigned address_width) 
#else
bool readNewThreadEventInfo(PDSOCKET fd, unsigned long &start_pc, unsigned long &stack_addr, 
      unsigned &index, int &lwpid, dynthread_t &tid, unsigned ) 
#endif
{
   BPatch_newThreadEventRecord call_rec;
   readReturnValue_t retval;

#if defined(x86_64_unknown_linux2_4)
   //is the mutatee 32 or 64 bit?
   if ( address_width == 4)
   {   
	   //32 bit
      BPatch_newThreadEventRecord32 call_rec_32;
      retval = P_socketRead<BPatch_newThreadEventRecord32>(fd, call_rec_32);

      call_rec.ppid=call_rec_32.ppid;
      call_rec.tid=(void*)call_rec_32.tid;
      call_rec.lwp=call_rec_32.lwp;
      call_rec.index=call_rec_32.index;
      call_rec.stack_addr=(void*)call_rec_32.stack_addr;
      call_rec.start_pc=(void*)call_rec_32.start_pc;
   } 
   else 
   {
      retval = P_socketRead<BPatch_newThreadEventRecord>(fd, call_rec);
   }
#else
   retval = P_socketRead<BPatch_newThreadEventRecord>(fd, call_rec);
#endif

   if (retval != RRVsuccess) 
   {
      fprintf(stderr, "%s[%d]:  failed to read thread event call record\n",
            FILE__, __LINE__);
      return false;
   }

   start_pc = (unsigned long) call_rec.start_pc;
   stack_addr = (unsigned long) call_rec.stack_addr;
   index = (unsigned) call_rec.index;
   lwpid = call_rec.lwp;
   tid = (dynthread_t) call_rec.tid;
   return true;
}

bool handleThreadExit(process *appProc,  unsigned index)
{
   int pid = appProc->getPid();

   BPatch_process *bpprocess = BPatch::bpatch->getProcessByPid(pid);
   if (!bpprocess)
   {
	   fprintf(stderr, "%s[%d]:  ERROR:  cannot find relevant bpatch process\n", FILE__, __LINE__);
	   return false;
   }

   BPatch_thread *appThread = bpprocess->getThreadByIndex(index);

   if (!appThread) 
   {
      fprintf(stderr, "%s[%d]:  thread index %d does not exist\n", FILE__, __LINE__, index);
      return false;
   }

   //  this is a bit nasty:  since we need to ensure that the callbacks are 
   //  called before the thread is deleted, we use a special callback function,
   //  threadExitWrapper, specified above, which guarantees serialization.


   pdvector<CallbackBase *> cbs;
   pdvector<AsyncThreadEventCallback *> *cbs_copy = new pdvector<AsyncThreadEventCallback *>;
   getCBManager()->dispenseCallbacksMatching(evtThreadExit, cbs);

   for (unsigned int i = 0; i < cbs.size(); ++i) 
   {
      BPatch::bpatch->signalNotificationFD();
      cbs_copy->push_back((AsyncThreadEventCallback *)cbs[i]); 
   }

   InternalThreadExitCallback *cb_ptr = new InternalThreadExitCallback(threadExitWrapper);
   InternalThreadExitCallback &cb = *cb_ptr;

   cb(bpprocess, appThread, cbs_copy); 

   return true;
}

bool BPatch_asyncEventHandler::handleEventLocked(EventRecord &ev)
{
   //if ((ev.type != evtNewConnection) && (ev.type != evtNullEvent))
      async_printf("%s[%d]:  inside handleEvent, got %s\n", 
            FILE__, __LINE__, eventType2str(ev.type));

   int event_fd = -1;
   process *appProc = NULL;
   unsigned int j;
   //  Go through our process list and find the appropriate record

   for (j = 0; j < process_fds.size(); ++j) 
   {
      if (!process_fds[j].proc) 
      {
         fprintf(stderr, "%s[%d]:  invalid process record!\n", FILE__, __LINE__);
         continue;
      }

      int process_pid = process_fds[j].proc->getPid();

      if (ev.proc && process_pid == ev.proc->getPid()) 
      {
         event_fd = process_fds[j].fd;
         appProc = process_fds[j].proc; 
         break;
      }
   }

   if (!appProc) 
   {
      if (ev.type == evtNullEvent) 
		  return true; 

      //  This can happen if we received a connect packet before the BPatch_process has
      //  been created.  Shove it on the front of the queue.

      pdvector<EventRecord> temp;

      for (unsigned int i = 0; i < event_queue.size(); ++i) 
	  {
         temp.push_back(event_queue[i]);
      }

      event_queue.clear();
      event_queue.push_back(ev);

      for (unsigned int i = 0; i < temp.size(); ++i) 
	  {
         event_queue.push_back(temp[i]);
      }

      return true;
   }

   async_printf("%s[%d]:  handling event type %s\n", FILE__, __LINE__,
         eventType2str(ev.type));

   switch (ev.type) {
      case evtNullEvent:
         return true;
      case evtNewConnection: 
         {
#if defined (os_windows)
            //  add this fd to the pair.
            //  this fd will then be watched by select for new events.

            if (event_fd != -1) {
               // Can happen if we're execing...
               fprintf(stderr, "%s[%d]:  WARNING:  event fd for process %d " \
                     "is %d (not -1)\n", FILE__, __LINE__, 
                     process_fds[j].proc->getPid(), event_fd);
            }         
            process_fds[j].fd = ev.what;

            async_printf("%s[%d]:  after handling new connection, we have\n", 
                  FILE__, __LINE__);
            for (unsigned int t = 0; t < process_fds.size(); ++t) {
               async_printf("\tpid = %d, fd = %d\n", 
                     process_fds[t].proc->getPid(), process_fds[t].fd);
            }
#endif
            return true;
         }

      case evtShutDown:
         return false;

      case evtThreadCreate:
      {
         //  Read details of new thread from fd 
         async_printf("%s[%d]: reading event from fd %d\n",
                      FILE__, __LINE__, ev.fd);
         
         int lock_depth = eventlock->depth();
         for (int i = 0; i < lock_depth; i++) {
            eventlock->_Unlock(FILE__, __LINE__);
         }
         
         
         unsigned long start_pc = (unsigned long) -1;
         unsigned long stack_addr = (unsigned long) -1;
         unsigned index = (unsigned) -1;
         int lwpid = -1;
         dynthread_t tid;

         if (!readNewThreadEventInfo(ev.fd, start_pc, stack_addr, index, lwpid, tid, appProc->getAddressWidth()) ) {
            fprintf(stderr, "%s[%d]:  failed to read thread event call record\n",
                    FILE__, __LINE__);
            return false;
         }
         
         for (int i = 0; i < lock_depth; i++) {
            eventlock->_Lock(FILE__, __LINE__);
         }

#if defined(os_linux) && (defined(arch_x86_64) || defined(arch_x86))
         unsigned maps_size;
         map_entries* entries = getLinuxMaps(appProc->getPid(), maps_size);
         if (entries) {
            bool found = false;
            for (unsigned i=0; i<maps_size; i++) {
               if (stack_addr >= entries[i].start && stack_addr < entries[i].end)
               {
                  stack_addr = entries[i].end;
                  found = true;
                  break;
               }
            }
            if (!found)
               stack_addr = 0x0;
            free(entries);
         }
         else {
            stack_addr = 0x0;
         }
#endif
         
         bool ret = handleThreadCreate(appProc, ev, index, lwpid, tid, stack_addr, start_pc);
         
         async_printf("%s[%d]: signalling event...\n", FILE__, __LINE__);
         ev.proc->sh->signalEvent(evtThreadCreate);
         async_printf("%s[%d]: done signalling event, returning %d\n", FILE__, __LINE__, ret);
         return ret;
      }
      case evtThreadExit: 
         {
            BPatch_deleteThreadEventRecord rec;
            int lock_depth = eventlock->depth();

            for (int i = 0; i < lock_depth; i++) 
			{
               eventlock->_Unlock(FILE__, __LINE__);
            }

            readReturnValue_t retval = P_socketRead<BPatch_deleteThreadEventRecord>(ev.fd, rec);
            async_printf("%s[%d]: read event, retval %d\n", FILE__, __LINE__);

            for (int i = 0; i < lock_depth; i++) 
			{
               eventlock->_Lock(FILE__, __LINE__);
            }

            if (retval != RRVsuccess) 
			{
               fprintf(stderr, "%s[%d]:  failed to read thread event call record\n",
                     FILE__, __LINE__);
               return false;
            }

            unsigned index = (unsigned) rec.index;

            BPatch::bpatch->mutateeStatusChange = true;

            if (!handleThreadExit(appProc, index)) 
			{
               fprintf(stderr, "%s[%d]:  failed to handleThreadExit \n",
                     FILE__, __LINE__);
               return false;
            }

            ev.proc->sh->signalEvent(evtThreadExit);
            return true;
         }
      case evtDynamicCall:
         {
			 async_printf("%s[%d]:  got evtDynamicCall\n",  FILE__, __LINE__);
            //  Read auxilliary packet with dyn call info

            Address callsite_addr = (Address) -1;
            Address func_addr = (Address) -1;

            int lock_depth = eventlock->depth();
            for (int i = 0; i < lock_depth; i++) 
			{
               eventlock->_Unlock(FILE__, __LINE__);
            }

            if (!readDynamicCallInfo(ev.fd, callsite_addr, func_addr, appProc->getAddressWidth())) 
			{
               fprintf(stderr, "%s[%d]:  failed to read dynamic call record\n",
                     FILE__, __LINE__);
               return false;
            }

            for (int i = 0; i < lock_depth; i++) 
			{
               eventlock->_Lock(FILE__, __LINE__);
            }

            if (!handleDynamicCall(appProc, monitored_points, 
                     callsite_addr, func_addr)) 
			{
               fprintf(stderr, "%s[%d]:  failed to handleDynamicCall for address %lu\n", 
                     FILE__, __LINE__, (unsigned long) callsite_addr);
               return false;
            }

            return true;
         }
      case evtUserEvent:
         {
#if !defined (os_windows)
            assert(ev.info > 0);
            char *userbuf = new char[ev.info];

            int lock_depth = eventlock->depth();
            for (int i = 0; i < lock_depth; i++) 
			{
               eventlock->_Unlock(FILE__, __LINE__);
            }

            //  Read auxilliary packet with user specifiedbuffer
            readReturnValue_t retval = P_socketRead<char>(ev.what, *userbuf, ev.info);

            for (int i = 0; i < lock_depth; i++) 
			{
               eventlock->_Lock(FILE__, __LINE__);
            }

            if (retval != RRVsuccess) 
			{
               bperr("%s[%d]:  failed to read user specified data\n",
                     FILE__, __LINE__);
               delete [] userbuf;
               return false;
            }

            pdvector<CallbackBase *> cbs;
            getCBManager()->dispenseCallbacksMatching(evtUserEvent, cbs);

            for (unsigned int i = 0; i < cbs.size(); ++i) 
			{
               BPatch::bpatch->signalNotificationFD();

			   int pid = appProc->getPid();

			   BPatch_process *bpprocess = BPatch::bpatch->getProcessByPid(pid);
			   if (!bpprocess)
			   {
				   fprintf(stderr, "%s[%d]:  ERROR:  cannot find relevant bpatch process\n", FILE__, __LINE__);
				   return false;
			   }

               UserEventCallback *cb = dynamic_cast<UserEventCallback *>(cbs[i]);
               if (cb)
                  (*cb)(bpprocess, userbuf, ev.info);
            }

            delete [] userbuf;
#endif
            return true;
         } 
      default:
         bperr("%s[%d]:  request to handle unsupported event: %s\n", 
               FILE__, __LINE__, eventType2str(ev.type));
         return false;
         break;

   }
   return true;
}
int disconnectCallback(process *, unsigned , void * data, void *)
{
    *((bool*)(data)) = true;
    return 0;
}
        
bool BPatch_asyncEventHandler::mutateeDetach(process *p)
{
   // The process may have already exited... in this case, do nothing
   // but return true. 

   if ((p == NULL) ||
         (p->status() == exited) ||
         (p->status() == detached))
      return true;

#if 1

   // We have a race condition here... if the process already exited we may have
   // destroyed the signal generator.
   if (p == NULL) return true;
   if (p->sh == NULL) return true;


   while (p->sh->isActivelyProcessing()) 
   {
	   async_printf("%s[%d]:  waiting before doing user stop for process %d\n", FILE__,
			   __LINE__, p->getPid());
	   p->sh->waitForEvent(evtAnyEvent);
   }

   // Check again here...
   if (p->sh == NULL) return true;

   if (p->hasExited()) 
   {
	   return true;
   }


   pdvector<AstNodePtr> the_args;
   AstNodePtr dynInit = AstNode::funcCallNode("DYNINSTasyncDisconnect", the_args);
   bool doneDisconnect = false;
   p->getRpcMgr()->postRPCtoDo(dynInit,
                               true, // Don't update cost
                               &disconnectCallback, // callback for correct waiting
                               (void*)(&doneDisconnect), // local flag, set to true
                               false, // Don't run when done
                               true, // Use reserved memory
                               NULL, NULL);// No particular thread or LWP


   p->sh->overrideSyncContinueState(ignoreRequest);

   async_printf("%s[%d]:  about to launch RPC for disconnect\n", FILE__, __LINE__);

   bool rpcNeedsContinue = false;
   //bool rpcNeedsContinue = true;
   p->getRpcMgr()->launchRPCs(rpcNeedsContinue,
                                false); // false: not running
   assert(rpcNeedsContinue);


   async_printf("%s[%d]:  continued proc to run RPC -- wait for RPCSignal\n", FILE__, __LINE__);

   if (p->hasExited()) 
   {
	   return true;
   }
   eventType evt = evtUndefined;
   while(!doneDisconnect) {
       evt = p->sh->waitForEvent(evtRPCSignal, p, NULL /*lwp*/, statusRPCDone);
   }

   if (p->hasExited()) 
   {
	   return true;
   }

   if (evt == evtProcessExit)
   {
	   return true;
   }

   async_printf("%s[%d]:  after waitForEvent(evtRPCSignal\n", FILE__, __LINE__);

   //  is this needed?
   getMailbox()->executeCallbacks(FILE__, __LINE__);
#else
   //  find the function that will initiate the disconnection
   BPatch_Vector<BPatch_function *> funcs;

   if (!p->getImage()->findFunction("DYNINSTasyncDisconnect", funcs)
         || ! funcs.size() ) 
   {
      bpfatal("%s[%d]:  could not find function: DYNINSTasyncDisconnect\n",
            FILE__, __LINE__);
      return false;
   }

   if (funcs.size() > 1) 
   {
      bperr("%s[%d]:  found %d varieties of function: DYNINSTasyncDisconnect\n",
            FILE__, __LINE__, funcs.size());
   }

   //  The (int) argument to this function is our pid

   BPatch_Vector<BPatch_snippet *> args;
   args.push_back(new BPatch_constExpr(P_getpid()));
   BPatch_funcCallExpr disconnectcall(*funcs[0], args);

   //  Run the connect call as oneTimeCode

   if ( p->oneTimeCodeInt(disconnectcall) != 0 ) 
   {
      bpfatal("%s[%d]:  failed to disconnect mutatee to async handler\n", 
            FILE__, __LINE__);
      return false;
   }
#endif

   return true;
}

bool BPatch_asyncEventHandler::cleanUpTerminatedProcs()
{
   bool ret = false;

   //  iterate from end of vector in case we need to use erase()

   for (int i = (int) process_fds.size() -1; i >= 0; i--) 
   {
      if (process_fds[i].proc->status() == exited) 
	  {
         //  fprintf(stderr, "%s[%d]:  Process %d has terminated, cleaning up\n", FILE__, __LINE__, process_fds[i].proc->getPid());
         VECTOR_ERASE(process_fds,i,i);
         ret = true;
      }
   }
   return ret;
}

bool BPatch_asyncEventHandler::cleanupProc(process *p)
{
   bool ret = false;

   //  iterate from end of vector in case we need to use erase()

   for (int i = (int) process_fds.size() -1; i >= 0; i--) 
   {
      if (process_fds[i].proc == p) 
	  {
         //fprintf(stderr, "%s[%d]: Cleaning up process %d\n", FILE__, __LINE__, process_fds[i].proc->getPid());
         VECTOR_ERASE(process_fds,i,i);
         ret = true;
      }
   }

   return ret;
}

eventType rt2EventType(rtBPatch_asyncEventType t)
{       
   switch(t) {
      case rtBPatch_nullEvent: return evtNullEvent;
      case rtBPatch_newConnectionEvent: return evtNewConnection;
      case rtBPatch_internalShutDownEvent: return evtShutDown;
      case rtBPatch_threadCreateEvent: return evtThreadCreate;
      case rtBPatch_threadDestroyEvent: return evtThreadExit;
      case rtBPatch_dynamicCallEvent: return evtDynamicCall;
      case rtBPatch_userEvent: return evtUserEvent;
      default:
		   fprintf(stderr, "%s[%d], invalid conversion\n", FILE__, __LINE__);
   };

   return evtUndefined;
}         


readReturnValue_t BPatch_asyncEventHandler::readEvent(PDSOCKET fd, EventRecord &ev)
{
   rtBPatch_asyncEventRecord rt_ev;
   readReturnValue_t retval = P_socketRead<rtBPatch_asyncEventRecord>(fd, rt_ev);

   if (retval != RRVsuccess) 
   {
      async_printf("%s[%d]:  read failed\n", FILE__, __LINE__);
      return retval;
   }

   ev.proc = process::findProcess(rt_ev.pid);

   if (ev.proc == NULL) 
   {
      // Message failed... I've seen this before when we get garbage
      // over the FD (juniper, first runs'll do it) --bernat
      async_printf("%s[%d]:  read failed, incorrect pid\n", FILE__, __LINE__);
      return RRVerror;
      //return REillegalProcess;
   }

   ev.what = rt_ev.event_fd;
   ev.fd = fd;
   ev.type = rt2EventType(rt_ev.type);
#if !defined(os_windows)
   ev.info = rt_ev.size;
#endif

   async_printf("%s[%d]: read event, proc = %d, fd = %d\n", FILE__, __LINE__,
         ev.proc->getPid(), ev.fd);

   return RRVsuccess;
}

#ifndef CASE_RETURN_STR
#define CASE_RETURN_STR(x) case x: return #x
#endif

const char *asyncEventType2Str(BPatch_asyncEventType ev) 
{
   switch(ev) {
      CASE_RETURN_STR(BPatch_nullEvent);
      CASE_RETURN_STR(BPatch_newConnectionEvent);
      CASE_RETURN_STR(BPatch_internalShutDownEvent);
      CASE_RETURN_STR(BPatch_threadCreateEvent);
	  CASE_RETURN_STR(BPatch_threadDestroyEvent);
	  CASE_RETURN_STR(BPatch_dynamicCallEvent);
	   default:
	  return "BadEventType";
  }
}

bool BPatch_asyncEventHandler::startupThread()
{
  if (!isRunning()) 
  {
    if (!createThread()) 
	{
      fprintf(stderr, "%s[%d]:  failed to create thread\n", FILE__, __LINE__);
      return false;
    }
  }

  return true;
}

bool BPatch_asyncEventHandler::registerMonitoredPoint(BPatch_point *p)
{
  if (monitored_points.defines((Address)p->getAddress())) 
  {
     //    fprintf(stderr, "%s[%d]:  address %lu already exists in monitored_points hash\n", FILE__, __LINE__, (unsigned long) p->getAddress());
    return false;
  }

  monitored_points[(Address)p->getAddress()] = p;

  return true;
}
