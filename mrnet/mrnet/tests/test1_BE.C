#include "mrnet/h/MC_Network.h"
#include "test1.h"
#include "timer.h"

int main(int argc, char **argv){
  MC_Stream * stream;
  char * buf=NULL;
  int tag, recv_val;
  timer exp_timer("BE:RECV/SEND");

  if( MC_Network::init_Backend(argv[argc-5], argv[argc-4], argv[argc-3],
                               argv[argc-2], argv[argc-1]) == -1){
    fprintf(stderr, "BBB: backend_init() failed\n");
    return -1;
  }

  while(1){
    if ( MC_Stream::recv(&tag, (void **)&buf, &stream) == 1){
      fprintf(stderr, "BBB: recv() succeeded\n");
      break;
    }
    //fprintf(stderr, "BBB: recv() failed\n");
  }
  exp_timer.start();
  if(tag != PROT_HELLO){
    fprintf(stderr, "BBB: Protocol Error: tag = %d\n", tag);
    return -1;
  }

  stream->unpack(buf, "%d", &recv_val);

  fprintf(stderr, "BBB: Recieved val: %d from Frontend\n", recv_val);

  if(stream->send(PROT_HELLO, "%d", recv_val*2) == -1){
    fprintf(stderr, "BBB: stream.send() failed\n");
    exit(-1);
  }
  fprintf(stderr, "BBB: stream.send() succeeded\n");

  if(stream->flush() == -1){
    fprintf(stderr, "BBB: stream.flush() failed\n");
   exit(-1);
  }
  fprintf(stderr, "BBB: stream.flush() succeeded\n");


  //fprintf(stderr, "BBB: Exiting\n");
  exp_timer.print_start();
  exit(0);
}
