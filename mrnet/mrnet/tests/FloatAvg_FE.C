#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "mrnet/h/MR_Network.h"
#include "mrnet/tests/FloatAvg.h"

using namespace MRN;

int main(int argc, char **argv){
  char *topology_file, *application;
  int num_trials;
  Stream * stream_BC;
  int tag;
  float *recv_vals;
  char * buf=NULL;
  //timer init_timer("FE:NETWORK_INIT"), exp_timer("FE:BROADCAST/REDUCE");

  if(argc !=4){
    fprintf(stderr,
	    "Usage: %s <num_trials> <topology file> <application exe> ",
	    argv[0]);
    exit(-1);
  }

  num_trials = atoi(argv[1]); assert(num_trials > 0);
  recv_vals = (float *) malloc(num_trials * sizeof(float));
  topology_file = argv[2];
  application = argv[3];

  if( Network::new_Network(topology_file, application) == -1){
    fprintf(stderr, "%s: Network Initialization failed\n", argv[0]);
    Network::error_str(argv[0]);
    exit(-1);
  }

  Communicator * comm_BC = Communicator::get_BroadcastCommunicator();

  stream_BC = Stream::new_Stream(comm_BC, AGGR_FLOAT_AVG_ID);

  fprintf(stderr, "%s: sending number of trials to BEs: %d ...\n", argv[0],
          num_trials);

  if(stream_BC->send(FLOAT_AVG_PROT, "%d", num_trials) == -1){
    fprintf(stderr, "%s: stream.send() failed\n", argv[0]);
    exit(-1);
  }

  fprintf(stderr, "%s: flushing the stream\n", argv[0]);
  if(stream_BC->flush() == -1){
    fprintf(stderr, "FFF: stream.flush() failed\n");
    exit(-1);
  }

  for (int i = 0; i < num_trials; ){
    Stream * stream;

    if(Stream::recv(&tag, (void **)&buf, &stream) > 0){
      stream->unpack(buf, "%f", recv_vals+i);
      fprintf(stderr, "%s: Recieved val: %f from backends\n", argv[0],
              recv_vals[i]);
      i++;
    }
  }

  Network::delete_Network();
  fprintf(stderr, "%s: Experiment Summary:\n\tReceived Values: [", argv[0]);
  for(int i=0; i<num_trials; i++){
    fprintf(stderr, "%f%s", recv_vals[i], (i==num_trials-1? "]": ", "));
  }
}
