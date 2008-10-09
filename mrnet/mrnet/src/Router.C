#include "Router.h"
#include "PeerNode.h"
#include "mrnet/NetworkTopology.h"
#include "mrnet/MRNet.h"

#include <assert.h>

namespace MRN {
bool Router::update_Table()
{
    _sync.Lock();
    mrn_dbg_func_begin();

    NetworkTopology * net_topo = _network->get_NetworkTopology( );

    //clear the map
    _table.clear();

    //get local node from network topology
    NetworkTopology::Node * local_node = net_topo->
        find_Node( _network->get_LocalRank() );

    mrn_dbg(5, mrn_printf(FLF, stderr, "local_node: %p\n", local_node ));

    //for each child, get descendants and put child as outlet node
    std::set< NetworkTopology::Node * > children;
    std::vector< NetworkTopology::Node * > descendants;

    children = local_node->get_Children();

    std::set< NetworkTopology::Node * >::iterator iter;
    for( iter=children.begin(); iter!=children.end(); iter++ ){
        mrn_dbg(5, mrn_printf(FLF, stderr, "Looking up peer node[%d]\n",
                              (*iter)->get_Rank() ));
        PeerNodePtr cur_outlet = _network->get_PeerNode( (*iter)->get_Rank() );
        if( cur_outlet == PeerNode::NullPeerNode ) {
            mrn_dbg(5, mrn_printf(FLF, stderr, "PeerNode[%d] doesn't exist -- likely failed\n",
                                  (*iter)->get_Rank() ));
            continue;
        }

        _table[ (*iter)->get_Rank() ] = cur_outlet;

        mrn_dbg(5, mrn_printf(FLF, stderr, "Getting descendants of node[%d]\n",
                              (*iter)->get_Rank() ));
        net_topo->get_Descendants( (*iter), descendants );

        for( unsigned int j=0; j<descendants.size(); j++ ){
            mrn_dbg(5, mrn_printf(FLF, stderr,
                                  "Setting child[%d] as outlet for node[%d]\n",
                                  (*iter)->get_Rank(),
                                  descendants[j]->get_Rank() ));
            _table[ descendants[j]->get_Rank() ] = cur_outlet;
        }
        descendants.clear();
    }

    _sync.Unlock();
    mrn_dbg_func_end();
    return true;
}

PeerNodePtr Router::get_OutletNode( Rank irank ) const
{
    _sync.Lock();
    std::map< Rank, PeerNodePtr >::const_iterator iter;

    iter = _table.find( irank );

    if( iter == _table.end() ){
        _sync.Unlock();
        return PeerNode::NullPeerNode;
    }

    _sync.Unlock();
    return (*iter).second;
}

}
