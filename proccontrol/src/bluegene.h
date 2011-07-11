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

#ifndef BLUEGENE_H_
#define BLUEGENE_H_

#include "proccontrol/h/Generator.h"
#include "proccontrol/h/Decoder.h"
#include "proccontrol/h/PCErrors.h"
#include "proccontrol/src/int_process.h"
#include "proccontrol/src/sysv.h"
#include "proccontrol/src/ppc_process.h"
#include "proccontrol/src/procpool.h"
#include "proccontrol/src/int_thread_db.h"

#define SINGLE_STEP_SIG 32064
#define DEBUG_REG_SIG 32066

#include "external/bluegene/bgp-debugger-interface.h"

#define BG_INITIAL_THREAD_ID 5

class ArchEventBlueGene;

class bg_process : public sysv_process, public thread_db_process, public ppc_process
{
   friend class HandleBGAttached;
   friend class DecoderBlueGene;
   friend class GeneratorBlueGene;
  private:
   enum {
      bg_init,
      bg_stop_pending,
      bg_stopped,
      bg_auxv_pending,
      bg_auxv_done,
      bg_thread_pending,
      bg_ready,
      bg_bootstrapped,
      bg_done
   } bootstrap_state;

   DebuggerInterface::BG_Process_Data_t procdata;
   
   signed int pending_thread_alives;
   std::set<int> initial_lwps;
   std::queue<ArchEventBlueGene *> held_arch_events;
   std::map<uint32_t, uint32_t> auxv_info;

  public:
   static int protocol_version;
   static int phys_procs;
   static int virt_procs;

   bg_process(Dyninst::PID p, std::string e, std::vector<std::string> a, 
              vector<string> envp, std::map<int, int> f);
   bg_process(Dyninst::PID pid_, int_process *proc_);

   virtual ~bg_process();

   virtual bool plat_create();
   virtual bool plat_create_int();
   virtual bool plat_attach(bool all_stopped);
   virtual bool plat_forked();
   virtual bool post_forked();
   virtual bool plat_detach(bool &needs_sync);
   virtual bool plat_terminate(bool &needs_sync);

   virtual bool plat_needsAsyncIO() const;
   virtual bool plat_readMemAsync(int_thread *, Dyninst::Address addr, 
                                  mem_response::ptr result);
   virtual bool plat_writeMemAsync(int_thread *thr, const void *local, Dyninst::Address addr,
                                   size_t size, result_response::ptr result);
   virtual bool plat_readMem(int_thread *thr, void *local, 
                             Dyninst::Address remote, size_t size);
   virtual bool plat_writeMem(int_thread *thr, const void *local, 
                              Dyninst::Address remote, size_t size);
   virtual bool plat_getOSRunningStates(std::map<Dyninst::LWP, bool> &runningStates);
   virtual bool needIndividualThreadAttach();
   virtual bool getThreadLWPs(std::vector<Dyninst::LWP> &lwps);
   virtual Dyninst::Architecture getTargetArch();
   virtual unsigned getTargetPageSize();
   virtual Dyninst::Address plat_mallocExecMemory(Dyninst::Address, unsigned size);
   virtual bool plat_individualRegAccess();   

   virtual bool plat_createDeallocationSnippet(Dyninst::Address addr, unsigned long size, void* &buffer,
                                               unsigned long &buffer_size, unsigned long &start_offset);
   virtual bool plat_createAllocationSnippet(Dyninst::Address addr, bool use_addr, unsigned long size, 
                                             void* &buffer, unsigned long &buffer_size, 
                                             unsigned long &start_offset);
   virtual bool plat_collectAllocationResult(int_thread *thr, reg_response::ptr resp);
   virtual bool plat_getInterpreterBase(Address &base);

   int_process::ThreadControlMode plat_getThreadControlMode() const;
   virtual SymbolReaderFactory *plat_defaultSymReader();
   unsigned plat_getRecommendedReadSize();

   void addHeldArchEvent(ArchEventBlueGene *ae);
   bool hasHeldArchEvent();
   void readyHeldArchEvent();

   static void getVersionInfo(int &protocol, int &phys, int &virt);
};

class bg_thread : public thread_db_thread
{
  public:
   bg_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l);
   virtual ~bg_thread();

   virtual bool plat_cont();
   virtual bool plat_stop();
   virtual bool plat_getAllRegisters(int_registerPool &reg);
   virtual bool plat_getRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal &val);
   virtual bool plat_setAllRegisters(int_registerPool &reg);
   virtual bool plat_setRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal val);
   virtual bool plat_getRegisterAsync(Dyninst::MachRegister reg, 
                                      reg_response::ptr result);
   virtual bool plat_setRegisterAsync(Dyninst::MachRegister reg, 
                                      Dyninst::MachRegisterVal val,
                                      result_response::ptr result);   
   bool plat_getAllRegistersAsync(allreg_response::ptr result);
   bool plat_setAllRegistersAsync(int_registerPool &pool, result_response::ptr result);
   virtual bool plat_convertToSystemRegs(const int_registerPool &pool, unsigned char *regs);
   virtual bool attach();   

   bool decoderPendingStop();
   void setDecoderPendingStop(bool b);
  private:
   bool decoderPendingStop_;
};

class ArchEventBlueGene : public ArchEvent
{
  private:
   DebuggerInterface::BG_Debugger_Msg *msg;
   response::ptr pc_resp;

  public:
   ArchEventBlueGene(DebuggerInterface::BG_Debugger_Msg *m);
   virtual ~ArchEventBlueGene();
   DebuggerInterface::BG_Debugger_Msg *getMsg() const;

   response::ptr getPCResp();
   void setPCResp(response::ptr r);
};

class DebugPortReader
{
  private:
   static const int port_num = 0; //9000
   DThread debug_port_thread;
   static DebugPortReader *me;
   bool shutdown;
   bool initialized;
   int fd;
   int pfd[2];
   CondVar init_lock;
   bool init();
  public:
   DebugPortReader();
   ~DebugPortReader();
   static void mainLoopWrapper(void *);
   void mainLoop();
};

#define CHECK_HELD_AEVS 'c'
class GeneratorBlueGene : public GeneratorMT
{
  private:
   DebugPortReader *dpr;
   int restart_fds[2];
   
   Mutex held_aes_lock;
   std::queue<ArchEventBlueGene *> held_aes;

  public:
   GeneratorBlueGene();
   virtual ~GeneratorBlueGene();

   virtual bool initialize();
   virtual bool canFastHandle();
   virtual ArchEvent *getEvent(bool block);
   virtual bool plat_skipGeneratorBlock();

   void addHeldArchEvent(ArchEventBlueGene *ae);
   ArchEventBlueGene *getHeldArchEvent();
};

class DecoderBlueGene : public Decoder
{
 public:
   DecoderBlueGene();
   virtual ~DecoderBlueGene();

   virtual bool getProcAndThread(ArchEventBlueGene *archbg, bg_process* &p, bg_thread* &t);
   virtual unsigned getPriority() const;
   virtual bool decode(ArchEvent *archE, std::vector<Event::ptr> &events);
   
   Event::ptr decodeGetRegAck(DebuggerInterface::BG_Debugger_Msg *msg, response::ptr & resp, bool &err);
   Event::ptr decodeGetAllRegAck(DebuggerInterface::BG_Debugger_Msg *msg, response::ptr & resp, bool &err);
   Event::ptr decodeGetMemAck(DebuggerInterface::BG_Debugger_Msg *msg, response::ptr & resp, bool &err);
   Event::ptr decodeResultAck(DebuggerInterface::BG_Debugger_Msg *msg, response::ptr & resp, bool &err);
   Event::ptr decodeAsyncAck(response::ptr resp);

   bool getPC(Address &addr, int_thread *thr, ArchEventBlueGene *cur_event);

   //Sometimes the decoder will generate an async event (e.g., reading PC values
   // for breakpoint decoding).  decodeDecoderAsync will handle upon recieving
   // the appropriate ACK event, and turn the ACK event into the original ArchEvent
   // that triggered the original decode.
   Event::ptr decodeDecoderAsync(response::ptr resp);
};


class HandleBGAttached : public Handler
{
  public:
   HandleBGAttached();
   ~HandleBGAttached();

   virtual void getEventTypesHandled(vector<EventType> &etypes);
   virtual handler_ret_t handleEvent(Event::ptr ev);
   virtual int getPriority() const;
};

struct auxv_element {
  uint32_t type;
  uint32_t value;
};

struct thrd_alive_ack_t {
   int lwp_id;
   bool alive;
};
#endif
