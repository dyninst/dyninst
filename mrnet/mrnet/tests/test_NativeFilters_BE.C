/***********************************************************************
 * Copyright © 2003 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#include "mrnet/h/MRNet.h"
#include "mrnet/src/Types.h"
#include "mrnet/src/DataElement.h"
#include "mrnet/tests/test_NativeFilters.h"

using namespace MRN;

int main(int argc, char **argv)
{
    Stream * stream;
    char * buf=NULL;
    int tag;
    bool success=true;

    if( Network::init_Backend(argv[argc-5], argv[argc-4], argv[argc-3],
                              argv[argc-2], argv[argc-1]) == -1){
        fprintf(stderr, "backend_init() failed\n");
        return -1;
    }

    do{
        if ( Stream::recv(&tag, (void **)&buf, &stream) != 1){
            fprintf(stderr, "stream::recv() failure\n");
        }

        switch(tag){
        case PROT_CHAR_SUM:
            fprintf( stdout, "Processing PROT_CHAR_SUM ...\n");
            if( stream->send(tag, "%c", CHARVAL) == -1 ){
                fprintf(stderr, "stream::send(%%c) failure\n");
                success=false;
            }
            break;
        case PROT_UCHAR_SUM:
            fprintf( stdout, "Processing PROT_UCHAR_SUM ...\n");
            if( stream->send(tag, "%uc", UCHARVAL) == -1 ){
                fprintf(stderr, "stream::send(%%uc) failure\n");
                success=false;
            }
            break;
        case PROT_INT16_SUM:
            fprintf( stdout, "Processing PROT_INT16_SUM ...\n");
            if( stream->send(tag, "%hd", INT16VAL) == -1 ){
                fprintf(stderr, "stream::send(%%hd) failure\n");
                success=false;
            }
            break;
        case PROT_UINT16_SUM:
            fprintf( stdout, "Processing PROT_UINT16_SUM ...\n");
            if( stream->send(tag, "%uhd", UINT16VAL) == -1 ){
                fprintf(stderr, "stream::send(%%uhd) failure\n");
                success=false;
            }
            break;
        case PROT_INT32_SUM:
            fprintf( stdout, "Processing PROT_INT32_SUM ...\n");
            if( stream->send(tag, "%d", INT32VAL) == -1 ){
                fprintf(stderr, "stream::send(%%d) failure\n");
                success=false;
            }
            break;
        case PROT_UINT32_SUM:
            fprintf( stdout, "Processing PROT_UINT32_SUM ...\n");
            if( stream->send(tag, "%ud", UINT32VAL) == -1 ){
                fprintf(stderr, "stream::send(%%ud) failure\n");
                success=false;
            }
            break;
        case PROT_INT64_SUM:
            fprintf( stdout, "Processing PROT_INT64_SUM ...\n");
            if( stream->send(tag, "%ld", INT64VAL) == -1 ){
                fprintf(stderr, "stream::send(%%ld) failure\n");
                success=false;
            }
            break;
        case PROT_UINT64_SUM:
            fprintf( stdout, "Processing PROT_UINT64_SUM ...\n");
            if( stream->send(tag, "%uld", UINT64VAL) == -1 ){
                fprintf(stderr, "stream::send(%%uld) failure\n");
                success=false;
            }
            break;
        case PROT_FLOAT_SUM:
            fprintf( stdout, "Processing PROT_FLOAT_SUM ...\n");
            if( stream->send(tag, "%f", FLOATVAL) == -1 ){
                fprintf(stderr, "stream::send(%%f) failure\n");
                success=false;
            }
            break;
        case PROT_DOUBLE_SUM:
            fprintf( stdout, "Processing PROT_DOUBLE_SUM ...\n");
            if( stream->send(tag, "%lf", DOUBLEVAL) == -1 ){
                fprintf(stderr, "stream::send(%%lf) failure\n");
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
