#include "../h/GraphAdapter.h"
#include <boost/graph/dominator_tree.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/version.hpp>

#include <boost/property_map/property_map.hpp>
#include <boost/bind/bind.hpp>

#include <sstream>

using namespace Dyninst;
using namespace ParseAPI;
using namespace boost;
using namespace std;

typedef graph_traits<Function>::vertex_descriptor Vertex;

namespace boost
{
  template<>
  struct property_map<Function, vertex_index_t>
  {
    typedef boost::associative_property_map< std::map<graph_traits<Function>::vertex_descriptor, int> > type;
    typedef type const_type;
      
  };
}


namespace Dyninst
{
  namespace ParseAPI
  {
    
    
      

    template <typename PROPERTY>
    typename property_map<Function, PROPERTY>::const_type get(PROPERTY, const Function& f)
    {
      typedef typename property_map<Function, PROPERTY>::const_type prop_map;
      typedef typename property_traits<prop_map>::value_type value_type;
      typedef typename property_traits<prop_map>::key_type key_type;
      static std::map<key_type, value_type> m;
      
      typename property_map<Function, PROPERTY>::const_type r(m);
      return r;
    }
    
  }
}

bool contains(Block* b, Address a)
{
  return b && (b->start() <= a) && (a < b->end());
}


std::string block_info(Block* b)
{
  std::stringstream s;
  if(b) 
  {
    s << "[" << b->start() << ", " << b->end() << ")";
  }
  else
  {
    s << "(NULL)";
  }
  return s.str();
}

template<typename OS>
OS& operator<<(OS& os, Block* b)
{
  os << block_info(b);
  return os;
}

struct print_vertex : public base_visitor<print_vertex> 
{
  typedef on_discover_vertex event_filter;
  template<class Vertex, class Graph>
  void operator()(Vertex v, Graph& g)
  {
    cout << v << endl;
  }
};

template<class IndexMap>
std::string block_and_index(Block* b, IndexMap indexMap)
{
  std::stringstream s;
  s << "Block " << get(indexMap, b) << ": " << block_info(b);
  return s.str();
}

bool dominates(Function& f, Block* a, Block* b)
{
  graph_traits<Function>::vertex_iterator firstBlock, curBlock, lastBlock;
  

  // Concept checks for testing
  BOOST_CONCEPT_ASSERT((GraphConcept<Function>));
  BOOST_CONCEPT_ASSERT((IncidenceGraphConcept<Function>));
  BOOST_CONCEPT_ASSERT((VertexListGraphConcept<Function>));
  BOOST_CONCEPT_ASSERT((BidirectionalGraphConcept<Function>));
  BOOST_CONCEPT_ASSERT((MultiPassInputIteratorConcept<graph_traits<Function>::vertex_iterator>));
  
  tie(firstBlock, lastBlock) = vertices(f);
  vector<Vertex> domTreePredVector = vector<Vertex>(num_vertices(f), graph_traits<Function>::null_vertex());
  typedef property_map<Function, vertex_index_t>::type IndexMap;
  typedef iterator_property_map<vector<Vertex>::iterator, IndexMap> PredMap;
  const IndexMap indexMap(get(vertex_index, f));
  graph_traits<Function>::vertex_iterator ui, ue;
  int index = 0;
  for(tie(ui, ue) = vertices(f); ui != ue; ++ui, ++index)
  {
    put(indexMap, *ui, index);
  }
  

  PredMap results(make_iterator_property_map(domTreePredVector.begin(), indexMap));
  

  lengauer_tarjan_dominator_tree(f, f.entry(), results);

  // now walk through the tree and find whether a dominates b
  Vertex cur_idom = b;
  do
  {
    Vertex next_idom = get(results, cur_idom);
    cur_idom = next_idom;
  } while(cur_idom != a && cur_idom != graph_traits<Function>::null_vertex());
  return cur_idom == a;
}

bool dominates(Function& f, Address a, Address b)
{
  Function::blocklist::const_iterator found_a, found_b;
  found_a = std::find_if(f.blocks().begin(),
		      f.blocks().end(),
		      boost::bind(contains, boost::placeholders::_1, a));
  found_b = std::find_if(f.blocks().begin(),
		      f.blocks().end(),
		      boost::bind(contains, boost::placeholders::_1, b));
  // If either address is not in f,
  // then no dominator relationship
  if(found_a == f.blocks().end()) return false;
  if(found_b == f.blocks().end()) return false;
  // If same block, go by address
  if(found_a == found_b) return a < b;
  // Check the blocks
  return dominates(f, *found_a, *found_b);
  
}
