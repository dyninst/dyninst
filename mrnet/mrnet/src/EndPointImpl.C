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

} // namespace MRN
