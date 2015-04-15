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

#include "Generator.h"
#include "Decoder.h"
#include "PCErrors.h"
#include "int_process.h"
#include "processplat.h"
#include "sysv.h"
#include "ppc_process.h"
#include "procpool.h"
#include "int_thread_db.h"
#include "mmapalloc.h"
#include "resp.h"

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
class HandlePreControlAuthority;
class HandlePostControlAuthority;

template <class CmdType, class AckType> class Transaction;

class bgq_process : 
   public sysv_process,
   public thread_db_process,
   public ppc_process,
   public hybrid_lwp_control_process,
   public mmap_alloc_process,
   public int_multiToolControl,
   public int_signalMask,
   public int_callStackUnwinding,
   public int_BGQData,
   public int_remoteIO,
   public int_memUsage
{
   friend class ComputeNode;
   friend class HandlerBGQStartup;
   friend class HandlePreControlAuthority;
   friend class HandlePostControlAuthority;
   friend class DecoderBlueGeneQ;
   friend class bgq_thread;
  private:
   static bool sendMessage(const ToolMessage &msg, uint16_t msg_type, uint32_t rank,
                           response::ptr resp);

   bool sendCommand(const ToolCommand &cmd, uint16_t cmd_type, response::ptr resp = response::ptr(), unsigned int resp_mod = 0);
   bool sendCommand(const ToolCommand &cmd, uint16_t cmd_type, Resp::ptr resp);

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
   virtual bool plat_detach(result_response::ptr resp, bool leave_stopped);
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
   virtual unsigned int plat_getCapabilities();
   virtual Event::ptr plat_throwEventsBeforeContinue(int_thread *thr);

   void getStackInfo(bgq_thread *thr, CallStackCallback *cbs);
   virtual bool plat_getStackInfo(int_thread *thr, stack_response::ptr stk_resp);
   virtual bool plat_handleStackInfo(stack_response::ptr stk_resp, CallStackCallback *cbs);

   virtual bool plat_waitAndHandleForProc();
   virtual bool plat_readMem(int_thread *thr, void *local, Dyninst::Address addr, size_t size);
   virtual bool plat_writeMem(int_thread *thr, const void *local, Dyninst::Address addr, size_t size, bp_write_t bp_write);
   virtual bool plat_needsAsyncIO() const;
   virtual bool plat_readMemAsync(int_thread *thr, Dyninst::Address addr, 
                                  mem_response::ptr result);
   virtual bool plat_writeMemAsync(int_thread *thr, const void *local, Dyninst::Address addr,
                                   size_t size, result_response::ptr result, bp_write_t bp_write);
   bool internal_readMem(int_thread *stop_thr, Dyninst::Address addr, 
                         mem_response::ptr resp, int_thread *thr);
   bool internal_writeMem(int_thread *stop_thr, const void *local, Dyninst::Address addr,
                          size_t size, result_response::ptr result, int_thread *thr, bp_write_t bp_write);
   virtual bool plat_needsThreadForMemOps() const;

   virtual bool plat_preHandleEvent();
   virtual bool plat_postHandleEvent();
   virtual bool plat_preAsyncWait();
   bool rotateTransaction();

   virtual bool plat_individualRegRead(Dyninst::MachRegister reg, int_thread *thr);
   virtual bool plat_individualRegSet();

   virtual bool plat_getOSRunningStates(std::map<Dyninst::LWP, bool> &runningStates);

   virtual bool plat_suspendThread(int_thread *thrd);
   virtual bool plat_resumeThread(int_thread *thrd);
   virtual bool plat_debuggerSuspended();
   virtual void plat_threadAttachDone();

   virtual LWPTracking *getLWPTracking();
   virtual bool plat_lwpRefresh(result_response::ptr resp);
   virtual bool plat_lwpRefreshNoteNewThread(int_thread *thr);

   bool handleStartupEvent(void *data);
   ComputeNode *getComputeNode() const;
   uint32_t getRank();
   void fillInToolMessage(ToolMessage &toolmsg, uint16_t msg_type, response::ptr resp = response::ptr());

   bool decoderPendingStop();
   void setDecoderPendingStop(bool b);

   virtual std::string mtool_getName();
   virtual MultiToolControl::priority_t mtool_getPriority();
   virtual MultiToolControl *mtool_getMultiToolControl();

   virtual void bgq_getProcCoordinates(unsigned &a, unsigned &b, unsigned &c, unsigned &d, unsigned &e, unsigned &t) const;
   virtual unsigned int bgq_getComputeNodeID() const;
   virtual void bgq_getSharedMemRange(Dyninst::Address &start, Dyninst::Address &end) const;
   virtual void bgq_getPersistantMemRange(Dyninst::Address &start, Dyninst::Address &end) const;
   virtual void bgq_getHeapMemRange(Dyninst::Address &start, Dyninst::Address &end) const;

   virtual bool plat_getFileNames(FileSetResp_t *resp);
   virtual bool plat_getFileStatData(std::string filename, Dyninst::ProcControlAPI::stat64_ptr *stat_results,
                                     std::set<StatResp_t *> &resps);
   virtual bool plat_getFileDataAsync(int_eventAsyncFileRead *fileread);
   virtual int getMaxFileReadSize();

   virtual bool allowSignal(int signal_no);
   
   virtual int threaddb_getPid();

   bool fillInMemQuery(MemUsageResp_t *resp);
   bool handleMemQuery(MemUsageResp_t *resp);
   virtual bool plat_getStackUsage(MemUsageResp_t *resp);
   virtual bool plat_getHeapUsage(MemUsageResp_t *resp);
   virtual bool plat_getSharedUsage(MemUsageResp_t *resp);
   virtual bool plat_residentNeedsMemVals();
   virtual bool plat_getResidentUsage(unsigned long stacku, unsigned long heapu, unsigned long sharedu,
                                      MemUsageResp_t *resp);
  private:
   typedef Transaction<QueryMessage, QueryAckMessage> QueryTransaction;
   typedef Transaction<UpdateMessage, UpdateAckMessage> UpdateTransaction;
   
   QueryTransaction *query_transaction;
   UpdateTransaction *update_transaction;
   ComputeNode *cn;
   bgq_thread *last_ss_thread;
   LWPTracking *lwp_tracker;
   result_response::ptr lwp_tracking_resp;

   bool hasControlAuthority;
   bool interp_base_set;
   bool page_size_set;
   bool debugger_suspended;
   bool decoder_pending_stop;
   bool is_doing_temp_detach;
   bool stopped_on_startup;
   bool held_on_startup;
   bool got_startup_stop;

   uint32_t rank;

   unsigned int page_size;
   Address interp_base;

   enum {
      no_memquery,
      shared_memquery,
      stack_memquery,
      heap_memquery
   } cur_memquery;
   bool procdata_result_valid;
   GetProcessDataAckCmd get_procdata_result;
   GetAuxVectorsAckCmd get_auxvectors_result;
   GetThreadListAckCmd *get_thread_list;

   EventControlAuthority::ptr pending_control_authority; //Used for releasing control authority
   int_eventControlAuthority *stopwait_on_control_authority; //Used for gaining control authority
   std::string tooltag;
   uint8_t priority;
   MultiToolControl *mtool;
   enum {
      issue_attach = 0,
      waitfor_attach,
      issue_control_request,
      waitfor_control_request_ack,
      waitfor_control_request_notice,
      issue_data_collection,
      waitfor_data_or_stop,
      skip_control_request_signal,
      waitfor_control_request_signal,
      waitfor_data_collection,
      waits_done,
      step_insn,
      reissue_data_collection,
      startup_done,
      startup_donedone
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
};

class bgq_thread : public thread_db_thread, public ppc_thread
{
   friend class bgq_process;
   friend class DecoderBlueGeneQ;
  private:
   bool last_signaled;
   CallStackUnwinding *unwinder;
   Address last_ss_addr;
   stack_response::ptr pending_stack_resp;
   Address last_stack_start;
   Address last_stack_end;
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
   virtual bool plat_convertToSystemRegs(const int_registerPool &pool, unsigned char *regs, bool gpr_only = false);

   virtual CallStackUnwinding *getStackUnwinder();

   static void regBufferToPool(BG_Reg_t *gprs, BG_Special_Regs *sregs, int_registerPool &reg);
   static void regPoolToBuffer(int_registerPool &reg, BG_Reg_t *gprs, BG_Special_Regs *sregs);
   static void regPoolToUser(const int_registerPool &reg, void *user_area);
   static bool genRegToSystem(MachRegister reg, GeneralRegSelect &result);
   static bool specRegToSystem(MachRegister reg, SpecialRegSelect &result);
};

struct buffer_t {
   void *buffer;
   size_t size;
   bool is_heap_allocated;
   bool is_timeout;
   buffer_t(void *b, size_t s, bool h) :
     buffer(b),
     size(s),
     is_heap_allocated(h),
     is_timeout(false)
   {
   }
   buffer_t() :
     buffer(NULL),
     size(0),
     is_heap_allocated(false),
     is_timeout(false)
   {
   }

};

class ComputeNode
{
   friend class bgq_process;
   friend class WriterThread;
  private:
   static std::map<int, ComputeNode *> id_to_cn;
   static std::map<std::string, int> socket_to_id;
   static std::set<ComputeNode *> all_compute_nodes;
   static std::map<int, ComputeNode *> rank_to_cn;

   static bool constructSocketToID();
   ComputeNode(int cid);

   std::set<bgq_process *> procs;
   std::vector<int> former_procs;
   std::set<int> ranks_owned; //All ranks, not just ones we're attached to

   static bool fillInAllRanksOwned();
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

   int numRanksOwned();
   const std::set<bgq_process *> &getProcs() const { return procs; }
   ~ComputeNode();

   static const std::set<ComputeNode *> &allNodes();
   static void emergencyShutdown();
  private:
   int fd;
   int cn_id;
   Mutex<> send_lock;
   Mutex<> attach_lock;
   Mutex<> detach_lock;
   bool do_all_attach;
   bool issued_all_attach;
   bool all_attach_done;
   bool all_attach_error;

   bool have_pending_message;
   Mutex<> pending_queue_lock;
   std::queue<buffer_t> queued_pending_msgs;
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

//Handles the notice event, while we still have CA.  Removes BPs, etc
class HandlePreControlAuthority : public Handler
{
  public:
   HandlePreControlAuthority();
   ~HandlePreControlAuthority();

   virtual void getEventTypesHandled(vector<EventType> &etypes);
   virtual handler_ret_t handleEvent(Event::ptr ev);
};

//Handles the releasing of CA, triggers on the continue after a HandlePreControlAuthority event
class HandlePostControlAuthority : public Handler
{
  public:
   HandlePostControlAuthority();
   ~HandlePostControlAuthority();

   virtual void getEventTypesHandled(vector<EventType> &etypes);
   virtual handler_ret_t handleEvent(Event::ptr ev);
};

class GeneratorBGQ : public GeneratorMT
{
  friend class ComputeNode;
  private:
   int kick_pipe[2];
   static const int kick_val = 0xfeedf00d; //0xdeadbeef is passe

   bool readMessage(int fd, vector<ArchEvent *> &events);
   bool readMessage(vector<ArchEvent *> &events, bool block);
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
   bool timeout_msg;
  public:
   ArchEventBGQ(ToolMessage *msg);
   ArchEventBGQ();
   virtual ~ArchEventBGQ();

   ToolMessage *getMsg() const;
   bool isTimeout() const;
   void dontFreeMsg();
};

class DecoderBlueGeneQ : public Decoder
{
  private:
   Event::ptr decodeCompletedResponse(response::ptr resp, int_process *proc, int_thread *thrd,
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
                         Address addr, vector<Event::ptr> &events, bool allow_signal_decode = true);
   bool decodeGenericSignal(ArchEventBGQ *archevent, bgq_process *proc, int_thread *thr,
                            int signum, vector<Event::ptr> &events);
   bool decodeStop(ArchEventBGQ *archevent, bgq_process *proc, int_thread *thr,
                   std::vector<Event::ptr> &events);
   bool decodeStep(ArchEventBGQ *archevent, bgq_process *proc, int_thread *thr,
                   Address addr, std::vector<Event::ptr> &events);
   bool decodeExit(ArchEventBGQ *archevent, bgq_process *proc, std::vector<Event::ptr> &events);
   bool decodeControlNotify(ArchEventBGQ *archevent, bgq_process *proc, std::vector<Event::ptr> &events);
   bool decodeDetachAck(ArchEventBGQ *archevent, bgq_process *proc, std::vector<Event::ptr> &events);
   bool decodeReleaseControlAck(ArchEventBGQ *archevent, bgq_process *proc, int err_code, std::vector<Event::ptr> &events);
   bool decodeControlAck(ArchEventBGQ *ev, bgq_process *qproc, vector<Event::ptr> &events);
   bool decodeLWPRefresh(ArchEventBGQ *ev, bgq_process *proc, ToolCommand *cmd, std::vector<Event::ptr> &events);
   bool decodeTimeout(vector<Event::ptr> &events);
   bool decodeFileStat(ToolCommand *cmd, bgq_process *proc, unsigned int resp_id, int rc);
   bool decodeFileContents(ArchEventBGQ *ev, bgq_process *proc, ToolCommand *cmd, 
                           int rc, unsigned int resp_id, bool owns_msg,
                           std::vector<Event::ptr> &events);
   bool decodeGetFilenames(ArchEventBGQ *ev, bgq_process *proc, ToolCommand *cmd, int rc, int id);
   bool decodeMemoryQuery(ArchEventBGQ *ev, bgq_process *proc, ToolCommand *cmd, unsigned int resp_id, int rc);
   bool usesResp(uint16_t cmdtype);

   Event::ptr createEventDetach(bgq_process *proc, bool err);
 public:
   DecoderBlueGeneQ();
   virtual ~DecoderBlueGeneQ();
   virtual bool decode(ArchEvent *ae, std::vector<Event::ptr> &events);
   virtual unsigned getPriority() const;
};

class IOThread
{
  protected:
   static void mainWrapper(void *);

   IOThread();
   ~IOThread();
   void init();
   void kick();
   void thrd_main();
   virtual void run() = 0;
   virtual void localInit() = 0;
   virtual void thrd_kick();
   CondVar<> initLock;
   CondVar<> shutdownLock;
   bool do_exit;
   bool kicked;
   bool init_done;
   bool shutdown_done;
   DThread thrd;
   int lwp;
   int pid;
  public:
   void shutdown();
};

class ReaderThread : public IOThread
{
  protected:
   static ReaderThread *me;
   std::queue<buffer_t> msgs;
   std::set<int> fds;
   CondVar<> queue_lock;
   CondVar<> fd_lock;

   ReaderThread();
   virtual void run();
   virtual void localInit();
   int kick_fd;
   int kick_fd_write;
   bool is_gen_kicked;
   unsigned timeout_set;
   struct timeval timeout;
   Mutex<> timeout_lock;
  protected:
   virtual void thrd_kick();
  public:
   ~ReaderThread();
   static ReaderThread *get();
   buffer_t readNextElement(bool block);
   void addComputeNode(ComputeNode *cn);
   void rmComputeNode(ComputeNode *cn);
   void setKickPipe(int fd);
   void setTimeout(const struct timeval &tv);
   void clearTimeout();
   void kick_generator();
};

class WriterThread : public IOThread
{
  private:
   static WriterThread *me;
   std::map<int, ComputeNode *> rank_to_cn;
   Mutex<> rank_lock;
   WriterThread();
   virtual void run();
   virtual void localInit();
   
   std::vector<int> acks;
   std::vector<ComputeNode *> writes;
   CondVar<> msg_lock;
  protected:
   virtual void thrd_kick();
  public:
   ~WriterThread();
   static WriterThread *get();
   void writeMessage(buffer_t buf, ComputeNode *cn);
   void notifyAck(int rank);
   void addProcess(bgq_process *proc);
   void rmProcess(bgq_process *proc);
   void forcePastAck(ComputeNode *cn);
};


class DebugThread
{
  private:
   DThread debug_thread;
   static DebugThread *me;
   bool shutdown;
   bool initialized;
   int pfd[2];
   CondVar<> init_lock;
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
