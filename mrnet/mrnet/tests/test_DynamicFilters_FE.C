/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#include "mrnet/MRNet.h"
#include "test_common.h"
#include "test_DynamicFilters.h"

#include <string>
#include <assert.h>

using namespace MRN;
using namespace MRN_test;
Test * test;

int test_CountFilter( Network *,  const char * so_file );
int test_CountOddsAndEvensFilter(  Network *, const char * so_file );

int main(int argc, char **argv)
{
    if( argc != 4 ){
        fprintf(stderr, "Usage: %s <shared_object file> <topology file> "
                "<backend_exe>\n", argv[0]);
        exit(-1);
    }
    const char * so_file = argv[1];
    const char * topology_file = argv[2];
    const char * backend_exe = argv[3];

    fprintf(stderr,"MRNet C++ Interface *Dynamic Filter* Test Suite\n"
            "--------------------------------------\n"
            "This test suite performs tests that exercise\n"
            "MRNet's \"Filter Loading\" functionality.\n");

    test = new Test("MRNet Dynamic Filter Test");

	const char * dummy_argv=NULL;
    Network * network = new Network( topology_file, backend_exe, &dummy_argv );
    if( network->fail() ){
        fprintf(stderr, "Network Initialization failure\n");
        network->print_error(argv[0]);
        exit(-1);
    }

    /* For all the following tests, the 1st bool param indicates *
     * whether the recv() call should be stream-anonymous or not *
     * and the 2nd bool param indicates whether the recv should block *
     * or not */
    test_CountFilter( network, so_file );

    //WARNING: This test_CountOddsAndEvensFilter() must be the last test
    //because it is the one that instructs the backends to exit.
    test_CountOddsAndEvensFilter( network, so_file );
  
    Communicator * comm_BC = network->get_BroadcastCommunicator( );
    Stream * stream = network->new_Stream(comm_BC, TFILTER_NULL, SFILTER_WAITFORALL);

    if(stream->send(PROT_EXIT, "") == -1){
        fprintf(stderr, "stream->send(\"PROT_EXIT\") failed\n");
        network->print_error(argv[0]);
        return -1;
    }

    if(stream->flush() == -1){
        fprintf(stderr, "stream->flush() failed\n");
        network->print_error(argv[0]);
        return -1;
    }

    delete network;

    test->end_Test();
    delete test;

    return 0;
}

int test_CountFilter( Network * network, const char * so_file )
{
    int retval, tag, recv_val=0;
    Packet * buf;
    std::string testname("test_Count"); 

    test->start_SubTest(testname);

    int filter_id = network->load_FilterFunc( so_file, "aggr_Count" );
    if( filter_id == -1 ){
        test->print("Stream::load_FilterFunc() failure\n", testname);
        test->end_SubTest(testname, FAILURE);
        return -1;
    }

    Communicator * comm_BC = network->get_BroadcastCommunicator( );
    Stream * stream = network->new_Stream(comm_BC, filter_id, SFILTER_WAITFORALL);

    if( stream->send(PROT_COUNT, "") == -1 ){
        test->print("stream::send() failure\n", testname);
        test->end_SubTest(testname, FAILURE);
        return -1;
    }
    if( stream->flush( ) == -1 ){
        test->print("stream::flush() failure\n", testname);
        test->end_SubTest(testname, FAILURE);
        return -1;
    }

    retval = stream->recv(&tag, &buf);
    assert( retval != 0 ); //shouldn't be 0, either error or block till data
    if( retval == -1){
        //recv error
        test->print("stream::recv() failure\n", testname);
        test->end_SubTest(testname, FAILURE);
        return -1;
    }
    else{
        //Got data
        if( Stream::unpack( buf, "%d", &recv_val ) == -1 ){
            test->print("stream::unpack() failure\n", testname);
            return -1;
        }
        if( recv_val != (int)stream->get_NumEndPoints( ) ){
            char tmp_buf[256];
            sprintf(tmp_buf, "recv_val(%d) != NumEndPoints(%d). Failure.\n",
                    recv_val, stream->get_NumEndPoints( ) );
            test->print(tmp_buf, testname);
            test->end_SubTest(testname, FAILURE);
            return -1;
        }
    }

    test->end_SubTest(testname, SUCCESS);
    return 0;
}

int test_CountOddsAndEvensFilter( Network * network, const char * so_file )
{
    int num_odds=0, num_evens=0, retval, tag=0;
    Packet * buf;
    std::string testname("test_CountOddsAndEvens"); 

    test->start_SubTest(testname);

    int filter_id = network->load_FilterFunc( so_file,
                                             "aggr_CountOddsAndEvens" );
    if( filter_id == -1 ){
        test->print("Stream::load_FilterFunc() failure\n", testname);
        test->end_SubTest(testname, FAILURE);
        return -1;
    }

    Communicator * comm_BC = network->get_BroadcastCommunicator( );
    Stream * stream = network->new_Stream(comm_BC, filter_id, SFILTER_WAITFORALL);

    if( stream->send(PROT_COUNTODDSANDEVENS, "") == -1 ){
        test->print("stream::send() failure\n", testname);
        test->end_SubTest(testname, FAILURE);
        return -1;
    }

    if( stream->flush( ) == -1 ){
        test->print("stream::flush() failure\n", testname);
        test->end_SubTest(testname, FAILURE);
        return -1;
    }

    retval = stream->recv(&tag, &buf);
    assert( retval != 0 ); //shouldn't be 0, either error or block till data
    if( retval == -1){
        //recv error
        test->print("stream::recv() failure\n", testname);
        test->end_SubTest(testname, FAILURE);
        return -1;
    }
    else{
        //Got data
        if( Stream::unpack( buf, "%d %d", &num_odds, &num_evens ) == -1 ){
            test->print("stream::unpack() failure\n", testname);
            test->end_SubTest(testname, FAILURE);
            return -1;
        }
        char tmp_buf[256];
        sprintf(tmp_buf, "num_odds = %d; num_evens = %d\n",
                num_odds, num_evens);
        test->print(tmp_buf, testname);
    }

    test->end_SubTest(testname, SUCCESS);
    return 0;
}
