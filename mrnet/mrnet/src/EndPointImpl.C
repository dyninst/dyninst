/*==========================================================*/
/*      EndPointImpl CLASS METHOD DEFINITIONS            */
/*==========================================================*/

#include "mrnet/src/EndPointImpl.h"

namespace MRN
{

EndPointImpl::EndPointImpl(int _id, const char * _hostname,
                                 unsigned short _port)
  :id(_id), hostname(_hostname), port(_port)
{
}

EndPointImpl::~EndPointImpl()
{
}

const char * EndPointImpl::get_HostName()
{
  return hostname.c_str();
}

unsigned short EndPointImpl::get_Port()
{
  return port;
}

unsigned int EndPointImpl::get_Id()
{
  return id;
}

bool EndPointImpl::compare(const char * _hostnamestr, unsigned short _port)
{
  std::string _hostname(_hostnamestr);

  if( (_port == port) &&
      (_hostname == hostname) ) {
    return true;
  }
  else{
    return false;
  }
}

} // namespace MRN
