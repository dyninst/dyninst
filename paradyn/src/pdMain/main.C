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

// $Id: main.C,v 1.70 2004/06/21 19:37:58 pcroth Exp $

/*
 * main.C - main routine for paradyn.  
 *   This routine creates DM, UIM, VM, and PC threads.
 */

#include "../TCthread/tunableConst.h"
#include "common/h/headers.h"
#include "paradyn.h"
#include "pdthread/h/thread.h"
#include "dataManager.thread.SRVR.h"
#include "VM.thread.SRVR.h"
#include "paradyn/src/DMthread/BufferPool.h"
#include "paradyn/src/DMthread/DVbufferpool.h"
#include "paradyn/src/PCthread/PCextern.h"
#include "paradyn/src/UIthread/UIglobals.h"


// trace data streams
BufferPool<traceDataValueType>  tracedatavalues_bufferpool;

// maybe this should be a thread, but for now it's a global var.
BufferPool<dataValueType>  datavalues_bufferpool;


extern void *UImain(void *);
extern void *DMmain(void *);
extern void *PCmain(void *);
extern void *VMmain (void *);

#define MBUFSIZE 256
#define DEBUGBUFSIZE	4096


// expanded stack by a factor of 10 to support AIX - jkh 8/14/95
// wrapped it in an ifdef so others don't pay the price --ari 10/95
#if defined(rs6000_ibm_aix3_2) || defined(rs6000_ibm_aix4_1)
char UIStack[327680];
char DMStack[327680];
#else
char UIStack[32768];
char DMStack[32768];
#endif


// applicationContext *context;
dataManagerUser *dataMgr = NULL;
performanceConsultantUser *perfConsult = NULL;
UIMUser *uiMgr = NULL;
VMUser  *vmMgr = NULL;
int paradyn_debug=0;
char debug_buf[DEBUGBUFSIZE];
pdstring pclStartupFileName = "";
pdstring tclStartupFileName = "";
pdstring daemonStartupInfoFileName = "";
bool useGUITermWin = true;

// default_host defines the host where programs run when no host is
// specified in a PCL process definition, or in the process definition window.
pdstring default_host="";
pdstring local_domain="";

#define PRINT_DEBUG_MACRO				\
do {							\
	va_list args;					\
	va_start(args,format);				\
	(void) fflush(stdout);				\
	(void) vsprintf(debug_buf, format, args);	\
	(void) fprintf(stdout,"THREAD %d: %s\n",	\
	       thr_self(), debug_buf);			\
	(void) fflush(stdout);                          \
	va_end(args);					\
} while (0)

void print_debug_macro(const char* format, ...){
    if(paradyn_debug > 0)
     PRINT_DEBUG_MACRO;
    return;
}

void eFunction(int errno, char *message)
{
    printf("error #%d: %s\b", errno, message);
    abort();
}

bool metDoTunable();
bool metDoProcess();
bool metDoDaemon();
void ParseCommandLine( int argc, char* argv[] );


int
main(int argc, char* argv[])
{
    unsigned int msgsize;
    char mbuf[MBUFSIZE];
    thread_t mtid;
    tag_t mtag;

    // Check whether we're to output debug messages
    // If the value of PARADYNDEBUG environment variable is > 0,
    // PARADYN_DEBUG messages will be printed to stdout
    char* temp = (char *) getenv("PARADYNDEBUG");
    if (temp != NULL) {
        paradyn_debug = atoi(temp);
    }

    // Parse the command line
    // We do this before we create the UI thread because we need to know
    // what type of UI to create
    ParseCommandLine( argc, argv );

    // save the tid of the main thread (and, as a side effect,
    // initialize the thread library)
    thread_t MAINtid = thr_self();

    // spawn our UI thread early, so we can use it in case there
    // are problems with startup of other threads
    UIthreadArgs uiArgs( MAINtid, argv[0] );
    thread_t UIMtid = THR_TID_UNSPEC;
    int cret = thr_create( UIStack, sizeof(UIStack),
                            &UImain, &uiArgs,
                            0, &UIMtid );
    if( cret == THR_ERR )
    {
        // we have no user interface thread -
        // use lowest common denominator to report the error
        fprintf( stderr, "Fatal error: Paradyn was unable to create its user interface and must exit.\n" );
        exit(1);
    }

    // ensure the UI thread is initialized before continuing,
    // so all other threads can use the UI for reporting status, errors, etc.
    msgsize = MBUFSIZE;
    mtid = THR_TID_UNSPEC;
    mtag = MSG_TAG_UIM_READY;
    msg_recv( &mtid, &mtag, mbuf, &msgsize );
    assert( mtid == UIMtid );
    assert( mtag == MSG_TAG_UIM_READY );
    uiMgr = new UIMUser(UIMtid);
    PARADYN_DEBUG (("UI thread created\n"));


    //
    // We check our own read/write events.
    //
    // TODO is this necessary anymore?
#if !defined(i386_unknown_nt4_0)
    P_signal(SIGPIPE, (P_sig_handler) SIG_IGN);
#endif // !defined(i386_unknown_nt4_0)


    const pdstring localhost = getNetworkName();
    //cerr << "main: localhost=<" << localhost << ">" << endl;
    unsigned index=0;
    while (index<localhost.length() && localhost[index]!='.') index++;
    if (index == localhost.length())
      cerr << "Failed to determine local machine domain: localhost=<" 
           << localhost << ">" << endl;
    else
      local_domain = localhost.substr(index+1,localhost.length());
    //cerr << "main: local_domain=<" << local_domain << ">" << endl;

    // enable interaction between thread library and RPC package
    rpcSockCallback += (RPCSockCallbackFunc)clear_ready_sock;


// TODO let these threads do this initialization once they are started
// why do they have to do this initialization as part of the main thread?

    // call sequential initialization routines
    if(!dataManager::DM_sequential_init( pclStartupFileName.c_str() )) {
    printf("Error found in Paradyn Configuration File, exiting\n");
    exit(-1);
    }


    // spawn the remaining main threads: data manager, visi manager,
    // and Performance Consultant.  Other threads are created on an 
    // as-needed basis.
  
    // Spawn data manager thread
    DMthreadArgs dmArgs( MAINtid, useGUITermWin );
    thread_t DMtid = THR_TID_UNSPEC;
    cret = thr_create( DMStack, sizeof(DMStack),
                        DMmain, &dmArgs,
                        0, &DMtid );
    if( cret == THR_ERR )
    {
        exit( -1 );
    }
    PARADYN_DEBUG (("DM thread created\n"));

    msgsize = MBUFSIZE;
    mtid = THR_TID_UNSPEC;
    mtag = MSG_TAG_DM_READY;
    msg_recv(&mtid, &mtag, mbuf, &msgsize);
    assert( mtid == DMtid );
    msg_send (DMtid, MSG_TAG_ALL_CHILDREN_READY, (char *) NULL, 0);
    dataMgr = new dataManagerUser (DMtid);
    uiMgr->DMready();
    // context = dataMgr->createApplicationContext(eFunction);


    // Spawn the Performance Consultant
    PCthreadArgs pcArgs( MAINtid );
    thread_t PCtid = THR_TID_UNSPEC;
    cret = thr_create(0, 0, 
            PCmain, &pcArgs,
            0, (unsigned int *) &PCtid);
    if( cret == THR_ERR)
    {
        exit(1);
    }
    PARADYN_DEBUG (("PC thread created\n"));

    msgsize = MBUFSIZE;
    mtid = THR_TID_UNSPEC;
    mtag = MSG_TAG_PC_READY;
    msg_recv(&mtid, &mtag, mbuf, &msgsize);
    assert( mtid == PCtid );
    msg_send (PCtid, MSG_TAG_ALL_CHILDREN_READY, (char *) NULL, 0);
    perfConsult = new performanceConsultantUser (PCtid);

    // Spawn the Visi Manager thread
    VMthreadArgs vmArgs( MAINtid );
    thread_t VMtid = THR_TID_UNSPEC;
    cret = thr_create(0, 0,
                VMmain, &vmArgs,
                0, (unsigned int *) &VMtid);
    if( cret == THR_ERR)
    {
        exit(1);
    }

    PARADYN_DEBUG (("VM thread created\n"));
    msgsize = MBUFSIZE;
    mtid = THR_TID_UNSPEC;
    mtag = MSG_TAG_VM_READY;
    msg_recv(&mtid, &mtag, mbuf, &msgsize);
    assert( mtid == VMtid );
    msg_send (VMtid, MSG_TAG_ALL_CHILDREN_READY, (char *) NULL, 0);
    vmMgr = new VMUser(VMtid);

    // execute the commands in the configuration files
    metDoTunable();
    metDoDaemon();
    metDoProcess();

    // keep this here to prevent UI from starting up till everything's 
    // been initialized properly!!
    //  -OR-
    // move this elsewhere to create a race condition
    if( tclStartupFileName.length() > 0 )
    {
        assert( uiMgr != NULL );
        uiMgr->readStartupFile( tclStartupFileName.c_str() );
    }

    if( daemonStartupInfoFileName.length() > 0 )
    {
        assert( dataMgr != NULL );
        dataMgr->printDaemonStartInfo( daemonStartupInfoFileName.c_str() );
    }

    // Block until the UI thread exits, indicating we should shut down
    thr_join(UIMtid, NULL, NULL);

    // Tell the other threads to exit
    msg_send(DMtid, MSG_TAG_DO_EXIT_CLEANLY, (char *) NULL, 0);
    thr_join(DMtid, NULL, NULL);

    msg_send(VMtid, MSG_TAG_DO_EXIT_CLEANLY, (char *) NULL, 0);
    thr_join(VMtid, NULL, NULL);

    msg_send(PCtid, MSG_TAG_DO_EXIT_CLEANLY, (char *) NULL, 0);
    thr_join(PCtid, NULL, NULL);

    thr_library_cleanup();
  
    return 0;
}


void
ParseCommandLine( int /* argc */, char* argv[] )
{
    int a_ct=1;
    while (argv[a_ct])
    {
        if(!strcmp(argv[a_ct], "-f") && argv[a_ct+1] &&
            (pclStartupFileName.length() == 0) )
        {
            pclStartupFileName = argv[a_ct+1];
            a_ct += 2;
        }
        else if(!strcmp(argv[a_ct], "-s") && argv[a_ct+1] &&
            (tclStartupFileName.length() == 0) )
        {
            tclStartupFileName = argv[a_ct+1];
            a_ct += 2;
        }
        else if(!strcmp(argv[a_ct], "-x") && argv[a_ct+1] &&
            (daemonStartupInfoFileName.length() == 0) )
        {
            daemonStartupInfoFileName = argv[a_ct+1];
            a_ct += 2;
        }
        else if( !strcmp(argv[a_ct], "-cl") )
        {
            useGUITermWin = false;
            a_ct += 1;
        }
        else if((!strcmp(argv[a_ct], "-default_host") || 
                    !strcmp(argv[a_ct], "-d")) && argv[a_ct+1] &&
                    (default_host.length() == 0) )
        {
            default_host = argv[a_ct+1];
            a_ct += 2;
        }
        else
        {
            // unrecognized command line switch
            std::cerr << "usage: " << argv[0]
                << " [-f <pcl_filename>]"
                << " [-s <tcl_scriptname>]"
                << " [-x <connect_filename>]"
                << " [-cl]"
                << " [-default_host <hostname>]"
                << std::endl;
            exit(-1);
        }
    }
}

