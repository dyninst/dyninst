/***********************************************************************
 * Copyright © 2003 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: Once-pthread.C,v 1.1 2003/11/14 19:27:03 pcroth Exp $
#include "xplat/src/Once-pthread.h"

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

