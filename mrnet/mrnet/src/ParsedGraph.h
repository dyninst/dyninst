/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#if !defined(ParsedGraph_h)
#define ParsedGraph_h

#include <vector>
#include <map>
#include <string>
#include <stdio.h>

#include "utils.h"
#include "Error.h"
#include "SerialGraph.h"

namespace MRN
{

class ParsedGraph;
extern ParsedGraph * parsed_graph;

class ParsedGraph: public Error {
  public:
    class Node {
        friend class ParsedGraph;
    private:
        std::string _hostname;
        Rank _local_rank;
        Rank _rank;
        std::vector < Node * > _children;
        Node * _parent;
        bool _visited;
        
    public:
        Node( const char *ihostname, Rank ilocal_rank )
            :_hostname(ihostname), _local_rank(ilocal_rank),
            _rank(UnknownRank), _parent(NULL), _visited(false) {}
        std::string const& get_HostName( ) const { return _hostname; }
        Rank get_LocalRank( ) const { return _local_rank; }
        Rank get_Rank( ) const { return _rank; }
        bool visited( ) const { return _visited; }
        void visit( ) { _visited=true; }
        void add_Child( Node * c ) { _children.push_back(c); }
        void remove_Child( Node * c ) ;
        void set_Parent( Node * p ) { _parent = p; };
        
        void print_Node( FILE *, unsigned int idepth );
    };

 private:
    Node * _root;
    std::map < std::string, Node * > _nodes;
    bool _fully_connected;
    bool _cycle_free;

    unsigned int preorder_traversal( Node * );
    void serialize( Node * );

    static Rank _next_node_rank;
    SerialGraph _serial_graph;

 public:
    ParsedGraph( )
        : _root(NULL), _fully_connected(true), _cycle_free(true) { }
    void set_Root( Node * r ) { _root = r; }
    Node *get_Root( ) const  { return _root; }
    Node *find_Node( char *hostname, Rank ilocal_rank );
    bool validate( );
    void add_Node( Node * );
    int get_Size ( ) const { return _nodes.size(); }
    void assign_NodeRanks( bool iassign_backend_ranks );

    std::string get_SerializedGraphString( );
    SerialGraph & get_SerializedGraph( );

    void print_Graph( FILE *f ) { _root->print_Node(f, 0); }
    void print_DOTGraph( const char * filename );
};

}                               // namespace MRN

#endif                          /* ParsedGraph_h */
