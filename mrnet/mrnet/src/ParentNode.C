/****************************************************************************
 * Copyright  2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#include <stdio.h>
#include <set>

#include "mrnet/Types.h"
#include "xplat/Process.h"
#include "xplat/Error.h"
#include "ParentNode.h"
#include "ChildNode.h"
#include "InternalNode.h"
#include "utils.h"
#include "config.h"
#include "SerialGraph.h"
#include "PeerNode.h"
#include "Router.h"
#include "Filter.h"
#include "EventDetector.h"
#include "mrnet/Event.h"
#include "mrnet/NetworkTopology.h"
#include "mrnet/MRNet.h"

namespace MRN
{

/*====================================================*/
/*  ParentNode CLASS METHOD DEFINITIONS            */
/*====================================================*/
ParentNode::ParentNode( Network * inetwork, std::string const& ihostname, Rank irank )
    : CommunicationNode( ihostname, UnknownPort, irank ), _network(inetwork),
      _num_children( 0 ), _num_children_reported( 0 ), listening_sock_fd( 0 )
{
    mrn_dbg( 5, mrn_printf(FLF, stderr, "ParentNode(local[%u]:\"%s:%u\")\n",
                           _rank, _hostname.c_str(), _port ));
    mrn_dbg( 3, mrn_printf(FLF, stderr, "Calling bind_to_port(%d)\n", _port ));
    if( bindPort( &listening_sock_fd, &_port ) == -1 ) {
        mrn_dbg( 1, mrn_printf(FLF, stderr, "bind_to_port() failed\n" ));
        error( MRN_ESYSTEM, "bindPort(%d): %s\n", _port, strerror(errno) );
        return;
    }

    subtreereport_sync.RegisterCondition( ALLNODESREPORTED );
    
    mrn_dbg( 3, mrn_printf(FLF, stderr, "Leaving ParentNode()\n" ));
}


ParentNode::~ParentNode( void )
{
}

int ParentNode::recv_PacketsFromChildren( std::list< PacketPtr >&pkt_list,
                                            bool blocking ) const
{
    int ret = 0;

    mrn_dbg( 2, mrn_printf(FLF, stderr, "In PN::recv_PacketsFromChildren( "
                "blocking=%s)\n", (blocking? "true" : "false") ));

    // add the passed set of remote nodes to the poll set
    fd_set rfds;
    int max_fd = 0;
    FD_ZERO( &rfds );

    const std::set < PeerNodePtr > peers = _network->get_ChildPeers();
    std::set < PeerNodePtr >::const_iterator iter;
    for( iter=peers.begin(); iter!=peers.end(); iter++ ) {
        PeerNodePtr cur_node = *iter;
        assert( cur_node != NULL );
        if( cur_node->has_Error() || cur_node->is_parent() )
            continue;

        int curr_fd = cur_node->get_DataSocketFd();
        FD_SET( curr_fd, &rfds );

        if( curr_fd > max_fd )
        {
            max_fd = curr_fd;
        }
    }

    // check for input on our child connections
    int pollret = 0;
    struct timeval * timeout=NULL;

    if( blocking ) {
        mrn_dbg( 3, mrn_printf(FLF, stderr, "Calling \"blocking\" select"
                    "( timeout=%d )\n", PeerNode::get_BlockingTimeOut() ));

        if( PeerNode::get_BlockingTimeOut() != 0 ){
            timeout = (struct timeval *) new char[ sizeof(struct timeval) ];
            timeout->tv_sec = (PeerNode::get_BlockingTimeOut() / 1000);
            timeout->tv_usec = (PeerNode::get_BlockingTimeOut() % 1000) * 1000;
        }

    }
    else {
        timeout = (struct timeval *) new char[ sizeof(struct timeval) ];
        timeout->tv_sec = 0;
        timeout->tv_usec = 0;
    }
    pollret = select( max_fd + 1, &rfds, NULL, NULL, timeout );
    delete [] timeout;

    mrn_dbg( 3, mrn_printf(FLF, stderr, "select() returned %d\n", pollret ));

    if( pollret > 0 ) {
        // there is input on some connection
        // determine the connection on which input exists
        for( iter=peers.begin(); iter!=peers.end(); iter++ ) {
            PeerNodePtr cur_node = *iter;
            if( cur_node->has_Error() )
                continue;
            int curr_fd = cur_node->get_DataSocketFd();
            if( FD_ISSET( curr_fd, &rfds ) ) {
                if( cur_node->recv( pkt_list ) == -1 ) {
                    ret = -1;
                    mrn_dbg( 1, mrn_printf(FLF, stderr,
                                           "PN: recv() from ready node failed\n" ));
                    cur_node->error( MRN_ENETWORK_FAILURE, "recv() failed");
                    //TODO: handle error
                }
            }
        }
    }
    else if( pollret < 0 ) {
        // an error occurred
        ret = -1;
        mrn_dbg( 1, mrn_printf(FLF, stderr,
                    "PN::recv_PacketsFromChildren() poll failed\n" ));
        fprintf( stderr, "%d: poll failed: %d: %s\n",
            XPlat::Process::GetProcessId(), errno, strerror(errno) );
    }

    mrn_dbg( 2, mrn_printf(FLF, stderr, "PN::recv_PacketsFromChildren() %s\n",
                ( ret >= 0 ? "succeeded" : "failed" ) ));
    if( ret >= 0 && pkt_list.size() > 0 ){
        mrn_dbg(2, mrn_printf(FLF, stderr,
                   "recv_PacketsFromChildren() => %d packets. Tags:",
                   pkt_list.size() ));

        std::list <PacketPtr>::iterator piter;
        for(piter = pkt_list.begin(); piter != pkt_list.end(); piter++){
            mrn_dbg(2, mrn_printf(0,0,0, stderr, " %d", (*piter)->get_Tag() ));
        }
        mrn_dbg(2, mrn_printf(0,0,0, stderr, "\n"));
    }

    return ret;
}

int ParentNode::proc_PacketsFromChildren( std::list < PacketPtr >&
                                            packet_list ) const
{
    int retval = 0;
    PacketPtr cur_packet;

    mrn_dbg_func_begin();

    std::list < PacketPtr >::iterator iter = packet_list.begin(  );
    for( ; iter != packet_list.end(  ); iter++ ) {
        cur_packet = ( *iter );
        switch ( cur_packet->get_Tag(  ) ) {
        case PROT_CLOSE_STREAM:
            mrn_dbg( 5, mrn_printf(FLF, stderr,
                                   "Calling proc_closeStream() ...\n" ));
            if( proc_closeStream( cur_packet ) == -1 ) {
                mrn_dbg( 1, mrn_printf(FLF, stderr,
                            "proc_closeStream() failed\n" ));
                retval = -1;
            }
            //printf(3, mrn_printf(FLF, stderr, "proc_closeStream() succeeded\n");
            break;
        case PROT_NEW_SUBTREE_RPT:
            mrn_dbg( 5, mrn_printf(FLF, stderr,
                                   "Calling proc_newSubTreeReport() ...\n" ));
            if( proc_newSubTreeReport( cur_packet ) == -1 ) {
                mrn_dbg( 1, mrn_printf(FLF, stderr,
                            "proc_newSubTreeReport() failed\n" ));
                retval = -1;
            }
            //printf(3, mrn_printf(FLF, stderr, "proc_newSubTreeReport() succeeded\n");
            break;
         case PROT_DEL_SUBTREE_ACK:
            mrn_dbg( 5, mrn_printf(FLF, stderr,
                                   "Calling proc_DeleteSubTreeAck() ...\n" ));
            if( proc_DeleteSubTreeAck( cur_packet ) == -1 ) {
                mrn_dbg( 1, mrn_printf(FLF, stderr,
                            "proc_DeleteSubTreeAck() failed\n" ));
                retval = -1;
            }
            //printf(3, mrn_printf(FLF, stderr, "proc_DeleteSubTreeAck() succeeded\n");
            break;
       case PROT_EVENT:
            if( proc_Event( cur_packet ) == -1 ){
                mrn_dbg( 1, mrn_printf(FLF, stderr, "proc_Event() failed\n" ));
                retval = -1;
            }
            break;
        case PROT_FAILURE_RPT:
            if( proc_FailureReport( cur_packet ) == -1 ){
                mrn_dbg( 1, mrn_printf(FLF, stderr,
                                       "proc_FailureReport() failed\n" ));
                retval = -1;
            }
            break;
        case PROT_RECOVERY_RPT:
            if( proc_RecoveryReport( cur_packet ) == -1 ){
                mrn_dbg( 1, mrn_printf(FLF, stderr, "proc_RecoveryReport() failed\n" ));
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
        case PROT_SUBTREE_INFO_REQ:
            if( proc_SubTreeInfoRequest( cur_packet ) == -1 ){
                mrn_dbg( 1, mrn_printf(FLF, stderr,
                                       "proc_SubTreeInfoRequest() failed\n" ));
                retval = -1;
            }
            break;
        default:
            //Any unrecognized tag is assumed to be data
            if( proc_DataFromChildren( cur_packet ) == -1 ) {
                mrn_dbg( 1, mrn_printf(FLF, stderr,
                            "proc_DataFromChildren() failed\n" ));
                retval = -1;
            }
        }
    }

    mrn_dbg( 3, mrn_printf(FLF, stderr, "proc_PacketsFromChildren() %s",
                ( retval == -1 ? "failed\n" : "succeeded\n" ) ));
    packet_list.clear(  );
    return retval;
}

int ParentNode::proc_newSubTree( PacketPtr ipacket )
{
    char *byte_array = NULL;
    char *backend_exe = NULL;
    char *commnode_path=NULL;
    char **backend_argv;
    unsigned int backend_argc;

    mrn_dbg_func_begin();

    _initial_subtree_packet = ipacket;
    if( ipacket->ExtractArgList( "%s%s%s%as", &byte_array, &commnode_path,
                               &backend_exe, &backend_argv, &backend_argc ) == -1 ) {
        mrn_dbg( 1, mrn_printf(FLF, stderr, "ExtractArgList() failed\n" ));
        return -1;
    }

    SerialGraph sg( byte_array );
    std::string backend_exe_str( ( backend_exe == NULL ) ? "" : backend_exe );

    SerialGraph *cur_sg, *my_sg;

    //use "UnknownPort" in lookup since this is what serialgraph was created w/
    my_sg = sg.get_MySubTree( _hostname, UnknownPort, _rank );
    if( my_sg == NULL ) {
        mrn_dbg( 1, mrn_printf(FLF, stderr, "get_MySuBTree() failed\n" ));
        return -1;
    }
    my_sg->set_ToFirstChild( );
    
    for( cur_sg = my_sg->get_NextChild( ); cur_sg;
         cur_sg = my_sg->get_NextChild( ) ) {
        subtreereport_sync.Lock( );
        _num_children++;
        subtreereport_sync.Unlock( );

        std::string cur_node_hostname = cur_sg->get_RootHostName( ); 
        Rank cur_node_rank = cur_sg->get_RootRank( );

        if( !cur_sg->is_RootBackEnd( ) ) {
            mrn_dbg( 5, mrn_printf(FLF, stderr, "launching internal node ...\n" ));
            launch_InternalNode( cur_node_hostname, cur_node_rank, commnode_path );
        }
        else {
            if( backend_exe_str.length( ) > 0 ) {
                mrn_dbg( 5, mrn_printf(FLF, stderr, "launching backend_exe: \"%s\"\n",
                                       backend_exe_str.c_str() )); 
                std::vector < std::string > args;
                for(unsigned int i=0; i<backend_argc; i++){
                    args.push_back( backend_argv[i] );
                }
                if( launch_Application( cur_node_hostname, cur_node_rank,
                                        backend_exe_str, args ) == -1 ){
                    mrn_dbg( 1, mrn_printf(FLF, stderr,
                                "launch_application() failed\n" ));
                    return -1;
                }
            }
            else {
                // BE attach case
                mrn_dbg( 5, mrn_printf(FLF, stderr, "launching internal node ...\n" ));
                launch_InternalNode( cur_node_hostname, cur_node_rank, commnode_path );
            }
        }
    }

    mrn_dbg( 3, mrn_printf(FLF, stderr, "proc_newSubTree() succeeded\n" ));
    return 0;
}

int ParentNode::waitfor_SubTreeReports( void ) const
{
    std::list < PacketPtr >packet_list;

    subtreereport_sync.Lock( );
    while( _num_children > _num_children_reported ) {
        mrn_dbg( 3, mrn_printf(FLF, stderr, "Waiting for child nodes ...\n",
                               _num_children ));
        subtreereport_sync.WaitOnCondition( ALLNODESREPORTED );
        mrn_dbg( 3, mrn_printf(FLF, stderr,
                               "%d of %d Descendants have checked in.\n",
                               _num_children_reported, _num_children ));
    }
    subtreereport_sync.Unlock( );

    mrn_dbg( 3, mrn_printf(FLF, stderr, "All %d children nodes have reported\n",
                _num_children ));
    return 0;
}

bool ParentNode::waitfor_DeleteSubTreeAcks( void ) const
{
    mrn_dbg_func_begin();

    subtreereport_sync.Lock( );
    while( _num_children > _num_children_reported ) {
        mrn_dbg( 3, mrn_printf(FLF, stderr, "Waiting for %u of %u reports ...\n",
                               _num_children - _num_children_reported,
                               _num_children ));
        subtreereport_sync.WaitOnCondition( ALLNODESREPORTED );
        mrn_dbg( 3, mrn_printf(FLF, stderr,
                               "%d of %d children have ack'd.\n",
                               _num_children, _num_children_reported ));
    }
    subtreereport_sync.Unlock( );

    mrn_dbg_func_end();
    return true;
}

int ParentNode::proc_DeleteSubTree( PacketPtr ipacket ) const
{
    mrn_dbg_func_begin();

    _num_children_reported = _num_children = 0;
    const std::set < PeerNodePtr > peers = _network->get_ChildPeers();
    std::set < PeerNodePtr >::const_iterator iter;
    for( iter=peers.begin(); iter!=peers.end(); iter++ ) {
        if( (*iter)->is_child() ) {
            _num_children++;
        }
    }

    //processes will be exiting -- disable failure recovery
    _network->disable_FailureRecovery();

    //Send ack to parent, if any
    if( _network->is_LocalNodeChild() ) {
        if( _network->get_LocalChildNode()->ack_DeleteSubTree() ) {
            mrn_dbg( 1, mrn_printf(FLF, stderr, "ack_DeleteSubTree() failed\n" ));
        }
    }

    //send delete_subtree message to all children
    if( ( _network->send_PacketToChildren( ipacket ) == -1 ) ||
        ( _network->flush_PacketsToChildren( ) == -1 ) ) {
        mrn_dbg( 1, mrn_printf(FLF, stderr, "send/flush_PacketToChildren() failed\n" ));
    }

    //wait for acks -- so children don't initiate failure recovery when we exit
    if( !waitfor_DeleteSubTreeAcks() ) {
        mrn_dbg( 1, mrn_printf(FLF, stderr, "waitfor_DeleteSubTreeAcks() failed\n" ));
    }

    // turn off debug output to prevent cancelled threads from causing mrn_printf deadlock
    MRN::set_OutputLevel( -1 );

    //kill all IO threads, should allow downstream threads to flush
    _network->cancel_IOThreads();
    EventDetector::stop();

    //if internal, signal network termination
    if( _network->is_LocalNodeInternal() ) {
        _network->get_LocalInternalNode()->signal_NetworkTermination();
    }

    mrn_dbg_func_end();
    return 0;
}

int ParentNode::proc_newSubTreeReport( PacketPtr ipacket ) const
{
    mrn_dbg_func_begin();

    char * topo_ptr=NULL;

    ipacket->unpack( "%s", &topo_ptr ); 

    SerialGraph sg( topo_ptr );
    if( !_network->add_SubGraph( _network->get_LocalRank(), sg ) ){
        mrn_dbg(5, mrn_printf(FLF, stderr, "add_SubGraph() failed\n"));
        return -1;
    }

    subtreereport_sync.Lock( );

    if( _num_children_reported == _num_children ) {
        // already saw reports from known children, must be a newborn
        if( sg.is_RootBackEnd() ) {
            _num_children++;
            _num_children_reported++;
        }
        subtreereport_sync.Unlock( );
        if( _network->is_LocalNodeChild() ) {
            _network->get_LocalChildNode()->send_NewSubTreeReport();
        }
    }
    else {
        _num_children_reported++;
        mrn_dbg( 3, mrn_printf(FLF, stderr, "%d of %d descendants reported\n",
                               _num_children_reported, _num_children ));
        if( _num_children_reported == _num_children ) {
            subtreereport_sync.SignalCondition( ALLNODESREPORTED );
        }
        subtreereport_sync.Unlock( );
    }

    mrn_dbg_func_end();
    return 0;
}

int ParentNode::proc_DeleteSubTreeAck( PacketPtr /* ipacket */ ) const
{
    mrn_dbg_func_begin();

    subtreereport_sync.Lock( );
    _num_children_reported++;
    mrn_dbg( 3, mrn_printf(FLF, stderr, "%d of %d children reported\n",
                           _num_children_reported, _num_children ));
    if( _num_children_reported == _num_children ) {
        subtreereport_sync.SignalCondition( ALLNODESREPORTED );
    }
    subtreereport_sync.Unlock( );
	
    mrn_dbg_func_end();
    return 0;
}

int ParentNode::proc_Event( PacketPtr ipacket ) const
{
    char *edesc=NULL;
    EventType etype;
    Rank erank;

    mrn_dbg( 3, mrn_printf(FLF, stderr, "In parentnode.proc_Event()\n" ));
    if( ipacket->ExtractArgList( "%d %ud %s", &etype, &erank, &edesc ) == -1 ) {
        mrn_dbg( 1, mrn_printf(FLF, stderr, "ExtractArgList failed\n" ));
        return -1;
    }

    //Event * event = new Event( etype, erank, edesc );
    //Event::add_Event( *event );
    //delete event;

    mrn_dbg( 3, mrn_printf(FLF, stderr, "Leaving parentnode.proc_Event()\n" ));
    return 0;
}

Stream * ParentNode::proc_newStream( PacketPtr ipacket ) const
{
    unsigned int num_backends;
    Rank *backends;
    int stream_id, sync_id;
    int ds_filter_id = -1;
    int us_filter_id = -1;

    mrn_dbg_func_begin();

    // extract the info needed to build the stream
    if( ipacket->ExtractArgList( "%d %ad %d %d %d", 
                                 &stream_id, &backends, &num_backends, 
                                 &us_filter_id, &sync_id, &ds_filter_id ) == -1 ) {
        mrn_dbg( 1, mrn_printf(FLF, stderr, "ExtractArgList() failed\n" ));
        return NULL;
    }

    //register new stream w/ network
    Stream * stream = _network->new_Stream( stream_id, backends, num_backends,
                                            us_filter_id, sync_id, ds_filter_id );

    //send packet to children nodes
    if( _network->send_PacketToChildren(ipacket) == -1 ) {
        mrn_dbg( 3, mrn_printf(FLF, stderr, "send_PacketToChildren() failed\n" ));
        return NULL;
    }

    mrn_dbg_func_end();
    return stream;
}


int ParentNode::proc_DownstreamFilterParams( PacketPtr &ipacket ) const
{
    int stream_id;

    mrn_dbg_func_begin();

    stream_id = ipacket->get_StreamId();
    Stream* strm = _network->get_Stream( stream_id );

    //send packet to child nodes
    if( _network->send_PacketToChildren( ipacket ) == -1 ) {
        mrn_dbg( 3, mrn_printf(FLF, stderr, "send_PacketToChildren() failed\n" ));
        return -1;
    }

    // local update
    strm->set_FilterParams( false, ipacket );

    mrn_dbg_func_end();
    return 0;
}

int ParentNode::proc_UpstreamFilterParams( PacketPtr &ipacket ) const
{
    int stream_id;

    mrn_dbg_func_begin();

    stream_id = ipacket->get_StreamId();
    Stream* strm = _network->get_Stream( stream_id );

    //send packet to children nodes
    if( _network->send_PacketToChildren( ipacket ) == -1 ) {
        mrn_dbg( 3, mrn_printf(FLF, stderr, "send_PacketToChildren() failed\n" ));
        return -1;
    }

    // local update
    strm->set_FilterParams( true, ipacket );

    mrn_dbg_func_end();
    return 0;
}


int ParentNode::proc_deleteStream( PacketPtr ipacket ) const
{
    int stream_id;

    mrn_dbg_func_begin();

    if( ipacket->ExtractArgList( "%d", &stream_id ) == -1 ) {
        mrn_dbg( 1, mrn_printf(FLF, stderr, "ExtractArgList() failed\n" ));
        return -1;
    }

    if( _network->send_PacketToChildren( ipacket ) == -1 ) {
        mrn_dbg(2, mrn_printf(FLF, stderr, "send_PacketToChildren() failed\n"));
        return -1;
    }

    //delete only @ internal node, front-end stream destructor invokes this function
    if( _network->is_LocalNodeInternal() ) {
        delete _network->get_Stream( stream_id );
    }

    mrn_dbg_func_end();
    return 0;
}

int ParentNode::proc_newFilter( PacketPtr ipacket ) const
{
    int retval = 0;
    unsigned short fid = 0;
    const char *so_file = NULL, *func = NULL;

    mrn_dbg_func_begin();

    if( ipacket->ExtractArgList( "%uhd %s %s", &fid, &so_file, &func ) == -1 ) {
        mrn_dbg( 1, mrn_printf(FLF, stderr, "ExtractArgList() failed\n" ));
        return -1;
    }

    retval = Filter::load_FilterFunc( so_file, func );

    if( retval != ( int )fid ) {
        mrn_dbg( 1, mrn_printf(FLF, stderr,
                               "Filter::load_FilterFunc() failed.\n" ));
        return -1;
    }

    //Filter registered locally, now propagate to tree
    _network->send_PacketToChildren( ipacket );

    mrn_dbg_func_end();
    return fid;
}

bool lt_PeerNodePtr( PeerNode * p1, PeerNode * p2 )
{
    assert( p1 && p2 );

    if( p1->get_HostName( ) < p2->get_HostName( ) ) {
        return true;
    }
    else if( p1->get_HostName( ) == p2->get_HostName( ) ) {
        return ( p1->get_Port( ) < p2->get_Port( ) );
    }
    else {
        return false;
    }
}

bool equal_PeerNodePtr( PeerNode * p1, PeerNode * p2 )
{
    return p1 == p2;        //tmp hack, should be something like below

    if( p1->get_HostName( ) != p2->get_HostName( ) ) {
        return false;
    }
    else {
        return ( p1->get_Port( ) == p2->get_Port( ) );
    }
}

int ParentNode::proc_NewChildDataConnection( PacketPtr ipacket, int isock )
{
    char * child_hostname_ptr=NULL;
    Port child_port;
    Rank child_rank, old_parent_rank;
    uint16_t child_incarnation;
    char is_internal_char;
    char * topo_ptr=NULL;

    ipacket->unpack( "%s %uhd %ud %uhd %ud %c %s",
                     &child_hostname_ptr,
                     &child_port,
                     &child_rank,
                     &child_incarnation,
                     &old_parent_rank,
                     &is_internal_char,
                     &topo_ptr ); 

    mrn_dbg(5, mrn_printf(FLF, stderr, "New child node[%s:%u:%u] (incarnation:%u) on socket %d\n",
                          child_hostname_ptr, child_rank, child_port,
                          child_incarnation, isock ));

    mrn_dbg(5, mrn_printf(FLF, stderr, "is_internal?: '%c'\n", is_internal_char ));
    bool is_internal = ( is_internal_char  ? true : false );

    std::string child_hostname( child_hostname_ptr );
    PeerNodePtr child_node = _network->new_PeerNode( child_hostname,
                                                     child_port,
                                                     child_rank,
                                                     false,
                                                     is_internal );

    child_node->set_DataSocketFd( isock );

    SerialGraph sg( topo_ptr );
    if( !_network->add_SubGraph( _network->get_LocalRank(), sg ) ){
        mrn_dbg(5, mrn_printf(FLF, stderr, "add_SubGraph() failed\n"));
        return -1;
    }

    //Create send/recv threads
    mrn_dbg(5, mrn_printf(FLF, stderr, "Creating comm threads for new child\n" ));
    child_node->start_CommunicationThreads();

    if( child_incarnation > 1 ) {
        //child's parent has failed
        mrn_dbg(5, mrn_printf(FLF, stderr,
                              "child[%s:%u]'s old parent: %u, new parent: %u\n",
                              child_hostname_ptr, child_rank,
                              old_parent_rank, _network->get_LocalRank() ));

        PacketPtr packet( new Packet( 0, PROT_RECOVERY_RPT, "%ud %ud %ud",
                                      child_rank,
                                      old_parent_rank,
                                      _network->get_LocalRank() ) );

        if( proc_RecoveryReport( packet ) == -1 ) {
            mrn_dbg( 1, mrn_printf(FLF, stderr, "proc_RecoveryReport() failed()\n" ));
        }
    }

    return 0;
}

int ParentNode::proc_SubTreeInfoRequest( PacketPtr  ipacket ) const
{
    //send the packet containing the initial subtree to requesting node
    mrn_dbg(5, mrn_printf(FLF, stderr, "sending initial subtree packet:\n"
                          "\t%s\n", (*_initial_subtree_packet)[0]->get_string() ));
    PeerNodePtr outlet = _network->get_PeerNode( ipacket->get_InletNodeRank() );
    if( ( outlet->send( _initial_subtree_packet ) == -1 ) ||
        ( outlet->flush() == -1 ) ){
        mrn_dbg(1, mrn_printf(FLF, stderr, "send()/flush() failed\n" ));
        return -1;
    }

    return 0;
}

int ParentNode::launch_InternalNode( std::string ihostname, Rank irank,
                                     std::string icommnode_exe )
    const
{
    char parent_port_str[128];
    snprintf(parent_port_str, sizeof(parent_port_str), "%d", _port);
    char parent_rank_str[128];
    snprintf(parent_rank_str, sizeof(parent_rank_str), "%d", _rank);
    char rank_str[128];
    snprintf(rank_str, sizeof(rank_str), "%d", irank );

    mrn_dbg(3, mrn_printf(FLF, stderr, "Launching %s:%d ...",
                          ihostname.c_str(), irank ));

    // set up arguments for the new process
    std::vector <std::string> args;
    args.push_back(icommnode_exe);
    args.push_back(ihostname);
    args.push_back(rank_str);
    args.push_back(_hostname);
    args.push_back(parent_port_str);
    args.push_back( parent_rank_str );

    if( XPlat::Process::Create( ihostname, icommnode_exe, args ) != 0 ){
        int err = XPlat::Process::GetLastError();
        
        error( MRN_ESYSTEM, "XPlat::Process::Create(%s %s): %s\n",
               ihostname.c_str(), icommnode_exe.c_str(),
               XPlat::Error::GetErrorString( err ).c_str() );
        mrn_dbg(1, mrn_printf(FLF, stderr,
                              "XPlat::Process::Create(%s %s): %s\n",
                              ihostname.c_str(), icommnode_exe.c_str(),
                              XPlat::Error::GetErrorString( err ).c_str() ));
        return -1;
    }

    mrn_dbg(3, mrn_printf(0,0,0, stderr, "Success!\n" ));

    return 0;
}


int ParentNode::launch_Application( std::string ihostname, Rank irank, std::string &ibackend_exe,
                                    std::vector <std::string> &ibackend_args)
    const
{

    mrn_dbg(3, mrn_printf(FLF, stderr, "Launching application on %s:%d (\"%s\")\n",
                          ihostname.c_str(), irank, ibackend_exe.c_str()));
  
    char parent_port_str[128];
    snprintf(parent_port_str, sizeof( parent_port_str), "%d", _port);
    char parent_rank_str[128];
    snprintf(parent_rank_str, sizeof( parent_rank_str), "%d", _rank );
    char rank_str[128];
    snprintf(rank_str, sizeof(rank_str), "%d", irank );

    // set up arguments for new process: copy to get the cmd in front

    std::vector<std::string> new_args;

    new_args.push_back( ibackend_exe );
    for(unsigned int i=0; i<ibackend_args.size(); i++){
        new_args.push_back(ibackend_args[i]);
    }
    new_args.push_back( _hostname );
    new_args.push_back( parent_port_str );
    new_args.push_back( parent_rank_str );
    new_args.push_back( ihostname );
    new_args.push_back( rank_str );

    mrn_dbg(5, mrn_printf(FLF, stderr, "Creating \"%s\" on \"%s:%d\"\n",
                          ibackend_exe.c_str(), ihostname.c_str(), irank ));
  
    if( XPlat::Process::Create( ihostname, ibackend_exe, new_args ) != 0 ){
        mrn_dbg(1, mrn_printf(FLF, stderr, "XPlat::Process::Create() failed\n"));
        int err = XPlat::Process::GetLastError();
        error( MRN_ESYSTEM, "XPlat::Process::Create(%s %s): %s\n",
               ihostname.c_str(), ibackend_exe.c_str(),
               XPlat::Error::GetErrorString( err ).c_str() );
        return -1;
    }
    mrn_dbg(5, mrn_printf(FLF, stderr, "success\n"));

    return 0;
}

int ParentNode::proc_FailureReport( PacketPtr ipacket ) const
{
    Rank failed_rank;

    ipacket->unpack( "%ud", &failed_rank ); 

    mrn_dbg( 5, mrn_printf(FLF, stderr, "node[%u] has failed\n", failed_rank ));

    //update local topology
    ParentNode::_network->remove_Node( failed_rank );

    //propagate to children except on incident channel
    if( _network->send_PacketToChildren( ipacket, false ) == -1 ){
        mrn_dbg( 1, mrn_printf(FLF, stderr,
                               "send_PacketToChildren() failed()\n" ));
        return -1;
    }

    if( _network->is_LocalNodeChild() ) {
        //propagate to parent
        if( ( _network->send_PacketToParent( ipacket ) == -1 ) ||
            ( _network->flush_PacketsToChildren( ) == -1 ) ) {
            mrn_dbg( 1, mrn_printf(FLF, stderr, "parent.send/flush() failed()\n" ));
            return -1;
        }
    }

    return 0;
}

int ParentNode::proc_RecoveryReport( PacketPtr ipacket ) const
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

    //send packet to all children except the one on which the packet arrived
    if( _network->send_PacketToChildren( ipacket, false ) == -1 ){
        mrn_dbg(1, mrn_printf(FLF, stderr, "send_PacketToChildren() failed\n"));
        return -1;
    }

    //Internal nodes must report parent failure to parent
    if( _network->is_LocalNodeInternal() ) {
        if( ( _network->get_ParentNode()->send( ipacket ) == -1 ) ||
            ( _network->get_ParentNode()->flush( ) == -1 ) ) {
            mrn_dbg(1, mrn_printf(FLF, stderr, "send()/flush() failed\n"));
            return -1;
        }
    }
    
    mrn_dbg_func_end();
    return 0;
}

int ParentNode::proc_closeStream( PacketPtr ipacket ) const
{
    int stream_id;
    ipacket->unpack( "%d", &stream_id );

    Stream * stream = _network->get_Stream( stream_id );
    assert( stream );

    stream->close_Peer( ipacket->get_InletNodeRank() );

    if( stream->is_Closed() ) {
        if( _network->is_LocalNodeChild() ) {
            //Internal Nodes should propagate "close"
            if( _network->send_PacketToParent( ipacket ) == -1 ) {
                mrn_dbg(1, mrn_printf(FLF, stderr, "send() failed\n"));
                return -1;
            }
        }
        else {
            //Front-end node should awaken any folks blocked on recv()
            assert( _network->is_LocalNodeFrontEnd() );
            stream->signal_BlockedReceivers();
        }
    }
    
    return 0;
}

}                               // namespace MRN
