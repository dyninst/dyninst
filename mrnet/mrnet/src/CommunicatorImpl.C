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


CommunicatorImpl::CommunicatorImpl(void)
{
}

CommunicatorImpl::CommunicatorImpl( const std::vector<EndPoint*>& eps )
  : endpoints( eps )
{
}

CommunicatorImpl::CommunicatorImpl(Communicator &comm)
{
    endpoints = ((CommunicatorImpl)comm).endpoints;
}

CommunicatorImpl::~CommunicatorImpl(void)
{
}

int CommunicatorImpl::add_EndPoint(const char * _hostname,
                                   unsigned short _port)
{
    EndPoint * new_endpoint = Network::network->get_EndPoint(_hostname, _port);

    if(new_endpoint == NULL){
        return -1;
    }

    endpoints.push_back(new_endpoint);
    return 0;
}

} // namespace MRN
