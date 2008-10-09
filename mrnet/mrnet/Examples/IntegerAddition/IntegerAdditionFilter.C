/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#include "FilterDefinitions.h"
#include "mrnet/Packet.h"

using namespace MRN;

extern "C" {

const char * IntegerAdd_format_string = "%d";
void IntegerAdd( std::vector< PacketPtr >& packets_in,
                 std::vector< PacketPtr >& packets_out,
                 std::vector< PacketPtr >& /* packets_out_reverse */,
                 void ** /* client data */,
		 PacketPtr& /* params */ )
{
    int sum = 0;
    
    for( unsigned int i = 0; i < packets_in.size( ); i++ ) {
        PacketPtr cur_packet = packets_in[i];
	int val;
	cur_packet->unpack("%d", &val);
        sum += val;
    }
    
    PacketPtr new_packet ( new Packet(packets_in[0]->get_StreamId(),
                                      packets_in[0]->get_Tag(), "%d", sum ) );
    packets_out.push_back( new_packet );
}

} /* extern "C" */
