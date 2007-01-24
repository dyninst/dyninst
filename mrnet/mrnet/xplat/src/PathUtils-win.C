/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

// $Id: PathUtils-win.C,v 1.3 2007/01/24 19:34:18 darnold Exp $
#include <windows.h>
#include <shlwapi.h>
#include "xplat/PathUtils.h"


namespace XPlat
{

namespace PathUtils
{

std::string
GetFilename( const std::string& path )
{
    return PathFindFileName( path.c_str() );
}

std::string
GetDirname( const std::string& path )
{
    char* pathCopy = new char[path.length() + 1];
    strcpy( pathCopy, path.c_str() );

    std::string ret;
    if( PathRemoveFileSpec( pathCopy ) )
    {
        ret = pathCopy;
    }

    return ret;
}

} // namespace PathUtils

} // namespace XPlat

