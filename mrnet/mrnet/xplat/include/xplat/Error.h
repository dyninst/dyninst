/***********************************************************************
 * Copyright © 2003 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: Error.h,v 1.1 2003/11/14 19:36:04 pcroth Exp $
#ifndef XPLAT_ERROR_H
#define XPLAT_ERROR_H

#include <string>

namespace XPlat
{

namespace Error
{

std::string GetErrorString( int code );
bool ETimedOut( int code );
bool EAddrInUse( int code );

} // namespace Error

} // namespace XPlat

#endif // XPLAT_ERROR_H
