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

#include "proccontrol/src/bluegeneq.h"
#include "proccontrol/src/int_event.h"
#include "proccontrol/src/irpc.h"
#include "proccontrol/h/PCProcess.h"
#include "proccontrol/h/PlatFeatures.h"
#include "proccontrol/h/PCErrors.h"
#include "proccontrol/h/Mailbox.h"

using namespace Dyninst;
using namespace ProcControlAPI;
using namespace std;

#include "symlite/h/SymLite-elf.h"

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
#include <ucontext.h>
#include <elf.h>
#include <link.h>
#include <execinfo.h>

using namespace bgq;
using namespace std;

static void registerSignalHandlers(bool enable);

uint64_t bgq_process::jobid = 0;
uint32_t bgq_process::toolid = 0;
bool bgq_process::set_ids = false;
bool bgq_process::do_all_attach = true;

const char *getCommandName(uint16_t cmd_type);

static void printMessage(ToolMessage *msg, const char *str)
{
   if (!dyninst_debug_proccontrol)
      return;
   MessageHeader *header = &msg->header;
  
   pthrd_printf("%s MessageHeader:\n"
                "\tservice = %u\t"
                "\tversion = %u\t"
                "\ttype = %u\t"
                "\trank = %u\n"
                "\tsequenceId = %u\t"
                "\treturnCode = %u\t"
                "\terrCode = %u\t"
                "\tlength = %u\n"
                "\tjobID = %lu\t"
                "\ttoolId = %u\n\t",
                str,
                (unsigned) header->service,
                (unsigned) header->version,
                (unsigned) header->type,
                header->rank,
                header->sequenceId,
                header->returnCode,
                header->errorCode,
                header->length,
                (unsigned long) header->jobId,
                (unsigned) msg->toolId);
  
   switch (msg->header.type) {
      case ControlAck: {
         ControlAckMessage *cak = static_cast<ControlAckMessage *>(msg);
         pclean_printf("ControlAck: controllingToolId - %u, toolTag - %.8s, priority - %u\n", 
                       (unsigned) cak->controllingToolId, cak->toolTag, (unsigned) cak->priority);
         break;
      }
      case AttachAck: {
         AttachAckMessage *aak = static_cast<AttachAckMessage *>(msg);
         pclean_printf("AttachAck: numProcess - %u, rank = [", (unsigned) aak->numProcess);
         for (unsigned i=0; i<aak->numProcess; i++) {
            pclean_printf("%u,", aak->rank[i]);
         }
         pclean_printf("]\n");
         break;
      }
      case DetachAck: {
         DetachAckMessage *dak = static_cast<DetachAckMessage *>(msg);
         pclean_printf("DetachAck: numProcess - %u, rank = [", (unsigned) dak->numProcess);
         for (unsigned i=0; i<dak->numProcess; i++) {
            pclean_printf("%u, ", dak->rank[i]);
         }
         pclean_printf("]\n");
         break;
      }
      case QueryAck: {
         QueryAckMessage *m = static_cast<QueryAckMessage *>(msg);
         pclean_printf("QueryAck: numCommands = %u\n", (unsigned) m->numCommands);
         for (unsigned i=0; i<(unsigned) m->numCommands; i++) {
            CommandDescriptor &c = m->cmdList[i];
            pclean_printf("\t[%u] type = %s, reserved = %u, offset = %u, length = %u, returnCode = %u\n",
                          i, getCommandName(c.type), (unsigned) c.reserved, c.offset,
                          c.length, c.returnCode);
         }
         break;
      }
      case UpdateAck: {
         UpdateAckMessage *m = static_cast<UpdateAckMessage *>(msg);
         pclean_printf("UpdateAck: numCommands = %u\n", (unsigned) m->numCommands);
         for (unsigned i=0; i<(unsigned) m->numCommands; i++) {
            CommandDescriptor &c = m->cmdList[i];
            pclean_printf("\t[%u] type = %s, reserved = %u, offset = %u, length = %u, returnCode = %u\n",
                          i, getCommandName(c.type), (unsigned) c.reserved, c.offset,
                          c.length, c.returnCode);
         }
         break;
      }
      case Query: {
         QueryMessage *m = static_cast<QueryMessage *>(msg);
         pclean_printf("Query: numCommands = %u\n", (unsigned) m->numCommands);
         for (unsigned i=0; i<(unsigned) m->numCommands; i++) {
            CommandDescriptor &c = m->cmdList[i];
            pclean_printf("\t[%u] type = %s, reserved = %u, offset = %u, length = %u, returnCode = %u\n",
                          i, getCommandName(c.type), (unsigned) c.reserved, c.offset,
                          c.length, c.returnCode);
         }
         break;
      }
      case Update: {
         UpdateMessage *m = static_cast<UpdateMessage *>(msg);
         pclean_printf("Update: numCommands = %u\n", (unsigned) m->numCommands);
         for (unsigned i=0; i<(unsigned) m->numCommands; i++) {
            CommandDescriptor &c = m->cmdList[i];
            pclean_printf("\t[%u] type = %s, reserved = %u, offset = %u, length = %u, returnCode = %u\n",
                          i, getCommandName(c.type), (unsigned) c.reserved, c.offset,
                          c.length, c.returnCode);
         }
         break;
      }
      case Notify: {
         pclean_printf("Notify ");
         NotifyMessage *no = static_cast<NotifyMessage *>(msg);
         switch (no->notifyMessageType) {
            case NotifyMessageType_Signal:
               pclean_printf("Signal: signum = %u, threadId = %u, instAddress = 0x%lx, dataAddress = %lx, reason = %u\n",
                             no->type.signal.signum, (unsigned) no->type.signal.threadID, no->type.signal.instAddress,
                             no->type.signal.dataAddress, (unsigned) no->type.signal.reason);
               break;
            case NotifyMessageType_Termination:
               pclean_printf("Exit: exitStatus = %u\n", no->type.termination.exitStatus);
               break;
            case NotifyMessageType_Control:
               pclean_printf("Control: toolid = %u, toolTag = %.8s, priority = %u, reason = %u\n",
                             no->type.control.toolid, no->type.control.toolTag, (unsigned) no->type.control.priority,
                             (unsigned) no->type.control.reason);
               break;
         }
         break;
      }
      case SetupJobAck: {
         pclean_printf("SetupJobAck\n");
         break;
      }
      case NotifyAck: {
         pclean_printf("NotifyAck\n");
         break;
      }
      case Attach: {
         AttachMessage *am = static_cast<AttachMessage *>(msg);
         pclean_printf("AttachMessage: toolTag = %.8s, priority = %u, procSelect = %u\n",
                       am->toolTag, (unsigned) am->priority, (unsigned) am->procSelect);
         break;
      }
      case Detach: {
         DetachMessage *dm = static_cast<DetachMessage *>(msg);
         pclean_printf("DetachMessage: procSelect = %u\n", (unsigned) dm->procSelect);
         break;
      }
      case SetupJob: {
         pclean_printf("SetupJobMessage\n");
         break;
      }
      case Control: {
         ControlMessage *cm = static_cast<ControlMessage *>(msg);
         pclean_printf("ControlMessage: notifySet - %lx, sendSignal = %u, dynamicNotifyMode = %u, dacTrapMode = %u\n",
                       cm->notifySet, cm->sndSignal, (unsigned) cm->dynamicNotifyMode, (unsigned) cm->dacTrapMode);
         break;
      }
      default:
         pclean_printf("Unknown message\n");
         break;
   }
}

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
   is_doing_temp_detach(false),
   rank(pid),
   page_size(0),
   interp_base(0),
   initial_thread_list(NULL),
   priority(0),
   mtool(NULL),
   startup_state(issue_attach),
   detach_state(no_detach_issued)
{
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

   query_transaction = new QueryTransaction(this, Query);
   update_transaction = new UpdateTransaction(this, Update);
   cn = ComputeNode::getComputeNodeByRank(rank);
   cn->procs.insert(this);
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
   if (mtool) {
      delete mtool;
      mtool = NULL;
   }
   cn->removeNode(this);
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

bool bgq_process::plat_detach(result_response::ptr)
{
   if (detach_state == no_detach_issued) {
      pthrd_printf("plat_detach for %d is resuming all threads\n", getPid());
      int_threadPool *tp = threadPool();
      for (int_threadPool::iterator i = tp->begin(); i != tp->end(); i++) {
         resumeThread(*i);
      }
      
      //Record whether this is a temporary detach so that we can maintain
      // the state if we throw a new detach event.
      Event::ptr cur_event = handlerPool()->curEvent();
      assert(cur_event);
      EventDetach::ptr ev_detach = cur_event->getEventDetach();
      assert(ev_detach);
      is_doing_temp_detach = ev_detach->getInternal()->temporary_detach;

      detach_state = issue_control_release;
   }

   if (detach_state == issue_control_release && !hasControlAuthority) {
      pthrd_printf("plat_detach: Process %d doesn't have control authority, skipping release.\n", getPid());
      detach_state = choose_detach_mechanism;
   }

   if (detach_state == issue_control_release) {
      pthrd_printf("Releasing control of process %d\n", getPid());
      detach_state = control_release_sent;

      ReleaseControlCmd release;
      release.notify = ReleaseControlNotify_Inactive;
      release.threadID = 0;
      bool result = sendCommand(release, ReleaseControl);
      if (!result) {
         perr_printf("Error writing release control tool message\n");
         return false;
      }

      return true;
   }

   if (detach_state == control_release_sent) {
      pthrd_printf("Handling release control ack to %d.\n", getPid());
      hasControlAuthority = false;
      detach_state = choose_detach_mechanism;
   }

   if (detach_state == choose_detach_mechanism) {
      bool everyone_detaching = true;
      bool is_last_detacher = true;
      
      { 
         ScopeLock lock_this_scope(cn->detach_lock);
         
         for (set<bgq_process *>::iterator i = cn->procs.begin(); i != cn->procs.end(); i++) {
            bgq_process *peer_proc = *i;
            if (peer_proc == this)
               continue;
            
            int_thread *peer_thread = peer_proc->threadPool()->initialThread();
            if (!peer_thread->getDetachState().isDesynced()) {
               pthrd_printf("Process %d is not detaching.  %d not doing group detach\n", 
                            peer_proc->getPid(), getPid());
               everyone_detaching = false;
            }
            if (peer_proc->detach_state != waitfor_all_detach) {
               pthrd_printf("Process %d hasn't yet completed detach.  Postponing any group detach from %d\n",
                            peer_proc->getPid(), getPid());
               is_last_detacher = false;
            }
         }
      }

      if (everyone_detaching && !is_last_detacher) {
         pthrd_printf("Process %d will wait for another proc to do all detach\n", getPid());
         detach_state = waitfor_all_detach;
         return true;
      }

      DetachMessage detach;
      if (everyone_detaching) {
         pthrd_printf("Process %d issuing all-detach for compute node %d\n",
                      getPid(), cn->getID());
         detach_state = issued_all_detach;
         detach.procSelect = RanksInNode;
      }
      else {
         pthrd_printf("Issuing targeted detach to process %d\n", getPid());
         detach_state = issued_detach;
         detach.procSelect = RankInHeader;
      }
      fillInToolMessage(detach, Detach);
      bool result = cn->writeToolMessage(this, &detach, false);
      if (!result) {
         perr_printf("Error writing detach tool message\n");
         return false;
      }       
   
      return true;
   }

   if (detach_state == issued_all_detach || detach_state == issued_detach) {
      pthrd_printf("Handling detach response to %d.\n", getPid());
      detach_state = detach_cleanup;
   }
   else if (detach_state == waitfor_all_detach) {
      pthrd_printf("Finished waiting for all detach on %d\n", getPid());
      detach_state = detach_cleanup;
   }

   if (detach_state == detach_cleanup) {
      getComputeNode()->removeNode(this);
      detach_state = detach_done;
      return true;
   }

   perr_printf("Unreachable code.  Bad detach state on %d: %lu\n",
               getPid(), (unsigned long) detach_state);
   assert(0);
   return false;
}

bool bgq_process::plat_detachDone()
{
   if (detach_state != detach_done)
      return false;

   //We're done.  Reset the detach states
   detach_state = no_detach_issued;
   is_doing_temp_detach = false;
   return true;
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
      
   uint32_t len = get_auxvectors_result.length;
   Elf64_auxv_t *auxvs = (Elf64_auxv_t *) get_auxvectors_result.data;

   for (uint32_t i = 0; i < len; i++) {
      if (auxvs[i].a_type == AT_BASE) {
         base = interp_base = (Address) auxvs[i].a_un.a_val;
         interp_base_set = true;
         return true;
      }
   }
   
   pthrd_printf("Didn't find interpreter base for %d. Statically linked\n", getPid());
   pthrd_printf("%u auxv elements\n", len);
   for (uint32_t i = 0; i < len; i += 2) {
      pclean_printf("\t%lu - 0x%lx\n", get_auxvectors_result.data[i], get_auxvectors_result.data[i+1]);
   }
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

   uint32_t len = get_auxvectors_result.length;
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

Dyninst::Address bgq_process::plat_mallocExecMemory(Dyninst::Address addr, unsigned int)
{
   return get_procdata_result.heapStartAddr + addr;
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

bool bgq_process::plat_writeMem(int_thread *, const void *, Dyninst::Address, size_t, bp_write_t)
{
   assert(0); //No synchronous IO
   return false;
}

bool bgq_process::plat_needsAsyncIO() const
{
   return true;
}

bool bgq_process::plat_preAsyncWait()
{
   return rotateTransaction();
}

bool bgq_process::plat_preHandleEvent()
{
   return true;
}

bool bgq_process::plat_postHandleEvent()
{
   return rotateTransaction();
}

bool bgq_process::rotateTransaction()
{
   bool result;
   pthrd_printf("Flushing and beginning transactions for %d\n", getPid());
   if (query_transaction->activeTransaction()) {
      pthrd_printf("Ending query transaction\n");
      result = query_transaction->endTransaction();

      pthrd_printf("Beginning new query transaction\n");
      query_transaction->beginTransaction();

      if (!result) {
         perr_printf("Error flushing query transaction\n");
         return false;
      }
   }

   if (update_transaction->activeTransaction()) {
      pthrd_printf("Ending update transaction\n");
      result = update_transaction->endTransaction();

      pthrd_printf("Beginning new update transaction\n");
      update_transaction->beginTransaction();

      if (!result) {
         perr_printf("Error flushing update transaction\n");
         return false;
      }
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
                                    size_t size, result_response::ptr resp, int_thread *thr, bp_write_t bp_write)
{
   if (bp_write == int_process::bp_install) {
      pthrd_printf("Writing breakpoint memory %lx +%lu on %d with response ID %d\n",
                   addr, (unsigned long) size, getPid(), resp->getID());
      SetBreakpointCmd setbp;
      setbp.threadID = thr ? thr->getLWP() : 0;
      setbp.addr = addr;
      setbp.instruction = *((int *) local);
      bool result = sendCommand(setbp, SetBreakpoint, resp);
      if (!result) {
         pthrd_printf("Error sending SetBreakpoint\n");
         return false;
      }
      return true;
   }
   if (bp_write == int_process::bp_clear) {
      pthrd_printf("Reset breakpoint memory %lx +%lu on %d with response ID %d\n",
                   addr, (unsigned long) size, getPid(), resp->getID());
      ResetBreakpointCmd setbp;
      setbp.threadID = thr ? thr->getLWP() : 0;
      setbp.addr = addr;
      setbp.instruction = *((int *) local);
      bool result = sendCommand(setbp, ResetBreakpoint, resp);
      if (!result) {
         pthrd_printf("Error sending ResetBreakpoint\n");
         return false;
      }
      return true;
   }

   pthrd_printf("Writing memory %lx +%lu on %d with response ID %d\n", 
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
      SetMemoryCmd *setmem = (SetMemoryCmd *) malloc(sizeof(SetMemoryCmd) + write_size + 1);
      setmem->threadID = thr ? thr->getLWP() : 0;
      setmem->addr = addr + cur;
      setmem->length = write_size;
      setmem->specAccess = thr ? SpecAccess_UseThreadState : SpecAccess_ForceNonSpeculative;
      setmem->sharedMemoryAccess = SharedMemoryAccess_Allow;
      memcpy(setmem->data, ((const char *) local) + cur, write_size);

      bool result = sendCommand(*setmem, SetMemory, resp);
      free(setmem);
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
                                     size_t size, result_response::ptr result, bp_write_t bp_write)
{
   return internal_writeMem(thr, local, addr, size, result, NULL, bp_write);
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

void bgq_process::noteNewDequeuedEvent(Event::ptr ev)
{
   if (ev->getSyncType() == Event::sync_process) {
      debugger_suspended = true;
   }
}

void bgq_process::plat_threadAttachDone()
{
   pthrd_printf("Adding bootstrap event to mailbox\n");
   EventBootstrap::ptr ev = EventBootstrap::ptr(new EventBootstrap());
   int_thread *thrd = threadPool()->initialThread();
   ev->setProcess(proc());
   ev->setThread(thrd->thread());

   int_thread::State new_state = int_thread::none;
   Event::SyncType new_ev_sync_state = Event::unset;

   if (priority != QueryPriorityLevel) {
      new_ev_sync_state = Event::async;
      new_state = int_thread::stopped;
   }
   else {
      new_ev_sync_state = Event::async;
      new_state = int_thread::running;
   }

   for (int_threadPool::iterator i = threadPool()->begin(); i != threadPool()->end(); i++) {
      int_thread *thr = *i;
      thr->getGeneratorState().setState(new_state);
      thr->getHandlerState().setState(new_state);
      thr->getUserState().setState(new_state);
   }
   ev->setSyncType(new_ev_sync_state);

   getStartupTeardownProcs().inc();
      
   mbox()->enqueue(ev);
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
         pthrd_printf("Doing all attach\n");
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
      getStartupTeardownProcs().inc();
      setForceGeneratorBlock(true);
      query_transaction->beginTransaction();
      update_transaction->beginTransaction();
      tooltag = MultiToolControl::getDefaultToolName();
      priority = (uint8_t) MultiToolControl::getDefaultToolPriority();
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
      strncpy(attach.toolTag, tooltag.c_str(), sizeof(attach.toolTag));
      attach.priority = priority;
      pthrd_printf("Sending attach with name %s, priority %u\n", tooltag.c_str(), attach.priority);
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
      bool attach_retcode = attach_ack->header.returnCode;
      free(attach_ack);
      attach_ack = NULL;
      
      if (cn->issued_all_attach) {
         ScopeLock lock(cn->attach_lock);
         cn->all_attach_done = true;
         set<ToolMessage *>::iterator i;
         
         if (attach_retcode != Success) {
            perr_printf("Group attach returned error %d: %s\n", (int) attach_retcode,
                        bgq_process::bgqErrorMessage(attach_retcode));
            setState(int_process::errorstate);
            cn->all_attach_error = true;
            for (set<bgq_process *>::iterator i = cn->procs.begin(); i != cn->procs.end(); i++) {
               (*i)->setState(int_process::errorstate);
            }
            return false;
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
      uint16_t controllingToolId = ctrl_ack->controllingToolId;
      char toolTag[ToolTagSize];
      memcpy(toolTag, ctrl_ack->toolTag, ToolTagSize);
      uint8_t tool_priority = ctrl_ack->priority;
      uint32_t ctrl_retcode = ctrl_ack->header.returnCode;
      free(ctrl_ack);
      ctrl_ack = NULL;

      if (ctrl_retcode != Success) {
         perr_printf("Error return in ControlAckMessage: %s\n",
                     bgqErrorMessage(ctrl_retcode));
         setLastError(err_internal, "Could not send ControlMessage\n");
         return false;                     
      }
      if (controllingToolId != bgq_process::getToolID()) {
         pthrd_printf("Tool '%s' with ID %d has authority at priority %d\n", 
                      toolTag, controllingToolId, tool_priority);
         if (tool_priority > priority) {
            pthrd_printf("WARNING: Tool %s has higher priority (%d > %d), running in Query-Only mode\n",
                         toolTag, tool_priority, priority);
            startup_state = issue_data_collection;
         }
         else {
            pthrd_printf("Waiting for control priority release from %s (%d < %d)\n",
                         toolTag, tool_priority, priority);
            startup_state = waitfor_control_request_notice;
            return true;
         }
      }
      else {
         pthrd_printf("ProcControlAPI is only tool, taking control authority\n");
         hasControlAuthority = true;
         startup_state = waitfor_control_request_signal;
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
         hasControlAuthority = true;
         startup_state = waitfor_control_request_signal;
      }
      else if (notify_msg->type.control.reason == NotifyControl_Conflict) {
         pthrd_printf("Conflict message from control request, moving to data collection\n");
         pthrd_printf("Conflicting tag is %s, id = %u, priority = %u\n", 
                      notify_msg->type.control.toolTag,
                      notify_msg->type.control.toolid,
                      notify_msg->type.control.priority);
         startup_state = skip_control_request_signal;
      }
      else
         assert(0);
      free(notify_msg);
   }

   if (startup_state == waitfor_control_request_signal) {
      pthrd_printf("Received attach SIGNAL\n");
      NotifyMessage *notify_msg = static_cast<NotifyMessage *>(data);
      assert(notify_msg->type.signal.reason == NotifySignal_Generic);
      assert(notify_msg->type.signal.signum == SIGSTOP);

      int_thread *thrd = threadPool()->initialThread();
      assert(thrd);
      thrd->getStartupState().desyncStateProc(int_thread::stopped);
      
      startup_state = issue_data_collection;
      free(notify_msg);
   }
   if (startup_state == skip_control_request_signal) {
      int_thread *thrd = threadPool()->initialThread();
      assert(thrd);
      thrd->getStartupState().desyncStateProc(int_thread::running);
      startup_state = issue_data_collection;
   }

   /**
    * Send a set of query in a single CommandMessage.  We want:
    *  - GetProcessData
    *  - AuxVectors
    *  - ThreadList
    **/
   if (startup_state == issue_data_collection) {
      pthrd_printf("Issuing data request\n");

      GetProcessDataCmd procdata_cmd;
      procdata_cmd.threadID = 0;
      sendCommand(procdata_cmd, GetProcessData);

      GetAuxVectorsCmd auxv_cmd;
      auxv_cmd.threadID = 0;
      sendCommand(auxv_cmd, GetAuxVectors);

      GetThreadListCmd thread_list;
      thread_list.threadID = 0;
      sendCommand(thread_list, GetThreadList);

      startup_state = waitfor_data_collection;
      return true;
   }

   /**
    * Receive the data from the above query command.
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
      startup_state = data_collected;
   }

   if (startup_state == data_collected) {
      startup_state = startup_done;
      setState(running);
      getStartupTeardownProcs().dec();

      int_thread *initial_thread = threadPool()->initialThread();
      assert(initial_thread_list->numthreads > 0);
      initial_thread->changeLWP(initial_thread_list->threadlist[0].tid);
      
      return true;
   }

   if (startup_state == startup_done) {
      int_thread *thrd = threadPool()->initialThread();
      thrd->getStartupState().restoreStateProc();
      getStartupTeardownProcs().dec();
      setForceGeneratorBlock(false);
      pthrd_printf("Startup done on %d\n", getPid());
   }

   return true;
}

void bgq_process::fillInToolMessage(ToolMessage &msg, uint16_t msg_type, response::ptr resp)
{
   msg.header.service = ToolctlService;
   msg.header.version = ProtocolVersion;
   msg.header.type = msg_type;
   msg.header.rank = getRank();
   if (resp) {
      ResponseSet *resp_set = new ResponseSet();
      msg.header.sequenceId = resp_set->getID();
      resp_set->addID(resp->getID(), 0);
   }
   else {
      msg.header.sequenceId = 0;
   }
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

std::string bgq_process::mtool_getName()
{
   return tooltag;
}

MultiToolControl::priority_t bgq_process::mtool_getPriority()
{
   return priority;
}

MultiToolControl *bgq_process::mtool_getMultiToolControl()
{
   if (!mtool) {
      mtool = new MultiToolControl(proc());
   }
   return mtool;
}

bool bgq_process::plat_waitAndHandleForProc()
{
   pthrd_printf("Rotating transactions at top of plat_waitAndHandleForProc\n");
   rotateTransaction();
   return true;
}

bool bgq_process::sendCommand(const ToolCommand &cmd, uint16_t cmd_type, response::ptr resp, unsigned int resp_mod)
{
   uint16_t msg_type = getCommandMsgType(cmd_type);
   if (msg_type == Query) {
      return query_transaction->writeCommand(&cmd, cmd_type, resp, resp_mod);
   }
   else if (msg_type == Update) {
      return update_transaction->writeCommand(&cmd, cmd_type, resp, resp_mod);
   }
   else {
      assert(0); //Are we trying to send an Ack?
   }
   return false;
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

bool bgq_process::plat_getStackInfo(int_thread *thr, stack_response::ptr stk_resp)
{
   GetThreadDataCmd thr_cmd;

   thr_cmd.threadID = thr->getLWP();

   bool result = sendCommand(thr_cmd, GetThreadData, stk_resp);
   if (!result) {
      setLastError(err_internal, "Error while asking for thread data\n");
      perr_printf("Could not send GetThreadData to %d/%d\n", 
                  thr->llproc()->getPid(), thr->getLWP());
      return false;
   }
   
   //Needed to track to know when to free the ArchEventBGQ objects
   // associated with the stackwalks.
   num_pending_stackwalks++;
   return true;
}

bool bgq_process::plat_handleStackInfo(stack_response::ptr stk_resp, CallStackCallback *cbs)
{
   GetThreadDataAckCmd *thrdata = static_cast<GetThreadDataAckCmd *>(stk_resp->getData());
   int_thread *t = stk_resp->getThread();
   assert(stk_resp);
   assert(t);
   Thread::ptr thr = t->thread();
   bool had_error = false, result;
   
   assert(num_pending_stackwalks > 0);
   num_pending_stackwalks--;

   if (stk_resp->hasError()) {
      perr_printf("Error getting thread data on %d/%d", t->llproc()->getPid(), thr->getLWP());
      setLastError(err_internal, "Error receiving thread data\n");
      had_error = true;
      goto done;
   }

   pthrd_printf("Handling response with call stack %d/%d\n", t->llproc()->getPid(), t->getLWP());

   result = cbs->beginStackWalk(thr);
   if (!result) {
      pthrd_printf("User choose not to receive call stack data for %d/%d\n",
                   t->llproc()->getPid(), t->getLWP());
      goto done;
   }

   if (cbs->top_first) {
      for (signed int i=thrdata->numStackFrames-1; i >= 0; i--) {
         result = cbs->addStackFrame(thr, thrdata->stackInfo[i].savedLR, thrdata->stackInfo[i].frameAddr, 0);
         if (!result)
            break;
      }
   }
   else {
      for (unsigned int i=0; i < thrdata->numStackFrames; i++) {
         result = cbs->addStackFrame(thr, thrdata->stackInfo[i].savedLR, thrdata->stackInfo[i].frameAddr, 0);
         if (!result)
            break;
      }
   }
   if (!result) {
      pthrd_printf("User choose not to continue receive call stack data for %d/%d\n",
                   t->llproc()->getPid(), t->getLWP());
      goto done;
   }

   cbs->endStackWalk(thr);
  done:
   if (!num_pending_stackwalks) {
      for (set<void *>::iterator i = held_msgs.begin(); i != held_msgs.end(); i++) {
         free(*i);
      }
      held_msgs.clear();
   }
   return !had_error;
}

set<void *> bgq_process::held_msgs;
unsigned int bgq_process::num_pending_stackwalks = 0;

int_thread *int_thread::createThreadPlat(int_process *proc, 
                                         Dyninst::THR_ID thr_id, 
                                         Dyninst::LWP lwp_id,
                                         bool /*initial_thrd*/)
{
   bgq_thread *qthrd = new bgq_thread(proc, thr_id, lwp_id);
   assert(qthrd);

   map<int, int> &states = proc->getProcDesyncdStates();
   int myid = StartupStateID;
   map<int, int>::iterator i = states.find(myid);
   if (i != states.end()) {
      int_thread::State ns = proc->threadPool()->initialThread()->getStartupState().getState();      
      for (int j = 0; j < i->second; j++) {
         qthrd->getStartupState().desyncState(ns);
      }
   }

   return static_cast<int_thread *>(qthrd);
}

bgq_thread::bgq_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l) :
   int_thread(p, t, l),
   thread_db_thread(p, t, l),
   ppc_thread(p, t, l),
   last_signaled(false),
   unwinder(NULL)
{
}

bgq_thread::~bgq_thread()
{
   if (unwinder) {
      delete unwinder;
      unwinder = NULL;
   }
}

bool bgq_thread::plat_cont()
{
   int_thread *signal_thrd = NULL;
   int cont_signal = 0;
   set<int_thread *> ss_threads;
   bgq_process *proc = dynamic_cast<bgq_process *>(llproc());

   proc->debugger_suspended = false;

   int_threadPool *tp = proc->threadPool();
   for (int_threadPool::iterator i = tp->begin(); i != tp->end(); i++) {
      bgq_thread *t = dynamic_cast<bgq_thread *>(*i);
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

#define REG_TO_USER(R)                                      \
   int_registerPool::const_iter i = regs.find(ppc64:: R);   \
   assert(i != regs.end();                                  \
   
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

   bool result = bgproc->sendCommand(get_gpr_regs, GetGeneralRegs, resp, 0);
   if (!result) {
      pthrd_printf("Error in sendCommand for GetGeneralRegs\n");
      return false;
   }

   result = bgproc->sendCommand(get_spec_regs, GetSpecialRegs, resp, 1);
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
   
   bool result = bgproc->sendCommand(set_gen, SetGeneralRegs, resp, 0);
   if (!result) {
      pthrd_printf("Error in sendCommand for SetGeneralRegs\n");
      return false;
   }

   result = bgproc->sendCommand(set_spec, SetSpecialRegs, resp, 1);
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

bool bgq_thread::plat_convertToSystemRegs(const int_registerPool &pool, unsigned char *output_buffer, bool gpr_only)
{
   bgq_thread::regPoolToUser(pool, output_buffer);
   return true;
}

CallStackUnwinding *bgq_thread::getStackUnwinder()
{
   if (!unwinder) {
      unwinder = new CallStackUnwinding(thread());
   }
   return unwinder;
}

map<int, ComputeNode *> ComputeNode::id_to_cn;
map<string, int> ComputeNode::socket_to_id;
set<ComputeNode *> ComputeNode::all_compute_nodes;

ComputeNode *ComputeNode::getComputeNodeByID(int cn_id)
{
   map<int, ComputeNode *>::iterator i = id_to_cn.find(cn_id);
   if (i != id_to_cn.end())
      return i->second;

   pthrd_printf("Constructing new compute node for id: %d\n", cn_id);
   ComputeNode *new_cn = new ComputeNode(cn_id);
   if (!new_cn || new_cn->fd == -1) {
      perr_printf("Failed to create new ComputeNode object\n");
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
   
   pthrd_printf("Opening %s to parse links\n", id_dir);
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
      if (dent->d_name[0] == '.')
         continue;
      int node_id = atoi(dent->d_name);
      pthrd_printf("Parsing link %d\n", node_id);

      strcpy(id_dir + dir_length, dent->d_name);
      char *target = realpath(id_dir, target_path);
      if (!target) {
         int error = errno;
         perr_printf("Could not resolve symbolic link for %s: %s\n", id_dir, strerror(error));
         closedir(dir);
         return false;
      }
      pthrd_printf("Real path of %d is %s\n", node_id, id_dir);
      
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
      return NULL;
   }

   string target_str(target);
   if (!constructSocketToID()) {
      pthrd_printf("Error constructing socket/id mapping.\n");
      return NULL;
   }

   map<string, int>::iterator i = socket_to_id.find(target_str);
   if (i == socket_to_id.end()) {
      perr_printf("Unable to find mapping for rank %d with socket %s\n", rank, target);
      return NULL;
   }

   return getComputeNodeByID(i->second);
}

ComputeNode::ComputeNode(int cid) :
   cn_id(cid),
   do_all_attach(bgq_process::do_all_attach),
   issued_all_attach(false),
   all_attach_done(false),
   all_attach_error(false),
   have_pending_message(false)
{
   sockaddr_un saddr;
   int result;
   socklen_t socklen = sizeof(sockaddr_un);
   
   pthrd_printf("Constructing new ComputeNode %d\n", cid);
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

   all_compute_nodes.insert(this);
   pthrd_printf("Successfully connected to CN %d on FD %d\n", cid, fd);
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

   set<ComputeNode *>::iterator j = all_compute_nodes.find(this);
   assert(j != all_compute_nodes.end());
   all_compute_nodes.erase(j);
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
   ScopeLock lock(send_lock);

   if (have_pending_message) {
      pthrd_printf("Enqueuing message while another is pending\n");
      pair<void *, size_t> newmsg;
      newmsg.second = msg->header.length;
      if (heap_alloced) {
         newmsg.first = msg;
      }
      else {
         newmsg.first = malloc(newmsg.second);
         memcpy(newmsg.first, msg, newmsg.second);
      }
      queued_pending_msgs.push(newmsg);
      return true;
   }

   printMessage(msg, "Writing");
   bool result = reliableWrite(msg, msg->header.length);
   pthrd_printf("Wrote message of size %u to FD %d, result is %s\n", msg->header.length,
                fd, result ? "true" : "false");
   if (heap_alloced) {
      free(msg);
   }
   if (!result) {
      pthrd_printf("Failed to write message to %d\n", proc->getPid());
      return false;
   }
   
   have_pending_message = true;
   return true;
}

bool ComputeNode::flushNextMessage()
{
   ScopeLock lock(send_lock);
   assert(have_pending_message);

   if (queued_pending_msgs.empty()) {
      have_pending_message = false;
      return true;
   }

   pair<void *, size_t> buffer = queued_pending_msgs.front();
   queued_pending_msgs.pop();

   
   ToolMessage *msg = (ToolMessage *) buffer.first;
   printMessage(msg, "Writing");
   bool result = reliableWrite(msg, buffer.second);
   pthrd_printf("Flush wrote message of size %u to FD %d, result is %s\n", msg->header.length,
                fd, result ? "true" : "false");
   free(buffer.first);
   if (!result) {
      pthrd_printf("Failed to flush message to compute-node %d\n", cn_id);
      have_pending_message = false;
      return false;
   }
   return true;
}

bool ComputeNode::handleMessageAck()
{
   return flushNextMessage();
}

bool ComputeNode::writeToolAttachMessage(bgq_process *proc, ToolMessage *msg, bool heap_alloced)
{
   if (all_attach_done || !do_all_attach) {
      return writeToolMessage(proc, msg, heap_alloced);
   }

   ScopeLock lock(attach_lock);

   if (!all_attach_done) {
      have_pending_message = true;
   }
   return writeToolMessage(proc, msg, heap_alloced);
}

void ComputeNode::removeNode(bgq_process *proc) {
   set<bgq_process *>::iterator i = procs.find(proc);
   if (i == procs.end())
      return;

   procs.erase(i);
   issued_all_attach = false;
   all_attach_done = false;
   all_attach_error = false;
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
   void *data = NULL;
   if (int_bs)
      data = int_bs->getData();

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
   etypes.push_back(EventType(EventType::None, EventType::Bootstrap));
}

const char *bgq_process::bgqErrorMessage(uint32_t rc, bool long_form)
{
   if ((int) rc < 0) {
      //Likely got an error in errorCode rather then returnCode, treat as errno
      return strerror((int) rc);
   }
   //Error Descriptions are taken from comments in IBM's MessageHeader.h
#undef STR_CASE
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
      STR_CASE(ReturnCodeReserved1, "Formerly request in progress.");
      STR_CASE(ToolControlConflict, "Control authority conflict with another tool.");
      STR_CASE(NodesInJobError, "No compute nodes matched job specifications.");
      STR_CASE(ToolConflictingCmds, "An invalid combination of commands was specifed in the command list.");
      STR_CASE(ToolControlRequired, "Tool control authority is required.");
      STR_CASE(ToolControlReleaseRequired, "Tool release of control authority is required.");
      STR_CASE(AppInvalidorMissingMapfile, "Map file is missing or contains invalid values.");
      STR_CASE(ToolProcessExiting, "Tool message not handled because target process is exiting.");
      STR_CASE(ReturnCodeListEnd, "End of the return code enumerations.");
   }
   return "Unknown Error Code";   
}

GeneratorBGQ::GeneratorBGQ() :
   GeneratorMT(std::string("BGQ Generator"))
{
   if (dyninst_debug_proccontrol) {
      pthrd_printf("Enabling debug thread\n");
      DebugThread::getDebugThread();
   }
   int result = pipe(kick_pipe);
   if (result == -1) {
      int error = errno;
      perr_printf("Error creating kick pipe: %s\n", strerror(error));
      globalSetLastError(err_internal, "Could not create internal file descriptors\n");
      kick_pipe[0] = kick_pipe[1] = -1;
   }
   decoders.insert(new DecoderBlueGeneQ());
}

GeneratorBGQ::~GeneratorBGQ()
{
   kick();
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

bool GeneratorBGQ::getMultiEvent(bool block, vector<ArchEvent *> &events)
{
   bool have_fd = false;
   int max_fd = kick_pipe[0];

   fd_set read_set;
   FD_ZERO(&read_set);
   
   const set<ComputeNode *> &nodes = ComputeNode::allNodes();
   std::string fd_list_str;
   for (set<ComputeNode *>::const_iterator i = nodes.begin(); i != nodes.end(); i++) {
      int fd = (*i)->getFD();
      if (fd > max_fd)
         max_fd = fd;
      FD_SET(fd, &read_set);
      have_fd  = true;
      if (dyninst_debug_proccontrol) {
         char buffer[32];
         if (!fd_list_str.empty())
            fd_list_str += ", ";
         snprintf(buffer, 32, "%d", fd);
         fd_list_str += buffer;
      }
   }
   assert(have_fd);
   FD_SET(kick_pipe[0], &read_set);

   struct timeval timeout;
   timeout.tv_sec = 0;
   timeout.tv_usec = 0;
   struct timeval *timeoutp = block ? NULL : &timeout;

   pthrd_printf("BGQ Generator is blocking on file descriptors: %s\n", fd_list_str.c_str());
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
      return false;
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

bool GeneratorBGQ::reliableRead(int fd, void *buffer, size_t buffer_size, int timeout_s)
{
   size_t bytes_read = 0;
   while (bytes_read < buffer_size) {
      if (timeout_s > 0) {
         fd_set readfds;
         struct timeval timeout;
         FD_ZERO(&readfds);
         FD_SET(fd, &readfds);
         int nfds = fd+1;
         timeout.tv_sec = timeout_s;
         timeout.tv_usec = 0;
         int result = select(nfds, &readfds, NULL, NULL, &timeout);
         if (result == 0) {
            pthrd_printf("Timed out in reliableRead\n");
            return false;
         }
      }

      int result = read(fd, ((char *) buffer) + bytes_read, buffer_size - bytes_read);

      if (result == -1 && errno == EINTR) {
         continue;
      }
      else if (result == -1) {
         int error = errno;
         perr_printf("Failed to read from FD %d: %s\n", fd, strerror(error));
         globalSetLastError(err_internal, "Failed to read from CDTI file descriptor");
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

      printMessage((ToolMessage *) message, "Read");
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

bool GeneratorBGQ::plat_skipGeneratorBlock()
{
   bool result = getResponses().hasAsyncPending(false);
   if (result) {
      pthrd_printf("Async events pending, skipping generator block\n");
   }
   return result;
}

void GeneratorBGQ::kick()
{
   int result;
   int kval = kick_val;
   do {
      result = write(kick_pipe[1], &kval, sizeof(int));
   } while (result == -1 && errno == EINTR);
}

extern void GeneratorInternalJoin(GeneratorMTInternals *);
void GeneratorBGQ::shutdown()
{
   setState(Generator::exiting);
   kick();

   bool result = ProcPool()->condvar()->trylock();
   if (result) {
      ProcPool()->condvar()->signal();
      ProcPool()->condvar()->unlock();
   } 
   sleep(1);
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
   if (msg && free_msg) {
      free(msg);
   }
   msg = NULL;
}

ToolMessage *ArchEventBGQ::getMsg() const
{
   return msg;
}

void ArchEventBGQ::dontFreeMsg()
{
   free_msg = false;
}

DecoderBlueGeneQ::DecoderBlueGeneQ()
{
}

DecoderBlueGeneQ::~DecoderBlueGeneQ()
{
}

unsigned DecoderBlueGeneQ::getPriority() const 
{
   return Decoder::default_priority;
}

Event::ptr DecoderBlueGeneQ::decodeCompletedResponse(response::ptr resp,
                                                     map<Event::ptr, EventAsync::ptr> &async_evs)
{
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

Event::ptr DecoderBlueGeneQ::createEventDetach(bgq_process *proc, bool err)
{
   EventDetach::ptr new_ev = EventDetach::ptr(new EventDetach());
   new_ev->setProcess(proc->proc());
   new_ev->setThread(Thread::ptr());
   new_ev->setSyncType(Event::async);
   new_ev->getInternal()->temporary_detach = proc->is_doing_temp_detach;
   new_ev->getInternal()->removed_bps = true;
   new_ev->getInternal()->done = false;
   new_ev->getInternal()->had_error = err;
   return new_ev;
}

bool DecoderBlueGeneQ::decodeReleaseControlAck(ArchEventBGQ *, bgq_process *proc, 
                                               int err_code, std::vector<Event::ptr> &events)
{
   if (proc->detach_state == bgq_process::control_release_sent) {
      pthrd_printf("This ControlReleaseAck is part of a detach\n");
      Event::ptr new_ev = createEventDetach(proc, err_code != 0);
      events.push_back(new_ev);
      return true;
   }
   assert(0);
   return false;
}

bool DecoderBlueGeneQ::decodeUpdateOrQueryAck(ArchEventBGQ *archevent, bgq_process *proc,
                                              int num_commands, CommandDescriptor *cmd_list, 
                                              vector<Event::ptr> &events)
{
   map<Event::ptr, EventAsync::ptr> async_ev_map;
   ToolMessage *msg = archevent->getMsg();
   uint32_t seq_id = msg->header.sequenceId;
   bool resp_lock_held = false;

   ResponseSet *resp_set = NULL;
   if (seq_id) {
      resp_set = ResponseSet::getResponseSetByID(seq_id);
      if (resp_set)
         pthrd_printf("Associated ACK with response set %d\n", seq_id);
   }

   for (int i=0; i<num_commands; i++) 
   {
      CommandDescriptor *desc = cmd_list+i;
      ToolCommand *base_cmd = (ToolCommand *) (((char *) msg) + desc->offset);

      BG_ThreadID_t threadID = base_cmd->threadID;
      int_thread *thr = proc->threadPool()->findThreadByLWP(threadID);

      response::ptr cur_resp = response::ptr();
      if (resp_set) {
         getResponses().lock();
         resp_lock_held = true;
         bool found = false;
         unsigned int id = resp_set->getIDByIndex(i, found);
         pthrd_printf("Associated ACK %u and index %u with internal id %u\n", seq_id, i, id);
         if (found)
            cur_resp = getResponses().rmResponse(id);
         if (!cur_resp) {
            resp_lock_held = false;
            getResponses().unlock();
         }
         pthrd_printf("%s response in message\n", cur_resp ? "Found" : "Did not find");
      }
  
      switch (desc->type) {
         case GetSpecialRegsAck: 
         case GetGeneralRegsAck: {
            allreg_response::ptr allreg = cur_resp->getAllRegResponse();
            assert(allreg);
            int_registerPool *regpool = allreg->getRegPool();
            assert(allreg);
            assert(regpool);
            if (desc->type == GetSpecialRegsAck) {
               pthrd_printf("Decoding GetSpecialRegsAck on %d/%d\n", 
                            proc->getPid(), thr->getLWP());
               GetSpecialRegsAckCmd *cmd = static_cast<GetSpecialRegsAckCmd *>(base_cmd);
               bgq_thread::regBufferToPool(NULL, &cmd->sregs, *regpool);
            }
            else {
               pthrd_printf("Decoding GetGeneralRegsAck on %d/%d\n", 
                            proc->getPid(), thr->getLWP());
               GetGeneralRegsAckCmd *cmd = static_cast<GetGeneralRegsAckCmd *>(base_cmd);
               bgq_thread::regBufferToPool(cmd->gpr, NULL, *regpool);
            }
            allreg->postResponse();
            if (desc->returnCode)
               allreg->markError(desc->returnCode);
            
            Event::ptr new_ev = decodeCompletedResponse(allreg, async_ev_map);
            if (new_ev)
               events.push_back(new_ev);

            //BG/Q doesn't have an individual register access mechanism.
            // Thus we turn requests for one register into requests for all registers.
            // Here we check if this request is in response to a single register query
            // and fill out the reg_response if so.
            reg_response::ptr indiv_reg = allreg->getIndividualAcc();
            if (indiv_reg && !indiv_reg->isReady()) {
               if (allreg->hasError()) {
                  allreg->markError(desc->returnCode);
               }
               else {
                  MachRegister reg = allreg->getIndividualReg();
                  int_registerPool::reg_map_t::iterator i = regpool->regs.find(reg);
                  if (i != regpool->regs.end()) {
                     indiv_reg->postResponse(i->second);
                     new_ev = decodeCompletedResponse(indiv_reg, async_ev_map);
                     if (new_ev)
                        events.push_back(new_ev);
                  }
               }
            }

            resp_lock_held = false;
            getResponses().signal();
            getResponses().unlock();
            break;
         }
         case GetFloatRegsAck:
         case GetDebugRegsAck:
            assert(0); //Currently unused, but eventually needed
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

            Event::ptr new_ev = decodeCompletedResponse(memresp, async_ev_map);
            if (new_ev)
               events.push_back(new_ev);

            resp_lock_held = false;
            getResponses().signal();
            getResponses().unlock();
            break;
         }
         case GetThreadDataAck: {
            pthrd_printf("Decoded new event on %d to GetThreadDataAck message\n", proc->getPid());
            stack_response::ptr stkresp = cur_resp->getStackResponse();
            GetThreadDataAckCmd *cmd = static_cast<GetThreadDataAckCmd *>(base_cmd);
            assert(stkresp);
            assert(cmd);
            archevent->dontFreeMsg();
            bgq_process::held_msgs.insert(msg);
            stkresp->postResponse((void *) cmd);
            if (desc->returnCode) 
               stkresp->markError(desc->returnCode);
            
            Event::ptr new_ev = decodeCompletedResponse(stkresp, async_ev_map);
            if (new_ev)
               events.push_back(new_ev);
            
            resp_lock_held = false;
            getResponses().signal();
            getResponses().unlock();
            break;
         }
         case GetThreadListAck:
            pthrd_printf("Decoded GetThreadListAck on %d/%d. Dropping\n",
                         proc ? proc->getPid() : -1, thr ? thr->getLWP() : -1);
            break;
         case GetAuxVectorsAck:
            pthrd_printf("Decoded GetAuxVectorsAck on %d/%d. Dropping\n", 
                         proc ? proc->getPid() : -1, thr ? thr->getLWP() : -1);
            break;
         case GetProcessDataAck: {
            pthrd_printf("Decoded new event on %d to GetProcessData message\n", proc->getPid());
            decodeStartupEvent(archevent, proc, msg, Event::async, events);
            break;
         }
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
               Event::ptr new_ev = decodeCompletedResponse(result_resp, async_ev_map);
               if (new_ev)
                  events.push_back(new_ev);

               resp_lock_held = false;
               getResponses().signal();
               getResponses().unlock();
               break;
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
            pthrd_printf("Decoded ReleaseControlAck on %d\n", proc->getPid());
            decodeReleaseControlAck(archevent, proc, desc->returnCode, events);
            break;
         case SetContinuationSignalAck:
            pthrd_printf("Decoded SetContinuationSignalAck on %d/%d. Dropping\n", proc->getPid(), thr->getLWP());
            break;
         default:
            break;
      }
      assert(!resp_lock_held);
   }

   delete archevent;

   if (resp_set)
      delete resp_set;

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
   sw_breakpoint *ibp = proc->getBreakpoint(addr);
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
       (int) proc->startup_state <= (int) bgq_process::waitfor_control_request_signal)
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
   
   bp_instance *ibp = thr->isClearingBreakpoint();
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
   if (!thr && proc->startup_state != bgq_process::startup_done) {
      thr = proc->threadPool()->initialThread();
   }
   assert(thr);
   
   pthrd_printf("Decoding signal on %d/%d at 0x%lx\n", proc->getPid(), thr->getLWP(), insn_addr);
   switch (msg->type.signal.reason) {
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
   }
   assert(0);
   return false;
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

   int_threadPool::iterator i = proc->threadPool()->begin();
   for (; i != proc->threadPool()->end(); i++) {
      (*i)->getGeneratorState().setState(int_thread::exited);
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
#warning TODO implement control notify
      assert(0); //Not yet implemented
      return false;
   }
}

bool DecoderBlueGeneQ::decodeDetachAck(ArchEventBGQ *archevent, bgq_process *proc, std::vector<Event::ptr> &events)
{
   DetachAckMessage *msg = static_cast<DetachAckMessage *>(archevent->getMsg());

   assert(proc->detach_state == bgq_process::issued_detach || proc->detach_state == bgq_process::issued_all_detach);

   //Create a set 'procs' that contains all the processes we're detaching from.  
   // For an all_detach, that's everything on the compute node.  Otherwise it's just 'proc'
   set<bgq_process *> me_set;
   me_set.insert(proc);
   const set<bgq_process *> &procs = (proc->detach_state == bgq_process::issued_detach) ? 
      me_set : 
      proc->getComputeNode()->getProcs();

   bool has_error = false;
   if (msg->header.errorCode != 0) {
      perr_printf("detachAck on process %d had error return %d", proc->getPid(), (int) msg->header.errorCode);
      has_error = true;
   }
   else if (msg->numProcess != procs.size()) {
      perr_printf("detachAck on process %d had wrong number of procs: %u != %u.  Had: ",
                  proc->getPid(), (unsigned) msg->numProcess, (unsigned) procs.size());
      assert(msg->numProcess < MaxRanksPerNode);
      for (unsigned i=0; i<msg->numProcess; i++) {
         pclean_printf("%u, ", (unsigned int) msg->rank[i]);
      }
      pclean_printf("\n");
      has_error = true;
   }

   //Throw a detach event for each process that may have been waiting for this detach
   for (set<bgq_process *>::iterator i = procs.begin(); i != procs.end(); i++) {
      bgq_process *qproc = *i;
      Event::ptr new_ev = createEventDetach(qproc, has_error);
      events.push_back(new_ev);
   }
   
   return true;
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
      qproc->getComputeNode()->handleMessageAck();
   }

   switch (header->type) {
      case ControlAck:
         pthrd_printf("Decoding ControlAck on %d\n", proc->getPid());
         return decodeStartupEvent(archevent, qproc, msg, Event::async, events);
      case AttachAck:
         pthrd_printf("Decoding AttachAck on %d\n", proc->getPid());
         return decodeStartupEvent(archevent, qproc, msg, Event::async, events);
      case DetachAck:
         pthrd_printf("Decoding DetachAck on %d\n", proc->getPid());
         return decodeDetachAck(archevent, qproc, events);
      case QueryAck: {
         QueryAckMessage *qack = static_cast<QueryAckMessage *>(msg);
         uint16_t num_commands = qack->numCommands;
         CommandDescriptor *cmd_list = qack->cmdList;
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

DebugThread *DebugThread::me;
extern void setXThread(long t);

DebugThread::DebugThread() :
   shutdown(false),
   initialized(false)
{
   if (me && me->initialized)
      return;

   init_lock.lock();
   if (!me) {
      pfd[0] = pfd[1] = -1;
      me = this;
      debug_thread.spawn(mainLoopWrapper, NULL);
      while (!initialized && !shutdown) {
         init_lock.wait();
      }
   }
   init_lock.unlock();
}

DebugThread::~DebugThread()
{
   if (initialized && !shutdown) {
      char c = 'x';
      shutdown = true;
      write(pfd[1], &c, 1);
      debug_thread.join();
   }
   if (pfd[0] != -1)
      close(pfd[0]);
   if (pfd[1] != -1)
      close(pfd[1]);
}

void DebugThread::mainLoopWrapper(void *)
{
   me->mainLoop();
}

bool DebugThread::init()
{
   bool ret = false;

   init_lock.lock();
   registerSignalHandlers(true);
  
   setXThread(DThread::self());
   int result = pipe(pfd);
   if (result == -1) {
      perr_printf("Error creating shutdown pipe: %s\n", strerror(errno));
      goto done;
   }
   ret = true;
   initialized = true;
  done:  
   init_lock.broadcast();
   init_lock.unlock();
   return ret;
}

void DebugThread::mainLoop()
{
   if (!init()) {
      pthrd_printf("Could not initialize debug thread\n");
      return;
   }

   while (!shutdown) {
      fd_set readfds;
      FD_ZERO(&readfds);
      int nfds = 0;
      if (pfd[0] != -1) {
         FD_SET(pfd[0], &readfds);
         if (pfd[0] > nfds)
            nfds = pfd[0];
      }

      struct timeval timeout;
      timeout.tv_sec = 10;
      timeout.tv_usec = 0;

      int result = select(nfds+1, &readfds, NULL, NULL, &timeout);
      if (result == -1) {
         int error = errno;
         if (error == EINTR)
            continue;
         perr_printf("Error calling select: %s\n", strerror(error));
         shutdown = true;
         break;
      }
    
      struct stat buf;
      int sresult = stat("./pc_force_shutdown", &buf);
      if (sresult == 0) {
         pthrd_printf("Forcing shutdown\n");
         abort();
      }
   }
}

DebugThread *DebugThread::getDebugThread()
{
   if (me)
      return me;
   DebugThread *debugthrd = new DebugThread();
   if (debugthrd != me) {
      delete debugthrd;
   }
   return me;
}

static const unsigned int max_stack_size = 256;

static void on_crash(int sig, siginfo_t *, void *context)
{
   registerSignalHandlers(false);
   if (dyninst_debug_proccontrol) {
      pthrd_printf("Caught signal %d, issuing emergency shutdown procedures\n", sig);

      pthrd_printf("Register dump:\n");
      ucontext_t *u_context = (ucontext_t *) context;
      for (unsigned i=0; i<NGREG; i++) {
         pclean_printf("r%u=0x%lx", i, u_context->uc_mcontext.gp_regs[i]);
         if (i % 4 == 3)
            pclean_printf("\n");
         else
            pclean_printf("\t");
      }

      pthrd_printf("Library List:\n");
      for (struct link_map *i = _r_debug.r_map; i; i = i->l_next) {
         pthrd_printf("\t0x%lx (%p) - %s\n", i->l_addr, i->l_ld, i->l_name);
      }
   
      pthrd_printf("Stack trace:\n");
      void *stack_size[max_stack_size];
      int addr_list_size = backtrace(stack_size, max_stack_size);
      int fd = fileno(pctrl_err_out);
      fflush(pctrl_err_out);
      backtrace_symbols_fd(stack_size, addr_list_size, fd);
   }

   ComputeNode::emergencyShutdown();
   pthrd_printf("Calling abort\n");
   abort();
}

static void registerSignalHandlers(bool enable)
{
   struct sigaction action;
   bzero(&action, sizeof(struct sigaction));
   if (!enable)
      action.sa_handler = SIG_DFL;
   else {
      action.sa_sigaction = on_crash;
      action.sa_flags = SA_SIGINFO;
   }

   sigaction(SIGSEGV, &action, NULL);
   sigaction(SIGBUS, &action, NULL);
   sigaction(SIGABRT, &action, NULL);
   sigaction(SIGILL, &action, NULL);
   sigaction(SIGQUIT, &action, NULL);
   sigaction(SIGTERM, &action, NULL);
}

void ComputeNode::emergencyShutdown()
{
   char message[8192];

   pthrd_printf("Shutting down generator\n");
   GeneratorBGQ *gen = static_cast<GeneratorBGQ *>(Generator::getDefaultGenerator());
   gen->shutdown();
   pthrd_printf("Done shutting down generator\n");

   pthrd_printf("all_compute_nodes.size() = %u\n", (unsigned) all_compute_nodes.size());
   for (set<ComputeNode *>::iterator i = all_compute_nodes.begin(); i != all_compute_nodes.end(); i++) {
      ComputeNode *cn = *i;
      if (!cn) {
         pthrd_printf("Skipping empty compute node\n");
         continue;
      }
      pthrd_printf("cn->procs.size() = %u\n", (unsigned) cn->procs.size());
      for (set<bgq_process *>::iterator j = cn->procs.begin(); j != cn->procs.end(); j++) {
        again: {
            bgq_process *proc = *j;
            if (!proc) {
               pthrd_printf("Skipping NULL proc\n");
               continue;
            }
            int pid = proc->getPid();
            pthrd_printf("Emergency shutdown of %d\n", pid);

            UpdateMessage *update = (UpdateMessage *) message;
            MessageHeader *header = &update->header;
            CommandDescriptor *command = update->cmdList;

            SendSignalCmd *send = (SendSignalCmd *) (update + 1);
            SetContinuationSignalCmd *cont_sig = (SetContinuationSignalCmd *) (send + 1);
            ReleaseThreadCmd *release = (ReleaseThreadCmd *) (cont_sig + 1);
            ContinueProcessCmd *cont = (ContinueProcessCmd *) (release + 1);

            size_t size = ((char *) (cont+1)) - message;
      
            header->service = ToolctlService;
            header->version = ProtocolVersion;
            header->type = Update;
            header->rank = pid;
            header->sequenceId = 0;
            header->returnCode = 0;
            header->length = size;
            header->jobId = bgq_process::getJobID();
      
            update->toolId = bgq_process::getToolID();
            update->numCommands = 4;

            int cur_cmd = 0;
      
            command[cur_cmd].type = SendSignal;
            command[cur_cmd].offset = ((char *) send) - message;
            command[cur_cmd].length = sizeof(SendSignalCmd);
            command[cur_cmd].returnCode = 0;
            cur_cmd++;

            command[cur_cmd].type = SetContinuationSignal;
            command[cur_cmd].offset = ((char *) cont_sig) - message;
            command[cur_cmd].length = sizeof(SetContinuationSignalCmd);
            command[cur_cmd].returnCode = 0;
            cur_cmd++;

            command[cur_cmd].type = ReleaseThread;
            command[cur_cmd].offset = ((char *) release) - message;
            command[cur_cmd].length = sizeof(ReleaseThreadCmd);
            command[cur_cmd].returnCode = 0;
            cur_cmd++;

            command[cur_cmd].type = ContinueProcess;
            command[cur_cmd].offset = ((char *) cont) - message;
            command[cur_cmd].length = sizeof(ContinueProcessCmd);
            command[cur_cmd].returnCode = 0;
            cur_cmd++;

            send->threadID = 0;
            send->signum = SIGKILL;

            cont_sig->threadID = 0;
            cont_sig->signum = SIGKILL;

            release->threadID = 0;

            cont->threadID = 0;
      
            pthrd_printf("Sending emergency KILL 9 to %d\n", pid);
            bool result = cn->reliableWrite(message, size);
            if (!result) {
               pthrd_printf("Failed to write to CN\n");
               continue;
            }
      
            for (unsigned k=0; k<2; i++) {
               MessageHeader *hdr = (MessageHeader *) message;
               pthrd_printf("Reading header ACK from KILL on %d\n", pid);
               result = gen->reliableRead(cn->getFD(), message, sizeof(MessageHeader), 5);
               if (!result) {
                  pthrd_printf("Failed to read header from CN\n");
                  break;
               }
               if (hdr->length > SmallMessageDataSize) {
                  pthrd_printf("Read corrupted ACK from CN\n");
                  break;
               }
               if (hdr->length > 8192) {
                  pthrd_printf("Could not read %d bytes\n", hdr->length);
                  break;
               }
               result = gen->reliableRead(cn->getFD(), message+sizeof(MessageHeader), hdr->length - sizeof(MessageHeader), 5);
               if (!result) {
                  pthrd_printf("Failed to read body from CN\n");
                  break;
               }
               if (header->type != UpdateAck) {
                  pthrd_printf("Not an update on attempt %d\n", k+1);
                  continue;
               }

               UpdateAckMessage *ua_message = (UpdateAckMessage *) message;
               for (unsigned l = 0; l < ua_message->numCommands; l++) {
                  pthrd_printf("Returned type %u with returnCode %u\n",
                               (unsigned) ua_message->cmdList[l].type,
                               (unsigned) ua_message->cmdList[l].returnCode);
               }
               break;
            }


            pthrd_printf("Building ReleaseControlCmd\n");
            ReleaseControlCmd *releasectrl = (ReleaseControlCmd *) (update + 1);
            size = ((char *) (cont+1)) - message;
       
            header->service = ToolctlService;
            header->version = ProtocolVersion;
            header->type = Update;
            header->rank = pid;
            header->sequenceId = 0;
            header->returnCode = 0;
            header->length = size;
            header->jobId = bgq_process::getJobID();

            update->toolId = bgq_process::getToolID();
            update->numCommands = 1;

            command[0].type = ReleaseControl;
            command[0].offset = ((char *) releasectrl) - message;
            command[0].length = sizeof(ReleaseControlCmd);
            command[0].returnCode = 0;

            releasectrl->notify = ReleaseControlNotify_Inactive;

            pthrd_printf("Sending ReleaseControlCommand to %d\n", pid);
            result = cn->reliableWrite(message, size);
            if (!result) {
               pthrd_printf("Failed to write to CN\n");
               continue;
            }

            MessageHeader *hdr = (MessageHeader *) message;
            pthrd_printf("Reading header ACK from KILL on %d\n", pid);
            result = gen->reliableRead(cn->getFD(), message, sizeof(MessageHeader), 5);
            if (!result) {
               pthrd_printf("Failed to read header from CN\n");
               break;
            }
            if (hdr->length > SmallMessageDataSize) {
               pthrd_printf("Read corrupted ACK from CN\n");
               break;
            }

            pthrd_printf("Reading body on %d\n", pid);
            result = gen->reliableRead(cn->getFD(), message+sizeof(MessageHeader), hdr->length - sizeof(MessageHeader), 5);
            if (!result) {
               pthrd_printf("Failed to read body from CN\n");
               break;
            }

            UpdateAckMessage *ua_message = (UpdateAckMessage *) message;
            pthrd_printf("Reading Have %u messages on %d\n", (unsigned) ua_message->numCommands, pid);
            for (unsigned l = 0; l < ua_message->numCommands; l++) {
               pthrd_printf("Returned type %u with returnCode %u\n",
                            (unsigned) ua_message->cmdList[l].type,
                            (unsigned) ua_message->cmdList[l].returnCode);
            }
            if (ua_message->numCommands && ua_message->cmdList[0].returnCode == CmdPendingNotify)
               goto again;
         }
      }
   }
   pthrd_printf("done.\n");
}
