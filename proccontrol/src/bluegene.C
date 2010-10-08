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

#include "proccontrol/h/Event.h"
#include "proccontrol/h/Handler.h"
#include "proccontrol/h/Mailbox.h"

#include "proccontrol/src/bluegene.h"
#include "proccontrol/src/int_event.h"
#include "proccontrol/src/int_handler.h"

#include "common/h/SymLite-elf.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>

using namespace DebuggerInterface;
using namespace Dyninst;
using namespace ProcControlAPI;
using namespace std;

static BG_GPR_Num_t DynToBGGPRReg(Dyninst::MachRegister reg);
static Dyninst::MachRegister BGGPRToDynReg(BG_GPR_Num_t bg_regnum);
static bool BGSend(BG_Debugger_Msg &msg);
static bool bg_fdHasData(int fd);

static void register_for_crash(int pid);
static void remove_for_crash(int pid);
static void on_crash(int sig);

#define EXIT_TYPE_EXIT 0
#define EXIT_TYPE_SIGNAL 1

#define NUM_GPRS 41

ArchEventBlueGene::ArchEventBlueGene(BG_Debugger_Msg *m) :
   msg(m)
{
}

ArchEventBlueGene::~ArchEventBlueGene()
{
   if (msg) {
      delete msg;
      msg = NULL;
   }
}

BG_Debugger_Msg *ArchEventBlueGene::getMsg() const
{
   return msg;
}

GeneratorBlueGene::GeneratorBlueGene() :
   GeneratorMT(std::string("BlueGene Generator")),
   dpr(NULL)
{
   if (dyninst_debug_proccontrol) {
      pthrd_printf("Creating debug port reader\n");
      dpr = new DebugPortReader();
   }

   decoders.insert(new DecoderBlueGene());
}

GeneratorBlueGene::~GeneratorBlueGene()
{
}

bool GeneratorBlueGene::initialize()
{
   int prot, phys, virt;
   pthrd_printf("Initializing GeneratorBlueGene\n");
   bg_process::getVersionInfo(prot, phys, virt);
   return true;
}

bool GeneratorBlueGene::canFastHandle()
{
   return false;
}

bool GeneratorBlueGene::plat_skipGeneratorBlock()
{
   bool result = getResponses().hasAsyncPending(false);
   if (result) {
      pthrd_printf("Async events pending, skipping generator block\n");
   }
   return result;
}

ArchEvent *GeneratorBlueGene::getEvent(bool block)
{
   BG_Debugger_Msg *msg;
   
   if (!block && !bg_fdHasData(BG_DEBUGGER_READ_PIPE)) {
      return NULL;
   }
   
   msg = new BG_Debugger_Msg();
   // Read an event from the debug filehandle.
   pthrd_printf("Reading event from BG file descriptor %d\n", BG_DEBUGGER_READ_PIPE);
   bool result = BG_Debugger_Msg::readFromFd(BG_DEBUGGER_READ_PIPE, *msg);
   if (!result) {
      pthrd_printf("Failure waiting for debug event\n");
      delete msg;
      return NULL;
   }
   
   // extract process and thread id from the BG message.
   pid_t pid = msg->header.nodeNumber;   
   THR_ID tid = msg->header.thread;
   int returnCode = msg->header.returnCode;
   pthrd_printf("Received debug event %s from pid %d, tid %d, rc %d\n",
             BG_Debugger_Msg::getMessageName(msg->header.messageType), pid, tid, returnCode);
   if (dyninst_debug_proccontrol) {
      BG_Debugger_Msg::dump(*msg, pctrl_err_out);
   }
   
   if (returnCode > 0) {
      // Print out a message if we get a return code we don't expect.  Consult the debugger header
      // for the meanings of these.
      pthrd_printf("Warning. Return code for %s on pid %d, tid %d was non-zero: %d\n",
                BG_Debugger_Msg::getMessageName(msg->header.messageType), pid, tid,
                msg->header.returnCode);
   }

   ArchEvent *aevent = new ArchEventBlueGene(msg);
   assert(aevent);
   return aevent;
}

Generator *Generator::getDefaultGenerator()
{
   static GeneratorBlueGene *gen = NULL;
   if (!gen) {
      gen = new GeneratorBlueGene();
      assert(gen);
      gen->launch();
   }
   return static_cast<Generator *>(gen);
}

DecoderBlueGene::DecoderBlueGene()
{
}

DecoderBlueGene::~DecoderBlueGene()
{
}

unsigned DecoderBlueGene::getPriority() const
{
   return default_priority;
}

bool DecoderBlueGene::getProcAndThread(ArchEventBlueGene *archbg, bg_process* &proc_out, bg_thread* &thread_out)
{
   int_process *proc = NULL;
   int_thread *thread = NULL;

   proc = ProcPool()->findProcByPid(archbg->getMsg()->header.nodeNumber);
   if (proc) {
      thread = proc->threadPool()->findThreadByLWP(archbg->getMsg()->header.thread);
   }

   proc_out = dynamic_cast<bg_process *>(proc);
   thread_out = static_cast<bg_thread *>(thread);
   return true;
}

Event::ptr DecoderBlueGene::decodeGetRegAck(BG_Debugger_Msg *msg)
{
   pthrd_printf("Decode get reg ack\n");
   getResponses().lock();
   
   response::ptr resp = getResponses().rmResponse(msg->header.sequence);
   assert(resp);
   reg_response::ptr reg_resp = resp->getRegResponse();
   assert(reg_resp);
   reg_resp->postResponse(msg->dataArea.GET_REG_ACK.value);
         
   if (msg->header.returnCode > 0)
      resp->markError(msg->header.returnCode);
   
   Event::ptr ret = decodeAsyncAck(resp);

   getResponses().signal();
   getResponses().unlock();

   return ret;
}

Event::ptr DecoderBlueGene::decodeGetAllRegAck(BG_Debugger_Msg *msg)
{
   pthrd_printf("Decoding get all reg ack\n");
   getResponses().lock();
   
   response::ptr resp = getResponses().rmResponse(msg->header.sequence);
   assert(resp);
   allreg_response::ptr areg_resp = resp->getAllRegResponse();
   assert(areg_resp);

   int_registerPool *pool = areg_resp->getRegPool();
   BG_GPR_t *reg_values = (BG_GPR_t *) &(msg->dataArea.GET_ALL_REGS_ACK.gprs);
   pool->regs.clear();
   for (unsigned i=0; i<NUM_GPRS; i++) {
      Dyninst::MachRegister dyn_reg = BGGPRToDynReg((BG_GPR_Num_t) i);
      if (dyn_reg == Dyninst::InvalidReg)
         continue;
      pool->regs[dyn_reg] = reg_values[i];
   }

   areg_resp->postResponse();
         
   if (msg->header.returnCode > 0)
      resp->markError(msg->header.returnCode);
   
   Event::ptr ret = decodeAsyncAck(resp);

   getResponses().signal();
   getResponses().unlock();

   return ret;
}

Event::ptr DecoderBlueGene::decodeGetMemAck(BG_Debugger_Msg *msg)
{
   pthrd_printf("Decoding get mem ack\n");
   getResponses().lock();
   
   response::ptr resp = getResponses().rmResponse(msg->header.sequence);
   assert(resp);
   mem_response::ptr mem_resp = resp->getMemResponse();
   assert(mem_resp);

   mem_resp->postResponse((char *) msg->dataArea.GET_MEM_ACK.data, msg->dataArea.GET_MEM_ACK.len);
         
   if (msg->header.returnCode > 0)
      resp->markError(msg->header.returnCode);
   
   Event::ptr ret = decodeAsyncAck(resp);

   getResponses().signal();
   getResponses().unlock();

   return ret;
}

Event::ptr DecoderBlueGene::decodeResultAck(BG_Debugger_Msg *msg)
{
   pthrd_printf("Decoding result ack\n");
   getResponses().lock();
   
   response::ptr resp = getResponses().rmResponse(msg->header.sequence);
   assert(resp);
   result_response::ptr result_resp = resp->getResultResponse();
   assert(result_resp);

   bool has_error = msg->header.returnCode;
   result_resp->postResponse(!has_error);
         
   if (has_error)
      resp->markError(msg->header.returnCode);
   
   Event::ptr ret = decodeAsyncAck(resp);

   getResponses().signal();
   getResponses().unlock();

   return ret;   
}

Event::ptr DecoderBlueGene::decodeAsyncAck(response::ptr resp)
{
   Event::ptr ev = resp->getEvent();
   if (!ev) {
      pthrd_printf("Marking response %s/%d ready\n", resp->name().c_str(), resp->getID());
      resp->markReady();
      return Event::ptr();
   }

   pthrd_printf("Creating new EventAsync over %s for response %s/%d",
                ev->name().c_str(), resp->name().c_str(), resp->getID());
   int_eventAsync *internal = new int_eventAsync(resp);
   EventAsync::ptr async_ev(new EventAsync(internal));
   async_ev->setProcess(ev->getProcess());
   async_ev->setThread(ev->getThread());
   async_ev->setSyncType(Event::async);
   async_ev->addSubservientEvent(ev);

   return async_ev;
}

bool DecoderBlueGene::decode(ArchEvent *archE, std::vector<Event::ptr> &events)
{
   pthrd_printf("Decoding event\n");
   assert(archE);
   ArchEventBlueGene *archbg = static_cast<ArchEventBlueGene *>(archE);
   Event::ptr event = Event::ptr();

   BG_Debugger_Msg *msg = archbg->getMsg();
   
   bg_process *proc = NULL;
   bg_thread *thread = NULL;
   bool result = getProcAndThread(archbg, proc, thread);
   if (!result)
      return false;
   if (!thread) 
      thread = static_cast<bg_thread *>(proc->threadPool()->initialThread());
   
   Event::ptr new_event;
   switch (msg->header.messageType) {
      case GET_REG_ACK:
         new_event = decodeGetRegAck(msg);
         break;
      case GET_ALL_REGS_ACK:
         new_event = decodeGetAllRegAck(msg);
         break;
      case GET_MEM_ACK:
         new_event = decodeGetMemAck(msg);
         break;
      case SET_REG_ACK:
      case SET_MEM_ACK:
         new_event = decodeResultAck(msg);
         break;
      case SINGLE_STEP_ACK:
         pthrd_printf("Decoded SINGLE_STEP_ACK, dropping...\n");
         break;
      case CONTINUE_ACK:
         pthrd_printf("Decoded CONTINUE_ACK, dropping...\n");
         break;
      case KILL_ACK:
         pthrd_printf("Decoded KILL_ACK, dropping...\n");
         break;
      case ATTACH_ACK:
         pthrd_printf("Decoded ATTACH_ACK, creating bootstrap event\n");
         assert(proc->getState() == int_process::neonatal_intermediate);
         new_event = EventIntBootstrap::ptr(new EventIntBootstrap());         
         new_event->setSyncType(Event::async);
         break;
      case DETACH_ACK:
         pthrd_printf("Decoded DETACH_ACK, creating Detached event\n");
         new_event = EventDetached::ptr(new EventDetached());
         new_event->setSyncType(Event::sync_process);
         break;
      case PROGRAM_EXITED: {
         int code = msg->dataArea.PROGRAM_EXITED.type;
         int rc = msg->dataArea.PROGRAM_EXITED.rc;
         pthrd_printf("Decoded PROGRAM_EXITED, type = %d, rc = %d\n", code, rc);
                      
         if (code == EXIT_TYPE_EXIT) {
            new_event = EventExit::ptr(new EventExit(EventType::Post, rc));
         }
         else if (msg->dataArea.PROGRAM_EXITED.type == EXIT_TYPE_SIGNAL) {
            new_event = EventCrash::ptr(new EventCrash(rc));
         }
         else {
            perr_printf("Unknown exit type %d\n", code);
            break;
         }
         new_event->setSyncType(Event::sync_process);
         remove_for_crash(proc->getPid());
         break;
      }
      case SIGNAL_ENCOUNTERED: {
         int signo = msg->dataArea.SIGNAL_ENCOUNTERED.signal;
         int thrd_id = msg->header.thread;
         pthrd_printf("Decoded SIGNAL_ENCOUNTERED, signal = %d\n", signo);

         if (proc->getState() == int_process::neonatal_intermediate) {
            if (signo != SIGSTOP) {
               //The process is still running after an attach, and we got
               // an unexpected signal before we could stop and prep ourselves.
               // we're in no situation to handle this signal, just continue it
               // and wait for our SIGSTOP.
               pthrd_printf("Recieved unexpected signal %d on %d/%d during attach process, continue and ignore\n",
                            signo, proc->getPid(), thrd_id);
               BG_Debugger_Msg cmsg(CONTINUE, proc->getPid(), msg->header.thread, 0, 0);
               cmsg.header.dataLength = sizeof(msg->dataArea.CONTINUE);
               cmsg.dataArea.CONTINUE.signal = signo;
               BGSend(cmsg);
               break;
            }
            pthrd_printf("Decoded SIGSTOP for process bootstrap\n");
            assert(proc->bootstrap_state == bg_process::bg_stop_pending);
            new_event = EventIntBootstrap::ptr(new EventIntBootstrap());         
            new_event->setSyncType(Event::sync_process);
            break;
         }
         break;
      }
      case VERSION_MSG_ACK:
         pthrd_printf("Dropping VERSION_MSG_ACK\n");
         break;
      case THREAD_ALIVE_ACK:
         if (proc->getState() == int_process::neonatal_intermediate) {
            thrd_alive_ack_t *thrd_alive = (thrd_alive_ack_t *) malloc(sizeof(thrd_alive_ack_t));
            thrd_alive->lwp_id = msg->header.thread;
            thrd_alive->alive = (msg->header.returnCode != RC_NO_ERROR);
            new_event = EventIntBootstrap::ptr(new EventIntBootstrap(thrd_alive)); 
            new_event->setSyncType(Event::async);
            pthrd_printf("Decoded THREAD_ALIVE_ACK for %d/%d: alive %d\n",
                         proc->getPid(), thrd_alive->lwp_id, (int) thrd_alive->alive);
         }
         break;
      case GET_PROCESS_DATA_ACK:
         if (proc->getState() == int_process::neonatal_intermediate) {
            pthrd_printf("Decoded GET_PROCESS_DATA_ACK for %d/%d\n",
                         proc->getPid(), thread->getLWP());
            BG_Debugger_Msg::DataArea *data;
            data = (BG_Debugger_Msg::DataArea *) malloc(sizeof(BG_Debugger_Msg::DataArea));
            *data = msg->dataArea;
            
            new_event = EventIntBootstrap::ptr(new EventIntBootstrap(data));
            new_event->setSyncType(Event::async);
         }
         break;
      case SET_THREAD_OPS_ACK:
      case GET_REGS_AND_FLOATS_ACK:
      case GET_THREAD_ID_ACK:
      case SET_DEBUG_REGS_ACK:
      case GET_DEBUG_REGS_ACK:
      case GET_THREAD_INFO_ACK:
      case GET_AUX_VECTORS_ACK:
      case GET_STACK_TRACE_ACK:
      case END_DEBUG_ACK:
      case GET_THREAD_DATA_ACK:
      case SET_FLOAT_REG_ACK:
      case GET_FLOAT_REG_ACK:
      case GET_ALL_FLOAT_REGS_ACK:
         assert(0); //TODO
         break;
      case GET_REG:
      case GET_ALL_REGS:
      case SET_REG:
      case GET_MEM:
      case SET_MEM:
      case GET_FLOAT_REG:
      case GET_ALL_FLOAT_REGS:
      case SET_FLOAT_REG:
      case SINGLE_STEP:
      case CONTINUE:
      case KILL:
      case ATTACH:
      case DETACH:
      case VERSION_MSG:
      case GET_DEBUG_REGS:
      case SET_DEBUG_REGS:
      case GET_THREAD_INFO:
      case THREAD_ALIVE:
      case GET_THREAD_ID:
      case SET_THREAD_OPS:
      case GET_REGS_AND_FLOATS:
      case GET_AUX_VECTORS:
      case GET_STACK_TRACE:
      case END_DEBUG:
      case GET_PROCESS_DATA:
      case GET_THREAD_DATA:
      case THIS_SPACE_FOR_RENT:
         //We should never get these from the debugger.  They're included here
         // to keep the compiler from throwing missing case statement warnings.
         // I could have used a 'default' entry, but would like to see the warnings
         // when the interface next changes.
         assert(0);
         break;
   }

   if (!new_event) {
      pthrd_printf("No new event created, dropping\n");
      return true;
   }

   new_event->setProcess(proc->proc());
   new_event->setThread(thread->thread());
   events.push_back(new_event);
   return true;
}

int_process *int_process::createProcess(Dyninst::PID p, std::string e)
{
   std::map<int,int> f;
   std::vector<std::string> a;
   bg_process *bgproc = new bg_process(p, e, a, f);
   assert(bgproc);
   return static_cast<int_process *>(bgproc);
}

int_process *int_process::createProcess(std::string e, std::vector<std::string> a, std::map<int,int> f)
{
   bg_process *bgproc = new bg_process(0, e, a, f);
   assert(bgproc);
   return static_cast<int_process *>(bgproc);
}

int_process *int_process::createProcess(Dyninst::PID, int_process *)
{
   assert(0); //No fork on BlueGene
   return NULL;
}

int bg_process::protocol_version = -1;
int bg_process::phys_procs = -1;
int bg_process::virt_procs = -1;

bg_process::bg_process(Dyninst::PID p, std::string e, std::vector<std::string> a, std::map<int, int> f) :
   int_process(p, e, a, f),
   sysv_process(p, e, a, f),
   ppc_process(p, e, a, f)
{
}

bg_process::bg_process(Dyninst::PID pid_, int_process *proc_) :
   int_process(pid_, proc_),
   sysv_process(pid_, proc_),
   ppc_process(pid_, proc_),
   bootstrap_state(bg_init)
{
   assert(0); //No fork
}

bg_process::~bg_process()
{
}

bool bg_process::plat_create()
{
   pthrd_printf("Attempted to do an unsupported create on BlueGene\n");
   setLastError(err_unsupported, "Create not yet supported on BlueGene\n");
   return false;
}

bool bg_process::plat_create_int()
{
   pthrd_printf("Attempted to do an unsupported create on BlueGene\n");
   setLastError(err_unsupported, "Create not yet supported on BlueGene\n");
   return false;
}

bool bg_process::plat_attach()
{
   BG_Debugger_Msg msg(ATTACH, getPid(), 0, 0, 0);
   msg.header.dataLength = sizeof(msg.dataArea.ATTACH);
   
   pthrd_printf("Sending ATTACH message to %d\n", getPid());
   register_for_crash(getPid());
   bool result = BGSend(msg);
   if (!result) {
      pthrd_printf("Error sending ATTACH message\n");
      return false;
   }
   return true;
}
   
bool bg_process::plat_forked()
{
   pthrd_printf("Attempted to handle unsupported fork on BlueGene\n");
   setLastError(err_unsupported, "Fork not yet supported on BlueGene\n");
   return false;
}

bool bg_process::post_forked()
{
   pthrd_printf("Attempted to handle unsupported fork on BlueGene\n");
   setLastError(err_unsupported, "Fork not yet supported on BlueGene\n");
   return false;
}

bool bg_process::plat_detach(bool &needs_sync)
{
   pthrd_printf("Continuing process %d for plat detach", getPid());
   int_threadPool *tp = threadPool();
   bool result = tp->intCont();
   if (!result) {
      pthrd_printf("Error continuing proc %d for detach.\n", getPid());
      return false;
   }
   
   pthrd_printf("Finished continue, now detaching\n");
   BG_Debugger_Msg msg(DETACH, getPid(), 0, 0, 0);
   msg.header.dataLength = sizeof(msg.dataArea.DETACH);
   
   pthrd_printf("Sending DETACH message to %d\n", getPid());
   result = BGSend(msg);
   if (!result) {
      pthrd_printf("Error sending DETACH message\n");
      return false;
   }

   needs_sync = true;
   return true;   
}

bool bg_process::plat_terminate(bool &needs_sync)
{
   BG_Debugger_Msg msg(KILL, getPid(), 0, 0, 0);
   msg.dataArea.KILL.signal = SIGKILL;
   msg.header.dataLength = sizeof(msg.dataArea.KILL);
   
   pthrd_printf("Sending KILL-9 message to %d\n", getPid());
   bool result = BGSend(msg);
   if (!result) {
      pthrd_printf("Error sending KILL message\n");
      return false;
   }
   needs_sync = true;
   return true;   
}

bool bg_process::plat_needsAsyncIO() const
{
   return true;
}

bool bg_process::plat_readMemAsync(int_thread *, Dyninst::Address addr, 
                                  mem_response::ptr resp)
{
   pthrd_printf("Reading from memory %lx +%lx on %d with response ID %d\n", 
                addr, (unsigned long) resp->getSize(), getPid(), resp->getID());
   assert(resp->getSize() < BG_Debugger_Msg_MAX_MEM_SIZE); //MATT TODO

   BG_Debugger_Msg msg(GET_MEM, getPid(), 0, resp->getID(), 0);
   msg.dataArea.GET_MEM.addr = addr;
   msg.dataArea.GET_MEM.len = resp->getSize();
   msg.header.dataLength = sizeof(msg.dataArea.GET_MEM);
   
   bool result = BGSend(msg);
   if (!result) {
      pthrd_printf("Error sending GET_MEM message\n");
      return false;
   }
   return true;   
}

bool bg_process::plat_writeMemAsync(int_thread *, void *local, Dyninst::Address addr,
                                   size_t size, result_response::ptr resp)
{
   pthrd_printf("Writing memory %lx +%lx on %d with response ID %d\n", 
                addr, (unsigned long) size, getPid(), resp->getID());
   assert(size < BG_Debugger_Msg_MAX_MEM_SIZE); //MATT TODO

   BG_Debugger_Msg msg(SET_MEM, getPid(), 0, resp->getID(), 0);
   msg.dataArea.SET_MEM.addr = addr;
   msg.dataArea.SET_MEM.len = size;
   memcpy(msg.dataArea.SET_MEM.data, local, size);
   msg.header.dataLength = sizeof(msg.dataArea.SET_MEM);
   
   bool result = BGSend(msg);
   if (!result) {
      pthrd_printf("Error sending SET_MEM message\n");
      return false;
   }
   return true;
}

bool bg_process::plat_readMem(int_thread *, void *, Dyninst::Address, size_t)
{
   assert(0); //No synchronous IO
   return false;
}

bool bg_process::plat_writeMem(int_thread *, void *, Dyninst::Address, size_t)
{
   assert(0); //No synchronous IO
   return false;
}

bool bg_process::needIndividualThreadAttach()
{
   return true;
}

bool bg_process::getThreadLWPs(std::vector<Dyninst::LWP> &lwps)
{
   if (bootstrap_state != bg_done)
      return false;

   for (std::set<int>::iterator i = initial_lwps.begin(); i != initial_lwps.end(); i++)
   {
      lwps.push_back((Dyninst::LWP) *i);
   }
   initial_lwps.clear();
   return true;
}

Dyninst::Architecture bg_process::getTargetArch()
{
   return Dyninst::Arch_ppc32;
}

unsigned bg_process::getTargetPageSize()
{
   return 0x1000;
}

Dyninst::Address bg_process::plat_mallocExecMemory(Dyninst::Address addr, unsigned size)
{
   return 0;
}

bool bg_process::plat_individualRegAccess()
{
   return true;
}

bool bg_process::plat_createDeallocationSnippet(Dyninst::Address addr, unsigned long size, void* &buffer,
                                                unsigned long &buffer_size, unsigned long &start_offset)
{
   return false;
}

bool bg_process::plat_createAllocationSnippet(Dyninst::Address addr, bool use_addr, unsigned long size, 
                                              void* &buffer, unsigned long &buffer_size, 
                                              unsigned long &start_offset)
{
   return false;
}

bool bg_process::plat_collectAllocationResult(int_thread *thr, reg_response::ptr resp)
{
   return false;
}

void bg_process::getVersionInfo(int &protocol, int &phys, int &virt)
{
   pthrd_printf("Call to getVersionInfo, protocol = %d, phys = %d, virt = %d\n",
                protocol_version, phys_procs, virt_procs);
   if (protocol_version != -1) {
      protocol = protocol_version;
      phys = phys_procs;
      virt = virt_procs;
      return;
   }

   //This is called during generator initialization, so it
   // should always be uninitialized on the generator thread.
   pthrd_printf("Sending VERSION_MSG\n");
   assert(strcmp(thrdName(), "G") == 0);
   BG_Debugger_Msg msg(VERSION_MSG, 0, 0, 0, 0);
   msg.header.dataLength = sizeof(msg.dataArea.VERSION_MSG);
   
   bool result = BGSend(msg);
   if (!result) {
      perr_printf("Error sending VERSION message\n");
      return;
   }

   Generator *gen = Generator::getDefaultGenerator();
   GeneratorBlueGene *gen_bg = static_cast<GeneratorBlueGene *>(gen);

   pthrd_printf("Waiting for VERSION_MSG_ACK\n");
   BG_Debugger_Msg *ack_msg = NULL;
   ArchEvent *ae = NULL;
   
   ae = gen_bg->getEvent(true);
   ack_msg = static_cast<ArchEventBlueGene *>(ae)->getMsg();
   assert(ack_msg->header.messageType == VERSION_MSG_ACK);

   protocol_version = (int) ack_msg->dataArea.VERSION_MSG_ACK.protocolVersion;
   phys_procs = (int) ack_msg->dataArea.VERSION_MSG_ACK.numPhysicalProcessors;
   virt_procs = (int) ack_msg->dataArea.VERSION_MSG_ACK.numLogicalProcessors;
   
   pthrd_printf("Debug interface version = %d, phys_procs = %d, virt_procs = %d",
                protocol_version, phys_procs, virt_procs);
   protocol = protocol_version;
   phys = phys_procs;
   virt = virt_procs;

   delete ae;
}

int_process::ThreadControlMode bg_process::plat_getThreadControlMode() const
{
   return int_process::NoLWPControl;
}

SymbolReaderFactory *bg_process::plat_defaultSymReader()
{
  static SymbolReaderFactory *symreader_factory = NULL;
  if (symreader_factory)
    return symreader_factory;

  symreader_factory = (SymbolReaderFactory *) new SymElfFactory();
  return symreader_factory;
}

unsigned bg_process::plat_getRecommendedReadSize()
{
   return 2048;
}

bool ProcessPool::LWPIDsAreUnique()
{
   return false;
}

int_thread *int_thread::createThreadPlat(int_process *proc, 
                                         Dyninst::THR_ID thr_id, 
                                         Dyninst::LWP lwp_id,
                                         bool initial_thrd)
{
   bg_thread *bgthrd = new bg_thread(proc, thr_id, initial_thrd ? 0 : lwp_id);
   assert(bgthrd);
   return static_cast<int_thread *>(bgthrd);
}

bg_thread::bg_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l) :
   int_thread(p, t, l)
{
}

bg_thread::~bg_thread()
{
}

bool bg_thread::plat_cont()
{
   /*int_threadPool *tp = llproc()->threadPool();
   if (tp->initialThread() != this) {
      pthrd_printf("Not continuing non-initial thread\n");
      return true;
      }*/
   
   Dyninst::LWP lwp_to_cont = getLWP();
   int sig_to_cont = 0;

/*   for (int_threadPool::iterator i = tp->begin(); i != tp->end(); i++) {
      bg_thread *thr = static_cast<bg_thread *>(*i);
      if (thr->continueSig_) {
         lwp_to_cont = thr->getLWP();
         sig_to_cont = thr->continueSig_;
         thr->continueSig_ = 0;
         break;
      }
      }*/
   lwp_to_cont = getLWP();
   sig_to_cont = continueSig_;
   continueSig_ = 0;
   
   pthrd_printf("Sending CONTINUE with signal %d to %d/%d\n", 
                sig_to_cont, llproc()->getPid(), lwp_to_cont);
   BG_Debugger_Msg msg(CONTINUE, llproc()->getPid(), lwp_to_cont, continueSig_, 0);
   msg.dataArea.CONTINUE.signal = continueSig_;
   msg.header.dataLength = sizeof(msg.dataArea.CONTINUE);
   
   bool result = BGSend(msg);
   if (!result) {
      perr_printf("Error sending CONTINUE message\n");
      return false;
   }

   return true;
}

bool bg_thread::plat_stop()
{
/*   if (llproc()->threadPool()->initialThread() != this) {
      pthrd_printf("Not stopping non-initial thread\n");
      return true;
      }*/

   BG_Debugger_Msg msg(KILL, llproc()->getPid(), getLWP(), 0, 0);
   msg.dataArea.KILL.signal = SIGSTOP;
   msg.header.dataLength = sizeof(msg.dataArea.KILL);
   
   pthrd_printf("Sending sigstop KILL msg to %d/%d\n", llproc()->getPid(), getLWP());
   bool result = BGSend(msg);
   if (!result) {
      perr_printf("Error sending STOP message\n");
      return false;
   }
   return true;
}

bool bg_thread::plat_getAllRegisters(int_registerPool &)
{
   assert(0);
   return false;
}

bool bg_thread::plat_getRegister(Dyninst::MachRegister, Dyninst::MachRegisterVal &)
{
   assert(0);
   return false;
}

bool bg_thread::plat_setAllRegisters(int_registerPool &)
{
   assert(0);
   return false;
}

bool bg_thread::plat_setRegister(Dyninst::MachRegister, Dyninst::MachRegisterVal)
{
   assert(0);
   return false;
}

bool bg_thread::attach()
{
   pthrd_printf("Setting states for %d/%d to stopped\n", llproc()->getPid(), getLWP());
   setGeneratorState(stopped);
   setHandlerState(stopped);
   setInternalState(stopped);
   setUserState(stopped);
   return true;
}

HandleBGAttached::HandleBGAttached() : 
   Handler("BGAttach")
{
}

HandleBGAttached::~HandleBGAttached()
{
}

void HandleBGAttached::getEventTypesHandled(vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::Any, EventType::IntBootstrap));
   etypes.push_back(EventType(EventType::Any, EventType::Bootstrap));
}

int HandleBGAttached::getPriority() const
{
   return Handler::PrePlatformPriority;
}

/**
 * This handler takes a lot of different types of events: GET_PROCESS_DATA_ACK,
 * THREAD_ALIVE_ACK, SIGNAL_ENCOUNTERED (as IntBootstrap) and Bootstrap.  For
 * each event it walks the process through its initialization.  This could
 * have been many different handlers,  but I thought things would be clearer
 * in one bigger initialization object.
 * 
 * Logic during bootstrap is as follows:
 *  1. ATTACH_ACK triggers a KILL with SIGSTOP
 *  2. SIGNAL_ENCOUNTERED triggers either:
 *    protocol < 2: ready state
 *    protocol == 2 || protocol == 3: THREAD_ALIVE to each thread
 *    protocol >= 4: GET_PROCESS_DATA
 *  3. All ACKs from stage 2 triggers ready state
 *  4. Ready state triggers Bootstrap event
 *  5. Bootstrap event removes generator block
 *
 * Having the generator block on keeps us recieving events while the process
 * is stopped after stage 2.
 **/
Handler::handler_ret_t HandleBGAttached::handleEvent(Event::ptr ev)
{
   int_process *p = ev->getProcess()->llproc();
   bg_process *proc = dynamic_cast<bg_process *>(p);

   int protocol, phys, virt;
   bg_process::getVersionInfo(protocol, phys, virt);

   if (proc->bootstrap_state == bg_process::bg_init) {
      pthrd_printf("Process %d ATTACHED, forcing gen block\n", 
                   proc->getPid());
      ProcPool()->condvar()->lock();
      //Keep the generator blocking on this process through-out the
      //init process.
      proc->setForceGeneratorBlock(true);
      ProcPool()->condvar()->signal();
      ProcPool()->condvar()->unlock();

      pthrd_printf("Sending SIGSTOP to %d as part of init\n", proc->getPid());
      bg_thread *thrd = static_cast<bg_thread *>(proc->threadPool()->initialThread());
      thrd->plat_stop();
      
      proc->bootstrap_state = bg_process::bg_stop_pending;
   }
   else if (proc->bootstrap_state == bg_process::bg_stop_pending) {
      pthrd_printf("Recieved SIGSTOP ack during process initialization\n");
      proc->bootstrap_state = bg_process::bg_stopped;
   }
   if (proc->bootstrap_state == bg_process::bg_stopped) {
      pthrd_printf("Handling thread query during attach\n");
      
      if (protocol < 2) {
         //BGL, no threads so skip messages.
         pthrd_printf("Moving process to ready state on BG/L\n");
         proc->bootstrap_state = bg_process::bg_ready;
      }
      else if (protocol == 2 || protocol == 3) {
         for (unsigned i=1; i<=8; i++) {
            //Iterate over the max number of threads, and check which of them
            // are alive.  
            BG_Debugger_Msg msg(THREAD_ALIVE, proc->getPid(), i, 0, 0);
            msg.dataArea.THREAD_ALIVE.tid = i;
            msg.header.dataLength = sizeof(msg.dataArea.THREAD_ALIVE);
            
            pthrd_printf("Sending THREAD_ALIVE msg to %d/%d\n", proc->getPid(), i);
            bool result = BGSend(msg);
            if (!result) {
               perr_printf("Error sending THREAD_ALIVE message\n");
               return Handler::ret_error;
            }
            proc->pending_thread_alives++;
         }
         proc->bootstrap_state = bg_process::bg_thread_pending;
      }
      else if (protocol >= 4) {
         //Protocol 4 made things a little easier with a simple command
         // to get all threads.
         BG_Debugger_Msg msg(GET_PROCESS_DATA, proc->getPid(), 0, 0, 0);
         msg.header.dataLength = sizeof(msg.dataArea.GET_PROCESS_DATA);
         
         pthrd_printf("Sending GET_PROCESS_DATA msg to %d/%d\n", proc->getPid(), 0);
         bool result = BGSend(msg);
         if (!result) {
            perr_printf("Error sending GET_PROCESS_DATA message\n");
            return Handler::ret_error;
         }
         proc->bootstrap_state = bg_process::bg_thread_pending;
      }
   }
   else if (proc->bootstrap_state == bg_process::bg_thread_pending) {
      EventIntBootstrap::ptr evib = ev->getEventIntBootstrap();
      assert(protocol >= 2);
      if (protocol == 2 || protocol == 3) {
         //Collect all THREAD_ALIVE_ACKS to see how many threads exist.  No
         // mmoving on until we've found all the threads.
         thrd_alive_ack_t *thread_status = (thrd_alive_ack_t *) evib->getData();
         pthrd_printf("Recieved THREAD_ALIVE_ACK.  %d/%d is %s\n", proc->getPid(),
                      thread_status->lwp_id, thread_status->alive ? "alive" : "dead");
         proc->pending_thread_alives--;
         assert(proc->pending_thread_alives >= 0);
         if (thread_status->alive) {
            proc->initial_lwps.insert(thread_status->lwp_id);
         }
         if (!proc->pending_thread_alives) {
            pthrd_printf("Recieved last THREAD_ALIVE_ACK, moving bootstrap state\n");
            proc->bootstrap_state = bg_process::bg_ready;
         }
         else {
            pthrd_printf("Still have %d THREAD_ALIVE_ACK messages\n", 
                         proc->pending_thread_alives);
         }
      }
      else if (protocol >= 4) {
         //We got the PROCESS_DATA_ACK, which contains all the threads.
         pthrd_printf("Got PROCESS_DATA_ACK on %d\n", proc->getPid());
         BG_Debugger_Msg::DataArea *proc_data_ack = (BG_Debugger_Msg::DataArea *) evib->getData();
         proc->has_procdata = true;
         proc->procdata = proc_data_ack->GET_PROCESS_DATA_ACK.processData;
         for (unsigned i=0; i<proc_data_ack->GET_PROCESS_DATA_ACK.numThreads; i++) {
            int lwp_id = proc_data_ack->GET_PROCESS_DATA_ACK.threadIDS[i];
            pthrd_printf("Adding %d/%d to pending LWPs\n", proc->getPid(), lwp_id); 
            proc->initial_lwps.insert(lwp_id);
         }
         proc->bootstrap_state = bg_process::bg_ready;
      }
      if (evib->getData()) {
         free(evib->getData());
         evib->setData(NULL);
      }
   }
   if (proc->bootstrap_state == bg_process::bg_ready) {
      pthrd_printf("Process %d is ready, faking bootstrap event\n", proc->getPid());

      //Create and enqueue the faked bootstrap event
      int_thread *thrd = proc->threadPool()->initialThread();
      EventBootstrap::ptr new_ev = EventBootstrap::ptr(new EventBootstrap());
      new_ev->setProcess(proc->proc());
      new_ev->setThread(thrd->thread());
      new_ev->setSyncType(Event::async);
      
      mbox()->enqueue(new_ev, true);
      proc->bootstrap_state = bg_process::bg_bootstrapped;
   }
   else if (proc->bootstrap_state == bg_process::bg_bootstrapped)
   {
      pthrd_printf("Handling BG level bootstrap event on %d--removing generator block\n",
                   proc->getPid());

      ProcPool()->condvar()->lock();
      //No longer need to keep the generator blocked on this process
      proc->setForceGeneratorBlock(false);
      ProcPool()->condvar()->signal();
      ProcPool()->condvar()->unlock();

      proc->bootstrap_state = bg_process::bg_done;
   }

   return Handler::ret_success;
}

HandlerPool *plat_createDefaultHandlerPool(HandlerPool *hpool)
{
   static bool initialized = false;
   static HandleBGAttached *bg_attach = NULL;
   if (!initialized) {
      bg_attach = new HandleBGAttached();
      initialized = true;
   }
   hpool->addHandler(bg_attach);
   return hpool;
}

static bool BGSend(BG_Debugger_Msg &msg)
{
   static Mutex send_lock;
   pthrd_printf("Sending message to CIOD\n");
   if (dyninst_debug_proccontrol) {
      BG_Debugger_Msg::dump(msg, pctrl_err_out);
   }
   send_lock.lock();
   bool result = BG_Debugger_Msg::writeOnFd(BG_DEBUGGER_WRITE_PIPE, msg);
   send_lock.unlock();
   if (!result) {
      perr_printf("Failure sending message on BG pipe\n");
   }
   return result;
}

static bool bg_fdHasData(int fd) {
   int result;
   struct pollfd fds;
   fds.fd = fd;
   fds.events = POLLIN;
   fds.revents = 0;
   result = poll(&fds, 1, 0);
   if (result == -1) {
      int errnum = errno;
      pthrd_printf("Unable to poll fd %d: %s\n", fd, strerror(errnum));
      return false;
   }
   return (bool) (fds.revents & POLLIN);
}

static BG_GPR_Num_t DynToBGGPRReg(Dyninst::MachRegister reg)
{
#define CASE_GPR(NUM) case Dyninst::ppc32::ir ## NUM : return (BG_GPR_Num_t) NUM;
#define CASE_SPEC(BLUEG, DYN) case Dyninst::ppc32::i ## DYN: return BG_ ## BLUEG;
   switch (reg.val())
   {
      CASE_GPR(0);      CASE_GPR(1);      CASE_GPR(2);      CASE_GPR(3);
      CASE_GPR(4);      CASE_GPR(5);      CASE_GPR(6);      CASE_GPR(7);
      CASE_GPR(8);      CASE_GPR(9);      CASE_GPR(10);     CASE_GPR(11);
      CASE_GPR(12);     CASE_GPR(13);     CASE_GPR(14);     CASE_GPR(15);
      CASE_GPR(16);     CASE_GPR(17);     CASE_GPR(18);     CASE_GPR(19);
      CASE_GPR(20);     CASE_GPR(21);     CASE_GPR(22);     CASE_GPR(23);
      CASE_GPR(24);     CASE_GPR(25);     CASE_GPR(26);     CASE_GPR(27);
      CASE_GPR(28);     CASE_GPR(29);     CASE_GPR(30);     CASE_GPR(31);
      CASE_SPEC(FPSCR, fpscw);            CASE_SPEC(LR, lr);
      CASE_SPEC(CR, cr);                  CASE_SPEC(XER, xer);
      CASE_SPEC(CTR, ctr);                CASE_SPEC(IAR, pc);
      CASE_SPEC(MSR, msr);                
      default:
         return (BG_GPR_Num_t) -1;
   }
}

static Dyninst::MachRegister BGGPRToDynReg(BG_GPR_Num_t bg_regnum)
{
#undef CASE_GPR
#undef CASE_SPEC
#define DOUBLE_SEMI ::
#define CASE_GPR(NUM) case BG_GPR ## NUM: return Dyninst::ppc32::r ## NUM
#define CASE_SPEC(BLUEG, DYN) case BG_ ## BLUEG : return Dyninst::ppc32:: DYN
   switch (bg_regnum)
   {
      CASE_GPR(0);      CASE_GPR(1);      CASE_GPR(2);      CASE_GPR(3);
      CASE_GPR(4);      CASE_GPR(5);      CASE_GPR(6);      CASE_GPR(7);
      CASE_GPR(8);      CASE_GPR(9);      CASE_GPR(10);     CASE_GPR(11);
      CASE_GPR(12);     CASE_GPR(13);     CASE_GPR(14);     CASE_GPR(15);
      CASE_GPR(16);     CASE_GPR(17);     CASE_GPR(18);     CASE_GPR(19);
      CASE_GPR(20);     CASE_GPR(21);     CASE_GPR(22);     CASE_GPR(23);
      CASE_GPR(24);     CASE_GPR(25);     CASE_GPR(26);     CASE_GPR(27);
      CASE_GPR(28);     CASE_GPR(29);     CASE_GPR(30);     CASE_GPR(31);
      CASE_SPEC(FPSCR, fpscw);            CASE_SPEC(LR, lr);
      CASE_SPEC(CR, cr);                  CASE_SPEC(XER, xer);
      CASE_SPEC(CTR, ctr);                CASE_SPEC(IAR, pc);
      CASE_SPEC(MSR, msr);
      default:
      return Dyninst::InvalidReg;
   }
}

static std::set<int> live_pids;

static void register_for_crash(int pid)
{
   static bool init = false;
   pthrd_printf("registering %d for crash handling\n", pid);
   if (!init) {
      //signal(SIGSEGV, on_crash);
      //signal(SIGBUS, on_crash);
      //signal(SIGABRT, on_crash);
      init = true;
   }
   
   live_pids.insert(pid);
}

static void remove_for_crash(int pid)
{
   std::set<int>::iterator i = live_pids.find(pid);
   if (i != live_pids.end())
      live_pids.erase(i);
}

static void on_crash(int sig)
{
   pthrd_printf("Crashed with %d... Trying to detach and clean\n", sig);
   for (std::set<int>::iterator i = live_pids.begin(); i != live_pids.end(); i++) 
   {
      BG_Debugger_Msg cont_msg(CONTINUE, *i, 0, 0, 0);
      cont_msg.dataArea.CONTINUE.signal = 0;
      cont_msg.header.dataLength = sizeof(cont_msg.dataArea.CONTINUE);
      BG_Debugger_Msg::writeOnFd(BG_DEBUGGER_WRITE_PIPE, cont_msg);
   }
   for (std::set<int>::iterator i = live_pids.begin(); i != live_pids.end(); i++) 
   {
      BG_Debugger_Msg detach_msg(DETACH, *i, 0, 0, 0);
      detach_msg.header.dataLength = sizeof(detach_msg.dataArea.DETACH);
      BG_Debugger_Msg::writeOnFd(BG_DEBUGGER_WRITE_PIPE, detach_msg);
   }
   if (bg_process::protocol_version >= 3) {
      BG_Debugger_Msg end_msg(END_DEBUG, 0, 0, 0, 0);
      end_msg.header.dataLength = sizeof(end_msg.dataArea.END_DEBUG);
      BG_Debugger_Msg::writeOnFd(BG_DEBUGGER_WRITE_PIPE, end_msg);
   }

   //signal(SIGSEGV, SIG_DFL);
   //signal(SIGBUS, SIG_DFL);
   //signal(SIGABRT, SIG_DFL);

   *((int *) 0x0) = 0x0; //Crash here
}

DebugPortReader *DebugPortReader::me = NULL;
DebugPortReader::DebugPortReader() :
   shutdown(false),
   initialized(false),
   fd(-1)
{
   init_lock.lock();
   pfd[0] = -1;
   pfd[1] = -1;
   assert(!me);
   me = this;
   debug_port_thread.spawn(mainLoopWrapper, NULL);
   while (!initialized && !shutdown) {
      init_lock.wait();
   }
}

DebugPortReader::~DebugPortReader()
{
   if (initialized && !shutdown) {
      char c = 'x';
      shutdown = true;
      write(pfd[1], &c, 1);
      debug_port_thread.join();
   }
   if (fd != -1)
      close(fd);
   if (pfd[0] != -1) {
      close(pfd[0]);
   }   
   if (pfd[1] != -1) {
      close(pfd[1]);
   }
}

void DebugPortReader::mainLoopWrapper(void *)
{
   me->mainLoop();
}

extern void setXThread(long t);

bool DebugPortReader::init()
{
   bool ret = false;
   int result;
   struct sockaddr_in addr;
   struct in_addr iaddr;

   init_lock.lock();

   setXThread(DThread::self());

   result = pipe(pfd);
   if (result == -1) {
      pthrd_printf("Error creating shutdown pipe: %s\n", strerror(errno));
      goto done;
   }

   fd = socket(PF_INET, SOCK_STREAM, 0);
   if (fd == -1) {
      pthrd_printf("Unable to create client for debug port: %s\n", strerror(errno));
      goto done;
   }

   inet_aton("127.0.0.1", &iaddr);
   bzero(&addr, sizeof(addr));
   addr.sin_family = AF_INET;
   addr.sin_port = htons(port_num);
   addr.sin_addr = iaddr;

   pthrd_printf("Connecting to CIOD port\n");
   result = connect(fd, (struct sockaddr *) &addr, sizeof(addr));
   if (result == -1) {
      pthrd_printf("Error connection to CIOD port: %s\n", strerror(errno));
      goto done;
   }

   ret = true;
  done:
   initialized = true;

   init_lock.broadcast();
   init_lock.unlock();
   return ret;
}

void DebugPortReader::mainLoop()
{
   char buffer[4097];
   bool bresult = init();
   if (!bresult) {
      pthrd_printf("Could not read from CIOD port\n");
      fd = -1;
   }

   while (!shutdown) {
      fd_set readfds;
      FD_ZERO(&readfds);
      int nfds = 0;
      if (fd != -1) {
         FD_SET(fd, &readfds);
         nfds = fd;
      }
      if (pfd[0] != -1) {
         FD_SET(pfd[0], &readfds);
         if (pfd[0] > nfds)
            nfds = pfd[0];
      }
      nfds++;

      struct timeval timeout;
      timeout.tv_sec = 10;
      timeout.tv_usec = 0;

      int result = select(nfds, &readfds, NULL, NULL, &timeout);
      if (result == -1) {
         int error = errno;
         if (error == EINTR)
            continue;
         perr_printf("Error calling select: %s\n", strerror(error));
         shutdown = true;
         break;
      }

      struct stat buf;
      int sresult = stat("/g/g22/legendre/pc_force_shutdown", &buf);
      if (sresult == 0) {
         pthrd_printf("Forcing shutdown\n");
         on_crash(0);
      }
      if (fd == -1 || !FD_ISSET(fd, &readfds)) {
         continue;
      }
      ssize_t r = recv(fd, buffer, 4096, MSG_DONTWAIT);
      if (r == -1) {
         int error = errno;
         if (error == EINTR)
            continue;
         perr_printf("Error calling recv on CIOD fd: %s\n", strerror(error));
         break;
      }
      if (r == 0) {
         perr_printf("CIOD shutdown the debug port\n");
         break;
      }
      buffer[r] = '\0';
      unsigned start = 0;
      for (unsigned i=0; i< (unsigned) r; i++) {
         if (buffer[i] == '\n') {
            buffer[i] = '\0';
            pclean_printf("[CIOD] - %s\n", buffer+start);
            start = i+1;
         }
         else if (buffer[i] == '\0') {
            pclean_printf("[CIOD] - %s...\n", buffer+start);
            start = i+1;
         }
      }
   }
   shutdown = true;
}
