/****************************************************************************
 * Copyright © 2003-2008 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#include <stdio.h>

#include "mrnet/MRNet.h"
#include "CommunicationNode.h"
#include "utils.h"

namespace MRN
{

Communicator::Communicator( Network * inetwork )
    : _network(inetwork)
{
}

Communicator::Communicator( Network * inetwork,
                            const std::set<CommunicationNode*>& iback_ends )
    : _network(inetwork), _back_ends( iback_ends )
{
}

Communicator::Communicator( Network * inetwork, Communicator &icomm)
    : _network(inetwork)
{
    _back_ends = icomm.get_EndPoints();
}

bool Communicator::add_EndPoint(Rank irank)
{
    CommunicationNode * new_endpoint = _network->get_EndPoint(irank);

    if( new_endpoint == NULL ){
        return false;
    }

    _mutex.Lock();
    _back_ends.insert( new_endpoint );
    _mutex.Unlock();

    return true;
}

bool Communicator::add_EndPoint(CommunicationNode * iendpoint)
{
    if( iendpoint == NULL )
        return false;

    _mutex.Lock();
    _back_ends.insert( iendpoint );
    _mutex.Unlock();

    return true;
}

const std::set< CommunicationNode * >& Communicator::get_EndPoints() const
{
    return _back_ends;
}

} // namespace MRN
