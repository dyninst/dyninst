#include <stdio.h>


#include "MC_NetworkGraph.h"
#include "mrnet/src/MC_CommunicationNode.h"
#include "mrnet/src/utils.h"
#include "mrnet/src/config.h"

/***************************************************
 * MC_NetworkNode
 **************************************************/
MC_NetworkNode::MC_NetworkNode(char * _hostname, unsigned short _port)
  :hostname(_hostname), port(_port), _visited(false)
{
}

unsigned short MC_NetworkNode::get_Port()
{
  return port;
}

string MC_NetworkNode::get_HostName()
{
  return hostname;
}

void MC_NetworkNode::add_Child(MC_NetworkNode * child)
{
  mc_printf((stderr, "Adding %s as %d child of Node %s(%p)\n", 
	     child->hostname.c_str(),children.size(), hostname.c_str(), this));
  children.push_back(child);
}

bool MC_NetworkNode::visited()
{
  return _visited;
}

void MC_NetworkNode::visit()
{
  _visited=true;
}

/***************************************************
 * MC_NetworkGraph
 **************************************************/
MC_NetworkGraph::MC_NetworkGraph()
  :graph_checked(false), visited_nodes(0), _has_cycle(false)
{
  add_Node(root);
  endpoints = new vector <MC_EndPoint *>;
}

void MC_NetworkGraph::set_Root(MC_NetworkNode * _root)
{
  root = _root;
}

vector <MC_EndPoint *> * MC_NetworkGraph::get_EndPoints()
{
  return endpoints;
}

MC_SerialGraph & MC_NetworkGraph::get_SerialGraph()
{
  return serial_graph;
}

void MC_NetworkGraph::add_Node(MC_NetworkNode* new_node)
{
  char port_str[128];
  sprintf(port_str, "%d", new_node->get_Port());

  if(new_node){
    string key = new_node->get_HostName() + string(port_str);
    nodes[key] = new_node;
  }
}

MC_NetworkNode * MC_NetworkGraph::get_Root()
{
  return root;
}

MC_NetworkNode * MC_NetworkGraph::find_Node(char * hostname, unsigned short port)
{
  MC_NetworkNode *node;
  char port_str[128];
  sprintf(port_str, "%d", port);
  string key = string(hostname) + string(port_str);

  if( nodes.find(key) != nodes.end() ){
    return node;
  }
  else{
    return NULL;
  }
}

bool MC_NetworkGraph::has_cycle(){
  if(!graph_checked){
    preorder_traversal(root);
    graph_checked=true;
  }

  return _has_cycle;
}

void MC_NetworkGraph::preorder_traversal(MC_NetworkNode * node){
  static int next_leaf_id=0;

  mc_printf((stderr, "preorder_traversing node %s (%p) ...",
             node->get_HostName().c_str(), node ));

  if( node->visited() == true ){
    mc_printf((stderr, "Node already visited\n"));
    //Should I stop here?
    _has_cycle=true;
  }
  else{
    mc_printf((stderr, "Node's 1st visit\n"));
    node->visit();
    visited_nodes++;

    if(node->children.size() == 0){
      mc_printf((stderr, "Node has no children\n"));
      // I am a leaf node, just add my name to the serial representation and return
      node->id = next_leaf_id++;
      serial_graph.add_Child(node->id, node->get_HostName());
      endpoints->push_back(MC_EndPoint::new_EndPoint( node->id,
						 node->get_HostName().c_str(),
						 0));
      return;
    }
    else{
      mc_printf((stderr, "Node has %d children\n", node->children.size() ));
      //Starting new sub-tree component in graph serialization:
      serial_graph.add_SubTree(node->get_HostName());
    }
  }

  for(unsigned int i=0; i<node->children.size(); i++){
    preorder_traversal(node->children[i]);
  }

  //Ending sub-tree component in graph serialization:
  serial_graph.end_SubTree();
  return;
}

bool MC_NetworkGraph::fully_connected()
{
  if(!graph_checked){
    preorder_traversal(root);
    graph_checked=true;
  }

  mc_printf((stderr, "In fully_connected(). visited %d, exist %d\n",
             visited_nodes, nodes.size() ));
  return ( visited_nodes == nodes.size() ) ;
}

int MC_NetworkGraph::get_Size()
{
  return nodes.size();
}


/***************************************************
 * MC_SerialGraph
 **************************************************/
MC_SerialGraph::MC_SerialGraph(string _byte_array)
  :byte_array(_byte_array), num_nodes(0), num_backends(0)
{
}

MC_SerialGraph::MC_SerialGraph()
  :num_nodes(0), num_backends(0)
{
}

void MC_SerialGraph::add_Child(int id, string hostname)
{
  char id_str[128];
  sprintf(id_str, "%d", id);
  byte_array += hostname + ":" + string(id_str) + " ";
  num_nodes++; num_backends++;
}

void MC_SerialGraph::add_SubTree(string hostname)
{
  byte_array += "[ " + hostname + " ";
  num_nodes++;
}

void MC_SerialGraph::end_SubTree()
{
  byte_array += "] ";
}

void MC_SerialGraph::set_ToFirstChild()
{
  unsigned int i;
  //Set buf_idx to this positions: [ xxx yyy ...
  for(buf_idx = 3; byte_array[buf_idx-1] != ' '; buf_idx++);

  mc_printf((stderr, "In set_tofirstchild():\n"));
  mc_printf((stderr, "byte_array: %s\n", byte_array.c_str()));
  mc_printf((stderr, "1st child : "));
  for(i=0; i<buf_idx; i++){
    _fprintf((stderr, " "));
  }
  _fprintf((stderr, "^\n"));
}

MC_SerialGraph * MC_SerialGraph::get_NextChild()
{
  MC_SerialGraph * retval;
  unsigned int i, begin, end, cur;
  const char * buf = byte_array.c_str();
  bool leaf_node=false;

  mc_printf((stderr, "In get_nextchild():\n"));
  mc_printf((stderr, "byte_array: %s\n", byte_array.c_str()));
  mc_printf((stderr, "    child : "));
  for(i=0; i<buf_idx; i++){
    _fprintf((stderr, " "));
  }
  _fprintf((stderr, "^\n"));

  mc_printf((stderr, "buf_idx: %d, array_len: %d\n", buf_idx, byte_array.length()));
  if(buf_idx >= byte_array.length()-2){
    return NULL;
  }

  cur=begin=buf_idx; end=1;
  if(buf[begin] == '['){ //Child is a SubTree: find matching ']'
    int num_leftbrackets=1, num_rightbrackets=0;

    while(num_leftbrackets != num_rightbrackets){
      cur++, end++;
      if(buf[cur] == '[')
	num_leftbrackets++;
      else if(buf[cur] == ']')
	num_rightbrackets++;
    }
  }
  else{   //Child is a Leaf Node
    leaf_node = true;
    while(buf[cur+1] != ' '){
      cur++; end++;
    }
  }
  buf_idx = cur + 2;

  if(!leaf_node){
    retval = new MC_SerialGraph(byte_array.substr(begin, end));
    //retval->print();
  }
  else{
    retval = new MC_SerialGraph("[ " + byte_array.substr(begin, end) + " ]");
  }

  mc_printf((stderr, "get_nextchild() returning:"));  retval->print();
  return retval;
}

bool MC_SerialGraph::has_children()
{
  //does first "host" have ":id" suffix
  for(int idx=2; byte_array[idx] != ' '; idx++){
    if(byte_array[idx] == ':'){
      return false;
    }
  }
  return true;
}

string MC_SerialGraph::get_RootName()
{
  string retval;
  int cur=2, begin=2, end=1; //Byte array begins [ xxx ...

  //find first space or ':'
  while(byte_array[cur+1] != ' ' && byte_array[cur+1] != ':'){
    cur++; end++;
  }

  retval = byte_array.substr(begin, end);
  mc_printf((stderr, "In get_rootname(). array: %s, root %s\n",
	     byte_array.c_str(), retval.c_str()));

  return retval;
}

string MC_SerialGraph::get_ByteArray()
{
  return byte_array;
}

void MC_SerialGraph::print()
{
  _fprintf((stderr, "Serial Graph (%d nodes) = %s.\n", num_nodes,
          byte_array.c_str()));
}

int MC_SerialGraph::get_Id(){
  assert(!has_children());

  int retval=0;
  int cur, begin=1, end=1; //Byte array begins [ xxx ...

  //find ':'
  while(byte_array[begin-1] != ':'){
    begin++;
  }
  cur=begin;

  while(byte_array[cur+1] != ' '){
    cur++, end++;
  }

  string idstring = byte_array.substr(begin, end);

  retval = atoi(idstring.c_str());
  mc_printf((stderr, "In get_Id(). byte_array: %s, id: %s, %d\n",
             byte_array.c_str(), idstring.c_str(), retval));

  return retval;
}

unsigned int MC_SerialGraph::get_NumNodes()
{
  if(!num_nodes){
    find_NumNodes();
  }

  return num_nodes;
}

void MC_SerialGraph::find_NumNodes()
{
  char *cur, *tmp_str, *array_str = strdup(byte_array.c_str());
  num_nodes=0;

  for(cur = strtok_r(array_str, " \t\n%", &tmp_str);
      cur; cur = strtok_r(NULL, " \t\n%", &tmp_str)){
    if(cur[0] != '[' || cur[0] != ']'){
      num_nodes++;
    }
  }
}

unsigned int MC_SerialGraph::get_NumBackends()
{
  if(!num_backends){
    find_NumBackends();
  }

  return num_backends;
}

void MC_SerialGraph::find_NumBackends()
{
  unsigned int i;
  num_backends=0;

  for(i=0; i<byte_array.length(); i++){
    if(byte_array[i] == ':'){
      num_backends++;
    }
  }
}
