#include <stdio.h>


#include "mrnet/src/NetworkGraph.h"
#include "mrnet/src/CommunicationNode.h"
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

std::string MC_NetworkNode::get_HostName()
{
  return hostname;
}

void MC_NetworkNode::add_Child(MC_NetworkNode * child)
{
  //printf(MCFL, stderr, "Adding %s:%d as child %d of Node %s:%d(%p)\n", 
	     //child->hostname.c_str(), child->port, children.size(),
             //hostname.c_str(), port, this);
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
  //add_Node(root);
  endpoints = new std::vector <MC_EndPoint *>;
}

void MC_NetworkGraph::set_Root(MC_NetworkNode * _root)
{
  root = _root;
}

std::vector <MC_EndPoint *> * MC_NetworkGraph::get_EndPoints()
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
    std::string key = new_node->get_HostName() + std::string(port_str);
    nodes[key] = new_node;
  }
}

MC_NetworkNode * MC_NetworkGraph::get_Root()
{
  return root;
}

MC_NetworkNode * MC_NetworkGraph::find_Node(char * hostname, unsigned short port)
{
  char port_str[128];
  sprintf(port_str, "%d", port);
  std::string key = std::string(hostname) + std::string(port_str);

  std::map<std::string, MC_NetworkNode*>::iterator iter = nodes.find(key);
  if( iter == nodes.end() ){
    return NULL;
  }
  return (*iter).second;
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

  //printf(MCFL, stderr, "Preorder_traversing node %s:%d (%p) ...\n",
             //node->get_HostName().c_str(), node->get_Port(), node );

  if( node->visited() == true ){
    //printf(MCFL, stderr, "Node already visited\n");
    //Should I stop here?
    _has_cycle=true;
  }
  else{
    //printf(MCFL, stderr, "Node's 1st visit\n");
    node->visit();
    visited_nodes++;

    if(node->children.size() == 0){
      // Leaf node, just add my name to the serial representation and return
      node->id = next_leaf_id++;
      serial_graph.add_BackEnd(node->get_HostName(), node->get_Port(),
                               node->id);
      endpoints->push_back(MC_EndPoint::new_EndPoint( node->id,
						 node->get_HostName().c_str(),
						 node->get_Port()));
      return;
    }
    else{
      //Starting new sub-tree component in graph serialization:
      serial_graph.add_SubTreeRoot(node->get_HostName(), node->get_Port());
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

  //printf(MCFL, stderr, "In fully_connected(). visited %d, exist %d\n",
             //visited_nodes, nodes.size() );
  return ( visited_nodes == nodes.size() ) ;
}

int MC_NetworkGraph::get_Size()
{
  return nodes.size();
}


/***************************************************
 * MC_SerialGraph
 **************************************************/
MC_SerialGraph::MC_SerialGraph(const char * _byte_array)
  :byte_array(_byte_array), num_nodes(0), num_backends(0)
{
}

MC_SerialGraph::MC_SerialGraph(std::string _byte_array)
  :byte_array(_byte_array), num_nodes(0), num_backends(0)
{
}

MC_SerialGraph::MC_SerialGraph()
  :num_nodes(0), num_backends(0)
{
}

void MC_SerialGraph::add_BackEnd(std::string hostname, unsigned short port,
                                 unsigned short id)
{
  char id_str[128], port_str[128];
  sprintf(id_str, "%d", id);
  sprintf(port_str, "%d", port);
  byte_array += hostname + ":" + std::string(port_str) + ":" + std::string(id_str) + " ";
  num_nodes++; num_backends++;
}

void MC_SerialGraph::add_SubTreeRoot(std::string hostname, unsigned short port)
{
  char port_str[128];
  sprintf(port_str, "%d", port);
  byte_array += "[ " + hostname + ":" + std::string(port_str) + " ";
  num_nodes++;
}

void MC_SerialGraph::end_SubTree()
{
  byte_array += "] ";
}

void MC_SerialGraph::set_ToFirstChild()
{
  //unsigned int i;
  //Set buf_idx to this positions: [ xxx:0 yyy:1 ...
  //                                       ^
  //for(buf_idx = 3; byte_array[buf_idx-1] != ' '; buf_idx++);
  buf_idx = byte_array.find(' ', 2);
  buf_idx++;


  //printf(MCFL, stderr, "In set_tofirstchild():\n");
  //printf(MCFL, stderr, "byte_array: %s\n", byte_array.c_str());
  //printf(MCFL, stderr, "1st child : ");
  //for(i=0; i<buf_idx; i++){
    //_fprintf((stderr, " "));
  //}
  //_fprintf((stderr, "^\n"));
}

MC_SerialGraph * MC_SerialGraph::get_NextChild()
{
  MC_SerialGraph * retval;
  unsigned int begin, end, cur;
  const char * buf = byte_array.c_str();
  bool leaf_node=false;

  //printf(MCFL, stderr, "In get_nextchild():\n");
  //printf(MCFL, stderr, "byte_array: %s\n", byte_array.c_str());
  //printf(MCFL, stderr, "    child : ");
  //for(i=0; i<buf_idx; i++){
    //_fprintf((stderr, " "));
  //}
  //_fprintf((stderr, "^\n"));

  //printf(MCFL, stderr, "buf_idx: %d, array_len: %d\n", buf_idx, byte_array.length());
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

  //printf(MCFL, stderr, "get_nextchild() returning:");  retval->print();
  return retval;
}

bool MC_SerialGraph::has_children()
{
  int num_colons=0;

  //does first host have ":port:id" (internal nodes have ":port" suffix only)
  for(int idx=2; byte_array[idx] != ' '; idx++){
    if(byte_array[idx] == ':'){
      num_colons++;
    }
  }

  if(num_colons == 2){
    return false;
  }
  else{
    return true;
  }
}

std::string MC_SerialGraph::get_RootName()
{
  std::string retval;
  int begin=2, end=1; //Byte array begins [ xxx ...

  //find first ':'
  end = byte_array.find(':', begin);
  assert (end != -1);
  //while(byte_array[cur+1] != ':'){
    //cur++; end++;
  //}

  retval = byte_array.substr(begin, end-begin);
  //printf(MCFL, stderr, "In get_rootname(). array: %s, root %s\n",
	     //byte_array.c_str(), retval.c_str());

  return retval;
}

unsigned short MC_SerialGraph::get_RootPort()
{
  int begin, end;
  unsigned short retval;

  begin = byte_array.find(':', 2);
  assert( begin != -1);
  begin++;
  end = byte_array.find(' ', begin);
  std::string port_string = byte_array.substr(begin, end-begin);
  retval = atoi(port_string.c_str());
  //printf(MCFL, stderr, "In get_port(). array: %s, port: %d\n",
	     //byte_array.c_str(), retval);
  return retval;
}

std::string MC_SerialGraph::get_ByteArray()
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
  int begin=0, end=1; //Byte array begins [ xxx ...

  //find 2nd ':'
  begin = byte_array.find(':', begin);
  assert(begin != -1);
  begin++;
  begin = byte_array.find(':', begin);
  assert(begin != -1);
  begin++;
  end = byte_array.find(' ', begin);

  std::string idstring = byte_array.substr(begin, end-begin);

  retval = atoi(idstring.c_str());
  //printf(MCFL, stderr, "In get_Id(). byte_array: %s, id: %s, %d\n",
             //byte_array.c_str(), idstring.c_str(), retval);

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
