/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: Mutex-win.C,v 1.3 2005/03/21 18:58:29 darnold Exp $
#include "Mutex-win.h"

namespace XPlat
{

Mutex::Mutex( void )
  : data( new WinMutexData )
{
    // nothing else to do
}

} // namespace XPlat
