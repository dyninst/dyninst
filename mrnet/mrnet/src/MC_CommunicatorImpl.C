/*===========================================================*/
/*      MC_CommunicatorImpl CLASS METHOD DEFINITIONS             */
/*===========================================================*/

#include <stdio.h>
#include "mrnet/src/MC_CommunicatorImpl.h"
#include "mrnet/src/MC_NetworkImpl.h"
#include "mrnet/src/utils.h"

MC_CommunicatorImpl * MC_CommunicatorImpl::comm_Broadcast=NULL;

MC_CommunicatorImpl * MC_CommunicatorImpl::get_BroadcastCommunicator(void)
{
  return comm_Broadcast;
}

void MC_CommunicatorImpl::create_BroadcastCommunicator(vector <MC_EndPoint *> * _endpoints)
{
  //unsigned int i;
  comm_Broadcast = new MC_CommunicatorImpl();

  //mc_printf(MCFL, stderr, "In create_BroadCastComm(). comm_bc: [ ");
  comm_Broadcast->endpoints = _endpoints;
  //for(i=0; i<comm_Broadcast->endpoints->size(); i++){
    //_fprintf((stderr, "%s:%d:%d, ",
              //(*comm_Broadcast->endpoints)[i]->get_HostName(),
              //(*comm_Broadcast->endpoints)[i]->get_Port(),
              //(*comm_Broadcast->endpoints)[i]->get_Id()));
  //}
  //_fprintf((stderr, "]\n"));

  //mc_printf(MCFL, stderr, "comm_bc size: %d\n",
             //comm_Broadcast->get_EndPoints()->size());
  return;  
}

MC_CommunicatorImpl::MC_CommunicatorImpl(void)
{}

MC_CommunicatorImpl::MC_CommunicatorImpl(MC_Communicator &comm)
{
  endpoints = new std::vector <MC_EndPoint *>;
  *endpoints = *(static_cast<MC_CommunicatorImpl&>(comm).endpoints);
}

MC_CommunicatorImpl::~MC_CommunicatorImpl(void)
{}

int MC_CommunicatorImpl::add_EndPoint(const char * _hostname, unsigned short _port)
{
  MC_EndPoint * new_endpoint = MC_Network::network->get_EndPoint(_hostname, _port);

  if(new_endpoint == NULL){
    return -1;
  }

  endpoints->push_back(new_endpoint);
  return 0;
}

int MC_CommunicatorImpl::add_EndPoint(MC_EndPoint * new_endpoint)
{
  if(new_endpoint){
    endpoints->push_back(new_endpoint);
    return 0;
  }
  else{
    return -1;
  }
}

int MC_CommunicatorImpl::size(void){
  return MC_Network::network->endpoints->size();
}

const char * MC_CommunicatorImpl::get_HostName(int id)
{
  if( (unsigned int)id >= MC_Network::network->endpoints->size() )
    return NULL;

  return (*MC_Network::network->endpoints)[id]->get_HostName();
}

unsigned short MC_CommunicatorImpl::get_Port(int id)
{
  if( (unsigned int)id >= MC_Network::network->endpoints->size() )
    return 0;  //Needs better error code

  return (*MC_Network::network->endpoints)[id]->get_Port();
}

unsigned int MC_CommunicatorImpl::get_Id(int id)
{
  if( (unsigned int)id >= MC_Network::network->endpoints->size() )
    return 0;  //Needs better error code

  return (*MC_Network::network->endpoints)[id]->get_Id();
}

vector <MC_EndPoint *> * MC_CommunicatorImpl::get_EndPoints()
{
  return endpoints;
}
