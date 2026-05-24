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

#ifndef DYNINST_DYNINSTAPI_PARSING_DYNCFGFACTORY_H
#define DYNINST_DYNINSTAPI_PARSING_DYNCFGFACTORY_H

#include "CFG.h"
#include "CFGFactory.h"

#include <boost/thread/mutex.hpp>
#include <string>

namespace Dyninst { namespace ParseAPI {
  class CodeObject;
}}

class image;

namespace Dyninst { namespace DyninstAPI {

  class DynCFGFactory : public ParseAPI::CFGFactory {
  public:
    DynCFGFactory(image *im);

    ~DynCFGFactory() {
    }

    ParseAPI::Block *mkblock(ParseAPI::Function *f, ParseAPI::CodeRegion *r,
                             Dyninst::Address addr) override;

    ParseAPI::Edge *mkedge(ParseAPI::Block *src, ParseAPI::Block *trg,
                           ParseAPI::EdgeTypeEnum type) override;

    ParseAPI::Function *mkfunc(Dyninst::Address addr, ParseAPI::FuncSource src,
                               std::string name, ParseAPI::CodeObject *obj,
                               ParseAPI::CodeRegion *reg,
                               Dyninst::InstructionSource *isrc) override;

    ParseAPI::Block *mksink(ParseAPI::CodeObject *obj, ParseAPI::CodeRegion *r) override;

    // leaving default atm
    // void free_func(Dyninst::ParseAPI::Function * f);
    // void free_block(Dyninst::ParseAPI::Block * b);
    // void free_edge(Dyninst::ParseAPI::Edge * e);

    // void free_all();
    void dump_stats();

  private:
    boost::mutex _mtx;
    image *_img;
    std::vector<int> _func_allocs;
    std::vector<int> _edge_allocs;
    int _block_allocs;
    int _sink_block_allocs;
    // int _sink_edge_allocs; FIXME can't determine

    void _record_func_alloc(Dyninst::ParseAPI::FuncSource fs) {
      assert(fs < ParseAPI::_funcsource_end_);
      ++_func_allocs[fs];
    }
    void _record_edge_alloc(ParseAPI::EdgeTypeEnum et, bool /* sink */) {
      assert(et < ParseAPI::_edgetype_end_);
      ++_edge_allocs[et];

      // if(sink)
      //++_sink_block_allocs;
    }
    void _record_block_alloc(bool sink) {
      ++_block_allocs;
      if (sink)
        ++_sink_block_allocs;
    }
  };

}}

#endif
