#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include "Tree.h"

std::map<std::string, Node *> Tree::NodesByName;


Node::Node( std::string n) : _name(n), visited(false)
{
}

void Node::add_Child( Node * n)
{
    std::list<Node*>::iterator iter;

    for( iter=children.begin(); iter!=children.end(); iter++ ){
        if( (*iter) == n ){
            return;
        }
    }

    children.push_back( n );
}

void Node::print_ToFile(FILE *f)
{
    if( children.size() == 0 )
        return;

    fprintf(f, "%s =>", _name.c_str() );

    std::list<Node*>::iterator iter;
    for( iter=children.begin(); iter!= children.end(); iter++){
        fprintf(f, "\n\t%s", (*iter)->name().c_str() );
    }
    fprintf(f, ";\n\n");

    for( iter=children.begin(); iter!= children.end(); iter++){
        (*iter)->print_ToFile( f );
    }
}

unsigned int Node::visit()
{
    int retval;
    unsigned int num_nodes_visited=1;

    if( visited ){
        return 0;
    }

    visited=true;

    std::list<Node*>::iterator iter;

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
    :root(0), _contains_cycle(false), _contains_unreachable_nodes(false)
{
}

Node * Tree::get_Node( std::string & hostname )
{
    Node * node;

    std::map<std::string, Node *>::iterator iter;

    iter = NodesByName.find( hostname );
    if( iter != NodesByName.end() ){
        node = (*iter).second;
    }
    else{
        node = new Node( hostname );
        NodesByName[hostname] = node;
    }

    return node;
}

void Tree::create_TopologyFile( FILE * f )
{
    root->print_ToFile( f );
    //std::map<std::string, Node *>::iterator iter;
    //for( iter=NodesByName.begin(); iter!=NodesByName.end(); iter++){
        //(*iter).second->print_ToFile( f );
    //}
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

BalancedTree::BalancedTree( std::vector<std::string> &hosts, std::string & topology_spec )
    :_fanout(0), _num_leaves(0), _depth(0)
{
    int retval;

    retval = sscanf(topology_spec.c_str(), "%ux%u", &_fanout, &_num_leaves);
    if( retval != 2 ){
        fprintf(stderr, "Bad topology specification for balanced tree: \"%s\"."
                "Should be of format FxN.\n", topology_spec.c_str() );
        exit(-1);
    }

    unsigned int tmp_int = _num_leaves;
    do{
        if( tmp_int % _fanout != 0 ){
            fprintf(stderr, "num_backends(%d) must be a power of fanout(%d)\n",
                    _num_leaves, _fanout);
            exit(-1);
        }
        tmp_int /= _fanout;
        _depth++;
    } while (tmp_int != 1);

    unsigned int pow=1;
    unsigned int num_nodes_needed=0;
    for(unsigned int i=0; i<=_depth; i++){
        num_nodes_needed += pow;
        pow*=_fanout;
    }

    if ( hosts.size() < num_nodes_needed ){
        fprintf( stderr, "Need %d nodes for topology %s. Hostfile has %d\n",
                 num_nodes_needed, topology_spec.c_str(), hosts.size() );
        exit(-1);
    }

    unsigned int next_orphan=1, cur_parent=0;
    Node * cur_parent_node=0;

    fprintf(stderr, "%d backends, %d fanout, %d nodes\n", _num_leaves,
            _fanout, num_nodes_needed );

    for(unsigned int i=1; i<num_nodes_needed; ){
        cur_parent_node = get_Node( hosts[cur_parent++] );
        //fprintf(f, "%s => ", hosts[cur_parent-1]);

        for(unsigned int j=0; j<_fanout; j++){
            cur_parent_node->add_Child( get_Node( hosts[next_orphan++] ) );
            i++;
            //fprintf(f, "%s ", hosts[next_orphan-1]);
        }
        //fprintf(f, ";\n");
    }
}

GenericTree::GenericTree( std::vector<std::string> &hosts,
                          std::string & topology_spec )
{
    bool new_child_spec=false, new_level=false, first_time=true;
    const char * cur_pos_ptr;
    char cur_item[16];
    unsigned int cur_item_pos=0, next_orphan_pos=0, cur_num_children;
    std::vector< Node *> cur_level_nodes, next_level_nodes;
    std::vector< unsigned int > cur_level_num_children;
    Node * cur_node, *next_child;
    unsigned int child_spec_multiplier=1;

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
        for( unsigned int i=0; i<cur_level_num_children.size(); i++ ){
            cur_num_children = cur_level_num_children[ i ];

            if( !root ){
                root = get_Node( hosts[next_orphan_pos++] );
                cur_node = root;
            }
            else{
                cur_node = cur_level_nodes[ i ];
            }

            for( unsigned int j=0; j<cur_num_children; j++ ){
                next_child = get_Node( hosts[next_orphan_pos++] );

                cur_node->add_Child( next_child );
                next_level_nodes.push_back( next_child );
            }
        }
        cur_level_num_children.clear();

        if( *cur_pos_ptr == '\0' ){
            break;
        }
    }
}
