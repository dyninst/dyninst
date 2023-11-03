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

#ifndef _BPatch_libInfo_h_
#define _BPatch_libInfo_h_

#include <sys/types.h>
#include "dyninstAPI/h/BPatch_process.h"
#include "dyninstAPI/h/BPatch_point.h"
#include <unordered_map>
#include "dyntypes.h"
#include "common/h/util.h"
#include "util.h"

class BPatch_libInfo {
public:
   std::unordered_map<int, BPatch_process *> procsByPid;
   BPatch_libInfo() : 
       stopThreadIDCounter_(0)
    {}

   bool registerMonitoredPoint(BPatch_point *point);
   BPatch_point *getMonitoredPoint(Dyninst::Address addr);

   int getStopThreadCallbackID(Dyninst::Address cb);

protected:
   int stopThreadIDCounter_;
   std::unordered_map<Dyninst::Address, unsigned> stopThreadCallbacks_;
   std::unordered_map<Dyninst::Address, BPatch_point *> monitoredPoints_;
};

#endif /* _BPatch_libInfo_h_ */
