/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: Once-win.C,v 1.3 2005/03/21 18:58:31 darnold Exp $
#include "Once-win.h"

namespace XPlat
{

Once::Once( void )
  : data( new WinOnceData )
{
    // nothing else to do
}


int
WinOnceData::DoOnce( void (*func)( void ) )
{
    LONG incval = InterlockedIncrement( &latch );
    if( incval == 1 )
    {
        // We were first to increment the latch, so we should execute the func
        func();

        // Indicate we're now done with the initialization func
        done = true;
    }
    else
    {
        // We were not the first to increment the latch,
        // so we wait till the initialization is done.
        // We spin instead of blocking because we assume the
        // initialization will not take long.
        while( !done )
        {
            // spin!
        }
    }
    return 0;
}

} // namespace XPlat

