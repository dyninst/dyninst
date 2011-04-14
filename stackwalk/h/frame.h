/*
 * Copyright (c) 1996-2011 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef FRAME_H_
#define FRAME_H_

#include "basetypes.h"
#include "Annotatable.h"
#include <string>

namespace Dyninst {
namespace Stackwalker {

class Walker;
class FrameStepper;

class Frame : public AnnotatableDense {
  friend class Walker;
protected:
  Dyninst::MachRegisterVal ra;
  Dyninst::MachRegisterVal fp;
  Dyninst::MachRegisterVal sp;
	
  location_t ra_loc;
  location_t fp_loc;
  location_t sp_loc;
  
  mutable std::string sym_name;
  mutable void *sym_value;
  mutable enum { nv_unset, nv_set, nv_err } name_val_set;
  
  bool bottom_frame;
  bool frame_complete;
  
  FrameStepper *stepper;
  Walker *walker;
  THR_ID originating_thread;
  
  void setStepper(FrameStepper *newstep);
  void setWalker(Walker *newwalk);
  void markBottomFrame();
  
  void setNameValue() const;
  
 public:
  Frame(Walker *walker);
  static Frame *newFrame(Dyninst::MachRegisterVal ra, Dyninst::MachRegisterVal sp, Dyninst::MachRegisterVal fp, Walker *walker);
  
  Dyninst::MachRegisterVal getRA() const;
  Dyninst::MachRegisterVal getSP() const;
  Dyninst::MachRegisterVal getFP() const;
  
  void setRA(Dyninst::MachRegisterVal);
  void setSP(Dyninst::MachRegisterVal);
  void setFP(Dyninst::MachRegisterVal);
  void setThread(THR_ID);
  
  location_t getRALocation() const;
  location_t getSPLocation() const;
  location_t getFPLocation() const;
  
  void setRALocation(location_t newval);
  void setSPLocation(location_t newval);
  void setFPLocation(location_t newval);
  
  bool getName(std::string &str) const;
  bool getObject(void* &obj) const;
  bool getLibOffset(std::string &lib, Dyninst::Offset &offset, void* &symtab);
  
  bool isBottomFrame() const;
  bool isFrameComplete() const;
  
  FrameStepper *getStepper() const;
  Walker *getWalker() const;
  THR_ID getThread() const;

  ~Frame();
};

}
}

#endif
