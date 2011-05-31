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
  ParseCallback() : inBatch_(false) { }
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

  /*
   * Notify for interprocedural control transfers
   */
  struct interproc_details {
    typedef enum {
        ret,
        call,
        branch_interproc, // tail calls, branches to plts
        syscall,
        unres_call,
        unres_branch
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
        struct {
            Address target;
            bool absolute_address;
            bool dynamic;
        } unres;
    } data;
  };
  virtual void interproc_cf(Function*,Block *,Address,interproc_details*) { }

  /*
   * Allow examination of every instruction processed during parsing.
   */
  struct insn_details {
    InsnAdapter::InstructionAdapter * insn;
  };
  virtual void instruction_cb(Function*,Block *,Address,insn_details*) { }

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
  virtual void patch_nop_jump(Address) { }
  virtual bool updateCodeBytes(Address) { return false; }
  virtual void abruptEnd_cf(Address, Block *,default_details*) { }
  virtual bool loadAddr(Address, Address &) { return false; }
  virtual bool hasWeirdInsns(const Function*) const { return false; };
  virtual void foundWeirdInsns(Function*) {};


  /*
   * CFG change notifications, now with 50% more batch support!
   * These are intended for use by ParseAPI, not for user overrides.
   * User overrides are marked with a _cb suffix.
   */

  void batch_begin();
  // We need a CFGFactory object so that we can delete things; if we've
  // done a batch then we may have hung on to some objects that were
  // destroyed, and those can only be deleted with a CFGFactory object. 
  // Oy. 
  void batch_end(CFGFactory *fact);

  void destroy(Block *, CFGFactory *fact);
  void destroy(Edge *, CFGFactory *fact);
  void destroy(Function *, CFGFactory *fact);

  typedef enum {source, target } edge_type_t;
  
  void removeEdge(Block *, Edge *, edge_type_t);
  void addEdge(Block *, Edge *, edge_type_t);
  
  void removeBlock(Function *, Block *);
  void addBlock(Function *, Block *);
  
  void splitBlock(Block *, Block *);

  protected:
  // User override time
  // (orig, new split block)
  virtual void split_block_cb(Block *, Block *) {};

  virtual void destroy_cb(Block *) {};
  virtual void destroy_cb(Edge *) {};
  virtual void destroy_cb(Function *) {};

  virtual void remove_edge_cb(Block *, Edge *, edge_type_t) {};
  virtual void add_edge_cb(Block *, Edge *, edge_type_t) {};
  
  virtual void remove_block_cb(Function *, Block *) {};
  virtual void add_block_cb(Function *, Block *) {};

  private:
  bool inBatch_;

  typedef enum { removed, added } mod_t;

  struct BlockMod {
     Block *block;
     Edge *edge;
     edge_type_t type;
     mod_t action;
  BlockMod(Block *b, Edge *e, edge_type_t t, mod_t m) : block(b), edge(e), type(t), action(m) {};
  };     

  struct FuncMod {
     Function *func;
     Block *block;
     mod_t action;
  FuncMod(Function *f, Block *b, mod_t m) : func(f), block(b), action(m) {};
  };     

  typedef std::pair<Block *, Block *> BlockSplit;

  std::vector<Edge *> destroyedEdges_;
  std::vector<Block *> destroyedBlocks_;
  std::vector<Function *> destroyedFunctions_;
  std::vector<BlockMod> blockMods_;
  std::vector<FuncMod> funcMods_;
  std::vector<BlockSplit> blockSplits_;

};

}
}
#endif
