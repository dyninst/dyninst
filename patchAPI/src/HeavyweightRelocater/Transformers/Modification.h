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

#if !defined(_R_T_MODIFICATION_H_)
#define _R_T_MODIFICATION_H_

#include "Transformer.h"

class int_block;
class instPoint;

namespace Dyninst {
namespace PatchAPI {

class Modification : public Transformer {
  public:
    // Mimics typedefs in CodeMover.h, but I don't want
    // to include that file.
    typedef std::list<TracePtr> TraceList;
    //typedef std::map<Address, TraceList> TraceMap;
    typedef std::map<Block *, TracePtr> TraceMap;
    // These three mimic definitions in addressSpace.h
    typedef std::map<instPoint *, Function *> ext_CallReplaceMap;
    typedef std::map<Function *, Function *> ext_FuncReplaceMap;
    typedef std::set<instPoint *> ext_CallRemovalSet;

    typedef std::map<const Block *, std::pair<Function *, instPoint *> > CallReplaceMap;
    typedef std::map<const Block *, Function *> FuncReplaceMap;
    typedef std::set<const Block *> CallRemovalSet;

    virtual bool processTrace(TraceList::iterator &);

    Modification(const ext_CallReplaceMap &callRepl,
		 const ext_FuncReplaceMap &funcRepl,
		 const ext_CallRemovalSet &callRem);

    virtual ~Modification() {};

  private:

    void replaceCall(TracePtr block, int_block *target, instPoint *cur);
    void replaceFunction(TracePtr block, int_block *target);
    void removeCall(TracePtr block);

    CallReplaceMap callRep_;
    FuncReplaceMap funcRep_;
    CallRemovalSet callRem_;
  };
};
};

#endif
