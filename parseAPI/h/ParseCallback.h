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
#ifndef _PARSE_CALLBACK_H_
#define _PARSE_CALLBACK_H_

/** A ParseCallback allows extenders of this library to
    receive notifications during parsing of a variety of events.

    An implementer can choose to override one, several, or all of
    the methods in order to receive appropriate notification
    during parsing.
**/

#include "InstructionAdapter.h"
#include "dyntypes.h"

namespace Dyninst {
namespace ParseAPI {

class Function;

class ParseCallback {
 public:
  ParseCallback() { }
  virtual ~ParseCallback() { }

  /*
   * Notify when control transfers have run `off the rails' 
   */
  struct default_details {
    default_details(unsigned char*b,size_t s, bool ib) : ibuf(b), isize(s), isbranch(ib) { }
    unsigned char * ibuf;
    size_t isize;
    bool isbranch;
  };
  virtual void unresolved_cf(Function *,Address,default_details*) { }
  virtual void abruptEnd_cf(Address,default_details*) { }

  /*
   * Notify for interprocedural control transfers
   */
  struct interproc_details {
    typedef enum {
        ret,
        call,
        branch_interproc, // tail calls, branches to plts
        syscall
    } type_t;
    unsigned char * ibuf;
    size_t isize;
    type_t type;
    union { 
        struct {
            Address target;
            bool absolute_address;
            bool dynamic_call;
        } call;
    } data;
  };
  virtual void interproc_cf(Function*,Address,interproc_details*) { }

  /*
   * Allow examination of every instruction processed during parsing.
   */
  struct insn_details {
    InsnAdapter::InstructionAdapter * insn;
  };
  virtual void instruction_cb(Function*,Address,insn_details*) { }

  /* 
   * Notify about inconsistent parse data (overlapping blocks).
   * The blocks are *not* guaranteed to be finished parsing at
   * the time the callback is fired.
   */
  virtual void overlapping_blocks(Block*,Block*) { }

  /*
   * Defensive-mode notifications:
   * - Notify when a function's parse is finalized so Dyninst can 
       save its initial return status
   * - Notify every time a block is split, after the initial parse
   *   of the function
   * - Notify of the x86 obfuscation that performs a short jmp -1 (eb ff)
   *   so that dyninst can patch the opcode with a nop (0x90), which will
   *   keep code generation from doing bad things
   */
  virtual void newfunction_retstatus(Function*) { }
  virtual void block_split(Block *, Block *) { }
  virtual void patch_jump_neg1(Address) { }
};

}
}
#endif
