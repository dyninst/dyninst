#if !defined( __router_h )
#define __router_h 1

#include <map>
#include "xplat/Monitor.h"
#include "PeerNode.h"

namespace MRN {

class Network;

class Router{
 public:
    Router( Network * inetwork) :_network(inetwork) {}

    bool update_Table(); //recalculate table based on Topology
    PeerNodePtr get_OutletNode( Rank ) const ;

 private:
    mutable XPlat::Monitor _sync;
    std::map< Rank, PeerNodePtr > _table;
    Network * _network;
};

}

#endif /* __router_h */
