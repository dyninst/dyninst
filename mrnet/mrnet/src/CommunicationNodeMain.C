#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>


#include "mrnet/src/MC_CommunicationNode.h"
#include "mrnet/src/utils.h"

int main(int argc, char **argv)
{
  MC_InternalNode *comm_node;
  int i, status;
  list <MC_Packet *> packet_list;

  if( (status = pthread_key_create(&thread_name_key, NULL)) != 0){
    fprintf(stderr, "pthread_key_create(): %s\n", strerror(status)); 
    exit(-1);
  }
  tsd_t * local_data = new tsd_t;
  local_data->thread_id = pthread_self();

  if(argc !=3 && argc != 4){  // remember arg of -l0 added by rpccreateproc()
    mc_printf((stderr, "Usage: %s hostname port\n", argv[0]));
    mc_printf((stderr, "Called with (%d) args: ", argc));
    for(i=0; i<argc; i++){
      mc_printf((stderr, "%s ", argv[i]));
    }
    exit(-1);
  }

  string parent_hostname(argv[1]);
  unsigned short parent_port = atol(argv[2]);
  comm_node = new MC_InternalNode(parent_hostname, parent_port);

  comm_node->waitLoop();

  return 0;
}
