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

#if !defined(ppc_process_h_)
#define ppc_process_h_

#include <set>
#include <string>
#include <vector>
#include <map>
#include "int_process.h"

class ppc_process : virtual public int_process
{
 public:
  ppc_process(Dyninst::PID p, std::string e, std::vector<std::string> a, 
              std::vector<std::string> envp, std::map<int, int> f);
  ppc_process(Dyninst::PID pid_, int_process *p) ;
  virtual ~ppc_process();

  virtual unsigned plat_breakpointSize();
  virtual void plat_breakpointBytes(unsigned char *buffer);
  virtual bool plat_breakpointAdvancesPC() const;

  virtual async_ret_t plat_needsEmulatedSingleStep(int_thread *thr, std::vector<Address> &addrResult);
  virtual bool plat_convertToBreakpointAddress(Address &addr, int_thread *thr);
  virtual bool plat_needsPCSaveBeforeSingleStep();
  virtual void plat_getEmulatedSingleStepAsyncs(int_thread *thr, std::set<response::ptr> resps);


  void cleanupSSOnContinue(int_thread *thr);
 private:
  async_ret_t readPCForSS(int_thread *thr, Address &pc);
  async_ret_t readInsnForSS(Address pc, int_thread *thr, unsigned int &rawInsn);
  void registerSSClearCB();

  std::map<int_thread *, reg_response::ptr> pcs_for_ss;
  std::map<Address, mem_response::ptr> mem_for_ss;
};

class ppc_thread : virtual public int_thread
{
  protected:
   bool have_cached_pc;
   Address cached_pc;
  public:
   ppc_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l);
   virtual ~ppc_thread();

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

   void setCachedPC(Address pc);
   void clearCachedPC();
   bool haveCachedPC(Address &pc);
};

#endif

