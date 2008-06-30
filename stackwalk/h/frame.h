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

#ifndef FRAME_H_
#define FRAME_H_

#include "basetypes.h"
#include <string>

namespace Dyninst {
namespace Stackwalker {

class Walker;
class FrameStepper;

class Frame {
  friend class Walker;
protected:
  regval_t ra;
  regval_t fp;
  regval_t sp;
	
  location_t ra_loc;
  location_t fp_loc;
  location_t sp_loc;
  
  std::string sym_name;
  void *sym_value;
  
  enum { nv_unset, nv_set, nv_err } name_val_set;
  
  bool bottom_frame;
  bool frame_complete;
  
  FrameStepper *stepper;
  Walker *walker;
  
  void setStepper(FrameStepper *newstep);
  void setWalker(Walker *newwalk);
  void markBottomFrame();
  
  void setNameValue();
  
 public:
  Frame(Walker *walker);
  static Frame *newFrame(regval_t ra, regval_t sp, regval_t fp, Walker *walker);
  
  regval_t getRA() const;
  regval_t getSP() const;
  regval_t getFP() const;
  
  void setRA(regval_t);
  void setSP(regval_t);
  void setFP(regval_t);
  
  location_t getRALocation() const;
  location_t getSPLocation() const;
  location_t getFPLocation() const;
  
  void setRALocation(location_t newval);
  void setSPLocation(location_t newval);
  void setFPLocation(location_t newval);
  
  bool getName(std::string &str);
  bool getObject(void* &obj);
  
  bool isBottomFrame() const;
  bool isFrameComplete() const;
  
  FrameStepper *getStepper() const;
  Walker *getWalker() const;
  
  virtual ~Frame();
};

}
}

#endif
