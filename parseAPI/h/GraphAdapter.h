

#include "CFG.h"
#include <stddef.h>
#include <utility>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>

using namespace Dyninst;
using namespace ParseAPI;
using namespace std;

namespace boost
{
  template<>
  struct graph_traits<Function>
  {
    // Boost graph typedefs
    struct function_tags : public virtual boost::vertex_list_graph_tag, 
    public virtual boost::bidirectional_graph_tag
    {
    };
    
    
    typedef function_tags traversal_category;
    typedef Function::blocklist::const_iterator vertex_iterator;
    typedef size_t vertices_size_type;
    typedef Block* vertex_descriptor;
    typedef Edge* edge_descriptor;
    typedef boost::directed_tag directed_category;
    typedef boost::allow_parallel_edge_tag edge_parallel_category;
    static vertex_descriptor null_vertex() 
    {
      return NULL;
    }
    typedef Block::edgelist::const_iterator out_edge_iterator;
    typedef Block::edgelist::const_iterator in_edge_iterator;
    typedef void edge_iterator;
    typedef void adjacency_iterator;
    typedef size_t degree_size_type;
    typedef size_t edges_size_type;
  };
  template<>
  struct vertex_property_type<Function>
  {
    typedef property<vertex_index_t, int> type;
  };
  
  
  

  std::pair<boost::graph_traits<Function>::vertex_iterator,
  boost::graph_traits<Function>::vertex_iterator> vertices(const Dyninst::ParseAPI::Function& g)
  {
    return std::make_pair(g.blocks().begin(), g.blocks().end());
  }
}

boost::graph_traits<Function>::vertices_size_type num_vertices(const Function& g)
{
  return g.blocks().size();
}
boost::graph_traits<Function>::vertex_descriptor source(boost::graph_traits<Function>::edge_descriptor e, const Function& g)
{
  return e->src();
}
boost::graph_traits<Function>::vertex_descriptor target(boost::graph_traits<Function>::edge_descriptor e, const Function& g)
{
  return e->trg();
}  
std::pair<boost::graph_traits<Function>::out_edge_iterator, boost::graph_traits<Function>::out_edge_iterator>
 out_edges(boost::graph_traits<Function>::vertex_descriptor v, const Function& g)
{
  return std::make_pair(v->targets().begin(), v->targets().end());
}
boost::graph_traits<Function>::degree_size_type out_degree(boost::graph_traits<Function>::vertex_descriptor v, const Function& g)
{
  return v->targets().size();
}
std::pair<boost::graph_traits<Function>::in_edge_iterator, boost::graph_traits<Function>::in_edge_iterator>
 in_edges(boost::graph_traits<Function>::vertex_descriptor v, const Function& g)
{
  return std::make_pair(v->sources().begin(), v->sources().end());
}
boost::graph_traits<Function>::degree_size_type in_degree(boost::graph_traits<Function>::vertex_descriptor v, const Function& g)
{
  return v->sources().size();
}  
boost::graph_traits<Function>::degree_size_type degree(boost::graph_traits<Function>::vertex_descriptor v, const Function& g)
{
  return in_degree(v,g)+out_degree(v,g);
  
}  
PARSER_EXPORT bool dominates(Function& f, Address a, Address b);
PARSER_EXPORT bool dominates(Function& f, Block* a, Block* b);
