#include "ParameterDict.h"
#include "proccontrol_comp.h"
#include "communication.h"

#include <cstdio>
#include <cerrno>
#include <cstring>

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
   notification_fd(-1)
   
{
   notification_fd = evNotify()->getFD();
}

static ProcControlComponent *pccomp = NULL;
static Process::cb_ret_t eventCounterFunction(Event::const_ptr ev)
{
   pccomp->eventsRecieved[ev->getEventType()].push_back(ev);
   return Process::cbDefault;
}

bool ProcControlComponent::registerEventCounter(EventType et)
{
   pccomp = this;
   return Process::registerEventCallback(et, eventCounterFunction);
}

bool ProcControlComponent::checkThread(const Thread &thread)
{
   return true;
}

#define MAX_ARGS 128
Process::ptr ProcControlComponent::launchMutatee(RunGroup *group, ParameterDict &params)
{
   char *logfilename = params["logfilename"]->getString();
   char *humanlogname = params["humanlogname"]->getString();
   bool verboseFormat = (bool) params["verbose"]->getInt();
   char thread_num_str[128];

   const char *args[MAX_ARGS];
   unsigned n=0;
   args[n++] = group->mutatee;
   if (logfilename) {
      args[n++] = "-log";
      args[n++] = logfilename;
   }
   if (humanlogname) {
      args[n++] = "-humanlog";
      args[n++] = humanlogname;
   }
   if (!verboseFormat) {
      args[n++] = "-q";
   }
   args[n++] = "-un_socket";
   args[n++] = sockname;

   if (group->threadmode == SingleThreaded) {
      args[n++] = "-st";
   }
   else if (group->threadmode == MultiThreaded) {
      args[n++] = "-mt";
      snprintf(thread_num_str, 128, "%d", DEFAULT_NUM_THREADS);
      args[n++] = thread_num_str;
   }
   if (group->procmode == SingleProcess) {
      args[n++] = "-sp";
   }
   else if (group->procmode == MultiProcess) {
      args[n++] = "-mp";
   }

   bool printed_run = false;
   for (std::vector<TestInfo *>::iterator i = group->tests.begin(); i != group->tests.end(); i++)
   {
      if (shouldRunTest(group, *i)) {
         if (!printed_run) {
            args[n++] = "-run";
            printed_run = true;
         }
         args[n++] = (*i)->name;
      }
   }
   args[n] = NULL;
   assert(n < MAX_ARGS-1);

   Process::ptr proc = Process::ptr();
   if (group->useAttach == CREATE) {
      std::vector<std::string> vargs;
      for (unsigned i=0; i<n; i++) {
         vargs.push_back(std::string(args[i]));
      }
      proc = Process::createProcess(std::string(group->mutatee), vargs);
      if (!proc) {
         logerror("Failed to execute new mutatee\n");
         return Process::ptr();
      }
   }
   else if (group->useAttach == USEATTACH) {
#if !defined(os_windows_test)
      Dyninst::PID pid = fork_mutatee();
      if (!pid) {
         //Child
         execv(group->mutatee, (char * const *)args);
         char buffer[2048];
         snprintf(buffer, 2048, "execv for attach failed on %s: %s\n", 
                  group->mutatee, 
                  strerror(errno));
         logerror(buffer);
         exit(-1);
      }
      int sockfd;
      bool result = acceptConnections(1, &sockfd);
      if (!result) {
         logerror("Unable to accept attach connection\n");
         return Process::ptr();
      }
      
      proc = Process::attachProcess(pid, group->mutatee);
      if (!proc) {
         logerror("Failed to attach to new mutatee\n");
         return Process::ptr();
      }
      process_socks[proc] = sockfd;
#else
	   assert(!"attach not implemented on Windows ProcControl revision yet");
	   return Process::ptr();
#endif
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

bool ProcControlComponent::launchMutatees(RunGroup *group, ParameterDict &param)
{
   bool error = false;
   bool result = setupServerSocket();
   if (!result) {
      logerror("Failed to setup server side socket\n");
      return FAILED;
   }
   
   num_processes = 0;
   if (group->procmode == MultiProcess)
      num_processes = NUM_PARALLEL_PROCS;
   else
      num_processes = 1;
   
   for (unsigned i=0; i<num_processes; i++) {
      Process::ptr proc = launchMutatee(group, param);
      if (proc == NULL) {
         error = true;
         continue;
      }
   }

   EventType thread_create(EventType::None, EventType::ThreadCreate);
   registerEventCounter(thread_create);

   num_threads = group->threadmode == MultiThreaded ? DEFAULT_NUM_THREADS : 0;
   if (group->useAttach == CREATE)
   {
      int num_procs = 0;
      for (std::vector<Process::ptr>::iterator j = procs.begin(); j != procs.end(); j++) {
         bool result = (*j)->continueProc();
         num_procs++;
         if (!result) {
            error = true;
            continue;
         }
      }
   
      bool support_user_threads = false;
      bool support_lwps = false;
#if defined(os_linux_test)
      support_user_threads = true;
      support_lwps = true;
#elif defined(os_bg_test)
      support_user_threads = true;
      support_lwps = false;
#elif defined(os_freebsd_test)
      support_user_threads = true;
      support_lwps = false;
#endif
      assert(support_user_threads || support_lwps);

      if (support_lwps)
      {
         while (eventsRecieved[EventType(EventType::None, EventType::LWPCreate)].size() < num_procs*num_threads) {
            bool result = Process::handleEvents(true);
            if (!result) {
               logerror("Failed to handle events during thread create\n");
               error = true;
            }
         }
      }

      if (support_user_threads)
      {
         while (eventsRecieved[EventType(EventType::None, EventType::UserThreadCreate)].size() < num_procs*num_threads) {
            bool result = Process::handleEvents(true);
            if (!result) {
               logerror("Failed to handle events during thread create\n");
               error = true;
            }
         }
      }

      result = acceptConnections(num_procs, NULL);
      if (!result) {
         logerror("Failed to accept connections from new mutatees\n");
         error = true;
      }

      if (group->state == STOPPED) {
         std::map<Process::ptr, int>::iterator i;
         for (i = process_socks.begin(); i != process_socks.end(); i++) {
            bool result = i->first->stopProc();
            if (!result) {
               logerror("Failed to stop process\n");
               error = true;
            }
         }
      }
   }
   else if (group->useAttach == USEATTACH)
   {
      for (std::vector<Process::ptr>::iterator j = procs.begin(); j != procs.end(); j++) {
         Process::ptr proc = *j;
         if (proc->threads().size() != num_threads+1) {
            logerror("Process has incorrect number of threads");
            error = true;
         }
      }
      if (eventsRecieved[thread_create].size()) {
         logerror("Recieved unexpected thread creation events on process\n");
         error = true;
      }

      if (group->state == RUNNING) {
         std::map<Process::ptr, int>::iterator i;
         for (i = process_socks.begin(); i != process_socks.end(); i++) {
            bool result = i->first->continueProc();
            if (!result) {
               logerror("Failed to continue process");
               error = true;
            }
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

   bool result = launchMutatees(group, params);
   if (!result) {
      logerror("Failed to launch mutatees\n");
      return FAILED;
   }

   return PASSED;
}

Process::cb_ret_t on_exit(Event::const_ptr ev)
{
   return Process::cbDefault;
}

test_results_t ProcControlComponent::group_teardown(RunGroup *group, ParameterDict &params)
{
   bool error = false;
   bool hasRunningProcs;

   if (curgroup_self_cleaning)
      return PASSED;

   Process::registerEventCallback(EventType(EventType::Exit), on_exit);
   do {
      hasRunningProcs = false;
      for (std::vector<Process::ptr>::iterator i = procs.begin(); i != procs.end(); i++) {
         Process::ptr p = *i;
         if (!p->isTerminated()) {
            bool result = block_for_events();
            if (!result) {
               logerror("Process failed to handle events\n");
               return FAILED;
            }
            if (!p->isTerminated()) {
               hasRunningProcs = true;
            }
         }
      }
   } while(hasRunningProcs);

   for (std::vector<Process::ptr>::iterator i = procs.begin(); i != procs.end(); i++) {
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
         logerror("Process has unexpected error code\n");
         error = true;
         continue;
      }
   }
   procs.clear();

   for(std::map<Process::ptr, int>::iterator i = process_socks.begin(); i != process_socks.end(); ++i) {
#if !defined(os_windows_test)
	   if( close(i->second) == -1 ) {
           logerror("Could not close connected socket\n");
           error = true;
       }
#else
	   assert(!"implement socket close correctly");
#endif
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

std::string ProcControlComponent::getLastErrorMsg()
{
   return std::string("");
}

ProcControlComponent::~ProcControlComponent()
{
}

#if !defined(os_windows_test)
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

struct socket_types
{
	typedef sockaddr_un sockaddr_t;
	static int socket()
	{
		return socket(AF_UNIX, SOCK_STREAM, 0);
	}
	static sockaddr_t make_addr()
	{
	   sockaddr_t addr;
	   memset(&addr, 0, sizeof(socket_types::sockaddr_t));
	   addr.sun_family = AF_UNIX;
	   snprintf(addr.sun_path, sizeof(addr.sun_path)-1, "/tmp/pct%d", getpid());
	   return addr;
	}
	static int close(SOCKET s)
	{
		return ::close(s);
	}
	typedef ::socklen_t socklen_t;
};

#else

#include <process.h>
#include <winsock2.h>
#if !defined(MSG_WAITALL)
#define MSG_WAITALL 8
#endif
#define MSG_NOSIGNAL 0 // override unix-ism

struct socket_types
{
	typedef sockaddr_in sockaddr_t;
	static int socket()
	{
		return ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	}
	static sockaddr_t make_addr()
	{
	   sockaddr_t addr;
	   memset(&addr, 0, sizeof(socket_types::sockaddr_t));
	   addr.sin_family = AF_INET;
	   addr.sin_port = _getpid();
	   return addr;
	}
	static int close(SOCKET s)
	{
		return closesocket(s);
	}
	typedef int socklen_t;
};
#endif


#if 1 // !windows
bool ProcControlComponent::setupServerSocket()
{
	int fd = socket_types::socket();
   if (fd == -1) {
      char error_str[1024];
      snprintf(error_str, 1024, "Unable to create socket: %s\n", strerror(errno));
      logerror(error_str);
      return false;
   }
   socket_types::sockaddr_t addr = socket_types::make_addr();
   
   int result = bind(fd, (sockaddr *) &addr, sizeof(socket_types::sockaddr_t));
   if (result != 0){
      char error_str[1024];
      snprintf(error_str, 1024, "Unable to bind socket: %s\n", strerror(errno));
      logerror(error_str);
   }

   result = listen(fd, 512);
   if (result == -1) {
      char error_str[1024];
      snprintf(error_str, 1024, "Unable to listen on socket: %s\n", strerror(errno));
      logerror(error_str);
      return false;
   }

   sockfd = fd;   
   snprintf(sockname, sizeof(sockname)-1, "/tmp/pct%d", getpid());
   return true;
}

bool ProcControlComponent::acceptConnections(int num, int *attach_sock)
{
   std::vector<int> socks;
   assert(num == 1 || !attach_sock);  //If attach_sock, then num == 1

   while (socks.size() < num) {
      fd_set readset; FD_ZERO(&readset);
      fd_set writeset; FD_ZERO(&writeset);
      fd_set exceptset; FD_ZERO(&exceptset);

      FD_SET(sockfd, &readset);
      FD_SET(notification_fd, &readset);
      int nfds = (sockfd > notification_fd ? sockfd : notification_fd)+1;
      
      struct timeval timeout;
      timeout.tv_sec = 30;
      timeout.tv_usec = 0;
      int result = select(nfds, &readset, &writeset, &exceptset, &timeout);
      if (result == 0) {
         logerror("Timeout while waiting for socket connect");
         return false;
      }
      if (result == -1) {
         perror("Error in select");
         return false;
      }

      if (FD_ISSET(sockfd, &readset))
      {
		  socket_types::sockaddr_t addr;
         socket_types::socklen_t addr_size = sizeof(socket_types::sockaddr_t);
         int newsock = accept(sockfd, (struct sockaddr *) &addr, &addr_size);
         if (newsock == -1) {
            char error_str[1024];
            snprintf(error_str, 1024, "Unable to accept socket: %s\n", strerror(errno));
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

bool ProcControlComponent::cleanSocket()
{
   if (!sockname)
      return false;

   int result = unlink(sockname);
   if (result == -1) {
      logerror("Could not clean socket\n");
      return false;
   }
   free(sockname);
   sockname = NULL;
   result = socket_types::close(sockfd);
   if (result == -1) {
      logerror("Could not close socket\n");
      return false;
   }
   return true;
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
      timeout.tv_sec = 15;
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
                          
   result = recv(sfd, (char *)(msg), msg_size, MSG_WAITALL);
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
   for (std::map<Process::ptr, int>::iterator i = process_socks.begin(); i != process_socks.end(); i++) {
      bool result = recv_message(cur_pos, msg_size, i->second);
      if (!result) 
         return false;
      cur_pos += msg_size;
   }
   return true;
}

bool ProcControlComponent::send_message(unsigned char *msg, unsigned msg_size, int sfd)
{
   int result = send(sfd, (char*)(msg), msg_size, MSG_NOSIGNAL);
   if (result == -1) {
      char error_str[1024];
      snprintf(error_str, 1024, "Mutator unable to send message: %s\n", strerror(errno));
      logerror(error_str);
      return false;
   }
   return true;
}

bool ProcControlComponent::send_broadcast(unsigned char *msg, unsigned msg_size)
{
   unsigned char *cur_pos = msg;
   for (std::map<Process::ptr, int>::iterator i = process_socks.begin(); i != process_socks.end(); i++) {
      bool result = send_message(msg, msg_size, i->second);
      if (!result) 
         return false;
   }
   return true;
}

bool ProcControlComponent::block_for_events()
{
   int nfds = notification_fd+1;
   fd_set readset; FD_ZERO(&readset);
   fd_set writeset; FD_ZERO(&writeset);
   fd_set exceptset; FD_ZERO(&exceptset);
   FD_SET(notification_fd, &readset);

   struct timeval timeout;
   timeout.tv_sec = 15;
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
}
#else

bool ProcControlComponent::send_broadcast(unsigned char*, unsigned int)
{
	assert(!"not implemented");
	return false;
}

bool ProcControlComponent::cleanSocket()
{
	assert(!"not implemented");
	return false;
}

bool ProcControlComponent::acceptConnections(int, int*)
{
	assert(!"not implemented");
	return false;
}

bool ProcControlComponent::setupServerSocket()
{
	assert(!"not implemented");
	return false;
}

bool ProcControlComponent::block_for_events()
{
	assert(!"not implemented");
	return false;
}

bool ProcControlComponent::send_message(unsigned char*, unsigned int, Process::ptr)
{
	assert(!"not implemented");
	return false;
}

bool ProcControlComponent::recv_message(unsigned char*, unsigned int, Process::ptr)
{
	assert(!"not implemented");
	return false;
}

bool ProcControlComponent::recv_broadcast(unsigned char*, unsigned int)
{
	assert(!"not implemented");
	return false;
}


#endif

bool ProcControlComponent::poll_for_events()
{
   bool bresult = Process::handleEvents(false);
   return bresult;
}

Process::cb_ret_t on_breakpoint(Event::const_ptr ev) {
    RegisterPool regs;
    if( !ev->getThread()->getAllRegisters(regs) ) {
        fprintf(stderr, "Failed to get registers on breakpoint\n");
    }else{
        fprintf(stderr, "Registers at breakpoint 0x%lx:\n", ev->getEventBreakpoint()->getAddress());
        for(RegisterPool::iterator i = regs.begin(); i != regs.end(); i++) {
            fprintf(stderr, "\t%s = 0x%lx\n", (*i).first.name(), (*i).second);
        }
    }

    return Process::cbThreadContinue;
}

// To be called while debugging
void insertBreakpoint(Process::ptr proc, Address addr) {
    Breakpoint::ptr brkPt = Breakpoint::newBreakpoint();

    Process::registerEventCallback(EventType::Breakpoint, on_breakpoint);

    if( !proc->addBreakpoint(addr, brkPt) ) {
        fprintf(stderr, "Failed to add breakpoint to process %d at addr 0x%lx\n",
                proc->getPid(), addr);
    }else{
        fprintf(stderr, "Added breakpoint to process %d at addr 0x%lx\n",
                proc->getPid(), addr);
    }
}
