#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>


#include <list>

#include "mrnet/src/Message.h"
#include "mrnet/src/InternalNode.h"
#include "mrnet/src/utils.h"

int main(int argc, char **argv)
{
  MC_InternalNode *comm_node;
  int i, status;
  std::list <MC_Packet *> packet_list;

  if(argc != 6){
    fprintf(stderr, "Usage: %s hostname port phostname pport pid\n",
               argv[0]);
    fprintf(stderr, "Called with (%d) args: ", argc);
    for(i=0; i<argc; i++){
      fprintf(stderr, "%s ", argv[i]);
    }
    exit(-1);
  }

  std::string hostname(argv[1]);
  unsigned short port = atoi(argv[2]);
  std::string parent_hostname(argv[3]);
  unsigned short parent_port = atol(argv[4]);
  unsigned short parent_id = atol(argv[5]);

  //TLS: setup thread local storage for internal node
  //I am "COMM(hostname:port)"
  std::string name("COMM(");
  name += getHostName(hostname);
  name += ":";
  name += argv[2];
  name += ")";

  if( (status = pthread_key_create(&tsd_key, NULL)) != 0){
    fprintf(stderr, "pthread_key_create(): %s\n", strerror(status)); 
    exit(-1);
  }
  tsd_t * local_data = new tsd_t;
  local_data->thread_id = pthread_self();
  local_data->thread_name = strdup(name.c_str());
  if( (status = pthread_setspecific(tsd_key, local_data)) != 0){
    fprintf(stderr, "pthread_key_create(): %s\n", strerror(status)); 
    exit(-1);
  }

  comm_node = new MC_InternalNode(hostname, port, parent_hostname,
                                  parent_port, parent_id);

  comm_node->waitLoop();

  return 0;
}
