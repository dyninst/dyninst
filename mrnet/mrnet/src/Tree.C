/****************************************************************************
 * Copyright © 2003-2008 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#include <ctype.h>

#include "mrnet/MRNet.h"
#include "utils.h"

#include "xplat/Tokenizer.h"

using namespace std;

namespace MRN{

Tree::Node::Node(string n) : _name(n), visited(false)
{}


void Tree::Node::add_Child( Tree::Node * n)
{
    children.insert( n );
}

void Tree::Node::print_ToFile(FILE *f)
{
    if( children.size() == 0 )
        return;

    fprintf(f, "%s =>", _name.c_str() );

    set<Tree::Node*>::iterator iter;
    for( iter=children.begin(); iter!= children.end(); iter++){
        fprintf(f, "\n\t%s", (*iter)->name().c_str() );
    }
    fprintf(f, " ;\n\n");

    for( iter=children.begin(); iter!= children.end(); iter++){
        (*iter)->print_ToFile( f );
    }
}

unsigned int Tree::Node::visit()
{
    int retval;
    unsigned int num_nodes_visited=1;

    if( visited ){
        return 0;
    }

    visited=true;

    set<Tree::Node*>::iterator iter;

    for( iter=children.begin(); iter!=children.end(); iter++ ){
        retval = (*iter)->visit();
        if(  retval == 0 ){
            return 0;
        }
        num_nodes_visited += retval;
    }

    return num_nodes_visited;
}

Tree::Tree( )
    :root(0), _contains_cycle(false), _contains_unreachable_nodes(false),
    _fanout(0), _num_leaves(0), _depth(0)
{
}

Tree::Node * Tree::get_Node( string hostname )
{
    Tree::Node * node;

    map<string, Tree::Node *>::iterator iter;

    iter = NodesByName.find( hostname );
    if( iter != NodesByName.end() ){
        node = (*iter).second;
    }
    else{
        node = new Tree::Node( hostname );
        NodesByName[hostname] = node;
    }

    return node;
}

bool Tree::create_TopologyFile( FILE * ifile )
{
    root->print_ToFile( ifile );

    return true;
}

bool Tree::create_TopologyFile( const char * ifilename )
{
    FILE * f = fopen( ifilename, "w" );

    if( f == NULL ) {
        perror( "fopen()" );
        return false;
    }
    root->print_ToFile( f );
    fclose( f );

    return true;
}

void Tree::get_TopologyBuffer( char ** /* buf */ )
{
}

bool Tree::get_HostsFromFile( const char* ifilename, list< string >& hosts )
{
    FILE * f = fopen( ifilename, "r" );
    if( f == NULL ) {
        perror( "fopen()" );
        return false;
    }
    else {
        get_HostsFromFileStream( f, hosts );
        fclose( f );
        return true;
    }
}

void Tree::get_HostsFromFileStream( FILE* ifile, list< string >& hosts )
{
    char cur_host[256];
    while( fscanf( ifile, "%s", cur_host ) != EOF )
        hosts.push_back( cur_host );
}

bool Tree::validate()
{
    bool retval=true;

    unsigned int num_nodes_visited = root->visit() ;
    if( num_nodes_visited == 0 ){
        _contains_cycle = true;
        retval = false;
    }

    if(  num_nodes_visited != NodesByName.size() ){
        _contains_unreachable_nodes = true;
        retval = false;
    }

    return retval;
}

BalancedTree::BalancedTree( string &itopology_spec,
                            list< string > &ihosts,
                            unsigned int ife_procs_per_node  /* =1 */,
                            unsigned int ibe_procs_per_node  /* =1 */,
                            unsigned int iint_procs_per_node /* =1 */ )
{
    _fe_procs_per_node = ife_procs_per_node;
    _be_procs_per_node = ibe_procs_per_node;
    _int_procs_per_node = iint_procs_per_node;
    initialize_Tree( itopology_spec, ihosts );
}

BalancedTree::BalancedTree( string &itopology_spec,
                            string &ihost_file, 
                            unsigned int ife_procs_per_node  /* =1 */,
                            unsigned int ibe_procs_per_node  /* =1 */,
                            unsigned int iint_procs_per_node /* =1 */ )
{
    _fe_procs_per_node = ife_procs_per_node;
    _be_procs_per_node = ibe_procs_per_node;
    _int_procs_per_node = iint_procs_per_node;

    list< string > hosts;
    if( get_HostsFromFile( ihost_file.c_str(), hosts ) )
        initialize_Tree( itopology_spec, hosts );
}

bool BalancedTree::initialize_Tree( string &topology_spec, list< string > &hosts )
{
    bool uniform_fanout=false;
    vector< unsigned int > fanouts;
    vector< vector< Tree::Node*> > nodes_by_level;

    if( topology_spec.find_first_of( "^" ) != string::npos ) {
        uniform_fanout=true;

        if( sscanf(topology_spec.c_str(), "%u^%u", &_fanout, &_depth) != 2 ) {
            fprintf(stderr, "Bad topology specification: \"%s\"."
                "Should be of format Fanout^Depth.\n", topology_spec.c_str() );
            exit(-1);
        }

        for(unsigned int i=0; i<_depth; i++){
            fanouts.push_back( _fanout );
        }
    }
    else {
        XPlat::Tokenizer tok( topology_spec );
        std::string::size_type cur_len;
        const char* delim = "x\n";
        std::string::size_type cur_pos = tok.GetNextToken( cur_len, delim );

        while( cur_pos != std::string::npos ) {
            std::string cur_fanout = topology_spec.substr( cur_pos, cur_len );
            fanouts.push_back( atoi( cur_fanout.c_str() ) );
            cur_pos = tok.GetNextToken( cur_len, delim );
        }
    }

    fprintf( stderr, "%lu hosts for topology: \"%s\" (%u procs/fe, %u procs/be, %u procs/internal )\n",
             (unsigned long)hosts.size(), topology_spec.c_str(), _fe_procs_per_node,
             _be_procs_per_node, _int_procs_per_node );
    list< string >::const_iterator list_iter = hosts.begin();
    if( list_iter == hosts.end() ) {
        fprintf( stderr, "Not enough hosts(%lu) for topology %s\n",
                 (unsigned long)hosts.size(), topology_spec.c_str() );
        exit(-1);
    }
    unsigned int procs_on_cur_host=0;
    char cur_host[256];

    //Process 1st level (root)
    fprintf( stderr, "Processing root ...\n" );
    nodes_by_level.push_back( vector< Tree::Node*>() );
    snprintf( cur_host, sizeof(cur_host), "%s:%u",
              (*list_iter).c_str(), procs_on_cur_host++ );
    nodes_by_level[0].push_back( get_Node( cur_host ) );

    if( procs_on_cur_host >= _fe_procs_per_node ) {
        list_iter++;
        if( list_iter == hosts.end() ) {
            fprintf( stderr, "Not enough hosts(%lu) for topology %s\n",
                     (unsigned long)hosts.size(), topology_spec.c_str() );
            exit(-1);
        }
        procs_on_cur_host=0;
    }

    //Process internal levels
    fprintf( stderr, "Processing internal nodes ...\n" );
    unsigned int nodes_at_cur_level=1;
    for( unsigned int i=0; i<fanouts.size()-1; i++ ) {
        nodes_at_cur_level *= fanouts[i];
        nodes_by_level.push_back( vector< Tree::Node*>() );

        for( unsigned int j=0; j<nodes_at_cur_level; j++ ) {
            if( procs_on_cur_host >= _int_procs_per_node ) {
                list_iter++;
                if( list_iter == hosts.end() ) {
                    fprintf( stderr, "Not enough hosts(%lu) for topology %s\n",
                             (unsigned long)hosts.size(), topology_spec.c_str() );
                    exit(-1);
                }
                procs_on_cur_host=0;
            }

            snprintf( cur_host, sizeof(cur_host), "%s:%u",
                      (*list_iter).c_str(), procs_on_cur_host++ );
            nodes_by_level[ i+1 ].push_back( get_Node( cur_host ) );
        }
    }
    if( procs_on_cur_host >= _int_procs_per_node ) {
        list_iter++;
        if( list_iter == hosts.end() ) {
            fprintf( stderr, "Not enough hosts(%lu) for topology %s\n",
                     (unsigned long)hosts.size(), topology_spec.c_str() );
            exit(-1);
        }
        procs_on_cur_host=0;
    }

    //Process last level (leaves)
    fprintf( stderr, "Processing leaves ...\n" );
    nodes_at_cur_level *= fanouts[ fanouts.size()-1 ];
    nodes_by_level.push_back( vector< Tree::Node*>() );

    for( unsigned int i=0; i<nodes_at_cur_level; i++ ) {
        if( procs_on_cur_host >= _be_procs_per_node ) {
            list_iter++;
            if( list_iter == hosts.end() ) {
                fprintf( stderr, "Not enough hosts(%lu) for topology %s\n",
                         (unsigned long)hosts.size(), topology_spec.c_str() );
                exit(-1);
            }
            procs_on_cur_host=0;
        }

        snprintf( cur_host, sizeof(cur_host), "%s:%u",
                  (*list_iter).c_str(), procs_on_cur_host++ );
        nodes_by_level[ fanouts.size() ].push_back( get_Node( cur_host ) );
    }

    unsigned int next_orphan_idx=0;
    Tree::Node * cur_parent_node=0;
    for( unsigned int i=0; i<nodes_by_level.size()-1; i++ ) {
        next_orphan_idx=0;

        for( unsigned int j=0; j<nodes_by_level[i].size(); j++ ) {
            //assign orphans to each parent at current level
            cur_parent_node = nodes_by_level[i][j];
            if( !root ){
                root = cur_parent_node;
            }

            for(unsigned int k=0; k<fanouts[i]; k++){
                cur_parent_node->add_Child
                    ( nodes_by_level[i+1][next_orphan_idx] );
                next_orphan_idx++;
            }
        }
    }

    return true;
}

KnomialTree::KnomialTree( std::string &itopology_spec, std::list<std::string> &ihosts )
{
    initialize_Tree( itopology_spec, ihosts );
}

KnomialTree::KnomialTree( std::string &itopology_spec, std::string &ihost_file )
{
    list< string > hosts;
    if( get_HostsFromFile( ihost_file.c_str(), hosts ) )
        initialize_Tree( itopology_spec, hosts );
}

bool KnomialTree::initialize_Tree( std::string &topology_spec,
                                   std::list<std::string> &hosts )
{
    map< string, vector<string> > tree;
    unsigned kfactor, num_nodes;

    if( topology_spec.find_first_of( "@" ) != string::npos ) {
        if( sscanf(topology_spec.c_str(), "%u@%u", &kfactor, &num_nodes) != 2 ) {
            fprintf(stderr, "Bad topology specification: \"%s\". "
                "Should be of format Fanout^Depth.\n", topology_spec.c_str() );
            exit(-1);
        }
    }
    else {
        fprintf( stderr, "Bad topology specification: \"%s\". "
                 "Should be of format K_NumNodes.\n", topology_spec.c_str() );
        exit(-1);
    }

    _fanout = kfactor;
    unsigned max_depth = 0;

    /* Algorithm: Host list treated as two sub-lists, USED and AVAIL. USED is
                  initialized to the first host (the root), AVAIL to the rest.
                  While we still have available hosts and haven't reached the 
                  desired total number of nodes, each member of USED will be
                  assigned k-1 new children from AVAIL. After each round of 
                  adding children from AVAIL to the members of USED, the size
                  of USED is effectively multiplied by k.
    */

    unsigned curr_step_nodes = 1;

    list< string >::iterator avail_iter = hosts.begin();
    if( !root )
        root = get_Node( *avail_iter );
    avail_iter++;


    while( avail_iter != hosts.end() && 
           curr_step_nodes < num_nodes ) {

        max_depth++;

        list< string >::iterator used_iter = hosts.begin();
        list< string >::iterator stop_iter = avail_iter;

        for( ; (used_iter != stop_iter) && (curr_step_nodes < num_nodes); used_iter++) {
            vector< string >& children = tree[ *used_iter ];
            for(int i=0; (i < kfactor-1) && 
                         (avail_iter != hosts.end()) && 
                         (curr_step_nodes < num_nodes) ; i++, avail_iter++, curr_step_nodes++) {
                children.push_back( *avail_iter );
            }
        }

    }

    _depth = max_depth;
    _num_leaves = num_nodes - tree.size();

    map< string, vector< string > >::iterator parent = tree.begin();
    for( ; parent != tree.end() ; parent++ ) {
       Tree::Node* pnode = get_Node( parent->first );
       
       vector< string >& children = parent->second;
       vector< string >::iterator child = children.begin();
       for( ; child != children.end(); child++ )
          pnode->add_Child( get_Node(*child) );
    }

    return true;
}

GenericTree::GenericTree( string &itopology_spec,
                          list< string > &ihosts )
{
    initialize_Tree( itopology_spec, ihosts );
}

GenericTree::GenericTree( string &itopology_spec,
                          string &ihost_file )
{
    list< string > hosts;
    if( get_HostsFromFile( ihost_file.c_str(), hosts ) )
        initialize_Tree( itopology_spec, hosts );
}

bool GenericTree::initialize_Tree( string &topology_spec, list< string > &hosts )
{
    bool new_child_spec=false, new_level=false, first_time=true;
    const char * cur_pos_ptr;
    char cur_item[16];
    unsigned int cur_item_pos=0, cur_num_children;
    vector< Tree::Node *> cur_level_nodes, next_level_nodes;
    vector< unsigned int > cur_level_num_children;
    Tree::Node * cur_node, *next_child;
    unsigned int child_spec_multiplier=1;

    list<string>::iterator next_orphan_iter=hosts.begin();

    for( cur_pos_ptr = topology_spec.c_str(); ;
         cur_pos_ptr++ ){
        if( *cur_pos_ptr == ',' ){
            cur_item[ cur_item_pos ] = '\0';
            new_child_spec=true;
            cur_item_pos=0;
        }
        else if( *cur_pos_ptr == ':' ||
                 *cur_pos_ptr == '\0' ){
            cur_item[ cur_item_pos ] = '\0';
            new_child_spec=true;
            new_level=true;
            cur_item_pos=0;
        }
        else if( *cur_pos_ptr == 'x' ){
            cur_item[ cur_item_pos ] = '\0';
            cur_item_pos=0;
            child_spec_multiplier = atoi( cur_item );
        }
        else{
            if( !isdigit( *cur_pos_ptr ) ){
                fprintf(stderr, "Invalid character '%c' in topology "
                        "specification \"%s\".\n",
                        *cur_pos_ptr, topology_spec.c_str() );
                exit(-1);
            }
            cur_item[ cur_item_pos++ ] = *cur_pos_ptr;
        }

        if( new_child_spec || new_level ){
            cur_num_children = atoi( cur_item );
            cur_item_pos = 0;
            new_child_spec = false;

            for(unsigned int i=0; i<child_spec_multiplier; i++){
                cur_level_num_children.push_back( cur_num_children );
            }
            child_spec_multiplier=1;
        }

        if( !new_level ){
            continue;
        }
        new_level=false;

        //at the end of every level, add proper number of children to each
        //parent node
        if( first_time ){
            if( cur_level_num_children.size() != 1 ){
                fprintf(stderr, "Error: Bad topology \"%s\". First level "
                        "of topology tree should only have child\n",
                        topology_spec.c_str() );
                exit(-1);
            }

            first_time = false;
        }

        cur_level_nodes = next_level_nodes;
        next_level_nodes.clear();
        string next_orphan;
        for( unsigned int i=0; i<cur_level_num_children.size(); i++ ){
            cur_num_children = cur_level_num_children[ i ];

            if( !root ){
                next_orphan = *next_orphan_iter;
                root = get_Node( next_orphan );
                next_orphan_iter++;
                cur_node = root;
            }
            else{
                cur_node = cur_level_nodes[ i ];
            }

            for( unsigned int j=0; j<cur_num_children; j++ ){
                next_orphan = *next_orphan_iter;
                next_child = get_Node( next_orphan );
                next_orphan_iter++;

                cur_node->add_Child( next_child );
                next_level_nodes.push_back( next_child );
            }
        }
        cur_level_num_children.clear();

        if( *cur_pos_ptr == '\0' ){
            break;
        }
    }

    return true;
}

} /* namespace MRN */
