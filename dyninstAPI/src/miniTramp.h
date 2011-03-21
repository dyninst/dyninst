/*
 * Copyright (c) 1996-2009 Barton P. Miller
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

// $Id: miniTramp.h,v 1.19 2008/01/16 22:01:37 legendre Exp $

#ifndef MINI_TRAMP_H
#define MINI_TRAMP_H

#include "common/h/Types.h"
#include "dyninstAPI/src/codeRange.h"
#include "dyninstAPI/src/inst.h" // callOrder and callWhen
#include "dyninstAPI/src/ast.h"

// This is a serious problem: our code generation has no way to check
// the size of the buffer it's emitting to.
// 1 megabyte buffer
#define MAX_MINITRAMP_SIZE (0x100000)

// Callback func for deletion of a minitramp
class miniTramp;
class AstNode;
class AstMiniTrampNode;
class AddressSpace;

//typedef void (*miniTrampFreeCallback)(void *, miniTramp *);

class miniTramp {
  friend class instPoint;
    // Global numbering of minitramps
  static int _id;

  miniTramp() {};

  public:

  miniTramp(AstNodePtr ast, instPoint *point);
  
  ~miniTramp() {};

  bool uninstrument();

  // Given a child address space, get the corresponding miniTramp to us.
  miniTramp *getInheritedMiniTramp(process *child);

  instPoint *instP() const { return point_; }

  AstNodePtr ast() const { return ast_; }

  private:
  AstNodePtr ast_;
  instPoint *point_;


};


#endif
