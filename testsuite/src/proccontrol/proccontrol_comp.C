#include "ParameterDict.h"
#include "proccontrol_comp.h"
#include "communication.h"
#include "MutateeStart.h"
#include "SymReader.h"
#include "PCErrors.h"
#include <cstdio>
#include <cerrno>
#include <cstring>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

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
   use_mem_communication(true),
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

static int signaled_received = 0;

bool ProcControlComponent::registerEventCounter(EventType et)
{
   pccomp = this;
   Process::registerEventCallback(et, eventCounterFunction);
}

bool ProcControlComponent::checkThread(const Thread &thread)
{
   return true;
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
      if (!pid) {
         string mutateeString = launchMutatee(exec_name, vargs, group, params);
         if (mutateeString == string("")) {
            logerror("Error creating attach process\n");
            return Process::ptr();
         }
         registerMutatee(mutateeString);
         pid = getMutateePid(group);
      }
      assert(pid);

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

bool ProcControlComponent::startMutatees(RunGroup *group, ParameterDict &param)
{
   bool error = false;
   
   num_processes = 0;
   if (group->procmode == MultiProcess)
      num_processes = DEFAULT_NUM_PROCS;
   else
      num_processes = 1;

   bool result = setupServerSocket(param);
   if (!result) {
      logerror("Failed to setup server side socket");
      return false;
   }
   
   SymbolReaderFactory *factory = NULL;
   for (unsigned i=0; i<num_processes; i++) {
      Process::ptr proc = startMutatee(group, param);
      if (proc == NULL) {
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
      SymReader *reader = NULL;
      Dyninst::Offset sym_offset = 0, pid_sym_offset = 0;
      char socket_buffer[4096];
      memset(socket_buffer, 0, 4096);
      if (param["socket_type"] && param["socket_name"]) {
         snprintf(socket_buffer, 4095, "%s %s", param["socket_type"]->getString(), 
                  param["socket_name"]->getString());
      }
      int socket_buffer_len = strlen(socket_buffer);
      string exec_name;
      Dyninst::Offset exec_addr = 0;
      for (vector<Process::ptr>::iterator j = procs.begin(); j != procs.end(); j++) 
      {
         Process::ptr proc = *j;
         Library::ptr lib = proc->libraries().getExecutable();
         if (!reader) {
            if (lib == Library::ptr()) {
               exec_name = group->mutatee;
               exec_addr = 0;
            }
            else {
               exec_name = lib->getName();
               exec_addr = lib->getLoadAddress();
            }
            reader = factory->openSymbolReader(exec_name);
            if (!reader) {
               logerror("Could not open executable\n");
               error = true;
               continue;
            }
            Symbol_t sym = reader->getSymbolByName(string("MutatorSocket"));
            if (!reader->isValidSymbol(sym))
            {
               logerror("Could not find MutatorSocket symbol in executable\n");
               error = true;
               continue;
            }
            sym_offset = reader->getSymbolOffset(sym);

            sym = reader->getSymbolByName(string("expected_pid"));
            if (reader->isValidSymbol(sym))
            {
               pid_sym_offset = reader->getSymbolOffset(sym);
            }
         }
         Dyninst::Address addr = exec_addr + sym_offset;
         proc->writeMemory(addr, socket_buffer, socket_buffer_len+1);            
         if (pid_sym_offset) {
            int expected_pid = (int) proc->getPid();
            proc->writeMemory(exec_addr + pid_sym_offset, &expected_pid, sizeof(int));
         }
      }
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
   num_threads = group->threadmode == MultiThreaded ? DEFAULT_NUM_THREADS : 0;

   result = acceptConnections(num_procs, NULL);
   if (!result) {
      logerror("Failed to accept connections from new mutatees\n");
      error = true;
   }
   
   if (group->createmode == CREATE) {
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
   }
   else if (group->createmode == USEATTACH)
   {
      for (vector<Process::ptr>::iterator j = procs.begin(); j != procs.end(); j++) {
         Process::ptr proc = *j;
#if !defined(os_bg_test)
         if (proc->threads().size() != num_threads+1) {
            logerror("Process has incorrect number of threads");
            error = true;
         }
#else
         //BlueGene OS spawns extra treads
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
      for (vector<Process::ptr>::iterator i = procs.begin(); i != procs.end(); i++) {
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
         logerror("Process has unexpected error code\n");
         error = true;
         continue;
      }
   }
   procs.clear();

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
}

bool ProcControlComponent::setupServerSocket(ParameterDict &param)
{
   if (use_mem_communication) {
      return true;
   }
   sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
   if (sockfd == -1) {
      fprintf(stderr, "Unable to create socket: %s\n", strerror(errno));
      return false;
   }
   struct sockaddr_un addr;
   memset(&addr, 0, sizeof(struct sockaddr_un));
   addr.sun_family = AF_UNIX;
   snprintf(addr.sun_path, sizeof(addr.sun_path)-1, "/tmp/tsc%d", getpid());
   
   int result = bind(sockfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un));
   if (result != 0){
      fprintf(stderr, "Unable to bind socket: %s\n", strerror(errno));
      return false;
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

bool ProcControlComponent::acceptConnections(int num, int *attach_sock)
{
   vector<int> socks;
   assert(num == 1 || !attach_sock);  //If attach_sock, then num == 1

   if (use_mem_communication) {
      for (unsigned i=0; i<num; i++)
         socks.push_back(-1);
   }

   
   while (!use_mem_communication && socks.size() < num) {
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
         struct sockaddr_un addr;
         socklen_t addr_size = sizeof(struct sockaddr_un);
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
      bool result;
      if (!use_mem_communication)
         result = recv_message((unsigned char *) &msg, sizeof(send_pid), socks[i]);
      else
         result = recv_message((unsigned char *) &msg, sizeof(send_pid), procs[i]);
      if (!result) {
         logerror("Could not receive handshake pid\n");
         return false;
      }
      if (msg.code != SEND_PID_CODE)
      {
         logerror("Received bad code in handshake message\n");
         return false;
      }
      map<Dyninst::PID, Process::ptr>::iterator j = process_pids.find(msg.pid);
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

bool ProcControlComponent::cleanSocket()
{
   if (use_mem_communication) {
      return true;
   }

   if (!sockname)
      return false;

   int result = unlink(sockname);
   if (result == -1) {
      logerror("Could not clean socket\n");
      return false;
   }
   free(sockname);
   sockname = NULL;
   result = close(sockfd);
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
   if (use_mem_communication) {
      bool recv_done = false;
      commInfo cinfo(p);
      for (unsigned i=0; i<RECV_TIMEOUT; i++) {
         bool result = recv_message_mem(msg, msg_size, recv_done, cinfo);
         if (!result) return false;
         if (recv_done)
            break;
         sleep(1);
      }
      if (recv_done)
         return true;
      logerror("Timed out while in recv_message\n");
      return false;
   }
   return recv_message(msg, msg_size, process_socks[p]);
}

bool ProcControlComponent::send_message(unsigned char *msg, unsigned msg_size, Process::ptr p)
{
   if (use_mem_communication) {
      bool send_done = false;
      commInfo cinfo(p);
      do {
         bool result = send_message_mem(msg, msg_size, send_done, cinfo);
         if (!result) return false;
      } while (!send_done);
      return true;
   }
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

bool ProcControlComponent::send_message_mem(unsigned char *msg, uint32_t msg_size, bool &send_done, commInfo &info)
{
   bool result;
   if (info.is_done) {
      send_done = true;
      return true;
   }
   send_done = false;

   info.was_stopped = info.proc->allThreadsStopped();
   if (!info.was_stopped) {
      result = info.proc->stopProc();
      if (!result) {
         logerror("Unable to stop process for send\n");
         return false;
      }
   }
   uint32_t recv_buffer_size;
   result = info.proc->readMemory(&recv_buffer_size, info.recv_buffer_size_addr, sizeof(uint32_t));
   if (!result) {
      logerror("Unable to read memory in send\n");
      return false;
   }
   assert(recv_buffer_size == 0);
   result = info.proc->writeMemory(info.recv_buffer_addr, msg, msg_size);
   if (!result) {
      logerror("Unable to write buffer in send\n");
      return false;
   }
   result = info.proc->writeMemory(info.recv_buffer_size_addr, &msg_size, sizeof(uint32_t));
   if (!result) {
      logerror("Unable to write size in send\n");
      return false;
   }

   if (!info.was_stopped) {
      result = info.proc->continueProc();
      if (!result) {
         logerror("Unable to continue process after send\n");
         return false;
      }
   }

   info.is_done = true;
   send_done = true;
   return true;
}

bool ProcControlComponent::recv_message_mem(unsigned char *msg, unsigned msg_size, bool &recv_done, commInfo &info)
{
   bool result;
   if (info.is_done) {
      recv_done = true;
      return true;
   }
   recv_done = false;
   
   info.was_stopped = info.proc->allThreadsStopped();
   if (!info.was_stopped) {
      result = info.proc->stopProc();
      if (!result) {
         logerror("Unable to stop process for send\n");
         return false;
      }
   }
   
   uint32_t send_buffer_size;
   result = info.proc->readMemory(&send_buffer_size, info.send_buffer_size_addr, sizeof(uint32_t));
   if (!result) {
      logerror("Unable to read size memory in recv\n");
      return false;
   }
   if (send_buffer_size) {
      assert(send_buffer_size == msg_size);
      result = info.proc->readMemory(msg, info.send_buffer_addr, send_buffer_size);
      if (!result) {
         logerror("Unable to read buffer memory in recv\n");
         return false;
      }
      uint32_t zero = 0;
      result = info.proc->writeMemory(info.send_buffer_size_addr, &zero, sizeof(uint32_t));
      if (!result) {
         logerror("Unable to write buffer size in recv\n");
         return false;
      }
      info.is_done = true;
      recv_done = true;
   }
   
   if (!info.was_stopped) {
      result = info.proc->continueProc();
      if (!result) {
         logerror("Unable to continue process after send\n");
         return false;
      }
   }
   return true;
}

bool ProcControlComponent::recv_broadcast_mem(unsigned char *msg, unsigned msg_size)
{
   typedef pair<commInfo *, int> compair_t;
   typedef set<compair_t> commprocs_set_t;

   commprocs_set_t commProcs;
   unsigned j = 0;
   for (vector<Process::ptr>::iterator i = procs.begin(); i != procs.end(); i++, j++)
      commProcs.insert(compair_t(new commInfo(*i), j));
   
   int timeout_count = 0;
   for (;;) {
      for (commprocs_set_t::iterator i = commProcs.begin(); i != commProcs.end(); i++) {
         bool recv_done = false;
         bool result = recv_message_mem(msg + (msg_size * i->second), msg_size, recv_done, *i->first);
         if (!result) return false;
         if (recv_done) {
            delete i->first;
            commProcs.erase(i);
            timeout_count = 0;
         }
      }
      if (!commProcs.size()) 
         return true;
      if (timeout_count == RECV_TIMEOUT) {
         logerror("Timeout while receiving broadcast\n");
         return false;
      }
      timeout_count++;
      sleep(1);
   }
   
}

bool ProcControlComponent::send_broadcast_mem(unsigned char *msg, unsigned msg_size)
{
   typedef set<commInfo *> commprocs_set_t;

   commprocs_set_t commProcs;
   for (vector<Process::ptr>::iterator i = procs.begin(); i != procs.end(); i++)
      commProcs.insert(new commInfo(*i));
   
   int timeout_count = 0;
   for (;;) {
      for (commprocs_set_t::iterator i = commProcs.begin(); i != commProcs.end(); i++) {
         bool send_done = false;
         bool result = send_message_mem(msg, msg_size, send_done, **i);
         if (!result) return false;
         if (send_done) {
            delete *i;
            commProcs.erase(i);
            timeout_count = 0;
         }
      }
      if (!commProcs.size()) 
         return true;
      if (timeout_count == RECV_TIMEOUT) {
         logerror("Timeout while receiving broadcast\n");
         return false;
      }
      timeout_count++;
      sleep(1);
   }
}

bool ProcControlComponent::recv_broadcast(unsigned char *msg, unsigned msg_size)
{
   if (use_mem_communication) {
      return recv_broadcast_mem(msg, msg_size);
   }
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
   assert(!use_mem_communication);
   int result = send(sfd, msg, msg_size, MSG_NOSIGNAL);
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
   if (use_mem_communication) {
      return send_broadcast_mem(msg, msg_size);
   }
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
}

bool ProcControlComponent::poll_for_events()
{
   bool bresult = Process::handleEvents(false);
   return bresult;
}

