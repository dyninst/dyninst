/*===========================================================*/
/*      MC_CommunicatorImpl CLASS METHOD DEFINITIONS             */
/*===========================================================*/

#include <stdio.h>
#include "mrnet/src/CommunicatorImpl.h"
#include "mrnet/src/NetworkImpl.h"
#include "mrnet/src/utils.h"

MC_CommunicatorImpl * MC_CommunicatorImpl::comm_Broadcast=NULL;

MC_CommunicatorImpl * MC_CommunicatorImpl::get_BroadcastCommunicator(void)
{
  return comm_Broadcast;
}

void MC_CommunicatorImpl::create_BroadcastCommunicator(std::vector <MC_EndPoint *> * _endpoints)
{
  //unsigned int i;
  comm_Broadcast = new MC_CommunicatorImpl( *_endpoints );

  //mc_printf(MCFL, stderr, "In create_BroadCastComm(). comm_bc: [ ");
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
  : endpoints( new std::vector<MC_EndPoint*> )
{}

MC_CommunicatorImpl::MC_CommunicatorImpl( const std::vector<MC_EndPoint*>& eps )
  : endpoints( new std::vector<MC_EndPoint*> )
{
    *endpoints = eps;
}

MC_CommunicatorImpl::MC_CommunicatorImpl(MC_Communicator &comm)
  : endpoints( new std::vector<MC_EndPoint*> )
{
  *endpoints = *(static_cast<MC_CommunicatorImpl&>(comm).endpoints);
}

MC_CommunicatorImpl::~MC_CommunicatorImpl(void)
{
    delete endpoints;
}

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

unsigned int MC_CommunicatorImpl::size(void) const
{
    assert( endpoints != NULL );
    return endpoints->size();
}

const char * MC_CommunicatorImpl::get_HostName(int id) const
{
    const char* ret = NULL;

    assert( endpoints != NULL );
    if( (unsigned int)id < endpoints->size() )
    {
        ret = (*endpoints)[id]->get_HostName();
    }
    return ret;
}

unsigned short MC_CommunicatorImpl::get_Port(int id) const
{
    //Needs better error code
    unsigned short ret = 0;

    if( (unsigned int)id < endpoints->size() )
    {
        ret = (*endpoints)[id]->get_Port();
    }
    return ret;
}

unsigned int MC_CommunicatorImpl::get_Id(int id) const
{
    //Needs better error code
    unsigned int ret = 0;

    if( (unsigned int)id < endpoints->size() )
    {
        ret = (*endpoints)[id]->get_Id();
    }
    return ret;
}

const std::vector <MC_EndPoint *> * MC_CommunicatorImpl::get_EndPoints() const
{
  return endpoints;
}
