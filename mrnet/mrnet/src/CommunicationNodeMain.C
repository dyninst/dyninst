#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>


#include "mrnet/src/MC_CommunicationNode.h"
#include "mrnet/src/utils.h"

int main(int argc, char **argv)
{
  MC_InternalNode *comm_node;
  int i;
  list <MC_Packet *> packet_list;

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

  struct timeval cur_time, last_send_time;
  gettimeofday(&cur_time, NULL);
  last_send_time = cur_time;
  while(1){
    if(comm_node->recv_PacketsFromUpStream(packet_list) == -1){
      mc_printf((stderr, "recv_PacketsFromUpstream() failed\n"));
    }
    if(comm_node->proc_PacketsFromUpStream(packet_list) == -1){
      mc_printf((stderr, "proc_PacketsFromUpstream() failed\n"));
    }
    if(comm_node->recv_PacketsFromDownStream(packet_list) == -1){
      mc_printf((stderr, "recv_PacketsFromDownstream() failed\n"));
    }
    if(comm_node->proc_PacketsFromDownStream(packet_list) == -1){
      mc_printf((stderr, "proc_PacketsFromDownstream() failed\n"));
    }
    gettimeofday(&cur_time, NULL);
    if( ((cur_time.tv_sec - last_send_time.tv_sec) * 1000000) +
	(cur_time.tv_usec - last_send_time.tv_usec) > 1000000 ){
      //mc_printf((stderr, "Time to flush packets\n"));
      if(comm_node->flush_PacketsDownStream() == -1){
	last_send_time = cur_time;
        mc_printf((stderr, "flush_PacketsDownstream() failed\n"));
      }
      if(comm_node->flush_PacketsUpStream() == -1){
        mc_printf((stderr, "flush_PacketsUpStream() failed\n"));
      }
    }
  }

  return 0;
}
