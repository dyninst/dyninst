/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#include "mrnet/MRNet.h"
#include "mrnet/src/Types.h"
#include "test_common.h"
#include "test_basic.h"

#include <string>

using namespace MRN;
using namespace MRN_test;
Test * test;

int test_char( Network *, Stream *, bool anonymous=false, bool block=true);
int test_char_array( Network *, Stream *, bool anonymous=false, bool block=true);
int test_uchar( Network *, Stream *, bool anonymous=false, bool block=true);
int test_uchar_array( Network *, Stream *, bool anonymous=false, bool block=true);

int test_short( Network *, Stream *, bool anonymous=false, bool block=true);
int test_short_array( Network *, Stream *, bool anonymous=false, bool block=true);
int test_ushort( Network *, Stream *, bool anonymous=false, bool block=true);
int test_ushort_array( Network *, Stream *, bool anonymous=false, bool block=true);

int test_int( Network *, Stream *, bool anonymous=false, bool block=true);
int test_int_array( Network *, Stream *, bool anonymous=false, bool block=true);
int test_uint( Network *, Stream *, bool anonymous=false, bool block=true);
int test_uint_array( Network *, Stream *, bool anonymous=false, bool block=true);

int test_long( Network *, Stream *, bool anonymous=false, bool block=true);
int test_long_array( Network *, Stream *, bool anonymous=false, bool block=true);
int test_ulong( Network *, Stream *, bool anonymous=false, bool block=true);
int test_ulong_array( Network *, Stream *, bool anonymous=false, bool block=true);

int test_float( Network *, Stream *, bool anonymous=false, bool block=true);
int test_float_array( Network *, Stream *, bool anonymous=false, bool block=true);

int test_double( Network *, Stream *, bool anonymous=false, bool block=true);
int test_double_array( Network *, Stream *, bool anonymous=false, bool block=true);

int test_string( Network *, Stream *, bool anonymous=false, bool block=true);
int test_string_array( Network *, Stream *, bool anonymous=false, bool block=true);

int test_alltypes( Network *, Stream *, bool anonymous=false, bool block=true);


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
    fflush( stdout );

    scanf("%c",&dummy);

    test = new Test( "MRNet Basic Test", stdout );

    Network * network = new Network( argv[1], argv[2] );
    if( network->fail() ){
        fprintf(stderr, "Network Initialization failure\n");
        network->error_str(argv[0]);
        exit(-1);
    }

    Communicator * comm_BC = network->get_BroadcastCommunicator( );

    stream_BC = network->new_Stream(comm_BC, TFILTER_NULL, SFILTER_DONTWAIT);

    /* For all the following tests, the 1st bool param indicates *
     * whether the recv() call should be stream-anonymous or not *
     * and the 2nd bool param indicates whether the recv should block *
     * or not */

    if( test_char( network, stream_BC, false, true) == -1 ){
    }
    if( test_char( network, stream_BC, false, false) == -1 ){
    }
    if( test_char( network, stream_BC, true, false) == -1 ){
    }
    if( test_char( network, stream_BC, true, true) == -1 ){
    }
            
    if( test_uchar( network, stream_BC, false, true) == -1 ){
    }
    if( test_uchar( network, stream_BC, false, false) == -1 ){
    }
    if( test_uchar( network, stream_BC, true, false) == -1 ){
    }
    if( test_uchar( network, stream_BC, true, true) == -1 ){
    }
            
    if( test_short( network, stream_BC, false, true) == -1 ){
    }
    if( test_short( network, stream_BC, false, false) == -1 ){
    }
    if( test_short( network, stream_BC, true, false) == -1 ){
    }
    if( test_short( network, stream_BC, true, true) == -1 ){
    }
            
    if( test_ushort( network, stream_BC, false, true) == -1 ){
    }
    if( test_ushort( network, stream_BC, false, false) == -1 ){
    }
    if( test_ushort( network, stream_BC, true, false) == -1 ){
    }
    if( test_ushort( network, stream_BC, true, true) == -1 ){
    }
            
    if( test_int( network, stream_BC, false, true) == -1 ){
    }
    if( test_int( network, stream_BC, false, false) == -1 ){
    }
    if( test_int( network, stream_BC, true, false) == -1 ){
    }
    if( test_int( network, stream_BC, true, true) == -1 ){
    }
            
    if( test_uint( network, stream_BC, false, true) == -1 ){
    }
    if( test_uint( network, stream_BC, false, false) == -1 ){
    }
    if( test_uint( network, stream_BC, true, false) == -1 ){
    }
    if( test_uint( network, stream_BC, true, true) == -1 ){
    }
               
    if( test_long( network, stream_BC, false, true) == -1 ){
    }
    if( test_long( network, stream_BC, false, false) == -1 ){
    }
    if( test_long( network, stream_BC, true, false) == -1 ){
    }
    if( test_long( network, stream_BC, true, true) == -1 ){
    }
            
    if( test_ulong( network, stream_BC, false, true) == -1 ){
    }
    if( test_ulong( network, stream_BC, false, false) == -1 ){
    }
    if( test_ulong( network, stream_BC, true, false) == -1 ){
    }
    if( test_ulong( network, stream_BC, true, true) == -1 ){
    }

    if( test_float( network, stream_BC, false, true) == -1 ){
    }
    if( test_float( network, stream_BC, false, false) == -1 ){
    }
    if( test_float( network, stream_BC, true, false) == -1 ){
    }
    if( test_float( network, stream_BC, true, true) == -1 ){
    }
            
    if( test_double( network, stream_BC, false, true) == -1 ){
    }
    if( test_double( network, stream_BC, false, false) == -1 ){
    }
    if( test_double( network, stream_BC, true, false) == -1 ){
    }
    if( test_double( network, stream_BC, true, true) == -1 ){
    }

    if( test_string( network, stream_BC, false, true) == -1 ){
    }
    if( test_string( network, stream_BC, false, false) == -1 ){
    }
    if( test_string( network, stream_BC, true, false) == -1 ){
    }
    if( test_string( network, stream_BC, true, true) == -1 ){
    }
            
    if( test_alltypes( network, stream_BC, false, true) == -1 ){
    }
    if( test_alltypes( network, stream_BC, false, false) == -1 ){
    }
    if( test_alltypes( network, stream_BC, true, false) == -1 ){
    }
    if( test_alltypes( network, stream_BC, true, true) == -1 ){
    }

    if( stream_BC->send(PROT_EXIT, "") == -1){
        test->print("stream::send(exit) failure\n");
        return -1;
    }

    if( stream_BC->flush() == -1){
        test->print("stream::flush() failure\n");
        return -1;
    }

    delete network;

    test->end_Test();
    delete test;

    return 0;
}

/* 
 *  test_char(): bcast a char to all endpoints in stream.
 *  recv a char from every endpoint
 */
int test_char( Network * network, Stream *stream, bool anonymous, bool block)
{
    Stream *recv_stream;
    char send_val=-17, recv_val=0;
    int num_received=0, num_to_receive=0;
    int tag;
    char *buf;
    bool success = true;
    std::string testname("test_char(");

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

    num_to_receive = stream->get_NumEndPoints();
    if( num_to_receive == 0 ){
        test->print("No endpoints in stream\n", testname);
        test->end_SubTest(testname, NOTRUN);
        return -1;
    }


    if(stream->send(PROT_CHAR, "%c", send_val) == -1){
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
            retval = network->recv(&tag, (void**)&buf, &recv_stream, block);
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

            if( Stream::unpack( buf, "%c", &recv_val ) == -1 ){
                test->print("stream::unpack() failure\n", testname);
                success = false;
            }
            if(send_val != recv_val ){
                sprintf(tmp_buf, "send_val(%c) != recv_val(%c) failure.\n",
                        send_val, recv_val);
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

/* 
 *  test_uchar(): bcast an unsigned char to all endpoints in stream.
 *  recv a char from every endpoint
 */
int test_uchar( Network * network, Stream *stream, bool anonymous, bool block)
{
    Stream *recv_stream;
    unsigned char send_val=17, recv_val=0;
    int num_received=0, num_to_receive=0;
    int tag;
    char *buf;
    bool success = true;
    std::string testname("test_uchar(");

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

    num_to_receive = stream->get_NumEndPoints();
    if( num_to_receive == 0 ){
        test->print("No endpoints in stream\n", testname);
        test->end_SubTest(testname, NOTRUN);
        return -1;
    }


    if(stream->send(PROT_UCHAR, "%uc", send_val) == -1){
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
            retval = network->recv(&tag, (void**)&buf, &recv_stream, block);
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

            if( Stream::unpack( buf, "%uc", &recv_val ) == -1 ){
                test->print("stream::unpack() failure\n", testname);
                success = false;
            }
            if(send_val != recv_val ){
                sprintf(tmp_buf, "send_val(%uc) != recv_val(%uc) failure.\n",
                        send_val, recv_val);
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

/* 
 *  test_short(): bcast a 16-bit signed int to all endpoints in stream.
 *  recv a 16-bit signed int from every endpoint
 */
int test_short( Network * network, Stream *stream, bool anonymous, bool block)
{
    Stream *recv_stream;
    int16_t send_val=-17, recv_val=0;
    int num_received=0, num_to_receive=0;
    int tag;
    char *buf;
    bool success = true;
    std::string testname("test_short(");

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

    num_to_receive = stream->get_NumEndPoints();
    if( num_to_receive == 0 ){
        test->print("No endpoints in stream\n", testname);
        test->end_SubTest(testname, NOTRUN);
        return -1;
    }


    if(stream->send(PROT_SHORT, "%hd", send_val) == -1){
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
            retval = network->recv(&tag, (void**)&buf, &recv_stream, block);
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

            if( Stream::unpack( buf, "%hd", &recv_val ) == -1 ){
                test->print("stream::unpack() failure\n", testname);
                success = false;
            }
            if(send_val != recv_val ){
                sprintf(tmp_buf, "send_val(%hd) != recv_val(%hd) failure.\n",
                        send_val, recv_val);
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

/* 
 *  test_ushort():
 *    bcast a 16-bit unsigned int to all endpoints in stream.
 *    recv  a 16-bit unsigned int from every endpoint
 */
int test_ushort( Network * network, Stream *stream, bool anonymous, bool block)
{
    Stream *recv_stream;
    uint16_t send_val=17, recv_val=0;
    int num_received=0, num_to_receive=0;
    int tag;
    char *buf;
    bool success = true;
    std::string testname("test_ushort(");

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

    num_to_receive = stream->get_NumEndPoints();
    if( num_to_receive == 0 ){
        test->print("No endpoints in stream\n", testname);
        test->end_SubTest(testname, NOTRUN);
        return -1;
    }


    if(stream->send(PROT_USHORT, "%uhd", send_val) == -1){
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
            retval = network->recv(&tag, (void**)&buf, &recv_stream, block);
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

            if( Stream::unpack( buf, "%uhd", &recv_val ) == -1 ){
                test->print("stream::unpack() failure\n", testname);
                success = false;
            }
            if(send_val != recv_val ){
                sprintf(tmp_buf, "send_val(%uhd) != recv_val(%uhd) failure.\n",
                        send_val, recv_val);
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

/* 
 *  test_int():
 *    bcast a 32-bit signed int to all endpoints in stream.
 *    recv  a 32-bit signed int from every endpoint
 */
int test_int( Network * network, Stream *stream, bool anonymous, bool block)
{
    Stream *recv_stream;
    int32_t send_val = -17, recv_val=0;
    int num_received=0, num_to_receive=0;
    int tag;
    char *buf;
    bool success = true;
    std::string testname("test_int(");

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

    num_to_receive = stream->get_NumEndPoints();
    if( num_to_receive == 0 ){
        test->print("No endpoints in stream\n", testname);
        test->end_SubTest(testname, NOTRUN);
        return -1;
    }


    if(stream->send(PROT_INT, "%d", send_val) == -1){
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
            retval = network->recv(&tag, (void**)&buf, &recv_stream, block);
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

            if( Stream::unpack( buf, "%d", &recv_val ) == -1 ){
                test->print("stream::unpack() failure\n", testname);
                success = false;
            }
            if(send_val != recv_val ){
                sprintf(tmp_buf, "send_val(%d) != recv_val(%d) failure.\n",
                        send_val, recv_val);
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

/* 
 *  test_uint():
 *    bcast a 32-bit unsigned int to all endpoints in stream.
 *    recv  a 32-bit unsigned int from every endpoint
 */
int test_uint( Network * network, Stream *stream, bool anonymous, bool block)
{
    Stream *recv_stream;
    uint32_t send_val = 17, recv_val=0;
    int num_received=0, num_to_receive=0;
    int tag;
    char *buf;
    bool success = true;
    std::string testname("test_uint(");

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

    num_to_receive = stream->get_NumEndPoints();
    if( num_to_receive == 0 ){
        test->print("No endpoints in stream\n", testname);
        test->end_SubTest(testname, NOTRUN);
        return -1;
    }


    if(stream->send(PROT_UINT, "%ud", send_val) == -1){
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
            retval = network->recv(&tag, (void**)&buf, &recv_stream, block);
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

            if( Stream::unpack( buf, "%ud", &recv_val ) == -1 ){
                test->print("stream::unpack() failure\n", testname);
                success = false;
            }
            if(send_val != recv_val ){
                sprintf(tmp_buf, "send_val(%ud) != recv_val(%ud) failure.\n",
                        send_val, recv_val);
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

/* 
 *  test_long():
 *    bcast a 64-bit signed int to all endpoints in stream.
 *    recv  a 64-bit signed int from every endpoint
 */
int test_long( Network * network, Stream *stream, bool anonymous, bool block)
{
    Stream *recv_stream;
    int64_t send_val = -17, recv_val=0;
    int num_received=0, num_to_receive=0;
    int tag;
    char *buf;
    bool success = true;
    std::string testname("test_long(");

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

    num_to_receive = stream->get_NumEndPoints();
    if( num_to_receive == 0 ){
        test->print("No endpoints in stream\n", testname);
        test->end_SubTest(testname, NOTRUN);
        return -1;
    }


    if(stream->send(PROT_LONG, "%ld", send_val) == -1){
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
            retval = network->recv(&tag, (void**)&buf, &recv_stream, block);
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

            if( Stream::unpack( buf, "%ld", &recv_val ) == -1 ){
                test->print("stream::unpack() failure\n", testname);
                success = false;
            }
            if( send_val != recv_val ){
                sprintf(tmp_buf, "send_val(%lld) != recv_val(%lld) failure.\n",
                        send_val, recv_val);
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

/* 
 *  test_ulong():
 *    bcast a 64-bit unsigned int to all endpoints in stream.
 *    recv  a 64-bit unsigned int from every endpoint
 */
int test_ulong( Network * network, Stream *stream, bool anonymous, bool block)
{
    Stream *recv_stream;
    uint64_t send_val = 17, recv_val=0;
    int num_received=0, num_to_receive=0;
    int tag;
    char *buf;
    bool success = true;
    std::string testname("test_ulong(");

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

    num_to_receive = stream->get_NumEndPoints();
    if( num_to_receive == 0 ){
        test->print("No endpoints in stream\n", testname);
        test->end_SubTest(testname, NOTRUN);
        return -1;
    }


    if(stream->send(PROT_ULONG, "%uld", send_val) == -1){
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
            retval = network->recv(&tag, (void**)&buf, &recv_stream, block);
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

            if( Stream::unpack( buf, "%uld", &recv_val ) == -1 ){
                test->print("stream::unpack() failure\n", testname);
                success = false;
            }
            if(send_val != recv_val ){
                sprintf(tmp_buf, "send_val(%llu) != recv_val(%llu) failure.\n",
                        send_val, recv_val);
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

/* 
 *  test_float():
 *    bcast a 32-bit floating point number to all endpoints in stream.
 *    recv  a 32-bit floating point number from every endpoint
 */
int test_float( Network * network, Stream *stream, bool anonymous, bool block)
{
    Stream *recv_stream;
    float send_val = -17.234, recv_val=0;
    int num_received=0, num_to_receive=0;
    int tag;
    char *buf;
    bool success = true;
    std::string testname("test_float(");

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

    num_to_receive = stream->get_NumEndPoints();
    if( num_to_receive == 0 ){
        test->print("No endpoints in stream\n", testname);
        test->end_SubTest(testname, NOTRUN);
        return -1;
    }


    if(stream->send(PROT_FLOAT, "%f", send_val) == -1){
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
            retval = network->recv(&tag, (void**)&buf, &recv_stream, block);
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

            if( Stream::unpack( buf, "%f", &recv_val ) == -1 ){
                test->print("stream::unpack() failure\n", testname);
                success = false;
            }
            if(send_val != recv_val ){
                sprintf(tmp_buf, "send_val(%f) != recv_val(%f) failure.\n",
                        send_val, recv_val);
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

/* 
 *  test_double():
 *    bcast a 64-bit floating point number to all endpoints in stream.
 *    recv  a 64-bit floating point number from every endpoint
 */
int test_double( Network * network, Stream *stream, bool anonymous, bool block)
{
    Stream *recv_stream;
    double send_val = 17.3421, recv_val=0;
    int num_received=0, num_to_receive=0;
    int tag;
    char *buf;
    bool success = true;
    std::string testname("test_double(");

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

    num_to_receive = stream->get_NumEndPoints();
    if( num_to_receive == 0 ){
        test->print("No endpoints in stream\n", testname);
        test->end_SubTest(testname, NOTRUN);
        return -1;
    }


    if(stream->send(PROT_DOUBLE, "%lf", send_val) == -1){
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
            retval = network->recv(&tag, (void**)&buf, &recv_stream, block);
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

            if( Stream::unpack( buf, "%lf", &recv_val ) == -1 ){
                test->print("stream::unpack() failure\n", testname);
                success = false;
            }
            if(send_val != recv_val ){
                sprintf(tmp_buf, "send_val(%lf) != recv_val(%lf) failure.\n",
                        send_val, recv_val);
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

/* 
 *  test_string():
 *    bcast a null-terminated string to all endpoints in stream.
 *    recv  a null-terminated string from every endpoint
 */
int test_string( Network * network, Stream *stream, bool anonymous, bool block)
{
    Stream *recv_stream;
    char *send_val=strdup("Test String"), *recv_val=0;
    int num_received=0, num_to_receive=0;
    int tag;
    char *buf;
    bool success = true;
    std::string testname("test_string(");

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

    num_to_receive = stream->get_NumEndPoints();
    if( num_to_receive == 0 ){
        test->print("No endpoints in stream\n", testname);
        test->end_SubTest(testname, NOTRUN);
        return -1;
    }


    if(stream->send(PROT_STRING, "%s", send_val) == -1){
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
            retval = network->recv(&tag, (void**)&buf, &recv_stream, block);
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

            if( Stream::unpack( buf, "%s", &recv_val ) == -1 ){
                test->print("stream::unpack() failure\n", testname);
                success = false;
            }
            if( strcmp(send_val, recv_val) ){
                sprintf(tmp_buf,
                        "send_val(\"%s\") != recv_val(\"%s\") failure.\n",
                        send_val, recv_val);
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

/* 
 *  test_alltypes():
 *    bcast a packet containing data of all types to all endpoints in stream.
 *    recv  a packet containing data of all types from every endpoint
 */
int test_alltypes( Network * network, Stream *stream, bool anonymous, bool block)
{
    Stream *recv_stream;
    int num_received=0, num_to_receive=0;
    int tag;
    char *buf;
    bool success = true;

    char send_char='A', recv_char=0;
    unsigned char send_uchar='B', recv_uchar=0;
    int16_t send_short=-17, recv_short=0;
    uint16_t send_ushort=17, recv_ushort=0;
    int32_t send_int=-17, recv_int=0;
    int32_t send_uint=17, recv_uint=0;
    int64_t send_long=-17, recv_long=0;
    int64_t send_ulong=17, recv_ulong=0;
    float send_float=123.23412, recv_float=0;
    double send_double=123.23412, recv_double=0;
    char *send_string=strdup("Test String"), *recv_string=0;

    std::string testname("test_all(");

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

    num_to_receive = stream->get_NumEndPoints();
    if( num_to_receive == 0 ){
        test->print("No endpoints in stream\n", testname);
        test->end_SubTest(testname, NOTRUN);
        return -1;
    }


    if(stream->send(PROT_ALL, "%c %uc %hd %uhd %d %ud %ld %uld %f %lf %s",
                    send_char, send_uchar,
                    send_short, send_ushort,
                    send_int, send_uint,
                    send_long, send_ulong,
                    send_float, send_double, send_string ) == 1){
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
            retval = network->recv(&tag, (void**)&buf, &recv_stream, block);
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
            char tmp_buf[2048];

            num_received++;
            sprintf(tmp_buf, "Received %d packets; %d left.\n",
                    num_received, num_to_receive-num_received);
            test->print(tmp_buf, testname);

            if(stream->unpack(buf,
                              "%c %uc %hd %uhd %d %ud %ld %uld %f %lf %s",
                              &recv_char, &recv_uchar,
                              &recv_short, &recv_ushort,
                              &recv_int, &recv_uint, &recv_long, &recv_ulong,
                              &recv_float, &recv_double, &recv_string ) == 1){
                test->print("stream::unpack() failure\n", testname);
                success = false;
            }

            if( ( send_char != recv_char ) ||
                ( send_uchar != recv_uchar ) ||
                ( send_short != recv_short ) ||
                ( send_ushort != recv_ushort ) ||
                ( send_int != recv_int ) ||
                ( send_uint != recv_uint ) ||
                ( send_long != recv_long ) ||
                ( send_ulong != recv_ulong ) ||
                ( send_float != recv_float ) ||
                ( send_double != recv_double ) ||
                ( strcmp(send_string, recv_string)) ){
                sprintf(tmp_buf, "Values sent != Values received failure.\n"
                        "  send_char=%c recv_char=%c\n"
                        "  send_uchar=%c recv_uchar=%c\n"
                        "  send_short=%hd recv_short=%hd\n"
                        "  send_ushort=%u recv_ushort=%u\n"
                        "  send_int=%d recv_int=%d\n"
                        "  send_uint=%u recv_uint=%u\n"
                        "  send_long=%lld recv_long=%lld\n"
                        "  send_ulong=%llu recv_ulong=%llu\n"
                        "  send_float=%f recv_float=%f\n"
                        "  send_double=%lf recv_double=%lf\n"
                        "  send_string=%s recv_string=%s\n",
                        send_char, recv_char, send_uchar, recv_uchar,
                        send_short, recv_short, send_ushort, recv_ushort,
                        send_int, recv_int, send_uint, recv_uint,
                        send_long, recv_long,
                        send_ulong, recv_ulong,
                        send_float, recv_float, send_double, recv_double,
                        send_string, recv_string);
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
