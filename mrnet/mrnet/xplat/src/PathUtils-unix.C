/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: PathUtils-unix.C,v 1.2 2004/03/23 01:12:23 eli Exp $
#include <libgen.h>
#include "xplat/PathUtils.h"

namespace XPlat
{

namespace PathUtils
{

std::string
GetFilename( const std::string& path )
{
    // basename modifies the path
    char* pathCopy = new char[path.length() + 1];
    strcpy( pathCopy, path.c_str() );

    std::string ret = basename( pathCopy );

    delete[] pathCopy;

    return ret;
}

std::string
GetDirname( const std::string& path )
{
    // dirname modifies the path
    char* pathCopy = new char[path.length() + 1];
    strcpy( pathCopy, path.c_str() );

    std::string ret = dirname( pathCopy );

    delete[] pathCopy;

    return ret;
}

} // namespace PathUtils

} // namespace XPlat

