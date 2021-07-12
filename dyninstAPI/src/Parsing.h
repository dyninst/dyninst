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

#include "parseAPI/h/CFGFactory.h"
#include "parseAPI/h/CodeSource.h"
#include "parseAPI/h/InstructionSource.h"
#include "parseAPI/h/CFG.h"
#include "parseAPI/h/ParseCallback.h"

// some useful types
using ParseAPI::EdgeTypeEnum;
using ParseAPI::FuncReturnStatus;
using ParseAPI::FuncSource;
using std::vector;

/*** The image_* object factory ***/
class image;

namespace Dyninst {
namespace ParseAPI {
   class CodeObject;
}
}

class DynCFGFactory : public Dyninst::ParseAPI::CFGFactory {
  public:
    DynCFGFactory(image * im);
    ~DynCFGFactory() {}
    
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
    boost::mutex _mtx;
    image * _img;     
    std::vector<int> _func_allocs;
    std::vector<int> _edge_allocs;
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
  DynParseCallback(image * img) : ParseAPI::ParseCallback(), _img(img) { }
  ~DynParseCallback() { }

  protected:
  // defensive and exploratory mode callbacks
  virtual void abruptEnd_cf(Address,ParseAPI::Block *,default_details*);
  virtual void newfunction_retstatus(ParseAPI::Function*);
  virtual void patch_nop_jump(Address);
  virtual bool hasWeirdInsns(const ParseAPI::Function*) const;
  virtual void foundWeirdInsns(ParseAPI::Function*);

  // other callbacks
  virtual void interproc_cf(ParseAPI::Function*,ParseAPI::Block*,Address,interproc_details*);
  virtual void overlapping_blocks(ParseAPI::Block*,ParseAPI::Block*);
  virtual bool updateCodeBytes(Address target); // updates if needed
  virtual void split_block_cb(ParseAPI::Block *, ParseAPI::Block *); // needed for defensive mode

  virtual void destroy_cb(ParseAPI::Block *);
  virtual void destroy_cb(ParseAPI::Edge *);
  virtual void destroy_cb(ParseAPI::Function *);

  virtual void remove_edge_cb(ParseAPI::Block *, ParseAPI::Edge *, edge_type_t);
  virtual void remove_block_cb(ParseAPI::Function *, ParseAPI::Block *);

#if defined(arch_power) || defined(arch_aarch64)
  void instruction_cb(ParseAPI::Function*, ParseAPI::Block *, Address, insn_details*);
#endif
 private:
    image * _img;
};


#endif
