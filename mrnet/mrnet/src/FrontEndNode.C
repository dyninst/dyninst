/****************************************************************************
 * Copyright  2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller  *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#include <stdio.h>
#include "mrnet/MRNet.h"
#include "mrnet/NetworkTopology.h"
#include "FrontEndNode.h"
#include "utils.h"
#include "EventDetector.h"
#include "Router.h"

namespace MRN
{
/*===============================================*/
/*  FrontEndNode CLASS METHOD DEFINITIONS        */
/*===============================================*/
FrontEndNode::FrontEndNode( Network * inetwork, std::string const& ihostname, Rank irank )
    : ParentNode( inetwork, ihostname, irank )
{
    mrn_dbg_func_begin();

    _network->set_LocalHostName( _hostname  );
    _network->set_LocalPort( _port );
    _network->set_LocalRank( _rank );
    _network->set_FrontEndNode( this );
    _network->set_NetworkTopology( new NetworkTopology( inetwork, _hostname, _port, _rank ));
    _network->set_FailureManager( new CommunicationNode( _hostname, _port, _rank ) );
    
    mrn_dbg( 5, mrn_printf(FLF, stderr, "start_EventDetectionThread() ...\n" ));
    if( EventDetector::start( _network ) == -1 ){
        mrn_dbg( 1, mrn_printf(FLF, stderr, "start_EventDetectionThread() failed\n" ));
        error( MRN_ESYSTEM, "start_EventDetectionThread failed\n" );
        return;
    }
    mrn_dbg_func_end();
}

FrontEndNode::~FrontEndNode(  )
{
}

int FrontEndNode::proc_DataFromChildren( PacketPtr ipacket ) const
{
    mrn_dbg_func_begin();

    Stream *stream = _network->get_Stream( ipacket->get_StreamId() );
    std::vector < PacketPtr > packets, reverse_packets;

    stream->push_Packet( ipacket, packets, reverse_packets, true );

    mrn_dbg( 3, mrn_printf
             (FLF, stderr, "push_packet => %u packets, %u reverse_packets\n", 
              packets.size(), reverse_packets.size() ));

    if( !packets.empty() ) {
        for( unsigned int i = 0; i < packets.size( ); i++ ) {
            PacketPtr cur_packet( packets[i] );

            mrn_dbg( 3, mrn_printf(FLF, stderr, "Put packet in stream %d\n",
                                   cur_packet->get_StreamId(  ) ));
            stream->add_IncomingPacket( cur_packet );
        }
    }
    if( !reverse_packets.empty() ) {
        if( _network->send_PacketsToChildren( reverse_packets ) == -1 ){
            mrn_dbg( 1, mrn_printf(FLF, stderr, "send_PacketsToChildren() failed()\n" ));
            return -1;
        }
    }

    mrn_dbg_func_end();
    return 0;
}

int FrontEndNode::proc_NewParentReportFromParent( PacketPtr ipacket ) const
{
    Rank child_rank, parent_rank;

    ipacket->unpack( "%ud &ud", &child_rank, &parent_rank ); 

    //update local topology
    _network->change_Parent( child_rank, parent_rank );

    //propagate to children except on incident channel
    if( _network->send_PacketToChildren( ipacket, false ) == -1 ){
        mrn_dbg( 1, mrn_printf(FLF, stderr, "send_PacketToChildren() failed()\n" ));
        return -1;
    }

    return 0;
}

}    // namespace MRN
