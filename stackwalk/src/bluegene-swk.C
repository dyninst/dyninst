/*
 * Copyright (c) 1996-2007 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "stackwalk/h/swk_errors.h"
#include "stackwalk/h/symlookup.h"
#include "stackwalk/h/walker.h"
#include "stackwalk/h/framestepper.h"

#include "stackwalk/h/procstate.h"
#include "stackwalk/src/bluegene-swk.h"

#include "common/h/linuxKludges.h"

#include <string>

#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/poll.h>
#include <assert.h>

using namespace Dyninst;
using namespace Dyninst::Stackwalker;
using namespace DebuggerInterface;
using namespace std;

SymbolLookup *Walker::createDefaultSymLookup(const std::string &exec_name)
{
   SymbolLookup *new_lookup = new SwkDynSymtab(this, exec_name);
   return new_lookup;
}

bool SymbolLookup::lookupLibrary(Address, Address &load_addr,
                                 std::string &out_library)
{
   load_addr = 0;
   out_library = executable_path;
   return true;
}

bool ProcSelf::getThreadIds(std::vector<THR_ID> &threads)
{
  bool result;
  THR_ID tid;

  result = getDefaultThread(tid);
  if (!result) {
    sw_printf("[%s:%u] - Could not read default thread\n",
	       __FILE__, __LINE__);
    return false;
  }
  threads.clear();
  threads.push_back(tid);
  return true;
}

ProcSelf::ProcSelf()
{
  mypid = getpid();
}

bool ProcSelf::getDefaultThread(THR_ID &default_tid)
{
  default_tid = 0;
  return true;
}

static bool fdHasData(int fd)
{
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

#define SINGLE_STEP_SIG 32064

DebugEvent ProcDebug::debug_get_event(bool block)
{
   DebugEvent ev;
   pid_t pid;
   int sig_encountered;
   
   BGL_Debugger_Msg msg;


   if (!block && !fdHasData(BGL_DEBUGGER_READ_PIPE))
   {
      ev.dbg = dbg_noevent;
      return ev;
   }

   sw_printf("[%s:%u] - Waiting for debug event\n", __FILE__, __LINE__);
   bool result = BGL_Debugger_Msg::readFromFd(BGL_DEBUGGER_READ_PIPE, msg);
   if (!result) {
      sw_printf("[%s:%u] - Unable to wait for debug event on process\n",
                __FILE__, __LINE__);
      ev.dbg = dbg_err;
      return ev;
   }
   
   pid = msg.header.nodeNumber;   
   sw_printf("[%s:%u] - Recieved debug event %d on %d\n", __FILE__, __LINE__,
	     msg.header.messageType, pid);
   std::map<PID, ProcDebug *, procdebug_ltint>::iterator i = proc_map.find(pid);
   if (i == proc_map.end())
   {
      sw_printf("[%s:%u] - Error, recieved unknown pid %d\n",
                __FILE__, __LINE__, pid);
      setLastError(err_internal, "Error waiting for debug event");
      ev.dbg = dbg_err;
      return ev;
   }
   ev.proc = i->second;
   ProcDebugBG *procbg = dynamic_cast<ProcDebugBG *>(ev.proc);
   assert(procbg);

   switch (msg.header.messageType)
   {
      case PROGRAM_EXITED:
         ev.proc->isRunning = false;
         
         if (msg.dataArea.PROGRAM_EXITED.type == 0) {
            ev.dbg = dbg_exited;
         }
         else if (msg.dataArea.PROGRAM_EXITED.type == 1) {
            ev.dbg = dbg_crashed;
         }
         else {
            sw_printf("[%s:%u] - Unknown exit code (%d) on process %d\n",
                      __FILE__, __LINE__, msg.dataArea.PROGRAM_EXITED.type, pid);
            ev.dbg = dbg_err;
         }
         ev.data.idata = msg.dataArea.PROGRAM_EXITED.rc;
         sw_printf("[%s:%u] - Process %d %s with %d\n", 
                   __FILE__, __LINE__, pid, 
                   ev.dbg == dbg_exited ? "exited" : "crashed",
                   ev.data.idata);
         break;
      case SIGNAL_ENCOUNTERED:
         sw_printf("[%s:%u] - Process %d stopped with %d\n",
                   __FILE__, __LINE__, pid, msg.dataArea.SIGNAL_ENCOUNTERED.signal);
         ev.proc->isRunning = false;
         
         ev.dbg = dbg_stopped;
         sig_encountered = msg.dataArea.SIGNAL_ENCOUNTERED.signal;
         if (sig_encountered == SINGLE_STEP_SIG)
            sig_encountered = 0;
         ev.data.idata = msg.dataArea.SIGNAL_ENCOUNTERED.signal;
         break;
      case ATTACH_ACK:
         sw_printf("[%s:%u] - Process %d acknowledged attach\n",
                   __FILE__, __LINE__, pid);
         ev.dbg = dbg_attached;
         break;
      case CONTINUE_ACK:
         sw_printf("[%s:%u] - Process %d acknowledged continue\n", 
                   __FILE__, __LINE__, pid);
         ev.dbg = dbg_continued;
         break;
      case KILL_ACK:
         sw_printf("[%s:%u] - Process %d acknowledged kill\n", 
                   __FILE__, __LINE__, pid);
         ev.dbg = dbg_other;
         break;
      case GET_ALL_REGS_ACK:
         sw_printf("[%s:%u] - RegisterAll ACK on pid %d\n", 
                   __FILE__, __LINE__, pid);
         ev.dbg = dbg_allregs_ack;
         ev.size = msg.header.dataLength;
         procbg->gprs = msg.dataArea.GET_ALL_REGS_ACK.gprs;
         break;
      case GET_MEM_ACK:
         sw_printf("[%s:%u] - Memread ACK on pid %d\n", 
                   __FILE__, __LINE__, pid);
         ev.dbg = dbg_mem_ack;
         ev.size = msg.header.dataLength;
         ev.data.pdata = (void *) malloc(msg.header.dataLength);
         assert(ev.data.pdata);
         memcpy(ev.data.pdata, &msg.dataArea, msg.header.dataLength);
         break;
      case SINGLE_STEP_ACK:
         sw_printf("[%s:%u] - Process %d recieved SINGLE_STEP\n",
                   __FILE__, __LINE__, pid, ev.data);
         assert(ev.proc->isRunning);
         ev.proc->isRunning = false;
         
         ev.dbg = dbg_stopped;
         ev.data.idata = 0;
         break;
      default:
         sw_printf("[%s:%u] - Unknown debug message: %d\n",
                   __FILE__, __LINE__, msg.header.messageType);
         ev.dbg = dbg_noevent;
         break;
   }
   return ev;
}

bool ProcDebugBG::debug_handle_event(DebugEvent ev)
{
  bool result;

  switch (ev.dbg)
  {
     case dbg_stopped:
        isRunning = false;
        
        if (state == ps_attached_intermediate) {
           sw_printf("[%s:%u] - Moving %d to state running\n", 
                     __FILE__, __LINE__, pid);
           state = ps_attached;
           return true;
        }
        
        if (ev.data.idata == SINGLE_STEP_SIG) {
           if (user_isRunning) {
              ev.data.idata = 0;
              result = debug_handle_signal(&ev);
           }
        }

        if (ev.data.idata != SIGSTOP) {
           result = debug_handle_signal(&ev);
           if (!result) {
              sw_printf("[%s:%u] - Debug continue failed on %d with %d\n", 
                        __FILE__, __LINE__, pid, ev.data.idata);
              return false;
           }
        }
        return true;
    case dbg_crashed:
      sw_printf("[%s:%u] - Handling process crash on %d\n",
                __FILE__, __LINE__, pid);
    case dbg_exited:
      sw_printf("[%s:%u] - Handling process death on %d\n",
                __FILE__, __LINE__, pid);
      state = ps_exited;
      return true;
    case dbg_continued:
       sw_printf("[%s:%u] - Process %d continued\n", __FILE__, __LINE__, pid);
       clear_cache();
       assert(!isRunning);
       isRunning = true;
       break;
    case dbg_attached:
       sw_printf("[%s:%u] - Process %d attached\n", __FILE__, __LINE__, pid);
       assert(state == ps_neonatal);
       state = ps_attached_intermediate;
       debug_pause(); //TODO or not TODO?
       break;
    case dbg_mem_ack:
       sw_printf("[%s:%u] - Process %d returned a memory chunck of size %u", 
                 __FILE__, __LINE__, pid, ev.size);
       assert(!mem_data);
       mem_data = ev.data.pdata;
       mem_data_size = ev.size;
    case dbg_allregs_ack:
       sw_printf("[%s:%u] - Process %d returned a register chunck of size %u", 
                 __FILE__, __LINE__, pid, ev.size);
       gprs_set = true;
       break;
    case dbg_other:
       sw_printf("[%s:%u] - Skipping unimportant event\n", __FILE__, __LINE__);
       break;
    case dbg_err:
    case dbg_noevent:       
    default:
      sw_printf("[%s:%u] - Unexpectedly handling an error event %d on %d\n", 
                __FILE__, __LINE__, ev.dbg, pid);
      setLastError(err_internal, "Told to handle an unexpected event.");
      return false;
  }
  return true;
}

bool ProcDebugBG::debug_create(const std::string &, 
			       const std::vector<std::string> &)
{
  setLastError(err_unsupported, "Create mode not supported on BlueGene");
  return false;
}

bool ProcDebugBG::debug_attach()
{
   BGL_Debugger_Msg msg(ATTACH, pid, 0, 0, 0);
   msg.header.dataLength = sizeof(msg.dataArea.ATTACH);

   sw_printf("[%s:%u] - Attaching to pid\n", __FILE__, __LINE__, pid);
   bool result = BGL_Debugger_Msg::writeOnFd(BGL_DEBUGGER_WRITE_PIPE, msg);
   if (!result) {
      sw_printf("[%s:%u] - Unable to attach to process %d\n",
                __FILE__, __LINE__, pid);
      return false;
   }
   return true;
}

bool ProcDebugBG::debug_pause() 
{
   BGL_Debugger_Msg msg(KILL, pid, 0, 0, 0);
   msg.dataArea.KILL.signal = SIGSTOP;
   msg.header.dataLength = sizeof(msg.dataArea.KILL);

   sw_printf("[%s:%u] - Sending SIGSTOP to pid %d\n", __FILE__, __LINE__, pid);
   bool result = BGL_Debugger_Msg::writeOnFd(BGL_DEBUGGER_WRITE_PIPE, msg);

   if (!result) {
      sw_printf("[%s:%u] - Unable to send SIGSTOP to process %d\n",
                __FILE__, __LINE__, pid);
      return false;
   }
   return true;
}

bool ProcDebugBG::debug_continue()
{
   return debug_continue_with(0);
}

bool ProcDebugBG::debug_continue_with(long sig)
{
   assert(!isRunning);
   BGL_Debugger_Msg msg(CONTINUE, pid, 0, 0, 0);
   msg.dataArea.CONTINUE.signal = sig;
   msg.header.dataLength = sizeof(msg.dataArea.CONTINUE);


   sw_printf("[%s:%u] - Sending signal %d to pid %d\n", __FILE__, __LINE__, sig, pid);
   bool result = BGL_Debugger_Msg::writeOnFd(BGL_DEBUGGER_WRITE_PIPE, msg);
   if (!result) {
      sw_printf("[%s:%u] - Unable to send %d to process %d\n",
                __FILE__, __LINE__, sig, pid);
      return false;
   }
   return true;
}

bool ProcDebugBG::debug_handle_signal(DebugEvent *ev)
{
   assert(ev->dbg == dbg_stopped);
   sw_printf("[%s:%u] - Handling signal for pid %d\n", 
	     __FILE__, __LINE__, pid);
   return debug_continue_with(ev->data.idata);
}

#if !defined(BGL_READCACHE_SIZE)
#define BGL_READCACHE_SIZE 2048
#endif

bool ProcDebugBG::readMem(void *dest, Address source, size_t size)
{
   unsigned char *ucdest = (unsigned char *) dest;
   bool result;

   for (;;)
   {
      if (size == 0)
         return true;

      sw_printf("[%s:%u] - Reading memory from 0x%lx to 0x%lx (into %p)\n",
                __FILE__, __LINE__, source, source+size, ucdest);
      if (source >= read_cache_start && 
          source+size < read_cache_start + read_cache_size)
      {
         //Lucky us, we have everything cached.
         memcpy(ucdest, read_cache + (source - read_cache_start), size);
         return true;
      }
      if (source >= read_cache_start &&
          source < read_cache_start + read_cache_size)
      {
         //The beginning is in the cache, read those bytes
         long cached_bytes = read_cache_start + read_cache_size - source;
         memcpy(ucdest, read_cache + (source - read_cache_start), cached_bytes);
         //Change the parameters and continue the read (this is a kind of
         // optimized tail call).
         ucdest += cached_bytes;
         source = read_cache_start + read_cache_size;
         size -= cached_bytes;
         continue;
      }
      if (source+size > read_cache_start &&
          source+size <= read_cache_start + read_cache_size)
      {
         //The end is in the cache, read those bytes
         long cached_bytes = (source+size) - read_cache_start;
         memcpy(ucdest + read_cache_start - source, read_cache, cached_bytes);
         //Change the parameters and continue the read (this is a kind of
         // optimized tail call).
         size -= cached_bytes;
         continue;
      }
      if (source < read_cache_start &&
          source+size >= read_cache_start+read_cache_size)
      {
         //The middle is in the cache
         unsigned char *dest_start = ucdest + read_cache_start - source;
         Address old_read_cache_start = read_cache_start;
         memcpy(dest_start, read_cache, read_cache_size);
         //Use actual recursion here, as we need to queue up two reads
         // First read out of the high memory with recursion
         result = readMem(dest_start + read_cache_size, 
                          read_cache_start+read_cache_size,
                          source+size - read_cache_start+read_cache_size);
         if (!result) {
            return false;
         }
         // Now read out of the low memory
         size = old_read_cache_start - source;
         continue;
      }
      //The memory isn't cached, read a new cache'd page.
      assert(source < read_cache_start || 
             source >= read_cache_start+read_cache_size);
      
      if (!read_cache) {
         read_cache = (unsigned char *) malloc(BGL_READCACHE_SIZE);
         assert(read_cache);
      }

      read_cache_size = BGL_READCACHE_SIZE;
      read_cache_start = source - (source % read_cache_size);
      sw_printf("[%s:%u] - Caching memory from 0x%lx to 0x%lx\n",
             __FILE__, __LINE__, read_cache_start, 
             read_cache_start+read_cache_size);
                    
      //Read read_cache_start to read_cache_start+read_cache_size into our
      // cache.
      assert(!mem_data);
      assert(read_cache_size < BGL_Debugger_Msg_MAX_MEM_SIZE);

      BGL_Debugger_Msg msg(GET_MEM, pid, 0, 0, 0);
      msg.header.dataLength = sizeof(msg.dataArea.GET_MEM);
      msg.dataArea.GET_MEM.addr = read_cache_start;
      msg.dataArea.GET_MEM.len = read_cache_size;
      bool result = BGL_Debugger_Msg::writeOnFd(BGL_DEBUGGER_WRITE_PIPE, msg);
      if (!result) {
         sw_printf("[%s:%u] - Unable to write to process %d\n",
                   __FILE__, __LINE__, pid);
         return true;
      }
      
      //Wait for the read to return
      sw_printf("[%s:%u] - Waiting for read to return on pid %d\n",
                __FILE__, __LINE__);
      do {
         bool handled, result;
         result = debug_wait_and_handle(true, handled);
         if (!result)
         {
            sw_printf("[%s:%u] - Error while waiting for read to return\n",
                      __FILE__, __LINE__);
            return false;
         }
      } while (!mem_data);

      //TODO: How do we detect errors?
      BGL_Debugger_Msg::DataArea *da = (BGL_Debugger_Msg::DataArea *) mem_data;
      assert(da->GET_MEM_ACK.addr == read_cache_start);
      assert(da->GET_MEM_ACK.len == read_cache_size);
      memcpy(read_cache, da->GET_MEM_ACK.data, read_cache_size);
      
      //Free up the memory data
      free(mem_data);
      mem_data = NULL;
   }
}   

bool ProcDebugBG::getThreadIds(std::vector<THR_ID> &threads)
{
   threads.clear();
   threads.push_back((THR_ID) 0);
   return true;
}

bool ProcDebugBG::getDefaultThread(THR_ID &default_tid)
{
   default_tid = (THR_ID) 0;
   return true;
}

unsigned ProcDebugBG::getAddressWidth()
{
   return sizeof(long);
}

bool ProcDebugBG::getRegValue(reg_t reg, Dyninst::THR_ID, regval_t &val)
{

   if (!gprs_set)
   {
      BGL_Debugger_Msg msg(GET_ALL_REGS, pid, 0, 0, 0);
      msg.header.dataLength = sizeof(msg.dataArea.GET_ALL_REGS);
      bool result = BGL_Debugger_Msg::writeOnFd(BGL_DEBUGGER_WRITE_PIPE, msg);
      if (!result) {
         sw_printf("[%s:%u] - Unable to write to process %d\n",
                   __FILE__, __LINE__, pid);
         return true;
      }
      
      //Wait for the read to return
      sw_printf("[%s:%u] - Waiting for read to return on pid %d\n",
                __FILE__, __LINE__);
      do {
         bool handled, result;
         result = debug_wait_and_handle(true, handled);
         if (!result)
         {
            sw_printf("[%s:%u] - Error while waiting for read to return\n",
                      __FILE__, __LINE__);
            return false;
         }
      } while (!gprs_set);
   }
   
   switch(reg)
   {
      case REG_SP:
         val = 0x0;
	 break;
      case REG_FP:
         val = gprs.gpr[BGL_GPR1];
	 break;
      case REG_PC:
         val = gprs.iar;
	 break;
      default:
         sw_printf("[%s:%u] - Request for unsupported register %d\n",
                   __FILE__, __LINE__, reg);
         setLastError(err_badparam, "Unknown register passed in reg field");
         return false;
   }   
   return true;
}

ProcDebugBG::~ProcDebugBG()
{
}

ProcDebugBG::ProcDebugBG(PID pid)
   : ProcDebug(pid),
     mem_data(NULL),
     mem_data_size(0),
     gprs_set(false),
     read_cache(NULL),
     read_cache_start(0x0),
     read_cache_size(0x0)
{
}

ProcDebug *ProcDebug::newProcDebug(const std::string &, 
                                   const std::vector<std::string> &)
{
  setLastError(err_unsupported, "Executable launch not supported on BlueGene");
  return NULL;
}

ProcDebug *ProcDebug::newProcDebug(PID pid)
{
   ProcDebug *pd = new ProcDebugBG(pid);
   if (!pd)
   {
      sw_printf("[%s:%u] - Error creating new ProcDebug object\n",
                __FILE__, __LINE__);
      if (pd)
         delete pd;
      return NULL;
   }

   bool result = pd->attach();
   if (!result || pd->state != ps_running) {
     pd->state = ps_errorstate;
     proc_map.erase(pid);
     sw_printf("[%s:%u] - Error attaching to process %d\n",
               __FILE__, __LINE__, pid);
     delete pd;
     return NULL;
   }

   return pd;
}

bool ProcDebug::newProcDebugSet(const vector<Dyninst::PID> &pids,
                                vector<ProcDebug *> &out_set)
{
   bool result;
   vector<Dyninst::PID>::const_iterator i;
   for (i=pids.begin(); i!=pids.end(); i++)
   {
      Dyninst::PID pid = *i;
      ProcDebug *new_pd = new ProcDebugBG(pid);
      if (!new_pd) {
         fprintf(stderr, "[%s:%u] - Unable to allocate new ProcDebugBG\n",
                 __FILE__, __LINE__);
         return false;
      }
      out_set.push_back(new_pd);
   }

   result = multi_attach(out_set);
   return result;
}

bool Walker::createDefaultSteppers()
{
  FrameStepper *stepper;
  bool result;

  stepper = new FrameFuncStepper(this);
  result = addStepper(stepper);
  if (!result) {
    sw_printf("[%s:%u] - Error adding stepper %p\n", __FILE__, __LINE__,
              stepper);
    return false;
  }

  return true;
}

bool BGL_Debugger_Msg::writeOnFd(int fd, 
                                 DebuggerInterface::BGL_Debugger_Msg &msg)
{
   int result;
   result = write(fd, &msg.header, sizeof(msg.header));
   if (result != -1) {
      result = write(fd, &msg.dataArea, msg.header.dataLength);
   }

   if (result == -1) {
      int errnum = errno;
      sw_printf("[%s:%u] - Error writing to process: %s\n", 
                __FILE__, __LINE__, strerror(errnum));
      setLastError(err_internal, "Error writing message to process");
      return false;
   }
   
   return true;
}

bool BGL_Debugger_Msg::readFromFd(int fd, 
                                  DebuggerInterface::BGL_Debugger_Msg &msg)
{
   int result;
   result = read(fd, &msg.header, sizeof(msg.header));
   if (result != -1 && msg.header.dataLength) {
      result = read(fd, &msg.dataArea, msg.header.dataLength);
   }
   if (result == -1) {
      int errnum = errno;
      sw_printf("[%s:%u] - Error reading from process: %s\n", 
                __FILE__, __LINE__, strerror(errnum));
      setLastError(err_internal, "Error reading message from process");
      return false;
   }
   return true;
}

bool ProcSelf::readMem(void *dest, Address source, size_t size)
{
   memcpy(dest, (void *) source, size);
   return true;
}

int ProcDebug::getNotificationFD() {
  return BGL_DEBUGGER_READ_PIPE;
}

void ProcDebugBG::clear_cache()
{
  gprs_set = false;
  read_cache_start = 0x0;
  read_cache_size = 0x0;
  mem_data = NULL;
  mem_data_size = 0x0;
}

