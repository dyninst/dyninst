/*===========================================================*/
/*      CommunicatorImpl CLASS METHOD DEFINITIONS             */
/*===========================================================*/

#include <stdio.h>
#include "mrnet/src/CommunicatorImpl.h"
#include "mrnet/src/NetworkImpl.h"
#include "mrnet/src/utils.h"

namespace MRN
{

CommunicatorImpl * CommunicatorImpl::comm_Broadcast=NULL;

CommunicatorImpl * CommunicatorImpl::get_BroadcastCommunicator(void)
{
  return comm_Broadcast;
}

void CommunicatorImpl::create_BroadcastCommunicator(std::vector <EndPoint *> * _endpoints)
{
  comm_Broadcast = new CommunicatorImpl( *_endpoints );

  return;  
}

CommunicatorImpl::CommunicatorImpl(void)
  : endpoints( new std::vector<EndPoint*> )
{}

CommunicatorImpl::CommunicatorImpl( const std::vector<EndPoint*>& eps )
  : endpoints( new std::vector<EndPoint*> )
{
    *endpoints = eps;
}

CommunicatorImpl::CommunicatorImpl(Communicator &comm)
  : endpoints( new std::vector<EndPoint*> )
{
  *endpoints = *(static_cast<CommunicatorImpl&>(comm).endpoints);
}

CommunicatorImpl::~CommunicatorImpl(void)
{
    delete endpoints;
}

int CommunicatorImpl::add_EndPoint(const char * _hostname, unsigned short _port)
{
  EndPoint * new_endpoint = Network::network->get_EndPoint(_hostname, _port);

  if(new_endpoint == NULL){
    return -1;
  }

  endpoints->push_back(new_endpoint);
  return 0;
}

int CommunicatorImpl::add_EndPoint(EndPoint * new_endpoint)
{
  if(new_endpoint){
    endpoints->push_back(new_endpoint);
    return 0;
  }
  else{
    return -1;
  }
}

unsigned int CommunicatorImpl::size(void) const
{
    assert( endpoints != NULL );
    return endpoints->size();
}

const char * CommunicatorImpl::get_HostName(int id) const
{
    const char* ret = NULL;

    assert( endpoints != NULL );
    if( (unsigned int)id < endpoints->size() )
    {
        ret = (*endpoints)[id]->get_HostName();
    }
    return ret;
}

unsigned short CommunicatorImpl::get_Port(int id) const
{
    //Needs better error code
    unsigned short ret = 0;

    if( (unsigned int)id < endpoints->size() )
    {
        ret = (*endpoints)[id]->get_Port();
    }
    return ret;
}

unsigned int CommunicatorImpl::get_Id(int id) const
{
    //Needs better error code
    unsigned int ret = 0;

    if( (unsigned int)id < endpoints->size() )
    {
        ret = (*endpoints)[id]->get_Id();
    }
    return ret;
}

const std::vector <EndPoint *> * CommunicatorImpl::get_EndPoints() const
{
  return endpoints;
}

} // namespace MRN
