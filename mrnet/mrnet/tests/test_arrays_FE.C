#include "mrnet/h/MR_Network.h"
#include "test_common.h"
#include "test_arrays.h"

#include <string>

const int ARRAY_LEN=1000;
using namespace MRN;
using namespace MRN_test;
Test * test;

int test_char_array(Stream *stream, bool anonymous=false, bool block=true);
int test_uchar_array(Stream *stream, bool anonymous=false, bool block=true);

int test_short_array(Stream *stream, bool anonymous=false, bool block=true);
int test_ushort_array(Stream *stream, bool anonymous=false, bool block=true);

int test_int_array(Stream *stream, bool anonymous=false, bool block=true);
int test_uint_array(Stream *stream, bool anonymous=false, bool block=true);

int test_long_array(Stream *stream, bool anonymous=false, bool block=true);
int test_ulong_array(Stream *stream, bool anonymous=false, bool block=true);

int test_float_array(Stream *stream, bool anonymous=false, bool block=true);

int test_double_array(Stream *stream, bool anonymous=false, bool block=true);

int test_string_array(Stream *stream, bool anonymous=false, bool block=true);

int test_alltypes_array(Stream *stream, bool anonymous=false, bool block=true);


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

    stream_BC = Stream::new_Stream(comm_BC, AGGR_NULL, SYNC_DONTWAIT);

    /* For all the following tests, the 1st bool param indicates *
     * whether the recv() call should be stream-anonymous or not *
     * and the 2nd bool param indicates whether the recv should block *
     * or not */

    if( test_char_array(stream_BC, false, true) == -1 ){
    }
    if( test_char_array(stream_BC, false, false) == -1 ){
    }
    if( test_char_array(stream_BC, true, false) == -1 ){
    }
    if( test_char_array(stream_BC, true, true) == -1 ){
    }
            
    if( test_uchar_array(stream_BC, false, true) == -1 ){
    }
    if( test_uchar_array(stream_BC, false, false) == -1 ){
    }
    if( test_uchar_array(stream_BC, true, false) == -1 ){
    }
    if( test_uchar_array(stream_BC, true, true) == -1 ){
    }
            
    if( test_short_array(stream_BC, false, true) == -1 ){
    }
    if( test_short_array(stream_BC, false, false) == -1 ){
    }
    if( test_short_array(stream_BC, true, false) == -1 ){
    }
    if( test_short_array(stream_BC, true, true) == -1 ){
    }
            
    if( test_ushort_array(stream_BC, false, true) == -1 ){
    }
    if( test_ushort_array(stream_BC, false, false) == -1 ){
    }
    if( test_ushort_array(stream_BC, true, false) == -1 ){
    }
    if( test_ushort_array(stream_BC, true, true) == -1 ){
    }
            
    if( test_int_array(stream_BC, false, true) == -1 ){
    }
    if( test_int_array(stream_BC, false, false) == -1 ){
    }
    if( test_int_array(stream_BC, true, false) == -1 ){
    }
    if( test_int_array(stream_BC, true, true) == -1 ){
    }
            
    if( test_uint_array(stream_BC, false, true) == -1 ){
    }
    if( test_uint_array(stream_BC, false, false) == -1 ){
    }
    if( test_uint_array(stream_BC, true, false) == -1 ){
    }
    if( test_uint_array(stream_BC, true, true) == -1 ){
    }
               
    if( test_long_array(stream_BC, false, true) == -1 ){
    }
    if( test_long_array(stream_BC, false, false) == -1 ){
    }
    if( test_long_array(stream_BC, true, false) == -1 ){
    }
    if( test_long_array(stream_BC, true, true) == -1 ){
    }
            
    if( test_ulong_array(stream_BC, false, true) == -1 ){
    }
    if( test_ulong_array(stream_BC, false, false) == -1 ){
    }
    if( test_ulong_array(stream_BC, true, false) == -1 ){
    }
    if( test_ulong_array(stream_BC, true, true) == -1 ){
    }

    if( test_float_array(stream_BC, false, true) == -1 ){
    }
    if( test_float_array(stream_BC, false, false) == -1 ){
    }
    if( test_float_array(stream_BC, true, false) == -1 ){
    }
    if( test_float_array(stream_BC, true, true) == -1 ){
    }
            
    if( test_double_array(stream_BC, false, true) == -1 ){
    }
    if( test_double_array(stream_BC, false, false) == -1 ){
    }
    if( test_double_array(stream_BC, true, false) == -1 ){
    }
    if( test_double_array(stream_BC, true, true) == -1 ){
    }

    if( test_string_array(stream_BC, false, true) == -1 ){
    }
    if( test_string_array(stream_BC, false, false) == -1 ){
    }
    if( test_string_array(stream_BC, true, false) == -1 ){
    }
    if( test_string_array(stream_BC, true, true) == -1 ){
    }
            
    if( test_alltypes_array(stream_BC, false, true) == -1 ){
    }
    if( test_alltypes_array(stream_BC, false, false) == -1 ){
    }
    if( test_alltypes_array(stream_BC, true, false) == -1 ){
    }
    if( test_alltypes_array(stream_BC, true, true) == -1 ){
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

/* 
 *  test_char_array(): bcast a char to all endpoints in stream.
 *  recv a char from every endpoint
 */
int test_char_array(Stream *stream, bool anonymous, bool block)
{
    Stream *recv_stream;
    char send_array[ARRAY_LEN], *recv_array=NULL;
    int num_received=0, num_to_receive=0;
    int tag;
    char *buf;
    bool success = true;
    std::string testname("test_char_array(");

    if(!anonymous){
        testname += "stream_specific, ";
    }
    else{
        testname += "stream_anonymous, ";
    }

    if(block){
        testname += "blocking_recv)";
    }
    else{
        testname += "non-blocking_recv)";
    }

    test->start_SubTest(testname);
    for( int i=0; i<ARRAY_LEN; i++ ){
        send_array[i] = 'A' + (i%26);
    }

    num_to_receive = stream->get_NumEndPoints();
    if( num_to_receive == 0 ){
        test->print("No endpoints in stream\n", testname);
        test->end_SubTest(testname, NOTRUN);
        return -1;
    }

    if(stream->send(PROT_CHAR, "%ac", send_val, ARRAY_LEN) == -1){
        test->print("stream::send() failure\n", testname);
        test->end_SubTest(testname, FAILURE);
        return -1;
    }

    if(stream->flush() == -1){
        test->print("stream::flush() failure\n", testname);
        test->end_SubTest(testname, FAILURE);
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

            if( Stream::unpack( buf, "%ac", &recv_array, &recv_array_len ) == -1 ){
                test->print("stream::unpack() failure\n", testname);
                success = false;
            }
            if( memcmp(send_array, recv_array, sizeof(char)*ARRAY_LEN) ){
                sprintf(tmp_buf, "send_array != recv_array failure.\n");
                test->print(tmp_buf, testname);
                success = false;
            }
        }
    } while(num_received < num_to_receive);

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
