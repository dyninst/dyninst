/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: Mutex-win.C,v 1.2 2004/03/23 01:12:23 eli Exp $
#include "xplat/src/Mutex-win.h"

namespace XPlat
{

Mutex::Mutex( void )
  : data( new WinMutexData )
{
    // nothing else to do
}

} // namespace XPlat
