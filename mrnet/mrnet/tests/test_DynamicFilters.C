#include "mrnet/src/FilterDefinitions.h"
#include "mrnet/src/Packet.h"

using namespace MRN;

extern "C" {

const char * aggr_Count_format_string = "%d";
void aggr_Count( std::vector < Packet >&packets_in,
                 std::vector < Packet >&packets_out,
                 void ** /* client data */ )
{
    int sum = 0;
    
    for( unsigned int i = 0; i < packets_in.size(  ); i++ ) {
        Packet cur_packet = packets_in[i];
        fprintf(stderr, "FFF: Adding %d to sum\n", cur_packet[0].get_int32_t());
        sum += cur_packet[0].get_int32_t( );
    }
    
    Packet new_packet ( packets_in[0].get_StreamId(  ),
                        packets_in[0].get_Tag(  ), "%d", sum );
    packets_out.push_back( new_packet );
}

const char * aggr_CountOddsAndEvens_format_string = "%d %d";
void aggr_CountOddsAndEvens( std::vector < Packet >&packets_in,
                             std::vector < Packet >&packets_out,
                             void ** /* client data */ )
{
    int odd_sum=0, even_sum = 0;
    
    for( unsigned int i = 0; i < packets_in.size(  ); i++ ) {
        Packet cur_packet = packets_in[i];
        odd_sum += cur_packet[0].get_int32_t(	);
        even_sum += cur_packet[1].get_int32_t( );
        fprintf(stderr, "FFF: Adding %d to even and %d to odd\n", cur_packet[1].get_int32_t(), cur_packet[0].get_int32_t());
    }
    
    Packet new_packet( packets_in[0].get_StreamId(  ),
                       packets_in[0].get_Tag(  ),
                       "%d %d", odd_sum, even_sum );
    packets_out.push_back( new_packet );
}

} /* extern "C" */
