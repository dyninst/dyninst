/***********************************************************************
 * Copyright © 2003 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#if !defined(__communicator_h)
#define __communicator_h 1

#include <vector>

#include "mrnet/h/MRNet.h"

namespace MRN
{

class RemoteNode;
class CommunicatorImpl: public Communicator{
    friend class Stream;
    friend class Network;

 private:
    static CommunicatorImpl * comm_Broadcast;
    std::vector <RemoteNode *> downstream_nodes; 
    std::vector <EndPoint *> endpoints;  //BackEnds addressed by communicator

    // used to construct broadcast communicator
    CommunicatorImpl( const std::vector<EndPoint*>& eps );

 public:
    CommunicatorImpl( void );
    CommunicatorImpl(Communicator &);
    virtual ~CommunicatorImpl( void );

    static CommunicatorImpl * get_BroadcastCommunicator(void);
    static void create_BroadcastCommunicator(std::vector <EndPoint *> &);
    const std::vector <EndPoint *> & get_EndPoints() const;

    virtual int add_EndPoint(const char * hostname, unsigned short port);
    virtual void add_EndPoint(EndPoint *);

    virtual unsigned int size() const;
    virtual const char * get_HostName(int) const; 
    virtual unsigned short get_Port(int) const;
    virtual unsigned int get_Id(int) const;
};

inline CommunicatorImpl * CommunicatorImpl::get_BroadcastCommunicator(void)
{
    return comm_Broadcast;
}

inline void
CommunicatorImpl::create_BroadcastCommunicator( std::vector <EndPoint *>
                                                & _endpoints)
{
    comm_Broadcast = new CommunicatorImpl( _endpoints );
}


inline void CommunicatorImpl::add_EndPoint(EndPoint * new_endpoint)
{
    endpoints.push_back(new_endpoint);
}

inline unsigned int CommunicatorImpl::size(void) const
{
    return endpoints.size();
}

inline const char * CommunicatorImpl::get_HostName(int id) const
{
    return endpoints[id]->get_HostName();
}

inline unsigned short CommunicatorImpl::get_Port(int id) const
{
    return endpoints[id]->get_Port();
}

inline unsigned int CommunicatorImpl::get_Id(int id) const
{
    return endpoints[id]->get_Id();
}

inline const std::vector <EndPoint *> & CommunicatorImpl::get_EndPoints() const
{
  return endpoints;
}

} // namespace MRN

#endif /* __communicator_h */
