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

#if !defined (_R_E_PADDING_H_)
#define _R_E_PADDING_H_

#include "Atom.h"

namespace Dyninst {
namespace Relocation {

class Padding : public Atom {
 public:
  typedef dyn_detail::boost::shared_ptr<Padding> Ptr;

  virtual bool generate(GenStack &);

  virtual TrackerElement *tracker() const;

  static Ptr create(Address addr, unsigned size);

  virtual ~Padding() {};

  virtual std::string format() const;

  virtual Address addr() const { return addr_; }
  virtual unsigned size() const { return size_; }

 private:
  Padding(Address addr, unsigned size) : 
  addr_(addr), size_(size) {};

  // Our address
  Address addr_;

  // size
  unsigned size_;
};

};
};

#endif
