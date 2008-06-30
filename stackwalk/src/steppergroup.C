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

#include "steppergroup.h"
#include "framestepper.h"
//#include "addrRange.h"

using namespace Dyninst;
using namespace Dyninst::Stackwalker;

FrameStepper *StepperWrapper::getStepper() const {
    return stepper;
}

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

typedef struct {
  std::vector< std::pair<Address, Address> > ranges;
  AddrRangeStepper *stepper;
} AddrRangeStepperPair;

bool operator<(const std::pair<Address, Address> &a, 
	       const std::pair<Address, Address> &b)
{
  return a.first < a.second;
}

bool AddrRangeGroup::addStepper(FrameStepper *stepper) 
{
    assert(stepper);
    bool result;

    swk_printf("[%s:%u] - Adding stepper %p to group %p\n",
               __FILE__, __LINE__, stepper, this);

    AddrRangeStepperPair *new_pair = new AddrRangeStepperPair();
    assert(new_pair);

    result = stepper->getAddressRanges(new_pair->ranges);
    if (!result) {
        swk_printf("[%s:%u] - FrameStepper::getAddressRanges returned error\n",
                   __FILE__, __LINE__);
        return false;
    }

    std::sort(new_pair->ranges.begin(), new_pair->ranges.end());

    swk_printf("[%s:%u] - Stepper %p has %u ranges\n", 
               __FILE__, __LINE__, stepper, new_pair->ranges.size());
    for (unsigned i=0; i<new_pair->ranges.size(); i++) {
        swk_printf("[%s:%u] - Range %d is from 0x%lx to 0x%lx\n", __FILE__, 
		   __LINE__, i, new_pair->ranges[i].first, 
		   new_pair->ranges[i].second);
        if (ranges[i].second < ranges[i].first) {
            //TODO: Error return
        }

	/**
	 * Remove any overlapping ranges.
	 **/
	if (i+1 >= new_pair->ranges.size())
	  //Don't need to test last element
	  continue;
	if (ranges[i].second <= ranges[i+1].first) 
	  //Elements are not overlapping
	  continue;
	if (ranges[i].second >= ranges[i+1].second)
	  //

    }


    
}

bool StepperGroup::findStepperForAddr(Address addr, FrameStepper* &out, 
                                      const FrameStepper *last_tried)
{

}



