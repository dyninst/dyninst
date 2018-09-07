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

#include "RelocGraph.h"
#include "RelocBlock.h"
#include <iostream>

using namespace Dyninst;
using namespace Relocation;
using namespace std;

RelocGraph::~RelocGraph() {
   for (Edges::iterator iter = edges.begin(); iter != edges.end(); ++iter) {
      delete *iter;
   }
   std::set<func_instance*> funcs_to_clean;
   RelocBlock *cur = head;
   while (cur) {
      RelocBlock *next = cur->next();
      funcs_to_clean.insert(cur->func());
      delete cur;
      cur = next;
   }
#if defined(cap_stack_mod)
   for(auto f = funcs_to_clean.begin();
           f != funcs_to_clean.end();
           ++f)
   {
      if(*f) (*f)->freeStackMod();
   }
#endif   
}

void RelocGraph::addRelocBlock(RelocBlock *t) {
   if (t->type() == RelocBlock::Relocated) {
      springboards[std::make_pair(t->block(), t->func())] = t;
      reloc[t->block()->start()][t->func()] = t;
   }

   if (head == NULL) {
      assert(tail == NULL);
      head = t;
      tail = t;
   }
   else {
      assert(tail != NULL);
      link(tail, t);
      tail = t;
   }
   size++;
}

void RelocGraph::addRelocBlockBefore(RelocBlock *cur, RelocBlock *t) {
   if (t->type() == RelocBlock::Relocated) {
      springboards[std::make_pair(t->block(), t->func())] = t;
      reloc[t->block()->start()][t->func()] = t;
   }
   size++;
   if (cur == head) {
      head = t;
   }
   else {
      RelocBlock *prev = cur->prev();
      link(prev, t);
   }
   link(t, cur);
}

void RelocGraph::addRelocBlockAfter(RelocBlock *cur, RelocBlock *t) {
   if (t->type() == RelocBlock::Relocated) {
      springboards[std::make_pair(t->block(), t->func())] = t;
      reloc[t->block()->start()][t->func()] = t;
   }
   size++;
  if (cur == tail) {
      tail = t;
   }
   else {
      RelocBlock *next = cur->next();
      link(t, next);
   }
   link(cur, t);
}

   
RelocBlock *RelocGraph::find(block_instance *b, func_instance *f) const {
   InstanceMap::const_iterator iter = reloc.find(b->start());
   if (iter == reloc.end()) return NULL;
   SubMap::const_iterator iter2 = iter->second.find(f);
   if (iter2 == iter->second.end()) return NULL;

   return iter2->second;
}

RelocBlock *RelocGraph::findSpringboard(block_instance *b, func_instance *f) const {
   Map::const_iterator iter = springboards.find(std::make_pair(b, f));
   if (iter == springboards.end()) return NULL;
   return iter->second;
}

bool RelocGraph::setSpringboard(block_instance *from, func_instance *func, RelocBlock *to) {
   if (springboards.find(std::make_pair(from, func)) == springboards.end()) return false;
   springboards[std::make_pair(from, func)] = to;
   return true;
}

RelocEdge *RelocGraph::makeEdge(TargetInt *s, 
                                TargetInt *t,
                                edge_instance* e,
                                ParseAPI::EdgeTypeEnum et) {
   RelocEdge *edge = new RelocEdge(s, t, e, et);
   edges.push_back(edge);
   s->addTargetEdge(edge);
   t->addSourceEdge(edge);

   return edge;
}

bool RelocGraph::removeEdge(RelocEdge *e) {
   // Pull it from the source and target, but
   // don't worry about our vector; we'll just let it 
   // dangle for now and delete it later. 
   removeSource(e);
   removeTarget(e);
   return true;
}

void RelocGraph::removeSource(RelocEdge *e) {
   e->src->removeTargetEdge(e);
   e->src = NULL;
}

void RelocGraph::removeTarget(RelocEdge *e) {
   e->trg->removeSourceEdge(e);
   e->trg = NULL;
}

void RelocGraph::link(RelocBlock *s, RelocBlock *t) {
   if (s)
      s->setNext(t);
   if (t)
      t->setPrev(s);
}

bool RelocGraph::interpose(RelocEdge *e, RelocBlock *trace) {
   // Take s ->(e) t to
   // s ->(e) n, n ->(FT) t

   TargetInt *s = e->src;
   TargetInt *t = e->trg;
   // Targets aren't refcounted...
   TargetInt *n1 = new Target<RelocBlock *>(trace);
   TargetInt *n2 = new Target<RelocBlock *>(trace);

   makeEdge(s, n1, e->edge, e->type);
   makeEdge(n2, t, e->edge, ParseAPI::FALLTHROUGH);
   removeEdge(e);
   // So we don't try to free 'em...
   e->src = NULL;
   e->trg = NULL;

   return true;
}

bool RelocGraph::changeTarget(RelocEdge *e, TargetInt *n) {
   removeTarget(e);
   delete e->trg;
   e->trg = n;
   n->addSourceEdge(e);
   return true;
}

bool RelocGraph::changeSource(RelocEdge *e, TargetInt *n) {
   removeSource(e);
   delete e->src;
   e->src = n;
   n->addTargetEdge(e);
   return true;
}

bool RelocGraph::changeType(RelocEdge *e, ParseAPI::EdgeTypeEnum t) {
   e->type = t;
   return true;
}
   
bool Predicates::Interprocedural::operator()(RelocEdge *e) {
    // Calls and returns are always interprocedural
    if (e->type == ParseAPI::CALL || e->type == ParseAPI::RET) return true;

    // If there is an underlying edge_instance, use its interproc() method
    if (e->edge != NULL) return e->edge->interproc();

    // If both endpoints are RelocBlocks, check if they are in the same func
    if (e->src->type() == TargetInt::RelocBlockTarget &&
        e->trg->type() == TargetInt::RelocBlockTarget) {
        Target<RelocBlock *> *src = static_cast<Target<RelocBlock *>*>(e->src);
        Target<RelocBlock *> *trg = static_cast<Target<RelocBlock *>*>(e->trg);
        return src->t()->func() != trg->t()->func();
    }

    return false;
}

bool Predicates::Intraprocedural::operator()(RelocEdge *e) {
    // Calls and returns are always interprocedural
    if (e->type == ParseAPI::CALL || e->type == ParseAPI::RET) return false;

    // If there is an underlying edge_instance, use its interproc() method
    if (e->edge != NULL) return !e->edge->interproc();

    // If both endpoints are RelocBlocks, check if they are in the same func
    if (e->src->type() == TargetInt::RelocBlockTarget &&
        e->trg->type() == TargetInt::RelocBlockTarget) {
        Target<RelocBlock *> *src = static_cast<Target<RelocBlock *>*>(e->src);
        Target<RelocBlock *> *trg = static_cast<Target<RelocBlock *>*>(e->trg);
        return src->t()->func() == trg->t()->func();
    }

    return true;
}

bool Predicates::Fallthrough::operator()(RelocEdge *e) {
   return (e->type == ParseAPI::FALLTHROUGH);
}

bool Predicates::Call::operator()(RelocEdge *e) {
   return (e->type == ParseAPI::CALL);
}

bool Predicates::NonCall::operator()(RelocEdge *e) {
   return (e->type != ParseAPI::CALL);
}

bool Predicates::CallFallthrough::operator()(RelocEdge *e) {
   return (e->type == ParseAPI::CALL_FT);
}

bool Predicates::Edge::operator()(RelocEdge *e) {
   if (e->type != e_->type()) return false;
   // Match up source and target blocks
   if (e->src->block() == e_->src() &&
       e->trg->block() == e_->trg()) return true;
   return false;
}

bool Predicates::Type::operator()(RelocEdge *e) {
   return e->type == t_;
}
