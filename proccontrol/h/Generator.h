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
#if !defined (GENERATOR_H_)
#define GENERATOR_H_

#include "dyntypes.h"
#include "Decoder.h"
#include "PCErrors.h"

#include "common/h/dthread.h"

#include <set>
#include <map>
#include <string>

struct GeneratorMTInternals;

namespace Dyninst {
namespace ProcControlAPI {

class Process;
class ArchEvent;
class Event;
class Mailbox;

class Generator
{
 public:
   static Generator *getDefaultGenerator();
   virtual bool getAndQueueEvent(bool block) = 0;
   virtual ~Generator();
   
   typedef void (*gen_cb_func_t)();
   static void registerNewEventCB(gen_cb_func_t func);
   static void removeNewEventCB(gen_cb_func_t);
 protected:

   //State tracking
   typedef enum {
      none,
      initializing,
      process_blocked,
      system_blocked,
      decoding,
      handling,
      queueing,
      error,
      exiting
   } state_t;
   state_t state;
   bool isExitingState();
   void setState(state_t newstate);

   //Event handling
   static std::map<Dyninst::PID, Process *> procs;
   typedef std::set<Decoder *, decoder_cmp> decoder_set_t;
   decoder_set_t decoders;

   //Misc
   bool hasLiveProc();
   std::string name;
   Generator(std::string name_);
   bool getAndQueueEventInt(bool block);

   static std::set<gen_cb_func_t> CBs;
   static Mutex *cb_lock;

   //Public interface
   //  Implemented by architectures
   virtual bool initialize() = 0;
   virtual bool canFastHandle() = 0;
   virtual ArchEvent *getEvent(bool block) = 0;
   //  Implemented by MT or ST
   virtual bool processWait(bool block) = 0;
};

class GeneratorMT : public Generator
{
 private:
   GeneratorMTInternals *sync;
   void main();

 public:
   void launch(); //Launch thread
   void start(); //Startup function for new thread
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
