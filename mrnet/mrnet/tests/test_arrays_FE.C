/***********************************************************************
 * Copyright © 2003 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#include "mrnet/h/MRNet.h"
#include "mrnet/src/Types.h"
#include "mrnet/src/DataElement.h"
#include "test_common.h"
#include "test_arrays.h"

#include <string>

const unsigned int ARRAY_LEN=1000;
using namespace MRN;
using namespace MRN_test;
Test * test;

int test_array(Stream *stream, bool anonymous, bool block, DataType type);

int main(int argc, char **argv)
{
    Stream * stream_BC;
    char dummy;

    if( argc !=3 ){
        fprintf(stderr, 
                "Usage: %s <topology file> <backend_exe>\n", argv[0]);
        exit(-1);
    }

    fprintf(stdout, "\n"
            " ##########################################\n"
            " # MRNet C++ Interface *Basic* Test Suite #\n"
            " ##########################################\n\n"
            "   This test suite performs many tests that exercise\n"
            " MRNet's basic functionality. These tests use all MRNet\n"
            " data types, and the popular functions in the interface.\n\n"
            " Press <enter> to start the testing ...\n");

    scanf("%c",&dummy);

    test = new Test( "MRNet Basic Test", stdout );

    if( Network::new_Network(argv[1], argv[2]) == -1){
        fprintf(stderr, "Network Initialization failure\n");
        Network::error_str(argv[0]);
        exit(-1);
    }

    Communicator * comm_BC = Communicator::get_BroadcastCommunicator();

    stream_BC = Stream::new_Stream(comm_BC, TFILTER_NULL, SFILTER_DONTWAIT);

    /* For all the following tests, the 1st bool param indicates *
     * whether the recv() call should be stream-anonymous or not *
     * and the 2nd bool param indicates whether the recv should block *
     * or not */

    if( test_array(stream_BC, false, true, CHAR_ARRAY_T) == -1 ){
    }
    if( test_array(stream_BC, false, false, CHAR_ARRAY_T) == -1 ){
    }
    if( test_array(stream_BC, true, false, CHAR_ARRAY_T) == -1 ){
    }
    if( test_array(stream_BC, true, true, CHAR_ARRAY_T) == -1 ){
    }

    if( test_array(stream_BC, false, true, UCHAR_ARRAY_T) == -1 ){
    }
    if( test_array(stream_BC, false, false, UCHAR_ARRAY_T) == -1 ){
    }
    if( test_array(stream_BC, true, false, UCHAR_ARRAY_T) == -1 ){
    }
    if( test_array(stream_BC, true, true, UCHAR_ARRAY_T) == -1 ){
    }

    if( test_array(stream_BC, false, true, INT16_ARRAY_T) == -1 ){
    }
    if( test_array(stream_BC, false, false, INT16_ARRAY_T) == -1 ){
    }
    if( test_array(stream_BC, true, false, INT16_ARRAY_T) == -1 ){
    }
    if( test_array(stream_BC, true, true, INT16_ARRAY_T) == -1 ){
    }

    if( test_array(stream_BC, false, true, UINT16_ARRAY_T) == -1 ){
    }
    if( test_array(stream_BC, false, false, UINT16_ARRAY_T) == -1 ){
    }
    if( test_array(stream_BC, true, false, UINT16_ARRAY_T) == -1 ){
    }
    if( test_array(stream_BC, true, true, UINT16_ARRAY_T) == -1 ){
    }

    if( test_array(stream_BC, false, true, INT32_ARRAY_T) == -1 ){
    }
    if( test_array(stream_BC, false, false, INT32_ARRAY_T) == -1 ){
    }
    if( test_array(stream_BC, true, false, INT32_ARRAY_T) == -1 ){
    }
    if( test_array(stream_BC, true, true, INT32_ARRAY_T) == -1 ){
    }

    if( test_array(stream_BC, false, true, UINT32_ARRAY_T) == -1 ){
    }
    if( test_array(stream_BC, false, false, UINT32_ARRAY_T) == -1 ){
    }
    if( test_array(stream_BC, true, false, UINT32_ARRAY_T) == -1 ){
    }
    if( test_array(stream_BC, true, true, UINT32_ARRAY_T) == -1 ){
    }

    if( test_array(stream_BC, false, true, INT64_ARRAY_T) == -1 ){
    }
    if( test_array(stream_BC, false, false, INT64_ARRAY_T) == -1 ){
    }
    if( test_array(stream_BC, true, false, INT64_ARRAY_T) == -1 ){
    }
    if( test_array(stream_BC, true, true, INT64_ARRAY_T) == -1 ){
    }

    if( test_array(stream_BC, false, true, UINT64_ARRAY_T) == -1 ){
    }
    if( test_array(stream_BC, false, false, UINT64_ARRAY_T) == -1 ){
    }
    if( test_array(stream_BC, true, false, UINT64_ARRAY_T) == -1 ){
    }
    if( test_array(stream_BC, true, true, UINT64_ARRAY_T) == -1 ){
    }

    if( test_array(stream_BC, false, true, FLOAT_ARRAY_T) == -1 ){
    }
    if( test_array(stream_BC, false, false, FLOAT_ARRAY_T) == -1 ){
    }
    if( test_array(stream_BC, true, false, FLOAT_ARRAY_T) == -1 ){
    }
    if( test_array(stream_BC, true, true, FLOAT_ARRAY_T) == -1 ){
    }

    if( test_array(stream_BC, false, true, DOUBLE_ARRAY_T) == -1 ){
    }
    if( test_array(stream_BC, false, false, DOUBLE_ARRAY_T) == -1 ){
    }
    if( test_array(stream_BC, true, false, DOUBLE_ARRAY_T) == -1 ){
    }
    if( test_array(stream_BC, true, true, DOUBLE_ARRAY_T) == -1 ){
    }

    if(stream_BC->send(PROT_EXIT, "") == -1){
        test->print("stream::send(exit) failure\n");
        return -1;
    }

    if(stream_BC->flush() == -1){
        test->print("stream::flush() failure\n");
        return -1;
    }

    Network::delete_Network();

    test->end_Test();
    delete test;

    return 0;
}

int test_array(Stream *stream, bool anonymous, bool block, DataType type)
{
    Stream *recv_stream;
    void *send_array=NULL, *recv_array=NULL;
    unsigned int i, recv_array_len=0;
    int num_received=0, num_to_receive=0, data_size=0;
    int tag=0;
    char *buf;
    bool success = true;
    std::string testname, format_string;

    switch(type){
    case CHAR_ARRAY_T:
        data_size = sizeof(char);
        send_array = malloc ( ARRAY_LEN * data_size );
        for( i=0; i<ARRAY_LEN; i++){
            ((char*)send_array)[i] = 'a';
        }
        tag = PROT_CHAR;
        testname = "test_char_array";
        format_string = "%ac";
        break;
    case UCHAR_ARRAY_T:
        data_size = sizeof(unsigned char);
        send_array = malloc ( ARRAY_LEN * data_size );
        for( i=0; i<ARRAY_LEN; i++){
            ((unsigned char*)send_array)[i] = 'a';
        }
        tag = PROT_UCHAR;
        testname = "test_uchar_array";
        format_string = "%auc";
        break;
    case INT16_ARRAY_T:
        data_size = sizeof(int16_t);
        send_array = malloc ( ARRAY_LEN * data_size );
        for( i=0; i<ARRAY_LEN; i++){
            ((int16_t*)send_array)[i] = -17;
        }
        tag = PROT_SHORT;
        testname = "test_int16_array";
        format_string = "%ahd";
        break;
    case UINT16_ARRAY_T:
        data_size = sizeof(uint16_t);
        send_array = malloc ( ARRAY_LEN * data_size );
        for( i=0; i<ARRAY_LEN; i++){
            ((uint16_t*)send_array)[i] = 17;
        }
        tag = PROT_USHORT;
        testname = "test_uint16_array";
        format_string = "%auhd";
        break;
    case INT32_ARRAY_T:
        data_size = sizeof(int32_t);
        send_array = malloc ( ARRAY_LEN * data_size );
        for( i=0; i<ARRAY_LEN; i++){
            ((int32_t*)send_array)[i] = -17;
        }
        tag = PROT_INT;
        testname = "test_int32_array";
        format_string = "%ad";
        break;
    case UINT32_ARRAY_T:
        data_size = sizeof(uint32_t);
        send_array = malloc ( ARRAY_LEN * data_size );
        for( i=0; i<ARRAY_LEN; i++){
            ((uint32_t*)send_array)[i] = 17;
        }
        tag = PROT_UINT;
        testname = "test_uint32_array";
        format_string = "%aud";
        break;
    case INT64_ARRAY_T:
        data_size = sizeof(int64_t);
        send_array = malloc ( ARRAY_LEN * data_size );
        for( i=0; i<ARRAY_LEN; i++){
            ((int64_t*)send_array)[i] = -17;
        }
        tag = PROT_LONG;
        testname = "test_int64_array";
        format_string = "%ald";
        break;
    case UINT64_ARRAY_T:
        data_size = sizeof(uint64_t);
        send_array = malloc ( ARRAY_LEN * data_size );
        for( i=0; i<ARRAY_LEN; i++){
            ((uint64_t*)send_array)[i] = 17;
        }
        tag = PROT_ULONG;
        testname = "test_uint64_array";
        format_string = "%auld";
        break;
    case FLOAT_ARRAY_T:
        data_size = sizeof(float);
        send_array = malloc ( ARRAY_LEN * data_size );
        for( i=0; i<ARRAY_LEN; i++){
            ((float*)send_array)[i] = 123.456789;
        }
        tag = PROT_FLOAT;
        testname = "test_float_array";
        format_string = "%af";
        break;
    case DOUBLE_ARRAY_T:
        data_size = sizeof(double);
        send_array = malloc ( ARRAY_LEN * data_size );
        for( i=0; i<ARRAY_LEN; i++){
            ((double*)send_array)[i] = -123.456789;
        }
        tag = PROT_DOUBLE;
        testname = "test_double_array";
        format_string = "%alf";
        break;
    case CHAR_T:
    case UCHAR_T:
    case INT16_T:
    case UINT16_T:
    case INT32_T:
    case UINT32_T:
    case INT64_T:
    case UINT64_T:
    case FLOAT_T:
    case DOUBLE_T:
    case STRING_T:
    case UNKNOWN_T:
    default:
        return -1;
    }

    if(!anonymous){
        testname += "(stream_specific, ";
    }
    else{
        testname += "(stream_anonymous, ";
    }

    if(block){
        testname += "blocking_recv)";
    }
    else{
        testname += "non-blocking_recv)";
    }

    test->start_SubTest(testname);

    num_to_receive = stream->get_NumEndPoints();
    if( num_to_receive == 0 ){
        test->print("No endpoints in stream\n", testname);
        test->end_SubTest(testname, NOTRUN);
        free(send_array);
        return -1;
    }

    if(stream->send(tag, format_string.c_str(), send_array, ARRAY_LEN) == -1){
        test->print("stream::send() failure\n", testname);
        test->end_SubTest(testname, FAILURE);
        free(send_array);
        return -1;
    }

    if(stream->flush() == -1){
        test->print("stream::flush() failure\n", testname);
        test->end_SubTest(testname, FAILURE);
        free(send_array);
        return -1;
    }

    do{
        int retval;

        if(!anonymous){
            retval = stream->recv(&tag, (void**)&buf, block);
        }
        else{
            retval = Stream::recv(&tag, (void**)&buf, &recv_stream, block);
        }

        if( retval == -1){
            //recv error
            test->print("stream::recv() failure\n", testname);
            test->end_SubTest(testname, FAILURE);
            free(send_array);
            return -1;
        }
        else if ( retval == 0 ){
            //No data available
        }
        else{
            //Got data
            char tmp_buf[256];

            num_received++;
            sprintf(tmp_buf, "Received %d packets; %d left.\n",
                    num_received, num_to_receive-num_received);
            test->print(tmp_buf, testname);

            if( Stream::unpack( buf, format_string.c_str(), &recv_array,
                                &recv_array_len ) == -1 ){
                test->print("stream::unpack() failure\n", testname);
                success = false;
            }
            if( memcmp(send_array, recv_array, data_size * ARRAY_LEN) ){
                sprintf(tmp_buf, "send_array != recv_array failure.\n");
                test->print(tmp_buf, testname);
                success = false;
            }
        }
    } while(num_received < num_to_receive);

    free(send_array);
    free(recv_array);

    if( success ){
        test->end_SubTest(testname, SUCCESS);
        return 0;
    }
    else{
        test->end_SubTest(testname, FAILURE);
        return -1;
    }

    return 0;
}
