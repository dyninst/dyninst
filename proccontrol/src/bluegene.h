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
#include "proccontrol/src/int_process.h"
#include "proccontrol/src/sysv.h"
#include "proccontrol/src/ppc_process.h"
#include "proccontrol/src/procpool.h"

#if defined(os_bgl)
#include "external/bluegene/bgl-debugger-interface.h"
#elif defined (os_bgp)
#include "external/bluegene/bgp-debugger-interface.h"
#else
#error "ERROR: No suitable debug interface for this BG ION."
#endif

class bg_process : public sysv_process, public ppc_process
{
   friend class HandleBGAttached;
   friend class DecoderBlueGene;
  private:
   enum {
      bg_init,
      bg_stop_pending,
      bg_stopped,
      bg_thread_pending,
      bg_ready,
      bg_bootstrapped,
      bg_done
   } bootstrap_state;

   bool has_procdata;
   DebuggerInterface::BG_Process_Data_t procdata;
   
   signed int pending_thread_alives;
   std::set<int> initial_lwps;
  public:
   static int protocol_version;
   static int phys_procs;
   static int virt_procs;

   bg_process(Dyninst::PID p, std::string e, std::vector<std::string> a, std::map<int, int> f);
   bg_process(Dyninst::PID pid_, int_process *proc_);

   virtual ~bg_process();

   virtual bool plat_create();
   virtual bool plat_create_int();
   virtual bool plat_attach();   
   virtual bool plat_forked();
   virtual bool post_forked();
   virtual bool plat_detach(bool &needs_sync);
   virtual bool plat_terminate(bool &needs_sync);

   virtual bool plat_needsAsyncIO() const;
   virtual bool plat_readMemAsync(int_thread *, Dyninst::Address addr, 
                                  mem_response::ptr result);
   virtual bool plat_writeMemAsync(int_thread *thr, void *local, Dyninst::Address addr,
                                   size_t size, result_response::ptr result);
   virtual bool plat_readMem(int_thread *thr, void *local, 
                             Dyninst::Address remote, size_t size);
   virtual bool plat_writeMem(int_thread *thr, void *local, 
                              Dyninst::Address remote, size_t size);

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

   int_process::ThreadControlMode plat_getThreadControlMode() const;
   virtual SymbolReaderFactory *plat_defaultSymReader();
   unsigned plat_getRecommendedReadSize();

   static void getVersionInfo(int &protocol, int &phys, int &virt);
};

class bg_thread : public int_thread
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
   virtual bool attach();   

   virtual bool plat_suspend() { return true; }
   virtual bool plat_resume() { return true; }
};

class ArchEventBlueGene : public ArchEvent
{
  private:
   DebuggerInterface::BG_Debugger_Msg *msg;
  public:
   ArchEventBlueGene(DebuggerInterface::BG_Debugger_Msg *m);
   virtual ~ArchEventBlueGene();
   DebuggerInterface::BG_Debugger_Msg *getMsg() const;
};

class DebugPortReader
{
  private:
   static const int port_num = 7201;
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

class GeneratorBlueGene : public GeneratorMT
{
  private:
   DebugPortReader *dpr;
  public:
   GeneratorBlueGene();
   virtual ~GeneratorBlueGene();

   virtual bool initialize();
   virtual bool canFastHandle();
   virtual ArchEvent *getEvent(bool block);
   virtual bool plat_skipGeneratorBlock();
};

class DecoderBlueGene : public Decoder
{
 public:
   DecoderBlueGene();
   virtual ~DecoderBlueGene();

   virtual bool getProcAndThread(ArchEventBlueGene *archbg, bg_process* &p, bg_thread* &t);
   virtual unsigned getPriority() const;
   virtual bool decode(ArchEvent *archE, std::vector<Event::ptr> &events);
   
   Event::ptr decodeGetRegAck(DebuggerInterface::BG_Debugger_Msg *msg);
   Event::ptr decodeGetAllRegAck(DebuggerInterface::BG_Debugger_Msg *msg);
   Event::ptr decodeGetMemAck(DebuggerInterface::BG_Debugger_Msg *msg);
   Event::ptr decodeResultAck(DebuggerInterface::BG_Debugger_Msg *msg);
   

   Event::ptr decodeAsyncAck(response::ptr resp);
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

struct thrd_alive_ack_t {
   int lwp_id;
   bool alive;
};
#endif
