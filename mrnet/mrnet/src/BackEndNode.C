/****************************************************************************
 * Copyright  2003-2008 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#include <stdio.h>

#include "mrnet/MRNet.h"
#include "mrnet/NetworkTopology.h"
#include "BackEndNode.h"
#include "PeerNode.h"
#include "utils.h"
#include "EventDetector.h"
#include "Router.h"
#include "Filter.h"

namespace MRN
{

/*=====================================================*/
/*  BackEndNode CLASS METHOD DEFINITIONS            */
/*=====================================================*/

BackEndNode::BackEndNode( Network * inetwork, 
                          std::string imyhostname, Rank imyrank,
                          std::string iphostname, Port ipport, Rank iprank )
    :ChildNode( inetwork, imyhostname, imyrank, iphostname, ipport, iprank )
{
    _network->set_LocalHostName( _hostname  );
    _network->set_LocalRank( _rank );
    _network->set_BackEndNode( this );
    _network->set_NetworkTopology( new NetworkTopology( inetwork, _hostname, _port, _rank, true ) );

    //establish data connection w/ parent
    if( init_newChildDataConnection( _network->get_ParentNode() ) == -1 ) {
        mrn_dbg( 1, mrn_printf(FLF, stderr,
                               "init_newChildDataConnection() failed\n" ));
        return;
    }

    //start event detection thread
    if( ! EventDetector::start( _network ) ) {
      mrn_dbg( 1, mrn_printf(FLF, stderr, "start_EventDetector() failed\n" ));
      error( MRN_ESYSTEM, "start_EventDetector failed\n" );
      return;
    }

    //send new subtree report
    mrn_dbg( 5, mrn_printf(FLF, stderr, "Sending new child report.\n" ));
    if( send_NewSubTreeReport( ) == -1 ) {
        mrn_dbg( 1, mrn_printf(FLF, stderr,
                               "send_newSubTreeReport() failed\n" ));
    }
    mrn_dbg( 5, mrn_printf(FLF, stderr,
                           "send_newSubTreeReport() succeded!\n" ));
}

BackEndNode::~BackEndNode(void)
{
}

int BackEndNode::proc_DataFromParent(PacketPtr ipacket) const
{
    Stream * stream;

    stream = _network->get_Stream( ipacket->get_StreamId() );
    assert(  stream );

    stream->add_IncomingPacket(ipacket);

    return 0;
}

int BackEndNode::proc_newStream( PacketPtr ipacket ) const
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
                                 &us_filter_id, &sync_id, &ds_filter_id) == -1 ) {
        mrn_dbg( 1, mrn_printf(FLF, stderr, "ExtractArgList() failed\n" ));
        return -1;
    }

    //register new stream
    _network->new_Stream( stream_id, backends, num_backends,
                          us_filter_id, sync_id, ds_filter_id );

    mrn_dbg_func_end();
    return 0;
}

int BackEndNode::proc_DownstreamFilterParams( PacketPtr &ipacket ) const
{
    int stream_id;

    mrn_dbg_func_begin();

    stream_id = ipacket->get_StreamId();
    Stream* strm = _network->get_Stream( stream_id );
    strm->set_FilterParams( false, ipacket );

    mrn_dbg_func_end();
    return 0;
}

int BackEndNode::proc_UpstreamFilterParams( PacketPtr &ipacket ) const
{
    int stream_id;

    mrn_dbg_func_begin();

    stream_id = ipacket->get_StreamId();
    Stream* strm = _network->get_Stream( stream_id );
    strm->set_FilterParams( true, ipacket );

    mrn_dbg_func_end();
    return 0;
}

int BackEndNode::proc_DeleteSubTree( PacketPtr ipacket ) const
{
    mrn_dbg_func_begin();

    //processes will be exiting -- disable failure recovery
    _network->disable_FailureRecovery();

    //Send ack to parent
    if( !_network->get_LocalChildNode()->ack_DeleteSubTree() ) {
        mrn_dbg( 1, mrn_printf(FLF, stderr, "ack_DeleteSubTree() failed\n" ));
    }

    char delete_backend;

    ipacket->unpack( "%c", &delete_backend );

    //kill all mrnet threads
    _network->cancel_IOThreads();

    if( delete_backend == 't' ) {
        mrn_dbg( 1, mrn_printf(FLF, stderr, "Back-end exiting ... \n" ));
        exit(0);
    }

    EventDetector::stop();
   
    mrn_dbg_func_end();
    return 0;
}

int BackEndNode::proc_FailureReportFromParent( PacketPtr ipacket ) const
{
    Rank failed_rank;

    ipacket->unpack( "%uhd", &failed_rank ); 

    _network->remove_Node( failed_rank );

    return 0;
}

int BackEndNode::proc_NewParentReportFromParent( PacketPtr ipacket ) const
{
    Rank child_rank, parent_rank;

    ipacket->unpack( "%ud &ud", &child_rank, &parent_rank ); 

    _network->change_Parent( child_rank, parent_rank );

    return 0;
}

int BackEndNode::proc_newFilter( PacketPtr ipacket ) const
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

    mrn_dbg_func_end();
    return fid;
}

} // namespace MRN
