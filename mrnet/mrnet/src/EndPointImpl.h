#if !defined(endpointimpl_h)
#define endpointimpl_h 1

#include <string>
#include "mrnet/h/MRNet.h"

namespace MRN
{

class NetworkNode;
class EndPointImpl: public EndPoint{
 private:
    unsigned int id;
    std::string hostname;
    unsigned short port;

 public:
    EndPointImpl(int _id, const char * _hostname, unsigned short _port);
    virtual ~EndPointImpl();
    virtual const char * get_HostName();
    virtual unsigned short get_Port();
    virtual unsigned int get_Id();
    virtual bool compare(const char * _hostname, unsigned short _port);
};

inline const char * EndPointImpl::get_HostName()
{
    return hostname.c_str();
}

inline unsigned short EndPointImpl::get_Port()
{
    return port;
}

inline unsigned int EndPointImpl::get_Id()
{
    return id;
}

inline bool EndPointImpl::compare(const char * _hostnamestr,
                                  unsigned short _port)
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
