/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#if !defined(endpointimpl_h)
#define endpointimpl_h 1

#include <string>
#include "mrnet/MRNet.h"

namespace MRN
{

class NetworkNode;
class EndPointImpl{
 private:
    Rank rank;
    std::string hostname;
    Port port;

 public:
    EndPointImpl(Rank _rank, const char * _hostname, Port _port);
    ~EndPointImpl();
    const char * get_HostName()const;
    Port get_Port()const;
    Rank get_Rank()const;
    bool compare(const char * _hostname, Port _port)const;
};

inline const char * EndPointImpl::get_HostName() const
{
    return hostname.c_str();
}

inline Port EndPointImpl::get_Port() const
{
    return port;
}

inline Rank EndPointImpl::get_Rank() const
{
    return rank;
}

inline bool EndPointImpl::compare(const char * _hostnamestr, Port _port) const
{
    if( ( port == _port ) &&
        ( hostname == _hostnamestr ) ){
        return true;
    }
    else{
        return false;
    }
}

} // namespace MRN

#endif /* endpoint_h 1 */
