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
#include "ParameterDict.h"
#include "proccontrol_comp.h"
#include "communication.h"
#include "MutateeStart.h"
#include "SymReader.h"
#include "PCErrors.h"
#include <cstdio>
#include <cerrno>
#include <cstring>

#include <set>
#include <vector>
#include <map>

using namespace std;

TEST_DLL_EXPORT ComponentTester *componentTesterFactory()
{
   return (ComponentTester *) new ProcControlComponent();
}

ProcControlMutator::ProcControlMutator()
{
}

ProcControlMutator::~ProcControlMutator()
{
}

test_results_t ProcControlMutator::setup(ParameterDict &param)
{
   comp = (ProcControlComponent *) param["ProcControlComponent"]->getPtr();
   return PASSED;
}

test_results_t ProcControlMutator::pre_init(ParameterDict &param)
{
   return PASSED;
}

ProcControlComponent::ProcControlComponent() :
   sockfd(0),
   sockname(NULL),
   notification_fd(-1),
   num_processes(0),
   num_threads(0)
{
   notification_fd = evNotify()->getFD();
}

static ProcControlComponent *pccomp = NULL;
static Process::cb_ret_t eventCounterFunction(Event::const_ptr ev)
{
   pccomp->eventsRecieved[ev->getEventType()].push_back(ev);
   return Process::cbDefault;
}

static int signaled_received = 0;

bool ProcControlComponent::registerEventCounter(EventType et)
{
   pccomp = this;
   return Process::registerEventCallback(et, eventCounterFunction);
}

bool ProcControlComponent::checkThread(const Thread &thread)
{
   return true;
}

Process::cb_ret_t setSocketOnLibLoad(Event::const_ptr ev)
{
   EventLibrary::const_ptr lib_ev = ev->getEventLibrary();
   bool have_libc = false;
   for (set<Library::ptr>::const_iterator i = lib_ev->libsAdded().begin(); i != lib_ev->libsAdded().end(); i++) {
      Library::ptr lib = *i;
      if (lib->getName().find("libc-") != string::npos || lib->getName().find("libc.") != string::npos) {
         have_libc = true;
         break;
      }
   }
   if (have_libc) {
      ProcControlComponent::initializeConnectionInfo(ev->getProcess());
   }
   return Process::cbDefault;
}

bool ProcControlComponent::waitForSignalFD(int signal_fd)
{
   return false;
}

#define MAX_ARGS 128
Process::ptr ProcControlComponent::startMutatee(RunGroup *group, ParameterDict &params)
{
   vector<string> vargs;
   string exec_name;   
   getMutateeParams(group, params, exec_name, vargs);

   Process::ptr proc = Process::ptr();
   if (group->createmode == CREATE) {
#if defined(os_bg_test)
      Dyninst::PID pid = getMutateePid(group);
      proc = Process::attachProcess(pid, group->mutatee);
      if (!proc) {
         logerror("Failed to attach to new mutatee\n");
         return Process::ptr();
      }
#else
      proc = Process::createProcess(exec_name, vargs);
      if (!proc) {
         logerror("Failed to execute new mutatee\n");
         return Process::ptr();
      }
#endif
   }
   else if (group->createmode == USEATTACH) {
      Dyninst::PID pid = getMutateePid(group);
      if (pid == NULL_PID) {
         string mutateeString = launchMutatee(exec_name, vargs, group, params);
         if (mutateeString == string("")) {
            logerror("Error creating attach process\n");
            return Process::ptr();
         }
         registerMutatee(mutateeString);
         pid = getMutateePid(group);
      }
      assert(pid != NULL_PID);

      int signal_fd = params.find("signal_fd_in") != params.end() ? params["signal_fd_in"]->getInt() : -1;
      if (signal_fd > 0) {
         bool result = waitForSignalFD(signal_fd);
         if (!result) {
            logerror("Timeout waiting for signalFD\n");
            return Process::ptr();
         }
      }

      proc = Process::attachProcess(pid, group->mutatee);
      if (!proc) {
         logerror("Failed to attach to new mutatee\n");
         return Process::ptr();
      }

   }
   else {
      return Process::ptr();
   }

   assert(proc);
   Dyninst::PID pid = proc->getPid();
   process_pids[pid] = proc;
   procs.push_back(proc);
   return proc;
}

void setupSignalFD(ParameterDict &param)
{
   int fds[2];
   int result = pipe(fds);
   if (result == -1) {
      perror("Pipe error");
      exit(-1);
   }
   param["signal_fd_in"] = new ParamInt(fds[0]);
   param["signal_fd_out"] = new ParamInt(fds[1]);
}

void resetSignalFD(ParameterDict &param)
{
   if (param.find("signal_fd_in") != param.end()) {
      close(param["signal_fd_in"]->getInt());
   }
   if (param.find("signal_fd_out") != param.end()) {
      close(param["signal_fd_out"]->getInt());
   }
}

static char socket_buffer[4096];
static RunGroup *cur_group = NULL;
static SymbolReaderFactory *factory = NULL;

bool ProcControlComponent::initializeConnectionInfo(Process::const_ptr proc)
{
   static map<string, Offset> cached_ms_addrs;

   SymReader *reader = NULL;
   Dyninst::Offset sym_offset = 0;
   Dyninst::Offset exec_addr = 0;
   std::string exec_name;

   Library::const_ptr lib = proc->libraries().getExecutable();
   if (lib == Library::const_ptr()) {
      exec_name = cur_group->mutatee;
      exec_addr = 0;
   }
   else {
      exec_name = lib->getName();
      exec_addr = lib->getLoadAddress();
   }
   
   map<string, Offset>::iterator i = cached_ms_addrs.find(exec_name);
   if (i != cached_ms_addrs.end()) {
      sym_offset = i->second;
   }
   else {
      reader = factory->openSymbolReader(exec_name);
      if (!reader) {
         logerror("Could not open executable\n");
         return false;
      }
      Symbol_t sym = reader->getSymbolByName(string("MutatorSocket"));
      if (!reader->isValidSymbol(sym))
      {
         logerror("Could not find MutatorSocket symbol in executable\n");
         return false;
      }
      sym_offset = reader->getSymbolOffset(sym);
      cached_ms_addrs[exec_name] = sym_offset;
   }

   Dyninst::Address addr = exec_addr + sym_offset;
   bool result = proc->writeMemory(addr, socket_buffer, strlen(socket_buffer)+1);
   if (!result) {
      logerror("Could not write connection information\n");
      return false;
   }
   return true;
}

bool ProcControlComponent::startMutatees(RunGroup *group, ParameterDict &param)
{
   bool error = false;

   num_processes = 0;
   if (group->procmode == MultiProcess)
      num_processes = getNumProcs(param);
   else
      num_processes = 1;
   bool result = setupServerSocket(param);
   if (!result) {
      logerror("Failed to setup server side socket");
      return false;
   }
#if !defined(os_bg_test) && !defined(os_windows_test)
   setupSignalFD(param);
#endif
   for (unsigned i=0; i<num_processes; i++) {
      Process::ptr proc = startMutatee(group, param);
      if (proc == NULL) {
         logerror("Failed to start mutatee\n");
         error = true;
         continue;
      }
      if (!factory) factory = proc->getDefaultSymbolReader();
      assert(factory);
   }
   {
      /**
       * Set the socket name in each process
       **/
      assert(num_processes);
      assert(factory);
      memset(socket_buffer, 0, 4096);
      if (param.find("socket_type") != param.end() && param.find("socket_name") != param.end()) {
         snprintf(socket_buffer, 4095, "%s %s", param["socket_type"]->getString(), 
                  param["socket_name"]->getString());
      }
      cur_group = group;
      for (vector<Process::ptr>::iterator j = procs.begin(); j != procs.end(); j++) {
         bool result = initializeConnectionInfo(*j);
         if (!result) 
            error = true;
      }
#if defined(os_bg_test)
      Process::registerEventCallback(EventType::Library, setSocketOnLibLoad);
#endif
   }

   EventType thread_create(EventType::None, EventType::ThreadCreate);
   registerEventCounter(thread_create);

   int num_procs = 0;
   for (vector<Process::ptr>::iterator j = procs.begin(); j != procs.end(); j++) {
      bool result = (*j)->continueProc();
      num_procs++;
      if (!result) {
         error = true;
         continue;
      }
   }   
   num_threads = group->threadmode == MultiThreaded ? getNumThreads(param) : 0;

   result = acceptConnections(num_procs, NULL);
   if (!result) {
      logerror("Failed to accept connections from new mutatees\n");
      error = true;
   }
#if defined(os_bg_test)
   Process::removeEventCallback(EventType::Library, setSocketOnLibLoad);
#endif
   
   if (group->createmode == CREATE) {
      Process::ptr a_proc = *procs.begin();
      bool support_user_threads = a_proc->supportsUserThreadEvents();
      bool support_lwps = a_proc->supportsLWPEvents();

      assert(support_user_threads || support_lwps);

      if (support_lwps)
      {
         while (eventsRecieved[EventType(EventType::None, EventType::LWPCreate)].size() < num_procs*num_threads) {
            bool result = Process::handleEvents(true);
            if (!result) {
               logerror("Failed to handle events during thread create\n");
               error = true;
               break;
            }
         }
      }

      if (support_user_threads)
      {
         while (eventsRecieved[EventType(EventType::None, EventType::UserThreadCreate)].size() < num_procs*num_threads) {
            bool result = Process::handleEvents(true);
            if (!result) {
               logerror("Failed to handle events during user thread create\n");
               error = true;
               break;
            }
         }
      }
   }
   else if (group->createmode == USEATTACH)
   {
      for (vector<Process::ptr>::iterator j = procs.begin(); j != procs.end(); j++) {
         Process::ptr proc = *j;
#if !defined(os_bg_test)
         if (proc->threads().size() != num_threads+1) {
			 //std::cerr << "Proc " << proc->getPid() << " has " << proc->threads().size() << " threads, expected " << num_threads+1 << std::endl;
            logerror("Process has incorrect number of threads");
//            error = true;
         }
#else
         //BlueGene OS spawns extra threads
         if (proc->threads().size() < num_threads+1) {
            logerror("Process has incorrect number of threads");
            error = true;
         }
#endif
      }
   }

   if (group->state != RUNNING) {
      map<Process::ptr, int>::iterator i;
      for (i = process_socks.begin(); i != process_socks.end(); i++) {
         bool result = i->first->stopProc();
         if (!result) {
            logerror("Failed to stop process\n");
            error = true;
         }
      }
   }

   result = cleanSocket();
   if (!result) {
      logerror("Failed to clean up socket\n");
      error = true;
   }

   handshake shake;
   shake.code = HANDSHAKE_CODE;
   result = send_broadcast((unsigned char *) &shake, sizeof(handshake));
   if (!result) {
      logerror("Failed to send handshake message to processes\n");
      error = true;
   }


   return !error;
}

test_results_t ProcControlComponent::program_setup(ParameterDict &params)
{
   //Dyninst::ProcControlAPI::setDebug(true);
   return PASSED;
}

test_results_t ProcControlComponent::program_teardown(ParameterDict &params)
{
   return PASSED;
}

test_results_t ProcControlComponent::group_setup(RunGroup *group, ParameterDict &params)
{
   process_socks.clear();
   process_pids.clear();
   procs.clear();
   eventsRecieved.clear();
   sockfd = 0;
   sockname = NULL;
   curgroup_self_cleaning = false;

   me.setPtr(this);
   params["ProcControlComponent"] = &me;

   for (unsigned j=0; j<group->tests.size(); j++) {
      ProcControlMutator *mutator = static_cast<ProcControlMutator *>(group->tests[j]->mutator);
      if (!mutator) continue;
      test_results_t result = mutator->pre_init(params);
      if (result == FAILED)
         return FAILED;
   }

   

   bool result = startMutatees(group, params);
   if (!result) {
      logerror("Failed to launch mutatees\n");
      return FAILED;
   }

   return PASSED;
}

Process::cb_ret_t pc_on_exit(Event::const_ptr ev)
{
   return Process::cbDefault;
}

test_results_t ProcControlComponent::group_teardown(RunGroup *group, ParameterDict &params)
{
   bool error = false;
   bool hasRunningProcs;

   resetSignalFD(params);

   if (curgroup_self_cleaning)
      return PASSED;

   Process::registerEventCallback(EventType(EventType::Exit), pc_on_exit);
   do {
      hasRunningProcs = false;
      for (vector<Process::ptr>::iterator i = procs.begin(); i != procs.end(); i++) {
         Process::ptr p = *i;
         if (!p->isTerminated()) {
            bool result = block_for_events();
            if (!result) {
               logerror("Process failed to handle events\n");
			   error = true;
			   continue;
            }
            if (!p->isTerminated()) {
               hasRunningProcs = true;
            }
         }
      }
   } while(hasRunningProcs);

   for (vector<Process::ptr>::iterator i = procs.begin(); i != procs.end(); i++) {
      Process::ptr p = *i;
      if (!p->isTerminated()) {
         logerror("Process did not terminate\n");
         error = true;
         continue;
      }
      if (p->isCrashed()) {
         logerror("Process terminated on crash\n");
         error = true;
         continue;
      }
      if (!p->isExited()) {
         logerror("Process did not report as exited\n");
         error = true;
         continue;
      }
      if (p->getExitCode() != 0) {
         logerror("Process has unexpected error code: 0x%lx (%d)\n", p->getExitCode());
         error = true;
         continue;
      }
   }
   procs.clear();

   for(std::map<Process::ptr, int>::iterator i = process_socks.begin(); i != process_socks.end(); ++i) {
      if ((i->second) == -1)
         continue;
      if( close(i->second) == -1 ) {
         logerror("Could not close connected socket\n");
         error = true;
      }
   }

   return error ? FAILED : PASSED;
}

test_results_t ProcControlComponent::test_setup(TestInfo *test, ParameterDict &parms)
{
   return PASSED;
}

test_results_t ProcControlComponent::test_teardown(TestInfo *test, ParameterDict &parms)
{
   return PASSED;
}

string ProcControlComponent::getLastErrorMsg()
{
   return string("");
}

ProcControlComponent::~ProcControlComponent()
{
#if defined(os_windows_test)
	::WSACleanup();
	::CloseHandle(winsock_event);
#endif
}

bool ProcControlComponent::setupServerSocket(ParameterDict &param)
{
   sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
   if (sockfd == -1) {
      fprintf(stderr, "Unable to create socket: %s\n", strerror(errno));
      return false;
   }
   struct sockaddr_un addr;
   memset(&addr, 0, sizeof(struct sockaddr_un));
   addr.sun_family = AF_UNIX;
   snprintf(addr.sun_path, sizeof(addr.sun_path)-1, "/tmp/tsc%d", getpid());
   
   int timeout = RECV_TIMEOUT * 100;
   int result;
   for (;;) {
      result = bind(sockfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un));
      if (result == 0) {
         break;
      }
      int error = errno;
      if (error == EADDRINUSE && timeout) {
         timeout--;
         usleep(10000);
         continue;
      }
      if (result != 0){
         fprintf(stderr, "Unable to bind socket: %s\n", strerror(error));
         return false;
      }
   }
   result = listen(sockfd, 512);
   if (result == -1) {
      fprintf(stderr, "Unable to listen on socket: %s\n", strerror(errno));
      return false;
   }
   char *socket_type_str = "un_socket";
   sockname = strdup(addr.sun_path);

   ParamString *socket_type = new ParamString(socket_type_str);
   ParamString *socket_name = new ParamString(sockname);
   ParamInt *socket_num = new ParamInt(sockfd);
   param["socket_type"] = socket_type;
   param["socket_name"] = socket_name;
   param["socketfd"] = socket_num;

   return true;
}

#if !defined(os_windows_test)
bool ProcControlComponent::acceptConnections(int num, int *attach_sock)
{
   vector<int> socks;
   assert(num == 1 || !attach_sock);  //If attach_sock, then num == 1

   while (socks.size() < num) {
      fd_set readset; FD_ZERO(&readset);
      fd_set writeset; FD_ZERO(&writeset);
      fd_set exceptset; FD_ZERO(&exceptset);
      
      FD_SET(sockfd, &readset);
      FD_SET(notification_fd, &readset);
      int nfds = (sockfd > notification_fd ? sockfd : notification_fd)+1;
      
      struct timeval timeout;
      timeout.tv_sec = RECV_TIMEOUT;
      timeout.tv_usec = 0;
      int result = select(nfds, &readset, &writeset, &exceptset, &timeout);
      if (result == 0) {
         logerror("Timeout while waiting for socket connect");
         fprintf(stderr, "[%s:%u] - Have recieved %d / %d socks\n", __FILE__, __LINE__, socks.size(), num);
         return false;
      }
      if (result == -1) {
         handleError("Error in select");
         return false;
      }
      
      if (FD_ISSET(sockfd, &readset))
      {
		  socket_types::sockaddr_t addr;
         socket_types::socklen_t addr_size = sizeof(socket_types::sockaddr_t);
         int newsock = accept(sockfd, (struct sockaddr *) &addr, &addr_size);
         if (newsock == -1) {
            char error_str[1024];
            snprintf(error_str, 1024, "Unable to accept socket (2): %s\n", strerror(errno));
            logerror(error_str);
            return false;
         }
         socks.push_back(newsock);
      }
      if (FD_ISSET(notification_fd, &readset)) {
         bool result = Process::handleEvents(true);
         if (!result) {
            logerror("Failed to handle process events\n");
            return false;
         }
      }
   }

   for (unsigned i=0; i<num; i++) {
      send_pid msg;
      bool result;
      result = recv_message((unsigned char *) &msg, sizeof(send_pid), socks[i]);
      if (!result) {
         logerror("Could not receive handshake pid\n");
         return false;
      }
      if (msg.code != SEND_PID_CODE)
      {
         logerror("Received bad code in handshake message\n");
         return false;
      }
      int pid;
#if defined(os_bg_test)
      //BG pids don't always seem to be consistent.
      pid = procs[i]->getPid();
#else
      pid = msg.pid;
#endif
      map<Dyninst::PID, Process::ptr>::iterator j = process_pids.find(pid);
      if (j == process_pids.end()) {
         if (attach_sock) {
            *attach_sock = socks[i];
            return true;
         }
         logerror("Recieved unexpected PID (%d) in handshake message\n", msg.pid);
         return false;
      }
      process_socks[j->second] = socks[i];
   }

   return true;
}
#else
bool ProcControlComponent::acceptConnections(int num, int *attach_sock)
{
   std::vector<int> socks;
   assert(num == 1 || !attach_sock);  //If attach_sock, then num == 1

   while (socks.size() < num) {
	HANDLE notification_event = (HANDLE) notification_fd;

	::WSAEventSelect(sockfd, winsock_event, FD_ACCEPT);
	HANDLE wait_events[2];
	wait_events[0] = winsock_event;
	wait_events[1] = notification_event;
	// 30 second timeout
	int result = ::WaitForMultipleObjects(2, wait_events, FALSE, 30000);

	if(result == WAIT_TIMEOUT) {
		handleError("WaitForMultipleObjects timed out\n");
		return false;
	}
	if(result == WAIT_FAILED || result == WAIT_ABANDONED) {
		handleError("WaitForMultipleObjects failed\n");
		return false;
	}
	int which_event = (result - WAIT_OBJECT_0);
	switch(which_event)
	{
		// notification
	case 1:
		{
			 bool result = Process::handleEvents(true);
			 if (!result) {
				handleError("Failed to handle process events\n");
				return false;
			 }
			}
		 break;
	case 0:
		{
		  socket_types::sockaddr_t addr;
         socket_types::socklen_t addr_size = sizeof(socket_types::sockaddr_t);
         int newsock = accept(sockfd, (struct sockaddr *) &addr, &addr_size);
		 if (newsock == -1) {
			 if (::WSAGetLastError() == WSAEWOULDBLOCK) {
				 continue;
			 }
            char error_str[1024];
            snprintf(error_str, 1024, "Unable to accept socket: %d/%d, %s, %d\n", errno, WSAGetLastError(), strerror(errno), sockfd);
            logerror(error_str);
            return false;
         }
         socks.push_back(newsock);
		}
		break;
	}
   }
   for (unsigned i=0; i<num; i++) {
      send_pid msg;
      bool result = recv_message((unsigned char *) &msg, sizeof(send_pid), socks[i]);
      if (!result) {
         logerror("Could not receive handshake pid\n");
         return false;
      }
      if (msg.code != SEND_PID_CODE)
      {
         logerror("Received bad code in handshake message\n");
         return false;
      }
      std::map<Dyninst::PID, Process::ptr>::iterator j = process_pids.find(msg.pid);
      if (j == process_pids.end()) {
         if (attach_sock) {
            *attach_sock = socks[i];
            return true;
         }
         logerror("Recieved unexpected PID in handshake message\n");
         return false;
      }
      process_socks[j->second] = socks[i];
   }

	return true;
}

#endif

bool ProcControlComponent::cleanSocket()
{
   if (!sockname)
      return false;

   int result;
#if !defined(os_windows_test)
   result = unlink(sockname);
   if (result == -1) {
      logerror("Could not clean socket\n");
      return false;
   }
#endif
   delete[] sockname;
   sockname = NULL;
#if defined(os_windows_test)
   result = socket_types::close(sockfd, winsock_event);
#else
   result = socket_types::close(sockfd);
#endif
   if (result == -1) {
      logerror("Could not close socket\n");
      return false;
   }
   return true;
}

struct commInfo {
   static Address recv_buffer_addr;
   static Address send_buffer_addr;
   static Address recv_buffer_size_addr;
   static Address send_buffer_size_addr;
   static string sym_exec_name;
   Address lookupSym(string symname, SymReader *reader);

   commInfo(Process::ptr p);

   Process::ptr proc;
   bool is_accessed;
   bool is_done;
   bool was_stopped;
};

Address commInfo::recv_buffer_addr;
Address commInfo::send_buffer_addr;
Address commInfo::recv_buffer_size_addr;
Address commInfo::send_buffer_size_addr;
string commInfo::sym_exec_name;

commInfo::commInfo(Process::ptr p) :
   proc(p),
   is_accessed(false),
   is_done(false),
   was_stopped(false)
{
   string exec_name = proc->libraries().getExecutable()->getName();
   if (sym_exec_name != exec_name) {
      sym_exec_name = exec_name;
      SymbolReaderFactory *fact = proc->getDefaultSymbolReader();
      assert(fact);
      SymReader *reader = fact->openSymbolReader(exec_name);
      assert(reader);
      recv_buffer_addr = lookupSym("recv_buffer", reader);
      send_buffer_addr = lookupSym("send_buffer", reader);
      send_buffer_size_addr = lookupSym("send_buffer_size", reader);
      recv_buffer_size_addr = lookupSym("recv_buffer_size", reader);
      fact->closeSymbolReader(reader);
   }
}

Address commInfo::lookupSym(string symname, SymReader *reader)
{
   Symbol_t sym = reader->getSymbolByName(symname);
   assert(reader->isValidSymbol(sym));
   return (Address) reader->getSymbolOffset(sym);
}


bool ProcControlComponent::recv_message(unsigned char *msg, unsigned msg_size, Process::ptr p)
{
   return recv_message(msg, msg_size, process_socks[p]);
}

bool ProcControlComponent::send_message(unsigned char *msg, unsigned msg_size, Process::ptr p)
{
   return send_message(msg, msg_size, process_socks[p]);
}

bool ProcControlComponent::recv_message(unsigned char *msg, unsigned msg_size, int sfd)
{
   int result;
   for (;;) {
      int nfds = sfd > notification_fd ? sfd : notification_fd;
      nfds++;
      fd_set readset; FD_ZERO(&readset);
      fd_set writeset; FD_ZERO(&writeset);
      fd_set exceptset; FD_ZERO(&exceptset);
      FD_SET(sfd, &readset);
      FD_SET(notification_fd, &readset);
      struct timeval timeout;
      timeout.tv_sec = RECV_TIMEOUT;
      timeout.tv_usec = 0;
      do {
         result = select(nfds, &readset, &writeset, &exceptset, &timeout);
      } while (result == -1 && errno == EINTR);
      
      if (result == 0) {
         logerror("Timeout while waiting for communication\n");
         return false;
      }
      if (result == -1) {
         char error_str[1024];
         snprintf(error_str, 1024, "Error calling select: %s\n", strerror(errno));
         logerror(error_str);
         return false;
      }
      
      if (FD_ISSET(notification_fd, &readset)) {
         bool result = Process::handleEvents(true);
         if (!result) {
            logerror("Failed to handle process events\n");
            return false;
         }
      }
      if (FD_ISSET(sfd, &readset)) {
         break;
      }
   } 
                          
   result = recv(sfd, msg, msg_size, MSG_WAITALL);
   if (result == -1) {
      char error_str[1024];
      snprintf(error_str, 1024, "Unable to recieve message: %s\n", strerror(errno));
      logerror(error_str);
      return false;
   }
   return true;
}

bool ProcControlComponent::recv_broadcast(unsigned char *msg, unsigned msg_size)
{
   unsigned char *cur_pos = msg;
   for (map<Process::ptr, int>::iterator i = process_socks.begin(); i != process_socks.end(); i++) {
      bool result = recv_message(cur_pos, msg_size, i->second);
      if (!result) 
         return false;
      cur_pos += msg_size;
   }
   return true;
}

bool ProcControlComponent::send_message(unsigned char *msg, unsigned msg_size, int sfd)
{
	logerror("mutator sending %d bytes\n", msg_size);
   int result = send(sfd, (char*)(msg), msg_size, MSG_NOSIGNAL);
   if (result == -1) {
      char error_str[1024];
      snprintf(error_str, 1024, "Mutator unable to send message: %s\n", strerror(errno));
      logerror(error_str);
      return false;
   }
   logerror("mutator sent %d bytes OK\n", msg_size);
   return true;
}

bool ProcControlComponent::send_broadcast(unsigned char *msg, unsigned msg_size)
{
   unsigned char *cur_pos = msg;
   for (map<Process::ptr, int>::iterator i = process_socks.begin(); i != process_socks.end(); i++) {
      bool result = send_message(msg, msg_size, i->second);
      if (!result) 
         return false;
   }
   return true;
}

bool ProcControlComponent::block_for_events()
{
#if !defined(os_windows_test)
	int nfds = notification_fd+1;
   fd_set readset; FD_ZERO(&readset);
   fd_set writeset; FD_ZERO(&writeset);
   fd_set exceptset; FD_ZERO(&exceptset);
   FD_SET(notification_fd, &readset);

   struct timeval timeout;
   timeout.tv_sec = RECV_TIMEOUT;
   timeout.tv_usec = 0;
   int result;
   do {
      result = select(nfds, &readset, &writeset, &exceptset, &timeout);
   } while (result == -1 && errno == EINTR);
   
   if (result == 0) {
      logerror("Timeout while waiting for event\n");
      return false;
   }
   if (result == -1) {
      char error_str[1024];
      snprintf(error_str, 1024, "Error calling select: %s\n", strerror(errno));
      logerror(error_str);
      return false;
   }
      
   assert(result == 1 && FD_ISSET(notification_fd, &readset));
   bool bresult = Process::handleEvents(true);
   if (!bresult) {
      logerror("Error waiting for events\n");
      return false;
   }
   return true;
#else
	int result = ::WaitForSingleObject((HANDLE)(notification_fd), 15000);
	if(result == WAIT_TIMEOUT) {
        logerror("Timeout while waiting for event\n");
		return false;
	}
	if(result != WAIT_OBJECT_0) {
		logerror("Error waiting for notify\n");
		return false;
	}
	bool proc_result = Process::handleEvents(true);
	if(!proc_result) {
		logerror("Error waiting for events\n");
		return false;
	}
	return true;
#endif
}

bool ProcControlComponent::poll_for_events()
{
   bool bresult = Process::handleEvents(false);
   return bresult;
}

