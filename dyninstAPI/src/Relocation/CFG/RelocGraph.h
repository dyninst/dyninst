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


#if !defined(_R_RELOC_GRAPH_H_)
#define _R_RELOC_GRAPH_H_

#include "CFG.h"
#include "RelocBlock.h" // RelocEdges
#include "RelocTarget.h" // Targets
#include <vector>
#include <map>
#include <list>
#include <utility>

class codeGen;
class block_instance;
class func_instance;

namespace Dyninst {

namespace Relocation {

class RelocBlock;
struct RelocEdge;
class TargetInt;
class Widget;
typedef boost::shared_ptr<Widget> WidgetPtr;
typedef std::list<WidgetPtr> WidgetList;

class RelocGraph {
  public:
   typedef std::map<func_instance *, RelocBlock *> SubMap;
   typedef std::map<Address, SubMap> InstanceMap;
   typedef std::map<std::pair<block_instance *, func_instance *>,
      RelocBlock *> Map;
   typedef std::vector<RelocEdge *> Edges;
   
   RelocGraph() : head(0), tail(0), size(0) {}
   ~RelocGraph();
   RelocBlock *begin() { return head; }
   RelocBlock *end() { return NULL; }


   // For initial construction
   void addRelocBlock(RelocBlock *);
   void addRelocBlockBefore(RelocBlock *cur, RelocBlock *add);
   void addRelocBlockAfter(RelocBlock *cur, RelocBlock *add);

   RelocEdge *makeEdge(TargetInt *, TargetInt *, edge_instance* e, ParseAPI::EdgeTypeEnum et);

   bool removeEdge(RelocEdge *);
   void removeSource(RelocEdge *);
   void removeTarget(RelocEdge *);

   RelocBlock *head;
   RelocBlock *tail;
   unsigned size;

   // If we keep a master list here and just keep pointers
   // everywhere else, memory management gets a hell of a 
   // lot easier...

   Edges edges;
   
   Map springboards;
   InstanceMap reloc;

   RelocBlock *find(block_instance *, func_instance *) const;
   bool setSpringboard(block_instance *from, func_instance *func, RelocBlock *to);
   RelocBlock *findSpringboard(block_instance *from, func_instance *to) const;

  // Should this go here? Well, it's a transformation on RelocBlocks...
  void link(RelocBlock *s, RelocBlock *t);
  bool interpose(RelocEdge *e, RelocBlock *n);
  bool changeTarget(RelocEdge *e, TargetInt *n);
  bool changeSource(RelocEdge *e, TargetInt *n);
  bool changeType(RelocEdge *e, ParseAPI::EdgeTypeEnum t);

  template <class Predicate> 
  void applyPredicate(Predicate &p, RelocEdges *e, RelocEdges &results);
  //bool addTarget(RelocBlock *s, RelocBlock *t, ParseAPI::EdgeTypeEnum type);
  //bool remove(RelocEdge *e);
  RelocBlock *split(RelocBlock *c, WidgetList::iterator where);

  template <class Predicate> 
     bool interpose(Predicate &p, RelocEdges *e, RelocBlock *t);
  template <class Predicate, class Dest> 
     bool changeTargets(Predicate &p, RelocEdges *e, Dest n);
  template <class Predicate, class Source> 
     bool changeSources(Predicate &p, RelocEdges *e, Source n);
  template <class Predicate>
     bool removeEdge(Predicate &p, RelocEdges *e);
  template <class Predicate>
     bool changeType(Predicate &p, RelocEdges *e, ParseAPI::EdgeTypeEnum t);


  struct InterproceduralPredicate {
     bool operator()(RelocEdge *e);
  };

  struct IntraproceduralPredicate {
     bool operator()(RelocEdge *e);
  };
};
   
struct Predicates {
   struct Interprocedural {
      bool operator() (RelocEdge *e);
   };
   struct Intraprocedural {
      bool operator() (RelocEdge *e);
   };
   struct Fallthrough {
      bool operator() (RelocEdge *e);
   };
   struct CallFallthrough {
      bool operator() (RelocEdge *e);
   };
   struct NonCall {
      bool operator() (RelocEdge *e);
   };
   struct Call {
      bool operator() (RelocEdge *e);
   };

   struct Edge {
   Edge(edge_instance *e) : e_(e) {}
      bool operator() (RelocEdge *e);
      edge_instance *e_;
   };
   struct Type {
   Type(ParseAPI::EdgeTypeEnum t) : t_(t) {}
      bool operator() (RelocEdge *e);
      ParseAPI::EdgeTypeEnum t_;
   };
};

template <class Predicate> 
   void RelocGraph::applyPredicate(Predicate &p, RelocEdges *e, RelocEdges &results) {
   for (RelocEdges::iterator iter = e->begin();
        iter != e->end(); ++iter) {
      if (p(*iter)) results.push_back(*iter);
   }
}

template <class Predicate, class Dest>
   bool RelocGraph::changeTargets(Predicate &p, RelocEdges *e, Dest n) {
   RelocEdges tmp;
   applyPredicate(p, e, tmp);
   for (RelocEdges::iterator iter = tmp.begin();
        iter != tmp.end(); ++iter) {
      if (!changeTarget(*iter, new Target<Dest >(n))) return false;
   }
   return true;
}

template <class Predicate, class Source>
   bool RelocGraph::changeSources(Predicate &p, RelocEdges *e, Source n) {
   RelocEdges tmp;
   applyPredicate(p, e, tmp);
   for (RelocEdges::iterator iter = tmp.begin();
        iter != tmp.end(); ++iter) {
      if (!changeSource(*iter, new Target<Source>(n))) return false;
   }
   return true;
}

template <class Predicate> 
   bool RelocGraph::removeEdge(Predicate &p, RelocEdges *e) {
   RelocEdges tmp;
   applyPredicate(p, e, tmp);
   for (RelocEdges::iterator iter = tmp.begin();
        iter != tmp.end(); ++iter) {
      if (!removeEdge(*iter)) return false;
   }
   return true;
}

template <class Predicate> 
   bool RelocGraph::interpose(Predicate &p, RelocEdges *e, RelocBlock *t) {
   RelocEdges tmp;
   applyPredicate(p, e, tmp);
   for (RelocEdges::iterator iter = tmp.begin();
        iter != tmp.end(); ++iter) {
      if (!interpose(*iter, t)) return false;
   }
   return true;
}

template <class Predicate>
   bool RelocGraph::changeType(Predicate &p, RelocEdges *e, ParseAPI::EdgeTypeEnum t) {
   RelocEdges tmp;
   applyPredicate(p, e, tmp);
   for (RelocEdges::iterator iter = tmp.begin();
        iter != tmp.end(); ++iter) {
      if (!changeType(*iter, t)) return false;
   }
   return true;
}

}
}

#endif

