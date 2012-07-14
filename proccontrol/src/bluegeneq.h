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

#include "proccontrol/h/Generator.h"
#include "proccontrol/h/Decoder.h"
#include "proccontrol/h/PCErrors.h"
#include "proccontrol/src/int_process.h"
#include "proccontrol/src/sysv.h"
#include "proccontrol/src/ppc_process.h"
#include "proccontrol/src/procpool.h"
#include "proccontrol/src/int_thread_db.h"
#include "proccontrol/src/mmapalloc.h"

#include "ramdisk/include/services/MessageHeader.h"
#include "ramdisk/include/services/ToolctlMessages.h"

#include <queue>
#include <set>
#include <map>
#include <vector>

using namespace bgcios;
using namespace toolctl;

namespace bgq 
{

class bgq_process;
class bgq_thread;
class ComputeNode;
class ArchEventBGQ;

template <class CmdType, class AckType> class Transaction;

class bgq_process : 
   public sysv_process,
   public thread_db_process,
   public ppc_process,
   public hybrid_lwp_control_process,
   public mmap_alloc_process
{
   friend class ComputeNode;
   friend class HandlerBGQStartup;
   friend class DecoderBlueGeneQ;
   friend class bgq_thread;
  private:
   static bool sendMessage(const ToolMessage &msg, uint16_t msg_type, uint32_t rank,
                           response::ptr resp);

   bool sendCommand(const ToolCommand &cmd, uint16_t cmd_type, response::ptr resp = response::ptr(), unsigned int resp_mod = 0);

  public:
   static uint32_t getCommandLength(uint16_t cmd_type, const ToolCommand &cmd);
   static uint32_t getMessageLength(const ToolMessage &msg);
   static uint32_t getCommandAckLength(uint16_t cmd_type, const ToolCommand &cmd);
   static uint16_t getCommandMsgType(uint16_t cmd_id);
   static uint16_t getCommandExpectedAck(uint16_t cmd_id);
   static const char *bgqErrorMessage(uint32_t retcode, bool long_form = true);
   static bool isActionCommand(uint16_t cmd_type);
   static uint64_t getJobID();
   static uint32_t getToolID();

   bgq_process(Dyninst::PID p, std::string e, std::vector<std::string> a, 
              vector<string> envp, std::map<int, int> f);
   virtual ~bgq_process();

   virtual bool plat_create();
   virtual bool plat_attach(bool all_stopped, bool &needsSync);
   virtual bool plat_forked();
   virtual bool plat_detach(result_response::ptr resp);
   virtual bool plat_detachDone();
   virtual bool plat_terminate(bool &needs_sync);
   virtual bool needIndividualThreadAttach();
   virtual bool getThreadLWPs(std::vector<Dyninst::LWP> &lwps);
   virtual bool plat_getInterpreterBase(Address &base);
   virtual bool plat_supportDOTF();
   virtual OSType getOS() const;
   virtual Dyninst::Architecture getTargetArch();
   virtual unsigned int getTargetPageSize();
   virtual Dyninst::Address plat_mallocExecMemory(Dyninst::Address, unsigned int);
   virtual bool plat_individualRegAccess();
   virtual bool plat_supportLWPPostDestroy();
   virtual SymbolReaderFactory *plat_defaultSymReader();
   virtual void noteNewDequeuedEvent(Event::ptr ev);

   void getStackInfo(bgq_thread *thr, CallStackCallback *cbs);
   virtual bool plat_getStackInfo(int_thread *thr, stack_response::ptr stk_resp);
   virtual bool plat_handleStackInfo(stack_response::ptr stk_resp, CallStackCallback *cbs);

   virtual bool plat_waitAndHandleForProc();
   virtual bool plat_readMem(int_thread *thr, void *local, Dyninst::Address addr, size_t size);
   virtual bool plat_writeMem(int_thread *thr, const void *local, Dyninst::Address addr, size_t size);
   virtual bool plat_needsAsyncIO() const;
   virtual bool plat_readMemAsync(int_thread *thr, Dyninst::Address addr, 
                                  mem_response::ptr result);
   virtual bool plat_writeMemAsync(int_thread *thr, const void *local, Dyninst::Address addr,
                                   size_t size, result_response::ptr result);
   bool internal_readMem(int_thread *stop_thr, Dyninst::Address addr, 
                         mem_response::ptr resp, int_thread *thr);
   bool internal_writeMem(int_thread *stop_thr, const void *local, Dyninst::Address addr,
                          size_t size, result_response::ptr result, int_thread *thr);

   virtual bool plat_preHandleEvent();
   virtual bool plat_postHandleEvent();
   virtual bool plat_preAsyncWait();
   bool rotateTransaction();

   virtual bool plat_individualRegRead();
   virtual bool plat_individualRegSet();

   virtual bool plat_getOSRunningStates(std::map<Dyninst::LWP, bool> &runningStates);

   virtual bool plat_suspendThread(int_thread *thrd);
   virtual bool plat_resumeThread(int_thread *thrd);
   virtual bool plat_debuggerSuspended();
   virtual void plat_threadAttachDone();

   bool handleStartupEvent(void *data);
   ComputeNode *getComputeNode();
   uint32_t getRank();
   void fillInToolMessage(ToolMessage &toolmsg, uint16_t msg_type, response::ptr resp = response::ptr());

   bool decoderPendingStop();
   void setDecoderPendingStop(bool b);

   virtual std::string mtool_getName();
   virtual MultiToolControl::priority_t mtool_getPriority();
   virtual MultiToolControl *mtool_getMultiToolControl();

  private:
   typedef Transaction<QueryMessage, QueryAckMessage> QueryTransaction;
   typedef Transaction<UpdateMessage, UpdateAckMessage> UpdateTransaction;
   
   QueryTransaction *query_transaction;
   UpdateTransaction *update_transaction;
   ComputeNode *cn;
   int_thread *last_ss_thread;

   bool hasControlAuthority;
   bool interp_base_set;
   bool page_size_set;
   bool debugger_suspended;
   bool decoder_pending_stop;
   bool is_doing_temp_detach;

   uint32_t rank;

   unsigned int page_size;
   Address interp_base;

   GetProcessDataAckCmd get_procdata_result;
   GetAuxVectorsAckCmd get_auxvectors_result;
   GetThreadListAckCmd *initial_thread_list;

   string tooltag;
   uint8_t priority;
   MultiToolControl *mtool;
   enum {
      issue_attach = 0,
      waitfor_attach,
      issue_control_request,
      waitfor_control_request_ack,
      waitfor_control_request_notice,
      skip_control_request_signal,
      waitfor_control_request_signal,
      issue_data_collection,
      waitfor_data_collection,
      data_collected,
      startup_done
   } startup_state;

   enum {
      no_detach_issued,
      issue_control_release,
      control_release_sent,
      choose_detach_mechanism,
      issued_all_detach,
      issued_detach,
      waitfor_all_detach,
      detach_cleanup,
      detach_done
   } detach_state;

   static uint64_t jobid;
   static uint32_t toolid;
   static bool set_ids;
   static bool do_all_attach;

   static set<void *> held_msgs;
   static unsigned int num_pending_stackwalks;
};

class bgq_thread : public thread_db_thread, ppc_thread
{
   friend class bgq_process;
  private:
   bool last_signaled;
   CallStackUnwinding *unwinder;
  public:
   bgq_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l);
   virtual ~bgq_thread();

   virtual bool plat_cont();
   virtual bool plat_stop();
   virtual bool plat_getAllRegisters(int_registerPool &reg);
   virtual bool plat_getRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal &val);
   virtual bool plat_setAllRegisters(int_registerPool &reg);
   virtual bool plat_setRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal val);
   virtual bool attach();

   virtual bool plat_getAllRegistersAsync(allreg_response::ptr result);
   virtual bool plat_getRegisterAsync(Dyninst::MachRegister reg, 
                                      reg_response::ptr result);
   virtual bool plat_setAllRegistersAsync(int_registerPool &pool,
                                          result_response::ptr result);
   virtual bool plat_setRegisterAsync(Dyninst::MachRegister reg, 
                                      Dyninst::MachRegisterVal val,
                                      result_response::ptr result);
   virtual bool plat_convertToSystemRegs(const int_registerPool &pool, unsigned char *regs);

   virtual CallStackUnwinding *getStackUnwinder();

   static void regBufferToPool(BG_Reg_t *gprs, BG_Special_Regs *sregs, int_registerPool &reg);
   static void regPoolToBuffer(int_registerPool &reg, BG_Reg_t *gprs, BG_Special_Regs *sregs);
   static void regPoolToUser(const int_registerPool &reg, void *user_area);
   static bool genRegToSystem(MachRegister reg, GeneralRegSelect &result);
   static bool specRegToSystem(MachRegister reg, SpecialRegSelect &result);
};

class ComputeNode
{
   friend class bgq_process;
  private:
   static std::map<int, ComputeNode *> id_to_cn;
   static std::map<std::string, int> socket_to_id;
   static std::set<ComputeNode *> all_compute_nodes;

   static bool constructSocketToID();
   ComputeNode(int cid);

   std::set<bgq_process *> procs;
  public:
   static ComputeNode *getComputeNodeByID(int cn_id);
   static ComputeNode *getComputeNodeByRank(uint32_t rank);

   int getFD() const;
   int getID() const; //The BG/Q doc calls this the 'pid', which is unfortunate naming cause 
                      // it doesn't represent a process, but a compute node

   bool writeToolMessage(bgq_process *proc, ToolMessage *msg, bool heap_alloced);
   bool writeToolAttachMessage(bgq_process *proc, ToolMessage *msg, bool heap_alloced);
   bool flushNextMessage();
   bool handleMessageAck();

   bool reliableWrite(void *buffer, size_t buffer_size);
   void removeNode(bgq_process *proc);

   const std::set<bgq_process *> &getProcs() const { return procs; }
   ~ComputeNode();

   static const std::set<ComputeNode *> &allNodes();
   static void emergencyShutdown();
  private:
   int fd;
   int cn_id;
   Mutex send_lock;
   Mutex attach_lock;
   Mutex detach_lock;
   bool do_all_attach;
   bool issued_all_attach;
   bool all_attach_done;
   bool all_attach_error;
   bool have_pending_message;
   std::queue<std::pair<void *, size_t> > queued_pending_msgs;
};

class HandleBGQStartup : public Handler
{
  public:
   HandleBGQStartup();
   ~HandleBGQStartup();

   virtual void getEventTypesHandled(vector<EventType> &etypes);
   virtual handler_ret_t handleEvent(Event::ptr ev);
   virtual int getPriority() const;
};

class GeneratorBGQ : public GeneratorMT
{
  friend class ComputeNode;
  private:
   int kick_pipe[2];
   static const int kick_val = 0xfeedf00d; //0xdeadbeef is passe

   bool readMessage(int fd, vector<ArchEvent *> &events);
   bool read_multiple_msgs;
   bool reliableRead(int fd, void *buffer, size_t buffer_size, int timeout_s = -1);

 public:
   GeneratorBGQ();
   virtual ~GeneratorBGQ();

   virtual bool initialize();
   virtual bool canFastHandle();
   virtual ArchEvent *getEvent(bool block);
   virtual bool getMultiEvent(bool block, vector<ArchEvent *> &events);
   virtual bool plat_skipGeneratorBlock();
   void kick();
   void shutdown();
};

class ArchEventBGQ : public ArchEvent
{
  private:
   ToolMessage *msg;
   bool free_msg;
  public:
   ArchEventBGQ(ToolMessage *msg);
   virtual ~ArchEventBGQ();

   ToolMessage *getMsg() const;
   void dontFreeMsg();
};

class DecoderBlueGeneQ : public Decoder
{
  private:
   Event::ptr decodeCompletedResponse(response::ptr resp,
                                      map<Event::ptr, EventAsync::ptr> &async_evs);

   bool decodeStartupEvent(ArchEventBGQ *ae, bgq_process *proc, 
                           void *data, Event::SyncType stype, vector<Event::ptr> &events);
   bool decodeUpdateOrQueryAck(ArchEventBGQ *ae, bgq_process *proc,
                               int num_commands, CommandDescriptor *cmd_list, 
                               vector<Event::ptr> &events);
   bool decodeNotifyMessage(ArchEventBGQ *archevent, bgq_process *proc,
                            vector<Event::ptr> &events);
   bool decodeSignal(ArchEventBGQ *archevent, bgq_process *proc,
                     vector<Event::ptr> &events);
   bool decodeBreakpoint(ArchEventBGQ *archevent, bgq_process *proc, int_thread *thr,
                         Address addr, vector<Event::ptr> &events);
   bool decodeGenericSignal(ArchEventBGQ *archevent, bgq_process *proc, int_thread *thr,
                            int signum, vector<Event::ptr> &events);
   bool decodeStop(ArchEventBGQ *archevent, bgq_process *proc, int_thread *thr,
                   std::vector<Event::ptr> &events);
   bool decodeStep(ArchEventBGQ *archevent, bgq_process *proc, int_thread *thr,
                   std::vector<Event::ptr> &events);
   bool decodeExit(ArchEventBGQ *archevent, bgq_process *proc, std::vector<Event::ptr> &events);
   bool decodeControlNotify(ArchEventBGQ *archevent, bgq_process *proc, std::vector<Event::ptr> &events);
   bool decodeDetachAck(ArchEventBGQ *archevent, bgq_process *proc, std::vector<Event::ptr> &events);
   bool decodeReleaseControlAck(ArchEventBGQ *archevent, bgq_process *proc, int err_code, std::vector<Event::ptr> &events);

   Event::ptr createEventDetach(bgq_process *proc, bool err);
 public:
   DecoderBlueGeneQ();
   virtual ~DecoderBlueGeneQ();
   virtual bool decode(ArchEvent *ae, std::vector<Event::ptr> &events);
   virtual unsigned getPriority() const;
};

class DebugThread
{
  private:
   DThread debug_thread;
   static DebugThread *me;
   bool shutdown;
   bool initialized;
   int pfd[2];
   CondVar init_lock;
   bool init();
   DebugThread();
  public:
   static DebugThread *getDebugThread();
   ~DebugThread();
   static void mainLoopWrapper(void *);
   void mainLoop();
};


#include "bgq-transactions.h"

}
