#include <stdio.h>


#include "mrnet/src/NetworkGraph.h"
#include "mrnet/src/CommunicationNode.h"
#include "mrnet/src/utils.h"
#include "src/config.h"

namespace MRN
{

/***************************************************
 * NetworkNode
 **************************************************/
NetworkNode::NetworkNode(char * _hostname, unsigned short _port)
    :hostname(_hostname), port(_port), _visited(false)
{
}

/***************************************************
 * NetworkGraph
 **************************************************/

NetworkGraph::NetworkGraph()
    : root( NULL ),
      nodes( new std::map<std::string, NetworkNode*> ),
      graph_checked(false),
      visited_nodes(0),
      _has_cycle(false),
      endpoints(new std::vector<EndPoint*>)
{
}

void NetworkGraph::add_Node(NetworkNode* new_node)
{
    char port_str[128];
    sprintf(port_str, "%d", new_node->get_Port());

    if(new_node){
        std::string key = new_node->get_HostName() + std::string(port_str);
        assert( nodes != NULL );
        (*nodes)[key] = new_node;
    }
}

NetworkNode * NetworkGraph::find_Node(char * hostname, unsigned short port)
{
    char port_str[128];
    sprintf(port_str, "%d", port);
    std::string key = std::string(hostname) + std::string(port_str);

    assert( nodes != NULL );
    std::map<std::string, NetworkNode*>::iterator iter = nodes->find(key);
    if( iter == nodes->end() ){
        return NULL;
    }
    return (*iter).second;
}

bool NetworkGraph::has_cycle(){
    if(!graph_checked){
        preorder_traversal(root);
        graph_checked=true;
    }

    return _has_cycle;
}

void NetworkGraph::preorder_traversal(NetworkNode * node){
    static int next_leaf_id=0;

    //printf(3, MCFL, stderr, "Preorder_traversing node %s:%d (%p) ...\n",
    //node->get_HostName().c_str(), node->get_Port(), node );

    if( node->visited() == true ){
        //printf(3, MCFL, stderr, "Node already visited\n");
        //Should I stop here?
        _has_cycle=true;
    }
    else{
        //printf(3, MCFL, stderr, "Node's 1st visit\n");
        node->visit();
        visited_nodes++;

        if(node->children.size() == 0){
            // Leaf node, just add my name to the serial representation and return
            node->id = next_leaf_id++;
            serial_graph.add_BackEnd(node->get_HostName(), node->get_Port(),
                                     node->id);
            endpoints->push_back(EndPoint::new_EndPoint( node->id,
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

bool NetworkGraph::fully_connected()
{
    if(!graph_checked){
        preorder_traversal(root);
        graph_checked=true;
    }

    //printf(3, MCFL, stderr, "In fully_connected(). visited %d, exist %d\n",
    //visited_nodes, nodes.size() );
    assert( nodes != NULL );
    return ( visited_nodes == nodes->size() ) ;
}


/***************************************************
 * SerialGraph
 **************************************************/
SerialGraph::SerialGraph(const char * _byte_array)
    :byte_array(_byte_array), num_nodes(0), num_backends(0)
{
}

SerialGraph::SerialGraph(std::string _byte_array)
    :byte_array(_byte_array), num_nodes(0), num_backends(0)
{
}

SerialGraph::SerialGraph()
    :num_nodes(0), num_backends(0)
{
}

void SerialGraph::add_BackEnd(std::string hostname, unsigned short port,
                              unsigned short id)
{
    char id_str[128], port_str[128];
    sprintf(id_str, "%d", id);
    sprintf(port_str, "%d", port);
    byte_array += hostname + ":" + std::string(port_str) + ":" + std::string(id_str) + " ";
    num_nodes++; num_backends++;
}

void SerialGraph::add_SubTreeRoot(std::string hostname, unsigned short port)
{
    char port_str[128];
    sprintf(port_str, "%d", port);
    byte_array += "[ " + hostname + ":" + std::string(port_str) + " ";
    num_nodes++;
}

SerialGraph * SerialGraph::get_NextChild()
{
    SerialGraph * retval;
    unsigned int begin, end, cur;
    const char * buf = byte_array.c_str();
    bool leaf_node=false;

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
        retval = new SerialGraph(byte_array.substr(begin, end));
        //retval->print();
    }
    else{
        retval = new SerialGraph("[ " + byte_array.substr(begin, end) + " ]");
    }

    //printf(MCFL, stderr, "get_nextchild() returning:");  retval->print();
    return retval;
}

bool SerialGraph::has_children()
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

std::string SerialGraph::get_RootName()
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

unsigned short SerialGraph::get_RootPort()
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

int SerialGraph::get_Id(){
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

void SerialGraph::find_NumNodes()
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

unsigned int SerialGraph::get_NumBackends()
{
    if(!num_backends){
        find_NumBackends();
    }

    return num_backends;
}

void SerialGraph::find_NumBackends()
{
    unsigned int i;
    num_backends=0;

    for(i=0; i<byte_array.length(); i++){
        if(byte_array[i] == ':'){
            num_backends++;
        }
    }
}

} // namespace MRN
