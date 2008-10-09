/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#include "mrnet/MRNet.h"
#include "test_Recovery.h"
#include <ext/hash_set>
#include <stdlib.h>

using namespace MRN;
using namespace std;
using namespace __gnu_cxx;

extern "C" {

typedef struct {
    hash_set< uint32_t > elems;
    uint32_t npkts;
} EQClassStruct;

//number format: array of elements, # pkts processed
const char * uint32_EqClass_format_string = "%aud %ud";
void uint32_EqClass( vector < PacketPtr >&packets_in,
                     vector < PacketPtr >&packets_out,
                     void ** ifs )
{
    uint32_t npkts_procd=0;
    hash_set< uint32_t> new_elems;
    hash_set< uint32_t>::const_iterator iter;

    EQClassStruct * fs = (EQClassStruct *)*ifs;
    if( fs == NULL ) {
        //first call, allocate structure
        fs = new EQClassStruct;
        fs->npkts=0;

        *ifs = fs;
    }

    //fprintf( stderr, "Processing %u packets ...\n", packets_in.size() );
    for( uint32_t i=0; i < packets_in.size( ); i++ ) {
        uint32_t * cur_elems;
        uint32_t cur_nelems, cur_npkts;
        //fprintf(stderr, "%s(): packet #%u\n", __FUNCTION__, i+1 );

        //extract eq class from each packet
        packets_in[i]->unpack( "%aud %ud", &cur_elems, &cur_nelems, &cur_npkts );
        //fprintf( stderr, "npkts_proc'd: %u, cur_npkts: %u\n", npkts_procd, cur_npkts );
        npkts_procd += cur_npkts;

        //fprintf(stderr, "%s(): array size %u\n", __FUNCTION__, cur_nelems );
        for( uint32_t j=0; j<cur_nelems; j++ ) {
            uint32_t cur_elem;

            cur_elem=cur_elems[j];
            //if uint not found, insert in new and old list
            //fprintf(stderr, "%s(): new member: %u? ", __FUNCTION__, cur_elem );
            if( fs->elems.find( cur_elem ) == fs->elems.end() ){
                //fprintf(stderr, "yes\n" );
                new_elems.insert( cur_elem );
                fs->elems.insert( cur_elem );
            }
            else{
                //fprintf(stderr, "no\n" );
            }
        }
    }
    
    //create output packet from newly found class members
    uint32_t * new_elem_array;
    uint32_t new_elem_array_size, i;

    new_elem_array_size = new_elems.size();
    new_elem_array = new uint32_t [ new_elem_array_size ];

    //fprintf(stderr, "%s(): new members: ", __FUNCTION__ );
    for( iter=new_elems.begin(), i=0; iter!=new_elems.end(); iter++,i++ ){
        new_elem_array[i] = *iter;
        //fprintf(stderr, "%u ", new_elem_array[i] );
    }
    //fprintf(stderr, "\n" );

    PacketPtr new_packet( new Packet( packets_in[0]->get_StreamId( ),
                                      packets_in[0]->get_Tag( ),
                                      "%aud %ud",
                                      new_elem_array,
                                      new_elem_array_size,
                                      npkts_procd ) );
    packets_out.push_back( new_packet );
}

PacketPtr uint32_EqClass_get_state( void ** ifs, int istream_id )
{
    PacketPtr packet;

    //fprintf( stderr, "Getting filter state ...\n" );
    //extract previous equivalence class from filter state
    EQClassStruct * fs = (EQClassStruct *)*ifs;

    if( fs == NULL ) {
        fprintf( stderr, "No filter state!\n" );
        packet = Packet::NullPacket;
    }
    else{
        uint32_t *elems;
        elems = new uint32_t[ fs->elems.size() ];

        hash_set< uint32_t >::const_iterator iter;
        uint32_t i=0;
        for( iter=fs->elems.begin(); iter!=fs->elems.end(); iter++ ) {
            elems[ i ] = *iter;
            i++;
        }
        packet = PacketPtr( new Packet( istream_id, PROT_INT_EQCLASS,
                                        "%aud %ud",
                                        elems,
                                        fs->elems.size(),
                                        fs->npkts ));
    }

    return packet;
}

}
