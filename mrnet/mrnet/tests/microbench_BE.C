#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "mrnet/h/MR_Network.h"
#include "mrnet/src/utils.h"
#include "mrnet/tests/timer.h"
#include "mrnet/tests/microbench.h"

using namespace MRN;

int main(int argc, char **argv){
   Stream * stream;
   char * buf=NULL;
   int tag, num_waves;
   mb_time be_start, bc_recv;
   long int start_secs;
   struct timeval tmp_tv;

   be_start.set_time();
   if( Network::init_Backend(argv[argc-5], argv[argc-4], argv[argc-3],
                                argv[argc-2], argv[argc-1]) == -1){
      fprintf(stderr, "%s: backend_init() failed\n", argv[0]);
      return -1;
   }

   while(1){ //keep looping through until we get exit protocol
      while(1){
         if ( Stream::recv(&tag, (void **)&buf, &stream) == 1){
            break;
         }
      }
      
      if(tag == MB_SEND_BROADCAST_RECV_TIME){
          bc_recv.set_time();
          stream->unpack(buf, "%d %ld", &num_waves, &start_secs );

          //wait until time start_secs, then blast away
          do{
              while( gettimeofday(&tmp_tv, NULL) == -1 );
          } while( tmp_tv.tv_sec < start_secs );

          bc_recv.get_time( &tmp_tv);
          //printf("BE[%s.%d] sending bc_recv: %ld.%ld\n", argv[argc-4], getpid(), tmp_tv.tv_sec, //tmp_tv.tv_usec );
          //printf("BE[%s.%d] sending bc_recv: %lf\n", argv[argc-4], getpid(), bc_recv.get_double_time());
          //printf("\tsend time is %ld.%ld. should be: %ld.0\n", tmp_tv.tv_sec, tmp_tv.tv_usec, start_secs);
          for(int i=0; i<num_waves; i++){
              if(stream->send(tag, "%lf", bc_recv.get_double_time() ) == -1){
                  exit(-1);
              }
              if(stream->flush() == -1){
                  exit(-1);
              }
          }
      }
      else if(tag == MB_SEND_STARTUP_TIME){
           //printf("BE[%s] sending be_start: %lf\n", argv[argc-4], be_start.get_double_time());
         if(stream->send(tag, "%lf", be_start.get_double_time() ) == -1){
            exit(-1);
         }
         if(stream->flush() == -1){
            exit(-1);
         }
      }
      else if(tag == MB_EXIT){
         exit(0);
      }
   }
}
