#include "mrnet/src/FilterDefinitions.h"

using namespace MRN

const char * aggr_Count_format_string = "%d"
void aggr_Count( std::vector < Packet * >&packets_in,
                 std::vector < Packet * >&packets_out )
{
    int sum=0;
    
    for( unsigned int i = 0; i < packets_in.size( ); i++ ) {
        Packet *cur_packet = packets_in[i];
        sum += ( *cur_packet )[0]->get_int32_t(	);
    }
    
    Packet *new_packet = new Packet( packets_in[0]->get_Tag(  ),
                                     packets_in[0]->get_StreamId(  ),
                                     "%d", sum );
    packets_out.push_back( new_packet );
}

const char * aggr_CountOddsAndEvens_format_string = "%d"
void aggr_CountOddsAndEvens( std::vector < Packet * >&packets_in,
                             std::vector < Packet * >&packets_out )
{
    int numOdds=0, numEvens=0;
    
    for( unsigned int i = 0; i < packets_in.size( ); i++ ) {
        Packet *cur_packet = packets_in[i];
        numOdds += ( *cur_packet )[0]->get_int32_t(	);
        numEvens += ( *cur_packet )[1]->get_int32_t( );
    }
    
    Packet *new_packet = new Packet( packets_in[0]->get_Tag(  ),
                                     packets_in[0]->get_StreamId(  ),
                                     "%d %d", numOdds, numEvens );
    packets_out.push_back( new_packet );
}
