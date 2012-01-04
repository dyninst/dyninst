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

#include "proccontrol/src/bluegeneq.h"
#include "proccontrol/src/int_event.h"
#include "proccontrol/src/irpc.h"
#include "proccontrol/h/Process.h"

using namespace Dyninst;
using namespace ProcControlAPI;
using namespace std;

#define ELF_X_NAMESPACE ProcControlAPI
#include "common/h/SymLite-elf.h"
#include "common/src/Elf_X.C"

#include <dirent.h>
#include <limits.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <elf.h>

#define DEFAULT_TOOL_TAG "pcontrl"
#define DEFAULT_TOOL_PRIORITY 99

using namespace bgq;
using namespace std;

char *bgq_process::tooltag = DEFAULT_TOOL_TAG;
uint64_t bgq_process::jobid = 0;
uint32_t bgq_process::toolid = 0;
bool bgq_process::set_ids = false;
bool bgq_process::do_all_attach = false;
uint8_t bgq_process::priority = DEFAULT_TOOL_PRIORITY;

int_process *int_process::createProcess(Dyninst::PID p, std::string e)
{
   std::map<int,int> f;
   std::vector<std::string> a;
   std::vector<std::string> envp;
   bgq_process *bgproc = new bgq_process(p, e, a, envp, f);
   assert(bgproc);
   return static_cast<int_process *>(bgproc);
}

int_process *int_process::createProcess(std::string e, std::vector<std::string> a, 
                                        vector<string> envp, std::map<int,int> f)
{
   bgq_process *bgproc = new bgq_process(0, e, a, envp, f);
   assert(bgproc);
   return static_cast<int_process *>(bgproc);
}

int_process *int_process::createProcess(Dyninst::PID, int_process *)
{
   assert(0); //No fork on BlueGene
   return NULL;
}

static Mutex id_init_lock;
bgq_process::bgq_process(Dyninst::PID p, std::string e, std::vector<std::string> a, 
                         vector<string> envp, std::map<int, int> f) :
   int_process(p, e, a, envp, f),
   sysv_process(p, e, a, envp, f),
   thread_db_process(p, e, a, envp, f),
   ppc_process(p, e, a, envp, f),
   hybrid_lwp_control_process(p, e, a, envp, f),
   mmap_alloc_process(p, e, a, envp, f),
   last_ss_thread(NULL),
   hasControlAuthority(false),
   interp_base_set(false),
   page_size_set(false),
   debugger_suspended(false),
   decoder_pending_stop(false),
   have_pending_message(false),
   rank(pid),
   page_size(0),
   interp_base(0),
   initial_thread_list(NULL)
{
   query_transaction = new QueryTransaction(this, Query);
   update_transaction = new UpdateTransaction(this, Update);
   cn = ComputeNode::getComputeNodeByRank(rank);
   cn->procs.insert(this);

   if (!set_ids) {
      ScopeLock lock(id_init_lock);
      if (!set_ids) {
         char *jobid_env = getenv("BG_JOBID");
         assert(jobid_env);
         char *toolid_env = getenv("BG_TOOLID");
         assert(toolid_env);
         jobid = atoi(jobid_env);
         toolid = atoi(toolid_env);
         set_ids = true;
      }
   }
}

bgq_process::~bgq_process()
{
   if (initial_thread_list) {
      delete initial_thread_list;
      initial_thread_list = NULL;
   }
   if (query_transaction) {
      delete query_transaction;
      query_transaction = NULL;
   }
   if (update_transaction) {
      delete update_transaction;
      update_transaction = NULL;
   }
}

bool bgq_process::plat_create()
{
   pthrd_printf("Attempted to do an unsupported create on BlueGene\n");
   setLastError(err_unsupported, "Create not yet supported on BlueGene\n");
   return false;
}

bool bgq_process::plat_attach(bool, bool &needsSync)
{
   bool result = handleStartupEvent(NULL);
   if (!result) {
      pthrd_printf("handleStartupEvent failed\n");
      return false;
   }

   needsSync = true;
   return true;
}

bool bgq_process::plat_forked()
{
   assert(0); //No fork on BG/Q
}

bool bgq_process::plat_detach(result_response::ptr /*resp*/)
{
#warning TODO implement detach
   return false;
}

bool bgq_process::plat_terminate(bool & /*needs_sync*/)
{
#warning TODO implement terminate
   return false;
}

bool bgq_process::needIndividualThreadAttach()
{
   return true;
}

bool bgq_process::getThreadLWPs(std::vector<Dyninst::LWP> &lwps)
{
   if (!initial_thread_list)
      return true;

   for (uint32_t i = 0; i < initial_thread_list->numthreads; i++) {
      lwps.push_back(initial_thread_list->threadlist[i].tid);
   }
   delete initial_thread_list;
   initial_thread_list = NULL;
   return true;
}

bool bgq_process::plat_getInterpreterBase(Address &base)
{
   if (interp_base_set) {
      base = interp_base;
      return true;
   }
      
   uint32_t len = get_auxvectors_result.length / sizeof(Elf64_auxv_t);
   Elf64_auxv_t *auxvs = (Elf64_auxv_t *) get_auxvectors_result.data;
   
   for (uint32_t i = 0; i < len; i++) {
      if (auxvs[i].a_type == AT_BASE) {
         base = interp_base = (Address) auxvs[i].a_un.a_val;
         interp_base_set = true;
         return true;
      }
   }
   
   perr_printf("Failed to find interpreter base for %d\n", getPid());
   base = 0;
   return false;
}

bool bgq_process::plat_supportDOTF()
{
   return false;
}

Dyninst::OSType bgq_process::getOS() const
{
   return Dyninst::BlueGeneQ;
}

Dyninst::Architecture bgq_process::getTargetArch()
{
   return Dyninst::Arch_ppc64;
}

unsigned int bgq_process::getTargetPageSize()
{
   if (page_size_set) {
      return page_size;
   }

   uint32_t len = get_auxvectors_result.length / sizeof(Elf64_auxv_t);
   Elf64_auxv_t *auxvs = (Elf64_auxv_t *) get_auxvectors_result.data;
   
   for (uint32_t i = 0; i < len; i++) {
      if (auxvs[i].a_type == AT_PAGESZ) {
         page_size = (unsigned int) auxvs[i].a_un.a_val;
         page_size_set = true;
         return page_size;
      }
   }
   
   perr_printf("Failed to find page size for %d\n", getPid());
   return false;      
}

Dyninst::Address bgq_process::plat_mallocExecMemory(Dyninst::Address, unsigned int)
{
#warning TODO implement plat_mallocExecMemory
   return 0;
}

bool bgq_process::plat_individualRegAccess()
{
   return true;
}

bool bgq_process::plat_supportLWPPostDestroy()
{
   return false;
}

static SymbolReaderFactory *getElfReader()
{
  static SymbolReaderFactory *symreader_factory = NULL;
  if (symreader_factory)
    return symreader_factory;

  symreader_factory = (SymbolReaderFactory *) new SymElfFactory();
  return symreader_factory;
}

SymbolReaderFactory *bgq_process::plat_defaultSymReader()
{
   return getElfReader();
}

bool bgq_process::plat_readMem(int_thread *, void *, Dyninst::Address, size_t)
{
   assert(0); //No synchronous IO
   return false;
}

bool bgq_process::plat_writeMem(int_thread *, const void *, Dyninst::Address, size_t)
{
   assert(0); //No synchronous IO
   return false;
}

bool bgq_process::plat_needsAsyncIO() const
{
   return true;
}

bool bgq_process::plat_preHandleEvent()
{
   query_transaction->beginTransaction();
   update_transaction->beginTransaction();
   return true;
}

bool bgq_process::plat_postHandleEvent()
{
   bool result = query_transaction->endTransaction();
   if (!result) {
      perr_printf("Error flushing query transaction\n");
      return false;
   }
   result = update_transaction->endTransaction();
   if (!result) {
      perr_printf("Error flushing update transaction\n");
      return false;
   }
   return true;
}

bool bgq_process::plat_individualRegRead()
{
   return false;
}

bool bgq_process::plat_individualRegSet()
{
   return true;
}

bool bgq_process::internal_readMem(int_thread * /*stop_thr*/, Dyninst::Address addr, 
                                   mem_response::ptr resp, int_thread *thr)
{
   pthrd_printf("Reading from memory %lx +%lx on %d with response ID %d\n", 
                addr, (unsigned long) resp->getSize(), getPid(), resp->getID());

   uint32_t size = resp->getSize();
   int num_reads_needed = (size / MaxMemorySize);
   if (size % MaxMemorySize)
      num_reads_needed += 1;
   if (num_reads_needed > 1)
      resp->markAsMultiResponse(num_reads_needed);
   resp->setLastBase(addr);

   for (unsigned cur = 0, j = 0; cur < size; cur += MaxMemorySize, j++) {
      uint32_t read_size;
      if (cur + MaxMemorySize >= size)
         read_size = size - cur;
      else
         read_size = MaxMemorySize;

      GetMemoryCmd mem_cmd;
      mem_cmd.threadID = thr ? thr->getLWP() : 0;
      mem_cmd.addr = addr + cur;
      mem_cmd.length = read_size;
      mem_cmd.specAccess = thr ? SpecAccess_UseThreadState : SpecAccess_ForceNonSpeculative;

      bool result = sendCommand(mem_cmd, GetMemory, resp);
      if (!result) {
         pthrd_printf("Error sending command to read memory\n");
         return false;
      }
   }
   return true;   
}

bool bgq_process::internal_writeMem(int_thread * /*stop_thr*/, const void *local, Dyninst::Address addr,
                                    size_t size, result_response::ptr resp, int_thread *thr)
{
   pthrd_printf("Writing memory %lx +%lx on %d with response ID %d\n", 
                addr, (unsigned long) size, getPid(), resp->getID());

   int num_writes_needed = (size / MaxMemorySize);
   if (size % MaxMemorySize)
      num_writes_needed += 1;
   if (num_writes_needed > 1)
      resp->markAsMultiResponse(num_writes_needed);

   for (unsigned cur = 0, j = 0; cur < size; cur += MaxMemorySize, j++) {
      uint32_t write_size;
      if (cur + MaxMemorySize >= size)
         write_size = size - cur;
      else
         write_size = MaxMemorySize;

      SetMemoryCmd setmem;
      setmem.threadID = thr ? thr->getLWP() : 0;
      setmem.addr = addr + cur;
      setmem.length = write_size;
      setmem.specAccess = thr ? SpecAccess_UseThreadState : SpecAccess_ForceNonSpeculative;
      setmem.sharedMemoryAccess = SharedMemoryAccess_Allow;
      memcpy(setmem.data, ((const char *) local) + cur, write_size);

      bool result = sendCommand(setmem, SetMemory, resp);
      if (!result) {
         pthrd_printf("Error sending command to read memory\n");
         return false;
      }
   }
   return true;
}

bool bgq_process::plat_readMemAsync(int_thread *thr, Dyninst::Address addr, 
                                    mem_response::ptr resp)
{
   return internal_readMem(thr, addr, resp, NULL);
}

bool bgq_process::plat_writeMemAsync(int_thread *thr, const void *local, Dyninst::Address addr,
                                     size_t size, result_response::ptr result)
{
   return internal_writeMem(thr, local, addr, size, result, NULL);
}

bool bgq_process::plat_getOSRunningStates(std::map<Dyninst::LWP, bool> &)
{
   return true;
}

bool bgq_process::plat_suspendThread(int_thread *thrd)
{
   HoldThreadCmd hold_thrd;
   hold_thrd.threadID = thrd->getLWP();

   bool result = sendCommand(hold_thrd, HoldThread);
   if (!result) {
      pthrd_printf("Error sending HoldThread command\n");
      return false;
   }
   return true;
}

bool bgq_process::plat_resumeThread(int_thread *thrd)
{
   ReleaseThreadCmd release_thrd;
   release_thrd.threadID = thrd->getLWP();

   bool result = sendCommand(release_thrd, ReleaseThread);
   if (!result) {
      pthrd_printf("Error sending ReleaseThread command\n");
      return false;
   }
   return true;
}

bool bgq_process::plat_debuggerSuspended()
{
   return debugger_suspended;
}


bool bgq_process::handleStartupEvent(void *data)
{
   ComputeNode *cn = getComputeNode();
   enum {
      action_none = 0,
      do_lone_attach = 1,
      do_group_attach = 2,
      do_group_wait = 3,
      group_done = 4
   } attach_action = action_none;

   /**
    * Most of this complexity is for group attaches, which happen for
    * each process on the CN.  We only want a group attach action to
    * happen once, so only the first attach that's processed for that 
    * group does the attach.  Other processes will either wait for the
    * attach to be ack'd, or they will realize the attach is already ack'd
    * and move on.
    **/
   if (startup_state == issue_attach) {
      if (!cn->do_all_attach) {
         attach_action = do_lone_attach;
      }
      else if (cn->all_attach_error) {
         pthrd_printf("CN attach left us in an error state, failng attach to %d\n", getPid());
         setState(int_process::errorstate);
         return false;
      }
      else if (cn->all_attach_done) {
         attach_action = group_done;
      }
      else {
         ScopeLock lock(cn->attach_lock);
         if (cn->all_attach_error) {
            pthrd_printf("CN attach left us in an error state, failng attach to %d\n", getPid());
            setState(int_process::errorstate);
            return false;
         }
         if (cn->all_attach_done)
            attach_action = group_done;
         else if (cn->issued_all_attach)
            attach_action = do_group_wait;
         else {
            attach_action = do_group_attach;
            cn->issued_all_attach = true;
         }
      }
      pthrd_printf("Doing attach on %d, using attach action %d\n", getPid(), (int) attach_action);
   }

   /**
    * Short-cut the process startup state forward: If a group attach is done, move to 
    * the issue control request state.  If the process is in Query Mode (priority 0) then
    * skip requesting control authority and jump to the data collection stage.
    **/ 
   if (attach_action == group_done || attach_action == do_group_wait) {
      pthrd_printf("Already issued all attach, not re-doing attach to %d.\n", getPid());
      startup_state = issue_control_request;
   }

   if (attach_action == do_lone_attach || attach_action == do_group_attach) {
      AttachMessage attach;
      strncpy(attach.toolTag, tooltag, sizeof(attach.toolTag));
      attach.priority = priority;
      fillInToolMessage(attach, Attach);
      if (attach_action == do_lone_attach) {
         pthrd_printf("Sending attach message to rank %d\n", getPid());
         attach.procSelect = RankInHeader;
      }
      else {
         pthrd_printf("Sending attach message to compute node %d\n", cn->getID());
         attach.procSelect = RanksInNode;
      }
      
      bool result = getComputeNode()->writeToolMessage(this, &attach, false);
      if (!result) {
         pthrd_printf("Error sending Attach from startup handler\n");
         return false;
      }
      startup_state = waitfor_attach;
      return true;
   }

   if (startup_state == waitfor_attach) {
      AttachAckMessage *attach_ack = static_cast<AttachAckMessage *>(data);
      if (cn->issued_all_attach) {
         ScopeLock lock(cn->attach_lock);
         cn->all_attach_done = true;
         set<ToolMessage *>::iterator i;

         if (attach_ack->header.returnCode != Success) {
            perr_printf("Group attach returned error %d: %s\n", (int) attach_ack->header.returnCode,
                        bgq_process::bgqErrorMessage(attach_ack->header.returnCode));
            setState(int_process::errorstate);
            cn->all_attach_error = true;
            for (set<bgq_process *>::iterator i = cn->procs.begin(); i != cn->procs.end(); i++) {
               (*i)->setState(int_process::errorstate);
            }
            return false;
         }
         for (set<bgq_process *>::iterator i = cn->procs.begin(); i != cn->procs.end(); i++) {
            bool result = cn->flushNextMessage(*i);
            if (!result) {
               pthrd_printf("Error flushing startup messages after group attach\n");
               return false;
            }
         }
      }
      else {
         pthrd_printf("Rank attach to %d done.\n", getPid());
      }
      startup_state = issue_control_request;
   }
   
   if (startup_state == issue_control_request && priority == QueryPriorityLevel) {
      pthrd_printf("In Query-Only mode (priority 0). Not issuing control request to %d\n", getPid());
      startup_state = issue_data_collection;
   }

   if (startup_state == issue_control_request) {
      pthrd_printf("Issuing ControlMessage in startup handler\n");
      ControlMessage *ctrl_msg = (ControlMessage *) malloc(sizeof(ControlMessage));
      sigset_t all_sigs;
      sigfillset(&all_sigs);
      ctrl_msg->notifySignalSet(&all_sigs);
      ctrl_msg->sndSignal = SIGSTOP;
      ctrl_msg->dynamicNotifyMode = DynamicNotifyDLoader;
      ctrl_msg->dacTrapMode = DACTrap_NoChange;
      fillInToolMessage(*ctrl_msg, Control);
      
      bool result = getComputeNode()->writeToolAttachMessage(this, ctrl_msg, true);
      if (!result) {
         pthrd_printf("Error writing ControlMessage in attach handler\n");
         delete ctrl_msg;
         return false;
      }

      startup_state = waitfor_control_request_ack;
      return true;
   }
   
   if (startup_state == waitfor_control_request_ack) {
      ControlAckMessage *ctrl_ack = static_cast<ControlAckMessage *>(data);
      if (ctrl_ack->header.returnCode != Success) {
         perr_printf("Error return in ControlAckMessage: %s\n",
                     bgqErrorMessage(ctrl_ack->header.returnCode));
         setLastError(err_internal, "Could not send ControlMessage\n");
         return false;                     
      }
      
      if (ctrl_ack->controllingToolId) {
         pthrd_printf("Tool '%s' with ID %d has authority at priority %d\n", 
                      ctrl_ack->toolTag, ctrl_ack->controllingToolId, ctrl_ack->priority);
         if (ctrl_ack->priority > priority) {
            pthrd_printf("WARNING: Tool %s has higher priority (%d > %d), running in Query-Only mode\n",
                         ctrl_ack->toolTag, ctrl_ack->priority, priority);
            startup_state = issue_data_collection;
         }
         else {
            pthrd_printf("Waiting for control priority release from %s (%d < %d)\n",
                         ctrl_ack->toolTag, ctrl_ack->priority, priority);
            startup_state = waitfor_control_request_notice;
            return true;
         }
      }
      else {
         pthrd_printf("ProcControlAPI is only tool, taking control authority\n");
         startup_state = waitfor_control_request_notice;
         return true;
      }
   }

   if (startup_state == waitfor_control_request_notice) {
      /**
       * Don't really need to use the notify message for anything.
       * Most work handled in the control request handler.
       **/
      NotifyMessage *notify_msg = static_cast<NotifyMessage *>(data);
      if (notify_msg->type.control.reason == NotifyControl_Available) {
         pthrd_printf("Successfully took control, waiting for control request signal\n");
         startup_state = waitfor_control_request_signal;
      }
      else if (notify_msg->type.control.reason == NotifyControl_Conflict) {
         pthrd_printf("Conflict message from control request, moving to data collection\n");
         startup_state = issue_data_collection;
      }
      else
         assert(0);
   }

   if (startup_state == waitfor_control_request_signal) {
      NotifyMessage *notify_msg = static_cast<NotifyMessage *>(data);
      assert(notify_msg->type.signal.reason == NotifySignal_Generic);
      assert(notify_msg->type.signal.signum == SIGSTOP);
      startup_state = issue_data_collection;
   }

   /**
    * Send a set of query in a single CommandMessage.  We want:
    *  - GetProcessData
    *  - AuxVectors
    *  - ThreadList
    **/
   if (startup_state == issue_data_collection) {
      GetProcessDataCmd procdata_cmd;
      procdata_cmd.threadID = 0;
      sendCommand(procdata_cmd, GetProcessData);

      GetAuxVectorsCmd auxv_cmd;
      auxv_cmd.threadID = 0;
      sendCommand(auxv_cmd, GetAuxVectors);

      GetThreadListCmd thread_list;
      thread_list.threadID = 0;
      sendCommand(thread_list, GetThreadList);

      QueryMessage *query_msg = (QueryMessage *) malloc(sizeof(QueryMessage));
      *query_msg = query_transaction->getCommand();
      query_transaction->resetTransactionState();

      bool result = getComputeNode()->writeToolAttachMessage(this, query_msg, true);
      if (!result) {
         pthrd_printf("Error writing GetProcessDataCmd for %d", getPid());
         delete query_msg;
         return false;
      }
      startup_state = waitfor_data_collection;
      return true;
   }

   /**
    * Recieve the data from the above query command.
    **/
   if (startup_state == waitfor_data_collection) {
      pthrd_printf("Handling getProcessDataAck for %d\n", getPid());
      QueryAckMessage *query_ack = static_cast<QueryAckMessage *>(data);
      assert(query_ack->numCommands == 3);

      for (unsigned i = 0; i < (unsigned) query_ack->numCommands; i++) {
         char *cmd_ptr = ((char *) data) + query_ack->cmdList[i].offset;
         if (query_ack->cmdList[i].type == GetProcessDataAck)
            get_procdata_result = *(GetProcessDataAckCmd *) cmd_ptr;
         else if (query_ack->cmdList[i].type == GetAuxVectorsAck)
            get_auxvectors_result = *(GetAuxVectorsAckCmd *) cmd_ptr;
         else if (query_ack->cmdList[i].type == GetThreadListAck) {
            initial_thread_list = new GetThreadListAckCmd();
            *initial_thread_list = *(GetThreadListAckCmd *) cmd_ptr;
         }
      }
      startup_state = startup_done;
   }

   if (startup_state == startup_done) {
      setState(neonatal_intermediate);
      pthrd_printf("Done with startup for %d\n", getPid());
   }

   return true;
}

void bgq_process::fillInToolMessage(ToolMessage &msg, uint16_t msg_type, response::ptr resp)
{
   msg.header.service = ToolctlService;
   msg.header.version = ProtocolVersion;
   msg.header.type = msg_type;
   msg.header.rank = getRank();
   msg.header.sequenceId = resp ? resp->getID() : 0;
   msg.header.returnCode = 0;
   msg.header.errorCode = 0;
   msg.header.length = bgq_process::getMessageLength(msg);
   msg.header.jobId = bgq_process::getJobID();
   msg.toolId = bgq_process::getToolID();
}

ComputeNode *bgq_process::getComputeNode()
{
   return cn;
}

bool bgq_process::decoderPendingStop()
{
   return decoder_pending_stop;
}

void bgq_process::setDecoderPendingStop(bool b)
{
   decoder_pending_stop = b;
}

uint32_t bgq_process::getRank()
{
   return rank;
}

uint64_t bgq_process::getJobID()
{
   return jobid;
}

uint32_t bgq_process::getToolID()
{
   return toolid;
}

bool bgq_process::sendCommand(const ToolCommand &cmd, uint16_t cmd_type, response::ptr resp)
{
   uint16_t msg_type = getCommandMsgType(cmd_type);
   if (msg_type == Query) {
      return query_transaction->writeCommand(&cmd, cmd_type, resp);
   }
   else if (msg_type == Update) {
      return update_transaction->writeCommand(&cmd, cmd_type, resp);
   }
   else {
      assert(0); //Are we trying to send an Ack?
   }
   return false;
}

#define CMD_RET_SIZE(X) case X: return sizeof(X ## Cmd)
#define CMD_RET_VAR_SIZE(X, F) case X: return sizeof(X ## Cmd) + static_cast<const X ## Cmd &>(cmd).F
uint32_t bgq_process::getCommandLength(uint16_t cmd_type, const ToolCommand &cmd)
{
   switch (cmd_type) {
      CMD_RET_SIZE(GetSpecialRegs);
      CMD_RET_SIZE(GetSpecialRegsAck);
      CMD_RET_SIZE(GetGeneralRegs);
      CMD_RET_SIZE(GetGeneralRegsAck);
      CMD_RET_SIZE(GetFloatRegs);
      CMD_RET_SIZE(GetFloatRegsAck);
      CMD_RET_SIZE(GetDebugRegs);
      CMD_RET_SIZE(GetDebugRegsAck);
      CMD_RET_SIZE(GetMemory);
      CMD_RET_VAR_SIZE(GetMemoryAck, length);
      CMD_RET_SIZE(GetThreadList);
      CMD_RET_SIZE(GetThreadListAck);
      CMD_RET_SIZE(GetAuxVectors);
      CMD_RET_SIZE(GetAuxVectorsAck);
      CMD_RET_SIZE(GetProcessData);
      CMD_RET_SIZE(GetProcessDataAck);
      CMD_RET_SIZE(GetThreadData);
      CMD_RET_SIZE(GetThreadDataAck);
      CMD_RET_SIZE(GetPreferences);
      CMD_RET_SIZE(GetPreferencesAck);
      CMD_RET_SIZE(GetFilenames);
      CMD_RET_SIZE(GetFilenamesAck);
      CMD_RET_SIZE(GetFileStatData);
      CMD_RET_SIZE(GetFileStatDataAck);
      CMD_RET_SIZE(GetFileContents);
      CMD_RET_VAR_SIZE(GetFileContentsAck, numbytes);
      CMD_RET_SIZE(SetGeneralReg);
      CMD_RET_SIZE(SetGeneralRegAck);
      CMD_RET_SIZE(SetFloatReg);
      CMD_RET_SIZE(SetFloatRegAck);
      CMD_RET_SIZE(SetDebugReg);
      CMD_RET_SIZE(SetDebugRegAck);
      CMD_RET_SIZE(SetMemory);
      CMD_RET_VAR_SIZE(SetMemoryAck, length);
      CMD_RET_SIZE(HoldThread);
      CMD_RET_SIZE(HoldThreadAck);
      CMD_RET_SIZE(ReleaseThread);
      CMD_RET_SIZE(ReleaseThreadAck);
      CMD_RET_SIZE(InstallTrapHandler);
      CMD_RET_SIZE(InstallTrapHandlerAck);
      CMD_RET_SIZE(AllocateMemory);
      CMD_RET_SIZE(AllocateMemoryAck);
      CMD_RET_SIZE(SendSignal);
      CMD_RET_SIZE(SendSignalAck);
      CMD_RET_SIZE(ContinueProcess);
      CMD_RET_SIZE(ContinueProcessAck);
      CMD_RET_SIZE(StepThread);
      CMD_RET_SIZE(StepThreadAck);
      CMD_RET_SIZE(SetBreakpoint);
      CMD_RET_SIZE(SetBreakpointAck);
      CMD_RET_SIZE(ResetBreakpoint);
      CMD_RET_SIZE(ResetBreakpointAck);
      CMD_RET_SIZE(SetWatchpoint);
      CMD_RET_SIZE(SetWatchpointAck);
      CMD_RET_SIZE(ResetWatchpoint);
      CMD_RET_SIZE(ResetWatchpointAck);
      CMD_RET_SIZE(RemoveTrapHandler);
      CMD_RET_SIZE(RemoveTrapHandlerAck);
      CMD_RET_SIZE(SetPreferences);
      CMD_RET_SIZE(SetPreferencesAck);
      CMD_RET_SIZE(FreeMemory);
      CMD_RET_SIZE(FreeMemoryAck);
      CMD_RET_SIZE(SetSpecialReg);
      CMD_RET_SIZE(SetSpecialRegAck);
      CMD_RET_SIZE(SetGeneralRegs);
      CMD_RET_SIZE(SetGeneralRegsAck);
      CMD_RET_SIZE(SetFloatRegs);
      CMD_RET_SIZE(SetFloatRegsAck);
      CMD_RET_SIZE(SetDebugRegs);
      CMD_RET_SIZE(SetDebugRegsAck);
      CMD_RET_SIZE(SetSpecialRegs);
      CMD_RET_SIZE(SetSpecialRegsAck);
      CMD_RET_SIZE(ReleaseControl);
      CMD_RET_SIZE(ReleaseControlAck);
      CMD_RET_SIZE(SetContinuationSignal);
      CMD_RET_SIZE(SetContinuationSignalAck);
      default:
         perr_printf("Unknown command type\n");
         assert(0);
         return 0;
   }
}

#define MESSAGE_RET_SIZE(X) case X: return sizeof(X ## Message)
#define MESSAGE_RET_CMDLIST_SIZE(X)                                     \
   case X: do {                                                         \
      const X ## Message &s = static_cast<const X ## Message &>(msg);   \
      assert(s.numCommands);                                            \
      const CommandDescriptor &last = s.cmdList[s.numCommands-1];       \
      return sizeof(X ## Message) + last.offset + last.length;          \
   } while (0)

uint32_t bgq_process::getMessageLength(const ToolMessage &msg)
{
   switch (msg.header.type) {
      MESSAGE_RET_SIZE(ErrorAck);
      MESSAGE_RET_SIZE(Attach);
      MESSAGE_RET_SIZE(AttachAck);
      MESSAGE_RET_SIZE(Detach);
      MESSAGE_RET_SIZE(DetachAck);
      MESSAGE_RET_CMDLIST_SIZE(Query);
      MESSAGE_RET_CMDLIST_SIZE(QueryAck);
      MESSAGE_RET_CMDLIST_SIZE(Update);
      MESSAGE_RET_CMDLIST_SIZE(UpdateAck);
      MESSAGE_RET_SIZE(SetupJob);
      MESSAGE_RET_SIZE(SetupJobAck);
      MESSAGE_RET_SIZE(Notify);
      MESSAGE_RET_SIZE(NotifyAck);
      MESSAGE_RET_SIZE(Control);
      MESSAGE_RET_SIZE(ControlAck);
      default:
         perr_printf("Unknown command type\n");
         assert(0);
   }
}

uint16_t bgq_process::getCommandMsgType(uint16_t cmd_id)
{
   switch (cmd_id) {
      case GetSpecialRegs:
      case GetGeneralRegs:
      case GetFloatRegs:
      case GetDebugRegs:
      case GetMemory:
      case GetThreadList:
      case GetAuxVectors:
      case GetProcessData:
      case GetThreadData:
      case GetPreferences:
      case GetFilenames:
      case GetFileStatData:
      case GetFileContents:
         return Query;
      case GetSpecialRegsAck:
      case GetGeneralRegsAck:
      case GetFloatRegsAck:
      case GetDebugRegsAck:
      case GetMemoryAck:
      case GetThreadListAck:
      case GetAuxVectorsAck:
      case GetProcessDataAck:
      case GetThreadDataAck:
      case GetPreferencesAck:
      case GetFilenamesAck:
      case GetFileStatDataAck:
      case GetFileContentsAck:
         return QueryAck;
      case SetGeneralReg:
      case SetFloatReg:
      case SetDebugReg:
      case SetMemory:
      case HoldThread:
      case ReleaseThread:
      case InstallTrapHandler:
      case AllocateMemory:
      case SendSignal:
      case ContinueProcess:
      case StepThread:
      case SetBreakpoint:
      case ResetBreakpoint:
      case SetWatchpoint:
      case ResetWatchpoint:
      case RemoveTrapHandler:
      case SetPreferences:
      case FreeMemory:
      case SetSpecialReg:
      case SetGeneralRegs:
      case SetFloatRegs:
      case SetDebugRegs:
      case SetSpecialRegs:
      case ReleaseControl:
      case SetContinuationSignal:
         return Update;
      case SetGeneralRegAck:
      case SetFloatRegAck:
      case SetDebugRegAck:
      case SetMemoryAck:
      case HoldThreadAck:
      case ReleaseThreadAck:
      case InstallTrapHandlerAck:
      case AllocateMemoryAck:
      case SendSignalAck:
      case ContinueProcessAck:
      case StepThreadAck:
      case SetBreakpointAck:
      case ResetBreakpointAck:
      case SetWatchpointAck:
      case ResetWatchpointAck:
      case RemoveTrapHandlerAck:
      case SetPreferencesAck:
      case FreeMemoryAck:
      case SetSpecialRegAck:
      case SetGeneralRegsAck:
      case SetFloatRegsAck:
      case SetDebugRegsAck:
      case SetSpecialRegsAck:
      case ReleaseControlAck:
      case SetContinuationSignalAck:
         return UpdateAck;
      default:
         perr_printf("Unknown command %u\n", (unsigned) cmd_id);
         assert(0);
         return 0;
   }
}

bool bgq_process::isActionCommand(uint16_t cmd_type)
{
   switch (cmd_type) {
      case StepThread:
      case ContinueProcess:
      case ReleaseControl:
         return true;
   }
   return false;
}

int_thread *int_thread::createThreadPlat(int_process *proc, 
                                         Dyninst::THR_ID thr_id, 
                                         Dyninst::LWP lwp_id,
                                         bool /*initial_thrd*/)
{
   bgq_thread *qthrd = new bgq_thread(proc, thr_id, lwp_id);
   assert(qthrd);
   return static_cast<int_thread *>(qthrd);
}

bgq_thread::bgq_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l) :
   thread_db_thread(p, t, l)
{
}

bgq_thread::~bgq_thread()
{
}

bool bgq_thread::plat_cont()
{
   int_thread *signal_thrd = NULL;
   int cont_signal = 0;
   set<int_thread *> ss_threads;
   bgq_process *proc = dynamic_cast<bgq_process *>(llproc());

   int_threadPool *tp = proc->threadPool();
   for (int_threadPool::iterator i = tp->begin(); i != tp->end(); i++) {
      bgq_thread *t = static_cast<bgq_thread *>(*i);
      if (t->last_signaled) {
         signal_thrd = t;
         t->last_signaled = false;
      }
      if (t->continueSig_) {
         cont_signal = t->continueSig_;
         t->continueSig_ = 0;
      }
      if (t->singleStep()) {
         ss_threads.insert(t);
      }
   }

   if (!signal_thrd) {
      signal_thrd = tp->initialThread();
   }

   pthrd_printf("Sending SetContinueSignalCmd to %d/%d with signal %d\n",
                proc->getPid(), signal_thrd->getLWP(), cont_signal);

   SetContinuationSignalCmd cont_signal_cmd;
   cont_signal_cmd.threadID = signal_thrd->getLWP();
   cont_signal_cmd.signum = cont_signal;
   bool result = proc->sendCommand(cont_signal_cmd, SetContinuationSignal);
   if (!result) {
      pthrd_printf("Error sending continuation signal\n");
      return false;
   }

   if (ss_threads.empty()) {
      pthrd_printf("Sending ContinueProcess command to %d\n", proc->getPid());
      proc->last_ss_thread = NULL;
      ContinueProcessCmd cont_process;
      cont_process.threadID = 0;
      bool result = proc->sendCommand(cont_process, ContinueProcess);
      if (!result) {
         pthrd_printf("Error sending continue process command\n");
         return false;
      }
      return true;
   }

   pthrd_printf("%lu threads of %d are in single step, selecting thread for single step\n",
                (unsigned long) ss_threads.size(), proc->getPid());
   int_thread *ss_thread = NULL;
   if (ss_threads.size() > 1 && proc->last_ss_thread) {
      /**
       * Multiple threads are single-stepping at once, but the BG/Q debug interface only allows us to 
       * step one at a time.  We'll remember what thread we stepped last and rotate the next thread
       * that gets stepped with each call.
       **/
      for (set<int_thread *>::iterator i = ss_threads.begin(); i != ss_threads.end(); i++) {
         if (*i != proc->last_ss_thread)
            continue;
         i++;
         if (i == ss_threads.end())
            i = ss_threads.begin();
         ss_thread = proc->last_ss_thread = *i;
         break;
      }
   }
   if (!ss_thread) {
      ss_thread = proc->last_ss_thread = *ss_threads.begin();
   }
   assert(ss_thread);

   StepThreadCmd step_thrd;
   step_thrd.threadID = ss_thread->getLWP();
   result = proc->sendCommand(step_thrd, StepThread);
   if (!result) {
      pthrd_printf("Error doing sendCommand of StepThread\n");
      return false;
   }
   return true;
}

bool bgq_thread::plat_stop()
{
   pthrd_printf("Signaling %d/%d with SIGSTOP\n", llproc()->getPid(), getLWP());
   
   SendSignalCmd send_stop;
   send_stop.threadID = getLWP();
   send_stop.signum = SIGSTOP;
   bgq_process *bgproc = dynamic_cast<bgq_process *>(llproc());

   bool result = bgproc->sendCommand(send_stop, SendSignal);
   if (!result) {
      pthrd_printf("Error doing sendCommand with SendSignal\n");
      return false;
   }

   bgproc->setDecoderPendingStop(true);

   return true;
}

#define GPR_TO_POOL(REGNUM) reg.regs[ppc64::r ## REGNUM] = gprs[REGNUM];
#define SPEC_TO_POOL(REGNAME) reg.regs[ppc64:: REGNAME] = sregs-> REGNAME;
#define SPEC_TO_POOL2(REGNAME1, REGNAME2) reg.regs[ppc64:: REGNAME1] = sregs-> REGNAME2;
void bgq_thread::regBufferToPool(BG_Reg_t *gprs, BG_Special_Regs *sregs, int_registerPool &reg)
{
   if (gprs) {
      GPR_TO_POOL(0);     GPR_TO_POOL(1);     GPR_TO_POOL(2);
      GPR_TO_POOL(3);     GPR_TO_POOL(4);     GPR_TO_POOL(5);
      GPR_TO_POOL(6);     GPR_TO_POOL(7);     GPR_TO_POOL(8);
      GPR_TO_POOL(9);     GPR_TO_POOL(10);    GPR_TO_POOL(11);
      GPR_TO_POOL(12);    GPR_TO_POOL(13);    GPR_TO_POOL(14);
      GPR_TO_POOL(15);    GPR_TO_POOL(16);    GPR_TO_POOL(17);
      GPR_TO_POOL(18);    GPR_TO_POOL(19);    GPR_TO_POOL(20);
      GPR_TO_POOL(21);    GPR_TO_POOL(22);    GPR_TO_POOL(23);
      GPR_TO_POOL(24);    GPR_TO_POOL(25);    GPR_TO_POOL(26);
      GPR_TO_POOL(27);    GPR_TO_POOL(28);    GPR_TO_POOL(29);
      GPR_TO_POOL(30);    GPR_TO_POOL(31);
   }
   if (sregs) {
      SPEC_TO_POOL2(pc, iar);                 SPEC_TO_POOL(lr);
      SPEC_TO_POOL(msr);                      SPEC_TO_POOL(cr);
      SPEC_TO_POOL(ctr);                      SPEC_TO_POOL(xer);
      SPEC_TO_POOL2(fpscw, fpscr);
   }
}

#define REG_TO_USER(R) \
   int_registerPool::const_iter i = regs.find(ppc64:: R);               \
   assert(i != regs.end();                                              \
   
#define NUM_REGS_IN_PT_REGS 44
void bgq_thread::regPoolToUser(const int_registerPool &pool_regs, void *user_area)
{
   static MachRegister regs[NUM_REGS_IN_PT_REGS];
   static bool regs_init = false;

   if (!regs_init) {
      //Define the order of regs in the user region by order in this array
      regs[0] = ppc64::r0;       regs[1] = ppc64::r1;         regs[2] = ppc64::r2;
      regs[3] = ppc64::r3;       regs[4] = ppc64::r4;         regs[5] = ppc64::r5;
      regs[6] = ppc64::r6;       regs[7] = ppc64::r7;         regs[8] = ppc64::r8;
      regs[9] = ppc64::r9;       regs[10] = ppc64::r10;       regs[11] = ppc64::r11;
      regs[12] = ppc64::r12;     regs[13] = ppc64::r13;       regs[14] = ppc64::r14;
      regs[15] = ppc64::r15;     regs[16] = ppc64::r16;       regs[17] = ppc64::r17;
      regs[18] = ppc64::r18;     regs[19] = ppc64::r19;       regs[20] = ppc64::r20;
      regs[21] = ppc64::r21;     regs[22] = ppc64::r22;       regs[23] = ppc64::r23;
      regs[24] = ppc64::r24;     regs[25] = ppc64::r25;       regs[26] = ppc64::r26;
      regs[27] = ppc64::r27;     regs[28] = ppc64::r28;       regs[29] = ppc64::r29;
      regs[30] = ppc64::r30;     regs[31] = ppc64::r31;        
      regs[32] = ppc64::pc;
      regs[33] = ppc64::msr;
      regs[34] = InvalidReg; //orig_gpr3
      regs[35] = ppc64::ctr;
      regs[36] = ppc64::lr;
      regs[37] = ppc64::xer;
      regs[38] = InvalidReg; //ccr
      regs[39] = InvalidReg; //softe
      regs[40] = ppc64::trap;
      regs[41] = ppc64::dar;
      regs[42] = ppc64::dsisr;
      regs[43] = InvalidReg; //result;
      regs_init = true;
   }

   for (unsigned i=0; i < NUM_REGS_IN_PT_REGS; i++) {
      MachRegister r = regs[i];
      uint64_t val = 0;
      if (r != InvalidReg) {
         int_registerPool::const_iterator j = pool_regs.regs.find(r);
         if (j != pool_regs.regs.end()) {
            val = j->second;
         }
      }
      ((uint64_t *) user_area)[i] = val;
   }
}

#define GPR_TO_BUFFER(REGNUM) gprs[REGNUM] = reg.regs[ppc64::r ## REGNUM];
#define SPEC_TO_BUFFER(REGNAME) sregs->REGNAME = reg.regs[ppc64:: REGNAME];
#define SPEC_TO_BUFFER2(REGNAME1, REGNAME2) sregs-> REGNAME2 = reg.regs[ppc64:: REGNAME1];
void bgq_thread::regPoolToBuffer(int_registerPool &reg, BG_Reg_t *gprs, BG_Special_Regs *sregs)
{
   GPR_TO_BUFFER(0);     GPR_TO_BUFFER(1);     GPR_TO_BUFFER(2);
   GPR_TO_BUFFER(3);     GPR_TO_BUFFER(4);     GPR_TO_BUFFER(5);
   GPR_TO_BUFFER(6);     GPR_TO_BUFFER(7);     GPR_TO_BUFFER(8);
   GPR_TO_BUFFER(9);     GPR_TO_BUFFER(10);    GPR_TO_BUFFER(11);
   GPR_TO_BUFFER(12);    GPR_TO_BUFFER(13);    GPR_TO_BUFFER(14);
   GPR_TO_BUFFER(15);    GPR_TO_BUFFER(16);    GPR_TO_BUFFER(17);
   GPR_TO_BUFFER(18);    GPR_TO_BUFFER(19);    GPR_TO_BUFFER(20);
   GPR_TO_BUFFER(21);    GPR_TO_BUFFER(22);    GPR_TO_BUFFER(23);
   GPR_TO_BUFFER(24);    GPR_TO_BUFFER(25);    GPR_TO_BUFFER(26);
   GPR_TO_BUFFER(27);    GPR_TO_BUFFER(28);    GPR_TO_BUFFER(29);
   GPR_TO_BUFFER(30);    GPR_TO_BUFFER(31);
   SPEC_TO_BUFFER2(pc, iar);                    SPEC_TO_BUFFER(lr);
   SPEC_TO_BUFFER(msr);                        SPEC_TO_BUFFER(cr);
   SPEC_TO_BUFFER(ctr);                        SPEC_TO_BUFFER(xer);
   SPEC_TO_BUFFER2(fpscw, fpscr);
}

bool bgq_thread::genRegToSystem(MachRegister reg, GeneralRegSelect &result) {
   if (!(reg.val() & ppc64::GPR))
      return false;

   result = (GeneralRegSelect) (reg.val() & 0x0000ffff);
   return true;
}

bool bgq_thread::specRegToSystem(MachRegister reg, SpecialRegSelect &result) {
#define CASE_REG(DNAME, QNAME) case ppc64::i ## DNAME : result = QNAME; return true
   switch (reg.val()) {
      CASE_REG(pc, iar);
      CASE_REG(lr, lr);
      CASE_REG(msr, msr);
      CASE_REG(cr, cr);
      CASE_REG(ctr, ctr);
      CASE_REG(xer, xer);
      CASE_REG(fpscw, fpscr);
   }
   return false;
}

bool bgq_thread::plat_getAllRegisters(int_registerPool &)
{
   assert(0); //Async only
   return false;
}

bool bgq_thread::plat_getRegister(Dyninst::MachRegister, Dyninst::MachRegisterVal &)
{
   assert(0); //Async only
   return false;
}

bool bgq_thread::plat_setAllRegisters(int_registerPool &)
{
   assert(0); //Async only
   return false;
}

bool bgq_thread::plat_setRegister(Dyninst::MachRegister, Dyninst::MachRegisterVal)
{
   assert(0); //Async only
   return false;
}

bool bgq_thread::attach()
{
   return true;
}

bool bgq_thread::plat_getAllRegistersAsync(allreg_response::ptr resp)
{
   GetGeneralRegsCmd get_gpr_regs;
   GetSpecialRegsCmd get_spec_regs;
   bgq_process *bgproc = dynamic_cast<bgq_process *>(llproc());

   get_gpr_regs.threadID = get_spec_regs.threadID = getLWP();
   
   pthrd_printf("getAllRegisters for %d/%d\n", llproc()->getPid(), getLWP());

   resp->markAsMultiResponse(2); //2 commands need to complete: gpr and spec

   bool result = bgproc->sendCommand(get_gpr_regs, GetGeneralRegs, resp);
   if (!result) {
      pthrd_printf("Error in sendCommand for GetGeneralRegs\n");
      return false;
   }

   result = bgproc->sendCommand(get_spec_regs, GetSpecialRegs, resp);
   if (!result) {
      pthrd_printf("Error in sendCommand for GetSpecialRegs\n");
      return false;
   }

   return true;
}

bool bgq_thread::plat_getRegisterAsync(Dyninst::MachRegister,
                                       reg_response::ptr)
{
   assert(0); //No individual reg access on this platform.
   return false;
}

bool bgq_thread::plat_setAllRegistersAsync(int_registerPool &pool,
                                           result_response::ptr resp)
{
   SetGeneralRegsCmd set_gen;
   SetSpecialRegsCmd set_spec;
   bgq_process *bgproc = dynamic_cast<bgq_process *>(llproc());

   pthrd_printf("setAllRegisters for %d/%d\n", llproc()->getPid(), getLWP());

   set_gen.threadID = set_spec.threadID = getLWP();
   regPoolToBuffer(pool, set_gen.gpr, &set_spec.sregs);

   resp->markAsMultiResponse(2);
   
   bool result = bgproc->sendCommand(set_gen, GetGeneralRegs, resp);
   if (!result) {
      pthrd_printf("Error in sendCommand for SetGeneralRegs\n");
      return false;
   }

   result = bgproc->sendCommand(set_spec, GetSpecialRegs, resp);
   if (!result) {
      pthrd_printf("Error in sendCommand for SetSpecialRegs\n");
      return false;
   }
   return true;
}

bool bgq_thread::plat_setRegisterAsync(Dyninst::MachRegister reg, Dyninst::MachRegisterVal val,
                                       result_response::ptr resp)
{
   GeneralRegSelect gen_reg;
   SpecialRegSelect spec_reg;
   bgq_process *bgproc = dynamic_cast<bgq_process *>(llproc());

   if (genRegToSystem(reg, gen_reg)) {
      pthrd_printf("Doing set of GPR %d to %lx on %d/%d\n", (int) gen_reg, val, llproc()->getPid(), getLWP());
      SetGeneralRegCmd set_reg;
      set_reg.threadID = getLWP();
      set_reg.reg_select = gen_reg;
      set_reg.value = val;
      bool result = bgproc->sendCommand(set_reg, SetGeneralReg, resp);
      if (!result) {
         pthrd_printf("Error doing sendCommand with SetGeneralReg\n");
         return false;
      }
      return true;
   }
   else if (specRegToSystem(reg, spec_reg)) {
      pthrd_printf("Doing set of spec #%d to %lx on %d/%d\n", (int) gen_reg, val, llproc()->getPid(), getLWP());
      SetSpecialRegCmd set_reg;
      set_reg.threadID = getLWP();
      set_reg.reg_select = spec_reg;
      set_reg.value = val;
      bool result = bgproc->sendCommand(set_reg, SetSpecialReg, resp);
      if (!result) {
         pthrd_printf("Error doing sendCommand with SetGeneralReg\n");
         return false;
      }
      return true;
   }

   setLastError(err_badparam, "Invalid register specified to setRegister");
   return false;
}

bool bgq_thread::plat_convertToSystemRegs(const int_registerPool &pool, unsigned char *output_buffer)
{
   bgq_thread::regPoolToUser(pool, output_buffer);
   return true;
}

map<int, ComputeNode *> ComputeNode::id_to_cn;
map<string, int> ComputeNode::socket_to_id;
set<ComputeNode *> ComputeNode::all_compute_nodes;

ComputeNode *ComputeNode::getComputeNodeByID(int cn_id)
{
   map<int, ComputeNode *>::iterator i = id_to_cn.find(cn_id);
   if (i != id_to_cn.end())
      return i->second;

   ComputeNode *new_cn = new ComputeNode(cn_id);
   if (!new_cn || new_cn->fd == -1) {
      pthrd_printf("Failed to create new ComputeNode object\n");
      if (new_cn) delete new_cn;
      return NULL;
   }
   id_to_cn[cn_id] = new_cn;
   return new_cn;
}

bool ComputeNode::constructSocketToID()
{
   char id_dir[64];
   char target_path[PATH_MAX];
   int dir_length;

   if (!socket_to_id.empty())
      return true;

   snprintf(id_dir, 64, "/jobs/%lu/toolctl_node", bgq_process::getJobID());
   
   DIR *dir = opendir(id_dir);
   if (!dir) {
      int error = errno;
      perr_printf("Could not open directory %s: %s\n", id_dir, strerror(error));
      return false;
   }

   dir_length = strlen(id_dir);
   id_dir[dir_length] = '/';
   dir_length++;
   id_dir[dir_length] = '\0';

   for (struct dirent *dent = readdir(dir); dent; dent = readdir(dir)) {
      int node_id = atoi(dent->d_name);

      strcpy(id_dir + dir_length, dent->d_name);
      char *target = realpath(id_dir, target_path);
      if (!target) {
         int error = errno;
         perr_printf("Could not resolve symbolic link for %s: %s\n", id_dir, strerror(error));
         closedir(dir);
         return false;
      }
      
      string target_str(target);
      map<string, int>::iterator i = socket_to_id.find(target_str);
      if (i != socket_to_id.end()) {
         perr_printf("Compute nodes %d and %d share a socket %s\n", node_id, i->second, target_str.c_str());
         closedir(dir);
         return false;
      }

      socket_to_id[target_str] = node_id;
   }

   closedir(dir);
   return true;
}

ComputeNode *ComputeNode::getComputeNodeByRank(uint32_t rank)
{
   char rank_path[64];
   char target_path[PATH_MAX];
   snprintf(rank_path, 64, "/jobs/%lu/toolctl_rank/%u", bgq_process::getJobID(), rank);

   char *target = realpath(rank_path, target_path);
   if (!target) {
      int error = errno;
      perr_printf("Error.  %s was not a symlink: %s\n", rank_path, strerror(error));
      return false;
   }

   string target_str(target);
   if (!constructSocketToID()) {
      pthrd_printf("Error constructing socket/id mapping.\n");
      return false;
   }

   map<string, int>::iterator i = socket_to_id.find(target_str);
   if (i == socket_to_id.end()) {
      perr_printf("Unable to find mapping for rank %d with socket %s\n", rank, target);
      return NULL;
   }

   return getComputeNodeByID(i->second);
}

ComputeNode::ComputeNode(int cid) :
   cn_id(cid)
{
   sockaddr_un saddr;
   int result;
   socklen_t socklen = sizeof(sockaddr_un);
   
   fd = socket(PF_UNIX, SOCK_STREAM, 0);
   if (fd == -1) {
      int error = errno;
      perr_printf("Error creating socket for CN %d: %s\n", cid, strerror(error));
      return;
   }

   bzero(&saddr, socklen);
   saddr.sun_family = AF_UNIX;
   snprintf(saddr.sun_path, sizeof(saddr.sun_path), "/jobs/%lu/toolctl_node/%d", bgq_process::getJobID(), cn_id);
   
   do {
      result = connect(fd, (struct sockaddr *) &saddr, socklen);
   } while (result == -1 && errno == EINTR);
   if (result == -1) {
      int error = errno;
      perr_printf("Error connecting to socket for CN %d: %s\n", cid, strerror(error));
      close(fd);
      fd = -1;
      return;
   }

   pthrd_printf("Successfully connected to CN %d\n", cid);
}

ComputeNode::~ComputeNode()
{
   if (fd != -1) {
      close(fd);
      fd = -1;
   }
   map<int, ComputeNode *>::iterator i = id_to_cn.find(cn_id);
   if (i != id_to_cn.end())
      id_to_cn.erase(i);
}

int ComputeNode::getFD() const
{
   return fd;
}

int ComputeNode::getID() const
{
   return cn_id;
}

bool ComputeNode::reliableWrite(void *buffer, size_t buffer_size)
{
   ScopeLock lock(send_lock);

   size_t bytes_written = 0;
   while (bytes_written < buffer_size) {
      int result = write(fd, ((char *) buffer) + bytes_written, buffer_size - bytes_written);
      if (result == -1 && errno == EINTR)
         continue;
      else if (result == -1 || result == 0) {
         int error = errno;
         perr_printf("Error writing to ComputeNode %d on FD %d: %s\n", cn_id, fd, strerror(error));
         return false;
      }
      bytes_written += result;
   }
   return true;
}

bool ComputeNode::writeToolMessage(bgq_process *proc, ToolMessage *msg, bool heap_alloced)
{
   if (proc->have_pending_message) {
      pair<void *, size_t> newmsg;
      newmsg.second = msg->header.length;
      if (heap_alloced) {
         newmsg.first = msg;
      }
      else {
         newmsg.first = malloc(newmsg.second);
         memcpy(newmsg.first, msg, newmsg.second);
      }
      proc->queued_pending_msgs.push(newmsg);
      return true;
   }

   bool result = reliableWrite(msg, msg->header.length);
   if (heap_alloced) {
      free(msg);
   }
   if (!result) {
      pthrd_printf("Failed to write message to %d\n", proc->getPid());
      return false;
   }
   
   proc->have_pending_message = true;
   return true;
}

bool ComputeNode::flushNextMessage(bgq_process *proc)
{
   if (proc->queued_pending_msgs.empty()) {
      proc->have_pending_message = false;
      return true;
   }

   pair<void *, size_t> buffer = proc->queued_pending_msgs.front();
   proc->queued_pending_msgs.pop();

   bool result = reliableWrite(buffer.first, buffer.second);
   free(buffer.first);
   if (!result) {
      pthrd_printf("Failed to flush message to %d\n", proc->getPid());
      proc->have_pending_message = false;
      return false;
   }
   proc->have_pending_message = true;
   return true;
}

bool ComputeNode::handleMessageAck(bgq_process *proc)
{
   assert(proc->have_pending_message);
   return flushNextMessage(proc);
}

bool ComputeNode::writeToolAttachMessage(bgq_process *proc, ToolMessage *msg, bool heap_alloced)
{
   if (all_attach_done || !do_all_attach) {
      return writeToolMessage(proc, msg, heap_alloced);
   }

   ScopeLock lock(attach_lock);

   if (!all_attach_done) {
      proc->have_pending_message = true;
   }
   return writeToolMessage(proc, msg, heap_alloced);
}

const std::set<ComputeNode *> &ComputeNode::allNodes()
{
   return all_compute_nodes;
}

HandleBGQStartup::HandleBGQStartup() :
   Handler(std::string("BGQ Startup"))
{
}

HandleBGQStartup::~HandleBGQStartup()
{
}

Handler::handler_ret_t HandleBGQStartup::handleEvent(Event::ptr ev)
{
   
   bgq_process *proc = dynamic_cast<bgq_process *>(ev->getProcess()->llproc());
   assert(proc);
   pthrd_printf("Handling int bootstrap for %d\n", proc->getPid());
   EventIntBootstrap::ptr int_bs = ev->getEventIntBootstrap();
   void *data = int_bs->getData();

   bool result = proc->handleStartupEvent(data);
   return result ? ret_success : ret_error;
}

int HandleBGQStartup::getPriority() const
{
   return Handler::PrePlatformPriority;
}

void HandleBGQStartup::getEventTypesHandled(std::vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::None, EventType::IntBootstrap));
}

const char *bgq_process::bgqErrorMessage(uint32_t rc, bool long_form)
{
   if ((int) rc < 0) {
      //Likely got an error in errorCode rather then returnCode, treat as errno
      return strerror((int) rc);
   }
   //Error Descriptions are taken from comments in IBM's MessageHeader.h
#define STR_CASE(X, Y) case X: return long_form ? #X  ": " #Y : #X
   switch ((ReturnCode) rc) {
      STR_CASE(Success, "Success (no error).");
      STR_CASE(WrongService, "Service value in message header is not valid.");
      STR_CASE(UnsupportedType, "Type value in message header is not supported.");
      STR_CASE(JobIdError, "Job id value is not valid.");
      STR_CASE(ProcessIdError, "Rank value is not valid.");
      STR_CASE(RequestFailed, "Requested operation failed.   ");
      STR_CASE(SubBlockJobError, "Sub-block job specifications are not valid.  ");
      STR_CASE(SendError, "Sending a message failed.");
      STR_CASE(RecvError, "Receiving a message failed.");
      STR_CASE(VersionMismatch, "Protocol versions do not match.");
      STR_CASE(NodeNotReady, "Compute node is not ready for requested operation.");
      STR_CASE(SecondaryGroupIdError, "Setting secondary group id failed.");
      STR_CASE(PrimaryGroupIdError, "Setting primary group id failed.");
      STR_CASE(UserIdError, "Setting user id failed.");
      STR_CASE(WorkingDirError, "Changing to working directory failed.");
      STR_CASE(AppOpenError, "Opening application executable failed.");
      STR_CASE(AppAuthorityError, "No authority to application executable.");
      STR_CASE(AppReadError, "Reading data from application executable failed.");
      STR_CASE(AppElfHeaderSize, "Application executable ELF header is wrong size.");
      STR_CASE(AppElfHeaderError, "Application executable ELF header contains invalid value.");
      STR_CASE(AppNoCodeSection, "Application executable contains no code sections.");
      STR_CASE(AppCodeSectionSize, "Application executable code section is too big.");
      STR_CASE(AppSegmentAlignment, "Application executable segment has wrong alignment.");
      STR_CASE(AppTooManySegments, "Application executable has too many segments.");
      STR_CASE(AppStaticTLBError, "Generating static TLB map for application failed.");
      STR_CASE(AppMemoryError, "Initializing memory for process failed.");
      STR_CASE(ArgumentListSize, "Argument list has too many items.");
      STR_CASE(ToolStartError, "Starting tool process failed.");
      STR_CASE(ToolAuthorityError, "No authority to tool executable.");
      STR_CASE(ToolIdError, "Tool id is not valid.");
      STR_CASE(ToolTimeoutExpired, "Timeout expired ending a tool.");
      STR_CASE(ToolPriorityConflict, "Tool priority conflict.");
      STR_CASE(ToolMaxAttachedExceeded, "Tool maximum number of tools exceeded.");
      STR_CASE(ToolIdConflict, "Tool id conflict.");
      STR_CASE(JobsDirError, "Creating /jobs directory failed.");
      STR_CASE(JobsObjectError, "Creating object in /jobs directory failed.");
      STR_CASE(ToolPriorityError, "Tool priority level is not valid.");
      STR_CASE(ToolRankNotFound, "Tool request could not find requested target process.");
      STR_CASE(CornerCoreError, "Corner core number is not valid.");
      STR_CASE(NumCoresInProcessError, "Number of cores allocated to a process is not valid.");
      STR_CASE(ProcessActive, "Process is currently active on a hardware thread.");
      STR_CASE(NumProcessesError, "Number of processes on node is not valid.");
      STR_CASE(RanksInJobError, "Number of active ranks in job is not valid.");
      STR_CASE(ClassRouteDataError, "Class route data is not valid.");
      STR_CASE(ToolNumberOfCmdsExceeded, "Tool number of commands is not valid.");
      STR_CASE(RequestIncomplete, "Requested operation was partially successful.");
      STR_CASE(PrologPgmStartError, "Starting job prolog program process failed.");
      STR_CASE(PrologPgmError, "Job prolog program failed. ");
      STR_CASE(EpilogPgmStartError, "Starting job epilog program process failed.");
      STR_CASE(EpilogPgmError, "Job epilog program failed. ");
      STR_CASE(RequestInProgress, "Requested operation is currently in progress.");
      STR_CASE(ToolControlConflict, "Control authority conflict with another tool.");
      STR_CASE(NodesInJobError, "No compute nodes matched job specifications.");
      STR_CASE(ToolConflictingCmds, "An invalid combination of commands was specifed in the command list.");
      STR_CASE(ReturnCodeListEnd, "End of the return code enumerations.");
   }
   return "Unknown Error Code";   
}

GeneratorBGQ::GeneratorBGQ() :
   GeneratorMT(std::string("BGQ Generator"))
{
   int result = pipe(kick_pipe);
   if (result == -1) {
      int error = errno;
      perr_printf("Error creating kick pipe: %s\n", strerror(error));
      setLastError(err_internal, "Could not create internal file descriptors\n");
      kick_pipe[0] = kick_pipe[1] = -1;
   }
}

GeneratorBGQ::~GeneratorBGQ()
{
}

bool GeneratorBGQ::initialize()
{
   if (kick_pipe[0] == -1 || kick_pipe[1] == -1) {
      pthrd_printf("Failed to initialize kick pipe ealier\n");
      return false;
   }
   return true;
}

bool GeneratorBGQ::canFastHandle()
{
   return false;
}

ArchEvent *GeneratorBGQ::getEvent(bool)
{
   assert(0); //BGQ uses multi-event interface
   return NULL;
}

bool GeneratorBGQ::getEvent(bool block, vector<ArchEvent *> &events)
{
   int max_fd = kick_pipe[0];

   fd_set read_set;
   FD_ZERO(&read_set);
   
   const set<ComputeNode *> &nodes = ComputeNode::allNodes();
   for (set<ComputeNode *>::const_iterator i = nodes.begin(); i != nodes.end(); i++) {
      int fd = (*i)->getFD();
      if (fd > max_fd)
         max_fd = fd;
      FD_SET(fd, &read_set);
   }
   FD_SET(kick_pipe[0], &read_set);

   struct timeval timeout;
   timeout.tv_sec = 0;
   timeout.tv_usec = 0;
   struct timeval *timeoutp = block ? NULL : &timeout;

   pthrd_printf("BGQ Generator is blocking in select");
   int result;
   do {
      result = select(max_fd+1, &read_set, NULL, NULL, timeoutp);
   } while (result == -1 && errno == EINTR);

   if (result == 0) {
      if (!block) {
         pthrd_printf("Return with no events after polling BGQ pipes\n");
         return true;
      }
      perr_printf("Error. Blocking select returned early with no FDs.\n");
      return false;
   }
   if (result == -1) {
      int error = errno;
      perr_printf("Select returned unexpected error: %s", strerror(error));
      return false;
   }

   if (FD_ISSET(kick_pipe[0], &read_set)) {
      int i;
      result = read(kick_pipe[0], &i, sizeof(int));
      if (result != -1) {
         assert(i == kick_val);
      }
   }

   for (set<ComputeNode *>::const_iterator i = nodes.begin(); i != nodes.end(); i++) {
      int fd = (*i)->getFD();
      if (!FD_ISSET(fd, &read_set)) {
         continue;
      }
      
      bool bresult = readMessage(fd, events);
      if (!bresult) {
         perr_printf("Could not read messages from ComputeNode %d\n", (*i)->getID());
         continue;
      }
   }

   return true;
}

bool GeneratorBGQ::reliableRead(int fd, void *buffer, size_t buffer_size)
{
   size_t bytes_read = 0;
   while (bytes_read < buffer_size) {
      int result = read(fd, ((char *) buffer) + bytes_read, buffer_size - bytes_read);

      if (result == -1 && errno == EINTR) {
         continue;
      }
      else if (result == -1) {
         int error = errno;
         perr_printf("Failed to read from FD %d: %s\n", fd, strerror(error));
         setLastError(err_internal, "Failed to read from CDTI file descriptor");
         return false;
      }
      else if (result == 0) {
         pthrd_printf("EOF on FD %d\n", fd);
         return false;
      }
      else {
         bytes_read += result;
      }
   }
   return true;
}

bool GeneratorBGQ::readMessage(int fd, vector<ArchEvent *> &events)
{
   int msgs_read = 0;

   for (;;) {
      //Read a message from the fd
      MessageHeader hdr;
      bool result = reliableRead(fd, &hdr, sizeof(MessageHeader));
      if (!result) {
         pthrd_printf("Could not read header from file descriptor\n");
         return false;
      }

      assert(hdr.length <= SmallMessageDataSize);
      char *message = (char *) malloc(hdr.length);
      memcpy(message, &hdr, sizeof(MessageHeader));

      result = reliableRead(fd, message + sizeof(MessageHeader), hdr.length - sizeof(MessageHeader));
      if (result == -1) {
         pthrd_printf("Could not read message from file descriptor\n");
         return false;
      }
      msgs_read++;

      //Create and save a new ArchEvent from this message
      ArchEventBGQ *newArchEvent = new ArchEventBGQ((ToolMessage *) message);
      assert(newArchEvent);
      events.push_back(newArchEvent);

      if (!read_multiple_msgs) {
         break;
      }

      //Poll to see if there are any more messages
      fd_set read_set;
      FD_ZERO(&read_set);
      FD_SET(fd, &read_set);
      struct timeval timeout;
      timeout.tv_sec = 0;
      timeout.tv_usec = 0;

      int iresult = select(fd+1, &read_set, NULL, NULL, &timeout);
      if (iresult == -1) {
         int error = errno;
         perr_printf("Non-fatal error polling FD %d for new messages: %s\n", fd, strerror(error));
         break;
      }
      if (iresult == 0) {
         //No more messages are available
         break;
      }
   }
   
   pthrd_printf("Read %d messages from fd %d\n", msgs_read, fd);
   return (msgs_read > 0);
}

void GeneratorBGQ::kick()
{
   int result;
   int kval = kick_val;
   do {
      result = write(kick_pipe[1], &kval, sizeof(int));
   } while (result == -1 && errno == EINTR);
}

static GeneratorBGQ *gen = NULL;
Generator *Generator::getDefaultGenerator()
{
   if (!gen) {
      gen = new GeneratorBGQ();
      assert(gen);
      gen->launch();
   }
   return static_cast<Generator *>(gen);
}

ArchEventBGQ::ArchEventBGQ(ToolMessage *m) :
   msg(m),
   free_msg(true)
{
}

ArchEventBGQ::~ArchEventBGQ()
{
   if (msg && free_msg)
      free(msg);
   msg = NULL;
}

ToolMessage *ArchEventBGQ::getMsg() const
{
   return msg;
}

void ArchEventBGQ::dontFreeMsg()
{
   free_msg = true;
}

DecoderBlueGeneQ::DecoderBlueGeneQ()
{
   response::set_ids_only = true;
}

DecoderBlueGeneQ::~DecoderBlueGeneQ()
{
}

Event::ptr DecoderBlueGeneQ::decodeCompletedResponse(response::ptr resp, set_response::ptr set_resp,
                                                     map<Event::ptr, EventAsync::ptr> &async_evs)
{
   set_resp->subResps(-1); //One fewer sub-response left

   if (resp->isMultiResponse() && !resp->isMultiResponseComplete())
      return Event::ptr();
   
   Event::ptr ev = resp->getEvent();
   if (!ev) {
      pthrd_printf("Marking response %s/%d ready\n", resp->name().c_str(), resp->getID());
      resp->markReady();
      return Event::ptr();
   }

   pthrd_printf("Creating new EventAsync over %s for response %s/%d\n",
                ev->name().c_str(), resp->name().c_str(), resp->getID());

   EventAsync::ptr async_ev;
   map<Event::ptr, EventAsync::ptr>::iterator i = async_evs.find(ev);
   if (i == async_evs.end()) {
      //Create a new EventAsync for this event.
      int_eventAsync *internal = new int_eventAsync(resp);
      async_ev = EventAsync::ptr(new EventAsync(internal));
      async_ev->setProcess(ev->getProcess());
      async_ev->setThread(ev->getThread());
      async_ev->setSyncType(Event::async);
      async_ev->addSubservientEvent(ev);
      async_evs[ev] = async_ev;
      return async_ev;
   }
   else {
      //Optimization - this event already has an EventAsync.  Just add it to the existing one.
      async_ev = i->second;
      int_eventAsync *internal = async_ev->getInternal();
      internal->addResp(resp);
      return Event::ptr();
   }
}

bool DecoderBlueGeneQ::decodeStartupEvent(ArchEventBGQ *ae, bgq_process *proc, void *data,
                                          Event::SyncType stype, vector<Event::ptr> &events)
{
   Event::ptr new_ev = EventIntBootstrap::ptr(new EventIntBootstrap(data));
   new_ev->setProcess(proc->proc());
   new_ev->setThread(Thread::ptr());
   new_ev->setSyncType(stype);
   ae->dontFreeMsg();
   events.push_back(new_ev);
   return true;
}

bool DecoderBlueGeneQ::decodeUpdateOrQueryAck(ArchEventBGQ *archevent, bgq_process *proc,
                                              int num_commands, CommandDescriptor *cmd_list, 
                                              vector<Event::ptr> &events)
{
   map<Event::ptr, EventAsync::ptr> async_ev_map;
   ToolMessage *msg = archevent->getMsg();
   uint32_t seq_id = msg->header.sequenceId;

   ScopeLock lock(getResponses().condvar());

   set_response::ptr set_resp = getResponses().rmResponse(seq_id)->getSetResponse();
   assert(set_resp);

   for (int i=0; i<num_commands; i++) 
   {
      if (lock.isLocked() && !set_resp->numSubResps())
         lock.unlock();

      CommandDescriptor *desc = cmd_list+i;
      ToolCommand *base_cmd = (ToolCommand *) (((char *) msg) + desc->offset);
      response::ptr cur_resp = set_resp->getResps()[i];

      BG_ThreadID_t threadID;
      int_thread *thr = proc->threadPool()->findThreadByLWP(threadID);

      switch (desc->type) {
         case GetSpecialRegsAck: 
            pthrd_printf("Decoding GetSpecialRegsAck on %d/%d\n", 
                         proc->getPid(), thr->getLWP());
            goto HandleGetRegsAck;
         case GetGeneralRegsAck: 
            pthrd_printf("Decoding GetGeneralRegsAck on %d/%d\n", 
                         proc->getPid(), thr->getLWP());
         HandleGetRegsAck: {
            allreg_response::ptr allreg = cur_resp->getAllRegResponse();
            int_registerPool *regpool = allreg->getRegPool();
            assert(allreg);
            assert(regpool);
            if (desc->type == GetSpecialRegsAck) {
               GetSpecialRegsAckCmd *cmd = static_cast<GetSpecialRegsAckCmd *>(base_cmd);
               bgq_thread::regBufferToPool(NULL, &cmd->sregs, *regpool);
            }
            else {
               GetGeneralRegsAckCmd *cmd = static_cast<GetGeneralRegsAckCmd *>(base_cmd);
               bgq_thread::regBufferToPool(cmd->gpr, NULL, *regpool);
            }
            allreg->postResponse();
            if (desc->returnCode)
               allreg->markError(desc->returnCode);
            
            Event::ptr new_ev = decodeCompletedResponse(allreg, set_resp, async_ev_map);
            if (new_ev)
               events.push_back(new_ev);
            break;
         }
         case GetFloatRegsAck:
         case GetDebugRegsAck:
            assert(0); //Currently unused
            break;
         case GetMemoryAck: {
            pthrd_printf("Decoding GetMemoryAck on %d\n", proc->getPid());
            mem_response::ptr memresp = cur_resp->getMemResponse();
            GetMemoryAckCmd *cmd = static_cast<GetMemoryAckCmd *>(base_cmd);
            assert(memresp);
            assert(cmd);
            memresp->postResponse((char *) cmd->data, cmd->length, cmd->addr);
            if (desc->returnCode)
               memresp->markError(desc->returnCode);

            Event::ptr new_ev = decodeCompletedResponse(memresp, set_resp, async_ev_map);
            if (new_ev)
               events.push_back(new_ev);
         }
         case GetThreadListAck:
            pthrd_printf("Decoded GetThreadListAck on %d/%d. Dropping\n", proc->getPid(), thr->getLWP());
            break;
         case GetAuxVectorsAck:
            pthrd_printf("Decoded GetAuxVectorsAck on %d/%d. Dropping\n", proc->getPid(), thr->getLWP());
            break;
         case GetProcessDataAck: {
            pthrd_printf("Decoded new event on %d to GetProcessData message\n", proc->getPid());
            decodeStartupEvent(archevent, proc, msg, Event::async, events);
            break;
         }
         case GetThreadDataAck:
         case GetPreferencesAck:
         case GetFilenamesAck:
         case GetFileStatDataAck:
         case GetFileContentsAck:
            assert(0); //Currently unused
            break;
         case SetBreakpointAck:
            pthrd_printf("Decoding SetBreakpointAck on %d\n", proc->getPid());
            goto HandleResResponse;
         case ResetBreakpointAck:
            pthrd_printf("Decoding ResetBreakpointAck on %d\n", proc->getPid());
            goto HandleResResponse;
         case SetGeneralRegAck:
            pthrd_printf("Decoding SetGeneralRegAck on %d/%d\n", proc->getPid(), thr->getLWP());
            goto HandleResResponse;
         case SetGeneralRegsAck:
            pthrd_printf("Decoding SetGeneralRegsAck on %d/%d\n", proc->getPid(), thr->getLWP());
            goto HandleResResponse;
         case SetSpecialRegAck:
            pthrd_printf("Decoding SetSpecialRegAck on %d/%d\n", proc->getPid(), thr->getLWP());
            goto HandleResResponse;
         case SetSpecialRegsAck:
            pthrd_printf("Decoding SetSpecialRegsAck on %d/%d\n", proc->getPid(), thr->getLWP());
            goto HandleResResponse;
         case SetMemoryAck: 
            pthrd_printf("Decoding SetMemoryAck on %d\n", proc->getPid());
         HandleResResponse: {                  
            result_response::ptr result_resp = cur_resp->getResultResponse();
            assert(result_resp);
            bool has_error = (desc->returnCode != 0);
            if (has_error) 
               result_resp->markError(desc->returnCode);
            result_resp->postResponse(!has_error);
            Event::ptr new_ev = decodeCompletedResponse(result_resp, set_resp, async_ev_map);
            if (new_ev)
               events.push_back(new_ev);
         }
         case SetFloatRegAck:
         case SetDebugRegAck:
            assert(0); //Currently unused
            break;
         case HoldThreadAck:
            pthrd_printf("Decoded HoldThreadAck on %d/%d. Dropping\n", proc->getPid(), thr->getLWP());
            break;
         case ReleaseThreadAck:
            pthrd_printf("Decoded ReleaseThreadAck on %d/%d. Dropping\n", proc->getPid(), thr->getLWP());
            break;
         case InstallTrapHandlerAck:
         case AllocateMemoryAck:
            assert(0); //Currently unused
            break;                  
         case SendSignalAck:
            pthrd_printf("Decoded SendSignalAck on %d/%d. Dropping\n", proc->getPid(), thr->getLWP());
            break;
         case ContinueProcessAck:
            pthrd_printf("Decoded ContinueProcessAck on %d/%d. Dropping\n", proc->getPid(), thr->getLWP());
            break;
         case StepThreadAck:
            pthrd_printf("Decoded StepThreadAck on %d/%d. Dropping\n", proc->getPid(), thr->getLWP());
            break;
         case SetWatchpointAck:
         case ResetWatchpointAck:
         case RemoveTrapHandlerAck:
         case SetPreferencesAck:
         case FreeMemoryAck:
         case SetFloatRegsAck:
         case SetDebugRegsAck:
            assert(0); //Currently unused
            break;
         case ReleaseControlAck:
            pthrd_printf("Decoded ReleaseControlAck on %d/%d. Dropping\n", proc->getPid(), thr->getLWP());
            break;
         case SetContinuationSignalAck:
            pthrd_printf("Decoded SetContinuationSignalAck on %d/%d. Dropping\n", proc->getPid(), thr->getLWP());
            break;
         default:
            break;
      }
   }

   return true;
}

bool DecoderBlueGeneQ::decodeGenericSignal(ArchEventBGQ *, bgq_process *proc, int_thread *thr,
                                           int signum, vector<Event::ptr> &events)
{
   Event::ptr new_ev = EventSignal::ptr(new EventSignal(signum));
   new_ev->setProcess(proc->proc());
   new_ev->setThread(thr->thread());
   new_ev->setSyncType(Event::sync_process);
   events.push_back(new_ev);
   return true;
}

bool DecoderBlueGeneQ::decodeBreakpoint(ArchEventBGQ *archevent, bgq_process *proc, int_thread *thr,
                                        Address addr, vector<Event::ptr> &events)
{
   if (rpcMgr()->isRPCTrap(thr, addr)) {
      pthrd_printf("Decoded event to rpc completion on %d/%d at %lx\n", proc->getPid(), thr->getLWP(), addr);
      Event::ptr rpc_event = EventRPC::ptr(new EventRPC(thr->runningRPC()->getWrapperForDecode()));
      rpc_event->setProcess(proc->proc());
      rpc_event->setThread(thr->thread());
      rpc_event->setSyncType(Event::sync_process);
      events.push_back(rpc_event);
      return true;
   }
   installed_breakpoint *ibp = proc->getBreakpoint(addr);
   if (ibp) {
      pthrd_printf("Decoded breakpoint on %d/%d at %lx\n", proc->getPid(), thr->getLWP(), addr);
      
      EventBreakpoint::ptr bp_event = EventBreakpoint::ptr(new EventBreakpoint(new int_eventBreakpoint(addr, ibp, thr)));
      bp_event->setProcess(proc->proc());
      bp_event->setThread(thr->thread());
      bp_event->setSyncType(Event::sync_process);

      if (addr == proc->getLibBreakpointAddr()) {
         pthrd_printf("Breakpoint is library load/unload\n");
         Event::ptr lib_event = EventLibrary::ptr(new EventLibrary());
         lib_event->setProcess(proc->proc());
         lib_event->setThread(thr->thread());
         lib_event->setSyncType(Event::sync_process);
         bp_event->addSubservientEvent(lib_event);
      }
      
      async_ret_t result = proc->decodeTdbBreakpoint(bp_event);
      if (result == aret_error) {
         //Not really an error, just how we say that it isn't
         // a TDB breakpoint.
      }
      else if (result == aret_success) {
         //decodeTdbBreakpoint added a subservient event if this hits
         pthrd_printf("Breakpoint was thread event\n");
      }
      else if (result == aret_async) {
         assert(0); //New implementation of decodeTdbBreakpoint doesn't return async
      }

      events.push_back(bp_event);
      return true;
   }

   pthrd_printf("Decoded to SIGTRAP signal (Warning - this is frequently a bug).\n");
   return decodeGenericSignal(archevent, proc, thr, SIGTRAP, events);
}

bool DecoderBlueGeneQ::decodeStop(ArchEventBGQ *archevent, bgq_process *proc, int_thread *thr,
                                 vector<Event::ptr> &events)
{
   if ((int) proc->startup_state >= (int) bgq_process::issue_control_request &&
       (int) proc->startup_state <= (int) bgq_process::waitfor_control_request_notice)
   {
      pthrd_printf("Decoded event to startup control signal\n");
      return decodeStartupEvent(archevent, proc, archevent->getMsg(), Event::sync_process, events);
   }

   if (proc->decoderPendingStop()) {
      pthrd_printf("Decoded to Stop event\n");
      Event::ptr new_ev = EventStop::ptr(new EventStop());
      new_ev->setProcess(proc->proc());
      new_ev->setThread(thr->thread());
      new_ev->setSyncType(Event::sync_process);
      proc->setDecoderPendingStop(false);
      events.push_back(new_ev);
      return true;
   }

   pthrd_printf("Decoded to generic SIGSTOP signal (Warning: this may be a bug)\n");
   return decodeGenericSignal(archevent, proc, thr, SIGSTOP, events);
}

bool DecoderBlueGeneQ::decodeStep(ArchEventBGQ *, bgq_process *proc,
                                  int_thread *thr, vector<Event::ptr> &events)
{
   assert(thr->singleStep());
   
   installed_breakpoint *ibp = thr->isClearingBreakpoint();
   if (ibp) {
      pthrd_printf("Decoded to breakpoint cleanup\n");
      Event::ptr new_ev = EventBreakpointRestore::ptr(new EventBreakpointRestore(new int_eventBreakpointRestore(ibp)));
      new_ev->setProcess(proc->proc());
      new_ev->setThread(thr->thread());
      new_ev->setSyncType(Event::sync_process);
      events.push_back(new_ev);
      return true;
   }
   else {
      pthrd_printf("Decoded to single step\n");
      Event::ptr new_ev = EventSingleStep::ptr(new EventSingleStep);
      new_ev->setProcess(proc->proc());
      new_ev->setThread(thr->thread());
      new_ev->setSyncType(Event::sync_process);
      events.push_back(new_ev);
      return true;      
   }
}

bool DecoderBlueGeneQ::decodeSignal(ArchEventBGQ *archevent, bgq_process *proc,
                                    vector<Event::ptr> &events)
{
   NotifyMessage *msg = static_cast<NotifyMessage *>(archevent->getMsg());
   uint32_t sig = msg->type.signal.signum;
   BG_Addr_t insn_addr = msg->type.signal.instAddress;
   BG_ThreadID_t threadID = msg->type.signal.threadID;

   int_thread *thr = proc->threadPool()->findThreadByLWP(threadID);
   assert(thr);
   
   pthrd_printf("Decoding signal on %d/%d at 0x%lx\n", proc->getPid(), thr->getLWP(), insn_addr);
   switch (msg->type.control.reason) {
      case NotifySignal_Generic:
         switch (sig) {
            case SIGSTOP:
               pthrd_printf("Decoding SIGSTOP\n");
               return decodeStop(archevent, proc, thr, events);
            case SIGTRAP:
               pthrd_printf("Decoding SIGTRAP\n");
               return decodeBreakpoint(archevent, proc, thr, insn_addr, events);
            default:
               pthrd_printf("Decoding generic signal %d\n", sig);
               return decodeGenericSignal(archevent, proc, thr, sig, events);
         }
         break;
      case NotifySignal_Breakpoint:
         pthrd_printf("Decoding breakpoint\n");
         return decodeBreakpoint(archevent, proc, thr, insn_addr, events);
      case NotifySignal_WatchpointRead:
      case NotifySignal_WatchpointWrite:
         assert(0); //Currently unused
         return false;
      case NotifySignal_StepComplete:
         pthrd_printf("Decoded single step\n");
         return decodeStep(archevent, proc, thr, events);
      default:
         assert(0);
         return false;
   }
}

bool DecoderBlueGeneQ::decodeExit(ArchEventBGQ *archevent, bgq_process *proc, vector<Event::ptr> &events)
{
   int exitStatus = static_cast<NotifyMessage *>(archevent->getMsg())->type.termination.exitStatus;
   pthrd_printf("Decoded to termination with exitStatus 0x%x\n", exitStatus);
   int signum = (0x00ff0000 & exitStatus) >> 16;
   int exitcode = (0x0000ff00 & exitStatus) >> 8;

   Event::ptr new_ev;
   if (signum) {
      pthrd_printf("Decoded to crash with signal %d\n", signum);
      new_ev = EventCrash::ptr(new EventCrash(signum));
   }
   else {
      pthrd_printf("Decoded to exit with code %d\n", exitcode);
      new_ev = EventExit::ptr(new EventExit(EventType::Post, exitcode));
   }

   int_thread *initial_thread = proc->threadPool()->initialThread();
   new_ev->setThread(initial_thread->thread());
   new_ev->setProcess(proc->proc());
   new_ev->setSyncType(Event::sync_process);
   events.push_back(new_ev);
   return true;
}

bool DecoderBlueGeneQ::decodeControlNotify(ArchEventBGQ *archevent, bgq_process *proc, vector<Event::ptr> &events)
{
   if ((int) proc->startup_state >= (int) bgq_process::issue_control_request &&
       (int) proc->startup_state <= (int) bgq_process::waitfor_control_request_notice)
   {
      pthrd_printf("Decoded event to startup control request notice\n");
      return decodeStartupEvent(archevent, proc, archevent->getMsg(), Event::async, events);
   }
   else {
#warning TODO implement notify
      assert(0); //Not yet implemented
      return false;
   }
}

bool DecoderBlueGeneQ::decodeNotifyMessage(ArchEventBGQ *archevent, bgq_process *proc,
                                           vector<Event::ptr> &events)
{
   NotifyMessage *msg = static_cast<NotifyMessage *>(archevent->getMsg());
   
   switch (msg->notifyMessageType) {
      case NotifyMessageType_Signal:
         return decodeSignal(archevent, proc, events);
         break;
      case NotifyMessageType_Termination:
         return decodeExit(archevent, proc, events);
         break;
      case NotifyMessageType_Control:
         return decodeControlNotify(archevent, proc, events);
         break;
      default:
         assert(0);
         return false;
   }
}

bool DecoderBlueGeneQ::decode(ArchEvent *ae, vector<Event::ptr> &events)
{
   ArchEventBGQ *archevent = static_cast<ArchEventBGQ *>(ae);
   ToolMessage *msg = archevent->getMsg();
   struct MessageHeader *header = &msg->header;

   int_process *proc = ProcPool()->findProcByPid(header->rank);
   bgq_process *qproc = dynamic_cast<bgq_process *>(proc);

   assert(header->service == ToolctlService);
   assert(header->jobId == bgq_process::getJobID());
   assert(proc);

   if (proc && header->type != Notify) {
      qproc->getComputeNode()->handleMessageAck(qproc);
   }

   switch (header->type) {
      case ControlAck:
         pthrd_printf("Decoding ControlAck on %d\n", proc->getPid());
         return decodeStartupEvent(archevent, qproc, msg, Event::async, events);
      case AttachAck:
         pthrd_printf("Decoding AttachAck on %d\n", proc->getPid());
         return decodeStartupEvent(archevent, qproc, msg, Event::async, events);
      case DetachAck:
         //TODO: Implement
         assert(0);
         break;
      case QueryAck: {
         uint16_t num_commands = static_cast<QueryAckMessage *>(msg)->numCommands;
         CommandDescriptor *cmd_list = static_cast<QueryAckMessage *>(msg)->cmdList;
         pthrd_printf("Decoding QueryAck from %d with %d commands\n", proc->getPid(), num_commands);
         return decodeUpdateOrQueryAck(archevent, qproc, num_commands, cmd_list, events);
      }
      case UpdateAck: {
         uint16_t num_commands = static_cast<UpdateAckMessage *>(msg)->numCommands;
         CommandDescriptor *cmd_list = static_cast<UpdateAckMessage *>(msg)->cmdList;
         pthrd_printf("Decoding UpdateAck from %d with %d commands\n", proc->getPid(), num_commands);
         return decodeUpdateOrQueryAck(archevent, qproc, num_commands, cmd_list, events);
      }
      case Notify:
         pthrd_printf("Decoding Notify on %d\n", proc->getPid());
         return decodeNotifyMessage(archevent, qproc, events);
      case SetupJobAck:
      case NotifyAck:
      case Attach:
      case Detach:
      case Query:
      case Update:
      case SetupJob:
      case Control:
      default:
         perr_printf("Unexpected ControlMessage from process %d of type %d\n", proc->getPid(), (int) header->type);
         assert(0);
   }
   return true;
}

HandlerPool *plat_createDefaultHandlerPool(HandlerPool *hpool)
{
   static bool init = false;
   static HandleBGQStartup *handle_startup = NULL;
   if (!init) {
      handle_startup = new HandleBGQStartup();
      init = true;
   }
   hpool->addHandler(handle_startup);
   thread_db_process::addThreadDBHandlers(hpool);
   sysv_process::addSysVHandlers(hpool);
   return hpool;
}

bool ProcessPool::LWPIDsAreUnique()
{
   return false;
}


