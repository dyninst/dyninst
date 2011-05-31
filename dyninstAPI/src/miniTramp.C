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

// $Id: miniTramp.C,v 1.41 2008/02/20 22:34:20 legendre Exp $
// Code to install and remove instrumentation from a running process.

#include "miniTramp.h"
#include "baseTramp.h"
#include "instP.h"
#include "instPoint.h"
#include "process.h"
#include "ast.h"
#include "addressSpace.h"
#include "dyninstAPI/h/BPatch.h"

// for AIX
#include "function.h"

int miniTramp::_id = 1;

miniTramp::miniTramp(AstNodePtr ast, instPoint *point, bool recursive)
   : ast_(ast), point_(point), recursive_(recursive) {};

// Given a miniTramp parentMT, find the equivalent in the child
// process (matching by the ID member). Fill in childMT.
  
miniTramp *miniTramp::getInheritedMiniTramp(process *childProc) {
   instPoint *cPoint = instPoint::fork(point_, childProc);
   // Find the equivalent miniTramp...
   assert(point_->size() == cPoint->size());
/*
   instPoint::iterator c_iter = cPoint->begin();
   for (instPoint::iterator iter = point_->begin();
        iter != point_->end();
        ++iter) {
      if (*iter == this) return *c_iter;
      ++c_iter;
   }
*/
   instPoint::instance_iter c_iter = cPoint->begin();
   for (instPoint::instance_iter iter = point_->begin();
        iter != point_->end();
        ++iter) {
     miniTramp* mini = GET_MINI(*iter);
     if (mini == this) {
       miniTramp* c_mini = GET_MINI(*c_iter);
       return c_mini;
     }
     ++c_iter;
   }

   assert(0);
   return NULL;
}

bool miniTramp::uninstrument() {
   point_->erase(this);
   return true;
}
