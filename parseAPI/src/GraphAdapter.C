#include "../h/GraphAdapter.h"
#include <boost/graph/dominator_tree.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/property_map.hpp>
#include <boost/bind.hpp>

using namespace Dyninst;
using namespace ParseAPI;
using namespace boost;
using namespace std;

typedef graph_traits<Function>::vertex_descriptor Vertex;

template <typename Key>
struct auto_indexer : public boost::put_get_helper<int&, auto_indexer<Key> >
{
  
  auto_indexer() : m_index(0) 
  {
  }
  int& operator[](const Key& k) const
  {
    //if(m_values.find(k) == m_values.end()) m_values[k] = m_index++;
    return m_values[k];
  }

  
  

  mutable int m_index;
  mutable std::map<Key, int> m_values;
  
};


namespace boost
{
  template<typename Key>
  struct property_traits<auto_indexer<Key> >
  {
    typedef Key key_type;
    typedef int value_type;
    typedef int& reference;
    typedef lvalue_property_map_tag category;
    
  };
  
    
    template<>
    struct property_map<Function, vertex_index_t>
    {
      typedef auto_indexer<graph_traits<Function>::vertex_descriptor > type;
      typedef type const_type;
      
    };
}


namespace Dyninst
{
  namespace ParseAPI
  {
    
      
      

    template <typename PROPERTY>
    typename property_map<Function, PROPERTY>::type get(PROPERTY, const Function& f)
    {
      typename property_map<Function, PROPERTY>::const_type r;
      return r;
    }
    
  }
}

bool contains(Block* b, Address a)
{
  return b && (b->start() <= a) && (a < b->end());
}





bool dominates(Function& f, Block* a, Block* b)
{
  graph_traits<Function>::vertex_iterator firstBlock, curBlock, lastBlock;
  

  BOOST_CONCEPT_ASSERT((GraphConcept<Function>));
  BOOST_CONCEPT_ASSERT((IncidenceGraphConcept<Function>));
  BOOST_CONCEPT_ASSERT((VertexListGraphConcept<Function>));
  BOOST_CONCEPT_ASSERT((BidirectionalGraphConcept<Function>));
  BOOST_CONCEPT_ASSERT((MultiPassInputIteratorConcept<graph_traits<Function>::vertex_iterator>));
  
  // Concept checks for testing
  tie(firstBlock, lastBlock) = vertices(f);
  vector<Vertex> domTreePredVector = vector<Vertex>(num_vertices(f), graph_traits<Function>::null_vertex());
  typedef property_map<Function, vertex_index_t>::type IndexMap;
  typedef iterator_property_map<vector<Vertex>::iterator, IndexMap> PredMap;
  IndexMap indexMap(get(vertex_index, f));
  PredMap results(make_iterator_property_map(domTreePredVector.begin(), indexMap));
  
  lengauer_tarjan_dominator_tree(f, *firstBlock, results);
  // now walk through the tree and find whether a dominates b
  Vertex cur_idom = b;
  do
  {
    cur_idom = get(results, cur_idom);
  } while(cur_idom != a && cur_idom != graph_traits<Function>::null_vertex());
  return cur_idom == a;
}

bool dominates(Function& f, Address a, Address b)
{
  Function::blocklist::const_iterator found_a, found_b;
  found_a = std::find_if(f.blocks().begin(),
		      f.blocks().end(),
		      boost::bind(contains, _1, a));
  found_b = std::find_if(f.blocks().begin(),
		      f.blocks().end(),
		      boost::bind(contains, _1, b));
  // If either address is not in f,
  // then no dominator relationship
  if(found_a == f.blocks().end()) return false;
  if(found_b == f.blocks().end()) return false;
  // If same block, go by address
  if(found_a == found_b) return a < b;
  // Check the blocks
  return dominates(f, *found_a, *found_b);
  
}
