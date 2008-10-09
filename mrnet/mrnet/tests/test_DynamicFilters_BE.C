/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#include <stdlib.h>
#include <time.h>

#include "mrnet/MRNet.h"
#include "test_common.h"
#include "test_DynamicFilters.h"

using namespace MRN;
using namespace MRN_test;

int main(int argc, char **argv)
{
    Stream * stream;
    PacketPtr buf;
    int tag;
    bool success=true;

    srandom( time(NULL) ); //arbitrary seed to random()

    Network * network = new Network( argc, argv );

    do{
        if ( network->recv(&tag, buf, &stream) != 1){
            fprintf(stderr, "stream::recv() failure\n");
        }

        switch(tag){
        case PROT_COUNT:
            fprintf( stdout, "Processing PROT_COUNT ...\n");
            if( stream->send(tag, "%d", 1) == -1 ){
                fprintf(stderr, "stream::send(%%d) failure\n");
                success=false;
            }
            break;
        case PROT_COUNTODDSANDEVENS:
            fprintf( stdout, "Processing PROT_COUNTODDSANDEVENS ...\n");
            int odd, even;
            if( random() % 2 ){
                even=1, odd=0;
            }
            else{
                even=0, odd=1;
            }
            if( stream->send(tag, "%d %d", odd, even) == -1 ){
                fprintf(stderr, "stream::send(%%d) failure\n");
                success=false;
            }
            break;
        case PROT_EXIT:
            fprintf( stdout, "Processing PROT_EXIT ...\n");
            break;
        default:
            fprintf(stdout, "Unknown Protocol: %d\n", tag);
            break;
        }
        if( stream->flush( ) == -1 ){
            fprintf(stderr, "stream::flush() failure\n");
            success=false;
        }
    } while ( tag != PROT_EXIT );

    // FE delete network will shut us down, so just go to sleep!!
    sleep(10);
    return 0;
}
