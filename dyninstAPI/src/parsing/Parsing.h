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
#ifndef _PARSING_H_
#define _PARSING_H_

#include <assert.h>
#include <string>
#include <vector>
#include "parseAPI/h/CFGFactory.h"
#include "parseAPI/h/CodeSource.h"
#include "parseAPI/h/InstructionSource.h"
#include "parseAPI/h/CFG.h"
#include "parseAPI/h/ParseCallback.h"
#include "dyntypes.h"

// some useful types
using Dyninst::ParseAPI::EdgeTypeEnum;
using Dyninst::ParseAPI::FuncReturnStatus;
using Dyninst::ParseAPI::FuncSource;
using std::vector;

/*** The image_* object factory ***/
class image;

class image;
class DynParseCallback : public Dyninst::ParseAPI::ParseCallback {
 public:
  DynParseCallback(image * img) : Dyninst::ParseAPI::ParseCallback(), _img(img) { }
  ~DynParseCallback() { }

  protected:
  // defensive and exploratory mode callbacks
  virtual void abruptEnd_cf(Dyninst::Address,Dyninst::ParseAPI::Block *,default_details*);
  virtual void newfunction_retstatus(Dyninst::ParseAPI::Function*);
  virtual void patch_nop_jump(Dyninst::Address);
  virtual bool hasWeirdInsns(const Dyninst::ParseAPI::Function*) const;
  virtual void foundWeirdInsns(Dyninst::ParseAPI::Function*);

  // other callbacks
  virtual void interproc_cf(Dyninst::ParseAPI::Function*,Dyninst::ParseAPI::Block*,Dyninst::Address,interproc_details*);
  virtual void overlapping_blocks(Dyninst::ParseAPI::Block*,Dyninst::ParseAPI::Block*);
  virtual bool updateCodeBytes(Dyninst::Address target); // updates if needed
  virtual void split_block_cb(Dyninst::ParseAPI::Block *, Dyninst::ParseAPI::Block *); // needed for defensive mode

  virtual void destroy_cb(Dyninst::ParseAPI::Block *);
  virtual void destroy_cb(Dyninst::ParseAPI::Edge *);
  virtual void destroy_cb(Dyninst::ParseAPI::Function *);

  virtual void remove_edge_cb(Dyninst::ParseAPI::Block *, Dyninst::ParseAPI::Edge *, edge_type_t);
  virtual void remove_block_cb(Dyninst::ParseAPI::Function *, Dyninst::ParseAPI::Block *);

#if defined(DYNINST_HOST_ARCH_POWER) || defined(DYNINST_HOST_ARCH_AARCH64)
  void instruction_cb(Dyninst::ParseAPI::Function*, Dyninst::ParseAPI::Block *, Dyninst::Address, insn_details*);
#endif
 private:
    image * _img;
};


#endif
