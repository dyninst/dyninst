/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

// $Id: Mutex-win.C,v 1.4 2007/01/24 19:34:06 darnold Exp $
#include "Mutex-win.h"

namespace XPlat
{

Mutex::Mutex( void )
  : data( new WinMutexData )
{
    // nothing else to do
}

} // namespace XPlat
