/*
 * Copyright (c) 1996-2003 Barton P. Miller
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

// $Id: main-nt.C,v 1.4 2003/05/08 19:48:59 pcroth Exp $

/*
 * main-nt.C - WinMain for Paradyn on Windows.  
 *   This routine simply calls the existing main() routine.
 */
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <iostream.h>
#include <fstream.h>
#include <io.h>
#include <fcntl.h>
#include <strstrea.h>
#include <conio.h>


// prototypes of functions used in this file
int	main( int, char** );

// globals
bool waitForKeypressOnExit = true;

// local variables
static	ifstream*	pstdin_str	= NULL;
static	ofstream*	pstdout_str	= NULL;
static	ofstream*	pstderr_str	= NULL;



static
void
CleanupSockets( void )
{
	WSACleanup();
}



// We would prefer not to use goto for control flow here,
// but C++ exceptions are not available
static
void
InitSockets( void )
{
	WSADATA	data;


	// request WinSock 2.0
	if( WSAStartup( MAKEWORD(2,0), &data ) != 0 )
	{
		goto init_sockets_handle_error;
	}

	// we've seen a successful WSAStartup call, so set things
	// so that WSACleanup is called on process exit
	atexit( CleanupSockets );

	// verify that the version that was provided is one we can use
	if( (LOBYTE(data.wVersion) != 2) || (HIBYTE(data.wVersion) != 0) )
	{
		goto init_sockets_handle_error;
	}
	return;

init_sockets_handle_error:
	MessageBox( NULL,
				_T("Paradyn could not find a compatible WinSock library.  The program cannot continue."),
				_T("Paradyn"),
				MB_ICONSTOP | MB_OK );

	ExitProcess( 1 );
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
		goto handle_init_console_error;
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
		goto handle_init_console_error;
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
		goto handle_init_console_error;
	}

	// associate the standard C streams with our handles
	{
		FILE* stdin_file = _fdopen( stdin_fileno, "r" );
		FILE* stdout_file = _fdopen( stdout_fileno, "w" );
		FILE* stderr_file = _fdopen( stderr_fileno, "w" );
		*stdin = *stdin_file;
		*stdout = *stdout_file;
		*stderr = *stderr_file;

		// associate the standard C++ streams with our handles
		// note that we allocate the objects on the heap so that
		// they are not destroyed when we exit the current block
		// (which would leave cin, cout, and cerr as junk)
		pstdin_str = new ifstream( stdin_fileno );
		pstdout_str = new ofstream( stdout_fileno );
		pstderr_str = new ofstream( stderr_fileno );
		cin = *pstdin_str;
		cout = *pstdout_str;
		cerr = *pstderr_str;
	}

    // we've got a console
    //
    // ensure that we leave it around long enough for user to
    // see any error messages we dump out to the console
    atexit( WaitForKeypress );

	return;

handle_init_console_error:
	{
		ostrstream estr;

		estr << "Paradyn was unable to create a console."
			<< " [" << GetLastError() << "]"
			<< ends;

		MessageBox( NULL, estr.str(), _T("Paradyn"), MB_ICONSTOP | MB_OK );
		ExitProcess( 1 );
	}
}


static
void
CleanupConsole( void )
{
	delete pstdin_str;
	delete pstdout_str;
	delete pstderr_str;
}


int
WINAPI
WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	int ret;

	// set up a console for our console I/O
	InitConsole();

	// initialize our use of the WinSock library
	InitSockets();	

	// call the traditional main()
	ret = main( __argc, __argv );

	// cleanup
	CleanupConsole();

	return ret;
}


