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

#if !defined(_R_T_MODIFICATION_H_)
#define _R_T_MODIFICATION_H_

#include "Transformer.h"
#include "../Widgets/Widget.h"
#include "dyninstAPI/src/addressSpace.h"
#include <string>
#include <list>
#include <set>
#include <map>
#include "../DynInstrumenter.h"

class block_instance;
class instPoint;
class func_instance;

namespace Dyninst {
namespace Relocation {

class Modification : public Transformer {
  public:
    // Mimics typedefs in CodeMover.h, but I don't want
    // to include that file.
    typedef std::list<RelocBlock *> RelocBlockList;
    //typedef std::map<Address, RelocBlockList> RelocBlockMap;

    // Block (IDing a call site) -> func
    typedef Dyninst::PatchAPI::CallModMap CallModMap;
    // func -> func
    typedef Dyninst::PatchAPI::FuncModMap FuncModMap;
    typedef Dyninst::PatchAPI::FuncWrapMap FuncWrapMap;

    virtual bool process(RelocBlock *cur, RelocGraph *);

    Modification(const CallModMap &callRepl,
                 const FuncModMap &funcRepl,
                 const FuncWrapMap &funcWrap);

    virtual ~Modification() {}

  private:

    bool replaceCall(RelocBlock *trace, RelocGraph *);
    bool replaceFunction(RelocBlock *trace, RelocGraph *);
    bool wrapFunction(RelocBlock *trace, RelocGraph *);

    RelocBlock *makeRelocBlock(block_instance *block, 
			       func_instance *func, 
			       RelocBlock *cur,
			       RelocGraph *cfg);

    const CallModMap &callMods_;
    const FuncModMap &funcReps_;
    const FuncWrapMap &funcWraps_;

    struct WrapperPredicate {
       WrapperPredicate(func_instance *f);
       func_instance *f_;
       bool operator()(RelocEdge *e);
    };

    struct WrapperPatch : public Patch {
      WrapperPatch(func_instance *func, std::string name) : func_(func), name_(name) {}
       virtual bool apply(codeGen &gen, CodeBuffer *buf);
       virtual unsigned estimate(codeGen &) { return 0; }
       virtual ~WrapperPatch() {}

       func_instance *func_;
       std::string name_;
    };
    
  };
   
}
}

#endif
