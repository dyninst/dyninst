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

// $Id: main-nt.C,v 1.2 1999/12/17 16:25:01 pcroth Exp $

/*
 * main-nt.C - WinMain for Paradyn on Windows.  
 *   This routine simply calls the existing main() routine.
 */
#include <windows.h>

extern	int	main( int, char** );


static
void
CleanupSockets( void )
{
	WSACleanup();
}

static
void
InitSockets( const char* progname )
{
	WSADATA	data;


	// request WinSock 2.0
	if( WSAStartup( MAKEWORD(2,0), &data ) != 0 )
	{
		// indicate lack of ability to obtain required WinSock library
		MessageBox( NULL, "Unable to access a WinSock 2.0 library.  Be sure the WinSock libraries are in your PATH.", progname, MB_ICONSTOP | MB_OK );
		ExitProcess( 1 );
	}

	// we've seen a successful WSAStartup call, so set things
	// so that WSACleanup is called on process exit
	atexit( CleanupSockets );

	// verify that the version that was provided is one we can use
	if( (LOBYTE(data.wVersion) != 2) || (HIBYTE(data.wVersion) != 0) )
	{
		// indicate lack of ability to obtain required WinSock library
		MessageBox( NULL, "Unable to access a WinSock 2.0 library.  Be sure the WinSock libraries are in your PATH.", progname, MB_ICONSTOP | MB_OK );
		ExitProcess( 1 );
	}
}


int
WINAPI
WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
#ifdef _DEBUG
	// allow the user to attach a debugger if desired
    MessageBox( NULL, "Press OK to continue", "Pause", MB_OK );
#endif _DEBUG

	// initialize our use of the WinSock library
	InitSockets( __argv[0] );	

	// call the traditional main()
	return main( __argc, __argv );
}


