#include <unistd.h>
#include "test1.h"
#include "timer.h"
#include "mrnet/h/MR_Network.h"
using namespace MRN;

int main(int argc, char **argv){
  int filter_id = 0; /* Not yet meaningful */
  Stream * stream_BC;
  int send_val, recv_val, tag;
  char * buf=NULL;
  timer init_timer("FE:NETWORK_INIT"), exp_timer("FE:BROADCAST/REDUCE");

  if(argc !=3){
    fprintf(stderr, 
            "FFF: Usage: %s <topology file> <application exe>\n",
            argv[0]);
    exit(-1);
  }

  init_timer.start();
  if( Network::new_Network(argv[1], argv[2]) == -1){
    fprintf(stderr, "FFF: Network Initialization failed\n");
    Network::error_str(argv[0]);
    exit(-1);
  }
  init_timer.end();

  //fprintf(stderr, "FFF: Getting Broadcast communicator ...\n");
  Communicator * comm_BC = Communicator::get_BroadcastCommunicator();

  //fprintf(stderr, "FFF: Creating New Stream ...\n");
  stream_BC = Stream::new_Stream(comm_BC, filter_id);

  send_val = 1;
  fprintf(stderr, "FFF: sending integer on stream ...\n");
  exp_timer.start();
  if(stream_BC->send(PROT_HELLO, "%d", send_val) == -1){
    fprintf(stderr, "FFF: stream.send() failed\n");
    exit(-1);
  }
  exp_timer.end();
  fprintf(stderr, "FFF: stream.send() succeeded\n");

  if(stream_BC->flush() == -1){
    fprintf(stderr, "FFF: stream.flush() failed\n");
    exit(-1);
  }
  fprintf(stderr, "FFF: stream.flush() succeeded\n");

    unsigned int nTries = 0;
    unsigned int nReceived = 0;
    while( (nReceived < (unsigned int)comm_BC->size()) && 
            (nTries < (unsigned int)(5*comm_BC->size())) )
    {
        Stream* stream = NULL;

        nTries++;
        fprintf(stderr, "FFF: calling recv() on stream\n" );

        if( Stream::recv(&tag, (void**)&buf, &stream) > 0 )
        {
            stream->unpack( buf, "%d", &recv_val );
            fprintf(stderr, "FFF: Received val: %d from a backend\n", recv_val);
            nReceived++;
        }
    }

  fprintf(stderr, "FFF: deleting network\n" );
  Network::delete_Network();
  fprintf(stderr, "FFF: frontend deleted. Exiting ...\n");
  init_timer.print_start();
  init_timer.print_end();
  exp_timer.print_start();
  exp_timer.print_end();

  return 0;
}
