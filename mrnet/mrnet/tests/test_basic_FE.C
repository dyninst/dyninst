#include "mrnet/h/MR_Network.h"
#include "test_common.h"
#include "test_basic.h"

#include <string>

using namespace MRN;
using namespace MRN_test;
Test * test;

void test_int(Stream *stream, bool anonymous=false, bool block=true)
{
    Stream *recv_stream;
    int send_val = 17, recv_val=0, num_received=0, num_to_receive=0;
    int tag;
    char *buf;
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

    if(stream->send(PROT_SENDINT, "%d", send_val) == -1){
        test->end_SubTest(testname, FAILURE);
        return;
    }

    if(stream->flush() == -1){
        test->end_SubTest(testname, FAILURE);
        return;
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
            //Error
            test->end_SubTest(testname, FAILURE);
            return;
        }
        else if ( retval == 0 ){
            //No data available
        }
        else{
            //Got data
            num_received++;
            if( Stream::unpack( buf, "&d", &recv_val ) == -1 ){
                //unpack error
                test->end_SubTest(testname, FAILURE);
                return;
            }
            if(send_val != recv_val ){
                test->end_SubTest(testname, FAILURE);
                return;
                break;
            }
        }
    } while(num_received < num_to_receive);
}

int main(int argc, char **argv)
{
    Stream * stream_BC;
    char dummy;

    if(argc !=4){
        fprintf(stderr, 
                "Usage: %s <topology file> <backend_exe>\n", argv[0]);
        exit(-1);
    }

    fprintf(stderr,"MRNet C++ Interface *Basic* Test Suite\n"
            "--------------------------------------\n"
            "This test suite performs many tests that exercise\n"
            "MRNet's basic functionality. These tests use all\n"
            "MRNet data types, and the popular functions\n"
            "in the interface.\n"
            "Press <enter> to start the testing...\n");

    scanf("%c",&dummy);

    test = new Test("MRNet Basic Test");
    if( Network::new_Network(argv[1], argv[2]) == -1){
        fprintf(stderr, "Network Initialization failed\n");
        Network::error_str(argv[0]);
        exit(-1);
    }

    Communicator * comm_BC = Communicator::get_BroadcastCommunicator();

    stream_BC = Stream::new_Stream(comm_BC, AGGR_NULL, SYNC_DONTWAIT);

    test_int(stream_BC);
  
    Network::delete_Network();

    test->end_Test();
    delete test;

    return 0;
}
