/*===========================================================*/
/*             MC_Network Class DEFINITIONS                  */
/*===========================================================*/
#include <stdio.h>

#include "mrnet/src/MC_NetworkImpl.h"
#include "mrnet/src/MC_CommunicatorImpl.h"
#include "mrnet/src/utils.h"

extern MC_NetworkGraph * parsed_graph;

MC_NetworkImpl::MC_NetworkImpl(const char * _filename, const char * _application)
  :filename(_filename), application(_application)
{
  if( parse_configfile() == -1){
    return;
  }

  graph = parsed_graph;  
  if ( graph->has_cycle() ){
    //not really a cycle, but graph is not at least tree.
    mc_errno = MC_ENETWORK_CYCLE;
    _fail=true;
    return;
  }

  //TLS: setup thread local storage for frontend
  //I am "FE(hostname:port)"
  char port_str[128];
  sprintf(port_str, "%d", graph->get_Root()->get_Port());
  string name("FE(");
  name += graph->get_Root()->get_HostName();
  name += ":";
  name += port_str;
  name += ")";

  int status;
  if( (status = pthread_key_create(&tsd_key, NULL)) != 0){
    fprintf(stderr, "pthread_key_create(): %s\n", strerror(status)); 
    assert(0);
  }
  tsd_t * local_data = new tsd_t;
  local_data->thread_id = pthread_self();
  local_data->thread_name = strdup(name.c_str());
  if( (status = pthread_setspecific(tsd_key, (const void *)local_data)) != 0){
    fprintf(stderr, "pthread_key_create(): %s\n", strerror(status)); 
    exit(-1);
  }

  MC_NetworkImpl::endpoints = graph->get_EndPoints();
  MC_CommunicatorImpl::create_BroadcastCommunicator(MC_NetworkImpl::endpoints);

  //Frontend is root of the tree
  MC_NetworkImpl::front_end = 
    new MC_FrontEndNode(graph->get_Root()->get_HostName(),
                        graph->get_Root()->get_Port());

  MC_SerialGraph sg = graph->get_SerialGraph();
  sg.print();
  MC_Packet *packet = new MC_Packet(MC_NEW_SUBTREE_PROT, "%s%s",
                                    sg.get_ByteArray().c_str(),
				    application.c_str() );
  if( MC_NetworkImpl::front_end->proc_newSubTree( packet )
      == -1){
    mc_errno = MC_ENETWORK_FAILURE;
    _fail=true;
    return;
  }

  //packet = new MC_Packet(MC_NEW_APPLICATION_PROT, "%s", application.c_str());
  //if( MC_NetworkImpl::front_end->proc_newApplication(packet) == -1){
    //mc_errno = MC_ENETWORK_FAILURE;
    //_fail=true;
    //return;
  //}

  return;
}

extern FILE * yyin;
int yyparse();
extern int yydebug;

int MC_NetworkImpl::parse_configfile()
{
  //yydebug=1;
  yyin = fopen(filename.c_str(), "r");
  if( yyin == NULL){
    mc_errno = MC_EBADCONFIG_IO;
    _fail = true;
    return -1;
  }

  if( yyparse() != 0 ){
    mc_errno = MC_EBADCONFIG_FMT;
    _fail = true;
    return -1;
  }

  return 0;
}

MC_NetworkImpl::~MC_NetworkImpl()
{
  delete graph;
  delete endpoints;
  delete front_end;
}

MC_EndPoint * MC_NetworkImpl::get_EndPoint(const char * _hostname,
                                           unsigned short _port)
{
  unsigned int i;

  for(i=0; i<MC_NetworkImpl::endpoints->size(); i++){
    if((*MC_NetworkImpl::endpoints)[i]->compare(_hostname, _port) == true){
      return (*MC_NetworkImpl::endpoints)[i];
    }
  }

  return NULL;
}

int MC_NetworkImpl::recv(void)
{
  if(MC_Network::network){
    mc_printf(MCFL, stderr, "In net.recv(). Calling frontend.recv()\n");
    return MC_Network::network->front_end->recv();
  }
  else{
    mc_printf(MCFL, stderr, "In net.recv(). Calling backend.recv()\n");
    return MC_Network::back_end->recv();
  }
}


int MC_NetworkImpl::send(MC_Packet *packet)
{
  if(MC_Network::network){
    return MC_Network::network->front_end->send(packet);
  }
  else{
    return MC_Network::back_end->send(packet);
  }
}

