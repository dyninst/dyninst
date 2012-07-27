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

// $Id: Process_data.C,v 1.1 2008/10/30 19:17:05 legendre Exp $

#if !defined(COMPLIB_DLL_BUILD)
#define COMPLIB_DLL_BUILD
#endif

#include "Process_data.h"

Process_data::Process_data(BPatch_thread *thread)
{
   bp_process = thread;
}

Process_data::Process_data()
{
   bp_process = NULL;
}

BPatch_thread *Process_data::getThread()
{
   return bp_process;
}

bool Process_data::terminate()
{
   /* alpha workaround - terminateExecution did not exit on a stopped process */
    if ( bp_process->getProcess()->isStopped() )
   {
       bp_process->getProcess()->continueExecution();
   }
   return bp_process->getProcess()->terminateExecution();
}

void ProcessList::insertThread(BPatch_thread *appThread)
{
   Process_data proc(appThread);

   push_back(proc);
}

void ProcessList::terminateAllThreads()
{
   while ( ! empty() )
   {
      Process_data ps = back();
      ps.terminate();
      pop_back();
   }
}
