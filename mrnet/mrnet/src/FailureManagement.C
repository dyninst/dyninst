/****************************************************************************
 * Copyright © 2003-2008 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#include <map>
#include <set>

#include "mrnet/MRNet.h"
#include "mrnet/FailureManagement.h"
#include "CommunicationNode.h"
#include "Message.h"
#include "PeerNode.h"
#include "utils.h"
#include "xplat/Thread.h"

using namespace std;

extern char TopologyFileBasename[256];
namespace MRN {

map< Rank, FailureEvent * > FailureEvent::FailureEventMap;

static void waitFor_FailureRecoveryReports( int ilistening_sock_fd );
static NetworkTopology::Node * find_NodeToKill( NetworkTopology * );
static bool ExitFailureManager=false;
static set<Rank> OrphanRanksToReport;
static unsigned int NumFailures=0;
static unsigned int FailureFrequency=0;

static void * FailureInjectionThreadMain( void * iarg );
XPlat::Thread::Id FailureInjectionThreadId;

int start_FailureManager( Network * inetwork  )
{
    //start main thread for injecting failures
    mrn_dbg( 5, mrn_printf(FLF, stderr, "Creating Failure Injection thread ...\n"));
    int retval = XPlat::Thread::Create( FailureInjectionThreadMain, inetwork,
                                        &FailureInjectionThreadId );

    if(retval != 0){
        mrn_dbg( 1, mrn_printf(FLF, stderr, "Thread creation failed...\n" ));
        return -1;
    }

    return 0;
}

int stop_FailureManager( )
{
    ExitFailureManager=true;
    return 0;
}

void set_NumFailures( unsigned int infailures )
{
    NumFailures=infailures;
}

void set_FailureFrequency( unsigned int ifailure_frequency )
{
    FailureFrequency=ifailure_frequency;
}

int waitFor_FailureManager( )
{
    if( XPlat::Thread::Join( FailureInjectionThreadId, NULL ) != 0 ){
        fprintf( stderr, "Thread::Join() failed...\n" );
        return -1;
    }

    return 0;
}

void * FailureInjectionThreadMain( void * /* iarg */ )
{
    //Network * network = (Network *) iarg;
    NetworkTopology * topology;
    NetworkTopology::Node * node_to_kill;
    set<NetworkTopology::Node *> orphans;

    srand(0);

    //TLS: setup FIS thread local storage
    tsd_t * local_data = new tsd_t;
    local_data->thread_id = XPlat::Thread::GetId();
    local_data->thread_name = strdup("FIS");

    int status;
    mrn_dbg( 3, mrn_printf(FLF, stderr, "Setting local thread storage ...\n"));
    if( (status = tsd_key.Set( local_data )) != 0){
        fprintf(stderr, "XPlat::TLSKey::Set(): %s\n", strerror(status)); 
        return NULL;
    }

    //setup listening socket for failure event reporting from tree processes
    int listening_sock_fd;
    Port listening_port=FAILURE_REPORTING_PORT;
    mrn_dbg( 3, mrn_printf(FLF, stderr, "Binding to port %d ...\n", listening_port ));
    if( bindPort( &listening_sock_fd, &listening_port ) == -1 ) {
        mrn_dbg( 1, mrn_printf(FLF, stderr, "bind_to_port() failed\n" ));
        return NULL;
    }

    char filename[256];
    unsigned int nfailures=0;
    while ( nfailures < NumFailures && !ExitFailureManager ){

        //fprintf( stderr, "Sleeping for %u seconds\n", FailureFrequency );
        sleep(FailureFrequency);
        topology = network->get_NetworkTopology();

        //Print .dot and .top files
        snprintf( filename, sizeof(filename), "%s.%u.dot",
                  TopologyFileBasename, nfailures );
        topology->print_DOTGraph( filename );

        snprintf( filename, sizeof(filename), "%s.%u.top",
                  TopologyFileBasename, nfailures );
        topology->print_TopologyFile( filename );

        mrn_dbg( 3, mrn_printf(FLF, stderr, "Finding node to kill ...\n"));
        node_to_kill = find_NodeToKill( topology );
        mrn_dbg( 3, mrn_printf(FLF, stderr, "node_to_kill: %s:%d:%d\n",
                               node_to_kill->get_HostName().c_str(),
                               node_to_kill->get_Rank(),
                               node_to_kill->get_Port() ));

        //add children of failed rank to orphans_to_report list
        orphans = node_to_kill->get_Children();
        set<NetworkTopology::Node *>::iterator iter;
        for( iter=orphans.begin(); iter!=orphans.end(); iter++ ) {
            mrn_dbg( 3, mrn_printf(FLF, stderr, "Add to orphan list: %s:%d:%d\n",
                                   (*iter)->get_HostName().c_str(),
                                   (*iter)->get_Rank(),
                                   (*iter)->get_Port() ));
            OrphanRanksToReport.insert( (*iter)->get_Rank() );
        }

        nfailures++;
        mrn_dbg( 3, mrn_printf(FLF, stderr, "Injecting failure #%d...\n", nfailures));
        fprintf( stderr, "Injecting failure #%d: node[%d]...\n",
                 nfailures, node_to_kill->get_Rank() );
        inject_Failure( node_to_kill );

        mrn_dbg( 3, mrn_printf(FLF, stderr, "Waiting for %u recovery reports ...\n",
                               OrphanRanksToReport.size() ));
        waitFor_FailureRecoveryReports( listening_sock_fd );
    }

    sleep( 10 );
    //Print .dot and .top files
    snprintf( filename, sizeof(filename), "%s.%u.dot",
              TopologyFileBasename, nfailures );
    topology->print_DOTGraph( filename );
    
    snprintf( filename, sizeof(filename), "%s.%u.top",
              TopologyFileBasename, nfailures );
    topology->print_TopologyFile( filename );

    return NULL;
}

int inject_Failure( NetworkTopology::Node * inode )
{
    mrn_dbg_func_begin();
    FailureEvent * failure_event = new FailureEvent( inode->get_Rank() );

    //connect to node and tell to die
    int sock_fd=0;
    mrn_dbg( 1, mrn_printf(FLF, stderr, "Connecting to victim: %s:%d:%d\n",
                           inode->get_HostName().c_str(),
                           inode->get_Rank(),
                           inode->get_Port() ));
    if(connectHost(&sock_fd, inode->get_HostName().c_str(), inode->get_Port() ) == -1){
        mrn_dbg(1, mrn_printf(FLF, stderr, "connect_to_host() failed\n"));
        return -1;
    }
    
    PacketPtr packet( new Packet( 0, PROT_KILL_SELF, "" ) );

    Message msg;
    msg.add_Packet( packet );
    if( msg.send( sock_fd ) == -1 ) {
        mrn_dbg(1, mrn_printf(FLF, stderr, "Message.send failed\n" ));
        return -1;
    }

    mrn_dbg( 1, mrn_printf(FLF, stderr, "victim killed\n"));

    //TODO: get ack for better timing
    failure_event->_timer.start();
    close( sock_fd );

    mrn_dbg_func_end();
    return 0;
}

void waitFor_FailureRecoveryReports( int isock_fd )
{
    mrn_dbg_func_begin();
    //Prepare fds for select()
    fd_set rfds, rfds_copy;
    FD_ZERO( &rfds );
    FD_SET( isock_fd, &rfds );

    //unsigned int num_orphans = OrphanRanksToReport.size();
    list< PacketPtr > packets;
    Message msg;
    double max_recovery_timestamp=0;
    do {
        rfds_copy = rfds;
        mrn_dbg( 5, mrn_printf(FLF, stderr, "Orphan count: %u ...\n",
                               OrphanRanksToReport.size() ));
        mrn_dbg( 5, mrn_printf(FLF, stderr, "Blocking for reports\n"));
        select( isock_fd+1, &rfds_copy, NULL, NULL, NULL );

        //Activity on our local listening sock, accept connection
        mrn_dbg( 5, mrn_printf(FLF, stderr, "Activity on listening socket ...\n"));
        int connected_sock = getSocketConnection( isock_fd );
        if( connected_sock == -1 ){
            mrn_dbg( 1, mrn_printf(FLF, stderr, "getSocketConnection() failed\n"));
            perror("getSocketConnection()");
            continue;
        }

        packets.clear();
        msg.recv( connected_sock, packets, UnknownRank );
        list< PacketPtr >::iterator packet_list_iter;

        for( packet_list_iter = packets.begin();
             packet_list_iter != packets.end();
             packet_list_iter++  ) {
            PacketPtr cur_packet( *packet_list_iter );
            assert ( cur_packet->get_Tag() == PROT_RECOVERY_RPT );
            Rank orphan_rank;
            Rank failed_parent_rank;
            double recovery_timestamp;
            double lat_compute_new_parent;
            double lat_establish_connection;
            double lat_send_filter_state;
            double lat_cleanup;
            double lat_overall;
            cur_packet->unpack( "%ud %ud %lf %lf %lf %lf %lf %lf",
                                &orphan_rank,
                                &failed_parent_rank,
                                &recovery_timestamp,
                                &lat_compute_new_parent,
                                &lat_establish_connection,
                                &lat_send_filter_state,
                                &lat_cleanup,
                                &lat_overall );

            mrn_dbg( 5, mrn_printf(FLF, stderr, "Recovery report from %d\n",
                                   orphan_rank));
            mrn_dbg( 5, mrn_printf(FLF, stderr, "Orphan count: %u, erasing %d ...\n",
                                   OrphanRanksToReport.size(), orphan_rank ));
            OrphanRanksToReport.erase( orphan_rank );
            mrn_dbg( 5, mrn_printf(FLF, stderr, "Orphan count: %u after erasing %d ...\n",
                                   OrphanRanksToReport.size(), orphan_rank ));
            FailureEvent::RecoveryReport * report =
                new FailureEvent::RecoveryReport( orphan_rank,
                                                  failed_parent_rank,
                                                  lat_compute_new_parent,
                                                  lat_establish_connection,
                                                  lat_send_filter_state,
                                                  lat_cleanup,
                                                  lat_overall );

            map<Rank, FailureEvent *>::const_iterator failure_event_iter =
                FailureEvent::FailureEventMap.find( failed_parent_rank );
            assert( failure_event_iter != FailureEvent::FailureEventMap.end() );

            FailureEvent *failure_event = (*failure_event_iter).second;
            failure_event->_recovery_reports.push_back( report );

            if( recovery_timestamp > max_recovery_timestamp ) {
                max_recovery_timestamp = recovery_timestamp;
                failure_event->_timer.stop( recovery_timestamp );
            }
            //failure_event->_timer.stop();
        }
        close( connected_sock ); //transient connections
    } while ( !OrphanRanksToReport.empty() );

    //char statfile[256];
    //snprintf( statfile, sizeof(statfile),
              //"failure.stats.%u", num_orphans );
    //FailureEvent::dump_FailureRecoveryStatistics( statfile );
    mrn_dbg_func_end();
}

NetworkTopology::Node * find_NodeToKill( NetworkTopology * itopology )
{

    itopology->print( NULL );
    set<NetworkTopology::Node*> parent_nodes = itopology->get_ParentNodes();

    //for now, pick non-root parent node w/ most children
    set<NetworkTopology::Node*> ::iterator iter;
    unsigned int max_children=0;
    NetworkTopology::Node* node_to_kill=NULL;
    for( iter=parent_nodes.begin(); iter!=parent_nodes.end(); iter++ ){
        if( *iter == itopology->get_Root() )
            continue;
        if( (*iter)->get_NumChildren() > max_children ) {
            node_to_kill = *iter;
            max_children = (*iter)->get_NumChildren();
        }
    }
    return node_to_kill;

    //unsigned int rand_slot;
    //pick any random parent except root
    //do {
        //rand_slot = rand() % parent_nodes.size();
    //} while ( itopology->get_Root() != parent_nodes[ rand_slot ] );
    //return parent_nodes[ rand_slot ];
}

// FailureEvent Class Definition

FailureEvent::FailureEvent( Rank irank ): _failed_rank( irank ) {
    FailureEventMap[ irank ] = this;
}

int FailureEvent::dump_FailureRecoveryStatistics( const char * ifilename )
{
    FILE * fp = fopen( ifilename, "w" );
    if( !fp ) {
        perror( "fopen()" );
        return -1;
    }

    map< Rank, FailureEvent * >::const_iterator iter;
    for( iter=FailureEventMap.begin(); iter!=FailureEventMap.end(); iter++ ) {
        (*iter).second->dump_FailureRecoveryStatistics( fp );
    }

    fclose( fp );

    return 0;
}

void FailureEvent::dump_FailureRecoveryStatistics( FILE * ifp )
{
    double fm_latency = _timer.get_latency_msecs();
    FailureEvent::RecoveryReport * max_report;
    double tlat_overall=0;
    double tlat_filter_state=0;
    double tlat_connection=0;
    double tlat_new_parent=0;
    double tlat_cleanup=0;
    unsigned int nreports = _recovery_reports.size();

    max_report = _recovery_reports[0];
    for( unsigned int i=0; i < _recovery_reports.size(); i++ ) {
        if( _recovery_reports[i]->_lat_overall > max_report->_lat_overall ) {
            max_report = _recovery_reports[i];
        }
        tlat_overall += _recovery_reports[i]->_lat_overall;
        tlat_new_parent += _recovery_reports[i]->_lat_new_parent;
        tlat_connection += _recovery_reports[i]->_lat_connection;
        tlat_filter_state += _recovery_reports[i]->_lat_filter_state;
        tlat_cleanup += _recovery_reports[i]->_lat_cleanup;
    }

    fprintf( ifp, "    | %4.4s %5.5s %8.8s %8.8s %8.8s %8.8s %8.8s %8.8s\n",
             "Rank",
             "Orphs",
             "GOverall",
             "LOverall",
             "NParent",
             "Connect",
             "FState",
             "Cleanup" );

    fprintf( ifp, "----+----------------------------------------------------------------------\n"
             "MAX | %4u %5u %8.3lf %8.3lf %8.3lf %8.3lf %8.3lf %8.3lf\n",
             _failed_rank,
             max_report->_orphan_rank,
             fm_latency,
             max_report->_lat_overall,
             max_report->_lat_new_parent,
             max_report->_lat_connection,
             max_report->_lat_filter_state,
             max_report->_lat_cleanup );

    fprintf( ifp, "AVG | %4u %5u %8.3lf %8.3lf %8.3lf %8.3lf %8.3lf %8.3lf\n\n",
             _failed_rank,
             nreports,
             fm_latency,
             tlat_overall / nreports,
             tlat_new_parent / nreports,
             tlat_connection / nreports,
             tlat_filter_state / nreports,
             tlat_cleanup / nreports );

    for( unsigned int i=0; i < _recovery_reports.size(); i++ ) {
        if( i%20 == 0 ) {
            fprintf( ifp,
                     "  Parent  |  Orphan  | NParent  |  Connect |  FState  | Cleanup  |  Total  \n"
                     "==========+==========+==========+==========+==========+==========+=========\n" );
        }


        fprintf( ifp,
                 " %8u | %8u | %8.3lf | %8.3lf | %8.3lf | %8.3lf | %8.3lf\n",
                 _recovery_reports[i]->_failed_parent_rank,
                 _recovery_reports[i]->_orphan_rank,
                 _recovery_reports[i]->_lat_new_parent,
                 _recovery_reports[i]->_lat_connection,
                 _recovery_reports[i]->_lat_filter_state,
                 _recovery_reports[i]->_lat_cleanup,
                 _recovery_reports[i]->_lat_overall );
    }
}

} /* namespace MRN */
