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

#if !defined _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>

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
#include <sys/select.h>

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

static void register_for_crash(int pid);
static void remove_for_crash(int pid);
static void on_crash(int sig);

#define EXIT_TYPE_EXIT 0
#define EXIT_TYPE_SIGNAL 1

#define NUM_GPRS 41

static void dumpMessage(BG_Debugger_Msg *msg)
{
   char buffer[4096];
   FILE *f = fmemopen(buffer, 4096, "w+");
   if (!f) {
      BG_Debugger_Msg::dump(*msg, pctrl_err_out);
      return;
   }
   BG_Debugger_Msg::dump(*msg, f);
   fclose(f);
   char *c = buffer;
   buffer[4095] = '\0';
   while (*c == '\n') c++;
   pthrd_printf("%s", c);
}

ArchEventBlueGene::ArchEventBlueGene(BG_Debugger_Msg *m) :
   msg(m),
   pc_resp(response::ptr())
{
   pthrd_printf("In constructor for ArchEvent %p\n", this);
}

ArchEventBlueGene::~ArchEventBlueGene()
{
   pthrd_printf("In destructor for ArchEvent %p\n", this);
   if (msg) {
      delete msg;
      msg = NULL;
   }
}

BG_Debugger_Msg *ArchEventBlueGene::getMsg() const
{
   return msg;
}

response::ptr ArchEventBlueGene::getPCResp()
{
   return pc_resp;
}

void ArchEventBlueGene::setPCResp(response::ptr r)
{
   pc_resp = r;
   r->setDecoderEvent(this);
}

GeneratorBlueGene::GeneratorBlueGene() :
   GeneratorMT(std::string("BlueGene Generator")),
   dpr(NULL)
{
   if (dyninst_debug_proccontrol) {
      pthrd_printf("Creating debug port reader\n");
      dpr = new DebugPortReader();
   }
   int result = pipe(restart_fds);
   if (result == -1) {
      perr_printf("Error creating restart FDs\n");
      assert(0);
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

void GeneratorBlueGene::addHeldArchEvent(ArchEventBlueGene *ae)
{
   held_aes_lock.lock();
   held_aes.push(ae);
   held_aes_lock.unlock();

   if (restart_fds[1] == -1) 
      return;

   char c = CHECK_HELD_AEVS;
   int result;
   do {
      result = write(restart_fds[1], &c, sizeof(char));
   } while (result == -1 && errno == EINTR);
   if (result == -1) {
      int error = errno;
      perr_printf("Unable to write to restart_fd: %s\n", strerror(error));
      restart_fds[0] = -1;
      restart_fds[1] = -1;
   }
}

ArchEventBlueGene *GeneratorBlueGene::getHeldArchEvent()
{
   ArchEventBlueGene *ret = NULL;
   held_aes_lock.lock();
   if (held_aes.empty()) {
      ret = NULL;
   }
   else {
      ret = held_aes.front();
      held_aes.pop();
      held_aes_lock.unlock();
   }
   held_aes_lock.unlock();
   return ret;
}

static bool messageAllowedOnStoppedProc(const BG_Debugger_Msg &msg)
{
   switch (msg.header.messageType)
   {
      case GET_REG_ACK:
      case GET_ALL_REGS_ACK:
      case GET_MEM_ACK:
      case SET_REG_ACK:
      case SET_MEM_ACK:
      case SINGLE_STEP_ACK:
      case CONTINUE_ACK:
      case KILL_ACK:
      case ATTACH_ACK:
      case DETACH_ACK:
      case VERSION_MSG_ACK:
      case THREAD_ALIVE_ACK:
      case GET_PROCESS_DATA_ACK:
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
      case HOLD_THREAD_ACK:
      case RELEASE_THREAD_ACK:
      case SIGACTION_ACK:
      case MAP_MEM_ACK:
      case FAST_TRAP_ACK:
      case DEBUG_IGNORE_SIG_ACK:
         return true;
      default:
         return false;
   }
}

ArchEvent *GeneratorBlueGene::getEvent(bool block)
{
   BG_Debugger_Msg *msg = NULL;
   fd_set readfds;
   struct timeval do_poll, *timeout;
   do_poll.tv_sec = 0;
   do_poll.tv_usec = 0;
   timeout = block ? NULL : &do_poll;
   
   for (;;) {
      FD_ZERO(&readfds);
      FD_SET(BG_DEBUGGER_READ_PIPE, &readfds);
      if (restart_fds[0] != -1)
         FD_SET(restart_fds[0], &readfds);
      int max_fd = (BG_DEBUGGER_READ_PIPE > restart_fds[0]) ? BG_DEBUGGER_READ_PIPE : restart_fds[0];
      
      int result = select(max_fd+1, &readfds, NULL, NULL, timeout);
      if (result == -1) {
         int error = errno;
         if (error == EINTR)
            continue;
         perr_printf("Error calling select on debugger pipe %d and restart_fd %d\n", 
                     BG_DEBUGGER_READ_PIPE, restart_fds[0]);
         assert(0);
      }

      if (result == 0) {
         //Timeout
         return NULL;
      }
      break;
   }
    
  if (restart_fds[0] != -1 && FD_ISSET(restart_fds[0], &readfds)) {
     char c;
     int read_result;
     do {
        read_result = read(restart_fds[0], &c, sizeof(char));
     } while (read_result == -1 && errno == EINTR);
     if (read_result == -1) {
        int error = errno;
        perr_printf("Error reading from restart fd %d: %s\n", restart_fds[0], strerror(error));
        restart_fds[0] = -1;
        restart_fds[1] = -1;
     }
     
     if (c == CHECK_HELD_AEVS) {
        ArchEvent *aevent = static_cast<ArchEvent *>(getHeldArchEvent());
        return aevent;
     }
     perr_printf("Unexpected restart_fd message: %c\n", c);
     assert(0);
  }

  if (FD_ISSET(BG_DEBUGGER_READ_PIPE, &readfds)) {
     // Read an event from the debug filehandle.
     msg = new BG_Debugger_Msg();
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
     pthrd_printf("Received debug event %s from pid %d, tid %ld, rc %d\n",
                  BG_Debugger_Msg::getMessageName(msg->header.messageType), pid, 
                  (signed long) tid, returnCode);
     if (dyninst_debug_proccontrol) {
        dumpMessage(msg);
     }
     
     if (returnCode > 0) {
        // Print out a message if we get a return code we don't expect.  Consult the debugger header
        // for the meanings of these.
        pthrd_printf("Warning. Return code for %s on pid %d, tid %ld was non-zero: %d\n",
                     BG_Debugger_Msg::getMessageName(msg->header.messageType), pid,
                     (signed long) tid, msg->header.returnCode);
     }
     
     ArchEvent *aevent = new ArchEventBlueGene(msg);
     assert(aevent);
     return aevent;
  }

  assert(0);
  return NULL;
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

Event::ptr DecoderBlueGene::decodeGetRegAck(BG_Debugger_Msg *msg, response::ptr &resp)
{
   pthrd_printf("Decode get reg ack with value 0x%lx\n", (unsigned long) msg->dataArea.GET_REG_ACK.value);
   getResponses().lock();
   
   resp = getResponses().rmResponse(msg->header.sequence);
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

Event::ptr DecoderBlueGene::decodeGetAllRegAck(BG_Debugger_Msg *msg, response::ptr &resp)
{
   pthrd_printf("Decoding get all reg ack\n");
   getResponses().lock();
   
   resp = getResponses().rmResponse(msg->header.sequence);
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

Event::ptr DecoderBlueGene::decodeGetMemAck(BG_Debugger_Msg *msg, response::ptr &resp)
{
   pthrd_printf("Decoding get mem ack\n");
   getResponses().lock();
   
   resp = getResponses().rmResponse(msg->header.sequence);
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

Event::ptr DecoderBlueGene::decodeResultAck(BG_Debugger_Msg *msg, response::ptr &resp)
{
   pthrd_printf("Decoding result ack\n");
   getResponses().lock();
   
   resp = getResponses().rmResponse(msg->header.sequence);
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

   pthrd_printf("Creating new EventAsync over %s for response %s/%d\n",
                ev->name().c_str(), resp->name().c_str(), resp->getID());
   int_eventAsync *internal = new int_eventAsync(resp);
   EventAsync::ptr async_ev(new EventAsync(internal));
   async_ev->setProcess(ev->getProcess());
   async_ev->setThread(ev->getThread());
   async_ev->setSyncType(Event::async);
   async_ev->addSubservientEvent(ev);

   return async_ev;
}

/**
 * The first time this function is called it will post a reg response for the current
 * PC and return false.  Once the PC request is ACK'd, then subsequent calls to this
 * function will start retruning true with the addr field filled in.
 *
 * The state needed for this operation is tracked in the ArchEventBlueGene object
 * that initiallially triggered the getPC call in the decoder.
 **/
bool DecoderBlueGene::getPC(Address &addr, int_thread *thread, ArchEventBlueGene *cur_event)
{
   response::ptr resp = cur_event->getPCResp();
   if (resp) {
      reg_response::ptr regr = resp->getRegResponse();
      assert(regr);
      if (!regr->isReady())
         return false;
      if (regr->hasError()) {
         perr_printf("Error while reading PC register for decoder.\n");
         addr = 0x0;
         return true;
      }
      addr = (Dyninst::Address) regr->getResult();
      pthrd_printf("Decoder PC register read requested for %d/%d, returning %lx\n",
                   thread->llproc()->getPid(), thread->getLWP(), addr);
      return true;
   }

   
   reg_response::ptr regr = reg_response::createRegResponse();
   MachRegister pc_reg = MachRegister::getPC(thread->llproc()->getTargetArch());
   bool result = thread->getRegister(pc_reg, regr);
   cur_event->setPCResp(regr);
   if (!result || regr->hasError()) {
      perr_printf("Error while reading PC register for decoder.\n");
      addr = 0x0;
      return true;
   }
   if (regr->isReady()) {
      addr = (Dyninst::Address) regr->getResult();
      pthrd_printf("Decoder PC register read requested for %d/%d, returning %lx\n",
                   thread->llproc()->getPid(), thread->getLWP(), addr);
      return true;
   }
   
   //This is the typical case on BG--not an error.
   pthrd_printf("Decoder PC register read requested for %d/%d, postponing decode\n",
                thread->llproc()->getPid(), thread->getLWP());
   return false;
}

Event::ptr DecoderBlueGene::decodeDecoderAsync(response::ptr resp)
{
   ArchEvent *ae = resp->getDecoderEvent();
   if (!ae)
      return Event::ptr();

   pthrd_printf("Async event was generated by decoder, recursively calling decoder\n");
   std::vector<Event::ptr> events;

   bool result = decode(ae, events);
   if (!result) {
      perr_printf("Unable to decode original event\n");
   }
   assert(events.size() <= 1);
   if (events.size() == 1)
      return events[0];
   return Event::ptr();
}

bool DecoderBlueGene::decode(ArchEvent *archE, std::vector<Event::ptr> &events)
{
   pthrd_printf("Decoding event\n");
   assert(archE);
   ArchEventBlueGene *archbg = static_cast<ArchEventBlueGene *>(archE);

   BG_Debugger_Msg *msg = archbg->getMsg();
   
   bg_process *proc = NULL;
   bg_thread *thread = NULL;
   bool result = getProcAndThread(archbg, proc, thread);
   if (!result)
      return false;
   if (!thread) 
      thread = static_cast<bg_thread *>(proc->threadPool()->initialThread());
   
   if (thread && thread->getGeneratorState() == int_thread::stopped &&
       !messageAllowedOnStoppedProc(*archbg->getMsg()))
   {
      //We've recieved a non-ACK event on a stopped thread.  Unfortunately,
      //the BG debug interface allows a stopped thread to throw events.  This
      //seems to happen when two events happening simultaniously, or come from
      //different sources (I've seen a single-step and a signal).
      //
      //The higher levels of ProcControlAPI don't like this.  Dead men should tell
      //no tales and stopped processes should throw no events.  We'll hide this 
      //behavior by recognizing when this happens, stashing the events away, 
      //and then re-throwing them the next time the process is continued.
      //
      //These events will be known as held arch events.
      pthrd_printf("non-ACK Event on stopped thread %d/%d.  Holding until continue\n",
                   thread->getLWP(), proc->getPid());
      proc->addHeldArchEvent(archbg);
      return true;
   }

   Event::ptr new_event;
   response::ptr resp;

   switch (msg->header.messageType) {
      case GET_REG_ACK:
         new_event = decodeGetRegAck(msg, resp);
         if (!new_event)
            new_event = decodeDecoderAsync(resp);
         if (!new_event)
            return true;
         break;
      case GET_ALL_REGS_ACK:
         new_event = decodeGetAllRegAck(msg, resp);
         if (!new_event)
            new_event = decodeDecoderAsync(resp);
         if (!new_event)
            return true;
         break;
      case GET_MEM_ACK:
         new_event = decodeGetMemAck(msg, resp);
         if (!new_event)
            new_event = decodeDecoderAsync(resp);
         if (!new_event)
            return true;
         break;
      case SET_REG_ACK:
      case SET_MEM_ACK:
         new_event = decodeResultAck(msg, resp);
         if (!new_event)
            new_event = decodeDecoderAsync(resp);
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
         bg_thread *initial_thread = static_cast<bg_thread *>(proc->threadPool()->initialThread());
         pthrd_printf("Decoding SIGNAL_ENCOUNTERED, signal = %d\n", signo);

         if (signo == SIGTRAP) {
            Dyninst::Address pc_addr;
            pthrd_printf("Decoding SIGTRAP\n");
            bool result = getPC(pc_addr, thread, archbg);
            if (!result) {
               //We are explicitely returning here rather than breaking.  We don't yet
               // want the archE to be deleted.
               pthrd_printf("PC register not yet available, postponing decode\n");
               return Event::ptr();
            }
            pthrd_printf("Decoding SIGTRAP at address %lx\n", pc_addr);
            installed_breakpoint *ibp = proc->getBreakpoint(pc_addr);
            if (ibp) {
               pthrd_printf("Decoded breakpoint on %d/%d at %lx\n", proc->getPid(), 
                            thread->getLWP(), pc_addr);
               EventBreakpoint::ptr event_bp = EventBreakpoint::ptr(new EventBreakpoint(pc_addr, ibp));
               new_event = event_bp;
               new_event->setThread(thread->thread());

               if (pc_addr == proc->getLibBreakpointAddr()) {
                  pthrd_printf("Breakpoint is library load/unload\n");
                  EventLibrary::ptr lib_event = EventLibrary::ptr(new EventLibrary());
                  lib_event->setThread(thread->thread());
                  lib_event->setProcess(proc->proc());
                  lib_event->setSyncType(Event::sync_process);
                  new_event->addSubservientEvent(lib_event);
               }
            }
            if (!new_event) {
               pthrd_printf("WARNING: Got a SIGTRAP at a non-breakpoint address %lx.  Very unusal " 
                            "and probably a bug\n", pc_addr);
               pthrd_printf("Decoding SIGTRAP to new EventSignal\n");
               new_event = EventSignal::ptr(new EventSignal(signo));
            }
         }
         else if (signo == SINGLE_STEP_SIG) {
            if (!thread->singleStep()) {
               perr_printf("Error.  Got single step sig %d on %d/%d, which isn't single stepped\n",
                           signo, proc->getPid(), thread->getLWP());
               break;
            }
            pthrd_printf("Decoded single step signal\n");
            installed_breakpoint *ibp = thread->isClearingBreakpoint();
            if (ibp) {
               pthrd_printf("Decoded event to breakpoint cleanup\n");
               new_event = Event::ptr(new EventBreakpointClear(ibp));
            }
            else {
               new_event = Event::ptr(new EventSingleStep());
            }
         }
         else if (proc->getState() == int_process::neonatal_intermediate) {
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
            initial_thread->setDecoderPendingStop(false);
         }
         else if (signo == SIGSTOP && initial_thread->decoderPendingStop())
         {
            pthrd_printf("Recieved pending SIGSTOP on %d/%d\n", proc->getPid(), thread->getLWP());
            new_event = EventStop::ptr(new EventStop());
            initial_thread->setDecoderPendingStop(false);
         }
         else if (signo == SIGSTOP) {
            pthrd_printf("Found lost stop, turning into NOP event\n");
            new_event = EventNop::ptr(new EventNop());
         }
         else {
            pthrd_printf("Decoded event to signal %d on %d/%d\n",
                         signo, proc->getPid(), thread->getLWP());
            new_event = EventSignal::ptr(new EventSignal(signo));
         }

         if (new_event) {
            new_event->setSyncType(Event::sync_process);

            if (initial_thread->decoderPendingStop()) {
               pthrd_printf("Recieved other signal while waiting for stop.  Creating subservient "
                            "EventStop on %d/%d\n", proc->getPid(), initial_thread->getLWP());
               Event::ptr stop_event = EventStop::ptr(new EventStop());
               stop_event->setThread(thread->thread());
               stop_event->setProcess(proc->proc());
               stop_event->setSyncType(Event::sync_process);
               new_event->addSubservientEvent(stop_event);
               initial_thread->setDecoderPendingStop(false);
            }
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
      case HOLD_THREAD_ACK:
      case RELEASE_THREAD_ACK:
      case SIGACTION_ACK:
      case MAP_MEM_ACK:
      case FAST_TRAP_ACK:
      case DEBUG_IGNORE_SIG_ACK:
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
      case HOLD_THREAD:
      case RELEASE_THREAD:
      case SIGACTION:
      case MAP_MEM:
      case FAST_TRAP:
      case DEBUG_IGNORE_SIG:
      case THIS_SPACE_FOR_RENT:
         //We should never get these from the debugger.  They're included here
         // to keep the compiler from throwing missing case statement warnings.
         // I could have used a 'default' entry, but would like to see the warnings
         // when the interface next changes.
         assert(0);
         break;
   }
   
   if (archE) {
      delete archE;
   }
   archE = NULL;

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
   std::vector<std::string> envp;
   bg_process *bgproc = new bg_process(p, e, a, envp, f);
   assert(bgproc);
   return static_cast<int_process *>(bgproc);
}

int_process *int_process::createProcess(std::string e, std::vector<std::string> a, 
                                        vector<string> envp, std::map<int,int> f)
{
   bg_process *bgproc = new bg_process(0, e, a, envp, f);
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

bg_process::bg_process(Dyninst::PID p, std::string e, std::vector<std::string> a, 
                       vector<string> envp, std::map<int, int> f) :
   int_process(p, e, a, envp, f),
   sysv_process(p, e, a, envp, f),
   thread_db_process(p, e, envp, a, f),
   ppc_process(p, e, a, envp, f),
   bootstrap_state(bg_init),
   pending_thread_alives(0)
{
}

bg_process::bg_process(Dyninst::PID pid_, int_process *proc_) :
   int_process(pid_, proc_),
   sysv_process(pid_, proc_),
   thread_db_process(pid_, proc_),
   ppc_process(pid_, proc_),
   bootstrap_state(bg_init),
   pending_thread_alives(0)
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

bool bg_process::plat_writeMemAsync(int_thread *, const void *local, Dyninst::Address addr,
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

bool bg_process::plat_writeMem(int_thread *, const void *, Dyninst::Address, size_t)
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

Dyninst::Address bg_process::plat_mallocExecMemory(Dyninst::Address /*addr*/, unsigned /*size*/)
{
   assert(0);
   return 0;
}

bool bg_process::plat_individualRegAccess()
{
   return true;
}

bool bg_process::plat_createDeallocationSnippet(Dyninst::Address /*addr*/, unsigned long /*size*/, void* &/*buffer*/,
                                                unsigned long &/*buffer_size*/, unsigned long &/*start_offset*/)
{
   assert(0);
   return false;
}

bool bg_process::plat_createAllocationSnippet(Dyninst::Address /*addr*/, bool /*use_addr*/, unsigned long /*size*/, 
                                              void* &/*buffer*/, unsigned long &/*buffer_size*/, 
                                              unsigned long &/*start_offset*/)
{
   assert(0);
   return false;
}

bool bg_process::plat_collectAllocationResult(int_thread */*thr*/, reg_response::ptr /*resp*/)
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
   
   pthrd_printf("Debug interface version = %d, phys_procs = %d, virt_procs = %d\n",
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

bool bg_process::plat_getOSRunningState(Dyninst::LWP) const 
{
   return true;
}

void bg_process::addHeldArchEvent(ArchEventBlueGene *ae)
{
   held_arch_events.push(ae);
}

bool bg_process::hasHeldArchEvent()
{
   return !held_arch_events.empty();
}

void bg_process::readyHeldArchEvent()
{
   ArchEventBlueGene *ae = held_arch_events.front();
   held_arch_events.pop();

   GeneratorBlueGene *gen = static_cast<GeneratorBlueGene *>(Generator::getDefaultGenerator());
   gen->addHeldArchEvent(ae);
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
   bg_thread *bgthrd;
   if (initial_thrd) {
      int proto, phys, virt;
      bg_process::getVersionInfo(proto, phys, virt);
      bgthrd = new bg_thread(proc, thr_id, proto == 1 ? 0 : BG_INITIAL_THREAD_ID);
   }
   else {
      bgthrd = new bg_thread(proc, thr_id, lwp_id);
   }
   assert(bgthrd);
   return static_cast<int_thread *>(bgthrd);
}

bg_thread::bg_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l) :
   thread_db_thread(p, t, l),
   decoderPendingStop_(false)
{
}

bg_thread::~bg_thread()
{
}

bool bg_thread::plat_cont()
{
   bg_process *bgproc = dynamic_cast<bg_process *>(llproc());
   if (bgproc->hasHeldArchEvent()) 
   {
      pthrd_printf("Not really continuing thread %d/%d.  Held Arch event will be thrown instead\n",
                   getLWP(), bgproc->getPid());
      bgproc->readyHeldArchEvent();
      return true;
   }
   if (singleStep())
   {
      pthrd_printf("Sending SINGLESTEP to thread %d/%d\n", llproc()->getPid(), getLWP());
      BG_Debugger_Msg msg(SINGLE_STEP, llproc()->getPid(), getLWP(), 0, 0);
      msg.header.dataLength = sizeof(msg.dataArea.SINGLE_STEP);
      bool result = BGSend(msg);
      if (!result) {
         perr_printf("Error sending SINGLE_STEP message\n");
         return false;
      }
      return true;
   }
   int_threadPool *tp = llproc()->threadPool();
   if (tp->initialThread() != this) {
      pthrd_printf("Not continuing non-initial thread\n");
      return true;
   }
   
   Dyninst::LWP lwp_to_cont = getLWP();
   int sig_to_cont = 0;

   for (int_threadPool::iterator i = tp->begin(); i != tp->end(); i++) {
      bg_thread *thr = static_cast<bg_thread *>(*i);
      if (thr->continueSig_) {
         lwp_to_cont = thr->getLWP();
         sig_to_cont = thr->continueSig_;
         thr->continueSig_ = 0;
         break;
      }
   }
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
   if (llproc()->threadPool()->initialThread() != this) {
      pthrd_printf("Not stopping non-initial thread\n");
      return true;
   }

   BG_Debugger_Msg msg(KILL, llproc()->getPid(), getLWP(), 0, 0);
   msg.dataArea.KILL.signal = SIGSTOP;
   msg.header.dataLength = sizeof(msg.dataArea.KILL);
   
   pthrd_printf("Sending sigstop KILL msg to %d/%d\n", llproc()->getPid(), getLWP());
   bool result = BGSend(msg);
   if (!result) {
      perr_printf("Error sending STOP message\n");
      return false;
   }
   setDecoderPendingStop(true);

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

bool bg_thread::plat_getAllRegistersAsync(allreg_response::ptr resp)
{
   BG_Debugger_Msg msg(GET_ALL_REGS, llproc()->getPid(), getLWP(), resp->getID(), 0);
   msg.header.dataLength = sizeof(msg.dataArea.GET_ALL_REGS);

   pthrd_printf("Sending GET_ALL_REG to %d/%d\n", llproc()->getPid(), getLWP());

   bool result = BGSend(msg);
   if (!result) {
      pthrd_printf("Error sending GET_REG message\n");
      return false;
   }
   
   return true;   
}

bool bg_thread::plat_setAllRegistersAsync(int_registerPool &/*pool*/,
                                          result_response::ptr /*resp*/)
{
   assert(0);
   return false;
}

bool bg_thread::plat_getRegisterAsync(Dyninst::MachRegister reg, 
                                      reg_response::ptr resp)
{
   BG_Debugger_Msg msg(GET_REG, llproc()->getPid(), getLWP(), resp->getID(), 0);
   msg.dataArea.GET_REG.registerNumber = DynToBGGPRReg(reg);
   msg.header.dataLength = sizeof(msg.dataArea.GET_REG);

   pthrd_printf("Sending GET_REG of %s to %d/%d\n", reg.name(), llproc()->getPid(), getLWP());

   bool result = BGSend(msg);
   if (!result) {
      pthrd_printf("Error sending GET_REG message\n");
      return false;
   }
   
   return true;
}

bool bg_thread::plat_setRegisterAsync(Dyninst::MachRegister reg, 
                                      Dyninst::MachRegisterVal val,
                                      result_response::ptr resp)
{
   BG_Debugger_Msg msg(SET_REG, llproc()->getPid(), getLWP(), resp->getID(), 0);
   msg.dataArea.SET_REG.registerNumber = DynToBGGPRReg(reg);
   msg.dataArea.SET_REG.value = (BG_GPR_t) val;
   msg.header.dataLength = sizeof(msg.dataArea.SET_REG);

   pthrd_printf("Sending SET_REG of %s to %d/%d\n", reg.name(), llproc()->getPid(), getLWP());

   bool result = BGSend(msg);
   if (!result) {
      pthrd_printf("Error sending SET_REG message\n");
      return false;
   }
   
   return true;
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

 
bool bg_thread::plat_suspend() 
{ 
   return true; 
}

bool bg_thread::plat_resume() 
{ 
   return true; 
}

bool bg_thread::decoderPendingStop()
{
   return decoderPendingStop_;
}

void bg_thread::setDecoderPendingStop(bool b)
{
   decoderPendingStop_ = b;
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
      pthrd_printf("Recieved SIGSTOP during process initialization\n");
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
         for (unsigned i=1; i<=16; i++) {
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
   //thread_db_process::addThreadDBHandlers(hpool);
   return hpool;
}

static bool BGSend(BG_Debugger_Msg &msg)
{
   static Mutex send_lock;
   pthrd_printf("Sending message to CIOD\n");
   if (dyninst_debug_proccontrol) {
      dumpMessage(&msg);
   }
   send_lock.lock();
   bool result = BG_Debugger_Msg::writeOnFd(BG_DEBUGGER_WRITE_PIPE, msg);
   send_lock.unlock();
   if (!result) {
      perr_printf("Failure sending message on BG pipe\n");
   }
   return result;
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

   if (port_num > 0)
   {
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
   }
   else {
      fd = -1;
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
      int sresult = stat("/g/g0/legendre/pc_force_shutdown", &buf);
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
         pthrd_printf("CIOD shutdown the debug port\n");
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
