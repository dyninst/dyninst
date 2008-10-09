/****************************************************************************
 * Copyright © 2003-2008 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#if !defined(__communicator_h)
#define __communicator_h 1

#include <set>
#include <string>

#include "mrnet/Types.h"
#include "xplat/Mutex.h"

namespace MRN
{

class Network;
class CommunicationNode;

class Communicator{
    friend class Stream;
    friend class Network;

 public:
    bool add_EndPoint(Rank _rank);
    bool add_EndPoint(CommunicationNode *);
    const std::set <CommunicationNode *>& get_EndPoints() const;

 private:
    Network * _network;
    std::set <CommunicationNode *> _back_ends;
    XPlat::Mutex _mutex;

    Communicator( Network * );
    Communicator( Network *, Communicator &);
    Communicator( Network *, const std::set<CommunicationNode*>& eps );
};


} // namespace MRN

#endif /* __communicator_h */

