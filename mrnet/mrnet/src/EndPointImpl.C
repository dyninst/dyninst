/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

/*==========================================================*/
/*      EndPointImpl CLASS METHOD DEFINITIONS            */
/*==========================================================*/

#include "mrnet/src/EndPointImpl.h"

namespace MRN
{

EndPointImpl::EndPointImpl(Rank _rank, const char * _hostname, Port _port)
  :rank(_rank), hostname(_hostname), port(_port)
{
}

EndPointImpl::~EndPointImpl()
{
}

} // namespace MRN
