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

void MC_CommunicatorImpl::create_BroadcastCommunicator(vector <MC_EndPoint *> * endpoints)
{
  unsigned int i;
  comm_Broadcast = new MC_CommunicatorImpl();

  mc_printf((stderr, "In create_broadcastcomm()\n"));

  for(i=0; i<endpoints->size(); i++){
    mc_printf((stderr, "adding endpoint %s:%d[%d]\n",
              (*endpoints)[i]->get_HostName(),
              (*endpoints)[i]->get_Port(), (*endpoints)[i]->get_Id()));
    comm_Broadcast->add_EndPoint( (*endpoints)[i] );
  }
  return;  
}

MC_CommunicatorImpl::MC_CommunicatorImpl(void)
{}

MC_CommunicatorImpl::~MC_CommunicatorImpl(void)
{}

MC_CommunicatorImpl::MC_CommunicatorImpl(MC_Communicator &)
{
}
int MC_CommunicatorImpl::add_EndPoint(const char * _hostname, unsigned short _port)
{
  MC_EndPoint * new_endpoint = MC_Network::network->get_EndPoint(_hostname, _port);

  if(new_endpoint == NULL){
    return -1;
  }

  endpoints.push_back(new_endpoint);
  return 0;
}

int MC_CommunicatorImpl::add_EndPoint(MC_EndPoint * new_endpoint)
{
  if(new_endpoint){
    endpoints.push_back(new_endpoint);
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
  return &endpoints;
}
