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
#if !defined(RELOC_EDGE_H_)
#define RELOC_EDGE_H_

#include <list>

namespace Dyninst {
namespace Relocation {

struct RelocEdge {
   RelocEdge(TargetInt *s, TargetInt *t, edge_instance* e, ParseAPI::EdgeTypeEnum et) :
     src(s), trg(t), edge(e), type(et) {}

   ~RelocEdge();

   TargetInt *src;
   TargetInt *trg;
   edge_instance *edge;
   ParseAPI::EdgeTypeEnum type;
};

struct RelocEdges {
   RelocEdges() {}

   typedef std::list<RelocEdge *>::iterator iterator;
   typedef std::list<RelocEdge *>::const_iterator const_iterator;
   iterator begin() { return edges.begin(); }
   iterator end() { return edges.end(); }
   const_iterator begin() const { return edges.begin(); }
   const_iterator end() const { return edges.end(); }
   void push_back(RelocEdge *e) { edges.push_back(e); }
   void insert(RelocEdge *e) { edges.push_back(e); }

   RelocEdge *find(ParseAPI::EdgeTypeEnum e);
   void erase(RelocEdge *);
   
   bool contains(ParseAPI::EdgeTypeEnum e);
   std::list<RelocEdge *> edges;
};

}
}

#endif
