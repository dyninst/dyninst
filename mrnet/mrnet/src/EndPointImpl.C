/*==========================================================*/
/*      MC_EndPointImpl CLASS METHOD DEFINITIONS            */
/*==========================================================*/
#include "mrnet/src/EndPointImpl.h"

MC_EndPointImpl::MC_EndPointImpl(int _id, const char * _hostname,
                                 unsigned short _port)
  :id(_id), hostname(_hostname), port(_port)
{
}

MC_EndPointImpl::~MC_EndPointImpl()
{
}

const char * MC_EndPointImpl::get_HostName()
{
  return hostname.c_str();
}

unsigned short MC_EndPointImpl::get_Port()
{
  return port;
}

unsigned int MC_EndPointImpl::get_Id()
{
  return id;
}

bool MC_EndPointImpl::compare(const char * _hostnamestr, unsigned short _port)
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
