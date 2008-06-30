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

#ifndef BLUEGENE_SWK_H
#define BLUEGENE_SWK_H

#include "external/bluegene/debugger_interface.h"
using namespace DebuggerInterface;

namespace Dyninst {
namespace Stackwalker {

class ProcDebugBG : public ProcDebug {
  friend class ProcDebug;
 protected:
  virtual bool debug_create(const std::string &, 
			    const std::vector<std::string> &);
  virtual bool debug_attach();
  virtual bool debug_pause();
  virtual bool debug_continue();
  virtual bool debug_continue_with(long sig);
  virtual bool debug_handle_signal(DebugEvent *ev);

  virtual bool debug_handle_event(DebugEvent ev);

 public:
  ProcDebugBG(Dyninst::PID pid);
  virtual ~ProcDebugBG();

  virtual bool getRegValue(reg_t reg, Dyninst::THR_ID thread, regval_t &val);
  virtual bool readMem(void *dest, Dyninst::Address source, size_t size);
  virtual bool getThreadIds(std::vector<Dyninst::THR_ID> &threads);
  virtual bool getDefaultThread(Dyninst::THR_ID &default_tid);
  virtual unsigned getAddressWidth();

 protected:
  void *mem_data;
  unsigned mem_data_size;

  void clear_cache();

  bool gprs_set;
  BGL_GPRSet_t gprs;

  unsigned char *read_cache;
  Dyninst::Address read_cache_start;
  unsigned read_cache_size;
};

}
}

#endif
