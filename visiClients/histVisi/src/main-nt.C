/*
 * Copyright (c) 1996-1999 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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
//----------------------------------------------------------------------------
//
// main-nt.C
//
// main-nt.C - WinMain for runtime histogram visi on Windows.
// After some necessary initialization, this routine simply calls 
// the platform-shared main() routine.
//
//----------------------------------------------------------------------------
// $Id: main-nt.C,v 1.1 1999/10/05 22:09:05 pcroth Exp $
//----------------------------------------------------------------------------
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <iostream.h>
#include <fstream.h>
#include <io.h>
#include <fcntl.h>
#include <strstrea.h>



//----------------------------------------------------------------------------
// prototypes of functions used in this file
//----------------------------------------------------------------------------
extern    int        main( int, char** );
static    void    CleanupSockets( void );
static    void    InitSockets( const char* progname );



//----------------------------------------------------------------------------
// PDConsole
// Support class implementing nice handling of consoles under Windows.
//----------------------------------------------------------------------------
class PDConsole
{
private:
    ifstream*    pstdin_stream;
    ofstream*    pstdout_stream;
    ofstream*    pstderr_stream;

public:
    PDConsole( void );
    ~PDConsole( void );

    bool    Init( bool bTieStdInput = true,
                    bool bTieStdOutput = true,
                    bool bTieStdError = true );
};

PDConsole::PDConsole( void )
  : pstdin_stream( NULL ),
    pstdout_stream( NULL ),
    pstderr_stream( NULL )
{}



//
// PDConsole::Init - initialize a console window, with
// control over whether the process' standard input, output, and error
// handles are tied to the console
//
bool
PDConsole::Init( bool bTieStdin, bool bTieStdout, bool bTieStderr )
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
    bool ret = true;

    // ensure that we have a console
    FreeConsole();
    if( !AllocConsole() )
    {
        // indicate that console output is not available
        goto handle_init_console_error;
    }

    // set up standard input and output to the new console,
    // so that they can be inherited
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    if( bTieStdin )
    {
        stdin_handle = CreateFile( "CONIN$",
                                    GENERIC_READ,
                                    FILE_SHARE_READ,
                                    &sa,
                                    OPEN_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL );
        if( stdin_handle == INVALID_HANDLE_VALUE )
        {
            // indicate that console output is not available
            goto handle_init_console_error;
        }

        // set our standard handle to this new handle,
        // closing our old handle in the process
        hOldStdin = GetStdHandle( STD_INPUT_HANDLE );
        if( hOldStdin != stdin_handle )
        {
            SetStdHandle( STD_INPUT_HANDLE, stdin_handle );
            CloseHandle( hOldStdin );
        }

        // associate stdio fileno with the new handle
        stdin_fileno = _open_osfhandle( (LONG)stdin_handle, _O_RDONLY );
        if( stdin_fileno == -1 )
        {
            // indicate that console output is not available
            goto handle_init_console_error;
        }

        {
            // associate the standard C stream with our handle
            FILE* stdin_file = _fdopen( stdin_fileno, "r" );
            *stdin = *stdin_file;

            // associate the standard C++ streams with our handles
            // note that we allocate the objects on the heap so that
            // they are not destroyed when we exit the current block
            // (which would leave cin, cout, and cerr as junk)
            pstdin_stream = new ifstream( stdin_fileno );
            cin = *pstdin_stream;
        }
    }

    if( bTieStdout )
    {
        stdout_handle = CreateFile( "CONOUT$",
                                    GENERIC_WRITE,
                                    FILE_SHARE_WRITE,
                                    &sa,
                                    OPEN_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL );
        if( stdout_handle == INVALID_HANDLE_VALUE )
        {
            // indicate that console output is not available
            goto handle_init_console_error;
        }

        // set our standard handle to this new handle,
        // closing our old handle in the process
        hOldStdout = GetStdHandle( STD_OUTPUT_HANDLE );
        if( hOldStdout != stdout_handle )
        {
            SetStdHandle( STD_OUTPUT_HANDLE, stdout_handle );
            CloseHandle( hOldStdout );
        }

        // associate stdio fileno with the new handle
        stdout_fileno = _open_osfhandle( (LONG)stdout_handle, _O_WRONLY );
        if( stdout_fileno == -1 )
        {
            // indicate that console output is not available
            goto handle_init_console_error;
        }

        {
            // associate the standard C stream with our handle
            FILE* stdout_file = _fdopen( stdout_fileno, "w" );
            *stdout = *stdout_file;

            // associate the standard C++ streams with our handles
            // note that we allocate the objects on the heap so that
            // they are not destroyed when we exit the current block
            // (which would leave cin, cout, and cerr as junk)
            pstdout_stream = new ofstream( stdout_fileno );
            cout = *pstdout_stream;
        }
    }

    if( bTieStderr )
    {
        stderr_handle = CreateFile( "CONOUT$",
                                    GENERIC_WRITE,
                                    FILE_SHARE_WRITE,
                                    &sa,
                                    OPEN_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL );
        if( stderr_handle == INVALID_HANDLE_VALUE )
        {
            // indicate that console output is not available
            goto handle_init_console_error;
        }

        // set our standard handle to this new handle,
        // closing our old handle in the process
        hOldStderr = GetStdHandle( STD_ERROR_HANDLE );
        if( hOldStderr != stderr_handle )
        {
            SetStdHandle( STD_ERROR_HANDLE, stderr_handle );
            CloseHandle( hOldStderr );
        }
    
        // associate stdio fileno with the new handle
        stderr_fileno = _open_osfhandle( (LONG)stderr_handle, _O_WRONLY );
        if( stderr_fileno == -1 )
        {
            // indicate that console output is not available
            goto handle_init_console_error;
        }

        {
            // associate the standard C stream with our handle
            FILE* stderr_file = _fdopen( stderr_fileno, "w" );
            *stderr = *stderr_file;

            // associate the standard C++ streams with our handles
            // note that we allocate the objects on the heap so that
            // they are not destroyed when we exit the current block
            // (which would leave cin, cout, and cerr as junk)
            pstderr_stream = new ofstream( stderr_fileno );
            cerr = *pstderr_stream;
        }
    }

    return ret;

handle_init_console_error:
    {
        ostrstream estr;

        estr << "Paradyn was unable to create a console."
            << " [" << GetLastError() << "]"
            << ends;

        MessageBox( NULL, estr.str(), _T("Paradyn"), MB_ICONSTOP | MB_OK );
        ret = false;
    }

    return ret;
}


PDConsole::~PDConsole( void )
{
    delete pstdin_stream;
    delete pstdout_stream;
    delete pstderr_stream;
}





int
WINAPI
WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
#ifdef _DEBUG
    // pause for attaching a debugger if desired
    MessageBox( NULL, "Press OK to continue", "RTHist", MB_OK );

    // provide a console for the visi
    PDConsole cons;
    if( !cons.Init( false, true, true ) )
    {
        return -1;
    }
#endif // _DEBUG

    // initialize our use of the WinSock library
    InitSockets( __argv[0] );    

    // call the traditional main()
    return main( __argc, __argv );
}


//
// CleanupSockets - handles cleanup of WinSock library.
//
static
void
CleanupSockets( void )
{
    WSACleanup();
}



//
// InitSockets - handles initialization of WinSock library.
static
void
InitSockets( const char* progname )
{
    WSADATA    data;


    // request WinSock 2.0
    if( WSAStartup( MAKEWORD(2,0), &data ) != 0 )
    {
        // indicate lack of ability to obtain required WinSock library
        MessageBox( NULL,
            "Unable to access a WinSock 2.0 library.  Be sure the WinSock libraries are in your PATH.",
            progname,
            MB_ICONSTOP | MB_OK );
        ExitProcess( 1 );
    }

    // we've seen a successful WSAStartup call, so set things
    // so that WSACleanup is called on process exit
    atexit( CleanupSockets );

    // verify that the version that was provided is one we can use
    if( (LOBYTE(data.wVersion) != 2) || (HIBYTE(data.wVersion) != 0) )
    {
        // indicate lack of ability to obtain required WinSock library
        MessageBox( NULL,
            "Unable to access a WinSock 2.0 library.  Be sure the WinSock libraries are in your PATH.",
            progname,
            MB_ICONSTOP | MB_OK );
        ExitProcess( 1 );
    }
}



