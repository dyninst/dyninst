/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#include <stdio.h>
#include <math.h>
#include <sstream>
#include <set>
#include <vector>
#include <time.h>

#include "mrnet/NetworkTopology.h"
#include "mrnet/MRNet.h"
#include "CommunicationNode.h"
#include "utils.h"
#include "config.h"
#include "mrnet/FailureManagement.h"
#include "xplat/Tokenizer.h"

using namespace std;

namespace MRN
{

/***************************************************
 * NetworkTopology
 **************************************************/

//const float NetworkTopology::Node::_depth_weight=0.7;
//const float NetworkTopology::Node::_proximity_weight=0.3;

NetworkTopology::Node::Node( const string & ihostname, Port iport, Rank irank,
                             bool iis_backend )
    : _hostname(ihostname), _port(iport), _rank(irank), _failed(false), _parent(NULL),
      _is_backend(iis_backend), _depth(0), _subtree_height(0), _adoption_score(0)
{
    static bool first_time=true;

    if ( first_time ) {
        first_time = false;
        srand48( time(NULL) );
    }
    _rand_key = drand48();
}

bool NetworkTopology::Node::failed( void ) const 
{
    return _failed;
}

string NetworkTopology::Node::get_HostName( void ) const
{
    return _hostname;
}

Port NetworkTopology::Node::get_Port( void ) const
{
    return _port;
}

Rank NetworkTopology::Node::get_Rank( void ) const
{
    return _rank;
}

const set< NetworkTopology::Node * > &
NetworkTopology::Node::get_Children( void ) const
{
    return _children;
}

unsigned int NetworkTopology::Node::get_NumChildren( void ) const
{
    return _children.size();
}
        
void NetworkTopology::Node::set_Parent( Node * p )
{
    _parent = p;
}

void NetworkTopology::Node::add_Child( Node * c )
{
    _children.insert(c);
}

bool NetworkTopology::Node::is_BackEnd( void ) const
{
    return _is_backend;
}

double NetworkTopology::Node::get_WRSKey( void ) const
{
    return _wrs_key;
}

double NetworkTopology::Node::get_RandKey( void ) const
{
    return _rand_key;
}

double NetworkTopology::Node::get_AdoptionScore( void ) const
{
    return _adoption_score;
}

unsigned int NetworkTopology::Node::get_DepthIncrease( Node * iorphan )
const 
{
    unsigned int depth_increase=0;

    if( iorphan->_subtree_height + 1 > _subtree_height  ) {
        depth_increase = iorphan->_subtree_height + 1 - _subtree_height;
    }

    mrn_dbg(5, mrn_printf( FLF, stderr,
                           "\n\tOrphan[%s:%d] sub tree height: %d\n"
                           "\tNode:[%s:%d] sub tree height: %d.\n"
                           "\tDepth increase: %u\n",
                           iorphan->_hostname.c_str(), iorphan->_rank,
                           iorphan->_subtree_height,
                           _hostname.c_str(), _rank, _subtree_height,
                           depth_increase ));

    return depth_increase;
}

unsigned int NetworkTopology::Node::get_Proximity( Node *iorphan )
{
    set<Node*>::const_iterator iter;

    mrn_dbg(5, mrn_printf( FLF, stderr,
                           "Computing proximity: [%s:%d] <-> [%s:%d] ...\n",
                           iorphan->_hostname.c_str(), iorphan->_rank,
                           _hostname.c_str(), _rank ));

    //Find closest common ascendant to both current node and orphan
    Node * cur_ascendant = this;
    Node * common_ascendant = NULL;
    unsigned int node_ascendant_distance=0;
    do{
        mrn_dbg(5, mrn_printf( FLF, stderr,
                               "Is \"%s:%d\" a common ascendant? ", 
                               cur_ascendant->_hostname.c_str(),
                               cur_ascendant->_rank ));
        if( iorphan->_ascendants.find( cur_ascendant ) 
            != iorphan->_ascendants.end() ) {
            common_ascendant = cur_ascendant;
            mrn_dbg(5, mrn_printf(0,0,0, stderr, "yes!\n"));
            break;
        }
        mrn_dbg(5, mrn_printf(0,0,0, stderr, "no.\n"));
        cur_ascendant = cur_ascendant->_parent;
        node_ascendant_distance++;
    } while( common_ascendant == NULL ); ;

    //Find distance between orphan and common ascendant
    cur_ascendant = iorphan;
    unsigned int orphan_ascendant_distance=0;
    mrn_dbg(5, mrn_printf( FLF, stderr, "Ascend from orphan to common ascendant[%s:%d]:\n"
                           "\t[%s:%d]", 
                           common_ascendant->_hostname.c_str(),
                           common_ascendant->_rank,
                           cur_ascendant->_hostname.c_str(),
                           cur_ascendant->_rank ));

    while( cur_ascendant != common_ascendant ) {
        cur_ascendant = cur_ascendant->_parent;
        mrn_dbg(5, mrn_printf( 0,0,0, stderr, " <- [%s:%d]",
                               cur_ascendant->_hostname.c_str(),
                               cur_ascendant->_rank ));
        orphan_ascendant_distance++;
    }

    mrn_dbg(5, mrn_printf( 0,0,0, stderr, "\n\n"));
    mrn_dbg(5, mrn_printf( FLF, stderr,
                           "\t[%s:%d]<->[%s:%d]: %d\n\t[%s:%d]<->[%s:%d]: %d\n",
                           common_ascendant->_hostname.c_str(),
                           common_ascendant->_rank, _hostname.c_str(), _rank,
                           node_ascendant_distance,
                           common_ascendant->_hostname.c_str(),
                           common_ascendant->_rank,
                           iorphan->_hostname.c_str(), iorphan->_rank,
                           orphan_ascendant_distance ));

    return node_ascendant_distance + orphan_ascendant_distance;
}

void NetworkTopology::compute_AdoptionScores( vector<Node*> iadopters,
                                              Node *iorphan )
{
    compute_TreeStatistics();

    //fprintf( stdout, "computing scores for %u adopters\n", iadopters.size() );
    for( unsigned int i=0; i<iadopters.size(); i++ ) {
        iadopters[i]->compute_AdoptionScore( iorphan, _min_fanout, _max_fanout,
                                             _depth );
    }
}

const float WFANOUT=1.0;
const float WDEPTH=1.0;
const float WPROXIMITY=0.5;
void NetworkTopology::Node::compute_AdoptionScore( Node * iorphan,
                                                   unsigned int imin_fanout,
                                                   unsigned int imax_fanout,
                                                   unsigned int idepth )
{
    mrn_dbg(5, mrn_printf( FLF, stderr,
                           "Computing [%s:%d]'s score for adopting [%s:%d] ...\n",
                           _hostname.c_str(), _rank,
                           iorphan->_hostname.c_str(),
                           iorphan->_rank ));
    //fprintf( stdout, "Computing [%s:%d]'s score for adopting [%s:%d] ...\n",
             //_hostname.c_str(), _rank,
             //iorphan->_hostname.c_str(),
             //iorphan->_rank );
    
    unsigned int depth_increase = get_DepthIncrease( iorphan );
    unsigned int proximity = get_Proximity( iorphan );
    unsigned int fanout = _children.size();

    double fanout_score;

    if( imax_fanout == imin_fanout )
        fanout_score = 1;
    else
        fanout_score = (double)(imax_fanout - fanout) /
            (double)(imax_fanout - imin_fanout);

    double depth_increase_score = (double)( (idepth-1) - depth_increase ) /
        (double)(idepth-1);

    double proximity_score = (double)( 2 * idepth - 1 - proximity ) /
        (double)( 2 * idepth - 1 - 2 ) ;

    _adoption_score =
        (WFANOUT * fanout_score  ) +
        (WDEPTH * depth_increase_score ) +
        (WPROXIMITY * proximity_score );

    //_wrs_key = pow( _rand_key, 1/_adoption_score );
    _wrs_key = pow( drand48(), 1/_adoption_score );

    //fprintf(stdout, "[%s:%d]: (%u: %.2lf) + (%u: %.2lf) (%u: %.2lf): %.2lf (key:%.2lf)\n",
            //_hostname.c_str(), _rank,
            //fanout, fanout_score,
            //depth_increase, depth_increase_score,
            //proximity, proximity_score,
            //_adoption_score, _wrs_key );
    //fprintf( stdout, "\t max_fan: %u, min_fan: %u, depth: %u\n", imin_fanout, imax_fanout, idepth );
}

bool NetworkTopology::Node::remove_Child( NetworkTopology::Node * c ) 
{
    _children.erase( c );

    return true;
}

NetworkTopology::NetworkTopology( Network *inetwork,  string &ihostname, Port iport, 
                                  Rank irank, bool iis_backend /*=false*/ )
    : _network(inetwork),
      _root( new Node( ihostname, iport, irank, iis_backend ) ),
      _router( new Router( inetwork ) )
{
    _nodes[ irank ] = _root;
    mrn_dbg( 3, mrn_printf(FLF, stderr,
                           "Added rank %u to node list (%p) size: %u\n",
                           irank, &_nodes, _nodes.size() ));
}

NetworkTopology::NetworkTopology( Network *inetwork, SerialGraph & isg )
    : _network(inetwork), _root( NULL ), _router( new Router( inetwork ) )
{
    string sg_str = isg.get_ByteArray();
    //fprintf(stderr, "Resetting topology to \"%s\"\n", sg_str.c_str() );
    reset( sg_str );
}

void NetworkTopology::remove_SubGraph( Node * inode )
{
    mrn_dbg_func_begin();
    _sync.Lock();

    //remove all children subgraphs
    set < Node * > ::iterator iter;
    for( iter=inode->_children.begin(); iter!=inode->_children.end(); iter++ ){
        remove_SubGraph( *iter );
        remove_Node( *iter );
    }

    _sync.Unlock();
    mrn_dbg_func_end();
}

bool NetworkTopology::add_SubGraph( Rank irank, SerialGraph & isg, bool iupdate /*=false*/)
{
    _sync.Lock();
    bool retval;

    Node * node = find_Node( irank );

    if( node == NULL ) {
        retval = false;
    }

    retval = add_SubGraph( node, isg, false );

    if( iupdate )
        _router->update_Table();

    _sync.Unlock();
    return retval;
}

bool NetworkTopology::add_SubGraph( Node * inode, SerialGraph & isg, bool iupdate )
{
    mrn_dbg_func_begin();
    mrn_dbg( 5, mrn_printf( FLF, stderr, "Node[%d] adding subgraph \"%s\"\n",
                            inode->get_Rank(), isg.get_ByteArray().c_str() ));

    _parent_nodes.insert( inode );

    //search for root of subgraph
    Node * node = find_Node( isg.get_RootRank() );
    if( node == NULL ) {
        //Not found! allocate
        node = new Node( isg.get_RootHostName(), isg.get_RootPort(), 
                         isg.get_RootRank(), isg.is_RootBackEnd() );
        _nodes[ isg.get_RootRank() ] = node;
    }
    else{
        //Found! Remove old subgraph
        remove_SubGraph( node );
    }

    if( node->is_BackEnd() ) {
        mrn_dbg( 5, mrn_printf( FLF, stderr, "Adding node[%d] as backend\n",
                            node->get_Rank() ));
        _backend_nodes.insert( node );
    }

    mrn_dbg( 5, mrn_printf( FLF, stderr, "Adding node[%d] as child of node[%d]\n",
                            node->get_Rank(), inode->get_Rank() ));
    //add root of new subgraph as child of input node
    set_Parent( node->get_Rank(), inode->get_Rank(), iupdate );

    SerialGraph *cur_sg;
    isg.set_ToFirstChild( );
    for( cur_sg = isg.get_NextChild( ); cur_sg; cur_sg = isg.get_NextChild( ) ) {
        add_SubGraph( node, *cur_sg, iupdate );
    }

    mrn_dbg_func_end();
    return true;
}

NetworkTopology::Node * NetworkTopology::find_Node(Rank irank) const
{
    //mrn_dbg_func_begin();
    _sync.Lock();
    map<Rank, NetworkTopology::Node*>::const_iterator iter =
        _nodes.find( irank );

    //mrn_dbg( 3, mrn_printf(FLF, stderr, "Searching for rank %u ...", irank ));
    if( iter == _nodes.end() ){
        //mrn_dbg( 3, mrn_printf(0,0,0, stderr, "not found!\n"));
        _sync.Unlock();
        mrn_dbg_func_end();
        return NULL;
    }
    //mrn_dbg( 3, mrn_printf(0,0,0, stderr, "found!\n"));
    _sync.Unlock();
    //mrn_dbg_func_end();
    return (*iter).second;
}

bool NetworkTopology::remove_Node( NetworkTopology::Node *inode )
{
    mrn_dbg_func_begin();

    if( _root == inode ){
        _root=NULL;
    }

    //remove me as my parent's child
    if( inode->_parent )
        inode->_parent->remove_Child( inode );

    //remove from orphans, back-ends, parents list
    _nodes.erase( inode->_rank );
    _orphans.erase( inode );
    _backend_nodes.erase( inode );
    _parent_nodes.erase( inode );

    //remove me as my children's parent, and set children as orphans
    set < Node * >::iterator iter;
    for( iter=inode->_children.begin(); iter!=inode->_children.end(); iter++ ){
        if( (*iter)->_parent == inode ) {
            (*iter)->set_Parent( NULL );
            _orphans.insert( (*iter) );
        }
    }

    delete( inode );

    mrn_dbg_func_end();
    return true;
}

bool NetworkTopology::remove_Node(  Rank irank, bool iupdate /* = false */ )
{
    _sync.Lock();

    NetworkTopology::Node *node_to_remove = find_Node( irank );

    if( node_to_remove == NULL ){
        _sync.Unlock();
        return false;
    }

    node_to_remove->_failed = true;

    bool retval = remove_Node( node_to_remove );

    if( iupdate )
        _router->update_Table();

    _sync.Unlock();
    return retval;
}

bool NetworkTopology::set_Parent( Rank ichild_rank, Rank inew_parent_rank, bool iupdate /*=false*/ )
{
    mrn_dbg_func_begin();
    _sync.Lock();

    NetworkTopology::Node *child_node = find_Node( ichild_rank );
    if( child_node == NULL ){
        _sync.Unlock();
        mrn_dbg_func_end();
        return false;
    }

    NetworkTopology::Node *new_parent_node = find_Node( inew_parent_rank );
    if( new_parent_node == NULL ){
        _sync.Unlock();
        mrn_dbg_func_end();
        return false;
    }

    if( child_node->_parent != NULL ) {
        child_node->_parent->remove_Child( child_node );
    }

    child_node->set_Parent( new_parent_node );
    new_parent_node->add_Child( child_node );
    remove_Orphan( child_node->get_Rank() );

    child_node->_ascendants = new_parent_node->_ascendants;
    child_node->_ascendants.insert( new_parent_node );

    if( iupdate )
        _router->update_Table();

    _sync.Unlock();
    mrn_dbg_func_end();
    return true;
}

bool NetworkTopology::remove_Orphan( Rank r )
{
    _sync.Lock();
    NetworkTopology::Node * node = find_Node(r);
    if( !node ) {
        _sync.Unlock();
        return false;
    }

    _orphans.erase( node );

    _sync.Unlock();
    return true;
}

void NetworkTopology::print_DOTSubTree( NetworkTopology::Node * inode, FILE * f ) const
{
    set < NetworkTopology::Node * >::iterator iter;

    string parent_str, child_str;
    char rank_str[128];

    sprintf(rank_str, "%u", inode->_rank );
    parent_str = inode->_hostname + string(":")
        + string(rank_str);

    //fprintf(stderr, "Printing node[%d]\n", inode->_rank );
    if( inode->_children.empty() ) {
        //fprintf(stderr, "\"%s\";\n", parent_str.c_str() );
        fprintf(f, "\"%s\";\n", parent_str.c_str() );
    }
    else {
        for( iter = inode->_children.begin();
             iter != inode->_children.end();
             iter++ ) {
            sprintf(rank_str, "%u", (*iter)->_rank );
            child_str = (*iter)->_hostname +
                string(":") + string(rank_str);
            fprintf(f, "\"%s\" -> \"%s\";\n", parent_str.c_str(),
                    child_str.c_str() );
            //fprintf(stderr, "\"%s\" -> \"%s\";\n", parent_str.c_str(),
                    //child_str.c_str() );
            print_DOTSubTree( *iter, f );
        }
    }
}

void NetworkTopology::print_TopologyFile( const char * filename ) const
{
    _sync.Lock();
    map < Rank, NetworkTopology::Node * >::const_iterator iter;
    set < NetworkTopology::Node * >::const_iterator iter2;

    FILE * f = fopen(filename, "w" );
    if( f == NULL ){
        perror("fopen()");
        _sync.Unlock();
        return;
    }

    for( iter = _nodes.begin(); iter != _nodes.end(); iter++ ) {
        if( !(*iter).second->_children.empty() ) {
            fprintf( f, "%s:%u => \n", (*iter).second->get_HostName().c_str(),
                     (*iter).second->get_Rank() );

            for( iter2 = (*iter).second->_children.begin();
                 iter2 != (*iter).second->_children.end();
                 iter2++ ) {
                fprintf( f, "\t%s:%u\n", (*iter2)->get_HostName().c_str(),
                         (*iter2)->get_Rank() );
            }
            fprintf( f, "\t;\n\n" );
        }
    } 

    fclose( f );
    _sync.Unlock();
}

void NetworkTopology::print_DOTGraph( const char * filename ) const
{
    _sync.Lock();
    set < NetworkTopology::Node * >::const_iterator iter;

    FILE * f = fopen(filename, "w" );
    if( f == NULL ){
        perror("fopen()");
        _sync.Unlock();
        return;
    }

    fprintf( f, "digraph G{\n" );

    print_DOTSubTree( _root, f );

    for( iter = _orphans.begin(); iter != _orphans.end(); iter++ ) {
        print_DOTSubTree( *iter, f);
    } 

    fprintf( f, "}\n" );
    fclose( f );
    _sync.Unlock();
}

void NetworkTopology::print( FILE * f ) const
{
    _sync.Lock();
    map < Rank, NetworkTopology::Node * >::const_iterator iter;

    string cur_parent_str, cur_child_str;
    char rank_str[128];

    mrn_dbg(5, mrn_printf(0,0,0, f, "\n***NetworkTopology***\n" ));
    for( iter = _nodes.begin(); iter != _nodes.end(); iter++ ) {
        NetworkTopology::Node * cur_node = (*iter).second;

        //Only print on lhs if root or have children
        if( cur_node == _root || !(cur_node->_children.empty()) ){
            sprintf(rank_str, "%u", cur_node->_rank);
            cur_parent_str = cur_node->_hostname + string(":")
                + string(rank_str);
            mrn_dbg(5, mrn_printf(0,0,0, f, "%s =>\n", cur_parent_str.c_str() ));

            set < Node * > ::iterator set_iter;
            for( set_iter=cur_node->_children.begin();
                 set_iter!=cur_node->_children.end(); set_iter++ ){
                sprintf(rank_str, "%u", (*set_iter)->_rank);
                cur_child_str = (*set_iter)->_hostname +
                    string(":") + string(rank_str);
                mrn_dbg(5, mrn_printf(0,0,0, f, "\t%s\n", cur_child_str.c_str() ) );
            }
            mrn_dbg(5, mrn_printf(0,0,0, f, "\n"));
        } 
    }

    _sync.Unlock();
}

char * NetworkTopology::get_TopologyStringPtr( )
{
    _sync.Lock();
    _serial_graph="";

    serialize( _root );

    char * retval = strdup( _serial_graph.get_ByteArray().c_str() );
    _sync.Unlock();

    return retval;
}

char * NetworkTopology::get_LocalSubTreeStringPtr( )
{
    _sync.Lock();

    _serial_graph="";
    serialize( _root );

    ostringstream my_subtree;
    my_subtree << "["
               << _network->get_LocalHostName() << ":"
               << _network->get_LocalPort() << ":"
               << _network->get_LocalRank() << ":";

    string sgba = _serial_graph.get_ByteArray();
    const char * buf = sgba.c_str();
    
    size_t begin, end, cur;    
    begin = sgba.find(my_subtree.str());
    assert( begin != string::npos ); 

    cur=begin;
    end=1;
    int num_leftbrackets=1, num_rightbrackets=0;
    while(num_leftbrackets != num_rightbrackets){
        cur++, end++;
        if( buf[cur] == '[')
            num_leftbrackets++;
        else if( buf[cur] == ']')
            num_rightbrackets++;
    }

    char * retval = strdup( _serial_graph.get_ByteArray()
                            .substr(begin, end).c_str() );
    
    mrn_dbg( 5, mrn_printf(FLF, stderr, "returned:\"%s\"\n", retval ));

    _sync.Unlock();

    return retval;
}

void NetworkTopology::serialize(Node * inode)
{
    _sync.Lock();
    if( inode->is_BackEnd() ){
        // Leaf node, just add my name to serial representation and return
        _serial_graph.add_Leaf( inode->get_HostName(), inode->get_Port(),
                                inode->get_Rank() );
        _sync.Unlock();
        return;
    }
    else{
        //Starting new sub-tree component in graph serialization:
        _serial_graph.add_SubTreeRoot(inode->get_HostName(), inode->get_Port(),
                                      inode->get_Rank() );
    }

    set < Node * > ::iterator iter;
    for( iter=inode->_children.begin(); iter!=inode->_children.end(); iter++ ){
        serialize( *iter );
    }

    //Ending sub-tree component in graph serialization:
    _serial_graph.end_SubTree();
    _sync.Unlock();
}

set<NetworkTopology::Node*> NetworkTopology::get_BackEndNodes( void ) const
{
    return _backend_nodes;
}

set<NetworkTopology::Node*> NetworkTopology::get_ParentNodes( void ) const
{
    return _parent_nodes;
}

set<NetworkTopology::Node*> NetworkTopology::get_OrphanNodes( void ) const
{
    return _orphans;
}

unsigned int NetworkTopology::Node::find_SubTreeHeight( void )
{
    if( _children.empty() ) {
        _subtree_height = 0;
    }
    else{
        unsigned int max_height=0, cur_height;

        set < Node * > ::iterator iter;
        for( iter=_children.begin(); iter!=_children.end(); iter++ ){
            cur_height = (*iter)->find_SubTreeHeight( );
            max_height = (cur_height > max_height ? cur_height : max_height);
        }
        _subtree_height = max_height+1;
    }

    return _subtree_height;
}

struct lt_rank {
    bool operator()(const NetworkTopology::Node* n1, const NetworkTopology::Node* n2) const
    {
        return n1->get_Rank() < n2->get_Rank();
    }
};

struct lt_random {
    bool operator()(const NetworkTopology::Node* n1, const NetworkTopology::Node* n2) const
    {
        if( fabs(n1->get_RandKey() - n2->get_RandKey()) < .000001 )
            return n1->get_Rank() > n2->get_Rank();

        return n1->get_RandKey() > n2->get_RandKey();
    }
};

struct lt_wrs {
    bool operator()(const NetworkTopology::Node* n1, const NetworkTopology::Node* n2) const
    {
        if( fabs(n1->get_WRSKey() - n2->get_WRSKey()) < .000001 )
            return n1->get_Rank() > n2->get_Rank();
        return n1->get_WRSKey() > n2->get_WRSKey();
    }
};

struct lt_weight {
    bool operator()(const NetworkTopology::Node* n1, const NetworkTopology::Node* n2) const
    {
        if( fabs(n1->get_AdoptionScore() - n2->get_AdoptionScore()) < .000001 )
            return n1->get_Rank() > n2->get_Rank();
        return n1->get_AdoptionScore() > n2->get_AdoptionScore();
    }
};

NetworkTopology::Node * NetworkTopology::find_NewParent( Rank ichild_rank,
                                                         unsigned int inum_attempts,
                                                         ALGORITHM_T ialgorithm )
{
    mrn_dbg_func_begin();
    _sync.Lock();
    static vector<Node*> potential_adopters;
    Node * adopter=NULL;

    if( inum_attempts > 0 ) {
        //previously computed list, but failed to contact new parent

        if( inum_attempts >= potential_adopters.size() ) {
            return NULL;
        }

        adopter = potential_adopters[ inum_attempts ];

        mrn_dbg(5, mrn_printf(FLF, stderr, "Returning: node[%d]:%s:%d\n",
                                  adopter->get_Rank(),
                                  adopter->get_HostName().c_str(),
                                  adopter->get_Port() ));
        return adopter;
    }

    Node * orphan = find_Node( ichild_rank );
    assert( orphan );

    //compute list of potential adopters
    potential_adopters.clear();
    find_PotentialAdopters( orphan, _root, potential_adopters );
    if( potential_adopters.empty() ) {
        mrn_dbg(5, mrn_printf(FLF, stderr, "No Adopters left :(\n"));
        exit(-1);
    }

    if( ialgorithm == ALG_RANDOM ) {
        //randomly choose a parent
        sort( potential_adopters.begin(), potential_adopters.end(), lt_random() );
        adopter = potential_adopters[0];
    }
    else if( ialgorithm == ALG_WRS ) {
        //use weighted random selection to choose a parent
        compute_AdoptionScores( potential_adopters, orphan );
        sort( potential_adopters.begin(), potential_adopters.end(), lt_wrs() );
        adopter = potential_adopters[0];
    }
    else if ( ialgorithm == ALG_SORTED_RR ) {
        //use sorted round robin to choose a parent
        compute_AdoptionScores( potential_adopters, orphan );
        sort( potential_adopters.begin(), potential_adopters.end(), lt_weight() );

        //get sorted vector of siblings
        vector< Node * > siblings;
        set< Node * >::const_iterator iter;
        for( iter=orphan->_parent->_children.begin(); 
             iter!=orphan->_parent->_children.end();  iter++ ){
            siblings.push_back( *iter );
        }
        sort( siblings.begin(), siblings.end(), lt_rank() );

        //find local rank in siblings list
        unsigned int my_idx=0;
        for( unsigned int i=0; i<siblings.size(); i++ ) {
            if( siblings[i]->get_Rank() == ichild_rank ) {
                my_idx = i;
                break;
            }
        }

        adopter = potential_adopters[ my_idx % potential_adopters.size() ];
    }


    //fprintf(stderr, "%u potential_adopters:\n", potential_adopters.size() );
    //for( unsigned int i=0; i<potential_adopters.size(); i++ ){
    //fprintf( stderr, "\tnode[%u]: k=%lf, score:%lf\n",
    //potential_adopters[i]->get_Rank(),
    //potential_adopters[i]->get_WRSKey(),
    //potential_adopters[i]->get_AdoptionScore() );
    //}

    if( adopter ) {
        mrn_dbg(5, mrn_printf(FLF, stderr, "Returning: node[%d]:%s:%d\n",
                              adopter->get_Rank(),
                              adopter->get_HostName().c_str(),
                              adopter->get_Port() ));
    }

    _sync.Unlock();
    mrn_dbg_func_end();
    return adopter;
}

void NetworkTopology::find_PotentialAdopters( Node * iorphan,
                                              Node * ipotential_adopter,
                                              vector<Node*> &opotential_adopters )
{
    _sync.Lock();
    mrn_dbg_func_begin();

    mrn_dbg(5, mrn_printf(FLF, stderr,
                          "Can Node[%d]:%s:%d (%s) adopt Orphan[%d]:%s:%d ...",
                          ipotential_adopter->get_Rank(),
                          ipotential_adopter->get_HostName().c_str(),
                          ipotential_adopter->get_Port(),
                          ( ipotential_adopter->is_BackEnd() ? "be" : "int" ),
                          iorphan->get_Rank(),
                          iorphan->get_HostName().c_str(),
                          iorphan->get_Port() ));
    
    //fprintf(stderr, "Can Node[%d]:%s:%d (%s) adopt Orphan[%d]:%s:%d ...",
            //ipotential_adopter->get_Rank(),
            //ipotential_adopter->get_HostName().c_str(),
            //ipotential_adopter->get_Port(),
            //( ipotential_adopter->is_BackEnd() ? "be" : "int" ),
            //orphan->get_Rank(),
            //orphan->get_HostName().c_str(),
            //orphan->get_Port() );
    //stop at backends or the orphaned node's parent
    if( ( iorphan->_parent == ipotential_adopter ) ||
        ipotential_adopter->is_BackEnd() ) {
        _sync.Unlock();
        mrn_dbg(5, mrn_printf(0,0,0, stderr, "no!\n" ));
        //fprintf(stderr, "no!\n" );
        return;
    }

    //fprintf(stderr, "yes!\n" );
    mrn_dbg(5, mrn_printf(0,0,0, stderr, "yes!\n" ));
    
    opotential_adopters.push_back( ipotential_adopter );

    set < Node * > ::iterator iter;
    for( iter=ipotential_adopter->_children.begin();
         iter!=ipotential_adopter->_children.end(); iter++ ){
        find_PotentialAdopters( iorphan, *iter, opotential_adopters );
    }
    _sync.Unlock();
    mrn_dbg_func_end();
}

bool NetworkTopology::reset( string itopology_str, bool iupdate /* =true */ )
{
    _sync.Lock();
    mrn_dbg( 5, mrn_printf( FLF, stderr, "Reseting topology to \"%s\"\n",
                            itopology_str.c_str() ));
    _serial_graph = SerialGraph( itopology_str );
    _root = NULL;

    map<Rank, Node*>::iterator iter = _nodes.begin();
    for( ; iter != _nodes.end(); iter++ )
        delete iter->second ;

    _nodes.clear();
    _orphans.clear();
    _backend_nodes.clear();

    if( itopology_str == "" ) {
        _sync.Unlock();
        return true;
    }

    _serial_graph.set_ToFirstChild( );
    
    mrn_dbg( 5, mrn_printf( FLF, stderr, "Root: %s:%d:%d\n",
                            _serial_graph.get_RootHostName().c_str(),
                            _serial_graph.get_RootRank(),
                            _serial_graph.get_RootPort() ));

    _root = new Node( _serial_graph.get_RootHostName(),
                      _serial_graph.get_RootPort(),
                      _serial_graph.get_RootRank(),
                      _serial_graph.is_RootBackEnd() );
    _nodes[ _serial_graph.get_RootRank() ] = _root;

    if( _network )
        _network->set_FailureManager ( new CommunicationNode ( _root->get_HostName(),
                                                               FAILURE_REPORTING_PORT,
                                                               UnknownRank ));

    SerialGraph *cur_sg;
    for( cur_sg = _serial_graph.get_NextChild( ); cur_sg;
         cur_sg = _serial_graph.get_NextChild( ) ) {
        if( !add_SubGraph( _root, *cur_sg, false ) ){
            mrn_dbg( 1, mrn_printf(FLF, stderr, "add_SubTreeRoot() failed\n" ));
            _sync.Unlock();
            return false;
        }
    }

    if( iupdate ) {
        if( _network )
            _router->update_Table();
    }
    _sync.Unlock();

    return true;
}

void NetworkTopology::get_Leaves( vector< Node * > &oleaves ) const
{
    // A convenience function for helping with the BE attach case
    if( _root->get_NumChildren() == 0 ) {
        mrn_dbg(3, mrn_printf(FLF, stderr, "adding root node to leaves\n"));
        oleaves.push_back( _root );
    }
    else
        get_LeafDescendants( _root, oleaves ); 
}

void NetworkTopology::get_LeafDescendants( Node *inode,
                                           vector< Node * > &odescendants ) const
{
    _sync.Lock();

    set < Node * > ::iterator iter;
    for( iter=inode->_children.begin(); iter!=inode->_children.end(); iter++ ){
        if( (*iter)->get_NumChildren() )
            get_LeafDescendants( (*iter), odescendants );
        else {
            mrn_dbg(3, mrn_printf(FLF, stderr, "adding leaf node[%d] to descendants\n",
                                  (*iter)->get_Rank() ));
            odescendants.push_back( (*iter) );
        }
    }

    _sync.Unlock();
}

void NetworkTopology::get_Descendants( Node *inode,
                                       vector< Node * > &odescendants ) const
{
    _sync.Lock();

    set < Node * > ::iterator iter;
    for( iter=inode->_children.begin(); iter!=inode->_children.end(); iter++ ){
        mrn_dbg(3, mrn_printf(FLF, stderr, "adding node[%d] to descendants\n",
                              (*iter)->get_Rank() ));
        odescendants.push_back( (*iter) );

        get_Descendants( (*iter), odescendants );
    }

    _sync.Unlock();
}

unsigned int NetworkTopology::get_TreeDepth() const
{
    return _root->find_SubTreeHeight();
}

void NetworkTopology::compute_TreeStatistics( void )
{
    _sync.Lock();

    _max_fanout=0;
    _depth=0;
    _min_fanout=(unsigned int)-1;

    _depth=_root->find_SubTreeHeight( );

    set< Node * > ::const_iterator iter;
    for( iter=_parent_nodes.begin(); iter!=_parent_nodes.end(); iter++ ) {
        _max_fanout = ( (*iter)->_children.size() > _max_fanout ?
                        (*iter)->_children.size() : _max_fanout );
        _min_fanout = ( (*iter)->_children.size() < _min_fanout ?
                        (*iter)->_children.size() : _min_fanout );
    }

    _avg_fanout = ( (double)(_nodes.size()-1) ) / ( (double)_parent_nodes.size() );

    double diff=0,sum_of_square=0;
    for( iter=_parent_nodes.begin(); iter!=_parent_nodes.end(); iter++ ) {
        diff = _avg_fanout - (*iter)->_children.size();
        sum_of_square += (diff*diff);
    }

    _var_fanout = sum_of_square / _parent_nodes.size();
    
    _stddev_fanout = sqrt( _var_fanout );
    _sync.Unlock();
}

void NetworkTopology::get_TreeStatistics( unsigned int &onum_nodes,
                                          unsigned int &odepth,
                                          unsigned int &omin_fanout, 
                                          unsigned &omax_fanout,
                                          double &oavg_fanout,
                                          double &ostddev_fanout )
{
    compute_TreeStatistics();

    _sync.Lock();

    onum_nodes = _nodes.size();
    odepth = _depth;
    omax_fanout = _max_fanout;
    omin_fanout = _min_fanout;
    oavg_fanout = _avg_fanout;
    ostddev_fanout = _stddev_fanout;

    _sync.Unlock();
}

unsigned int NetworkTopology::get_NumNodes() const
{
    _sync.Lock();

    unsigned int curr_size = _nodes.size();

    _sync.Unlock();

    return curr_size;
}

PeerNodePtr NetworkTopology::get_OutletNode( Rank irank ) const
{
    return _router->get_OutletNode( irank );
}

bool NetworkTopology::node_Failed( Rank irank ) const 
{
    Node * node = find_Node( irank );

    if( !node)
        return true;

    return node->failed();
}

} // namespace MRN
