/*===========================================================*/
/*             MC_Network Class DEFINITIONS                  */
/*===========================================================*/
#include <stdio.h>

#include "mrnet/src/MC_NetworkImpl.h"
#include "mrnet/src/MC_CommunicatorImpl.h"

extern MC_NetworkGraph * parsed_graph;

MC_NetworkImpl::MC_NetworkImpl(const char * _filename, const char * _application)
  :filename(_filename), application(_application)
{
  if( parse_configfile() == -1){
    return;
  }
  graph = parsed_graph;  
  if ( graph->has_cycle() ){
    //Network has a cycle
    mc_errno = MC_ENETWORK_CYCLE;
    _fail=true;
    return;
  }

  if ( !graph->fully_connected() ){
   //All nodes not reachable from root
    mc_errno = MC_ENETWORK_NOTCONNECTED;
    _fail=true;
    return;
  }

  MC_NetworkImpl::endpoints = graph->get_EndPoints();
  MC_CommunicatorImpl::create_BroadcastCommunicator(MC_NetworkImpl::endpoints);

  MC_NetworkImpl::front_end = new MC_FrontEndNode;

  graph->get_SerialGraph().print();
  if( MC_NetworkImpl::front_end->send_newSubTree( graph->get_SerialGraph() )
      == -1){
    mc_errno = MC_ENETWORK_FAILURE;
    _fail=true;
    return;
  }

  string app(application);
  vector <string> args;

  if( MC_NetworkImpl::front_end->send_newApplication(app, args) == -1){
    mc_errno = MC_ENETWORK_FAILURE;
    _fail=true;
    return;
  }

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
    return MC_Network::network->front_end->recv();
  }
  else{
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

