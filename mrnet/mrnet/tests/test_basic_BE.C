/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#include <assert.h>

#include "mrnet/MRNet.h"
#include "mrnet/src/Types.h"
#include "mrnet/tests/test_basic.h"

using namespace MRN;

int main(int argc, char **argv){
    Stream * stream;
    Packet * buf=NULL;
    int tag;
    char recv_char;
    unsigned char recv_uchar;
    int16_t recv_short;
    uint16_t recv_ushort;
    int32_t recv_int;
    uint32_t recv_uint;
    int64_t recv_long;
    uint64_t recv_ulong;
    float recv_float;
    double recv_double;
    char * recv_string;
    bool success=true;

    //set_OutputLevel(5);
    if( argc != 4 ){
        fprintf(stderr, "usage: %s parent_hostname parent_port my_rank\n",
                argv[0]);
        exit( -1 );
    }

    const char* parHostname = argv[argc-3];
    Port parPort = (Port)strtoul( argv[argc-2], NULL, 10 );
    Rank myRank = (Rank)strtoul( argv[argc-1], NULL, 10 );
    Network * network = new Network( parHostname, parPort, myRank );
    if( network->fail() ){
        fprintf(stderr, "backend_init() failed\n");
        exit (-1);
    }

    do{
        if ( network->recv(&tag, &buf, &stream) != 1){
            fprintf(stderr, "stream::recv() failure ... backend exiting\n");
            exit (-1);
        }

        switch(tag){
        case PROT_CHAR:
#if defined(DEBUG)
            fprintf( stderr, "Processing PROT_CHAR ...\n");
#endif
            if( stream->unpack(buf, "%c", &recv_char) == -1 ){
                fprintf(stderr, "stream::unpack(%%c) failure\n");
                success=false;
            }
            if( stream->send(tag, "%c", recv_char) == -1 ){
                fprintf(stderr, "stream::send(%%c) failure\n");
                success=false;
            }
            break;
        case PROT_UCHAR:
#if defined(DEBUG)
            fprintf( stderr, "Processing PROT_UCHAR ...\n");
#endif
            if( stream->unpack(buf, "%uc", &recv_uchar) == -1 ){
                fprintf(stderr, "stream::unpack(%%uc) failure\n");
                success=false;
            }
            if( stream->send(tag, "%uc", recv_uchar) == -1 ){
                fprintf(stderr, "stream::send(%%uc) failure\n");
                success=false;
            }
            break;
        case PROT_INT:
#if defined(DEBUG)
            fprintf( stderr, "Processing PROT_INT ...\n");
#endif
            if( stream->unpack(buf, "%d", &recv_int) == -1 ){
                fprintf(stderr, "stream::unpack(%%d) failure\n");
                success=false;
            }
            if( stream->send(tag, "%d", recv_int) == -1 ){
                fprintf(stderr, "stream::send(%%d) failure\n");
                success=false;
            }
            break;
        case PROT_UINT:
#if defined(DEBUG)
            fprintf( stderr, "Processing PROT_UINT ...\n");
#endif
            if( stream->unpack(buf, "%ud", &recv_uint) == -1 ){
                fprintf(stderr, "stream::unpack(%%ud) failure\n");
                success=false;
            }
            if( stream->send(tag, "%ud", recv_uint) == -1 ){
                fprintf(stderr, "stream::send(%%ud) failure\n");
                success=false;
            }
            break;
        case PROT_SHORT:
#if defined(DEBUG)
            fprintf( stderr, "Processing PROT_SHORT ...\n");
#endif
            if( stream->unpack(buf, "%hd", &recv_short) == -1 ){
                fprintf(stderr, "stream::unpack(%%hd) failure\n");
                success=false;
            }
            if( stream->send(tag, "%hd", recv_short) == -1 ){
                fprintf(stderr, "stream::send(%%hd) failure\n");
                success=false;
            }
            break;
        case PROT_USHORT:
#if defined(DEBUG)
            fprintf( stderr, "Processing PROT_USHORT ...\n");
#endif
            if( stream->unpack(buf, "%uhd", &recv_ushort) == -1 ){
                fprintf(stderr, "stream::unpack(%%uhd) failure\n");
                success=false;
            }
            if( stream->send(tag, "%uhd", recv_ushort) == -1 ){
                fprintf(stderr, "stream::send(%%uhd) failure\n");
                success=false;
            }
            break;
        case PROT_LONG:
#if defined(DEBUG)
            fprintf( stderr, "Processing PROT_LONG ...\n");
#endif
            if( stream->unpack(buf, "%ld", &recv_long) == -1 ){
                fprintf(stderr, "stream::unpack(%%ld) failure\n");
                success=false;
            }
            if( stream->send(tag, "%ld", recv_long) == -1 ){
                fprintf(stderr, "stream::send(%%ld) failure\n");
                success=false;
            }
            break;
        case PROT_ULONG:
#if defined(DEBUG)
            fprintf( stderr, "Processing PROT_ULONG ...\n");
#endif
            if( stream->unpack(buf, "%uld", &recv_ulong) == -1 ){
                fprintf(stderr, "stream::unpack(%%uld) failure\n");
                success=false;
            }
            if( stream->send(tag, "%uld", recv_ulong) == -1 ){
                fprintf(stderr, "stream::send(%%uld) failure\n");
                success=false;
            }
            break;
        case PROT_FLOAT:
#if defined(DEBUG)
            fprintf( stderr, "Processing PROT_FLOAT ...\n");
#endif
            if( stream->unpack(buf, "%f", &recv_float) == -1 ){
                fprintf(stderr, "stream::unpack(%%f) failure\n");
                success=false;
            }
            if( stream->send(tag, "%f", recv_float) == -1 ){
                fprintf(stderr, "stream::send(%%f) failure\n");
                success=false;
            }
            break;
        case PROT_DOUBLE:
#if defined(DEBUG)
            fprintf( stderr, "Processing PROT_DOUBLE ...\n");
#endif
            if( stream->unpack(buf, "%lf", &recv_double) == -1 ){
                fprintf(stderr, "stream::unpack(%%lf) failure\n");
                success=false;
            }
            if( stream->send(tag, "%lf", recv_double) == -1 ){
                fprintf(stderr, "stream::send(%%lf) failure\n");
                success=false;
            }
            break;
        case PROT_STRING:
#if defined(DEBUG)
            fprintf( stderr, "Processing PROT_STRING ...\n");
#endif
            if( stream->unpack(buf, "%s", &recv_string) == -1 ){
                fprintf(stderr, "stream::unpack(%%s) failure\n");
                success=false;
            }
            if( stream->send(tag, "%s", recv_string) == -1 ){
                fprintf(stderr, "stream::send(%%s) failure\n");
                success=false;
            }
            free(recv_string);
            break;
        case PROT_ALL:
#if defined(DEBUG)
            fprintf( stderr, "Processing PROT_ALL ...\n");
#endif
            if(stream->unpack(buf,
                              "%c %uc %hd %uhd %d %ud %ld %uld %f %lf %s",
                              &recv_char, &recv_uchar, &recv_short, &recv_ushort,
                              &recv_int, &recv_uint, &recv_long, &recv_ulong,
                              &recv_float, &recv_double, &recv_string) == 1){
                fprintf(stderr, "stream::unpack(all) failure\n");
                success = false;
            }
            if(stream->send(tag, "%c %uc %hd %uhd %d %ud %ld %uld %f %lf %s",
                            recv_char, recv_uchar, recv_short, recv_ushort,
                            recv_int, recv_uint, recv_long, recv_ulong,
                            recv_float, recv_double, recv_string) == 1){
                fprintf(stderr, "stream::send(all) failure\n");
                success=false;
            }
            break;
        case PROT_EXIT:
#if defined(DEBUG)
            fprintf( stderr, "Processing PROT_EXIT ...\n");
#endif
            break;
        default:
            fprintf(stderr, "Unknown Protocol: %d\n", tag);
            exit(-1);
        }
        if( stream->flush() == -1){
            fprintf(stderr, "stream::flush() failure\n");
            return -1;
        }

    } while( tag != PROT_EXIT );
    
    exit(0);
}
