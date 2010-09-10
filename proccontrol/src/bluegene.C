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

#if   defined(os_bgl)
#include "external/bluegene/bgl-debugger-interface.h"
#elif defined (os_bgp)
#include "external/bluegene/bgp-debugger-interface.h"
#else
#error "ERROR: No suitable debug interface for this BG ION."
#endif

using namespace DebuggerInterface;
using namespace Dyninst;
using namespace std;

static BG_GPR_Num_t DynToBGGPRReg(Dyninst::MachRegister reg);
static Dyninst::MachRegister BGGPRToDynReg(BG_GPR_Num_t bg_regnum);
#define NUM_GPRS 41

static bool bg_fdHasData(int fd);

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
   GeneratorMT(std::string("BlueGene Generator"))
{
   decoders.insert(new DecoderBlueGene());
}

GeneratorBlueGene::~GeneratorBlueGene()
{
}

bool GeneratorBlueGene::initialize()
{
   return true;
}

bool GeneratorBlueGene::canFastHandle()
{
   return false;
}

ArchEvent *GeneratorBlueGene::getEvent(bool block)
{
   DebugEvent ev;   
   BG_Debugger_Msg *msg;
   
   if (!block && !bg_fdHasData(BG_DEBUGGER_READ_PIPE)) {
      return NULL;
   }
   
   msg = new BG_Debugger_msg();
   // Read an event from the debug filehandle.
   pthrd_printf("Reading event from BG file descriptor %d\n", BG_DEBUGGER_READ_PIPE);
   bool result = BG_Debugger_Msg::readFromFd(BG_DEBUGGER_READ_PIPE, msg);
   if (!result) {
      pthrd_printf("Failure waiting for debug event\n");
      delete msg;
      return NULL;
   }
   
   // extract process and thread id from the BG message.
   pid_t pid = msg.header.nodeNumber;   
   THR_ID tid = msg.header.thread;
   int returnCode = msg.header.returnCode;
   pthrd_printf("Received debug event %s from pid %d, tid %d, rc %d\n",
             BG_Debugger_Msg::getMessageName(msg.header.messageType), pid, tid, returnCode);
   
   if (returnCode > 0) {
      // Print out a message if we get a return code we don't expect.  Consult the debugger header
      // for the meanings of these.
      pthrd_printf("Warning. Return code for %s on pid %d, tid %d was non-zero: %d\n",
                BG_Debugger_Msg::getMessageName(msg.header.messageType), pid, tid,
                msg.header.returnCode);
   }

   ArchEvent *aevent = new ArchEventBlueGene(msg);
   assert(aevent);
   return aevent;
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

Event::ptr DecoderBlueGene::decodeGetRegAck(BG_Debugger_Msg *msg)
{
   pthrd_printf("Decode get reg ack\n");
   getResponses().lock();
   
   response::ptr resp = getResponses().rmResponse(msg->header.sequence);
   assert(resp);
   reg_response reg_resp = resp->getRegResponse();
   assert(reg_resp);
   reg_resp->postResponse(msg->dataArea.value);
         
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
   allreg_response areg_resp = resp->getRegResponse();
   assert(areg_resp);

   int_registerPool *pool = areg_resp->getRegPool();
   BG_GPR_t *reg_values = (BG_GPR_t *) &(msg->dataArea.GET_ALL_REGS_ACK.gprs);
   pool->regs.clear();
   for (unsigned i=0; i<NUM_GPRS; i++) {
      Dyninst::MachRegister dyn_reg = BGGPRToDynReg(i);
      if (dyn_reg == Dyninst::InvalidRegister)
         continue;
      pool->regs[dyn_reg] = reg_values[i];
   }

   reg_resp->postResponse();
         
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
   mem_response mem_resp = resp->getMemResponse();
   assert(mem_resp);

   mem_resp->postResponse(msg->dataArea.GET_MEM_ACK.data, msg->dataArea.GET_MEM_ACK.len);
         
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
   result_response result_resp = resp->getMemResponse();
   assert(result_resp);

   bool has_error = msg->header.returnCode;
   mem_resp->postResponse(!has_error)
         
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
   assert(archE);
   Event::ptr event = Event::ptr();

   BG_Debugger_Msg *msg = archE->getMsg();
   
   bg_process *proc = NULL;
   bg_thread *thread = NULL;
   bool result = getProcAndThread(proc, thread);
   if (!result)
      return false;
   
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
      case SET_FLOAT_REG_ACK:
      case GET_FLOAT_REG_ACK:
      case GET_ALL_FLOAT_REGS_ACK:
         assert(0); //TODO
         break;
      case SINGLE_STEP_ACK:
         pthrd_printf("Decoded SINGLE_STEP_ACK, dropping...\n");
         assert(thread->singleStepMode());
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
         new_event = EventBootstrap::ptr(new EventBootstrap());         
         break;
      case DETACH_ACK:
         pthrd_printf("Decoded DETACH_ACK, creating Detached event\n");
         new_event = EventDetached::ptr(new EventDetached());
         break;
      case PROGRAM_EXITED:
      case VERSION_MSG_ACK:
      case SIGNAL_ENCOUNTERED:
   }

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
   bool result = BG_Debugger_Msg::writeOnFd(BG_DEBUGGER_WRITE_PIPE, msg);
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

bool bg_process::plat_detach()
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
   bool result = BG_Debugger_Msg::writeOnFd(BG_DEBUGGER_WRITE_PIPE, msg);
   if (!result) {
      pthrd_printf("Error sending DETACH message\n");
      return false;
   }
   return true;   
}

bool bg_process::plat_terminate(bool &needs_sync)
{
   BG_Debugger_Msg msg(KILL, getPid(), 0, 0, 0);
   msgs.dataArea.KILL.signal = SIGKILL;
   msg.header.dataLength = sizeof(msg.dataArea.KILL);
   
   pthrd_printf("Sending KILL-9 message to %d\n", getPid());
   bool result = BG_Debugger_Msg::writeOnFd(BG_DEBUGGER_WRITE_PIPE, msg);
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
                                  mem_response::ptr result)
{
   pthrd_printf("Reading from memory %lx +%lx on %d with response ID %d\n", 
                addr, result->getSize(), getPid(), result->getID());
   assert(size < BG_Debugger_Msg_MAX_MEM_SIZE); //MATT TODO

   BG_Debugger_Msg msg(GET_MEM, getPid(), 0, result->getID(), 0);
   msg.dataArea.GET_MEM.addr = addr;
   msg.dataArea.GET_MEM.len = result->getSize();
   msg.header.dataLength = sizeof(msg.dataArea.GET_MEM);
   
   bool result = BG_Debugger_Msg::writeOnFd(BG_DEBUGGER_WRITE_PIPE, msg);
   if (!result) {
      pthrd_printf("Error sending GET_MEM message\n");
      return false;
   }
   return true;   
}

bool bg_process::plat_writeMemAsync(int_thread *thr, void *local, Dyninst::Address addr,
                                   size_t size, result_response::ptr result)
{
   pthrd_printf("Writing memory %lx +%lx on %d with response ID %d\n", 
                addr, size, getPid(), result->getID());
   assert(size < BG_Debugger_Msg_MAX_MEM_SIZE); //MATT TODO

   BG_Debugger_Msg msg(SET_MEM, getPid(), 0, result->getID(), 0);
   msg.dataArea.SET_MEM.addr = addr;
   msg.dataArea.SET_MEM.len = size;
   memcpy(msg.dataArea.SET_MEM.data, local, size);
   msg.header.dataLength = sizeof(msg.dataArea.SET_MEM);
   
   bool result = BG_Debugger_Msg::writeOnFd(BG_DEBUGGER_WRITE_PIPE, msg);
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
   return false;
}

Dyninst::Architecture bg_process::getTargetArch()
{
   return Dyninst::Arch_ppc32;
}

unsigned bg_process::getTargetPageSize()
{
   return 4092;
}

Dyninst::Address bg_process::plat_mallocExecMemory(Dyninst::Address, unsigned size)
{
   assert(0); //TODO: Figure this one out.
}

bool bg_process::plat_individualRegAccess()
{
   return true;
}


bg_thread::bg_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l)
{
}

bg_thread::~bg_thread()
{
}

bool bg_thread::plat_cont()
{
}

virtual bool bg_thread::plat_stop()
{
}

   virtual bool plat_getAllRegisters(int_registerPool &reg);
   virtual bool plat_getRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal &val);
   virtual bool plat_setAllRegisters(int_registerPool &reg);
   virtual bool plat_setRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal val);
   virtual bool attach();   
};

static bool bg_fdHasData(int fd) {
   int result;
   struct pollfd fds;
   fds.fd = fd;
   fds.events = POLLIN;
   fds.revents = 0;
   result = poll(&fds, 1, 0);
   if (result == -1) {
      int errnum = errno;
      sw_printf("[%s:%u] - Unable to poll fd %d: %s\n", __FILE__, __LINE__, 
                fd, strerror(errnum));
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
#define CASE_GPR(NUM) case BG_GPR ## NUM: return Dyninst::ppc32::r ## NUM
#define CASE_SPEC(BLUEG, DYN) case BG_ ## BLUEG : return Dyninst::ppc32:: ## DYN
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
      return Dyninst::InvalidReg;
   }
}
