/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#include "mrnet/MRNet.h"
#include "mrnet/src/Types.h"
#include "mrnet/src/DataElement.h"
#include "test_common.h"
#include "test_NativeFilters.h"

#include <string>

using namespace MRN;
using namespace MRN_test;
Test * test;

int test_Sum( Network * network, DataType type );

int main(int argc, char **argv)
{
    if( argc != 3 ){
        fprintf(stderr, "Usage: %s <topology file> <backend_exe>\n", argv[0]);
        exit(-1);
    }
    const char * topology_file = argv[1];
    const char * backend_exe = argv[2];

    fprintf(stderr,"MRNet C++ Interface *Native Filter* Test Suite\n"
            "--------------------------------------\n"
            "This test suite performs tests that exercise\n"
            "MRNet's built-in filters.\n\n");

    test = new Test("MRNet Native Filter Test");
    Network * network = new Network( topology_file, backend_exe );
    if( network->fail() ){
        fprintf(stderr, "Network Initialization failure\n");
        network->error_str(argv[0]);
        exit(-1);
    }

    test_Sum( network, CHAR_T );
    test_Sum( network, UCHAR_T );
    test_Sum( network, INT16_T );
    test_Sum( network, UINT16_T );
    test_Sum( network, INT32_T );
    test_Sum( network, UINT32_T );
    test_Sum( network, INT64_T );
    test_Sum( network, UINT64_T );
    test_Sum( network, FLOAT_T );
    test_Sum( network, DOUBLE_T );
  
    Communicator * comm_BC = network->get_BroadcastCommunicator( );
    Stream * stream = network->new_Stream( comm_BC );
    if(stream->send(PROT_EXIT, "") == -1){
        test->print("stream::send(exit) failure\n");
        return -1;
    }

    if(stream->flush() == -1){
        test->print("stream::flush() failure\n");
        return -1;
    }

    delete network;

    test->end_Test();
    delete test;

    return 0;
}

int test_Sum( Network * network, DataType type )
{
    void * buf;
    char recv_val[8];
    int tag=0, retval=0;
    std::string testname;
    bool success=true;

    switch(type){
    case CHAR_T:
        tag = PROT_CHAR_SUM;
        testname = "test_Sum(char)";
        break;
    case UCHAR_T:
        tag = PROT_UCHAR_SUM;
        testname = "test_Sum(uchar_t)";
        break;
    case INT16_T:
        tag = PROT_INT16_SUM;
        testname = "test_Sum(int16_t)";
        break;
    case UINT16_T:
        tag = PROT_UINT16_SUM;
        testname = "test_Sum(uint16_t)";
        break;
    case INT32_T:
        tag = PROT_INT32_SUM;
        testname = "test_Sum(int32_t)";
        break;
    case UINT32_T:
        tag = PROT_UINT32_SUM;
        testname = "test_Sum(uint32_t)";
        break;
    case INT64_T:
        tag = PROT_INT64_SUM;
        testname = "test_Sum(int64_t)";
        break;
    case UINT64_T:
        tag = PROT_UINT64_SUM;
        testname = "test_Sum(uint64_t)";
        break;
    case FLOAT_T:
        tag = PROT_FLOAT_SUM;
        testname = "test_Sum(float)";
        break;
    case DOUBLE_T:
        tag = PROT_DOUBLE_SUM;
        testname = "test_Sum(double)";
        break;
    case CHAR_ARRAY_T:
    case UCHAR_ARRAY_T:
    case INT16_ARRAY_T:
    case UINT16_ARRAY_T:
    case INT32_ARRAY_T:
    case UINT32_ARRAY_T:
    case INT64_ARRAY_T:
    case UINT64_ARRAY_T:
    case FLOAT_ARRAY_T:
    case DOUBLE_ARRAY_T:
    case STRING_T:
    case UNKNOWN_T:
    default:
        return -1;
    }

    test->start_SubTest(testname);

    Communicator * comm_BC = network->get_BroadcastCommunicator( );
    Stream * stream = network->new_Stream(comm_BC, TFILTER_SUM,
                                          SFILTER_WAITFORALL);
    int num_backends = stream->get_NumEndPoints();

    if( stream->send(tag, "") == -1 ){
        test->print("stream::send() failure\n", testname);
        test->end_SubTest(testname, FAILURE);
        return -1;
    }

    if( stream->flush( ) == -1 ){
        test->print("stream::flush() failure\n", testname);
        test->end_SubTest(testname, FAILURE);
        return -1;
    }

    retval = stream->recv(&tag, (void**)&buf);
    assert( retval != 0 ); //shouldn't be 0, either error or block till data
    if( retval == -1){
        //recv error
        test->print("stream::recv() failure\n", testname);
        test->end_SubTest(testname, FAILURE);
        return -1;
    }
    else{
        //Got data
        switch(type){
            char tmp_buf[1024];
        case CHAR_T:
            if( Stream::unpack( buf, "%c", recv_val ) == -1 ){
                test->print("stream::unpack() failure\n", testname);
                return -1;
            }
            if( *((char*)recv_val) != num_backends * CHARVAL ){
                sprintf(tmp_buf,
                        "recv_val(%d) != CHARVAL(%d)*num_backends(%d):%d.\n",
                        *((char*)recv_val), CHARVAL, num_backends,
                        CHARVAL*num_backends );
                test->print(tmp_buf, testname);
                success = false;
            }
            break;
        case UCHAR_T:
            if( Stream::unpack( buf, "%uc", recv_val ) == -1 ){
                test->print("stream::unpack() failure\n", testname);
                return -1;
            }
            if( *((unsigned char*)recv_val) != ((unsigned char)num_backends) * UCHARVAL ){
                sprintf(tmp_buf,
                        "recv_val(%d) != UCHARVAL(%d)*num_backends(%d):%d.\n",
                        *((unsigned char*)recv_val), UCHARVAL, num_backends,
                        UCHARVAL*num_backends );
                test->print(tmp_buf, testname);
                success = false;
            }
            break;
        case INT16_T:
            if( Stream::unpack( buf, "%hd", recv_val ) == -1 ){
                test->print("stream::unpack() failure\n", testname);
                return -1;
            }
            if( *((int16_t*)recv_val) != num_backends * INT16VAL ){
                sprintf(tmp_buf,
                        "recv_val(%hd) != INT16VAL(%hd)*num_backends(%hd):%hd.\n",
                        *((int16_t*)recv_val), INT16VAL, num_backends,
                        INT16VAL*num_backends );
                test->print(tmp_buf, testname);
                success = false;
            }
            break;
        case UINT16_T:
            if( Stream::unpack( buf, "%uhd", recv_val ) == -1 ){
                test->print("stream::unpack() failure\n", testname);
                return -1;
            }
            if( *((uint16_t*)recv_val) != num_backends * UINT16VAL ){
                sprintf(tmp_buf,
                        "recv_val(%u) != UINT16VAL(%u)*num_backends(%u):%u.\n",
                        *((uint16_t*)recv_val), UINT16VAL, num_backends,
                        UINT16VAL*num_backends );
                test->print(tmp_buf, testname);
                success = false;
            }
            break;
        case INT32_T:
            if( Stream::unpack( buf, "%d", recv_val ) == -1 ){
                test->print("stream::unpack() failure\n", testname);
                return -1;
            }
            if( *((int32_t*)recv_val) != num_backends * INT32VAL ){
                sprintf(tmp_buf,
                        "recv_val(%d) != INT32VAL(%d)*num_backends(%d):%d.\n",
                        *((int32_t*)recv_val), INT32VAL, num_backends,
                        INT32VAL*num_backends );
                test->print(tmp_buf, testname);
                success = false;
            }
            break;
        case UINT32_T:
            if( Stream::unpack( buf, "%ud", recv_val ) == -1 ){
                test->print("stream::unpack() failure\n", testname);
                return -1;
            }
            if( *((uint32_t*)recv_val) != num_backends * UINT32VAL ){
                sprintf(tmp_buf,
                        "recv_val(%u) != UINT32VAL(%u)*num_backends(%u):%u.\n",
                        *((uint32_t*)recv_val), UINT32VAL, num_backends,
                        UINT32VAL*num_backends );
                test->print(tmp_buf, testname);
                success = false;
            }
            break;
        case INT64_T:
            if( Stream::unpack( buf, "%ld", recv_val ) == -1 ){
                test->print("stream::unpack() failure\n", testname);
                return -1;
            }
            if( *((int64_t*)recv_val) != num_backends * INT64VAL ){
                sprintf(tmp_buf,
                        "recv_val(%lld) != INT64VAL(%lld)*num_backends(%d):%lld.\n",
                        *((int64_t*)recv_val), INT64VAL, num_backends,
                        INT64VAL*num_backends );
                test->print(tmp_buf, testname);
                success = false;
            }
            break;
        case UINT64_T:
            if( Stream::unpack( buf, "%uld", recv_val ) == -1 ){
                test->print("stream::unpack() failure\n", testname);
                return -1;
            }
            if( *((uint64_t*)recv_val) != num_backends * UINT64VAL ){
                sprintf(tmp_buf,
                        "recv_val(%llu) != INT64VAL(%llu)*num_backends(%d):%llu.\n",
                        *((uint64_t*)recv_val), UINT64VAL, num_backends,
                        UINT64VAL*(uint64_t)num_backends );
                test->print(tmp_buf, testname);
                success = false;
            }
            break;
        case FLOAT_T:
            if( Stream::unpack( buf, "%f", recv_val ) == -1 ){
                test->print("stream::unpack() failure\n", testname);
                return -1;
            }
            if( !compare_Float( *(float*)recv_val, num_backends * FLOATVAL, 3) ){
                sprintf(tmp_buf,
                        "recv_val(%f) != FLOATVAL(%f)*num_backends(%d):%f.\n",
                        *((float*)recv_val), FLOATVAL, num_backends,
                        FLOATVAL*num_backends);
                test->print(tmp_buf, testname);
                success = false;
            }
            break;
        case DOUBLE_T:
            if( Stream::unpack( buf, "%lf", recv_val ) == -1 ){
                test->print("stream::unpack() failure\n", testname);
                return -1;
            }
            if( !compare_Double(*(double*)recv_val, num_backends*DOUBLEVAL, 3) ){
                sprintf(tmp_buf,
                        "recv_val(%lf) != DOUBLEVAL(%lf)*num_backends(%d):%lf.\n",
                        *((double*)recv_val), DOUBLEVAL, num_backends,
                        num_backends*DOUBLEVAL);
                test->print(tmp_buf, testname);
                success = false;
            }
            break;
        case CHAR_ARRAY_T:
        case UCHAR_ARRAY_T:
        case INT16_ARRAY_T:
        case UINT16_ARRAY_T:
        case INT32_ARRAY_T:
        case UINT32_ARRAY_T:
        case INT64_ARRAY_T:
        case UINT64_ARRAY_T:
        case FLOAT_ARRAY_T:
        case DOUBLE_ARRAY_T:
        case STRING_T:
        case UNKNOWN_T:
        default:
            success=false;
            return -1;
        }
    }

    if(success){
        test->end_SubTest(testname, SUCCESS);
    }
    else{
        test->end_SubTest(testname, FAILURE);
    }
    return 0;
}
