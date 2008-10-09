/****************************************************************************
 * Copyright  2003-2008 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#include <stdio.h>

#include "mrnet/Types.h"
#include "ChildNode.h"
#include "InternalNode.h"
#include "BackEndNode.h"
#include "PeerNode.h"
#include "utils.h"
#include "mrnet/NetworkTopology.h"
#include "mrnet/MRNet.h"

namespace MRN
{

/*===================================================*/
/*  ChildNode CLASS METHOD DEFINITIONS               */
/*===================================================*/
ChildNode::ChildNode( Network * inetwork,
                      std::string const& ihostname, Rank irank,
                      std::string const& iphostname, Port ipport, Rank iprank )
    : CommunicationNode(ihostname, UnknownPort, irank),
      _network(inetwork), _incarnation(0)
{
    PeerNodePtr parent =
        _network->new_PeerNode( iphostname, ipport, iprank, true, true );
    _network->set_ParentNode( parent );
}

int ChildNode::proc_PacketsFromParent( std::list < PacketPtr >&packets ) const
{
    int retval = 0;
    PacketPtr cur_packet;

    mrn_dbg( 3, mrn_printf(FLF, stderr, "In proc_Packets()\n" ));
    std::list < PacketPtr >::iterator iter = packets.begin(  );
    for( ; iter != packets.end(  ); iter++ ) {
        cur_packet = ( *iter );
        switch ( cur_packet->get_Tag(  ) ) {

        case PROT_NEW_SUBTREE_RPT:
            assert(0);
            break;

        case PROT_NEW_SUBTREE:
            assert( _network->is_LocalNodeInternal() );
            mrn_dbg(3, mrn_printf(FLF, stderr, "Processing PROT_NEW_SUBTREE\n"));
            if( _network->get_LocalInternalNode()->proc_newSubTree( cur_packet ) == -1 ) {
                mrn_dbg( 1, mrn_printf(FLF, stderr, "proc_newSubTree() failed\n" ));
                retval = -1;
            }
            mrn_dbg(5, mrn_printf(FLF, stderr, "Waiting for subtrees to report ... \n" ));
            if( _network->get_LocalInternalNode()->waitfor_SubTreeReports() == -1 ) {
                mrn_dbg( 1, mrn_printf(FLF, stderr, "waitfor_SubTreeReports() failed\n" ));
                retval = -1;
            }
            mrn_dbg(5, mrn_printf(FLF, stderr, "Subtrees reported\n" ));

            //must send reports upwards
            if( send_NewSubTreeReport( ) == -1 ) {
                mrn_dbg( 1, mrn_printf(FLF, stderr,
                            "send_newSubTreeReport() failed\n" ));
                retval = -1;
            }
            break;

        case PROT_DEL_SUBTREE:
            if( _network->is_LocalNodeParent() ) {
                if( _network->get_LocalParentNode()->proc_DeleteSubTree( cur_packet ) == -1 ) {
                    mrn_dbg( 1, mrn_printf(FLF, stderr, "proc_deleteSubTree() failed\n" ));
                    retval = -1;
                }
            }
            else{
                if( _network->get_LocalBackEndNode()->proc_DeleteSubTree( cur_packet ) == -1 ) {
                    mrn_dbg( 1, mrn_printf(FLF, stderr, "proc_deleteSubTree() failed\n" ));
                    retval = -1;
                }
            }
            break;

        case PROT_NEW_STREAM:
            if( _network->is_LocalNodeInternal() ){
                if( _network->get_LocalInternalNode()-> proc_newStream( cur_packet ) == NULL ){
                    mrn_dbg( 1, mrn_printf(FLF, stderr, "proc_newStream() failed\n" ));
                    retval = -1;
                    break;
                }
            }
            else{
                if( _network->get_LocalBackEndNode()->
                    proc_newStream( cur_packet ) == -1 ) {
                    mrn_dbg( 1, mrn_printf(FLF, stderr, "proc_newStream() failed\n" ));
                    retval = -1;
                    break;
                }
            }
            break;

        case PROT_SET_FILTERPARAMS_UPSTREAM:
            if( _network->is_LocalNodeInternal() ){
                if( _network->get_LocalInternalNode()->proc_UpstreamFilterParams( cur_packet ) == -1 ) {
                    mrn_dbg( 1, mrn_printf(FLF, stderr, "proc_UpstreamFilterParams() failed\n" ));
                    retval = -1;
                    break;
                }
            }
            else{
                if( _network->get_LocalBackEndNode()->proc_UpstreamFilterParams( cur_packet ) == -1 ) {
                    mrn_dbg( 1, mrn_printf(FLF, stderr, "proc_UpstreamFilterParams() failed\n" ));
                    retval = -1;
                    break;
                }
            }
            break;

        case PROT_SET_FILTERPARAMS_DOWNSTREAM:
            if( _network->is_LocalNodeInternal() ){
                if( _network->get_LocalInternalNode()->proc_DownstreamFilterParams( cur_packet ) == -1 ) {
                    mrn_dbg( 1, mrn_printf(FLF, stderr, "proc_DownstreamFilterParams() failed\n" ));
                    retval = -1;
                    break;
                }
            }
            else{
                if( _network->get_LocalBackEndNode()->proc_DownstreamFilterParams( cur_packet ) == -1 ) {
                    mrn_dbg( 1, mrn_printf(FLF, stderr, "proc_DownstreamFilterParams() failed\n" ));
                    retval = -1;
                    break;
                }
            }
            break;

        case PROT_DEL_STREAM:
            if( (_network->is_LocalNodeInternal()) && (_network->get_LocalInternalNode()->proc_deleteStream( cur_packet ) == -1) ) {
                mrn_dbg( 1, mrn_printf(FLF, stderr, "proc_delStream() failed\n" ));
                retval = -1;
            }
            break;

        case PROT_NEW_FILTER:
            if( _network->is_LocalNodeInternal() ){
                if( _network->get_LocalInternalNode()->proc_newFilter( cur_packet ) == -1 ) {
                    mrn_dbg( 1, mrn_printf(FLF, stderr, "proc_newFilter() failed\n" ));
                    retval = -1;
                }
            }
            else {
                if( _network->get_LocalBackEndNode()->proc_newFilter( cur_packet ) == -1 ) {
                    mrn_dbg( 1, mrn_printf(FLF, stderr, "proc_newFilter() failed\n" ));
                    retval = -1;
                }
            }
            break;

        case PROT_FAILURE_RPT:
            if( proc_FailureReportFromParent( cur_packet ) == -1 ){
                mrn_dbg( 1, mrn_printf(FLF, stderr,
                                       "proc_FailureReport() failed\n" ));
                retval = -1;
            }
            break;
        case PROT_NEW_PARENT_RPT:
            if( proc_NewParentReportFromParent( cur_packet ) == -1 ){
                mrn_dbg( 1, mrn_printf(FLF, stderr,
                                       "proc_NewParentReport() failed\n" ));
                retval = -1;
            }
            break;
        case PROT_TOPOLOGY_RPT:
            if( proc_TopologyReport( cur_packet ) == -1 ){
                mrn_dbg( 1, mrn_printf(FLF, stderr,
                                       "proc_TopologyReport() failed\n" ));
                retval = -1;
            }
            break;
        case PROT_RECOVERY_RPT:
            if( proc_RecoveryReport( cur_packet ) == -1 ){
                mrn_dbg( 1, mrn_printf(FLF, stderr,
                                       "proc_RecoveryReport() failed\n" ));
                retval = -1;
            }
            break;
        case PROT_COLLECT_PERFDATA:
            if( proc_CollectPerfData( cur_packet ) == -1 ) {
                mrn_dbg( 1, mrn_printf(FLF, stderr,
                            "proc_CollectPerfData() failed\n" ));
                retval = -1;
            }
            break;
        default:
            //Any Unrecognized tag is assumed to be data
            if( proc_DataFromParent( cur_packet ) == -1 ) {
                mrn_dbg( 1, mrn_printf(FLF, stderr,
                            "proc_Data() failed\n" ));
                retval = -1;
            }
            break;
        }
    }

    packets.clear(  );
    mrn_dbg( 3, mrn_printf(FLF, stderr, "proc_Packets() %s",
                ( retval == -1 ? "failed\n" : "succeeded\n" ) ));
    return retval;
}

int ChildNode::proc_CollectPerfData( PacketPtr ipacket ) const
{
    mrn_dbg_func_begin();

    if( _network->is_LocalNodeParent() ) {
        if( _network->send_PacketToChildren( ipacket ) == -1 ) {
            mrn_dbg( 1, mrn_printf(FLF, stderr, "send_PacketToChildren() failed\n" ));
            return -1;
        }
    }

    _network->collect_PerfData();

    mrn_dbg_func_end();
    return 0;
}

int ChildNode::proc_TopologyReport( PacketPtr ipacket ) const 
{
    char * topology_ptr;
    mrn_dbg_func_begin();

    ipacket->unpack( "%s", &topology_ptr );
    std::string topology = topology_ptr;

    if( !_network->reset_Topology( topology ) ){
        mrn_dbg( 1, mrn_printf(FLF, stderr, "Topology->reset() failed\n" ));
        return -1;
    }

    if( _network->is_LocalNodeParent() ) {
        if( _network->send_PacketToChildren( ipacket ) == -1 ) {
            mrn_dbg( 1, mrn_printf(FLF, stderr, "send_PacketToChildren() failed\n" ));
            return -1;
        }
    }

    mrn_dbg_func_end();
    return 0;
}

int ChildNode::send_EventsToParent( ) const
{
    return -1;

    /*
    int status=0;
    mrn_dbg( 3, mrn_printf(FLF, stderr, "Entering send_Event() ... \n" ));

    while ( Event::have_RemoteEvent() ){
        Event * cur_event = Event::get_NextRemoteEvent();

        Packet packet( 0, PROT_EVENT, "%d %s %s %uhd",
                       cur_event->get_Type(),
                       cur_event->get_Description().c_str(),
                       cur_event->get_HostName().c_str(),
                       cur_event->get_Port() );

        if( !packet.has_Error(  ) ) {
            if( get_ParentNode()->send( packet ) == -1 ||
                get_ParentNode()->flush(  ) == -1 ) {
                mrn_dbg( 1, mrn_printf(FLF, stderr, "send/flush failed\n" ));
                status = -1;
            }
        }
        else {
            mrn_dbg( 1, mrn_printf(FLF, stderr, "new packet() failed\n" ));
            status = -1;
        }
    }

    mrn_dbg( 3, mrn_printf(FLF, stderr, "send_Event() succeeded\n" ));
    return status;
    */
}

void ChildNode::error( ErrorCode e, const char *fmt, ... ) const
{
    char buf[1024];

    va_list arglist;

    MRN_errno = e;
    va_start( arglist, fmt );
    vsprintf( buf, fmt, arglist );
    va_end( arglist );

    //Event * event = Event::new_Event( t, buf );

    //First add event to queue
    //Event::add_Event( *event );

    //Then invoke send_Events() to send upstream
    //send_Events();

    //delete event;
}

int ChildNode::init_newChildDataConnection( PeerNodePtr iparent,
                                            Rank ifailed_rank /* = UnknownRank */ )
{
    mrn_dbg_func_begin();

    // Establish data detection connection w/ new Parent
    if( iparent->connect_DataSocket() == -1 ) {
        mrn_dbg(1, mrn_printf(FLF, stderr, "PeerNode::connect() failed\n"));
        return -1;
    }

    _incarnation++;

    char is_internal_char;
    if( _network->is_LocalNodeInternal() ) {
        is_internal_char = 't';
    }
    else{
        is_internal_char = 'f';
    }

    char * topo_ptr = _network->get_LocalSubTreeStringPtr();

    mrn_dbg( 5, mrn_printf(FLF, stderr, "topology: (%p), \"%s\"\n", topo_ptr, topo_ptr ));
    PacketPtr packet( new Packet( 0, PROT_NEW_CHILD_DATA_CONNECTION,
                                  "%s %uhd %ud %uhd %ud %c %s",
                                  _hostname.c_str(),
                                  _port,
                                  _rank,
                                  _incarnation,
                                  ifailed_rank,
                                  is_internal_char,
                                  topo_ptr ) );
    mrn_dbg(5, mrn_printf(FLF, stderr, "Send initialization info ...\n" ));
    if( iparent->sendDirectly( packet ) ==  -1 ) {
        mrn_dbg( 1, mrn_printf(FLF, stderr, "send/flush() failed\n" ));
        return -1;
    }
    free( topo_ptr );

    //Create send/recv threads
    mrn_dbg(5, mrn_printf(FLF, stderr, "Creating comm threads for parent\n" ));
    iparent->start_CommunicationThreads();

    _network->set_ParentNode( iparent );
    mrn_dbg_func_end();

    return 0;
}

int ChildNode::send_NewSubTreeReport( ) const
{
    mrn_dbg_func_begin();

    char * topo_ptr = _network->get_LocalSubTreeStringPtr();
    PacketPtr packet( new Packet( 0, PROT_NEW_SUBTREE_RPT, "%s", topo_ptr));

    if( !packet->has_Error( ) ) {
        if( _network->get_ParentNode()->send( packet ) == -1 ||
            _network->get_ParentNode()->flush(  ) == -1 ) {
            mrn_dbg( 1, mrn_printf(FLF, stderr, "send/flush failed\n" ));
            return -1;
        }
    }
    else {
        mrn_dbg( 1, mrn_printf(FLF, stderr, "new packet() failed\n" ));
        return -1;
    }

    mrn_dbg_func_end();
    return 0;
}

int ChildNode::request_SubTreeInfo( void ) const 
{
    mrn_dbg_func_begin();
    PacketPtr packet( new Packet( 0, PROT_SUBTREE_INFO_REQ, "%ud", _rank ) );

    if( !packet->has_Error( ) ) {
        if( _network->get_ParentNode()->send( packet ) == -1 ||
            _network->get_ParentNode()->flush(  ) == -1 ) {
            mrn_dbg( 1, mrn_printf(FLF, stderr, "send/flush failed\n" ));
            return -1;
        }
    }
    else {
        mrn_dbg( 1, mrn_printf(FLF, stderr, "new packet() failed\n" ));
        return -1;
    }

    mrn_dbg_func_end();
    return 0;
}

int ChildNode::proc_RecoveryReport( PacketPtr ipacket ) const
{
    mrn_dbg_func_begin();

    Rank child_rank, failed_parent_rank, new_parent_rank;

    ipacket->unpack( "%ud %ud %ud",
                     &child_rank,
                     &failed_parent_rank,
                     &new_parent_rank );

    //remove node, but don't update datastructs since following procedure will
    _network->remove_Node( failed_parent_rank, false );
    _network->change_Parent( child_rank, new_parent_rank );

    //Internal nodes must report parent failure to children
    if( _network->is_LocalNodeInternal() ) {
        if( _network->send_PacketToChildren( ipacket )
            == -1 ){
            mrn_dbg(1, mrn_printf(FLF, stderr, "send_PacketToChildren() failed\n"));
            return -1;
        }
    }

    mrn_dbg_func_end();
    return 0;
}

bool ChildNode::ack_DeleteSubTree( void ) const
{
    mrn_dbg_func_begin();

    PacketPtr packet( new Packet( 0, PROT_DEL_SUBTREE_ACK, "" ));

    if( !packet->has_Error( ) ) {
        if( ( _network->get_ParentNode()->send( packet ) == -1 ) ||
            ( _network->get_ParentNode()->flush(  ) == -1 ) ) {
            mrn_dbg( 1, mrn_printf(FLF, stderr, "send/flush failed\n" ));
            return false;
        }
    }
    else {
        mrn_dbg( 1, mrn_printf(FLF, stderr, "new packet() failed\n" ));
        return false;
    }

    mrn_dbg_func_end();
    return true;
}

int ChildNode::recv_PacketsFromParent(std::list <PacketPtr> &packet_list) const {
    return _network->recv_PacketsFromParent(packet_list);
}

bool ChildNode::has_PacketsFromParent( ) const
{
    return _network->has_PacketsFromParent();
}

} // namespace MRN
