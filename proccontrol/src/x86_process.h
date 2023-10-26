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


#if !defined(x86_process_h_)
#define x86_process_h_

#include <map>
#include <set>
#include <stddef.h>
#include <string>
#include <vector>
#include "int_process.h"
#include "registers/MachRegister.h"

class x86_process : virtual public int_process
{
 public:
  x86_process(Dyninst::PID p, std::string e, std::vector<std::string> a, 
          std::vector<std::string> envp, std::map<int, int> f);
  x86_process(Dyninst::PID pid_, int_process *p) ;
  virtual ~x86_process();

  virtual unsigned plat_breakpointSize();
  virtual void plat_breakpointBytes(unsigned char *buffer);
  virtual bool plat_breakpointAdvancesPC() const;
  virtual Address plat_findFreeMemory(size_t) { return 0; }
};

class x86_thread : virtual public int_thread
{
  private:
   static const int max_dr_regs = 4;
   Dyninst::MachRegisterVal dr7_val;
   hw_breakpoint *active[max_dr_regs];
   unsigned int spaceNeeded(unsigned int perms);
   unsigned int spaceAvail();
   int getAvailDR();
   int getSlotForHBP(hw_breakpoint *, int last_slot);
   
  public:
   x86_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l);
   virtual ~x86_thread();
   virtual bool rmHWBreakpoint(hw_breakpoint *bp,
                               bool suspend,
                               std::set<response::ptr> &resps,
                               bool &done);
   virtual bool addHWBreakpoint(hw_breakpoint *bp,
                                bool resume,
                                std::set<response::ptr> &resps,
                                bool &done);
   virtual unsigned hwBPAvail(unsigned mode);

   virtual EventBreakpoint::ptr decodeHWBreakpoint(response::ptr &resp,
                                                   bool have_reg = false,
                                                   Dyninst::MachRegisterVal regval = 0);
   virtual bool bpNeedsClear(hw_breakpoint *hwbp);
};

#endif

