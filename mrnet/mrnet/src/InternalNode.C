/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#include <stdio.h>

#include "mrnet/src/InternalNode.h"
#include "mrnet/src/utils.h"

namespace MRN
{

/*======================================================*/
/*  InternalNode CLASS METHOD DEFINITIONS            */
/*======================================================*/
InternalNode::InternalNode( std::string _hostname, Port _port,
                            std::string _phostname, Port _pport )
    :ParentNode( true, _hostname, _port ),
     ChildNode( true, _hostname, _port ),
     CommunicationNode( _hostname, _port )
{
    int retval;
    mrn_printf( 3, MCFL, stderr,
                "In InternalNode: parent_host: %s, parent_port: %d\n",
                _phostname.c_str(  ), _pport );

    upstream_node = new RemoteNode( true, _phostname, _pport );
    RemoteNode::local_child_node = this;
    RemoteNode::local_parent_node = this;

    //printf(3, MCFL, stderr, "Calling connect() ...\n");
    upstream_node->connect(  );
    upstream_node->_is_upstream = true;
    if( upstream_node->fail(  ) ) {
        mrn_printf( 1, MCFL, stderr, "connect() failed\n" );
        return;
    }

    mrn_printf( 3, MCFL, stderr, "Creating Upstream recv thread ...\n" );
    retval = XPlat::Thread::Create( RemoteNode::recv_thread_main,
                                    ( void * )upstream_node,
                                    &( upstream_node->recv_thread_id ) );
    if( retval != 0 ) {
        //Call childnode's error here, because we want to record event
        //locally as well as send upstream
        ChildNode::error( MRN_ESYSTEM, "XPlat::Thread::Create() failed: %s\n",
                          strerror(errno) );
        mrn_printf( 1, MCFL, stderr, "Upstream recv thread creation failed\n" );
    }

    mrn_printf( 3, MCFL, stderr, "Creating Upstream send thread ...\n" );
    retval = XPlat::Thread::Create( RemoteNode::send_thread_main,
                                    ( void * )upstream_node,
                                    &( upstream_node->send_thread_id ) );
    if( retval != 0 ) {
        //Call childnode's error here, because we want to record event
        //locally as well as send upstream
        ChildNode::error( MRN_ESYSTEM, "XPlat::Thread::Create() failed: %s\n",
                          strerror(errno) );
        mrn_printf( 1, MCFL, stderr, "Upstream send thread creation failed\n" );
    }

    mrn_printf( 3, MCFL, stderr, "Leaving InternalNode()\n" );
}

InternalNode::~InternalNode( void )
{
    delete upstream_node;
    std::list < RemoteNode * >::iterator iter;

    for( iter = children_nodes.begin(  ); iter != children_nodes.end(  );
         iter++ ) {
        delete( RemoteNode * ) ( *iter );
    }
}

void InternalNode::waitLoop( )
{
    // TODO what should we base our termination decision on?
    // * whether we have *any* connections remaining?
    // * whether we have an upstream connection?
    // * whether we have any downstream connections?
    //

    // for now, we base our termination on when we lose our upstream connection
    int iret = XPlat::Thread::Join( upstream_node->recv_thread_id, NULL );
    if( iret != 0 ) {
        mrn_printf( 1, MCFL, stderr,
                    "comm_node failed to join with upstream internal receive thread: %d\n",
                    iret );
    }
}

int InternalNode::send_newSubTreeReport( bool status )
{
    unsigned int *backends, i;
    mrn_printf( 3, MCFL, stderr, "In send_newSubTreeReport()\n" );

    backends = new unsigned int[backend_descendant_nodes.size(  )];
    assert( backends );

    std::list < int >::iterator iter;
    mrn_printf( 3, MCFL, stderr, "Creating subtree report from %p: [ ",
                &backend_descendant_nodes );
    for( i = 0, iter = backend_descendant_nodes.begin(  );
         iter != backend_descendant_nodes.end(  ); i++, iter++ ) {
        backends[i] = *iter;
        mrn_printf( 3, 0, 0, stderr, "%d, ", backends[i] );
    }
    mrn_printf( 3, 0, 0, stderr, "]\n" );

    Packet packet( 0, PROT_RPT_SUBTREE, "%d %ad", status,
                   backends, backend_descendant_nodes.size(  ) );
    if( packet.good(  ) ) {
        if( upstream_node->send( packet ) == -1 ||
            upstream_node->flush(  ) == -1 ) {
            mrn_printf( 1, MCFL, stderr, "send/flush failed\n" );
            return -1;
        }
    }
    else {
        mrn_printf( 1, MCFL, stderr, "new packet() failed\n" );
        return -1;
    }

    mrn_printf( 3, MCFL, stderr, "send_newSubTreeReport() succeeded\n" );
    return 0;
}

int InternalNode::proc_DataFromUpStream( Packet& packet )
{
    int retval;

    mrn_printf( 3, MCFL, stderr, "In proc_DataFromUpStream()\n" );

    streammanagerbyid_sync.Lock(  );
    StreamManager *stream_mgr = StreamManagerById[packet.get_StreamId(  )];
    streammanagerbyid_sync.Unlock(  );

    std::vector < Packet >packets;
    stream_mgr->push_packet( packet, packets, false );  // packet going downstream

    if( !packets.empty(  ) ) {
        // deliver all packets to all downstream nodes
        for( unsigned int i = 0; i < packets.size(  ); i++ ) {
            Packet cur_packet = packets[i];

            std::list < RemoteNode * >::iterator iter;
            unsigned int j;
            for( j = 0, iter = stream_mgr->downstream_nodes.begin(  );
                 iter != stream_mgr->downstream_nodes.end(  );
                 iter++, j++ ) {

                mrn_printf( 3, MCFL, stderr,
                            "Calling node_set[%d(%p)].send() ...\n", j,
                            *iter );
                if( ( *iter )->send( cur_packet ) == -1 ) {
                    mrn_printf( 1, MCFL, stderr,
                                "node_set.send() failed\n" );
                    retval = -1;
                    continue;
                }
                mrn_printf( 3, MCFL, stderr,
                            "node_set.send() succeeded\n" );
            }
        }
    }

    mrn_printf( 3, MCFL, stderr,
                "internal.procDataFromUpStream() succeeded\n" );
    return 0;
}

int InternalNode::proc_DataFromDownStream( Packet& packet )
{
    mrn_printf( 3, MCFL, stderr, "In internal.proc_DataFromUpStream()\n" );

    streammanagerbyid_sync.Lock(  );
    StreamManager *stream_mgr = StreamManagerById[packet.get_StreamId(  )];
    streammanagerbyid_sync.Unlock(  );

    std::vector < Packet >packets;

    stream_mgr->push_packet( packet, packets, true );
    if( !packets.empty(  ) ) {
        for( unsigned int i = 0; i < packets.size(  ); i++ ) {
            if( upstream_node->send( packets[i] ) == -1 ) {
                mrn_printf( 1, MCFL, stderr, "upstream.send() failed()\n" );
                return -1;
            }
        }
    }

    mrn_printf( 3, MCFL, stderr,
                "Leaving internal.proc_DataFromUpStream()\n" );
    return 0;
}


/*===================================================*/
/*  LocalNode CLASS METHOD DEFINITIONS            */
/*===================================================*/
int InternalNode::proc_PacketsFromUpStream( std::list < Packet >&packets )
{
    int retval = 0;
    Packet cur_packet;
    StreamManager *stream_mgr;

    mrn_printf( 3, MCFL, stderr, "In proc_PacketsFromUpStream()\n" );

    std::list < Packet >::iterator iter = packets.begin(  );
    for( ; iter != packets.end(  ); iter++ ) {
        cur_packet = ( *iter );
        switch ( cur_packet.get_Tag(  ) ) {
        case PROT_NEW_SUBTREE:
            mrn_printf(3, MCFL, stderr, "Processing PROT_NEW_SUBTREE\n");
            if( proc_newSubTree( cur_packet ) == -1 ) {
                mrn_printf( 1, MCFL, stderr, "proc_newSubTree() failed\n" );
                retval = -1;
            }
            //AT this point, we have created subteee and collected all reports
            //must send reports upwards
            if( send_newSubTreeReport( true ) == -1 ) {
                mrn_printf( 1, MCFL, stderr,
                            "send_newSubTreeReport() failed\n" );
                retval = -1;
            }
            //printf(3, MCFL, stderr, "proc_newsubtree() succeded\n");
            break;
        case PROT_DEL_SUBTREE:
            //printf(3, MCFL, stderr, "Calling proc_delSubTree()\n");
            if( proc_delSubTree( cur_packet ) == -1 ) {
                mrn_printf( 1, MCFL, stderr, "proc_delSubTree() failed\n" );
                retval = -1;
            }
            //printf(3, MCFL, stderr, "proc_delSubTree() succeded\n");
            break;
        case PROT_NEW_APPLICATION:
            //printf(3, MCFL, stderr, "Calling proc_newApplication()\n");
            if( proc_newApplication( cur_packet ) == -1 ) {
                mrn_printf( 1, MCFL, stderr,
                            "proc_newApplication() failed\n" );
                retval = -1;
            }
            //printf(3, MCFL, stderr, "proc_newApplication() succeded\n");
            break;
        case PROT_DEL_APPLICATION:
            //printf(3, MCFL, stderr, "Calling proc_delApplication()\n");
            if( proc_delApplication( cur_packet ) == -1 ) {
                mrn_printf( 1, MCFL, stderr,
                            "proc_delApplication() failed\n" );
                retval = -1;
            }
            //printf(3, MCFL, stderr, "proc_delApplication() succeded\n");
            break;
        case PROT_NEW_STREAM:
            //printf(3, MCFL, stderr, "Calling proc_newStream()\n");
            stream_mgr = proc_newStream( cur_packet );
            if( stream_mgr == NULL ) {
                mrn_printf( 1, MCFL, stderr, "proc_newStream() failed\n" );
                retval = -1;
                break;
            }
            stream_mgr->upstream_node = upstream_node;
            if( send_newStream( cur_packet, stream_mgr ) == -1 ) {
                mrn_printf( 1, MCFL, stderr, "send_newStream() failed\n" );
                retval = -1;
                break;
            }
            break;
        case PROT_NEW_FILTER:
            if( proc_newFilter( cur_packet ) == -1 ) {
                mrn_printf( 1, MCFL, stderr, "proc_newFilter() failed\n" );
                retval = -1;
            }
            break;
        case PROT_DEL_STREAM:
            //printf(3, MCFL, stderr, "Calling proc_delStream()\n");
            if( proc_delStream( cur_packet ) == -1 ) {
                mrn_printf( 1, MCFL, stderr, "proc_delStream() failed\n" );
                retval = -1;
            }
            //printf(3, MCFL, stderr, "proc_delStream() succeded\n");
            break;

        case PROT_GET_LEAF_INFO:
            //printf(3, MCFL, stderr, "Calling proc_getLeafInfo()\n");
            if( proc_getLeafInfo( cur_packet ) == -1 ) {
                mrn_printf( 1, MCFL, stderr,
                            "proc_getLeafInfo() failed\n" );
                retval = -1;
            }
            //printf(3, MCFL, stderr, "proc_getLeafInfo() succeded\n");
            break;

        case PROT_CONNECT_LEAVES:
            if( proc_connectLeaves( cur_packet ) == -1 ) {
                mrn_printf( 1, MCFL, stderr,
                            "proc_connectLeaves() failed\n" );
                retval = -1;
            }
            break;

        default:
            //Any Unrecognized tag is assumed to be data
            //printf(3, MCFL, stderr, "Calling proc_DataFromUpStream(). Tag: %d\n",cur_packet->get_Tag());
            if( proc_DataFromUpStream( cur_packet ) == -1 ) {
                mrn_printf( 1, MCFL, stderr,
                            "proc_DataFromUpStream() failed\n" );
                retval = -1;
            }
            //printf(3, MCFL, stderr, "proc_DataFromUpStream() succeded\n");
            break;
        }
    }

    packets.clear(  );
    mrn_printf( 3, MCFL, stderr, "proc_PacketsFromUpStream() %s",
                ( retval == -1 ? "failed\n" : "succeeded\n" ) );
    return retval;
}

int InternalNode::proc_PacketsFromDownStream( std::list < Packet >&
                                              packet_list )
{
    int retval = 0;
    Packet cur_packet;

    mrn_printf( 3, MCFL, stderr, "In procPacketsFromDownStream()\n" );

    std::list < Packet >::iterator iter = packet_list.begin(  );
    for( ; iter != packet_list.end(  ); iter++ ) {
        cur_packet = ( *iter );
        switch ( cur_packet.get_Tag(  ) ) {
        case PROT_RPT_SUBTREE:
            if( proc_newSubTreeReport( cur_packet ) == -1 ) {
                mrn_printf( 1, MCFL, stderr,
                            "proc_newSubTreeReport() failed\n" );
                retval = -1;
            }
            break;

        case PROT_EVENT:
            if( proc_Event( cur_packet ) == -1 ){
                mrn_printf( 1, MCFL, stderr, "proc_Event() failed\n" );
                retval = -1;
            }
            if( send_Events( ) == -1 ){
                mrn_printf( 1, MCFL, stderr, "send_Event() failed\n" );
                retval = -1;
            }
            break;

        case PROT_GET_LEAF_INFO:
            if( proc_getLeafInfoResponse( cur_packet ) == -1 ) {
                mrn_printf( 1, MCFL, stderr,
                            "proc_getLeafInfoResponse() failed\n" );
                retval = -1;
            }
            break;

        case PROT_CONNECT_LEAVES:
            if( proc_connectLeavesResponse( cur_packet ) == -1 ) {
                mrn_printf( 1, MCFL, stderr,
                            "proc_connectLeavesResponse() failed\n" );
                retval = -1;
            }
            break;

        default:
            //Any unrecognized tag is assumed to be data
            if( proc_DataFromDownStream( cur_packet ) == -1 ) {
                mrn_printf( 1, MCFL, stderr,
                            "proc_DataFromDownStream() failed\n" );
                retval = -1;
            }
        }
    }

    mrn_printf( 3, MCFL, stderr, "proc_PacketsFromDownStream() %s",
                ( retval == -1 ? "failed\n" : "succeeded\n" ) );
    packet_list.clear(  );
    return retval;
}

int InternalNode::deliverLeafInfoResponse( Packet& pkt )
{
    int ret = 0;

    // deliver the aggregated response to our parent
    if( ( upstream_node->send( pkt ) == -1 ) ||
        ( upstream_node->flush(  ) == -1 ) ) {
        mrn_printf( 1, MCFL, stderr,
                    "failed to deliver response to parent\n" );
    }
    return ret;
}

int InternalNode::deliverConnectLeavesResponse( Packet& pkt )
{
    int ret = 0;

    // deliver the aggregated response to our parent
    if( ( upstream_node->send( pkt ) == -1 ) ||
        ( upstream_node->flush(  ) == -1 ) ) {
        mrn_printf( 1, MCFL, stderr,
                    "failed to deliver response to parent\n" );
    }
    return ret;
}

}                               // namespace MRN
