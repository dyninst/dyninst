#include "mrnet/h/MRNet.h"
#include "mrnet/src/Types.h"
#include "mrnet/tests/test_arrays.h"

using namespace MRN;

int main(int argc, char **argv){
    Stream * stream;
    char * buf=NULL;
    int tag=0, recv_array_len=0;
    void * recv_array=NULL;
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

        recv_array=NULL;
        switch(tag){
        case PROT_CHAR:
            fprintf( stdout, "Processing PROT_CHAR_ARRAY ...\n");
            if( stream->unpack(buf, "%ac", &recv_array, &recv_array_len) == -1 ){
                fprintf(stderr, "stream::unpack(%%ac) failure\n");
                success=false;
            }
            if( stream->send(tag, "%ac", recv_array, recv_array_len) == -1 ){
                fprintf(stderr, "stream::send(%%ac) failure\n");
                success=false;
            }
            break;
        case PROT_UCHAR:
            fprintf( stdout, "Processing PROT_UCHAR_ARRAY ...\n");
            if( stream->unpack(buf, "%auc", &recv_array, &recv_array_len) == -1 ){
                fprintf(stderr, "stream::unpack(%%auc) failure\n");
                success=false;
            }
            if( stream->send(tag, "%auc", recv_array, recv_array_len) == -1 ){
                fprintf(stderr, "stream::send(%%auc) failure\n");
                success=false;
            }
            break;
        case PROT_INT:
            fprintf( stdout, "Processing PROT_INT_ARRAY ...\n");
            if( stream->unpack(buf, "%ad", &recv_array, &recv_array_len) == -1 ){
                fprintf(stderr, "stream::unpack(%%ad) failure\n");
                success=false;
            }
            if( stream->send(tag, "%ad", recv_array, recv_array_len) == -1 ){
                fprintf(stderr, "stream::send(%%ad) failure\n");
                success=false;
            }
            break;
        case PROT_UINT:
            fprintf( stdout, "Processing PROT_UINT_ARRAY ...\n");
            if( stream->unpack(buf, "%aud", &recv_array, &recv_array_len) == -1 ){
                fprintf(stderr, "stream::unpack(%%aud) failure\n");
                success=false;
            }
            if( stream->send(tag, "%aud", recv_array, recv_array_len) == -1 ){
                fprintf(stderr, "stream::send(%%aud) failure\n");
                success=false;
            }
            break;
        case PROT_SHORT:
            fprintf( stdout, "Processing PROT_SHORT_ARRAY ...\n");
            if( stream->unpack(buf, "%ahd", &recv_array, &recv_array_len) == -1 ){
                fprintf(stderr, "stream::unpack(%%ahd) failure\n");
                success=false;
            }
            if( stream->send(tag, "%ahd", recv_array, recv_array_len) == -1 ){
                fprintf(stderr, "stream::send(%%ahd) failure\n");
                success=false;
            }
            break;
        case PROT_USHORT:
            fprintf( stdout, "Processing PROT_USHORT_ARRAY ...\n");
            if( stream->unpack(buf, "%auhd", &recv_array, &recv_array_len) == -1 ){
                fprintf(stderr, "stream::unpack(%%auhd) failure\n");
                success=false;
            }
            if( stream->send(tag, "%auhd", recv_array, recv_array_len) == -1 ){
                fprintf(stderr, "stream::send(%%auhd) failure\n");
                success=false;
            }
            break;
        case PROT_LONG:
            fprintf( stdout, "Processing PROT_LONG_ARRAY ...\n");
            if( stream->unpack(buf, "%ald", &recv_array, &recv_array_len) == -1 ){
                fprintf(stderr, "stream::unpack(%%ald) failure\n");
                success=false;
            }
            fprintf(stderr, "recv_array= %p, len=%d\n", recv_array, recv_array_len);
            if( stream->send(tag, "%ald", recv_array, recv_array_len) == -1 ){
                fprintf(stderr, "stream::send(%%ald) failure\n");
                success=false;
            }
            break;
        case PROT_ULONG:
            fprintf( stdout, "Processing PROT_ULONG_ARRAY ...\n");
            if( stream->unpack(buf, "%auld", &recv_array, &recv_array_len) == -1 ){
                fprintf(stderr, "stream::unpack(%%auld) failure\n");
                success=false;
            }
            if( stream->send(tag, "%auld", recv_array, recv_array_len) == -1 ){
                fprintf(stderr, "stream::send(%%auld) failure\n");
                success=false;
            }
            break;
        case PROT_FLOAT:
            fprintf( stdout, "Processing PROT_FLOAT_ARRAY ...\n");
            if( stream->unpack(buf, "%af", &recv_array, &recv_array_len) == -1 ){
                fprintf(stderr, "stream::unpack(%%af) failure\n");
                success=false;
            }
            if( stream->send(tag, "%af", recv_array, recv_array_len) == -1 ){
                fprintf(stderr, "stream::send(%%af) failure\n");
                success=false;
            }
            break;
        case PROT_DOUBLE:
            fprintf( stdout, "Processing PROT_DOUBLE_ARRAY ...\n");
            if( stream->unpack(buf, "%alf", &recv_array, &recv_array_len) == -1 ){
                fprintf(stderr, "stream::unpack(%%alf) failure\n");
                success=false;
            }
            if( stream->send(tag, "%alf", recv_array, recv_array_len) == -1 ){
                fprintf(stderr, "stream::send(%%alf) failure\n");
                success=false;
            }
            break;
        case PROT_EXIT:
            fprintf( stdout, "Processing PROT_EXIT ...\n");
            break;
        default:
            fprintf(stdout, "Unknown Protocol: %d\n", tag);
            exit(-1);
        }
        if(stream->flush() == -1){
            fprintf(stdout, "stream::flush() failure\n");
            return -1;
        }

    } while( tag != PROT_EXIT );
    
    exit(0);
}
