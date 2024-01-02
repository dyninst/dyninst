.. _`sec:GraphAdapter.h`:

GraphAdapter.h
##############

.. cpp:namespace:: Dyninst::ParseAPI

.. cpp:function:: boost::graph_traits<Function>::vertices_size_type num_vertices(const Function& g)
.. cpp:function:: boost::graph_traits<Function>::vertex_descriptor source(boost::graph_traits<Function>::edge_descriptor e, const Function& g)
.. cpp:function:: boost::graph_traits<Function>::vertex_descriptor target(boost::graph_traits<Function>::edge_descriptor e, const Function& g)
.. cpp:function:: std::pair<boost::graph_traits<Function>::out_edge_iterator, boost::graph_traits<Function>::out_edge_iterator> out_edges(boost::graph_traits<Function>::vertex_descriptor v, const Function& g)
.. cpp:function:: boost::graph_traits<Function>::degree_size_type out_degree(boost::graph_traits<Function>::vertex_descriptor v, const Function& g)
.. cpp:function:: std::pair<boost::graph_traits<Function>::in_edge_iterator, boost::graph_traits<Function>::in_edge_iterator> in_edges(boost::graph_traits<Function>::vertex_descriptor v, const Function& g)
.. cpp:function:: boost::graph_traits<Function>::degree_size_type in_degree(boost::graph_traits<Function>::vertex_descriptor v, const Function& g)
.. cpp:function:: boost::graph_traits<Function>::degree_size_type degree(boost::graph_traits<Function>::vertex_descriptor v, const Function& g)
.. cpp:function:: bool dominates(Function& f, Address a, Address b)
.. cpp:function:: bool dominates(Function& f, Block* a, Block* b)


Notes
=====

Only usable when ``ENABLE_PARSE_API_GRAPHS``.
