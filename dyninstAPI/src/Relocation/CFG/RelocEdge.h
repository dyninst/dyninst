#if !defined(RELOC_EDGE_H_)
#define RELOC_EDGE_H_

namespace Dyninst {
namespace Relocation {

struct RelocEdge {
   RelocEdge(TargetInt *s, TargetInt *t, ParseAPI::EdgeTypeEnum e) :
     src(s), trg(t), type(e) {};

   ~RelocEdge();

   TargetInt *src;
   TargetInt *trg;
   ParseAPI::EdgeTypeEnum type;
};

struct RelocEdges {
   RelocEdges() {};

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

};
};

#endif
