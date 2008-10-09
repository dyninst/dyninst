/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#include "mrnet/MRNet.h"

using namespace MRN;

extern "C" {

const char * aggr_Count_format_string = "%d";
void aggr_Count( const std::vector< PacketPtr >& packets_in,
                 std::vector< PacketPtr >& packets_out,
                 std::vector< PacketPtr >&,
                 void ** /* client data */, PacketPtr& )
{
    int sum = 0;
    
    for( unsigned int i = 0; i < packets_in.size(  ); i++ ) {
        PacketPtr cur_packet = packets_in[i];
	int new_sum = 0;
	cur_packet->unpack(aggr_Count_format_string, &new_sum);
        fprintf(stderr, "FFF: Adding %d to sum\n", new_sum);
        sum += new_sum;
    }
    
    PacketPtr new_packet( new Packet(packets_in[0]->get_StreamId(),
                                     packets_in[0]->get_Tag(), 
				     aggr_Count_format_string, sum ) );
    packets_out.push_back( new_packet );
}

const char * aggr_CountOddsAndEvens_format_string = "%d %d";
void aggr_CountOddsAndEvens( const std::vector< PacketPtr >& packets_in,
                             std::vector< PacketPtr >& packets_out,
                             std::vector< PacketPtr >&,
                             void ** /* client data */, PacketPtr& )
{
    int odd_sum=0, even_sum = 0;
    
    for( unsigned int i = 0; i < packets_in.size(  ); i++ ) {
        PacketPtr cur_packet = packets_in[i];
	int new_odd = 0, new_even = 0;
	cur_packet->unpack(aggr_CountOddsAndEvens_format_string, &new_odd, &new_even);
        fprintf(stderr, "FFF: Adding %d to even and %d to odd\n", new_even, new_odd);
	odd_sum += new_odd;
	even_sum += new_even;
    }
    
    PacketPtr new_packet( new Packet(packets_in[0]->get_StreamId(),
                                     packets_in[0]->get_Tag(),
                                     aggr_CountOddsAndEvens_format_string, 
				     odd_sum, even_sum ) );
    packets_out.push_back( new_packet );
}

} /* extern "C" */
