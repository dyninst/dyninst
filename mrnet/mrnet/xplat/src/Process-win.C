/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: Process-win.C,v 1.4 2005/03/24 04:59:24 darnold Exp $
#include <windows.h>
#include <winsock2.h>
#include <string.h>
#include "xplat/Process.h"



namespace XPlat
{

int
Process::CreateLocal( const std::string& cmd, 
                        const std::vector<std::string>& args )
{
    int ret = -1;


    // build the command line
    std::string cmdline;
    for( std::vector<std::string>::const_iterator iter = args.begin();
            iter != args.end();
            iter++ )
    {
        cmdline += (*iter);
        cmdline += ' ';
    }
    char* mutableCmdline = new char[cmdline.length() + 1];
    strncpy( mutableCmdline, cmdline.c_str(), cmdline.length() );

    // spawn the process
    STARTUPINFO startupInfo;
    PROCESS_INFORMATION processInfo;

    ZeroMemory( &startupInfo, sizeof(startupInfo) );
    ZeroMemory( &processInfo, sizeof(processInfo) );

    startupInfo.cb = sizeof(startupInfo);

    BOOL bRet = ::CreateProcess( 
                cmd.c_str(),    // module to execute
                mutableCmdline, // command line for new process
                NULL,           // process security attributes (default)
                NULL,           // thread security attributes (default)
                TRUE,           // allow inheritable handles to be inherited
                NORMAL_PRIORITY_CLASS,  // creation flags (nothing special)
                NULL,           // environment (inherit from our process)
                NULL,           // working directory (inherit from our process)
                &startupInfo,
                &processInfo );
    if( bRet )
    {
        // process was created successfully
        ret = 0;
    }

    // cleanup
    delete[] mutableCmdline;
    CloseHandle( processInfo.hProcess );
    CloseHandle( processInfo.hThread );

    return ret;
}

int
Process::GetLastError( void )
{
    return ::GetLastError();
}

int
Process::GetProcessId( void )
{
    return (int)GetCurrentProcessId();
}

} // namespace XPlat

