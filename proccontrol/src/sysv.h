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

#if !defined(SYSV_H_)
#define SYSV_H_

#include <map>
#include <set>
#include <string>
#include <vector>
#include "common/h/ProcReader.h"
#include "int_process.h"
#include "processplat.h"
#include "Handler.h"
#include "common/src/addrtranslate.h"

using namespace Dyninst;
using namespace ProcControlAPI;

class sysv_process;

class PCProcReader : public ProcessReader
{
   friend class sysv_process;
private:
   sysv_process *proc;
   
   std::set<mem_response::ptr> memresults;
public:
   PCProcReader(sysv_process *proc_);
   virtual ~PCProcReader();
   virtual bool start();
   virtual bool ReadMem(Address addr, void *buffer, unsigned size);
   virtual bool GetReg(MachRegister reg, MachRegisterVal &val);
   virtual bool done();

   bool hasPendingAsync();
   bool getNewAsyncs(std::set<response::ptr> &resps);
};

class sysv_process : public int_libraryTracking
{
   friend class PCProcReader;
 public:
   sysv_process(Dyninst::PID p, std::string e, std::vector<std::string> a, 
           std::vector<std::string> envp, std::map<int,int> f);
   sysv_process(Dyninst::PID pid_, int_process *p);
   virtual ~sysv_process();
   virtual bool refresh_libraries(std::set<int_library *> &added_libs,
                                  std::set<int_library *> &rmd_libs,
                                  bool &waiting_for_async,
                                  std::set<response::ptr> &async_responses);
   virtual bool initLibraryMechanism();

   Dyninst::Address getLibBreakpointAddr() const;

   bool isLibraryTrap(Dyninst::Address trap_addr);
   static bool addSysVHandlers(HandlerPool *hpool);

   virtual bool setTrackLibraries(bool b, int_breakpoint* &bp, Address &addr, bool &add_bp);
   virtual bool isTrackingLibraries();
 protected:
   virtual bool plat_execed();
   virtual bool plat_isStaticBinary();
   virtual int_library *plat_getExecutable();
   virtual bool plat_getInterpreterBase(Address &addr);

   AddressTranslate *constructTranslator(Dyninst::PID pid);
   AddressTranslate *translator();

   static int_breakpoint *lib_trap;

   Address breakpoint_addr;
   bool lib_initialized;
   PCProcReader *procreader;
  private:
   
   bool track_libraries;
   int_library *aout;
   static SymbolReaderFactory *symreader_factory;

   void createAddrTranslator();
   void deleteAddrTranslator();
   AddressTranslate *translator_;

   typedef enum {
     NotReady,
     Ready,
     Creating,
     Created } translator_state_t;

   translator_state_t translator_state;
};

#endif
