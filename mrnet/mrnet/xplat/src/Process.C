/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: Process.C,v 1.3 2004/07/19 18:42:15 pcroth Exp $
#include <assert.h>
#include "xplat/Process.h"


namespace XPlat
{

int
Process::CreateRemote( const std::string& host,
                    const std::string& cmd,
                    const std::vector<std::string>& args )
{
    assert( host.length() > 0 );
    assert( !NetUtils::IsLocalHost( host ) );

    // determine the remote shell program to use
    std::string rshCmd = "rsh";
    const char* varval = getenv( "XPLAT_RSHCOMMAND" );
    if( varval != NULL )
    {
        rshCmd = varval;
    }

    // determine whether the remote command must be run by some other
    // remote utility command (e.g., so that it has AFS tokens)
    std::string remCmd = "";
    varval = getenv( "XPLAT_REMCOMMAND" );
    if( varval != NULL )
    {
        remCmd = varval;
    }

    // build the arguments for the remote process
    std::vector<std::string> rshArgs;
    rshArgs.push_back( rshCmd );
    rshArgs.push_back( host );
    if( remCmd.length() > 0 )
    {
        rshArgs.push_back( remCmd );
    }
    for( std::vector<std::string>::const_iterator aiter = args.begin();
            aiter != args.end();
            aiter++ )
    {
        rshArgs.push_back( *aiter );
    }

    // execute the local command that will create the remote process
    return CreateLocal( rshCmd, rshArgs );
}

} // namespace XPlat

