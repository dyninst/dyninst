/***********************************************************************
 * Copyright © 2003 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#include <stdlib.h>
#include <time.h>

#include "mrnet/h/MRNet.h"
#include "mrnet/tests/test_DynamicFilters.h"

using namespace MRN;

int main(int argc, char **argv)
{
    Stream * stream;
    char * buf=NULL;
    int tag;
    bool success=true;

    srandom( time(NULL) ); //arbitrary seed to random()

    Network * network = new Network(argv[argc-5], argv[argc-4], argv[argc-3],
                                    argv[argc-2], argv[argc-1]);
    if( network->fail() ){
        fprintf(stderr, "backend_init() failed\n");
        return -1;
    }

    do{
        if ( network->recv(&tag, (void **)&buf, &stream) != 1){
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

    exit(0);
}
