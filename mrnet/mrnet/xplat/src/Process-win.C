/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: Process-win.C,v 1.6 2006/04/13 23:05:28 legendre Exp $
#include <winsock2.h>
#include <string.h>
#include "xplat/Process.h"



namespace XPlat
{

void printSysError(unsigned errNo) {
    char buf[1000];
    bool result = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, errNo, 
		  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		  buf, 1000, NULL);
    if (!result) {
        fprintf(stderr, "Couldn't print error message\n");
        printSysError(GetLastError());
    }
    fprintf(stderr, "System error [%d]: %s\n", errNo, buf);
    fflush(stderr);
}

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
    mutableCmdline[cmdline.length()] = '\0';

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
    else {
        printSysError(GetLastError());
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

