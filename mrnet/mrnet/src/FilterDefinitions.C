#include <map>

#include "mrnet/src/FilterDefinitions.h"
#include "mrnet/src/utils.h"

namespace MRN
{

/*==========================================*
 *    Default Aggregator Definitions        *
 *==========================================*/
void aggr_Int_Sum( std::vector < Packet * >&packets_in,
                   std::vector < Packet * >&packets_out,
                   void ** /* client data */ )
{
    int sum = 0;
    
    for( unsigned int i = 0; i < packets_in.size(  ); i++ ) {
        Packet *cur_packet = packets_in[i];
        sum += ( *cur_packet )[0]->get_int32_t(	 );
    }
    
    Packet *new_packet = new Packet( packets_in[0]->get_Tag(  ),
                                     packets_in[0]->get_StreamId(  ),
                                     "%d", sum );
    packets_out.push_back( new_packet );
}

#if (0)

void aggr_Float_Avg( DataElement ** in_elems, unsigned int in_count,
                     DataElement *** out_elems, unsigned int *out_count )
{
    float avg = 0;
    
    mrn_printf( 3, MCFL, stderr, "averaging: [" );
    for( unsigned int i = 0; i < in_count; i++ ) {
        mrn_printf( 3, 0, 0, stderr, "%f, ", in_elems[i][0].val.f );
        avg += in_elems[i][0].val.f;
        }
        mrn_printf( 3, 0, 0, stderr, "]\n" );

        avg /= ( float )in_count;

        *out_count = 1;
        ( *out_elems ) = new DataElement *[1];
        ( *out_elems )[0] = new DataElement;
        ( *out_elems )[0][0].val.f = avg;
        ( *out_elems )[0][0].type = FLOAT_T;
    }

    void aggr_Float_Max( DataElement ** in_elems, unsigned int in_count,
                         DataElement *** out_elems, unsigned int *out_count )
    {
        double max = 0;

        mrn_printf( 3, MCFL, stderr, "max'ing: [" );
        for( unsigned int i = 0; i < in_count; i++ ) {
            mrn_printf( 3, 0, 0, stderr, "%lf, ", in_elems[i][0].val.lf );
            if( in_elems[i][0].val.lf > max ) {
                max = in_elems[i][0].val.lf;
            }
        }
        mrn_printf( 3, 0, 0, stderr, "] => %lf\n", max );


        *out_count = 1;
        ( *out_elems ) = new DataElement *[1];
        ( *out_elems )[0] = new DataElement;
        ( *out_elems )[0][0].val.lf = max;
        ( *out_elems )[0][0].type = DOUBLE_T;
    }


    void aggr_CharArray_Concat( DataElement ** in_elems, unsigned int in_count,
                                DataElement *** out_elems,
                                unsigned int *out_count )
    {
        int result_array_size = 0;
        char *result_array;

        for( unsigned int i = 0; i < in_count; i++ ) {
            result_array_size += in_elems[i][0].array_len;
        }
        result_array = ( char * )malloc( result_array_size * sizeof( char ) );

        int pos = 0;
        for( unsigned int i = 0; i < in_count; i++ ) {
            memcpy( result_array + pos, in_elems[i][0].val.p,
                    in_elems[i][0].array_len );
            pos += in_elems[i][0].array_len;
        }

        *out_count = 1;
        ( *out_elems ) = new DataElement *[1];
        ( *out_elems )[0] = new DataElement;
        ( *out_elems )[0][0].val.p = result_array;
        ( *out_elems )[0][0].type = CHAR_ARRAY_T;
    }



    void aggr_IntEqClass( DataElement ** in_elems, unsigned int in_count,
                          DataElement *** out_elems, unsigned int *out_count )
    {
        std::map < unsigned int, std::vector < unsigned int >>classes;

        // find equivalence classes across our input 
        for( unsigned int i = 0; i < in_count; i++ ) {
            unsigned int *vals = ( unsigned int * )( in_elems[i][0].val.p );
            unsigned int *memcnts = ( unsigned int * )( in_elems[i][1].val.p );
            unsigned int *mems = ( unsigned int * )( in_elems[i][2].val.p );

            assert( in_elems[i][0].array_len == in_elems[i][1].array_len );
            unsigned int curClassMemIdx = 0;
            for( unsigned int j = 0; j < in_elems[i][0].array_len; j++ ) {
                mrn_printf( 3, MCFL, stderr,
                            "\tclass %d: val = %u, nMems = %u, mems = ", j,
                            vals[j], memcnts[j] );

                // update the members for the current class
                for( unsigned int k = 0; k < memcnts[j]; k++ ) {
                    mrn_printf( 3, MCFL, stderr, "%d ",
                                mems[curClassMemIdx + k] );
                    classes[vals[j]].push_back( mems[curClassMemIdx + k] );
                }
                mrn_printf( 3, MCFL, stderr, "\n" );
                curClassMemIdx += memcnts[j];
            }
        }

        // build data structures for the output 
        unsigned int *values = new unsigned int[classes.size(  )];
        unsigned int *memcnts = new unsigned int[classes.size(  )];
        unsigned int nMems = 0;
        unsigned int curIdx = 0;
        for( std::map < unsigned int,
                 std::vector < unsigned int >>::iterator iter = classes.begin(  );
             iter != classes.end(  ); iter++ ) {
            values[curIdx] = iter->first;
            memcnts[curIdx] = ( iter->second ).size(  );
            nMems += memcnts[curIdx];
            curIdx++;
        }
        unsigned int *mems = new unsigned int[nMems];
        unsigned int curMemIdx = 0;
        unsigned int curClassIdx = 0;
        for( std::map < unsigned int,
                 std::vector < unsigned int >>::iterator iter = classes.begin(  );
             iter != classes.end(  ); iter++ ) {
            for( unsigned int j = 0; j < memcnts[curClassIdx]; j++ ) {
                mems[curMemIdx] = ( iter->second )[j];
                curMemIdx++;
            }
            curClassIdx++;
        }

        // dump the output classes
        *out_count = 1;
        ( *out_elems ) = new DataElement *[1];
        ( *out_elems )[0] = new DataElement[3];

        // values
        ( *out_elems )[0][0].type = UINT32_ARRAY_T;
        ( *out_elems )[0][0].array_len = classes.size(  );
        ( *out_elems )[0][0].val.p = values;

        // member counts
        ( *out_elems )[0][1].type = UINT32_ARRAY_T;
        ( *out_elems )[0][1].array_len = classes.size(  );
        ( *out_elems )[0][1].val.p = memcnts;

        // members
        ( *out_elems )[0][2].type = UINT32_ARRAY_T;
        ( *out_elems )[0][2].array_len = nMems;
        ( *out_elems )[0][2].val.p = mems;

        mrn_printf( 3, MCFL, stderr, "aggrIntEqClass: returning\n" );
        unsigned int curMem = 0;
        for( unsigned int i = 0; i < classes.size(  ); i++ ) {
            mrn_printf( 3, MCFL, stderr,
                        "\tclass %d: val = %u, nMems = %u, mems = ", i,
                        ( ( unsigned int * )( ( *out_elems )[0][0].val.p ) )[i],
                        ( ( unsigned int * )( ( *out_elems )[0][1].val.
                                              p ) )[i] );
            for( unsigned int j = 0;
                 j < ( ( unsigned int * )( ( *out_elems )[0][1].val.p ) )[i];
                 j++ ) {
                mrn_printf( 3, MCFL, stderr, "%d ",
                            ( ( unsigned int * )( ( *out_elems )[0][2].val.
                                                  p ) )[curMem] );
                curMem++;
            }
            mrn_printf( 3, MCFL, stderr, "\n" );
        }
    }
#endif

/*============================================*
 *    Default SyncFilter Definitions        *
 *============================================*/

void sync_WaitForAll( std::vector < Packet * >&packets_in,
                      std::vector < Packet * >&packets_out,
                      std::list < RemoteNode * >&downstream_nodes,
                      void **local_storage )
{
    std::map < RemoteNode *, std::list < Packet * >*>*PacketListByNode;
    
    mrn_printf( 3, MCFL, stderr, "In sync_WaitForAll()\n" );
    if( *local_storage == NULL ) {
        PacketListByNode =
            new std::map < RemoteNode *, std::list < Packet * >*>;
        *local_storage = PacketListByNode;

        std::list < RemoteNode * >::iterator iter;
        mrn_printf( 3, MCFL, stderr,
                    "Creating Map of %d downstream_nodes\n",
                    downstream_nodes.size(  ) );
        for( iter = downstream_nodes.begin(  );
             iter != downstream_nodes.end(  ); iter++ ) {
            ( *PacketListByNode )[( *iter )] = new std::list < Packet * >;
        }
    }
    else {
        PacketListByNode =
            ( std::map < RemoteNode *, std::list < Packet * >*>* )
            * local_storage;
    }

    //place all incoming packets in appropriate list
    mrn_printf( 3, MCFL, stderr, "Placing %d incoming packets\n",
                packets_in.size(  ) );
    for( unsigned int i=0; i<packets_in.size(); i++ ){
        ( ( *PacketListByNode )[ packets_in[i]->inlet_node] )->
            push_back(  packets_in[i] );
    }
    packets_in.clear(  );

    //check to see if all lists have at least one packet, "a wave"
    mrn_printf( 3, MCFL, stderr,
                "Checking if all downstream_nodes are ready ..." );
    std::map < RemoteNode *, std::list < Packet * >*>::iterator iter2;
    for( iter2 = PacketListByNode->begin(  );
         iter2 != PacketListByNode->end(  ); iter2++ ) {
        if( ( ( *iter2 ).second )->size(  ) == 0 ) {
            //all lists not ready!
            mrn_printf( 3, 0, 0, stderr, "no!\n" );
            return;
        }
    }
    mrn_printf( 3, 0, 0, stderr, "yes!\n" );

    mrn_printf( 3, MCFL, stderr, "Placing outgoing packets\n" );
    //if we get here, all lists ready. push front of all lists onto "packets_out"
    for( iter2 = PacketListByNode->begin(  );
         iter2 != PacketListByNode->end(  ); iter2++ ) {
        packets_out.push_back( ( ( *iter2 ).second )->front(  ) );
        ( ( *iter2 ).second )->pop_front(  );
    }
    mrn_printf( 3, MCFL, stderr, "Returning %d outgoing packets\n",
                packets_out.size(  ) );
}

void sync_TimeOut( std::vector < Packet * >&packets_in,
                   std::vector < Packet * >&packets_out,
                   std::list < RemoteNode * >&, void **local_storage )
{
}

} /* namespace MRN */
