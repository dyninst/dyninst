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

#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

#include <dirent.h>
#include <limits.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/syscall.h>
#include <signal.h>
#include <ucontext.h>
#include <elf.h>
#include <link.h>
#include <execinfo.h>
#include <unistd.h>

#include "bluegeneq.h"
#include "int_event.h"
#include "irpc.h"
#include "PCProcess.h"
#include "PlatFeatures.h"
#include "PCErrors.h"
#include "Mailbox.h"

#define USE_THREADED_IO

using namespace Dyninst;
using namespace ProcControlAPI;
using namespace std;

#if defined(WITH_SYMLITE)
#include "symlite/h/SymLite-elf.h"
#elif defined(WITH_SYMTAB_API)
#include "symtabAPI/h/SymtabReader.h"
#else
#error "No defined symbol reader"
#endif

using namespace bgq;
using namespace std;

static bool emergency = false;

static void registerSignalHandlers(bool enable);

static long timeout_val = 0;
static long no_control_authority_val = 0;

static const long startup_timeout_sec = 300;
static const long stackwalk_timeout_sec = 300;

uint64_t bgq_process::jobid = 0;
uint32_t bgq_process::toolid = 0;
bool bgq_process::set_ids = false;
#warning TODO do we do an all_attach during subset debugging?
bool bgq_process::do_all_attach = true;

const char *getCommandName(uint16_t cmd_type);

static void printMessage(ToolMessage *msg, const char *str, bool short_msg = false)
{
   if (!dyninst_debug_proccontrol)
      return;
   MessageHeader *header = &msg->header;

   pc_print_lock();
   if (!short_msg) 
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
   else
      pthrd_printf("%s to %u: ", str, header->rank);
  
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
         if (!short_msg) {
            for (unsigned i=0; i<(unsigned) m->numCommands; i++) {
               CommandDescriptor &c = m->cmdList[i];
               pclean_printf("\t[%u] type = %s, reserved = %u, offset = %u, length = %u, returnCode = %u\n",
                             i, getCommandName(c.type), (unsigned) c.reserved, c.offset,
                             c.length, c.returnCode);
            }
            break;
         }
      }
      case UpdateAck: {
         UpdateAckMessage *m = static_cast<UpdateAckMessage *>(msg);
         pclean_printf("UpdateAck: numCommands = %u\n", (unsigned) m->numCommands);
         if (!short_msg) {
            for (unsigned i=0; i<(unsigned) m->numCommands; i++) {
               CommandDescriptor &c = m->cmdList[i];
               pclean_printf("\t[%u] type = %s, reserved = %u, offset = %u, length = %u, returnCode = %u\n",
                             i, getCommandName(c.type), (unsigned) c.reserved, c.offset,
                             c.length, c.returnCode);
            }
         }
         break;
      }
      case Query: {
         QueryMessage *m = static_cast<QueryMessage *>(msg);
         pclean_printf("Query: numCommands = %u\n", (unsigned) m->numCommands);
         if (!short_msg) {
            for (unsigned i=0; i<(unsigned) m->numCommands; i++) {
               CommandDescriptor &c = m->cmdList[i];
               pclean_printf("\t[%u] type = %s, reserved = %u, offset = %u, length = %u, returnCode = %u\n",
                             i, getCommandName(c.type), (unsigned) c.reserved, c.offset,
                             c.length, c.returnCode);
            }
         }
         break;
      }
      case Update: {
         UpdateMessage *m = static_cast<UpdateMessage *>(msg);
         pclean_printf("Update: numCommands = %u\n", (unsigned) m->numCommands);
         if (!short_msg) {
            for (unsigned i=0; i<(unsigned) m->numCommands; i++) {
               CommandDescriptor &c = m->cmdList[i];
               pclean_printf("\t[%u] type = %s, reserved = %u, offset = %u, length = %u, returnCode = %u\n",
                             i, getCommandName(c.type), (unsigned) c.reserved, c.offset,
                             c.length, c.returnCode);
            }
         }
         break;
      }
      case Notify: {
         pclean_printf("Notify ");
         NotifyMessage *no = static_cast<NotifyMessage *>(msg);
         switch (no->notifyMessageType) {
            case NotifyMessageType_Signal:
               pclean_printf("Signal: signum = %u, threadId = %u, instAddress = 0x%lx, "
                             "dataAddress = %lx, reason = %u, executeMode = %d\n",
                             no->type.signal.signum, (unsigned) no->type.signal.threadID,
                             no->type.signal.instAddress, no->type.signal.dataAddress, 
                             (unsigned) no->type.signal.reason,
                             (unsigned) no->type.signal.executeMode);
               break;
            case NotifyMessageType_Termination:
               pclean_printf("Exit: exitStatus = %u\n", no->type.termination.exitStatus);
               break;
            case NotifyMessageType_Control:
               pclean_printf("Control: toolid = %u, toolTag = %.8s, priority = %u, reason = %u\n",
                             no->type.control.toolid, no->type.control.toolTag, (unsigned) no->type.control.priority,
                             (unsigned) no->type.control.reason);
               break;
            default:
               pclean_printf("Unknown: %u\n", (unsigned int) no->notifyMessageType);
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
   pc_print_unlock();
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

static Mutex<> id_init_lock;
bgq_process::bgq_process(Dyninst::PID p, std::string e, std::vector<std::string> a, 
                         vector<string> envp, std::map<int, int> f) :
   int_process(p, e, a, envp, f),
   resp_process(p, e, a, envp, f),
   sysv_process(p, e, a, envp, f),
   thread_db_process(p, e, a, envp, f),
   ppc_process(p, e, a, envp, f),
   hybrid_lwp_control_process(p, e, a, envp, f),
   mmap_alloc_process(p, e, a, envp, f),
   int_multiToolControl(p, e, a, envp, f),
   int_signalMask(p, e, a, envp, f),
   int_callStackUnwinding(p, e, a, envp, f),
   int_BGQData(p, e, a, envp, f),
   int_remoteIO(p, e, a, envp, f),
   int_memUsage(p, e, a, envp, f),
   last_ss_thread(NULL),
   lwp_tracker(NULL),
   hasControlAuthority(false),
   interp_base_set(false),
   page_size_set(false),
   debugger_suspended(false),
   decoder_pending_stop(false),
   is_doing_temp_detach(false),
   stopped_on_startup(false),
   held_on_startup(false),
   got_startup_stop(false),
   rank(pid),
   page_size(0),
   interp_base(0),
   cur_memquery(no_memquery),
   procdata_result_valid(false),
   get_thread_list(NULL),
   stopwait_on_control_authority(NULL),
   priority(0),
   mtool(NULL),
   startup_state(issue_attach),
   detach_state(no_detach_issued)
{
   if (!set_ids) {
      ScopeLock<> lock(id_init_lock);
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
   ReaderThread::get();

   query_transaction = new QueryTransaction(this, Query);
   update_transaction = new UpdateTransaction(this, Update);
   cn = ComputeNode::getComputeNodeByRank(rank);
   cn->procs.insert(this);
   WriterThread::get()->addProcess(this);
}

bgq_process::~bgq_process()
{
   WriterThread::get()->rmProcess(this);
   if (get_thread_list) {
      delete get_thread_list;
      get_thread_list = NULL;
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
   struct timeval startup_timeout;
   startup_timeout.tv_sec = startup_timeout_sec;
   startup_timeout.tv_usec = 0;
   ReaderThread::get()->setTimeout(startup_timeout);

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
   return true;
}

bool bgq_process::plat_detach(result_response::ptr, bool)
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
         ScopeLock<> lock_this_scope(cn->detach_lock);
         
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
   return false;
}

bool bgq_process::needIndividualThreadAttach()
{
   return true;
}

bool bgq_process::getThreadLWPs(std::vector<Dyninst::LWP> &lwps)
{
   if (!get_thread_list)
      return true;

   for (uint32_t i = 0; i < get_thread_list->numthreads; i++) {
      lwps.push_back(get_thread_list->threadlist[i].tid);
   }
   delete get_thread_list;
   get_thread_list = NULL;
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
#if defined(WITH_SYMLITE)
   static SymbolReaderFactory *symreader_factory = NULL;
   if (symreader_factory)
      return symreader_factory;

   symreader_factory = (SymbolReaderFactory *) new SymElfFactory();
   return symreader_factory;
#elif defined(WITH_SYMTAB_API)
   return SymtabAPI::getSymtabReaderFactory();
#else
#error "No defined symbol reader"
#endif
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

bool bgq_process::plat_individualRegRead(Dyninst::MachRegister reg, int_thread *thr)
{
   if (!reg.isPC())
      return false;
   bgq_thread *bgqthr = dynamic_cast<bgq_thread *>(thr);
   Address addr;
   return bgqthr->haveCachedPC(addr);
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

bool bgq_process::plat_needsThreadForMemOps() const
{
   return false;
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

unsigned int bgq_process::plat_getCapabilities()
{
   if (!hasControlAuthority)
      return Process::pc_read;
   return int_process::plat_getCapabilities();
}

Event::ptr bgq_process::plat_throwEventsBeforeContinue(int_thread *thr)
{
   if (!pending_control_authority)
      return Event::ptr();
   if (thr != threadPool()->initialThread())
      return Event::ptr();

   pthrd_printf("Throwing new ControlAuthority event upon main thread continue in %d\n", getPid());
   int_eventControlAuthority *iev = new int_eventControlAuthority();
   *iev = *pending_control_authority->getInternalEvent();
   iev->unset_desync = false;

   thr->getControlAuthorityState().desyncStateProc(int_thread::stopped);

   EventControlAuthority::ptr new_ev = EventControlAuthority::ptr(new EventControlAuthority(EventType::Post, iev));
   new_ev->setProcess(proc());
   int_thread *initial_thread = threadPool()->initialThread();
   new_ev->setThread(initial_thread->thread());
   new_ev->setSyncType(Event::async);

   return new_ev;
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

   if (hasControlAuthority) {
      new_ev_sync_state = Event::async;
      new_state = int_thread::stopped;
   }
   else {
      new_ev_sync_state = Event::async;
      new_state = int_thread::running;
   }

   for (int_threadPool::iterator i = threadPool()->begin(); i != threadPool()->end(); i++) {
      int_thread *thr = *i;
      if (new_state == int_thread::stopped) {
         thr->getGeneratorState().setState(new_state);
         thr->getHandlerState().setState(new_state);
      }
      else {
         //The order we set these in matters on whether we're doing stop/run operations
         thr->getHandlerState().setState(new_state);
         thr->getGeneratorState().setState(new_state);
      }
      thr->getUserState().setState(new_state);
   }
   ev->setSyncType(new_ev_sync_state);

   getStartupTeardownProcs().inc();
      
   mbox()->enqueue(ev);
}

LWPTracking *bgq_process::getLWPTracking()
{
   if (!lwp_tracker) {
      lwp_tracker = new LWPTracking(proc());
   }
   return lwp_tracker;
}

bool bgq_process::plat_lwpRefresh(result_response::ptr resp)
{
   lwp_tracking_resp = resp;
   GetThreadListCmd cmd;
   cmd.threadID = 0;
   bool result = sendCommand(cmd, GetThreadList, resp);
   if (!result) {
      pthrd_printf("Error sending GetThreadList command\n");
      return false;
   }
   return true;
}

bool bgq_process::plat_lwpRefreshNoteNewThread(int_thread *thr)
{
   EventBootstrap::ptr newev = EventBootstrap::ptr(new EventBootstrap());
   newev->setProcess(proc());
   newev->setThread(thr->thread());
   newev->setSyncType(Event::async);
   mbox()->enqueue(newev);   
   return true;
}

int bgq_process::threaddb_getPid()
{
   return get_procdata_result.tgid;
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
   bool expecting_stop = false;

   if (getState() == errorstate) {
      pthrd_printf("Process is already in error state, skipping handling\n");
      return false;
   }

   /**
    * Check error triggered events
    **/
   if (data == &timeout_val) {
      pthrd_printf("Timeout waiting for startup event on %d\n", getPid());
      switch (startup_state) {
         case waitfor_attach:
            setLastError(err_dattachack, "Process timed out while waiting for attach ack");
            break;
         case waitfor_control_request_ack:
            setLastError(err_dcaack, "Process timed out while waiting for CA ack");
            break;
         case waitfor_data_or_stop:
            setLastError(err_dthrdstop, "Process timed out while waiting for stop/query");
            break;
         case waitfor_control_request_signal:
            setLastError(err_dsigstop, "Process timed out while waiting for stop");
            break;
         case reissue_data_collection: 
         case waitfor_data_collection:
            setLastError(err_dthrdquery, "Process timed out while waiting for query");
            break;
         case step_insn:
            setLastError(err_dstep, "Process timed out while waiting for step");
            break;
         default:
            setLastError(err_internal, "Process timed out during unexpected state");
      }
      setState(errorstate);
      ReaderThread::get()->clearTimeout();
      if (getStartupTeardownProcs().local())
         getStartupTeardownProcs().dec();
      startup_state = startup_donedone;
      return false;
   }
   else if (data == &no_control_authority_val) {
      pthrd_printf("Timeout waiting for CA event on %d\n", getPid());
      setLastError(err_cauthority, "Process timed out while waiting for control authority");
      setState(errorstate);
      ReaderThread::get()->clearTimeout();
      if (getStartupTeardownProcs().local())
         getStartupTeardownProcs().dec();
      return false;
   }

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
         pthrd_printf("CN attach left us in an error state, failing attach to %d\n", getPid());
         setState(int_process::errorstate);
         ReaderThread::get()->clearTimeout();
         return false;
      }
      else if (cn->all_attach_done) {
         attach_action = group_done;
      }
      else {
         ScopeLock<> lock(cn->attach_lock);
         pthrd_printf("Doing all attach\n");
         if (cn->all_attach_error) {
            pthrd_printf("CN attach left us in an error state, failng attach to %d\n", getPid());
            setState(int_process::errorstate);
            ReaderThread::get()->clearTimeout();
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
         setState(errorstate);
         if (getStartupTeardownProcs().local())
            getStartupTeardownProcs().dec();
         ReaderThread::get()->clearTimeout();
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
         ScopeLock<> lock(cn->attach_lock);
         cn->all_attach_done = true;
         set<ToolMessage *>::iterator i;
         
         if (attach_retcode != Success) {
            perr_printf("Group attach returned error %d: %s\n", (int) attach_retcode,
                        bgq_process::bgqErrorMessage(attach_retcode));
            setState(int_process::errorstate);
            cn->all_attach_error = true;
            for (set<bgq_process *>::iterator i = cn->procs.begin(); i != cn->procs.end(); i++) {
               (*i)->setState(int_process::errorstate);
               ReaderThread::get()->clearTimeout();
               if ((*i)->getStartupTeardownProcs().local())
                  (*i)->getStartupTeardownProcs().dec();
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
      startup_state = skip_control_request_signal;
   }

   if (startup_state == issue_control_request) {
      pthrd_printf("Issuing ControlMessage in startup handler\n");
      ControlMessage *ctrl_msg = (ControlMessage *) malloc(sizeof(ControlMessage));
      sigset_t sigs = getSigMask();;
      sigaddset(&sigs, SIGTRAP);
      sigaddset(&sigs, SIGSTOP);
      ctrl_msg->notifySignalSet(&sigs);
      ctrl_msg->sndSignal = SIGSTOP;
      ctrl_msg->dynamicNotifyMode = DynamicNotifyDLoader;
      ctrl_msg->dacTrapMode = DACTrap_NoChange;
      fillInToolMessage(*ctrl_msg, Control);
      
      startup_state = waitfor_control_request_ack;
      bool result = getComputeNode()->writeToolAttachMessage(this, ctrl_msg, true);
      if (!result) {
         pthrd_printf("Error writing ControlMessage in attach handler\n");
         delete ctrl_msg;
         setState(errorstate);
         ReaderThread::get()->clearTimeout();
         if (getStartupTeardownProcs().local())
            getStartupTeardownProcs().dec();
         return false;
      }

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

      if (ctrl_retcode != Success && ctrl_retcode != ToolControlConflict) {
         perr_printf("Error return in ControlAckMessage: %s\n",
                     bgqErrorMessage(ctrl_retcode));
         setLastError(err_internal, "Could not send ControlMessage\n");
         setState(errorstate);
         ReaderThread::get()->clearTimeout();
         if (getStartupTeardownProcs().local())
            getStartupTeardownProcs().dec();
         return false;
      }
      if (controllingToolId != bgq_process::getToolID()) {
         pthrd_printf("Tool '%s' with ID %d has authority at priority %d\n", 
                      toolTag, controllingToolId, tool_priority);
         if (tool_priority > priority) {
            pthrd_printf("WARNING: Tool %s has higher priority (%d > %d), running in Query-Only mode\n",
                         toolTag, tool_priority, priority);
            expecting_stop = false;
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
         pthrd_printf("We are top priority tool, now have control authority\n");
         hasControlAuthority = true;
         expecting_stop = true;
         startup_state = issue_data_collection;
      }
   }

   if (startup_state == waitfor_control_request_notice) {
      NotifyMessage *notify_msg = static_cast<NotifyMessage *>(data);
      if (notify_msg->type.control.reason == NotifyControl_Available) {
         pthrd_printf("ControlRequestNotify means we can now try to retake control\n");
         startup_state = issue_control_request;
         free(notify_msg);
         return handleStartupEvent(NULL);
      }
      else if (notify_msg->type.control.reason == NotifyControl_Conflict) {
         pthrd_printf("Conflict message from control request, moving to data collection\n");
         pthrd_printf("Conflicting tag is %s, id = %u, priority = %u\n",
                      notify_msg->type.control.toolTag,
                      notify_msg->type.control.toolid,
                      notify_msg->type.control.priority);
         startup_state = skip_control_request_signal;
         free(notify_msg);
      }
      else
         assert(0);
   }

   /**
    * Send a set of query in a single CommandMessage.  We want:
    *  - GetProcessData
    *  - AuxVectors
    *  - GetThreadData
    *  - ThreadList
    **/
   if (startup_state == issue_data_collection || startup_state == skip_control_request_signal) {
      pthrd_printf("Issuing data request\n");

      GetProcessDataCmd procdata_cmd;
      procdata_cmd.threadID = 0;
      sendCommand(procdata_cmd, GetProcessData);

      GetAuxVectorsCmd auxv_cmd;
      auxv_cmd.threadID = 0;
      sendCommand(auxv_cmd, GetAuxVectors);

      GetThreadDataCmd thrddata_cmd;
      thrddata_cmd.threadID = 0;
      sendCommand(thrddata_cmd, GetThreadData);

      GetThreadListCmd thread_list;
      thread_list.threadID = 0;
      sendCommand(thread_list, GetThreadList);

      if (expecting_stop && startup_state != skip_control_request_signal)
         startup_state = waitfor_data_or_stop;
      else
         startup_state = waitfor_data_collection;
      return true;
   }

   bool data_on_either = false;
   bool stop_on_either = false;
   if (startup_state == waitfor_data_or_stop) {
      ToolMessage *msg = static_cast<ToolMessage *>(data);
      data_on_either = (msg->header.type == QueryAck);
      stop_on_either = (msg->header.type == Notify);
      pthrd_printf("Waiting for either data or stop, got %s\n", data_on_either ? "data" : "stop");
      assert((data_on_either || stop_on_either) && !(data_on_either && stop_on_either));
   }

   if (startup_state == waitfor_control_request_signal || stop_on_either) {
      pthrd_printf("Received attach SIGNAL\n");
      NotifyMessage *notify_msg = static_cast<NotifyMessage *>(data);
      int_thread *thrd = threadPool()->initialThread();
      thrd->getUserState().setStateProc(int_thread::stopped);      
      assert(notify_msg->type.signal.reason == NotifySignal_Generic);
      assert(notify_msg->type.signal.signum == SIGSTOP);
      got_startup_stop = true;
      free(notify_msg);
      if (stop_on_either) {
         startup_state = waitfor_data_collection;
         return true;
      }
      else
         startup_state = waits_done;
   }

   /**
    * Receive the data from the above query command.
    **/
   if (startup_state == waitfor_data_collection || data_on_either) {
      pthrd_printf("Handling getProcessDataAck for %d\n", getPid());
      QueryAckMessage *query_ack = static_cast<QueryAckMessage *>(data);
      assert(query_ack->numCommands == 4);
      GetThreadDataAckCmd *tdata = NULL;

      for (unsigned i = 0; i < (unsigned) query_ack->numCommands; i++) {
         char *cmd_ptr = ((char *) data) + query_ack->cmdList[i].offset;
         if (query_ack->cmdList[i].type == GetProcessDataAck) {
            get_procdata_result = *(GetProcessDataAckCmd *) cmd_ptr;
            procdata_result_valid = true;
         }
         else if (query_ack->cmdList[i].type == GetAuxVectorsAck)
            get_auxvectors_result = *(GetAuxVectorsAckCmd *) cmd_ptr;
         else if (query_ack->cmdList[i].type == GetThreadListAck) {
            if (!get_thread_list) 
               get_thread_list = new GetThreadListAckCmd();
            *get_thread_list = *(GetThreadListAckCmd *) cmd_ptr;
         }
         else if (query_ack->cmdList[i].type == GetThreadDataAck) {
            tdata = (GetThreadDataAckCmd *) cmd_ptr;
            BG_Thread_ToolState ts = tdata->toolState;
            stopped_on_startup = ((ts == Suspend) || (ts == HoldSuspend));
            held_on_startup = ((ts == Hold) || (ts == HoldSuspend));
            pthrd_printf("On startup, found stopped: %s, held: %s\n",
                         stopped_on_startup ? "true" : "false",
                         held_on_startup ? "true" : "false");
         }
      }

      if (stopped_on_startup || held_on_startup) {
         debugger_stopped = true;
         debugger_suspended = true;
      }

      if (data_on_either && (stopped_on_startup || held_on_startup)) {
         pthrd_printf("Attaching to already stopped tool.  Skipping wait for SIGSTOP\n");
         startup_state = waits_done;
      }
      else if (data_on_either) {
         pthrd_printf("Got data ack during startup.  Waiting for SIGSTOP\n");
         startup_state = waitfor_control_request_signal;
         return true;
      }
      else {
         startup_state = waits_done;
      }
   }

   if (startup_state == waits_done) {
      pthrd_printf("In state waits_done for %d\n", getPid());
      int_thread *thrd = threadPool()->initialThread();
      assert(thrd);

      setState(neonatal_intermediate);

      if (hasControlAuthority) {
         thrd->getStartupState().desyncStateProc(int_thread::stopped);
         if (stopped_on_startup || held_on_startup || got_startup_stop) {
            thrd->getGeneratorState().setStateProc(int_thread::stopped);
            thrd->getHandlerState().setStateProc(int_thread::stopped);
            thrd->getUserState().setStateProc(int_thread::stopped);
         }
      }
      else {
         thrd->getStartupState().desyncStateProc(int_thread::running);
         thrd->getHandlerState().setStateProc(int_thread::running);
      }
    
      if (get_thread_list->numthreads > 0) {
         setState(running);
         getStartupTeardownProcs().dec();
         startup_state = startup_done;
         thrd->changeLWP(get_thread_list->threadlist[0].tid);
      }
      else {
         //BG/Q didn't give us the thread info we need on attach.
         // single-step one instruction and try again.
         pthrd_printf("Got 0 threads in thread list.  Single stepping process %d\n", getPid());
         startup_state = step_insn;
         thrd->getStartupState().setState(int_thread::running);
         thrd->setSingleStepMode(true);
         if (held_on_startup) {
            thrd->setSuspended(true);
            held_on_startup = false;
         }
      }
      
      return true;
   }

   /**
    * If we didn't get the GetThreadList cmd accurately right off, then
    * step the app one instruction and try again.
    **/
   if (startup_state == step_insn) {
      pthrd_printf("In startup state step_insn for %d\n", getPid());
      int_thread *thrd = threadPool()->initialThread();
      if (hasControlAuthority)
         thrd->getStartupState().setState(int_thread::stopped);
      thrd->setSingleStepMode(false);

      GetThreadListCmd thread_list;
      thread_list.threadID = 0;
      sendCommand(thread_list, GetThreadList);

      startup_state = reissue_data_collection;
      return true;
   }

   if (startup_state == reissue_data_collection) {
      QueryAckMessage *query_ack = static_cast<QueryAckMessage *>(data);
      assert(query_ack->cmdList[0].type = GetThreadListAck);
      *get_thread_list = *(GetThreadListAckCmd *) (((char *) data) + query_ack->cmdList[0].offset);
      
      assert(get_thread_list->numthreads > 0);
      int_thread *thrd = threadPool()->initialThread();
      thrd->changeLWP(get_thread_list->threadlist[0].tid);
      
      setState(running);
      getStartupTeardownProcs().dec();

      startup_state = startup_done;
      return true;
   }
   

   if (startup_state == startup_done) {
      pthrd_printf("Handling state startup_done for %d\n", getPid());
      int_thread *thrd = threadPool()->initialThread();
      thrd->getStartupState().restoreStateProc();
      getStartupTeardownProcs().dec();
      setForceGeneratorBlock(false);
      
      if (held_on_startup) {
         int_threadPool *tp = threadPool();
         for (int_threadPool::iterator i = tp->begin(); i != tp->end(); i++) {
            int_thread *thrd = *i;
            pthrd_printf("Threads were held on startup.  Updating held state for %d/%d\n",
                         getPid(), thrd->getLWP());
            thrd->setSuspended(true);
         }
      }
      pthrd_printf("Startup done on %d\n", getPid());
      startup_state = startup_donedone;
      ReaderThread::get()->clearTimeout();
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

ComputeNode *bgq_process::getComputeNode() const
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

void bgq_process::bgq_getProcCoordinates(unsigned &a, unsigned &b, unsigned &c, unsigned &d, unsigned &e, unsigned &t) const
{
   a = get_procdata_result.aCoord;
   b = get_procdata_result.bCoord;
   c = get_procdata_result.cCoord;
   d = get_procdata_result.dCoord;
   e = get_procdata_result.eCoord;
   t = get_procdata_result.tCoord;
}

unsigned int bgq_process::bgq_getComputeNodeID() const
{
   return getComputeNode()->getID();
}

void bgq_process::bgq_getSharedMemRange(Dyninst::Address &start, Dyninst::Address &end) const
{
   start = get_procdata_result.sharedMemoryStartAddr;
   end = get_procdata_result.sharedMemoryEndAddr;
}

void bgq_process::bgq_getPersistantMemRange(Dyninst::Address &start, Dyninst::Address &end) const
{
   start = get_procdata_result.persistMemoryStartAddr;
   end = get_procdata_result.persistMemoryEndAddr;
}

void bgq_process::bgq_getHeapMemRange(Dyninst::Address &start, Dyninst::Address &end) const
{
   start = get_procdata_result.heapStartAddr;
   end = get_procdata_result.heapEndAddr;
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
   unsigned int resp_id = 0;
   bool use_respid = false;
   if (resp) {
      resp_id = resp->getID() + resp_mod;
      use_respid = true;
   }
   if (msg_type == Query) {
      return query_transaction->writeCommand(&cmd, cmd_type, resp_id, use_respid);
   }
   else if (msg_type == Update) {
      return update_transaction->writeCommand(&cmd, cmd_type, resp_id, use_respid);
   }
   else {
      assert(0); //Are we trying to send an Ack?
   }
   return false;
}

bool bgq_process::sendCommand(const ToolCommand &cmd, uint16_t cmd_type, Resp::ptr resp)
{
   uint16_t msg_type = getCommandMsgType(cmd_type);
   if (msg_type == Query) {
      return query_transaction->writeCommand(&cmd, cmd_type, resp->getID(), true);
   }
   else if (msg_type == Update) {
      return update_transaction->writeCommand(&cmd, cmd_type, resp->getID(), true);
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
   bgq_thread *bgthr = dynamic_cast<bgq_thread *>(thr);
   assert(bgthr);

   thr_cmd.threadID = thr->getLWP();

   bool result = sendCommand(thr_cmd, GetThreadData, stk_resp);
   if (!result) {
      setLastError(err_internal, "Error while asking for thread data\n");
      perr_printf("Could not send GetThreadData to %d/%d\n", 
                  thr->llproc()->getPid(), thr->getLWP());
      return false;
   }

   struct timeval stackwalk_timeout;
   stackwalk_timeout.tv_sec = stackwalk_timeout_sec;
   stackwalk_timeout.tv_usec = 0;
   ReaderThread::get()->setTimeout(stackwalk_timeout);
   
   //Needed to track to know when to free the ArchEventBGQ objects
   // associated with the stackwalks.
   thr->pendingStackwalkCount().inc();
   bgthr->pending_stack_resp = stk_resp;

   return true;
}

bool bgq_process::plat_handleStackInfo(stack_response::ptr stk_resp, CallStackCallback *cbs)
{
   int_thread *t = stk_resp->getThread();
   bgq_thread *bgt = dynamic_cast<bgq_thread *>(t);
   assert(t);
   Thread::ptr thr = t->thread();
   bool had_error = false, result;
   GetThreadDataAckCmd *thrdata;
   
   ReaderThread::get()->clearTimeout();
   
   if (!t->pendingStackwalkCount().local()) {
      /* If this hits, maybe we got an event after it'd timed out and we'd given up*/
      pthrd_printf("Received stackwalk message on %d/%d when there are no pending stackwalks\n",
                   getPid(), t->getLWP());
      had_error = true;
      goto done;
   }
   t->pendingStackwalkCount().dec();
   bgt->pending_stack_resp = stack_response::ptr();

   if (stk_resp->hasError()) {
      setState(errorstate);
      perr_printf("Error getting thread data on %d/%d\n", t->llproc()->getPid(), thr->getLWP());
      setLastError(err_internal, "Error receiving thread data\n");
      had_error = true;
      goto done;
   }

   thrdata = static_cast<GetThreadDataAckCmd *>(stk_resp->getData());
   assert(stk_resp);

   pthrd_printf("Handling response with call stack %d/%d\n", t->llproc()->getPid(), t->getLWP());

   bgt->last_stack_start = thrdata->stackStartAddr;
   bgt->last_stack_end = thrdata->stackCurrentAddr;

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
   if (!Counter::global(Counter::PendingStackwalks)) {
      for (set<void *>::iterator i = held_msgs.begin(); i != held_msgs.end(); i++) {
         free(*i);
      }
      held_msgs.clear();
   }
   return !had_error;
}

bool bgq_process::plat_getFileDataAsync(int_eventAsyncFileRead *fileread)
{
   if (!fileread->orig_size)
      fileread->orig_size = MaxMemorySize;

   FileReadResp_t *readresp = new FileReadResp_t(fileread, this);
   readresp->post();

   GetFileContentsCmd getfile;
   getfile.threadID = 0;
   strncpy(getfile.pathname, fileread->filename.c_str(), MaxPersistPathnameSize);
   getfile.offset = fileread->offset;
   getfile.numbytes = fileread->orig_size;
   
   pthrd_printf("Sending GetFileContents for %s on %d from %lu to %lu\n", 
                getfile.pathname, getPid(), fileread->offset, fileread->offset + fileread->orig_size);
   bool result = sendCommand(getfile, GetFileContents, readresp);
   rotateTransaction();
   return result;
}

bool bgq_process::plat_getFileStatData(std::string filename, Dyninst::ProcControlAPI::stat64_ptr *stat_results,
                                       std::set<StatResp_t *> &resps)
{
   GetFileStatDataCmd filestat;
   filestat.threadID = 0;
   strncpy(filestat.pathname, filename.c_str(), MaxPersistPathnameSize);
   
   StatResp_t *statresp = new StatResp_t(stat_results, this);
   assert(statresp);
   statresp->post();

   pthrd_printf("Sending GetFileStatDataCmd for %s to %d\n", filename.c_str(), getPid());
   bool result = sendCommand(filestat, GetFileStatData, statresp);
   if (!result) {
      perr_printf("Failed to send STAT command\n");
      delete statresp;
      return false;
   }
   rotateTransaction();   
   resps.insert(statresp);
   return true;
}

bool bgq_process::plat_getFileNames(FileSetResp_t *resp)
{
   GetFilenamesCmd fnames;
   fnames.threadID = 0;

   resp->post();

   pthrd_printf("Sending getfilenames to %d\n", getPid());
   bool result = sendCommand(fnames, GetFilenames, resp);
   if (!result) {
      perr_printf("Failed to send GetFilenames command\n");
      return false;
   }
   rotateTransaction();
   return true;
}

string int_process::plat_canonicalizeFileName(std::string path)
{
   char *result = realpath(path.c_str(), NULL);
   if (result) {
      string sresult(result);
      free(result);
      return sresult;
   }
   else {
      return path;
   }
}

int bgq_process::getMaxFileReadSize()
{
   return MaxMemorySize;
}

bool bgq_process::fillInMemQuery(MemUsageResp_t *resp)
{
   assert(procdata_result_valid);

   switch (cur_memquery) {
      case no_memquery:
         assert(0);
      case shared_memquery:
         *(resp->get()) = get_procdata_result.sharedMemoryEndAddr - get_procdata_result.sharedMemoryStartAddr;
         pthrd_printf("Getting shared memory size for process %d: %lx to %lx = %lu bytes\n", getPid(),
                      get_procdata_result.sharedMemoryStartAddr, 
                      get_procdata_result.sharedMemoryEndAddr,
                      *resp->get());
         break;
      case stack_memquery:
         assert(0);
         break;
      case heap_memquery:
         *resp->get() = (get_procdata_result.heapBreakAddr - get_procdata_result.heapStartAddr);
         pthrd_printf("Getting heap memory size for process %d: %lx to %lx = %lu bytes\n", getPid(),
                      get_procdata_result.heapStartAddr,
                      get_procdata_result.heapBreakAddr,
                      get_procdata_result.heapBreakAddr - get_procdata_result.heapStartAddr);

         //mmap region doesn't measure what we want -- show the unmapped regions
         /*
         *resp->get() += get_procdata_result.mmapEndAddr - get_procdata_result.mmapStartAddr);
         pthrd_printf("Getting mmap memory size for process %d: %lx to %lx = %lu bytes\n", getPid(),
                      get_procdata_result.mmapStartAddr,
                      get_procdata_result.mmapEndAddr,
                      get_procdata_result.mmapEndAddr - get_procdata_result.mmapStartAddr);
         */
         break;
   }
   resp->done();
   return true;
}

bool bgq_process::handleMemQuery(MemUsageResp_t *resp)
{
   resp->post();

   if (procdata_result_valid)
      return fillInMemQuery(resp);

   pthrd_printf("Sending GetProcessDataCmd for memory query for %d\n", getPid());
   GetProcessDataCmd pdata;
   pdata.threadID = 0;
   bool result = sendCommand(pdata, GetProcessData, resp);
   if (!result) {
      perr_printf("Error sending get process data command\n");
      return false;
   }
   rotateTransaction();

   return true;
}

bool bgq_process::plat_getStackUsage(MemUsageResp_t *resp)
{
   pthrd_printf("Accumulating stack usage for %d\n", getPid());
   unsigned long stack_usage = 0;
   for (int_threadPool::iterator i = threadpool->begin(); i != threadpool->end(); i++) {
      bgq_thread *thr = dynamic_cast<bgq_thread *>(*i);
      stack_usage += (thr->last_stack_start - thr->last_stack_end);
      pthrd_printf("Getting stack memory size for %d/%d: %lx to %lx: %lu bytes\n", getPid(), thr->getLWP(),
                   thr->last_stack_start, thr->last_stack_end,
                   thr->last_stack_start - thr->last_stack_end);
   }
   *resp->get() = stack_usage;
   resp->done();
   return true;
}

bool bgq_process::plat_getHeapUsage(MemUsageResp_t *resp)
{
   cur_memquery = heap_memquery;
   return handleMemQuery(resp);
}

bool bgq_process::plat_getSharedUsage(MemUsageResp_t *resp)
{
   cur_memquery = shared_memquery;
   return handleMemQuery(resp);
}

bool bgq_process::plat_residentNeedsMemVals()
{
   return true;
}

bool bgq_process::plat_getResidentUsage(unsigned long stacku, unsigned long heapu, unsigned long sharedu,
                                        MemUsageResp_t *resp)
{
   resp->post();

   unsigned long memory_used = 0;
   unsigned int procs_per_cn = getComputeNode()->numRanksOwned();
   
   memory_used += stacku;
   memory_used += heapu;
   memory_used += sharedu / procs_per_cn;
   
   *resp->get() = memory_used;

   resp->done();

   return true;
}

bool bgq_process::allowSignal(int signal_no)
{
   dyn_sigset_t mask = getSigMask();
   return sigismember(&mask, signal_no);   
}

set<void *> bgq_process::held_msgs;

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
   unwinder(NULL),
   last_ss_addr(0)
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
   int_thread *signal_thrd = NULL, *resumed_thread = NULL;
   int cont_signal = 0;
   set<int_thread *> ss_threads;
   bgq_process *proc = dynamic_cast<bgq_process *>(llproc());

   proc->debugger_suspended = false;

   vector<bgq_thread *> resuming_threads;
   int_threadPool *tp = proc->threadPool();
   for (int_threadPool::iterator i = tp->begin(); i != tp->end(); i++) {
      bgq_thread *t = dynamic_cast<bgq_thread *>(*i);
      bool is_suspended = t->isSuspended();
      if (t->last_signaled && !is_suspended) {
         signal_thrd = t;
         t->last_signaled = false;
      }
      if (!is_suspended && !resumed_thread) {
         resumed_thread = t;
      }
      if (!is_suspended) {
         resuming_threads.push_back(t);
      }
      if (t->continueSig_) {
         cont_signal = t->continueSig_;
         t->continueSig_ = 0;
      }
      if (t->singleStep()) {
         ss_threads.insert(t);
      }
      else {
         t->last_ss_addr = 0;
      }
   }

   if (cont_signal) {
      if (!signal_thrd) {
         pthrd_printf("The signaled thread is suspended, selecting a resumed thread to re-signal\n");
         signal_thrd = resumed_thread;
      }
      assert(signal_thrd);
      
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
   }

   if (ss_threads.empty()) {
      pthrd_printf("Sending ContinueProcess command to %d\n", proc->getPid());
      for (vector<bgq_thread *>::iterator i = resuming_threads.begin(); i != resuming_threads.end(); i++)
         (*i)->clearCachedPC();
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

   pthrd_printf("%lu threads of process %d are in single step, selecting thread for single step\n",
                (unsigned long) ss_threads.size(), proc->getPid());
   bgq_thread *ss_thread = NULL;
   if (ss_threads.size() > 1 && proc->last_ss_thread) {
      /**
       * Multiple threads are single-stepping at once, but the BG/Q debug interface only allows us to 
       * step one at a time.  We'll remember what thread we stepped last and rotate the next thread
       * that gets stepped with each call.
       **/
      for (set<int_thread *>::iterator i = ss_threads.begin(); i != ss_threads.end(); i++) {
         if (*i != static_cast<int_thread *>(proc->last_ss_thread))
            continue;
         i++;
         if (i == ss_threads.end())
            i = ss_threads.begin();
         ss_thread = proc->last_ss_thread = dynamic_cast<bgq_thread *>(*i);
         break;
      }
   }
   if (!ss_thread) {
      ss_thread = proc->last_ss_thread = dynamic_cast<bgq_thread *>(*ss_threads.begin());
   }
   assert(ss_thread);

   StepThreadCmd step_thrd;
   pthrd_printf("Single stepping thread %d/%d\n", proc->getPid(), ss_thread->getLWP());
   ss_thread->clearCachedPC();
   if (ss_thread->getLWP() != -1)
      step_thrd.threadID = ss_thread->getLWP();
   else
      step_thrd.threadID = 0;
   bool result = proc->sendCommand(step_thrd, StepThread);
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

bool bgq_thread::plat_getRegisterAsync(Dyninst::MachRegister reg, reg_response::ptr resp)
{
   Address addr;
   bool result = haveCachedPC(addr);
   //This only works (and should only trigger) if we can return a cached PC.
   assert(reg.isPC());
   assert(result);

   resp->setResponse((Dyninst::MachRegisterVal) addr);
   
   return true;
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

bool bgq_thread::plat_convertToSystemRegs(const int_registerPool &pool, unsigned char *output_buffer, bool /*gpr_only*/)
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
std::map<int, ComputeNode *> ComputeNode::rank_to_cn;

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

bool ComputeNode::fillInAllRanksOwned()
{
   static bool done = false;
   if (done)
      return true;
   done = true;
   
   pthrd_printf("Filling in ranks_owned for each CN\n");
   char rank_dir[64];
   snprintf(rank_dir, 64, "/jobs/%lu/toolctl_rank", bgq_process::getJobID());   

   DIR *dir = opendir(rank_dir);
   if (!dir) {
      perr_printf("Error opening directory %s\n", rank_dir);
      return false;
   }

   struct dirent *dent;
   while ((dent = readdir(dir)) != NULL) {
      if (!dent->d_name || *dent->d_name < '0' || *dent->d_name > '9')
         continue;
      uint32_t rank = atoi(dent->d_name);
      ComputeNode *cn = getComputeNodeByRank(rank);
      if (!cn)
         continue;
      cn->ranks_owned.insert(rank);
   }

   closedir(dir);
   return true;
}

ComputeNode *ComputeNode::getComputeNodeByRank(uint32_t rank)
{
   map<int, ComputeNode *>::iterator j = rank_to_cn.find(rank);
   if (j != rank_to_cn.end())
      return j->second;

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

   ComputeNode *cn = getComputeNodeByID(i->second);
   rank_to_cn.insert(make_pair(rank, cn));
   return cn;
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
   ReaderThread::get()->addComputeNode(this);
   pthrd_printf("Successfully connected to CN %d on FD %d\n", cid, fd);
}

ComputeNode::~ComputeNode()
{
   ReaderThread::get()->rmComputeNode(this);
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

int ComputeNode::numRanksOwned()
{
   bool result = fillInAllRanksOwned();
   if (!result)
      return false;
   if (ranks_owned.empty())
      return false;
   return ranks_owned.size();
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
#if defined(USE_THREADED_IO)
   printMessage(msg, "Writing");
   buffer_t newmsg;
   newmsg.size = msg->header.length;
   if (heap_alloced) {
      newmsg.buffer = msg;
   }
   else {
      newmsg.buffer = malloc(newmsg.size);
      memcpy(newmsg.buffer, msg, newmsg.size);
   }
   newmsg.is_heap_allocated = true;

   WriterThread::get()->writeMessage(newmsg, proc->getComputeNode());
   return true;
#else
   ScopeLock<> lock(send_lock);

   if (have_pending_message) {
      pthrd_printf("Enqueuing message while another is pending\n");
      buffer_t newmsg;
      newmsg.size = msg->header.length;
      if (heap_alloced) {
         newmsg.buffer = msg;
      }
      else {
         newmsg.buffer = malloc(newmsg.size);
         memcpy(newmsg.buffer, msg, newmsg.size);
      }
      newmsg.is_heap_allocated = true;
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
#endif
}

bool ComputeNode::flushNextMessage()
{
   ScopeLock<> lock(send_lock);
   assert(have_pending_message);

   if (queued_pending_msgs.empty()) {
      have_pending_message = false;
      return true;
   }

   buffer_t buffer = queued_pending_msgs.front();
   queued_pending_msgs.pop();
   
   ToolMessage *msg = (ToolMessage *) buffer.buffer;
   printMessage(msg, "Writing");
   bool result = reliableWrite(msg, buffer.size);
   pthrd_printf("Flush wrote message of size %u to FD %d, result is %s\n", msg->header.length,
                fd, result ? "true" : "false");
   free(buffer.buffer);
   if (!result) {
      pthrd_printf("Failed to flush message to compute-node %d\n", cn_id);
      have_pending_message = false;
      return false;
   }
   return true;
}

bool ComputeNode::handleMessageAck()
{
#if !defined(USE_THREADED_IO)
   return flushNextMessage();
#else
   return true;
#endif
}

bool ComputeNode::writeToolAttachMessage(bgq_process *proc, ToolMessage *msg, bool heap_alloced)
{
#if !defined(USE_THREADED_IO)
   if (all_attach_done || !do_all_attach) {
      return writeToolMessage(proc, msg, heap_alloced);
   }

   ScopeLock<> lock(attach_lock);

   if (!all_attach_done) {
      have_pending_message = true;
   }
   return writeToolMessage(proc, msg, heap_alloced);
#else
   return writeToolMessage(proc, msg, heap_alloced);
#endif
}

void ComputeNode::removeNode(bgq_process *proc) {
   set<bgq_process *>::iterator i = procs.find(proc);
   if (i == procs.end())
      return;

   procs.erase(i);
   former_procs.push_back(proc->getPid());
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

HandlePreControlAuthority::HandlePreControlAuthority() :
   Handler("BGQ Pre-ControlAuthority")
{
}

HandlePreControlAuthority::~HandlePreControlAuthority()
{
}

void HandlePreControlAuthority::getEventTypesHandled(vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::Pre, EventType::ControlAuthority));
}

Handler::handler_ret_t HandlePreControlAuthority::handleEvent(Event::ptr ev)
{
   EventControlAuthority::ptr ca = ev->getEventControlAuthority();
   int_eventControlAuthority *iev = ca->getInternalEvent();
   int_process *proc = ca->getProcess()->llproc();
   bgq_process *bgproc = dynamic_cast<bgq_process *>(proc);

   pthrd_printf("Handling pre-controlauthority event on %d.  "
                "Other tool is priority %d, name %s.  We are priority %d\n",
                proc->getPid(), iev->priority, iev->toolname.c_str(), bgproc->priority);

   if (iev->trigger == EventControlAuthority::ControlNoChange) {
      return ret_success;
   }
   bool gained = (iev->trigger == EventControlAuthority::ControlGained);

   if (!iev->unset_desync) {
      ev->setSyncType(Event::sync_process);
      int_thread *thr = ca->getThread()->llthrd();
      thr->getControlAuthorityState().restoreStateProc();
      iev->unset_desync = true;
   }
   /**
    * If we've been allowed control authority, then submit the request
    **/
   if (gained && !iev->took_ca) {
      //Create a ControlMessage that requests control, and setup an async response object
      if (!iev->dresp) {
         pthrd_printf("Constructing ControlMessage in HandlePreControlAuthority handler\n");
         ControlMessage *ctrl_msg = (ControlMessage *) malloc(sizeof(ControlMessage));
         sigset_t sigs = bgproc->getSigMask();
         sigaddset(&sigs, SIGTRAP);
         sigaddset(&sigs, SIGSTOP);
         ctrl_msg->notifySignalSet(&sigs);
         ctrl_msg->sndSignal = SIGSTOP;
         ctrl_msg->dynamicNotifyMode = DynamicNotifyDLoader;
         ctrl_msg->dacTrapMode = DACTrap_NoChange;
         
         iev->dresp = data_response::createDataResponse();
         bgproc->fillInToolMessage(*ctrl_msg, Control, iev->dresp);

         getResponses().lock();
         bool result = bgproc->getComputeNode()->writeToolMessage(bgproc, ctrl_msg, true);
         if (result) {
            getResponses().addResponse(iev->dresp, bgproc);
         }
         getResponses().unlock();
         getResponses().noteResponse();
         if (!result) {
            perr_printf("Failed to write tool message to CN for process %d\n", bgproc->getPid());
            return ret_error;
         }
      }

      //Wait for async response to control request
      if (!iev->dresp->isReady()) {
         pthrd_printf("Postponing taking control authority as we wait for ControlAck on %d\n", bgproc->getPid());
         bgproc->handlerPool()->notifyOfPendingAsyncs(iev->dresp, ev);
         return ret_async;
      }

      //Have the ControlAck response.  Handle.
      ControlAckMessage *msg = (ControlAckMessage *) iev->dresp->getData();
      
      if (msg->controllingToolId == bgproc->getToolID()) {
         //Will mark haveControlAuthority after we get the SIGSTOP
         pthrd_printf("Yeah.  ControlAckMessage gave us control of %d\n", bgproc->getPid());
         iev->took_ca = true;
         free(msg);
      }
      else {
         char toolTag_cstr[ToolTagSize+1];
         bzero(toolTag_cstr, ToolTagSize+1);
         memcpy(toolTag_cstr, msg->toolTag, ToolTagSize);
         iev->toolname = toolTag_cstr;
         iev->toolid = msg->controllingToolId;
         iev->priority = msg->priority;
         iev->trigger = EventControlAuthority::ControlNoChange;
         pthrd_printf("Someone else grabbed Control Authority of us on %d: name = %s, id = %u, priority = %u\n",
                      bgproc->getPid(), iev->toolname.c_str(), iev->toolid, iev->priority);
         free(msg);
         return ret_success;
      }
   }

   if (gained && !iev->waiting_on_stop) {
      //We've got CA.  The system should still throw us a SIGSTOP now.
      //We'll decode that SIGSTOP into a new CA event with the same int_eventControlAuthority
      pthrd_printf("Returning from CA handler as we wait for the SIGSTOP\n");
      bgproc->stopwait_on_control_authority = iev;
      iev->dont_delete = true;
      iev->waiting_on_stop = true;
      ev->setSuppressCB(true);
      return ret_success;
   }

   /**
    * Disable or Resume breakpoints, depending on whether we're releasing or
    * taking control authority.  
    **/
   const char *action_str = gained ? "resuming" : "suspending";
   set<response::ptr> &async_responses = iev->async_responses;
   if (!iev->handled_bps) {
      pthrd_printf("%s all breakpoints before control authority release\n", action_str);
      for (std::map<Dyninst::Address, sw_breakpoint *>::iterator i = proc->memory()->breakpoints.begin();
           i != proc->memory()->breakpoints.end(); i++)
      {
         sw_breakpoint *bp = i->second;
         bool result;
         if (gained) 
            result = bp->resume(proc, async_responses);
         else
            result = bp->suspend(proc, async_responses);
         if (!result) {
            perr_printf("Error %s breakpoint at %lx in %d\n", action_str, i->first, bgproc->getPid());
            ev->setLastError(err_internal, "Error handling breakpoints before control authority transfer\n");
            return ret_error;
         }
      }
      iev->handled_bps = true;
   }

   pthrd_printf("Checking if breakpoint %s has completed for %d in control release handler\n",
                action_str, proc->getPid());
   for (set<response::ptr>::iterator i = async_responses.begin(); i != async_responses.end(); i++) {
      if ((*i)->hasError()) {
         perr_printf("Error while %s breakpoint\n", action_str);
         ev->setLastError(err_internal, "Error handling breakpoints during control authority transfer\n");
         return ret_error;
      }
      if (!(*i)->isReady()) {
         pthrd_printf("Suspending control authority handler due to async in breakpoint %s.\n", action_str);
         proc->handlerPool()->notifyOfPendingAsyncs(async_responses, ev);
         return ret_async;
      }
   }

   //TODO: When HW breakpoints are supported on BGQ, handle those too.

   if (!gained) {
      //This will cause the post-controlauthority handler to run when the process is next continued.
      bgproc->pending_control_authority = ca;
   }
   else {
      pthrd_printf("Marking ourselves as having CA\n");
      bgproc->hasControlAuthority = true;
   }

   return ret_success;
}

HandlePostControlAuthority::HandlePostControlAuthority() :
   Handler("BGQ Post-ControlAuthority")
{
}

HandlePostControlAuthority::~HandlePostControlAuthority()
{
}

void HandlePostControlAuthority::getEventTypesHandled(vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::Post, EventType::ControlAuthority));
}

Handler::handler_ret_t HandlePostControlAuthority::handleEvent(Event::ptr ev)
{
   //This triggers when the user continues the process after we're notified
   // of a control change.  Just send the ReleaseControl message
   EventControlAuthority::ptr ca = ev->getEventControlAuthority();
   int_process *proc = ca->getProcess()->llproc();
   bgq_process *bgproc = dynamic_cast<bgq_process *>(proc);
   pthrd_printf("Handling PostControlAuthority for %d\n", proc->getPid());

   int_thread *thr = ca->getThread()->llthrd();
   thr->getControlAuthorityState().restoreStateProc();

   bgproc->hasControlAuthority = false;
   bgproc->pending_control_authority = EventControlAuthority::ptr();

   pthrd_printf("Sending continue command to %d for control authority release\n", proc->getPid());
   SetContinuationSignalCmd cont_signal_cmd;
   cont_signal_cmd.threadID = 0;
   cont_signal_cmd.signum = 0;
   bool result = bgproc->sendCommand(cont_signal_cmd, SetContinuationSignal);
   if (!result) {
      pthrd_printf("Error sending continue command\n");
      return ret_error;
   }

   for (int_threadPool::iterator i = bgproc->threadPool()->begin(); i != bgproc->threadPool()->end(); i++) {
      int_thread *thr = *i;
      if (thr->isSuspended()) {
         ReleaseThreadCmd release_thrd;
         release_thrd.threadID = thr->getLWP();
         bool result = bgproc->sendCommand(release_thrd, ReleaseThread);
         if (!result) {
            pthrd_printf("Error sending ReleaseThread command\n");
            return ret_error;
         }
      }
   }

   ContinueProcessCmd cont_process;
   cont_process.threadID = 0;
   bgproc->sendCommand(cont_process, ContinueProcess);

   pthrd_printf("Sending control authority release command to %d\n", proc->getPid());
   ReleaseControlCmd release;
   release.threadID = 0;
   release.notify = ReleaseControlNotify_Active;
   result = bgproc->sendCommand(release, ReleaseControl);
   if (!result) {
      perr_printf("Error sending release control command to %d\n", proc->getPid());
      return ret_error;
   }

   for (int_threadPool::iterator i = proc->threadPool()->begin(); i != proc->threadPool()->end(); i++) {
      int_thread *thrd = *i;
      thrd->getUserState().setState(int_thread::running);
      thrd->getHandlerState().setState(int_thread::running);
      thrd->getGeneratorState().setState(int_thread::running);
   }
   ProcPool()->condvar()->lock();
   ProcPool()->condvar()->broadcast();
   ProcPool()->condvar()->unlock();
   return ret_success;
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
#if defined(USE_THREADED_IO)
   return readMessage(events, block);
#else
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
#endif
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

bool GeneratorBGQ::readMessage(vector<ArchEvent *> &events, bool block)
{
   for (;;) {
      buffer_t buf = ReaderThread::get()->readNextElement(block);
      if (buf.buffer == NULL && block && !buf.is_timeout) {
         pthrd_printf("Failed to read from ReaderThread\n");
         return false;
      }
      if (buf.buffer == NULL && !buf.is_timeout) {
         return true;
      }
      ArchEventBGQ *newArchEvent;
      if (buf.is_timeout) {
         pthrd_printf("Read timeout message\n");
         newArchEvent = new ArchEventBGQ();
      }
      else {
         ToolMessage *tm = (ToolMessage *) buf.buffer;
         printMessage(tm, "Read");
         newArchEvent = new ArchEventBGQ(tm);
      }
      assert(newArchEvent);
      events.push_back(newArchEvent);
      block = false;
   }
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
/*   bool result = getResponses().hasAsyncPending(false) ||
                 Counter::globalCount(Counter::AsyncEventCounts);
   if (result) {
      pthrd_printf("Async events pending, skipping generator block\n");
   }
   return result;*/
   return true;
}

void GeneratorBGQ::kick()
{
   int result;
   int kval = kick_val;
   do {
      result = write(kick_pipe[1], &kval, sizeof(int));
   } while (result == -1 && errno == EINTR);
   ReaderThread::get()->kick_generator();
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
   free_msg(true),
   timeout_msg(false)
{
}

ArchEventBGQ::ArchEventBGQ() :
   msg(NULL),
   free_msg(false),
   timeout_msg(true)
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

bool ArchEventBGQ::isTimeout() const
{
   return timeout_msg;
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

bool DecoderBlueGeneQ::decodeTimeout(vector<Event::ptr> &events) 
{
   //Check for timeouts during startup.  Timeouts aren't process specific, so
   // we need to decode the single timeout archevent into any process doing an
   // op that could timeout.
   pthrd_printf("Checking for processes affected by read timeout\n");
   const set<ComputeNode *> cns = ComputeNode::allNodes();
   for (set<ComputeNode *>::const_iterator i = cns.begin(); i != cns.end(); i++) {
      const set<bgq_process *> &procs = (*i)->getProcs();
      for (set<bgq_process *>::const_iterator j = procs.begin(); j != procs.end(); j++) {
         bgq_process *proc = *j;
         switch (proc->startup_state) {
            case bgq_process::waitfor_attach:
            case bgq_process::waitfor_control_request_ack:
            case bgq_process::waitfor_data_or_stop:
            case bgq_process::waitfor_control_request_signal:
            case bgq_process::waitfor_data_collection: 
            case bgq_process::step_insn:
            case bgq_process::reissue_data_collection:
            {
               pthrd_printf("Process %d timed out during startup state (%d)\n",
                            proc->getPid(), (int) proc->startup_state);
               Event::ptr new_ev = EventIntBootstrap::ptr(new EventIntBootstrap(&timeout_val));
               new_ev->setProcess(proc->proc());
               new_ev->setThread(Thread::ptr());
               new_ev->setSyncType(Event::async);
               events.push_back(new_ev);
               break;
            }
            case bgq_process::waitfor_control_request_notice: {
               pthrd_printf("Process %d timed out waiting for control authority (%d)\n",
                            proc->getPid(), (int) proc->startup_state);
               Event::ptr new_ev = EventIntBootstrap::ptr(new EventIntBootstrap(&no_control_authority_val));
               new_ev->setProcess(proc->proc());
               new_ev->setThread(Thread::ptr());
               new_ev->setSyncType(Event::async);
               events.push_back(new_ev);
               break;
            }
            default:
               break;
         }
      }

      if (Counter::globalCount(Counter::PendingStackwalks)) {
         for (set<bgq_process *>::const_iterator j = procs.begin(); j != procs.end(); j++) {
            bgq_process *proc = *j;
            for (int_threadPool::iterator k = proc->threadPool()->begin(); k != proc->threadPool()->end(); k++) {
               int_thread *thr = *k;
               if (thr->pendingStackwalkCount().local()) {
                  pthrd_printf("Thread %d/%d timed out while waiting for stackwalk\n",
                               proc->getPid(), thr->getLWP());
                  bgq_thread *bgthr = dynamic_cast<bgq_thread *>(thr);
                  assert(bgthr);
                  
                  stack_response::ptr sresp = bgthr->pending_stack_resp;
                  assert(sresp);

                  getResponses().lock();

                  sresp->markError(err_dstack);
                  sresp->postResponse(NULL);

                  map<Event::ptr, EventAsync::ptr> resp_map;
                  Event::ptr newev = decodeCompletedResponse(sresp, proc, thr, resp_map);
                  if (newev)
                     events.push_back(newev);

                  getResponses().signal();
                  getResponses().unlock();
               }
            }
         }
      }
      WriterThread::get()->forcePastAck(*i);
   }

   return true;
}

Event::ptr DecoderBlueGeneQ::decodeCompletedResponse(response::ptr resp, int_process *proc, int_thread *thrd,
                                                     map<Event::ptr, EventAsync::ptr> &async_evs)
{
   if (resp->isMultiResponse() && !resp->isMultiResponseComplete())
      return Event::ptr();

   Event::ptr ev = resp->getEvent();
   if (!ev) {
      pthrd_printf("Marking response %s/%d ready\n", resp->name().c_str(), resp->getID());
      resp->markReady();

      int_eventAsyncIO *ioev = resp->getAsyncIOEvent();
      if (!ioev) {
         return Event::ptr();
      }

      pthrd_printf("Decoded to User-level AsyncIO event.  Creating associated Event with ioev %p\n", ioev);
      EventAsyncIO::ptr newev;
      switch (ioev->iot) {
         case int_eventAsyncIO::memread:
            newev = EventAsyncRead::ptr(new EventAsyncRead(ioev));
            break;
         case int_eventAsyncIO::memwrite:
            newev = EventAsyncWrite::ptr(new EventAsyncWrite(ioev));
            break;
         case int_eventAsyncIO::regallread:
            newev = EventAsyncReadAllRegs::ptr(new EventAsyncReadAllRegs(ioev));
            break;
         case int_eventAsyncIO::regallwrite:
            newev = EventAsyncSetAllRegs::ptr(new EventAsyncSetAllRegs(ioev));
            break;
      }
      assert(newev);
      newev->setSyncType(Event::async);
      newev->setProcess(proc->proc());
      newev->setThread(thrd ? thrd->thread() : Thread::ptr());
      return newev;
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

bool DecoderBlueGeneQ::decodeLWPRefresh(ArchEventBGQ *, bgq_process *proc, ToolCommand *cmd, 
                                        vector<Event::ptr> &)
{
   proc->lwp_tracking_resp = result_response::ptr();
   if (!proc->get_thread_list)
      proc->get_thread_list = new GetThreadListAckCmd();
   *proc->get_thread_list = *((GetThreadListAckCmd *) cmd);
   return true;
}

bool DecoderBlueGeneQ::decodeFileContents(ArchEventBGQ *ev, bgq_process *proc, ToolCommand *cmd, 
                                          int rc, unsigned int resp_id, bool owns_msg,
                                          std::vector<Event::ptr> &events)
{
   bool is_complete;
   Resp_ptr r = proc->recvResp(resp_id, is_complete);
   assert(r && is_complete);
   
   FileReadResp_t *resp = static_cast<FileReadResp_t *>(r);
   int_eventAsyncFileRead *iev = resp->get();
   iev->resp = resp;

   GetFileContentsAckCmd *file_cmd_ack = static_cast<GetFileContentsAckCmd *>(cmd);
   iev->data = file_cmd_ack->data;
   iev->size = file_cmd_ack->numbytes;

   if (owns_msg) {
      ev->dontFreeMsg();
      iev->to_free = ev->getMsg();
   }

   if (rc != 0) {
      iev->errorcode = rc;
      proc->setLastError(err_internal, "Error from CDTI while reading file");
   }

   EventAsyncFileRead::ptr newev = EventAsyncFileRead::ptr(new EventAsyncFileRead(iev));
   newev->setProcess(proc->proc());
   newev->setThread(proc->threadPool()->initialThread()->thread());
   newev->setSyncType(Event::async);
   events.push_back(newev);

   resp->done();

   return true;
}

bool DecoderBlueGeneQ::decodeGetFilenames(ArchEventBGQ *, bgq_process *proc, ToolCommand *cmd, int rc, int id)
{
   bool is_complete;
   Resp_ptr r = proc->recvResp(id, is_complete);
   assert(r && is_complete);
   FileSetResp_t *fset_resp = static_cast<FileSetResp_t *>(r);
   FileSet *fset = fset_resp->get();
   
   GetFilenamesAckCmd *filelist = static_cast<GetFilenamesAckCmd *>(cmd);
   Process::ptr ui_proc = proc->proc();

   if (rc != 0) {
      proc->setLastError(err_internal, "Error from CDTI while getting filelist");
      fset_resp->done();
      return true;
   }

   for (uint32_t i = 0; i < filelist->numFiles; i++) {
      fset->insert(make_pair(ui_proc, FileInfo(string(filelist->pathname[i]))));
   }

   fset_resp->done();
   return true;
}

bool DecoderBlueGeneQ::decodeFileStat(ToolCommand *cmd, bgq_process *proc, unsigned int resp_id, int rc)
{
   bool is_complete;
   Resp_ptr r = proc->recvResp(resp_id, is_complete);
   assert(r && is_complete);
   StatResp_t *resp = static_cast<StatResp_t *>(r);
   GetFileStatDataAckCmd *statack = static_cast<GetFileStatDataAckCmd *>(cmd);
      
   stat64_ptr *statp = resp->get();
   if (rc != 0) {
      if (*statp)
         delete *statp;
      *statp = NULL;
      proc->setLastError(err_notfound, "Error stat'ing file from CDTI");
   }
   else {
      if (!*statp)
         *statp = new stat64_ret_t;
      memcpy(*statp, &statack->stat, sizeof(stat64_ret_t));
   }

   resp->done();

   return true;
}

bool DecoderBlueGeneQ::decodeMemoryQuery(ArchEventBGQ * /*ev*/, bgq_process *proc, ToolCommand *cmd, unsigned int resp_id, int /*rc*/)
{
   assert(proc->cur_memquery != bgq_process::no_memquery);
   proc->get_procdata_result = *static_cast<GetProcessDataAckCmd *>(cmd);
   proc->procdata_result_valid = true;

   bool is_complete;
   Resp_ptr r = proc->recvResp(resp_id, is_complete);
   assert(r && is_complete);
   MemUsageResp_t *resp = static_cast<MemUsageResp_t *>(r);

   proc->fillInMemQuery(resp);

   proc->cur_memquery = bgq_process::no_memquery;
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

bool DecoderBlueGeneQ::decodeControlAck(ArchEventBGQ *ev, bgq_process *proc, vector<Event::ptr> &events)
{
   if (proc->startup_state == bgq_process::waitfor_control_request_ack) {
      //Control ACKs during startup are handled by the start-up handler.
      pthrd_printf("Decoded ControlAck to startup event\n");
      return decodeStartupEvent(ev, proc, ev->getMsg(), Event::async, events);
   }

   pthrd_printf("Decoding ControlACK message response for %d\n", proc->getPid());
   ControlAckMessage *msg = static_cast<ControlAckMessage *>(ev->getMsg());
   ev->dontFreeMsg();

   // These ControlAck message always come in a response set of size 1, since they
   // don't pair with other messages.
   uint32_t seq_id = msg->header.sequenceId;
   ResponseSet *resp_set = ResponseSet::getResponseSetByID(seq_id);
   assert(resp_set);
   
   map<Event::ptr, EventAsync::ptr> async_ev_map;
   getResponses().lock();
   bool found = false;
   unsigned int id = resp_set->getIDByIndex(0, found);
   assert(found);
   response::ptr resp = getResponses().rmResponse(id);
   data_response::ptr dresp = resp->getDataResponse();
   dresp->postResponse((void *) msg);
   Event::ptr new_ev = decodeCompletedResponse(dresp, proc, NULL, async_ev_map);
   assert(new_ev);
   events.push_back(new_ev);
   getResponses().signal();
   getResponses().unlock();

   delete resp_set;

   return true;
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
   
   return false;
}

bool DecoderBlueGeneQ::usesResp(uint16_t cmdtype) 
{ 
   switch (cmdtype) {
      case GetFilenamesAck:
      case GetFileStatDataAck:
      case GetFileContentsAck:
         return true;
      default:
         return false;
   }
}
 
bool DecoderBlueGeneQ::decodeUpdateOrQueryAck(ArchEventBGQ *archevent, bgq_process *proc,
                                              int num_commands, CommandDescriptor *cmd_list, 
                                              vector<Event::ptr> &events)
{
   map<Event::ptr, EventAsync::ptr> async_ev_map;
   ToolMessage *msg = archevent->getMsg();
   uint32_t seq_id = msg->header.sequenceId;
   bool resp_lock_held = false;
   bool owns_message = true;

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
      unsigned int id = 0;
      if (resp_set) {
         if (!usesResp(desc->type)) {
            getResponses().lock();
            resp_lock_held = true;
            bool found = false;
            id = resp_set->getIDByIndex(i, found);
            if (found) {
               pthrd_printf("Old resp, associated ACK %u and index %u with internal id %u\n", seq_id, i, id);
               cur_resp = getResponses().rmResponse(id);
            }
            if (!cur_resp) {
               resp_lock_held = false;
               getResponses().unlock();
            }
         }
         else
         {
            bool found = false;
            id = resp_set->getIDByIndex(i, found);
            if (found) {
               pthrd_printf("New resp, associated ACK %u and index %u with internal id %u\n", seq_id, i, id);
            }
         }
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
            
            Event::ptr new_ev = decodeCompletedResponse(allreg, proc, thr, async_ev_map);
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
                     new_ev = decodeCompletedResponse(indiv_reg, proc, thr, async_ev_map);
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
            mem_response::ptr memresp = cur_resp->getMemResponse();
            GetMemoryAckCmd *cmd = static_cast<GetMemoryAckCmd *>(base_cmd);
            assert(memresp);
            assert(cmd);
            pthrd_printf("Decoding GetMemoryAck of length %u on %d. Contents: %u %u %u %u...\n",
                         cmd->length, proc->getPid(),
                         cmd->length > 0 ? (unsigned int) cmd->data[0] : 0, 
                         cmd->length > 1 ? (unsigned int) cmd->data[1] : 0, 
                         cmd->length > 2 ? (unsigned int) cmd->data[2] : 0, 
                         cmd->length > 3 ? (unsigned int) cmd->data[3] : 0);
            memresp->postResponse((char *) cmd->data, cmd->length, cmd->addr);
            if (desc->returnCode)
               memresp->markError(desc->returnCode);

            Event::ptr new_ev = decodeCompletedResponse(memresp, proc, thr, async_ev_map);
            if (new_ev)
               events.push_back(new_ev);

            resp_lock_held = false;
            getResponses().signal();
            getResponses().unlock();
            break;
         }
         case GetThreadDataAck: {
            pthrd_printf("Decoded new event on %d to GetThreadDataAck message\n", proc->getPid());
            if (proc->startup_state < bgq_process::startup_done) {
               pthrd_printf("GetThreadDataAck is startup event\n");
               break;
            }
            stack_response::ptr stkresp = cur_resp->getStackResponse();
            GetThreadDataAckCmd *cmd = static_cast<GetThreadDataAckCmd *>(base_cmd);
            assert(stkresp);
            assert(cmd);
            archevent->dontFreeMsg();
            bgq_process::held_msgs.insert(msg);
            stkresp->postResponse((void *) cmd);
            if (desc->returnCode) 
               stkresp->markError(desc->returnCode);
            
            Event::ptr new_ev = decodeCompletedResponse(stkresp, proc, thr, async_ev_map);
            if (new_ev)
               events.push_back(new_ev);
            
            resp_lock_held = false;
            getResponses().signal();
            getResponses().unlock();
            break;
         }
         case GetThreadListAck:
            if (proc->lwp_tracking_resp) {
               pthrd_printf("Decoded GetThreadList as LWP refresh on %d\n", proc->getPid());
               decodeLWPRefresh(archevent, proc, base_cmd, events);
               result_response::ptr result_resp = cur_resp->getResultResponse();
               result_resp->postResponse(true);
               Event::ptr new_ev = decodeCompletedResponse(result_resp, proc, thr, async_ev_map);
               if (new_ev)
                  events.push_back(new_ev);
               resp_lock_held = false;
               getResponses().signal();
               getResponses().unlock();
               break;
            }
            else if (proc->startup_state == bgq_process::reissue_data_collection) {
               pthrd_printf("Decoded to lwp refresh during startup on %d\n", proc->getPid());
               return decodeStartupEvent(archevent, proc, archevent->getMsg(), Event::async, events);
            }
            else {
               pthrd_printf("Decoded GetThreadListAck on %d/%d. Dropping\n",
                            proc ? proc->getPid() : -1, thr ? thr->getLWP() : -1);
            }
            break;
         case GetAuxVectorsAck:
            pthrd_printf("Decoded GetAuxVectorsAck on %d/%d. Dropping\n", 
                         proc ? proc->getPid() : -1, thr ? thr->getLWP() : -1);
            break;
         case GetProcessDataAck:
            pthrd_printf("Decoded new event on %d to GetProcessData message\n", proc->getPid());
            if (proc->startup_state < bgq_process::startup_done)
               decodeStartupEvent(archevent, proc, msg, Event::async, events);
            else
               decodeMemoryQuery(archevent, proc, base_cmd, id, desc->returnCode);
            break;
         case GetFilenamesAck:
            pthrd_printf("Decoded event on %d to GetFilenamesAck\n", proc->getPid());
            decodeGetFilenames(archevent, proc, base_cmd, desc->returnCode, id);
            break;
         case GetPreferencesAck:
            assert(0); //Currently unused
            break;
         case GetFileStatDataAck:
            pthrd_printf("Decoded new event on %d to GetFileStatDataAck message\n", proc->getPid());
            decodeFileStat(base_cmd, proc, id, desc->returnCode);
            break;
         case GetFileContentsAck:
            pthrd_printf("Decoding GetFileContentsAck on %d\n", proc->getPid());
            decodeFileContents(archevent, proc, base_cmd, desc->returnCode, id, owns_message, events);
            owns_message = false;
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
               Event::ptr new_ev = decodeCompletedResponse(result_resp, proc, thr, async_ev_map);
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
            pthrd_printf("Decoded SendSignalAck on %d/%d. Dropping\n", 
                         proc ? proc->getPid() : -1, thr ? thr->getLWP() : -1);
            break;
         case ContinueProcessAck:
            pthrd_printf("Decoded ContinueProcessAck on %d/%d.\n", proc->getPid(), thr->getLWP());
            if (proc)
               proc->procdata_result_valid = false;
            break;
         case StepThreadAck:
            pthrd_printf("Decoded StepThreadAck on %d/%d. Dropping\n",
                         proc ? proc->getPid() : -1, thr ? thr->getLWP() : -1);
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
                                        Address addr, vector<Event::ptr> &events, bool allow_signal_decode)
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

   if (!allow_signal_decode)
      return false;

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
   if (proc->stopwait_on_control_authority) {
      pthrd_printf("Decoded SIGSTOP to control authority response\n");
      int_eventControlAuthority *iev = proc->stopwait_on_control_authority;
      EventControlAuthority::ptr newev = EventControlAuthority::ptr(new EventControlAuthority(EventType::Pre, iev));
      iev->dont_delete = false;
      proc->stopwait_on_control_authority = NULL;
      newev->setProcess(proc->proc());
      newev->setThread(thr->thread());
      newev->setSyncType(Event::sync_process);
      events.push_back(newev);
      return true;
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

bool DecoderBlueGeneQ::decodeStep(ArchEventBGQ *ae, bgq_process *proc,
                                  int_thread *t, Address addr,
                                  vector<Event::ptr> &events)
{
   bgq_thread *thr = dynamic_cast<bgq_thread *>(t);
   assert(thr->singleStep());

   if (thr->last_ss_addr == addr) {
      //We did a SS that didn't move.  Perhaps we've hit a BP.
      pthrd_printf("Single step did not move thread %d/%d.  Checking for BP\n",
                   proc->getPid(), thr->getLWP());
      bool result = decodeBreakpoint(ae, proc, thr, addr, events, false);
      if (result) {
         pthrd_printf("Decoded spinning single-step to a breakpoint\n");
         return true;
      }
   }
   thr->last_ss_addr = addr;
   
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
   else if (proc->startup_state == bgq_process::step_insn) {
      pthrd_printf("Decoded to single step during startup on %d\n", proc->getPid());
      return decodeStartupEvent(ae, proc, ae->getMsg(), Event::sync_process, events);
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

   bgq_thread *thr = dynamic_cast<bgq_thread *>(proc->threadPool()->findThreadByLWP(threadID));
   if (!thr && proc->startup_state != bgq_process::startup_done) {
      thr = dynamic_cast<bgq_thread *>(proc->threadPool()->initialThread());
   }
   assert(thr);

   Address instAddress = msg->type.signal.instAddress;
   if (instAddress) 
      thr->setCachedPC(instAddress);

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
         return decodeStep(archevent, proc, thr, insn_addr, events);
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

   NotifyMessage *notify = (NotifyMessage *) archevent->getMsg();
   char toolname[ToolTagSize+1];
   bzero(toolname, ToolTagSize+1);
   memcpy(toolname, notify->type.control.toolTag, ToolTagSize);
   EventControlAuthority::Trigger trigger;

   pthrd_printf("Decoded event to control authority notice.  Tag = %s, id = %u, priority = %d, reason = %s\n",
                toolname, notify->type.control.toolid, notify->type.control.priority,
                notify->type.control.reason == NotifyControl_Conflict ? "Conflict" : "Available");
   
   //Make a policy decision on whether to do nothing, release, or take control
   if (notify->type.control.reason == NotifyControl_Conflict) {
      if (notify->type.control.priority > proc->priority && proc->hasControlAuthority) {
         pthrd_printf("Requesting tool has higher priority, releasing control authority\n");
         trigger = EventControlAuthority::ControlLost;
      }
      else {
         pthrd_printf("Requesting tool has lower priority, keeping control authority\n");
         trigger = EventControlAuthority::ControlNoChange;
      }
   }
   else {
      if (proc->priority == QueryPriorityLevel) {
         pthrd_printf("Requesting tool is releasing authority, but we are not interested in taking it\n");
         trigger = EventControlAuthority::ControlNoChange;
      }
      else {
         pthrd_printf("Other tool is releasing control authority.  Taking\n");
         trigger = EventControlAuthority::ControlGained;
      }
   }
   
   int_eventControlAuthority *iev;
   iev = new int_eventControlAuthority(toolname,
                                       notify->type.control.toolid,
                                       notify->type.control.priority,
                                       trigger);

   EventControlAuthority::ptr new_ev = EventControlAuthority::ptr(new EventControlAuthority(EventType::Pre, iev));
   new_ev->setProcess(proc->proc());
   int_thread *initial_thread = proc->threadPool()->initialThread();
   new_ev->setThread(initial_thread->thread());
   new_ev->setSyncType(Event::async);
   events.push_back(new_ev);
   return true;
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
   bool ret_result = true;

   if (archevent->isTimeout()) {
      pthrd_printf("Decoding timeout\n");
      ret_result = decodeTimeout(events);
      delete archevent;
      return ret_result;
   }

   ToolMessage *msg = archevent->getMsg();
   struct MessageHeader *header = &msg->header;

   int_process *proc = ProcPool()->findProcByPid(header->rank);
   if (!proc)
      return false;
   if (proc->getState() == int_process::errorstate)
      return false;
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
         ret_result = decodeControlAck(archevent, qproc, events);
         break;
      case AttachAck:
         pthrd_printf("Decoding AttachAck on %d\n", proc->getPid());
         ret_result = decodeStartupEvent(archevent, qproc, msg, Event::async, events);
         break;
      case DetachAck:
         pthrd_printf("Decoding DetachAck on %d\n", proc->getPid());
         ret_result = decodeDetachAck(archevent, qproc, events);
         break;
      case QueryAck: {
         QueryAckMessage *qack = static_cast<QueryAckMessage *>(msg);
         uint16_t num_commands = qack->numCommands;
         CommandDescriptor *cmd_list = qack->cmdList;
         pthrd_printf("Decoding QueryAck from %d with %d commands\n", proc->getPid(), num_commands);
         ret_result = decodeUpdateOrQueryAck(archevent, qproc, num_commands, cmd_list, events);
         break;
      }
      case UpdateAck: {
         uint16_t num_commands = static_cast<UpdateAckMessage *>(msg)->numCommands;
         CommandDescriptor *cmd_list = static_cast<UpdateAckMessage *>(msg)->cmdList;
         pthrd_printf("Decoding UpdateAck from %d with %d commands\n", proc->getPid(), num_commands);
         ret_result = decodeUpdateOrQueryAck(archevent, qproc, num_commands, cmd_list, events);
         break;
      }
      case Notify:
         pthrd_printf("Decoding Notify on %d\n", proc->getPid());
         ret_result = decodeNotifyMessage(archevent, qproc, events);
         break;
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

   delete archevent;
   return ret_result;
}

HandlerPool *plat_createDefaultHandlerPool(HandlerPool *hpool)
{
   static bool init = false;
   static HandleBGQStartup *handle_startup = NULL;
   static HandlePreControlAuthority *handle_pre_control = NULL;
   static HandlePostControlAuthority *handle_post_control = NULL;
   if (!init) {
      handle_startup = new HandleBGQStartup();
      handle_pre_control = new HandlePreControlAuthority();
      handle_post_control = new HandlePostControlAuthority();
      init = true;
   }
   hpool->addHandler(handle_startup);
   hpool->addHandler(handle_pre_control);
   hpool->addHandler(handle_post_control);
   thread_db_process::addThreadDBHandlers(hpool);
   sysv_process::addSysVHandlers(hpool);
   return hpool;
}

bool ProcessPool::LWPIDsAreUnique()
{
   return false;
}

static volatile int on_sigusr2_hit;
static void on_sigusr2(int)
{
   on_sigusr2_hit = 1;
}

#ifndef SYS_tkill
#define SYS_tkill 208
#endif

static pid_t P_gettid()
{
  long int result = syscall(SYS_gettid);
  return (pid_t) result;
}


static bool t_kill(int pid, int sig)
{
  long int result = syscall(SYS_tkill, pid, sig);
  return (result == 0);
}

static void kick_thread(int pid, int tid)
{
   if (!pid || !tid)
      return;
   if (pid != getpid())
      return;

   struct sigaction newact, oldact;
   memset(&newact, 0, sizeof(struct sigaction));
   memset(&oldact, 0, sizeof(struct sigaction));
   newact.sa_handler = on_sigusr2;

   int result = sigaction(SIGUSR2, &newact, &oldact);
   if (result == -1) {
      int error = errno;
      perr_printf("Error signaling generator thread: %s\n", strerror(error));
      return;
   }
   on_sigusr2_hit = 0;
   bool bresult = t_kill(tid, SIGUSR2);
   while (bresult && !on_sigusr2_hit) {
      //Don't use a lock because pthread_mutex_unlock is not signal safe
      sched_yield();
   }

   result = sigaction(SIGUSR2, &oldact, NULL);
   if (result == -1) {
      int error = errno;
      perr_printf("Error signaling generator thread: %s\n", strerror(error));
   }      
}

extern void setXThread(long t);
extern void setRThread(long t);
extern void setWThread(long t);

IOThread::IOThread() :
   do_exit(false),
   kicked(false),
   init_done(false),
   shutdown_done(false),
   lwp(0),
   pid(0)
{
}

void IOThread::init()
{
   initLock.lock();
   bool result = thrd.spawn(IOThread::mainWrapper, this);
   assert(result);
   
   while (!init_done)
      initLock.wait();
   initLock.unlock();
}

IOThread::~IOThread()
{
   shutdown();
}

void IOThread::mainWrapper(void *param)
{
   IOThread *iothrd = static_cast<IOThread *>(param);
   iothrd->thrd_main();
}

void IOThread::thrd_main()
{
   lwp = P_gettid();
   pid = getpid();

   localInit();

   initLock.lock();
   init_done = true;
   initLock.signal();
   initLock.unlock();

   while (!do_exit) {
      kicked = false;
      run();
   }

   shutdownLock.lock();
   shutdown_done = true;
   shutdownLock.signal();
   shutdownLock.unlock();
}

void IOThread::thrd_kick()
{
}

void IOThread::kick()
{
   kicked = true;
   if (emergency)
      kick_thread(pid, lwp);
   thrd_kick();
}

void IOThread::shutdown()
{
   if (do_exit || shutdown_done)
      return;

   shutdownLock.lock();
   do_exit = true;
   kick();
   while (!shutdown_done)
      shutdownLock.wait();
   shutdownLock.unlock();
   thrd.join();
}

ReaderThread *ReaderThread::me = NULL;
ReaderThread::ReaderThread() :
   kick_fd(-1),
   kick_fd_write(-1),
   is_gen_kicked(false),
   timeout_set(0)
{
   assert(!me);
   me = this;
   
   int pipe_fds[2];
   int result = pipe(pipe_fds);
   if (result != -1) {
      kick_fd = pipe_fds[0];
      kick_fd_write = pipe_fds[1];
   }
}

ReaderThread::~ReaderThread()
{
   me = NULL;
   if (kick_fd != -1)
      close(kick_fd);
   if (kick_fd_write != -1)
      close(kick_fd_write);
}

void ReaderThread::run()
{
   static vector<int> cur_fds;
   fd_set readfds;
   int nfds = 0;
   int num_fds = 0;
   FD_ZERO(&readfds);
   cur_fds.clear();

   /* Setup the FD list for select.  Block if there's no FDs ready yet */
   {
      pthrd_printf("Creating FD list in reader thread\n");
      fd_lock.lock();
      while (fds.empty()) {
         if (kicked) {
            fd_lock.unlock();
            return;
         }
         fd_lock.wait();
      }
      for (set<int>::iterator i = fds.begin(); i != fds.end(); i++) {
         int fd = *i;
         FD_SET(fd, &readfds);
         cur_fds.push_back(fd);
         if (fd > nfds) nfds = fd;
         num_fds++;
      }
      fd_lock.unlock();
      if (kick_fd != -1) {
         nfds = (nfds > kick_fd) ? nfds : kick_fd;
         FD_SET(kick_fd, &readfds);
         num_fds++;
      }
      pthrd_printf("Created FD list with %d elements.  Calling select.\n", num_fds);
   }

   /* Call select on the FD list */
   {
      struct timeval tv, *ptv = NULL;
      if (timeout_set) {
         tv = timeout;
         ptv = &tv;
      }
      
      int result = select(nfds+1, &readfds, NULL, NULL, ptv);
      pthrd_printf("Select returned %d\n", result);
      if (result == -1 && errno == EINTR) {
         pthrd_printf("Reader thread kicked.\n");
         return;
      }
      if (result == -1 && errno == EBADF) {
         pthrd_printf("Reader thread got EBADF on select return.  Trying again.\n");
         return;
      }
      assert(result != -1);
      if (result == 0) {
         if (!timeout_set) {
            //Timeout has been disabled since it was set.  Treat as kick.
            return;
         }
         //Actual timeout.
         buffer_t timeout_msg;
         timeout_msg.is_timeout = true;
         queue_lock.lock();
         msgs.push(timeout_msg);
         queue_lock.signal();
         queue_lock.unlock();
         return;
      }
   }

   /* Read data from the FDs that have any */
   bool been_kicked = false;
   if (kick_fd != -1 && FD_ISSET(kick_fd, &readfds)) {
      pthrd_printf("Reader thread kicked\n");
      char c;
      read(kick_fd, &c, 1);
      return;
   }
   for (vector<int>::iterator i = cur_fds.begin(); i != cur_fds.end(); i++) {
      int fd = *i;
      if (!FD_ISSET(fd, &readfds))
         continue;
      
      /* Read message header */
      MessageHeader hdr;
      size_t bytes_read = 0;
      pthrd_printf("Reading header from fd %d\n", fd);
      while (bytes_read < sizeof(MessageHeader)) {
         int result = read(fd, ((unsigned char *) &hdr) + bytes_read, sizeof(MessageHeader) - bytes_read);
         if (result == -1 && errno == EINTR) {
            pthrd_printf("Reader thread was kicked.  Finishing read first.\n");
            been_kicked = true;
            continue;
         }
         if (result == -1) {
            int error = errno;
            perr_printf("Failed to read from FD %d: %s\n", fd, strerror(error));
            globalSetLastError(err_internal, "Failed to read from CDTI file descriptor");
            return;
         }
         bytes_read += result;
      }
      assert(hdr.length <= SmallMessageDataSize);
      
      /* Allocate message buffer */
      unsigned char *message = (unsigned char *) malloc(hdr.length);
      assert(message);
      memcpy(message, &hdr, sizeof(MessageHeader));
      unsigned char *message_body = message + sizeof(MessageHeader);
      size_t remaining_bytes = hdr.length - sizeof(MessageHeader);
      
      /* Read remaining amount of message */
      bytes_read = 0;
      pthrd_printf("Reading message from fd %d\n", fd);
      while (bytes_read < remaining_bytes) {
         int result = read(fd, message_body + bytes_read, remaining_bytes - bytes_read);
         if (result == -1 && errno == EINTR) {
            pthrd_printf("Reader thread was kicked.  Finishing read first.\n");
            been_kicked = true;
            continue;
         }
         else if (result == -1) {
            int error = errno;
            perr_printf("Failed to read from FD %d: %s\n", fd, strerror(error));
            globalSetLastError(err_internal, "Failed to read from CDTI file descriptor");
            return;
         }
         bytes_read += result;
      }

      /* Create buffer_t object and put it into the queue */
      queue_lock.lock();
      msgs.push(buffer_t((void *) message, hdr.length, true));
      queue_lock.signal();
      queue_lock.unlock();
      
      /* If this is an ACK message, then let the writer thread know that it may
         be free to write now.*/
      switch (hdr.type) {
         case ErrorAck:
         case AttachAck:
         case DetachAck:
         case QueryAck:
         case UpdateAck:
         case SetupJobAck:
         case NotifyAck:
         case ControlAck:
            pthrd_printf("Notifying writer of Ack\n");
            WriterThread::get()->notifyAck(hdr.rank);
            break;
         default:
            break;
      }
      
      if (been_kicked) return;
   }
}

void ReaderThread::localInit()
{
   setRThread(DThread::self());
}

buffer_t ReaderThread::readNextElement(bool block)
{
   queue_lock.lock();
   while (msgs.empty() && !is_gen_kicked) {
      if (!block) {
         queue_lock.unlock();
         return buffer_t(NULL, 0, false);
      }
      queue_lock.wait();
   }
   
   if (is_gen_kicked) {
      is_gen_kicked = false;
      buffer_t ret;
      queue_lock.unlock();
      return ret;
   }
   buffer_t ret = msgs.front();
   msgs.pop();
   queue_lock.unlock();
   return ret;
}

void ReaderThread::addComputeNode(ComputeNode *cn)
{
   int fd = cn->getFD();
   fd_lock.lock();
   fds.insert(fd);
   fd_lock.signal();
   fd_lock.unlock();
   kick();
}

void ReaderThread::rmComputeNode(ComputeNode *cn)
{
   int fd = cn->getFD();
   fd_lock.lock();
   set<int>::iterator i = fds.find(fd);
   assert(i != fds.end());
   fds.erase(i);
   fd_lock.signal();
   fd_lock.unlock();
}

ReaderThread *ReaderThread::get()
{
   if (!me) {
      me = new ReaderThread();
      assert(me);
      me->init();
   }
   return me;
}

void ReaderThread::thrd_kick()
{
   char c = 'k';
   write(kick_fd_write, &c, 1);

   fd_lock.lock();
   fd_lock.signal();
   fd_lock.unlock();
}

void ReaderThread::kick_generator()
{
   queue_lock.lock();
   is_gen_kicked = true;
   queue_lock.broadcast();
   queue_lock.unlock();
}

void ReaderThread::setTimeout(const struct timeval &tv)
{
   bool was_timeout_set;
   pthrd_printf("Setting timeout to %ld sec + %ld usec\n", (long) tv.tv_sec, tv.tv_usec);
   timeout_lock.lock();
   was_timeout_set = (timeout_set != 0);
   timeout_set++;
   timeout_lock.unlock();
   if (!was_timeout_set) {
      timeout = tv;
      thrd_kick();
   }
}

void ReaderThread::clearTimeout()
{
   pthrd_printf("Clearing timeout\n");
   timeout_lock.lock();
   assert(timeout_set != 0);
   timeout_set--;
   timeout_lock.unlock();
}

WriterThread *WriterThread::me = NULL;
WriterThread::WriterThread()
{
   assert(!me);
   me = this;
}

WriterThread::~WriterThread()
{
   me = NULL;
}

WriterThread *WriterThread::get() {
   if (!me) {
      me = new WriterThread();
      assert(me);
      me->init();
   }
   return me;
}

void WriterThread::forcePastAck(ComputeNode *cn)
{
   if (!cn->have_pending_message) {
      return;
   }

   msg_lock.lock();
   //A timeout occured and we want to move the CN past its missing ACK.
   // We'll grab a process from this CN (any process, doesn't matter)
   // and pretend we got an ack for it.  That'll move the CN onto the next
   // message.
   rank_lock.lock();
   set<bgq_process *>::iterator i = cn->procs.begin();
   if (i != cn->procs.end()) {
      bgq_process *proc = *i;
      acks.push_back(proc->getPid());
   }
   rank_lock.unlock();
   msg_lock.signal();
   msg_lock.unlock();
}

void WriterThread::run()
{
   static vector<int> acks_to_handle;
   static vector<ComputeNode *> writes_to_handle;

   /* Get the work todo from the global vectors into our local vectors*/
   if (acks_to_handle.empty() && writes_to_handle.empty())
   {
      pthrd_printf("Waiting for acks or writes\n");
      msg_lock.lock();
      while (acks.empty() && writes.empty() && !kicked) {
         msg_lock.wait();
      }
      if (kicked) {
         msg_lock.unlock();
         pthrd_printf("Kick on writer thread\n");
         return;
      }
      acks_to_handle = acks;
      writes_to_handle = writes;
      acks.clear();
      writes.clear();
      msg_lock.unlock();
      pthrd_printf("Got %lu acks and %lu writes to handle\n", acks_to_handle.size(), writes_to_handle.size());
   }

   /* Turn all ACKs into potential writes. */
   if (!acks_to_handle.empty()) {
      rank_lock.lock();
      for (vector<int>::iterator i = acks_to_handle.begin(); i != acks_to_handle.end(); i++) {
         int rank = *i;
         map<int, ComputeNode *>::iterator j = rank_to_cn.find(rank);
         if (j == rank_to_cn.end())
            continue;
         ComputeNode *cn = j->second;
         cn->have_pending_message = false;
         pthrd_printf("Ack on CN %d\n", cn->getID());
         writes_to_handle.push_back(cn);
      }
      rank_lock.unlock();
      acks_to_handle.clear();
   }

   /* Check the writes for anything that's good to go. */
   for (vector<ComputeNode *>::iterator i = writes_to_handle.begin(); i != writes_to_handle.end(); i++) {
      /* Get message for this ComputeNode */
      ComputeNode *cn = *i;
      if (cn->have_pending_message) {
         pthrd_printf("CN %d not ready to write, has pending messages\n", cn->getID());
         continue;
      }
      cn->pending_queue_lock.lock();
      if (cn->queued_pending_msgs.empty()) {
         pthrd_printf("No queued messages to write to CN %d\n", cn->getID());
         cn->pending_queue_lock.unlock();
         continue;
      }
      buffer_t buf = cn->queued_pending_msgs.front();
      cn->queued_pending_msgs.pop();
      cn->have_pending_message = true;
      cn->pending_queue_lock.unlock();
      pthrd_printf("Writing to CN %d, have_pending_message set to true\n", cn->getID());
      
      /* Write message on pipe */
      printMessage((ToolMessage *) buf.buffer, "Flushing", true);
      bool got_kicked = false;
      size_t bytes_written = 0;      
      while (bytes_written < buf.size) {
         int result = write(cn->getFD(), ((unsigned char *) buf.buffer) + bytes_written,
                            buf.size - bytes_written);
         int error = errno;
         if (result == -1 && error == EINTR) {
            pthrd_printf("Kick on writer thread\n");
            got_kicked = true;
            continue;
         }
         else if (result == -1) {
            perr_printf("Failed to write data to CN %d: %s\n", cn->getID(), strerror(error));
            globalSetLastError(err_internal, "Failed to write to CDTI file descriptor");
            break;
         }
         bytes_written += result;
      }

      if (buf.is_heap_allocated) {
         free(buf.buffer);
      }
      if (got_kicked) break;
   }
   writes_to_handle.clear();
}

void WriterThread::localInit()
{
   setWThread(DThread::self());
}

void WriterThread::writeMessage(buffer_t buf, ComputeNode *cn)
{
   cn->pending_queue_lock.lock();
   cn->queued_pending_msgs.push(buf);
   cn->pending_queue_lock.unlock();

   msg_lock.lock();
   writes.push_back(cn);
   msg_lock.signal();
   msg_lock.unlock();
}

void WriterThread::notifyAck(int rank)
{
   msg_lock.lock();
   acks.push_back(rank);
   msg_lock.signal();
   msg_lock.unlock();
}

void WriterThread::addProcess(bgq_process *proc) {
   int rank = proc->getPid();
   ComputeNode *cn = proc->getComputeNode();
   rank_lock.lock();
   rank_to_cn.insert(make_pair(rank, cn));
   rank_lock.unlock();
}

void WriterThread::rmProcess(bgq_process *proc) {
   int rank = proc->getPid();
   rank_lock.lock();
   map<int, ComputeNode *>::iterator i = rank_to_cn.find(rank);
   assert(i != rank_to_cn.end());
   rank_to_cn.erase(i);
   rank_lock.unlock();
}

void WriterThread::thrd_kick() {
   msg_lock.lock();
   msg_lock.broadcast();
   msg_lock.unlock();
}

DebugThread *DebugThread::me;

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

static void registerIfDefault(int sig, struct sigaction *act)
{
   struct sigaction orig;
   sigaction(sig, NULL, &orig);
   if (orig.sa_handler != SIG_DFL)
      return;
   sigaction(sig, act, NULL);
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

   registerIfDefault(SIGSEGV, &action);
   registerIfDefault(SIGBUS, &action);
   registerIfDefault(SIGABRT, &action);
   registerIfDefault(SIGILL, &action);
   registerIfDefault(SIGQUIT, &action);
   registerIfDefault(SIGTERM, &action);
}

void ComputeNode::emergencyShutdown()
{
   char message[8192];

   emergency = true;
   pthrd_printf("Shutting down generator\n");
   GeneratorBGQ *gen = static_cast<GeneratorBGQ *>(Generator::getDefaultGenerator());
   gen->shutdown();
   pthrd_printf("Done shutting down generator\n");
#if defined(USE_THREADED_IO)
   pthrd_printf("Shutting down Reader thread\n");
   ReaderThread::get()->shutdown();
   pthrd_printf("Shutting down Writer thread\n");
   WriterThread::get()->shutdown();
   pthrd_printf("Done shutting down IO threads\n");
#endif
   pthrd_printf("all_compute_nodes.size() = %u\n", (unsigned) all_compute_nodes.size());

   vector<int> pids;
   pids.reserve(64); //At most 64 procs per CN--system limit

   for (set<ComputeNode *>::iterator i = all_compute_nodes.begin(); i != all_compute_nodes.end(); i++) {
      ComputeNode *cn = *i;
      if (!cn) {
         pthrd_printf("Skipping empty compute node\n");
         continue;
      }
      pthrd_printf("cn->procs.size() = %u\n", (unsigned) cn->procs.size());
      pids.clear();
      for (set<bgq_process *>::iterator j = cn->procs.begin(); j != cn->procs.end(); j++) {
         bgq_process *proc = *j;
         if (!proc) {
            pthrd_printf("Skipping NULL proc\n");
            continue;
         }
         pthrd_printf("Process %d is in startup_state %d on CN %d\n", 
                      proc->getPid(), (int) proc->startup_state, proc->getComputeNode()->getID());
         pids.push_back(proc->getPid());
      }
      for (vector<int>::iterator j = cn->former_procs.begin(); j != cn->former_procs.end(); j++) {
         pthrd_printf("Process %d is former process on CN %d\n", *j, cn->getID());
         pids.push_back(*j);
      }
      for (vector<int>::iterator j = pids.begin(); j != pids.end(); j++) {
        again: {
            int pid = *j;
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
