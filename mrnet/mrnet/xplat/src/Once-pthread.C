/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: Once-pthread.C,v 1.3 2005/03/21 18:58:30 darnold Exp $
#include "Once-pthread.h"

namespace XPlat
{

Once::Once( void )
  : data( new PthreadOnceData )
{
    // nothing else to do
}


int
PthreadOnceData::DoOnce( void (*func)( void ) )
{
    return pthread_once( &latch, func );
}

} // namespace XPlat

