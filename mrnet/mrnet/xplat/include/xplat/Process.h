/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: Process.h,v 1.4 2004/06/01 18:23:49 pcroth Exp $
#ifndef XPLAT_PROCESS_H
#define XPLAT_PROCESS_H

#include <string>
#include <vector>
#include "xplat/NetUtils.h"


namespace XPlat
{

class Process
{
private:
    static int CreateLocal( const std::string& cmd,
                                const std::vector<std::string>& args );
    static int CreateRemote( const std::string& host,
                                const std::string& cmd,
                                const std::vector<std::string>& args );

public:
    static int GetProcessId( void );

    // spawn a new process
    // expects args[0] *not* to be cmd
    static int Create( const std::string& host,
                                const std::string& cmd,
                                const std::vector<std::string>& args )
    {
        int ret = -1;
        if( NetUtils::IsLocalHost( host ) )
        {
            ret = CreateLocal( cmd, args );
        }
        else
        {
            ret = CreateRemote( host, cmd, args );
        }
        return ret;
    }

    static int GetLastError( void );
};

} // namespace XPlat

#endif // XPLAT_PROCESS_H
