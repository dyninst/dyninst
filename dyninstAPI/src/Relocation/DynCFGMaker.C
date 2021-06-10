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

#include "DynObject.h"
#include "dyninstAPI/src/mapped_module.h"
#include "dyninstAPI/src/mapped_object.h"
#include "dyninstAPI/src/parse-cfg.h"
#include "dyninstAPI/src/function.h"

using Dyninst::PatchAPI::DynCFGMaker;
using Dyninst::PatchAPI::PatchObject;
using Dyninst::ParseAPI::Function;
using Dyninst::PatchAPI::PatchFunction;
using Dyninst::ParseAPI::Block;
using Dyninst::PatchAPI::PatchBlock;
using Dyninst::ParseAPI::Edge;
using Dyninst::PatchAPI::PatchEdge;

PatchFunction* DynCFGMaker::makeFunction(Function* f,
                                         PatchObject* obj) {
  Address code_base = obj->codeBase();
  mapped_object* mo = SCAST_MO(obj);
  parse_func* img_func = SCAST_PF(f);
  if (!img_func) return NULL;
  assert(img_func->getSymtabFunction());
  mapped_module* mod = mo->findModule(img_func->pdmod());
  if (!mod) {
    fprintf(stderr, "%s[%d]: ERROR: cannot find module %p\n", FILE__, __LINE__,
            (void*)img_func->pdmod());
    fprintf(stderr, "%s[%d]:  ERROR:  Cannot find module %s\n", FILE__, __LINE__,
            img_func->pdmod()->fileName().c_str());
  }
  func_instance* fi = new func_instance(img_func, code_base, mod);
  mo->addFunction(fi);
  return fi;
}

PatchFunction* DynCFGMaker::copyFunction(PatchFunction* f, PatchObject* o) {
  func_instance *parFunc = SCAST_FI(f);
  mapped_object *mo = SCAST_MO(o);
  assert(parFunc->mod());
  mapped_module *mod = mo->getOrCreateForkedModule(parFunc->mod());
  func_instance *newFunc = new func_instance(parFunc, mod);
  mo->addFunction(newFunc);
  return newFunc;
}

PatchBlock* DynCFGMaker::makeBlock(ParseAPI::Block* b, PatchObject* obj) {
  block_instance *inst = new block_instance(b, SCAST_MO(obj));
  obj->addBlock(inst);
  return inst;
}

PatchBlock* DynCFGMaker::copyBlock(PatchBlock* b, PatchObject* o) {
  block_instance *newBlock = new block_instance(SCAST_BI(b), SCAST_MO(o));
  // o->addBlock(newBlock);
  return  newBlock;
}

// assumes edges do not cross object boundaries
PatchEdge* DynCFGMaker::makeEdge(ParseAPI::Edge* e,
                                 PatchBlock* s,
                                 PatchBlock* t,
                                 PatchObject* o) 
{
# if 1 // allows inter-object edges
  mapped_object* moS = NULL;
  mapped_object* moT = NULL;
  if (!s) {
      if (e->src()->obj() == o->co()) 
          moS = SCAST_MO(o);
      else 
          moS = SCAST_MO(o)->as()->findObject(e->src()->obj());
  }
  if (!t) {
      if (e->trg()->obj() == o->co()) 
          moT = SCAST_MO(o);
      else 
          moT = SCAST_MO(o)->as()->findObject(e->trg()->obj());
  }

  edge_instance *inst = new edge_instance(e,
                         s ? SCAST_BI(s) : moS->findBlock(e->src()),
                         t ? SCAST_BI(t) : moT->findBlock(e->trg()));

#else // doesn't allow inter-object edges
  mapped_object* mo = t ? t->obj() : NULL;
  edge_instance *inst = new edge_instance(e,
                         s ? SCAST_BI(s) : mo->findBlock(e->src()),
                         t ? SCAST_BI(t) : mo->findBlock(e->trg()));
#endif
  return inst;
}

PatchEdge* DynCFGMaker::copyEdge(PatchEdge* e, PatchObject* o) {
  edge_instance *new_edge = new edge_instance(SCAST_EI(e), SCAST_MO(o));
  return new_edge;
}
