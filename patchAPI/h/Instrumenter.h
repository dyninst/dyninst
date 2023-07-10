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
/* Plugin Interface */

#ifndef PATCHAPI_H_INSTRUMENTOR_H_
#define PATCHAPI_H_INSTRUMENTOR_H_

#include <string>
#include "PatchCommon.h"
#include "Point.h"
#include "AddrSpace.h"
#include "Command.h"

namespace Dyninst {
namespace PatchAPI {

/* Relocate the original code and generate snippet binary code in mutatee's
   address space. */

class PATCHAPI_EXPORT Instrumenter : public BatchCommand {
  public:
    friend class Patcher;
    static Instrumenter* create(AddrSpace* as);
    virtual ~Instrumenter() {}

    // Code Modification interfaces
    // Function Replacement
    virtual bool replaceFunction(PatchFunction* oldfunc,
                                                 PatchFunction* newfunc);
    virtual bool revertReplacedFunction(PatchFunction* oldfunc);
    virtual FuncModMap& funcRepMap() { return functionReplacements_; }

    // Function Wrapping
    virtual bool wrapFunction(PatchFunction* oldfunc,
                                              PatchFunction* newfunc,
                                              std::string name);
    virtual bool revertWrappedFunction(PatchFunction* oldfunc);
    virtual FuncWrapMap& funcWrapMap() { return functionWraps_; }

    // Call Modification
    virtual bool modifyCall(PatchBlock *callBlock, PatchFunction *newCallee,
                                    PatchFunction *context = NULL);
    virtual bool revertModifiedCall(PatchBlock *callBlock, PatchFunction *context = NULL);
    virtual bool removeCall(PatchBlock *callBlock, PatchFunction *context = NULL);
    virtual CallModMap& callModMap() { return callModifications_; }

    // Getters and setters
    AddrSpace* as() const { return as_; }
    void setAs(AddrSpace* as) { as_ = as; }
    virtual bool isInstrumentable(PatchFunction* ) { return true; }
  protected:
    AddrSpace* as_;
    CommandList user_commands_;
    FuncModMap functionReplacements_;
    FuncWrapMap functionWraps_;
    CallModMap callModifications_;

    explicit Instrumenter(AddrSpace* as) : as_(as) {}
    Instrumenter(): as_(NULL) {}
};
}
}
#endif  // PATCHAPI_H_INSTRUMENTOR_H_
