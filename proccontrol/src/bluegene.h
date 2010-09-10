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

class bg_process : public sysv_process, public ppc_process
{
  public:
   bg_process(Dyninst::PID p, std::string e, std::vector<std::string> a, std::map<int, int> f);
   bg_process(Dyninst::PID pid_, int_process *proc_);

   virtual ~bg_process();

   virtual bool plat_create();
   virtual bool plat_create_int();
   virtual bool plat_attach();   
   virtual bool plat_forked();
   virtual bool post_forked();
   virtual bool plat_execed();
   virtual bool plat_detach();
   virtual bool plat_terminate(bool &needs_sync);

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
};

class ArchEventBlueGene : public ArchEvent
{
  private:
   BG_Debugger_Msg *msg;
  public:
   ArchEventBlueGene(BG_Debugger_Msg *m);
   virtual ~ArchEventBlueGene();
   BG_Debugger_Msg *getMsg() const;
};

class GeneratorBlueGene : public GeneratorMT
{
  public:
   GeneratorBlueGene();
   virtual ~GeneratorBlueGene();

   virtual bool initialize();
   virtual bool canFastHandle();
   virtual ArchEvent *getEvent(bool block);
};

class DecoderBlueGene : public Decoder
{
 public:
   DecoderBlueGene();
   virtual ~DecoderBlueGene();

   virtual bool getProcAndThread(bg_process* &p, bg_thread* &t) = 0;
   virtual unsigned getPriority() const;
   virtual bool decode(ArchEvent *archE, std::vector<Event::ptr> &events);

   Event::ptr decodeGetRegAck(BG_Debugger_Msg *msg);
   Event::ptr decodeGetAllRegAck(BG_Debugger_Msg *msg);
   Event::ptr decodeGetMemAck(BG_Debugger_Msg *msg);
   Event::ptr decodeResultAck(BG_Debugger_Msg *msg);
   

   Event::ptr decodeAsyncAck(response::ptr resp);
};

#endif
