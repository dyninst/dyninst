/****************************************************************************
 * Copyright © 2003-2005 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ***************************************************************************/

/*==========================================================*/
/*      EndPointImpl CLASS METHOD DEFINITIONS            */
/*==========================================================*/

#include "mrnet/src/EndPointImpl.h"

namespace MRN
{

EndPointImpl::EndPointImpl(Rank irank, const char * ihostname, Port iport)
  :rank(irank), hostname(ihostname), port(iport)
{
}

EndPointImpl::~EndPointImpl()
{
}

} // namespace MRN
