#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "mrnet/h/MRNet.h"
#include "mrnet/src/utils.h"
#include "mrnet/tests/test_NativeFilters.h"

using namespace MRN;

int main(int argc, char **argv){
  Stream * stream;
  char * buf=NULL;
  int tag, num_trials;
  float *send_vals=0;

  if( Network::init_Backend(argv[argc-5], argv[argc-4],
                               argv[argc-3], argv[argc-2], argv[argc-1]) == -1){
    fprintf(stderr, "%s: backend_init() failed\n", argv[0]);
    return -1;
  }

  while(1){
    if ( Stream::recv(&tag, (void **)&buf, &stream) == 1){
      break;
    }
  }

  if(tag != FLOAT_AVG_PROT){
    fprintf(stderr, "%s: Protocol Error: tag = %d\n", argv[0], tag);
    return -1;
  }

  stream->unpack(buf, "%d", &num_trials);

  fprintf(stderr, "%s: Recieved num_trials: %d from Frontend\n", argv[0],
          num_trials);

  assert(num_trials > 0);
  for(int i=0; i<num_trials; i++){
    send_vals[i] = i+10;
  }

  for(int i=0; i<num_trials; i++){
    mrn_printf(3, MCFL, stderr, "sending float: %f\n", send_vals[i]);
    if(stream->send(FLOAT_AVG_PROT, "%f", send_vals[i]) == -1){
      fprintf(stderr, "%s: stream.send() failed\n", argv[0]);
      exit(-1);
    }
  }

  fprintf(stderr, "%s: stream.send() succeeded\n", argv[0]);

  if(stream->flush() == -1){
    fprintf(stderr, "%s: stream.flush() failed\n", argv[0]);
   exit(-1);
  }

  fprintf(stderr, "%s: stream.flush() succeeded\n", argv[0]);
  exit(0);
}
