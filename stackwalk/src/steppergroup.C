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

#include "stackwalk/h/steppergroup.h"
#include "stackwalk/h/framestepper.h"
#include "stackwalk/h/swk_errors.h"
#include "stackwalk/src/sw.h"

using namespace Dyninst;
using namespace Dyninst::Stackwalker;
using namespace std;

StepperGroup::StepperGroup(Walker *new_walker) :
    walker(new_walker)
{
    assert(walker);
}

StepperGroup::~StepperGroup() 
{
}

Walker *StepperGroup::getWalker() const 
{
    assert(walker);
    return walker;
}

void StepperGroup::newLibraryNotification(LibAddrPair *libaddr,
                                          lib_change_t change)
{
   std::set<FrameStepper *>::iterator i = steppers.begin();
   for (; i != steppers.end(); i++)
   {
      (*i)->newLibraryNotification(libaddr, change);
   }
}

void StepperGroup::registerStepper(FrameStepper *stepper)
{
   steppers.insert(stepper);
   stepper->registerStepperGroup(this);
}

class Dyninst::Stackwalker::AddrRangeGroupImpl
{
public:
   addrRangeTree<addrRange> range_map;
};


AddrRangeGroup::AddrRangeGroup(Walker *new_walker) :
   StepperGroup(new_walker)
{
   sw_printf("[%s:%d] - Constructing new AddrRangeGroup at %p\n",
              FILE__, __LINE__, (void*)this);
   impl = new AddrRangeGroupImpl();
}

bool AddrRangeGroup::findStepperForAddr(Address addr, FrameStepper* &out, 
                                        const FrameStepper *last_tried)
{
   addrRange *range;
   sw_printf("[%s:%d] - AddrRangeGroup trying to find stepper at %lx " 
             "(last_tried = %s)\n", FILE__, __LINE__, addr, 
             last_tried ? last_tried->getName() : "<NONE>");

   bool result = impl->range_map.find(addr, range);
    if (!result) {
      sw_printf("[%s:%d] - Couldn't find a FrameStepper at %lx\n",
                 FILE__, __LINE__, addr);
      setLastError(err_nostepper, "No FrameStepper found at the given address");
        return false;
    }

   AddrRangeStepper *stepper_set = dynamic_cast<AddrRangeStepper*>(range);
   assert(stepper_set);

   if (!last_tried) {
      assert(stepper_set->steppers.size());
      StepperSet::iterator iter = stepper_set->steppers.begin();
      out = *(iter);
      sw_printf("[%s:%d] - Found FrameStepper %s at address %lx\n",
                FILE__, __LINE__, out->getName(), addr);
      return true;
    }

   FrameStepper *last_tried_nc = const_cast<FrameStepper *>(last_tried);
   StepperSet::iterator iter = stepper_set->steppers.find(last_tried_nc);
   if (iter == stepper_set->steppers.end()) {
      setLastError(err_badparam, "last_tried points to an invalid FrameStepper");
      return false;
   }

   iter++;
   if (iter == stepper_set->steppers.end()) {
      setLastError(err_nostepper, "No FrameStepper was able to walk through " 
                   "the given address");
      return false;
   }
    
   out = *iter;
   sw_printf("[%s:%d] - Found FrameStepper %s at address %lx\n",
             FILE__, __LINE__, out->getName(), addr);
   return true;
}

bool AddrRangeGroup::addStepper(FrameStepper *stepper, Address start, Address end)
{
   std::vector<addrRange *> ranges;
   bool result = impl->range_map.find(start, end, ranges);

   if (!stepper || end <= start)
   {
      sw_printf("[%s:%d] - addStepper called with bad params: %s, %lx, %lx\n",
                FILE__, __LINE__, stepper->getName(), start, end);
      setLastError(err_badparam, "Invalid parameters");
      return false;
   }

   steppers.insert(stepper);
   sw_printf("[%s:%d] - Adding stepper %s to address ranges %lx -> %lx\n",
             FILE__, __LINE__, stepper->getName(), start, end);
   if (!result) {
      //We don't have anything that overlaps.  Just add the stepper.
      AddrRangeStepper *new_range = new AddrRangeStepper(start, end);
      new_range->steppers.insert(stepper);
      impl->range_map.insert(new_range);
      return true;
   }
   assert(ranges.size());

   Address cur = start;
   unsigned i = 0;
   while (cur < end)
   {
      if (i >= ranges.size())
      {
         //We have an empty space in the address range starting at cur and
         //ending at end, fill it in with one range that fills the empty 
         //space and only contains the new stepper
         AddrRangeStepper *new_range = new AddrRangeStepper(cur, end);
         new_range->steppers.insert(stepper);
         impl->range_map.insert(new_range);
         cur = end;
         continue;         
      }
      if (cur < ranges[i]->get_address()) {
         //We have an empty space in the address range starting at cur and
         //ending at ranges[i]->get_address(), fill it in with one range 
         // that fills the empty space and only contains the new stepper
         AddrRangeStepper *new_range;
         new_range = new AddrRangeStepper(cur, ranges[i]->get_address());
         new_range->steppers.insert(stepper);
         impl->range_map.insert(new_range);
         cur = ranges[i]->get_address();
         continue;
      }
      if (cur > ranges[i]->get_address())
      {
         //Ranges[i] starts before cur.  We'll shorten ranges[i] to start and
         // add a new range with the same stepper set as ranges[i] to 
         // (start, ranges[i]->end].  This should only happen on the first time 
         // through this loop.
         assert(i == 0 && cur == start);
         AddrRangeStepper *old_range = dynamic_cast<AddrRangeStepper*>(ranges[i]);
         Address old_end = old_range->end;
         old_range->end = start;

         AddrRangeStepper *new_range = new AddrRangeStepper(start, old_end);
         new_range->steppers = old_range->steppers;
         impl->range_map.insert(new_range);
         
         //We'll cheat.  We should add the new_range into our ranges
         // vector and operate on it during the next loop iteration.
         // However, we're done with ranges[i], so let's just override
         // it with new_range and not bother incrementing i.
         ranges[i] = new_range;
         continue;
      }
      assert(cur == ranges[i]->get_address());
      if (ranges[i]->get_address() + ranges[i]->get_size() < end)
      {
         //Ranges[i] is contained within the total range.  No need
         // to add new things or resize, we'll just add stepper to its set.
         AddrRangeStepper *ars = dynamic_cast<AddrRangeStepper*>(ranges[i]);
         ars->steppers.insert(stepper);
         cur = ranges[i]->get_address() + ranges[i]->get_size();
         i++;
         continue;
      }

      if (ranges[i]->get_address() + ranges[i]->get_size() >= end)
      {
         //Ranges[i] overflows past end.  We'll break it into two parts: 
         // [ranges[i].start, end) and [end, ranges[i].end).  The first range
         // will get the stepper added to it.  This should only happen on the last case.
         assert(i == ranges.size() - 1);
         AddrRangeStepper *old_range = dynamic_cast<AddrRangeStepper*>(ranges[i]);
         Address old_end = old_range->end;
         old_range->end = end;
         
         AddrRangeStepper *new_range = new AddrRangeStepper(end, old_end);
         new_range->steppers = old_range->steppers;
         impl->range_map.insert(new_range);

         old_range->steppers.insert(stepper);
         cur = end;
         continue;
      }
      assert(0);
   }
   return true;
}

AddrRangeGroup::~AddrRangeGroup()
{
  delete impl;
}

void StepperGroup::getSteppers(std::set<FrameStepper *> &steps)
{
  steps = steppers;
}
