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

#include <assert.h>
#include <time.h>

#include "dynutil/h/dyn_regs.h"
#include "dynutil/h/dyntypes.h"
#include "symtabAPI/h/Symtab.h"
#include "common/h/pathName.h"
#include "proccontrol/h/PCErrors.h"
#include "proccontrol/h/Generator.h"
#include "proccontrol/h/Event.h"
#include "proccontrol/h/Handler.h"
#include "proccontrol/h/Mailbox.h"

#include "proccontrol/src/procpool.h"
#include "proccontrol/src/irpc.h"
#include "proccontrol/src/windows.h"
#include "proccontrol/src/int_handler.h"
#include "proccontrol/src/response.h"
#include "proccontrol/src/int_event.h"

#include "proccontrol/src/snippets.h"

#include "common/h/parseauxv.h"

using namespace Dyninst;
using namespace std;

static GeneratorWindows *gen = NULL;

Generator *Generator::getDefaultGenerator()
{
   if (!gen) {
      gen = new GeneratorWindows();
      assert(gen);
      gen->launch();
   }
   return static_cast<Generator *>(gen);
}

void Generator::stopDefaultGenerator()
{
    if(gen) delete gen;
}

bool GeneratorWindows::initialize()
{
   return true;
}

bool GeneratorWindows::canFastHandle()
{
   return false;
}

ArchEvent *GeneratorWindows::getEvent(bool block)
{
	assert(!"Not implemented");
	return NULL;
}

GeneratorWindows::GeneratorWindows() :
   GeneratorMT(std::string("Windows Generator"))
{
   decoders.insert(new DecoderWindows());
}

GeneratorWindows::~GeneratorWindows()
{
}

DecoderWindows::DecoderWindows()
{
}

DecoderWindows::~DecoderWindows()
{
}

unsigned DecoderWindows::getPriority() const 
{
   return Decoder::default_priority;
}

Dyninst::Address DecoderWindows::adjustTrapAddr(Dyninst::Address addr, Dyninst::Architecture arch)
{
  if (arch == Dyninst::Arch_x86 || arch == Dyninst::Arch_x86_64) {
    return addr-1;
  }
  return addr;
}

bool DecoderWindows::decode(ArchEvent *ae, std::vector<Event::ptr> &events)
{
	assert(!"Not implemented");
	return false;
}

int_process *int_process::createProcess(Dyninst::PID p, std::string e)
{
	assert(!"Not implemented");
	return NULL;
}

int_process *int_process::createProcess(std::string e, std::vector<std::string> a, std::vector<std::string> envp, 
        std::map<int,int> f)
{
	assert(!"Not implemented");
	return NULL;
}

int_process *int_process::createProcess(Dyninst::PID pid_, int_process *p)
{
	assert(!"Not implemented");
	return NULL;
}

Dyninst::Architecture windows_process::getTargetArch()
{
	// Fix this when we add 64-bit windows support...
	return Dyninst::Arch_x86;
}

windows_process::windows_process(Dyninst::PID p, std::string e, std::vector<std::string> a, 
                             std::vector<std::string> envp,  std::map<int,int> f) :
   int_process(p, e, a, envp, f),
   arch_process(p, e, a, envp, f)
{
}

windows_process::windows_process(Dyninst::PID pid_, int_process *p) :
   int_process(pid_, p),
   arch_process(pid_, p)
{
}

windows_process::~windows_process()
{
}

bool windows_process::plat_create()
{
	assert(!"Not implemented");
	return false;
}

bool windows_process::plat_create_int()
{
	assert(!"Not implemented");
	return false;
}

bool windows_process::plat_getOSRunningStates(std::map<Dyninst::LWP, bool> &runningStates) 
{
	assert(!"Not implemented");
	return false;
}

bool windows_process::plat_attach(bool)
{
	assert(!"Not implemented");
	return false;
}

bool windows_process::plat_attachWillTriggerStop() 
{
	assert(!"Not implemented");
	return false;
}

bool windows_process::plat_execed()
{
	assert(!"Not implemented");
	return false;
}

bool windows_process::plat_forked()
{
	assert(!"Not implemented");
	return false;
}

bool windows_process::plat_readMem(int_thread *thr, void *local, 
                                 Dyninst::Address remote, size_t size)
{
	assert(!"Not implemented");
	return false;
}

bool windows_process::plat_writeMem(int_thread *thr, const void *local, 
                                  Dyninst::Address remote, size_t size)
{
	assert(!"Not implemented");
	return false;
}


bool windows_process::needIndividualThreadAttach()
{
	assert(!"Not implemented");
	return false;
}

bool windows_process::plat_supportLWPEvents() const
{
	assert(!"Not implemented");
	return false;
}

bool windows_process::getThreadLWPs(std::vector<Dyninst::LWP> &lwps)
{
	assert(!"Not implemented");
	return false;
}

int_process::ThreadControlMode int_process::getThreadControlMode() 
{
    return int_process::IndependentLWPControl;
}

bool windows_thread::plat_cont()
{
	assert(!"Not implemented");
	return false;
}

SymbolReaderFactory *windows_process::plat_defaultSymReader()
{
	// Singleton this jive
	assert(!"Not implemented");
	return NULL;
}


int_thread *int_thread::createThreadPlat(int_process *proc, 
                                         Dyninst::THR_ID thr_id, 
                                         Dyninst::LWP lwp_id,
                                         bool initial_thrd)
{
	assert(!"Not implemented");
	return NULL;
}

windows_thread::windows_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l) :
   int_thread(p, t, l)
{
}

windows_thread::~windows_thread()
{
}

bool windows_thread::plat_stop()
{
	assert(!"Not implemented");
	return false;
}

void windows_thread::setOptions()
{
	assert(!"Not implemented");
}

bool windows_process::plat_individualRegAccess()
{
   return false;
}

bool windows_process::plat_detach()
{
	assert(!"Not implemented");
	return false;
}

bool windows_process::plat_terminate(bool &needs_sync)
{
	assert(!"Not implemented");
	return false;
}

Dyninst::Address windows_process::plat_mallocExecMemory(Dyninst::Address min, unsigned size) 
{
	assert(!"Not implemented");
	return 0;
}


bool windows_thread::plat_getAllRegisters(int_registerPool &regpool)
{
	assert(!"Not implemented");
	return false;
}

bool windows_thread::plat_getRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal &val)
{
	assert(!"Not implemented");
	return false;
}

bool windows_thread::plat_setAllRegisters(int_registerPool &regpool) 
{
	assert(!"Not implemented");
	return false;
}

bool windows_thread::plat_convertToSystemRegs(const int_registerPool &regpool, unsigned char *user_area) 
{
	assert(!"Not implemented");
	return false;
}

bool windows_thread::plat_setRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal val)
{
	assert(!"Not implemented");
	return false;
}

bool windows_thread::attach()
{
	assert(!"Not implemented");
	return false;
}

bool windows_thread::plat_getThreadArea(int val, Dyninst::Address &addr)
{
	assert(!"Not implemented");
	return false;
}

ArchEventWindows::ArchEventWindows(bool inter_)
{
}

ArchEventWindows::ArchEventWindows(pid_t p, int s)
{
}

ArchEventWindows::ArchEventWindows(int e)
{
}
      
ArchEventWindows::~ArchEventWindows()
{
}

std::vector<ArchEventWindows *> ArchEventWindows::pending_events;

bool ArchEventWindows::findPairedEvent(ArchEventWindows* &parent, ArchEventWindows* &child)
{
	assert(!"Not implemented");
	return false;
}

void ArchEventWindows::postponePairedEvent()
{
   pending_events.push_back(this);
}

WindowsHandleNewThr::WindowsHandleNewThr() :
   Handler("Windows New Thread")
{
}

WindowsHandleNewThr::~WindowsHandleNewThr()
{
}

Handler::handler_ret_t WindowsHandleNewThr::handleEvent(Event::ptr ev)
{
	assert(!"Not implemented");
	return ret_success;
}

int WindowsHandleNewThr::getPriority() const
{
   return PostPlatformPriority;
}

void WindowsHandleNewThr::getEventTypesHandled(std::vector<EventType> &etypes)
{
	assert(!"Not implemented");
}

WindowsHandleLWPDestroy::WindowsHandleLWPDestroy()
    : Handler("Windows LWP Destroy")
{
}

WindowsHandleLWPDestroy::~WindowsHandleLWPDestroy()
{
}

Handler::handler_ret_t WindowsHandleLWPDestroy::handleEvent(Event::ptr ev) 
{
	assert(!"Not implemented");
    return ret_success;
}

int WindowsHandleLWPDestroy::getPriority() const
{
    return PostPlatformPriority;
}

void WindowsHandleLWPDestroy::getEventTypesHandled(std::vector<EventType> &etypes)
{
    etypes.push_back(EventType(EventType::Pre, EventType::LWPDestroy));
}

HandlerPool *plat_createDefaultHandlerPool(HandlerPool *hpool)
{
	assert(!"Not implemented");
	return NULL;
}

bool ProcessPool::LWPIDsAreUnique()
{
	assert(!"Not implemented");
	return false;
}

void int_notify::readFromPipe()
{
   if (!pipesValid())
      return;

   char c;
   BOOL result;
   DWORD bytes_read;
   int error;
   do {
	   result = ::ReadFile(pipe_in, &c, 1, &bytes_read, NULL);
      error = errno;
   } while (result == TRUE);

   assert(result == TRUE && c == 'e');
   pthrd_printf("Cleared notification pipe %d\n", pipe_in);
}

bool int_notify::pipesValid()
{
	return (pipe_in != INVALID_HANDLE_VALUE) &&
		(pipe_out != INVALID_HANDLE_VALUE);
}

void int_notify::writeToPipe()
{
   if (!pipesValid()) 
      return;

   char c = 'e';
   DWORD bytes_written;
   BOOL result = ::WriteFile(pipe_out, &c, 1, &bytes_written, NULL);
   if (result == FALSE) {
      int error = errno;
      setLastError(err_internal, "Could not write to notification pipe\n");
      perr_printf("Error writing to notification pipe: %s\n", strerror(error));
      return;
   }
   pthrd_printf("Wrote to notification pipe %d\n", pipe_out);
}

bool int_notify::createPipe()
{
	::CreatePipe(&pipe_in, &pipe_out, NULL, 0);
	if(!pipesValid())
	{
		pthrd_printf("Error creating pipes\n");
		return false;
	}
	return true;
}

bool windows_process::plat_convertToBreakpointAddress(Dyninst::psaddr_t& addr)
{
	assert(!"Not implemented");
	return false;
}

bool windows_thread::plat_needsPCSaveBeforeSingleStep()
{
	assert(!"Not implemented");
	return false;
}

bool windows_thread::plat_needsEmulatedSingleStep(std::vector<Dyninst::Address> &result)
{
	assert(!"Not implemented");
	return false;
}

