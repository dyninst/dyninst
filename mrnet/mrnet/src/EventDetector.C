/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#include "mrnet/MRNet.h"

#include "EventDetector.h"
#include "InternalNode.h"
#include "FrontEndNode.h"
#include "BackEndNode.h"
#include "ParentNode.h"
#include "ChildNode.h"
#include "ParsedGraph.h"
#include "PeerNode.h"
#include "Router.h"
#include "PerfDataEvent.h"
#include "mrnet/Stream.h"
#include "utils.h"
#include "xplat/NetUtils.h"
#include "xplat/SocketUtils.h"

using namespace std;

namespace MRN {

long EventDetector::_thread_id=0;

static int proc_NewChildFDConnection( PacketPtr ipacket, int isock );

static map< int, Rank > childRankByEventDetectionSocket;

typedef struct
{
    Network *network;
    PeerNodePtr parent_node;
} EDTArg ;

bool EventDetector::stop( void )
{
    mrn_dbg_func_begin();

    if( XPlat::Thread::Cancel( _thread_id ) != 0 ) {
        mrn_dbg(1, mrn_printf(FLF, stderr, "Thread::Cancel(%d) failed\n",
                              _thread_id ));
        ::perror( "Thread::Cancel()\n");
        return false;
    }

    mrn_dbg_func_end();
    return true;
}

bool EventDetector::start( Network * /* inetwork */ )
{
    mrn_dbg_func_begin();
    //EDTArg * thread_arg = new EDTArg;

    //thread_arg->network = inetwork;
    //thread_arg->parent_node = inetwork->get_ParentNode();
    mrn_dbg(3, mrn_printf(FLF, stderr, "Creating Event Detection thread ...\n"));

    if( XPlat::Thread::Create( main, NULL, &_thread_id ) == -1 ) {
        mrn_dbg(1, mrn_printf(FLF, stderr, "Thread creation failed...\n"));
        return false;
    }

    mrn_dbg_func_end();
    return true;
}

void * EventDetector::main( void * /* iarg */ )
{
    list< int > watch_list; //list of sockets to detect events on
    int parent_sock=0;
    int local_sock=0;
    int max_sock=0;

    //EDTArg * arg=(EDTArg*)iarg;
    //Network * network = arg->network;
    PeerNodePtr  parent_node = PeerNode::NullPeerNode;
    if( network->is_LocalNodeChild() ) {
        parent_node = network->get_ParentNode();
    }
    //delete arg;
    
    //set up debugging stuff
    string prettyHost;
    XPlat::NetUtils::GetHostName( network->get_LocalHostName(), prettyHost );
    char rank_str[128];
    snprintf( rank_str, sizeof(rank_str), "%u", network->get_LocalRank() ); 
    string name("EDT(");
    name += prettyHost;
    name += ":";
    name += rank_str;
    name += ")";

    tsd_t * local_data = new tsd_t;
    local_data->thread_id = XPlat::Thread::GetId();
    local_data->thread_name = strdup(name.c_str());

    int status;
    if( (status = tsd_key.Set( local_data )) != 0){
        fprintf(stderr, "XPlat::TLSKey::Set(): %s\n", strerror(status)); 
        return NULL;
    }

    srand48( network->get_LocalRank() );
    
    // initialize CPU performance data collection
    handle_PerfDataCPU( Packet::NullPacket, network->get_LocalRank() );                        

    //Prepare fds for select()
    fd_set rfds, rfds_copy;

    FD_ZERO( &rfds );

    //(1) Establish connection with parent Event Detection Thread
    if( network->is_LocalNodeChild() ) {
        if( init_NewChildFDConnection( network, parent_node ) == -1 ){
            mrn_dbg( 1, mrn_printf(FLF, stderr,
                                   "init_NewChildFDConnection() failed\n") );
        }
        mrn_dbg( 5, mrn_printf(0,0,0, stderr, "success!\n"));

        //monitor parent sock for failure
        parent_sock = parent_node->get_EventSocketFd();
        FD_SET( parent_sock, &rfds );
        max_sock = parent_sock;
        watch_list.push_back( parent_sock );
        mrn_dbg( 5, mrn_printf(FLF, stderr,
                               "Parent socket:%d added to list.\n", parent_sock));
    }

    if( network->is_LocalNodeParent() ){
        //(2) Add local socket to event list
        local_sock = network->get_ListeningSocket();
        if( local_sock != -1 ){
            FD_SET( local_sock, &rfds );
            mrn_dbg( 5, mrn_printf(FLF, stderr,
                                   "Monitoring local socket:%d.\n", local_sock));
            if( local_sock > max_sock )
                max_sock = local_sock;
        }
    }

    //3) do EventDetection Loop, current events are:
    //   - PROT_KILL_SELF 
    //   - PROT_NEW_CHILD_FD_CONNECTION (a new child peer to monitor)
    //   - PROT_NEW_CHILD_DATA_CONNECTION (a new child peer for data)
    //   - socket failures
    Message msg;
    list< PacketPtr > packets;
    mrn_dbg( 5, mrn_printf(FLF, stderr, "EDT Main Loop ...\n"));
    while ( true ) {
        rfds_copy = rfds;
        mrn_dbg( 5, mrn_printf(FLF, stderr, "Blocking on select(): %d sockets...\n",
                               watch_list.size() ));

        int retval = select( max_sock+1, &rfds_copy, NULL, NULL, NULL );

        if( retval == -1 ) {
            //select error
            perror("select()");
            continue;
        }
        else if( retval == 0 ){
            //shouldn't get here since we block forever
            perror("select()");
            assert(0);
        }
        else{
            if( FD_ISSET ( local_sock, &rfds_copy ) ){
                //Activity on our local listening sock, accept connection
                mrn_dbg( 5, mrn_printf(FLF, stderr, "Activity on listening socket ...\n"));
                int connected_sock = getSocketConnection( local_sock );
                if( connected_sock == -1 ){
                    mrn_dbg( 1, mrn_printf(FLF, stderr, "getSocketConnection() failed\n"));
                    perror("getSocketConnection()");
                    continue;
                }

                packets.clear();
                msg.recv( connected_sock, packets, UnknownRank );
                list< PacketPtr >::iterator packet_list_iter;
                list< int >::iterator iter;

                for( packet_list_iter = packets.begin();
                     packet_list_iter != packets.end();
                     packet_list_iter++  ) {
                    ParentNode* p;
                    PacketPtr cur_packet( *packet_list_iter );
                    switch ( cur_packet->get_Tag() ) {

                    case PROT_GUI_CPUPERCENT: {
                        fflush( stdout );
                        mrn_dbg( 5, mrn_printf(FLF, stderr, "PROT_GUI_CPUPERCENT ...\n"));
                        handle_PerfDataCPU( cur_packet, network->get_LocalRank() );
                        break;
                    }

                    case PROT_GUI_INIT: {
                        fflush( stdout );
                        mrn_dbg( 5, mrn_printf(FLF, stderr, "PROT_GUI_INIT ...\n"));
                        handle_PerfGuiInit( cur_packet );
                        break;
                    }

                    case PROT_KILL_SELF:
                        mrn_dbg( 5, mrn_printf(FLF, stderr, "PROT_KILL_SELF ...\n"));
                        //network->close_PeerNodeConnections();

                        //close event sockets explicitly
                        mrn_dbg(5, mrn_printf(FLF, stderr,
                                              "Closing %d sockets\n",
                                              watch_list.size() ));
                        for(iter=watch_list.begin(); iter!=watch_list.end(); iter++){
                            mrn_dbg(5, mrn_printf(FLF, stderr,
                                                  "Closing event socket: %d\n", *iter ));
                            char buf[16];
                            mrn_dbg(5, mrn_printf(FLF, stderr, "writing ...\n"));
                            if( write( *iter, buf, 1) == -1 ) {
                                perror("write(event_fd)");
                            }
                            mrn_dbg(5, mrn_printf(FLF, stderr, "closing ...\n"));
                            if( XPlat::SocketUtils::Close( *iter ) == -1 ){
                                perror("close(event_fd)");
                            }
                        }
                        mrn_dbg(5, mrn_printf(FLF, stderr, "bye!\n"));
                        exit(0);
                        break;

                    case PROT_NEW_CHILD_FD_CONNECTION:
                        mrn_dbg( 5, mrn_printf(FLF, stderr, "PROT_NEW_CHILD_FD_CONNECTION ...\n"));
                        FD_SET( connected_sock, &rfds );
                        if( connected_sock > max_sock )
                            max_sock=connected_sock;
                        watch_list.push_back( connected_sock );
                        mrn_dbg( 5, mrn_printf(FLF, stderr,
                                               "FD socket:%d added to list.\n",
                                               connected_sock));
                        
                        proc_NewChildFDConnection( cur_packet, connected_sock );
                        break;

                    case PROT_NEW_CHILD_DATA_CONNECTION:
                        mrn_dbg( 5, mrn_printf(FLF, stderr, "PROT_NEW_CHILD_DATA_CONNECTION ...\n"));
                        //get ParentNode obj. Try internal node, then FE
                        p = network->get_LocalParentNode();
                        assert(p);

                        p->proc_NewChildDataConnection( cur_packet,
                                                        connected_sock );
                        mrn_dbg( 5, mrn_printf(FLF, stderr, "New child connected.\n"));
                        break;

                    case PROT_NEW_SUBTREE_RPT:
                        // NOTE: needed since back-ends are now threaded, and we can't
                        //       guarantee a packet containing this protocol message
                        //       won't arrive in a group with NEW_CHILD_DATA_CONNECTION
                        mrn_dbg( 5, mrn_printf(FLF, stderr, "PROT_NEW_SUBTREE_RPT ...\n"));
                        //get ParentNode obj. Try internal node, then FE
                        p = network->get_LocalParentNode();
                        assert(p);

                        if( p->proc_newSubTreeReport( cur_packet ) == -1 ) {
                            mrn_dbg( 1, mrn_printf(FLF, stderr, "proc_newSubTreeReport() failed\n" ));
                        }
                        break;

                    default:
                        mrn_dbg( 1, mrn_printf(FLF, stderr, 
                                               "### PROTOCOL ERROR: Unexpected tag %d ###\n",
                                               cur_packet->get_Tag()));
                        break;
                    }
                }
            }

            if( network->is_LocalNodeChild() && FD_ISSET ( parent_sock, &rfds_copy ) ){
                mrn_dbg( 1, mrn_printf(FLF, stderr, "Parent failure detected ...\n"));
                //event happened on parent monitored connections, likely failure

                //remove old parent socket from "select" sockets list
                FD_CLR( parent_sock, &rfds );
                watch_list.remove( parent_sock );

                if( network->recover_FromFailures() ){
                    mrn_dbg( 1, mrn_printf(FLF, stderr, "recovering from parent failure ...\n"));
                    recover_FromParentFailure( network );

                    //add new parent sock to monitor for failure
                    parent_node = network->get_ParentNode();
                    parent_sock = parent_node->get_EventSocketFd();
                    FD_SET( parent_sock, &rfds );
                    max_sock = parent_sock;
                    watch_list.push_back( parent_sock );
                    mrn_dbg( 5, mrn_printf(FLF, stderr,
                                           "Parent socket:%d added to list.\n", parent_sock));
                }
                else{
                    mrn_dbg( 1, mrn_printf(FLF, stderr, "NOT recovering from parent failure ...\n"));
                    if( watch_list.size() == 0 ) {
                        mrn_dbg(5, mrn_printf(FLF, stderr, "No more sockets to watch, bye!\n"));
                        exit(0);
                    }	
                }
                
            }

            //Check for child failures. Whom to notify?
            list< int >::iterator iter;
            for( iter=watch_list.begin(); iter != watch_list.end(); ) {
                int cur_sock = *iter;
                //skip local_sock and parent_sock or if socket isn't set
                mrn_dbg( 5, mrn_printf(FLF, stderr, "Is socket:%d set?  ", cur_sock));
                if( ( cur_sock == local_sock ) ||
                    ( (network->is_LocalNodeChild() ) && (cur_sock == parent_sock) ) ||
                    ( !FD_ISSET( cur_sock, &rfds_copy ) ) ){
                    mrn_dbg( 5, mrn_printf(0,0,0, stderr, "NO!\n", cur_sock));
                    iter++;
                    continue;
                }
                mrn_dbg( 5, mrn_printf(0,0,0, stderr, "YES!\n", cur_sock));

                map< int, Rank >:: iterator iter2 =
                    childRankByEventDetectionSocket.find( cur_sock );

                if( iter2 != childRankByEventDetectionSocket.end() ) {
                    //this child has failed
                    int failed_rank = (*iter2).second;

                    mrn_dbg( 1, mrn_printf(FLF, stderr,
                                           "Child[%u] failure detected ...\n",
                                           failed_rank ));

                    if( network->recover_FromFailures() )
                        recover_FromChildFailure( network, failed_rank );

                    childRankByEventDetectionSocket.erase( iter2 );

                    //remove from "select" sockets list
                    FD_CLR( cur_sock, &rfds );

                    //remove socket from socket watch list
                    list< int >::iterator tmp_iter;
                    tmp_iter = iter;
                    iter++;
                    watch_list.erase( tmp_iter );
                }
            }
        }
    }

    return NULL;
}

int EventDetector::init_NewChildFDConnection( Network * inetwork, PeerNodePtr iparent_node )
{
    string lhostname = inetwork->get_LocalHostName();
    Port lport = inetwork->get_LocalPort();
    Rank lrank = inetwork->get_LocalRank();

    mrn_dbg( 5, mrn_printf(FLF, stderr, "Initializing new Child FD Connection ...\n"));
    if( iparent_node->connect_EventSocket() == -1 ){
        mrn_dbg( 1, mrn_printf(FLF, stderr, "connect( %s:%u ) failed\n",
                               iparent_node->get_HostName().c_str(),
                               iparent_node->get_Port() ) );
        perror("connect()");
    }

    PacketPtr packet( new Packet( 0, PROT_NEW_CHILD_FD_CONNECTION, "%s %uhd %ud",
                                  lhostname.c_str(), lport, lrank ) );
    Message msg;
    msg.add_Packet( packet );
    if( msg.send( iparent_node->get_EventSocketFd() ) == -1 ) {
        mrn_dbg(1, mrn_printf(FLF, stderr, "Message.send failed\n" ));
        return -1;
    }

    return 0;
}

int proc_NewChildFDConnection( PacketPtr ipacket, int isock )
{
    char * child_hostname_ptr;
    Port child_port;
    Rank child_rank;

    ipacket->unpack( "%s %uhd %ud", &child_hostname_ptr, &child_port,
                    &child_rank ); 


    string child_hostname( child_hostname_ptr );

    mrn_dbg(5, mrn_printf(FLF, stderr,
                          "New FD connection on sock %d from: %s:%u:%u\n",
                          isock, child_hostname_ptr, child_port, child_rank ) ); 

    childRankByEventDetectionSocket[ isock ] = child_rank;

    return 0;
}

int EventDetector::recover_FromChildFailure( Network *inetwork, Rank ifailed_rank )
{
    PacketPtr packet( new Packet( 0, PROT_FAILURE_RPT, "%ud", ifailed_rank ) );

    assert( inetwork->is_LocalNodeParent() );

    mrn_dbg(1, mrn_printf(FLF, stderr, "proc_FailureReport(node:%u) ...\n", ifailed_rank));
    if( inetwork->get_LocalParentNode()-> proc_FailureReport( packet ) == -1 ) {
        mrn_dbg(1, mrn_printf(FLF, stderr, "proc_FailureReport() failed\n"));
        return -1;
    }

    return 0;
}

int  EventDetector::recover_FromParentFailure( Network *inetwork )
{
    Timer new_parent_timer, cleanup_timer, connection_timer, filter_state_timer,
        overall_timer;
    mrn_dbg_func_begin();


    inetwork->get_ParentNode()->mark_Failed();
    Rank failed_rank = inetwork->get_ParentNode()->get_Rank();
    network->set_ParentNode( PeerNode::NullPeerNode );
    mrn_dbg(3, mrn_printf( FLF, stderr, "Recovering from parent[%d]'s failure\n",
                           failed_rank ));

    inetwork->get_NetworkTopology()->print(NULL);

    //Step 1: Compute new parent
    overall_timer.start();
    new_parent_timer.start();
    NetworkTopology::Node * new_parent_node =inetwork->get_NetworkTopology()->
        find_NewParent( inetwork->get_LocalRank() );
    if( !new_parent_node ) {
        mrn_dbg(1, mrn_printf( FLF, stderr, "Can't find new parent! Exiting ...\n" ));
        exit(-1);
    }

    mrn_dbg(1, mrn_printf( FLF, stderr, "RECOVERY: NEW PARENT: %s:%d:%d\n",
                           new_parent_node->get_HostName().c_str(),
                           new_parent_node->get_Port(),
                           new_parent_node->get_Rank() ));
    
    string new_parent_name = new_parent_node->get_HostName();
    PeerNodePtr new_parent = inetwork->new_PeerNode( new_parent_name,
                                                     new_parent_node->get_Port(),
                                                     new_parent_node->get_Rank(),
                                                     true, true );
    new_parent_timer.stop();

    //Step 2. Establish data connection w/ new Parent
    connection_timer.start();
    mrn_dbg(3, mrn_printf( FLF, stderr, "Establish new data connection ...\n"));
    if( inetwork->get_LocalChildNode()->init_newChildDataConnection( new_parent,
                                                                     failed_rank ) == -1 ) {
        mrn_dbg(1, mrn_printf(FLF, stderr,
                              "PeerNode::init_newChildDataConnection() failed\n"));
        return -1;
    }

    //Step 3. Establish event detection connection w/ new Parent
    mrn_dbg(3, mrn_printf( FLF, stderr, "Establish new event connection ...\n"));
    if( init_NewChildFDConnection( inetwork, new_parent ) == -1 ){
        mrn_dbg( 1, mrn_printf(FLF, stderr,
                               "init_NewChildFDConnection() failed\n") );
        return -1;
    }
    mrn_dbg(3, mrn_printf( FLF, stderr, "New event connection established...\n"));
    connection_timer.stop();

    //Step 4. Propagate filter state for active streams to new parent
    filter_state_timer.start();
    mrn_dbg(3, mrn_printf( FLF, stderr, "Sending filter states ...\n"));
    if( inetwork->send_FilterStatesToParent() == -1 ) {
        mrn_dbg( 1, mrn_printf(FLF, stderr,
                               "send_FilterStatesToParent() failed\n") );
        return -1;
    }
    mrn_dbg(3, mrn_printf( FLF, stderr, "Sending filter states complete!\n"));
    filter_state_timer.stop();

    //Step 5. Update local topology and data structures
    cleanup_timer.start();
    mrn_dbg(3, mrn_printf( FLF, stderr, "Updating local structures ...\n"));
    //remove node, but don't update datastructs since following procedure will
    inetwork->remove_Node( failed_rank, false );
    inetwork->change_Parent( inetwork->get_LocalRank(),
                             new_parent_node->get_Rank() );
    cleanup_timer.stop();

    overall_timer.stop();

    //Internal nodes must report parent failure to children
    mrn_dbg(3, mrn_printf( FLF, stderr, "Report failure to children ...\n"));
    if( inetwork->is_LocalNodeInternal() ) {
        //format is my_rank, failed_parent_rank, new_parent_rank
        PacketPtr packet( new Packet( 0, PROT_RECOVERY_RPT, "%ud %ud %ud",
                                      inetwork->get_LocalRank(),
                                      failed_rank,
                                      inetwork->get_ParentNode()->get_Rank() ) );

        if( inetwork->send_PacketToChildren( packet ) == -1 ){
            mrn_dbg(1, mrn_printf(FLF, stderr, "send_PacketDownStream() failed\n"));
        }
    }
    mrn_dbg(3, mrn_printf( FLF, stderr, "Report failure to children complete!\n"));

   //Notify Failure Manager that recovery is complete
    PacketPtr packet( new Packet( 0, PROT_RECOVERY_RPT,
                                  "%ud %ud %lf %lf %lf %lf %lf %lf",
                                  inetwork->get_LocalRank(),
                                  failed_rank,
                                  overall_timer._stop_d,
                                  new_parent_timer.get_latency_msecs(),
                                  connection_timer.get_latency_msecs(),
                                  filter_state_timer.get_latency_msecs(),
                                  cleanup_timer.get_latency_msecs(),
                                  overall_timer.get_latency_msecs() ) );

    mrn_dbg(3, mrn_printf( FLF, stderr, "ostart: %lf, oend: %lf\n",
                           overall_timer._start_d, overall_timer._stop_d ));
    mrn_dbg(3, mrn_printf( FLF, stderr, "pstart: %lf, cend: %lf\n",
                           new_parent_timer._start_d, cleanup_timer._stop_d ));


    mrn_dbg(3, mrn_printf( FLF, stderr, "%u %u %lf %lf %lf %lf %lf %lf\n",
                           inetwork->get_LocalRank(),
                           failed_rank,
                           overall_timer._stop_d,
                           new_parent_timer.get_latency_msecs(),
                           connection_timer.get_latency_msecs(),
                           filter_state_timer.get_latency_msecs(),
                           cleanup_timer.get_latency_msecs(),
                           overall_timer.get_latency_msecs() ) );

    mrn_dbg(3, mrn_printf( FLF, stderr, "Notifying FIS...\n"));
    int sock_fd=0;
    mrn_dbg(3, mrn_printf( FLF, stderr, "Sending recovery report to FIS:%s:%d ...\n",
                           inetwork->get_FailureManager()->get_HostName().c_str(),
                           inetwork->get_FailureManager()->get_Port() )); 
    if(connectHost( &sock_fd, inetwork->get_FailureManager()->get_HostName().c_str(),
                    inetwork->get_FailureManager()->get_Port() ) == -1){
        mrn_dbg(1, mrn_printf(FLF, stderr, "connectHost() failed\n"));
        return -1;
    }
    
    Message msg;
    msg.add_Packet( packet );
    if( msg.send( sock_fd ) == -1 ) {
        mrn_dbg(1, mrn_printf(FLF, stderr, "Message.send failed\n" ));
        return -1;
    }
    XPlat::SocketUtils::Close( sock_fd );
    mrn_dbg(3, mrn_printf( FLF, stderr, "Recovery report to FIS complete!\n"));

    mrn_dbg_func_end();

    return 0;
}

};
