/***********************************************************************
 * Copyright © 2003 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

/*===========================================================*/
/*      CommunicatorImpl CLASS METHOD DEFINITIONS             */
/*===========================================================*/

#include <stdio.h>
#include "mrnet/src/CommunicatorImpl.h"
#include "mrnet/src/NetworkImpl.h"
#include "mrnet/src/utils.h"

namespace MRN
{

CommunicatorImpl::CommunicatorImpl( Network * _network )
    : network(_network)
{
}

CommunicatorImpl::CommunicatorImpl( Network * _network,
                                    const std::vector<EndPoint*>& eps )
  : network(_network), endpoints( eps )
{
}

CommunicatorImpl::CommunicatorImpl( Network * _network, Communicator &comm)
    :network(_network)
{
    endpoints = comm.get_EndPoints();
}

CommunicatorImpl::~CommunicatorImpl(void)
{
}

int CommunicatorImpl::add_EndPoint(const char * _hostname,
                                   unsigned short _port)
{
    EndPoint * new_endpoint = network->get_EndPoint(_hostname, _port);

    if(new_endpoint == NULL){
        return -1;
    }

    endpoints.push_back(new_endpoint);
    return 0;
}

} // namespace MRN
