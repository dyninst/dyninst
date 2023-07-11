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

#ifndef PATCHAPI_H_DYNINST_OBJECT_H_
#define PATCHAPI_H_DYNINST_OBJECT_H_

#include <map>
#include <string>
#include <vector>
#include "PatchCommon.h"
#include "CFGMaker.h"

namespace Dyninst {
namespace PatchAPI {

class PatchFunction;
class PatchCallback;
class PatchParseCallback;

/* PatchObject represents a binary object, which could be either a library or
   executable. It is also an instrumentation  unit. */
class PATCHAPI_EXPORT PatchObject {
  friend class AddrSpace;
  friend class PatchParseCallback;

  public:
    static PatchObject* create(ParseAPI::CodeObject* co, Address base,
                                               CFGMaker* cm = NULL,
                                               PatchCallback *cb = NULL);

    static PatchObject* clone(PatchObject* par_obj, Address base,
                                               CFGMaker* cm = NULL,
                                               PatchCallback *cb = NULL);

    virtual ~PatchObject();

    typedef std::vector<PatchFunction *> funclist;
    typedef std::map<const ParseAPI::Function*, PatchFunction*> FuncMap;
    typedef std::map<const ParseAPI::Block*, PatchBlock*> BlockMap;
    typedef std::map<const ParseAPI::Edge*, PatchEdge*> EdgeMap;

    std::string format() const;

    // Getters and setter
    Address codeBase() const { return codeBase_; }
    Address codeOffsetToAddr(Address offset) const;
    Address addrMask() const;
    ParseAPI::CodeObject* co() const { return co_; }
    //ParseAPI::CodeSource* cs() const { return cs_; }
    AddrSpace* addrSpace() const { return addr_space_; }
    void setAddrSpace(AddrSpace* as);
    PatchMgrPtr mgr() const;

    // Function
    PatchFunction *getFunc(ParseAPI::Function *, bool create = true);
    void addFunc(PatchFunction*);
    void removeFunc(PatchFunction*);
    void removeFunc(ParseAPI::Function *);
    template <class Iter> 
	void funcs(Iter iter); 
    // Block
    PatchBlock *getBlock(ParseAPI::Block*, bool create = true);
    void addBlock(PatchBlock*);
    void removeBlock(PatchBlock*);
    void removeBlock(ParseAPI::Block*);
    template <class Iter>
     void blocks(Iter iter); 

    // Edge
    PatchEdge *getEdge(ParseAPI::Edge*, PatchBlock* = NULL, PatchBlock* = NULL, bool create = true);
    void addEdge(PatchEdge*);
    void removeEdge(PatchEdge*);
    void removeEdge(ParseAPI::Edge*);
    template <class Iter>
      void edges(Iter iter); 

    PatchCallback *cb() const { return cb_; }

    bool consistency(const AddrSpace *as) const;


  protected:
    ParseAPI::CodeObject* co_;
    Address codeBase_;
    AddrSpace* addr_space_;
    FuncMap funcs_;
    BlockMap blocks_;
    EdgeMap edges_;
    CFGMaker* cfg_maker_;

    PatchObject(ParseAPI::CodeObject* o, Address a, CFGMaker* cm, PatchCallback *cb = NULL);
    PatchObject(const PatchObject* par_obj, Address a, CFGMaker* cm, PatchCallback *cb = NULL);
    void copyCFG(PatchObject* par_obj);
    bool splitBlock(PatchBlock *first, ParseAPI::Block *second);

    void createFuncs();
    void createBlocks();
    void createEdges();

    PatchCallback *cb_;
    PatchParseCallback *pcb_;
};

template <class Iter>
   void PatchObject::funcs(Iter iter) {
   createFuncs();
   for (FuncMap::iterator tmp = funcs_.begin(); tmp != funcs_.end(); ++tmp) {
      *iter = tmp->second;
      ++iter;
   }
}

template <class Iter>
   void PatchObject::blocks(Iter iter) {
   createBlocks();
   for (BlockMap::iterator tmp = blocks_.begin(); tmp != blocks_.end(); ++tmp) {
      *iter = tmp->second;
      ++iter;
   }
}

template <class Iter>
   void PatchObject::edges(Iter iter) {
   createEdges();
   for (EdgeMap::iterator tmp = edges_.begin(); tmp != edges_.end(); ++tmp) {
      *iter = tmp->second;
      ++iter;
   }
}



}
}

#endif  // PATCHAPI_H_DYNINST_MODULE_H_
