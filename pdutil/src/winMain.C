/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <conio.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include "pdutil/h/winMain.h"


// variables used only by functions in this file
static bool waitForKeypressOnExit = true;

// declare some global objects that restore the standard iostreams' 
// streambufs at process exit.  We need this because we redirect them 
// in InitConsole when we set up our console.
//
// We declare the class in this file instead of a header because we
// don't see that this class has any more general usefulness.
class StdStreamBufRestorer
{
private:
    std::ios& str;
    std::streambuf* origBuf;
    
public:
    StdStreamBufRestorer( std::ios& s )
        : str( s ),
          origBuf( s.rdbuf() )
    {
        // nothing else to do
    }
    ~StdStreamBufRestorer( void )
    {
        // restore the original buffer
        str.rdbuf( origBuf );
    }
};

StdStreamBufRestorer cinCleanup( std::cin );
StdStreamBufRestorer coutCleanup( std::cout );
StdStreamBufRestorer cerrCleanup( std::cerr );



// prototypes of functions used in this file
static void WaitForKeypress( void );
static void CleanupSockets( void );
static void HandleInitConsoleError( void );



void
InitConsole( void )
{

    HANDLE stdin_handle;
    HANDLE stdout_handle;
    HANDLE stderr_handle;
    HANDLE hOldStdin;
    HANDLE hOldStdout;
    HANDLE hOldStderr;
    int stdin_fileno; 
    int stdout_fileno;
    int stderr_fileno;


    // ensure that we have a console
    FreeConsole();
    if( !AllocConsole() )
    {
        // indicate that console output is not available
        HandleInitConsoleError();
    }

    // set up standard intput and output to the new console,
    // so that they can be inherited
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;
    
    stdin_handle = CreateFile( "CONIN$",
                                GENERIC_READ,
                                FILE_SHARE_READ,
                                &sa,
                                OPEN_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL );
    stdout_handle = CreateFile( "CONOUT$",
                                GENERIC_WRITE,
                                FILE_SHARE_WRITE,
                                &sa,
                                OPEN_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL );
    stderr_handle = CreateFile( "CONOUT$",
                                GENERIC_WRITE,
                                FILE_SHARE_WRITE,
                                &sa,
                                OPEN_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL );
    if( (stdin_handle == INVALID_HANDLE_VALUE) ||
        (stdout_handle == INVALID_HANDLE_VALUE) ||
        (stderr_handle == INVALID_HANDLE_VALUE) )
    {
        // indicate that console output is not available
        HandleInitConsoleError();
    }
    // set our standard handles to these new handles,
    // closing our old handles in the process
    hOldStdin = GetStdHandle( STD_INPUT_HANDLE );
    if( hOldStdin != stdin_handle )
    {
        SetStdHandle( STD_INPUT_HANDLE, stdin_handle );
        CloseHandle( hOldStdin );
    }

    hOldStdout = GetStdHandle( STD_OUTPUT_HANDLE );
    if( hOldStdout != stdout_handle )
    {
        SetStdHandle( STD_OUTPUT_HANDLE, stdout_handle );
        CloseHandle( hOldStdout );
    }

    hOldStderr = GetStdHandle( STD_ERROR_HANDLE );
    if( hOldStderr != stderr_handle )
    {
        SetStdHandle( STD_ERROR_HANDLE, stderr_handle );
        CloseHandle( hOldStderr );
    }

    // associate stdio filenos with the new handles
    stdin_fileno = _open_osfhandle( (LONG)stdin_handle, _O_RDONLY );
    stdout_fileno = _open_osfhandle( (LONG)stdout_handle, _O_WRONLY );
    stderr_fileno = _open_osfhandle( (LONG)stderr_handle, _O_WRONLY );
    if( (stdin_fileno == -1) ||
        (stdout_fileno == -1) ||
        (stderr_fileno == -1) )
    {
        // indicate that console output is not available
        HandleInitConsoleError();
    }

    // associate the standard C streams with our handles
    {
        FILE* stdin_file = _fdopen( stdin_fileno, "r" );
        FILE* stdout_file = _fdopen( stdout_fileno, "w" );
        FILE* stderr_file = _fdopen( stderr_fileno, "w" );
        *stdin = *stdin_file;
        *stdout = *stdout_file;
        *stderr = *stderr_file;
    }

    // associate the standard C++ streams with the new console
    //
    // first create ifstream/ofstreams associated with the new console
    // we create ifstreams/ofstreams on the heap so they don't go away
    // once we return from this function
    // we *do* leak this memory, but it is done once and these objects 
    // need to live the lifetime of the process
    std::ifstream* cinstr = new std::ifstream( "CONIN$" );
    std::ofstream* coutstr = new std::ofstream( "CONOUT$" );
    std::ofstream* cerrstr = new std::ofstream( "CONOUT$" );
    
    // next, associate our new ifstream/ofstreams with cin, cout, and cerr
    // note that we have a separate class (StdStreamBufRestorer) which will
    // reset their streambufs when the process exits, so that the cin/cout/cerr
    // the new streambufs are not destroyed twice
    std::cin.rdbuf( cinstr->rdbuf() );
    std::cout.rdbuf( coutstr->rdbuf() );
    std::cerr.rdbuf( cerrstr->rdbuf() );

    
   // we've got a console
   //
   // ensure that we leave it around long enough for user to
   // see any error messages we dump out to the console
   atexit( WaitForKeypress );
}


static
void
WaitForKeypress( void )
{
    if( waitForKeypressOnExit )
    {
        fprintf( stdout, "Press any key to exit..." );
        fflush( stdout );
        while( !_kbhit() )
        {
            Sleep( 500 );
        }
    }
}


static
void
HandleInitConsoleError( void )
{
    std::ostringstream estr;

    estr << "Paradyn was unable to create a console."
        << " [" << GetLastError() << "]"
        << std::ends;

    MessageBox( NULL, estr.str().c_str(), _T("Paradyn"), MB_ICONSTOP | MB_OK );
    ExitProcess( 1 );
}



void
InitSockets( const char* progname )
{
    WSADATA data;
    bool ok = false;
    

    // request WinSock 2.0
    if( WSAStartup( MAKEWORD(2,0), &data ) == 0 )
    {
        // we've seen a successful WSAStartup call, so set things
        // so that WSACleanup is called on process exit
        atexit( CleanupSockets );

        // verify that the version that was provided is one we can use
        if( (LOBYTE(data.wVersion) == 2) && (HIBYTE(data.wVersion) == 0) )
        {
            ok = true;
        }
    }

    if( !ok )
    {
        MessageBox( NULL,
                    _T("Unable to access a compatible WinSock library.  The program cannot continue."),
                    progname,
                    MB_ICONSTOP | MB_OK );

        ExitProcess( 1 );
    }
}


static
void
CleanupSockets( void )
{
    WSACleanup();
}   


void
ClearWaitForConsoleKeypress( void )
{
    waitForKeypressOnExit = false;
}

