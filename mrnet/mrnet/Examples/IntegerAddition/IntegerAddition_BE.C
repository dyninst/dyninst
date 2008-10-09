/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#include "mrnet/MRNet.h"
#include "IntegerAddition.h"

using namespace MRN;

int main(int argc, char **argv)
{
    Stream * stream=NULL;
    PacketPtr p;
    int tag=0, recv_val=0, num_iters=0;

    Network * network = new Network( argc, argv );

    do{
        if ( network->recv(&tag, p, &stream) != 1){
            fprintf(stderr, "stream::recv() failure\n");
            return -1;
        }

        switch(tag){
        case PROT_SUM:
            p->unpack( "%d %d", &recv_val, &num_iters );

            // Send num_iters waves of integers
            for( unsigned int i=0; i<num_iters; i++ ){
                if( stream->send(tag, "%d", recv_val*i) == -1 ){
                    fprintf(stderr, "stream::send(%%d) failure\n");
                    return -1;
                }
                if( stream->flush( ) == -1 ){
                    fprintf(stderr, "stream::flush() failure\n");
                    return -1;
                }
            }
            break;

        case PROT_EXIT:
            fprintf( stdout, "Processing PROT_EXIT ...\n");
            break;

        default:
            fprintf(stdout, "Unknown Protocol: %d\n", tag);
            break;
        }
    } while ( tag != PROT_EXIT );

    return 0;
}
