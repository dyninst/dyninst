/***********************************************************************
 * Copyright © 2003 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: Mutex-win.C,v 1.1 2003/11/14 19:27:03 pcroth Exp $
#include "xplat/src/Mutex-win.h"

namespace XPlat
{

Mutex::Mutex( void )
  : data( new WinMutexData )
{
    // nothing else to do
}

} // namespace XPlat
