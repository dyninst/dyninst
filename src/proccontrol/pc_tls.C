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
#include "proccontrol_comp.h"
#include "communication.h"
#include "SymReader.h"

#include <map>

//NUM_ITERATIONS needs to match pc_tls_mutatee
#define NUM_ITERATIONS 8

using namespace std;

class pc_tlsMutator : public ProcControlMutator {
public:
  virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* pc_tls_factory()
{
   return new pc_tlsMutator();
}

static map<Thread::const_ptr, int> thread_iters;
static bool hasError = false;
static bool initialized_symbols;
static bool is_static;
static std::map<Process::const_ptr, Library::const_ptr> libtesta;
static std::map<Process::const_ptr, Library::const_ptr> executable;
static Dyninst::Offset lib_tls_read_int;
static Dyninst::Offset lib_tls_write_char;
static Dyninst::Offset lib_tls_read_long;
static Dyninst::Offset exe_tls_read_int;
static Dyninst::Offset exe_tls_write_char;
static Dyninst::Offset exe_tls_read_long;

static bool readSymbol(Process::const_ptr proc, Library::const_ptr lib, string symbolname, Dyninst::Offset &val)
{
   static SymbolReaderFactory *factory = NULL;
   if (!factory) {
      factory = proc->getSymbolReader();
   }
   
   SymReader *reader = factory->openSymbolReader(lib->getName());
   if (!reader) {
      logerror("Failed to open file %s\n", lib->getName().c_str());
      hasError = true;
      return false;
   }
    
   Symbol_t sym = reader->getSymbolByName(symbolname);
   if (!reader->isValidSymbol(sym)) {
      logerror("Couldn't find symbol %s in file %s\n", symbolname.c_str(), lib->getName().c_str());
      hasError = true;
      return false;
   }

   val = reader->getSymbolOffset(sym);
   return true;
}

static bool initSymbols(Process::const_ptr proc)
{
   Library::const_ptr lib, exe;
   for (LibraryPool::const_iterator i = proc->libraries().begin(); i != proc->libraries().end(); i++) {
      if ((*i)->getName().find("libtestA") != string::npos) {
         lib = *i;
         libtesta.insert(make_pair(proc, lib));
         break;
      }
   }
   exe = proc->libraries().getExecutable();
   executable.insert(make_pair(proc, exe));

   if (initialized_symbols)
      return true;
   initialized_symbols = true;

   is_static = !lib;
   bool result = true;
   if (!is_static) {
      result = readSymbol(proc, lib, "lib_tls_read_int", lib_tls_read_int);
      result &= readSymbol(proc, lib, "lib_tls_write_char", lib_tls_write_char);
      result &= readSymbol(proc, lib, "lib_tls_read_long", lib_tls_read_long);
   }
   result &= readSymbol(proc, exe, "tls_read_int", exe_tls_read_int);
   result &= readSymbol(proc, exe, "tls_write_char", exe_tls_write_char);
   result &= readSymbol(proc, exe, "tls_read_long", exe_tls_read_long);
   if (!result) {
      logerror("Failed to find a symbol\n");
      hasError = true;
      return false;
   }

   return true;
}

static Process::cb_ret_t on_breakpoint(Event::const_ptr ev)
{
   Process::const_ptr proc = ev->getProcess();
   Thread::const_ptr thread = ev->getThread();
   int iteration;

   map<Thread::const_ptr, int>::iterator i = thread_iters.find(thread);
   iteration = (i != thread_iters.end() ? i->second : 0);

   int int_val;
   signed long long_val;
   unsigned char char_val = 0x40 + ((unsigned char) iteration + 1);
   bool result;

   initSymbols(proc);

#define ERR_CHECK(NAME, VAL, EXPECTED)                                  \
   if (!result) { logerror("Couldn't read TLS variable " NAME "\n"); hasError = true; goto done; } \
   if ((long) VAL != (long) EXPECTED) { logerror("Unexpected value of " NAME " %ld != %ld\n", (long) VAL, (long) EXPECTED); hasError = true; goto done; }

   if (hasError) goto done;
   if (!is_static) {
      result = thread->readThreadLocalMemory(&int_val, libtesta[proc], lib_tls_read_int, sizeof(int));
      ERR_CHECK("lib_tls_read_int", int_val, iteration);
      result = thread->readThreadLocalMemory(&long_val, libtesta[proc], lib_tls_read_long, sizeof(long));
      ERR_CHECK("lib_tls_read_long", long_val, -1 * iteration);
      result = thread->writeThreadLocalMemory(libtesta[proc], lib_tls_write_char, &char_val, sizeof(char));
      ERR_CHECK("lib_tls_read_long", 0, 0);
   }
   result = thread->readThreadLocalMemory(&int_val, executable[proc], exe_tls_read_int, sizeof(int));
   ERR_CHECK("exe_tls_read_int", int_val, iteration);
   result = thread->readThreadLocalMemory(&long_val, executable[proc], exe_tls_read_long, sizeof(long));
   ERR_CHECK("exe_tls_read_long", long_val, -1 * iteration);
   result = thread->writeThreadLocalMemory(executable[proc], exe_tls_write_char, &char_val, sizeof(char));
   ERR_CHECK("exe_tls_read_long", 0, 0);
   
  done:
   thread_iters[thread] = iteration + 1;

   return Process::cbProcContinue;
}

static Process::cb_ret_t on_exit(Event::const_ptr)
{
   //Just need to know when processes exit, don't need to do any handling here.
   return Process::cbProcContinue;
}

test_results_t pc_tlsMutator::executeTest()
{
   thread_iters.clear();
   hasError = false;
   initialized_symbols = false;
   libtesta.clear();
   executable.clear();
   Breakpoint::ptr bp = Breakpoint::newBreakpoint();

   Process::registerEventCallback(EventType::Terminate, on_exit);
   Process::registerEventCallback(EventType::Breakpoint, on_breakpoint);

   std::vector<Process::ptr>::iterator i;
   for (i = comp->procs.begin(); i != comp->procs.end(); i++) {
      Process::ptr proc = *i;
      bool result = proc->continueProc();
      if (!result) {
         logerror("Failed to continue process\n");
         return FAILED;
      }
   }

   for (i = comp->procs.begin(); i != comp->procs.end(); i++) {
      Process::ptr proc = *i;
      send_addr addrmsg;
      bool result = comp->recv_message((unsigned char *) &addrmsg, sizeof(send_addr), proc);
      if (!result) {
         logerror("Failed to recieve address message from process\n");
         return FAILED;
      }
      if (addrmsg.code != SENDADDR_CODE) {
         logerror("Recieved unexpected message instead of address message\n");
         return FAILED;
      }

      result = proc->stopProc();
      if (!result) {
         logerror("Failed to stop process\n");
         return FAILED;
      }

      addrmsg.addr = comp->adjustFunctionEntryAddress(proc, addrmsg.addr);
      result = proc->addBreakpoint(addrmsg.addr, bp);
      if (!result) {
         logerror("Failed to add breakpoint\n");
         return FAILED;
      }
   }
 
   syncloc sync_point;
   sync_point.code = SYNCLOC_CODE;
   bool result = comp->send_broadcast((unsigned char *) &sync_point, sizeof(syncloc));
   if (!result) {
      logerror("Failed to send sync broadcast\n");
      return FAILED;
   }   
   for (i = comp->procs.begin(); i != comp->procs.end(); i++) {
      Process::ptr proc = *i;
      bool result = proc->continueProc();
      if (!result) {
         logerror("Failed to continue a process\n");
         return FAILED;
      }
   }


   bool all_done;
   for (;;) {
      all_done = true;
      for (i = comp->procs.begin(); i != comp->procs.end(); i++) {
         Process::ptr proc = *i;
         if (!proc->isTerminated()) {
            all_done = false;
            break;
         }
      }
      if (all_done)
         break;
      comp->block_for_events();
   }

   Process::removeEventCallback(on_breakpoint);
   Process::removeEventCallback(on_exit);

   for (map<Thread::const_ptr, int>::iterator i = thread_iters.begin(); i != thread_iters.end(); i++) {
      Thread::const_ptr thrd = i->first;
      int iterations = i->second;
      if (iterations != NUM_ITERATIONS) {
         logerror("Thread did not complete exepected number of iterations\n");
         return FAILED;
      }
   }

   thread_iters.clear();

   return hasError ? FAILED : PASSED;
}
