/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: PathUtils.h,v 1.2 2004/03/23 01:12:22 eli Exp $
#ifndef XPLAT_PATH_UTILS_H
#define XPLAT_PATH_UTILS_H

#include <string>

namespace XPlat
{

namespace PathUtils
{

std::string GetFilename( const std::string& path );
std::string GetDirname( const std::string& path );

} // namespace PathUtils

} // namespace XPlat

#endif // XPLAT_PATH_UTILS_H
