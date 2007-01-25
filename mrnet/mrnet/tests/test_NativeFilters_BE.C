/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#include "mrnet/MRNet.h"
#include "Types.h"
#include "DataElement.h"
#include "test_NativeFilters.h"
#include "test_common.h"

using namespace MRN;
using namespace MRN_test;

int main(int argc, char **argv)
{
    Stream * stream;
    Packet * buf=NULL;
    int tag;
    bool success=true;

    const char* parHostname = argv[argc-3];
    Port parPort = (Port)strtoul( argv[argc-2], NULL, 10 );
    Rank myRank = (Rank)strtoul( argv[argc-1], NULL, 10 );
    Network * network = new Network( parHostname, parPort, myRank );
    if( network->fail() ){
        fprintf(stderr, "backend_init() failed\n");
        return -1;
    }

    do{
        if ( network->recv(&tag, &buf, &stream) != 1){
            fprintf(stderr, "stream::recv() failure\n");
        }

        DataType type;
        Stream::unpack( buf, "%d", &type );

        switch(tag){
        case PROT_SUM:
            switch(type) {
            case CHAR_T:
                fprintf( stdout, "Processing CHAR_SUM ...\n");
                if( stream->send(tag, "%c", CHARVAL) == -1 ){
                    fprintf(stderr, "stream::send(%%c) failure\n");
                    success=false;
                }
                break;
            case UCHAR_T:
                fprintf( stdout, "Processing UCHAR_SUM ...\n");
                if( stream->send(tag, "%uc", UCHARVAL) == -1 ){
                    fprintf(stderr, "stream::send(%%uc) failure\n");
                    success=false;
                }
                break;
            case INT16_T:
                fprintf( stdout, "Processing INT16_SUM ...\n");
                if( stream->send(tag, "%hd", INT16VAL) == -1 ){
                    fprintf(stderr, "stream::send(%%hd) failure\n");
                    success=false;
                }
                break;
            case UINT16_T:
                fprintf( stdout, "Processing UINT16_SUM ...\n");
                if( stream->send(tag, "%uhd", UINT16VAL) == -1 ){
                    fprintf(stderr, "stream::send(%%uhd) failure\n");
                    success=false;
                }
                break;
            case INT32_T:
                fprintf( stdout, "Processing INT32_SUM ...\n");
                if( stream->send(tag, "%d", INT32VAL) == -1 ){
                    fprintf(stderr, "stream::send(%%d) failure\n");
                    success=false;
                }
                break;
            case UINT32_T:
                fprintf( stdout, "Processing UINT32_SUM ...\n");
                if( stream->send(tag, "%ud", UINT32VAL) == -1 ){
                    fprintf(stderr, "stream::send(%%ud) failure\n");
                    success=false;
                }
                break;
            case INT64_T:
                fprintf( stdout, "Processing INT64_SUM ...\n");
                if( stream->send(tag, "%ld", INT64VAL) == -1 ){
                    fprintf(stderr, "stream::send(%%ld) failure\n");
                    success=false;
                }
                break;
            case UINT64_T:
                fprintf( stdout, "Processing UINT64_SUM ...\n");
                if( stream->send(tag, "%uld", UINT64VAL) == -1 ){
                    fprintf(stderr, "stream::send(%%uld) failure\n");
                    success=false;
                }
                break;
            case FLOAT_T:
                fprintf( stdout, "Processing FLOAT_SUM ...\n");
                if( stream->send(tag, "%f", FLOATVAL) == -1 ){
                    fprintf(stderr, "stream::send(%%f) failure\n");
                    success=false;
                }
                break;
            case DOUBLE_T:
                fprintf( stdout, "Processing DOUBLE_SUM ...\n");
                if( stream->send(tag, "%lf", DOUBLEVAL) == -1 ){
                    fprintf(stderr, "stream::send(%%lf) failure\n");
                    success=false;
                }
                break;
            default:
                break;
            }
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
