/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#if !defined(NetworkTopology_h)
#define NetworkTopology_h 1

#include <vector>
#include <set>
#include <map>
#include <string>
#include <stdio.h>
#include <math.h>

#include "mrnet/Types.h"
#include "xplat/Monitor.h"
#include "utils.h"
#include "Error.h"
#include "SerialGraph.h"
#include "Router.h"
#include "PeerNode.h"

namespace MRN
{

typedef enum{ ALG_RANDOM=0, ALG_WRS, ALG_SORTED_RR } ALGORITHM_T;

class Network;
class NetworkTopology: public Error {
    friend class Router;

  public:
    class Node{
        friend class NetworkTopology;
        
    public:
        std::string get_HostName( void ) const;
        Port get_Port( void ) const;
        Rank get_Rank( void ) const;
        const std::set< Node * > & get_Children( void ) const;
        unsigned int get_NumChildren( void ) const;
        unsigned int find_SubTreeHeight( void );
        double get_AdoptionScore( void ) const;
        double get_WRSKey( void ) const;
        double get_RandKey( void ) const;
        bool failed( void ) const ;

    private:
        Node( const std::string &, Port, Rank, bool iis_backend );
        void set_Parent( Node * p );
        void add_Child( Node * c );
        bool remove_Child( Node * c ) ;
        bool is_BackEnd( void ) const;
        unsigned int get_DepthIncrease( Node * ) const ;
        unsigned int get_Proximity( Node * );
        void compute_AdoptionScore( Node *iorphan,
                                    unsigned int imin_fanout,
                                    unsigned int imax_fanout,
                                    unsigned int idepth );

        std::string _hostname;
        Port _port;
        Rank _rank;
        bool _failed;
        std::set < Node * > _children;
        std::set < Node * > _ascendants;
        Node * _parent;
        bool _is_backend;

        unsigned int _depth, _subtree_height;
        double _adoption_score;
        double _rand_key;
        double _wrs_key;
    };

  public:
    NetworkTopology( Network *, SerialGraph & );
    NetworkTopology( Network *, std::string &ihostname, Port iport, Rank irank, 
                     bool iis_backend = false );

    //Topology update operations
    bool add_SubGraph( Rank, SerialGraph &, bool iupdate );
    bool remove_Node( Rank, bool iupdate );
    bool set_Parent( Rank c,  Rank p, bool iupdate );
    bool reset( std::string serial_graph="", bool iupdate=true );

    //Access topology components
    bool node_Failed( Rank irank ) const ;
    PeerNodePtr get_OutletNode( Rank irank ) const;
    char * get_TopologyStringPtr( );
    char * get_LocalSubTreeStringPtr();

    void print_TopologyFile( const char * filename ) const;
    void print_DOTGraph( const char * filename ) const;
    void print( FILE * ) const;

    Node * find_NewParent( Rank ichild_rank, unsigned int iretry=0,
                           ALGORITHM_T algorithm=ALG_WRS );
    void get_TreeStatistics( unsigned int &onum_nodes,
                             unsigned int &odepth,
                             unsigned int &omin_fanout, 
                             unsigned &omax_fanout,
                             double &oavg_fanout,
                             double &ostddev_fanout);
    void compute_TreeStatistics( void );

    unsigned int get_NumNodes() const;
    void get_Leaves( std::vector< Node * > &leaves ) const;

    std::set<NetworkTopology::Node*> get_ParentNodes() const;
    std::set<NetworkTopology::Node*> get_OrphanNodes() const;
    std::set<NetworkTopology::Node*> get_BackEndNodes() const;
    Node * get_Root() { return _root; }

    Node * find_Node( Rank ) const;
    bool remove_Node( Node * );

  private:
    bool remove_Orphan( Rank );
    void remove_SubGraph( Node * inode );

    void get_Descendants( Node *, std::vector< Node * > &odescendants ) const;
    void get_LeafDescendants( Node *inode, 
                              std::vector< Node * > &odescendants ) const;

    unsigned int get_TreeDepth() const;

    void print_DOTSubTree( NetworkTopology::Node * inode, FILE * f ) const;

    void serialize( Node * );
    bool add_SubGraph( Node *, SerialGraph &, bool iupdate );
    void find_PotentialAdopters( Node * iadoptee,
                                 Node * ipotential_adopter,
                                 std::vector<Node*> &oadopters );
    void compute_AdoptionScores( std::vector<Node*> iadopters, Node *iorphan );

    //Data Members
    Network *_network;
    Node *_root;
    Router *_router;
    unsigned int _min_fanout, _max_fanout, _depth;
    double _avg_fanout, _stddev_fanout, _var_fanout;
    std::map< Rank, Node * > _nodes;
    std::set< Node * > _orphans;
    std::set< Node * > _backend_nodes;
    std::set< Node * > _parent_nodes;
    mutable XPlat::Monitor _sync;
    SerialGraph _serial_graph;

    void find_PotentialAdopters( Node * iadoptee,
                                 Node * ipotential_adopter,
                                 std::list<Node*> &oadopters );
};


}                               // namespace MRN

#endif                          /* NetworkTopology_h */
