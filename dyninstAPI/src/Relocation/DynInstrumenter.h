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
#ifndef _DYNINSTRUMENTER_H_
#define _DYNINSTRUMENTER_H_


#include "Instrumenter.h"
#include "DynAddrSpace.h"
#include "Command.h"

#include "dyninstAPI/src/instPoint.h"

using Dyninst::PatchAPI::DynAddrSpace;
using Dyninst::PatchAPI::Command;

class BPatch_point;
class BPatchSnippetHandle;

namespace Dyninst {
namespace PatchAPI {

class DynInstrumenter : public Dyninst::PatchAPI::Instrumenter {
  public:
    DynInstrumenter() {}
    virtual ~DynInstrumenter() {}
    virtual bool run();
    virtual bool undo();
    virtual bool isInstrumentable(PatchFunction*);

};

/* Dyninst-specific Insert Snippet Command  */
class DynInsertSnipCommand : public Command {
  public:
    DynInsertSnipCommand(instPoint* pt, callOrder order,
                         AstNodePtr ast, bool recursive);
    static DynInsertSnipCommand* create(instPoint* pt, callOrder order,
                      AstNodePtr ast, bool recursive);
    virtual ~DynInsertSnipCommand() {}

    Instance::Ptr inst() { return inst_; }

    virtual bool run();
    virtual bool undo();

  protected:
    Instance::Ptr inst_;
};

/* Dyninst-specific Remove Snippet Command  */
class DynRemoveSnipCommand : public Command {
  public:
      DynRemoveSnipCommand(Instance::Ptr inst);
      static DynRemoveSnipCommand* create(Instance::Ptr inst);
    virtual ~DynRemoveSnipCommand() {}

    virtual bool run();
    virtual bool undo();

  protected:
    Instance::Ptr inst_;
};

/* Dyninst-specific Function Replacement */
class DynReplaceFuncCommand : public Command {
  public:
    DynReplaceFuncCommand(AddressSpace* as,
                          func_instance* old_func,
                          func_instance* new_func);
    static DynReplaceFuncCommand* create(AddressSpace* as,
                      func_instance* old_func,
                      func_instance* new_func);
    virtual ~DynReplaceFuncCommand() {}

    virtual bool run();
    virtual bool undo();

  protected:
    AddressSpace* as_;
    func_instance *old_func_;
    func_instance *new_func_;
};

/* Dyninst-specific Modify Function call */
class DynModifyCallCommand : public Command {
  public:
    DynModifyCallCommand(AddressSpace* as,
                         block_instance* block,
                         func_instance* new_func,
                         func_instance* context);
    static DynModifyCallCommand* create(AddressSpace* as,
                      block_instance* block,
                      func_instance* new_func,
                      func_instance* context);
    virtual ~DynModifyCallCommand() {}

    virtual bool run();
    virtual bool undo();

  protected:
    AddressSpace* as_;
    block_instance *block_;
    func_instance *new_func_;
    func_instance *context_;
};

/* Dyninst-specific Remove Function call */
class DynRemoveCallCommand : public Command {
  public:
    DynRemoveCallCommand(AddressSpace* as,
                         block_instance* block,
                         func_instance* context);
    static DynRemoveCallCommand* create(AddressSpace* as,
                      block_instance* block,
                      func_instance* context);
    virtual ~DynRemoveCallCommand() {}

    virtual bool run();
    virtual bool undo();

  protected:
    AddressSpace* as_;
    block_instance *block_;
    func_instance *context_;
};


}
}

#endif /* _DYNINSTRUMENTER_H_ */
