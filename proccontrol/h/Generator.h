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
#if !defined (GENERATOR_H_)
#define GENERATOR_H_

#include "dyntypes.h"
#include "Decoder.h"
#include "PCErrors.h"

#include "common/src/dthread.h"
#include "util.h"

#include <set>
#include <map>
#include <string>
#include <vector>

struct GeneratorMTInternals;
class int_process;

namespace Dyninst {
namespace ProcControlAPI {

class Process;
class ArchEvent;
class Event;
class Mailbox;

class PC_EXPORT Generator
{
 public:
   static Generator *getDefaultGenerator();
   static void stopDefaultGenerator();
   static bool startedAnyGenerator;
   virtual bool getAndQueueEvent(bool block) = 0;
   virtual ~Generator();
   
   typedef void (*gen_cb_func_t)();
   static void registerNewEventCB(gen_cb_func_t func);
   static void removeNewEventCB(gen_cb_func_t);
   
   void forceEventBlock();

   //State tracking
   typedef enum {
      none,
      initializing,
      process_blocked,
      system_blocked,
      decoding,
      statesync,
      handling,
      queueing,
      error,
      exiting
   } state_t;
   state_t state;
   static const char* generatorStateStr(state_t);
   virtual bool isExitingState();
   virtual void setState(state_t newstate);
   virtual state_t getState();

 protected:
   //Event handling
   static std::map<Dyninst::PID, Process *> procs;
   typedef std::set<Decoder *, decoder_cmp> decoder_set_t;
   decoder_set_t decoders;

   ArchEvent* m_Event;
   virtual ArchEvent* getCachedEvent();
   virtual void setCachedEvent(ArchEvent* ae);

   //Misc
   virtual bool hasLiveProc();
   std::string name;
   Generator(std::string name_);
   bool getAndQueueEventInt(bool block);
   static bool allStopped(int_process *proc, void *);

   static std::set<gen_cb_func_t> CBs;
   static Mutex<> *cb_lock;

   //Public interface
   //  Implemented by architectures
   virtual bool initialize() = 0;
   virtual bool canFastHandle() = 0;
   virtual ArchEvent *getEvent(bool block) = 0;
   virtual bool plat_skipGeneratorBlock();
   //  Implemented by MT or ST
   virtual bool processWait(bool block) = 0;

   virtual bool plat_continue(ArchEvent* /*evt*/) { return true; }
   virtual void wake(Dyninst::PID/* proc */, long long /* sequence */) {}

   //  Optional interface for systems that want to return multiple events
   virtual bool getMultiEvent(bool block, std::vector<ArchEvent *> &events);

  private:
   bool eventBlock_;
};

class PC_EXPORT GeneratorMT : public Generator
{
 private:
   GeneratorMTInternals *sync;
   void main();
protected:
	void lock();
	void unlock();

 public:
   void launch(); //Launch thread
   void start(); //Startup function for new thread
   virtual void plat_start() {}
   virtual bool plat_continue(ArchEvent* /*evt*/) { return true;}
   GeneratorMTInternals *getInternals();

   GeneratorMT(std::string name_);
   virtual ~GeneratorMT();
   virtual bool processWait(bool block);
   virtual bool getAndQueueEvent(bool block);
};

class GeneratorST : public Generator
{
 public:
   GeneratorST(std::string name_);
   virtual ~GeneratorST();
   
   virtual bool processWait(bool block);
   virtual bool getAndQueueEvent(bool block);
};

}
}

#endif
