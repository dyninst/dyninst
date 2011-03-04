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
#ifndef _PARSING_H_
#define _PARSING_H_

#include "parseAPI/h/CFGFactory.h"
#include "parseAPI/h/CodeObject.h"
#include "parseAPI/h/CodeSource.h"
#include "parseAPI/h/InstructionSource.h"
#include "parseAPI/h/CFG.h"
#include "parseAPI/h/ParseCallback.h"

using namespace Dyninst;

// some useful types
using ParseAPI::EdgeTypeEnum;
using ParseAPI::FuncReturnStatus;
using ParseAPI::FuncSource;

/*** The image_* object factory ***/
class image;

class DynCFGFactory : public ParseAPI::CFGFactory {
  public:
    DynCFGFactory(image * im);
    ~DynCFGFactory();
    
    ParseAPI::Function * mkfunc(Address addr, FuncSource src, std::string name,
            ParseAPI::CodeObject * obj, ParseAPI::CodeRegion * reg,
            InstructionSource * isrc);
    ParseAPI::Block * mkblock(ParseAPI::Function * f, ParseAPI::CodeRegion * r,
            Address addr);
    ParseAPI::Edge * mkedge(ParseAPI::Block * src, ParseAPI::Block * trg, 
            EdgeTypeEnum type);

    ParseAPI::Block * mksink(ParseAPI::CodeObject *obj, ParseAPI::CodeRegion*r);

    // leaving default atm    
    //void free_func(ParseAPI::Function * f);
    //void free_block(ParseAPI::Block * b);
    //void free_edge(ParseAPI::Edge * e);

    //void free_all();
    void dump_stats();

  private:
    image * _img;     
    vector<int> _func_allocs;
    vector<int> _edge_allocs;
    int _block_allocs;
    int _sink_block_allocs;
    //int _sink_edge_allocs; FIXME can't determine

    void _record_func_alloc(ParseAPI::FuncSource fs)
    {
        assert(fs < ParseAPI::_funcsource_end_);
        ++_func_allocs[fs];
    }
    void _record_edge_alloc(ParseAPI::EdgeTypeEnum et,bool /* sink */)
    {
        assert(et < ParseAPI::_edgetype_end_);
        ++_edge_allocs[et];

        //if(sink)
            //++_sink_block_allocs;
    }
    void _record_block_alloc(bool sink)
    {
        ++_block_allocs;
        if(sink)
            ++_sink_block_allocs;
    }
};

class image;
class DynParseCallback : public ParseAPI::ParseCallback {
 public:
  DynParseCallback(image * img) : _img(img) { }
  ~DynParseCallback() { }

  // defensive and exploratory mode callbacks
  void abruptEnd_cf(Address,ParseAPI::Block *,default_details*);
  void newfunction_retstatus(ParseAPI::Function*);
  void block_split(ParseAPI::Block *first, ParseAPI::Block *second);
  void patch_nop_jump(Address);
  bool hasWeirdInsns(const ParseAPI::Function*) const;
  void foundWeirdInsns(ParseAPI::Function*);
  // other callbacks
  void interproc_cf(ParseAPI::Function*,ParseAPI::Block*,Address,interproc_details*);
  void overlapping_blocks(ParseAPI::Block*,ParseAPI::Block*);
  bool updateCodeBytes(Address target); // updates if needed
  bool absAddr(Address addr, Address &loadAddr, ParseAPI::CodeObject *&obj);
  void block_delete(ParseAPI::Block *b);

#if defined(arch_power) || defined(arch_sparc)
  void instruction_cb(ParseAPI::Function*,Address,insn_details*);
#endif
 private:
    image * _img;
};


#endif
