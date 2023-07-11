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

#if !defined(MMAPALLOC_H_)
#define MMAPALLOC_H_

#include <map>
#include <string>
#include <vector>
#include "int_process.h"

/**
 * A process that can use mmap to allocate memory.
 **/
class mmap_alloc_process : virtual public int_process
{
  public:
   mmap_alloc_process(Dyninst::PID p, std::string e, std::vector<std::string> a, 
           std::vector<std::string> envp, std::map<int,int> f);
   mmap_alloc_process(Dyninst::PID pid_, int_process *p);
   virtual ~mmap_alloc_process();

   virtual bool plat_collectAllocationResult(int_thread *thr, reg_response::ptr resp);
   virtual bool plat_createAllocationSnippet(Dyninst::Address addr, bool use_addr, unsigned long size, 
                                             void* &buffer, unsigned long &buffer_size, 
                                             unsigned long &start_offset);
   virtual bool plat_createDeallocationSnippet(Dyninst::Address addr, 
                                               unsigned long size, void* &buffer, 
                                               unsigned long &buffer_size, 
                                               unsigned long &start_offset);
};

#endif
