/*
 * Copyright (c) 1996-2000 Barton P. Miller
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

// $Id: main.C,v 1.110 2002/12/14 16:37:51 schendel Exp $

#include "common/h/headers.h"
#include "pdutil/h/makenan.h"
#include "common/h/Ident.h"


#if defined(MT_THREAD)
extern "C" const char V_paradyndMT[];
#else
extern "C" const char V_paradynd[];
#endif //MT_THREAD
extern "C" const char V_libpdutil[];

#if defined(MT_THREAD)
Ident V_id(V_paradyndMT,"Paradyn");
#else
Ident V_id(V_paradynd,"Paradyn");
#endif
Ident V_Uid(V_libpdutil,"Paradyn");

#include "rtinst/h/rtinst.h"

#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/dyninstP.h"
#include "paradynd/src/comm.h"
#include "paradynd/src/internalMetrics.h"
#include "common/h/machineType.h"
#include "paradynd/src/init.h"
#include "paradynd/src/perfStream.h"
#include "paradynd/src/mdld.h"
#include "paradynd/src/processMgr.h"
#include "paradynd/src/pd_process.h"


pdRPC *tp = NULL;

#ifdef PARADYND_PVM
#include "pvm_support.h"
extern "C" {
#include <pvm3.h>
}
#endif     


bool pvm_running = false;

static string machine_name;
string osName;
int pd_debug=0;

int ready;

#ifdef mips_sgi_irix6_4
extern bool execIrixMPIProcess(vector<string> &argv);
#endif

#ifdef DETACH_ON_THE_FLY
extern void initDetachOnTheFly();
#endif

/*
 * These variables are global so that we can easily find out what
 * machine/socket/etc we're connected to paradyn on; we may need to
 * start up other paradynds (such as on the CM5), and need this later.
 */
string pd_machine;
static int pd_attpid;
static int pd_known_socket_portnum=0;
static int pd_flag=0;
static string pd_flavor;
// Unused on NT, but this makes prototypes simpler.
int termWin_port = -1;


void configStdIO(bool closeStdIn)
{
    int nullfd;

    /* now make stdin, out and error things that we can't hurt us */
    if ((nullfd = open("/dev/null", O_RDWR, 0)) < 0) {
	abort();
    }

    if (closeStdIn) (void) dup2(nullfd, 0);
    (void) dup2(nullfd, 1);
    (void) dup2(nullfd, 2);

    if (nullfd > 2) close(nullfd);
}

void sigtermHandler()
{
  showErrorCallback(98,"paradynd has been terminated");
}

// Cleanup for pvm and exit.
// This function must be called when we exit, to clean up and exit from pvm.
// Now also cleans up shm segs by deleting all processes  -ari
void cleanUpAndExit(int status) {
#ifdef PARADYND_PVM
  if (pvm_running)
    PDYN_exit_pvm();
#endif

#if !defined(i386_unknown_nt4_0)
  // delete the trace socket file
  unlink(traceSocketPath.c_str());
#endif

#ifdef SHM_SAMPLING_DEBUG
   cerr << "paradynd cleanUpAndExit: deleting all process structures now" << endl;
#endif

   processMgr::procIter itr = getProcMgr().begin();
   while(itr != getProcMgr().end()) {
      pd_process *theProc = *itr++;
      if (theProc == NULL)
	 continue; // process has already been cleaned up

#if defined(i386_unknown_linux2_0) || defined(alpha_dec_osf4_0) || defined(ia64_unknown_linux2_4)
      // Try to be a bit smarter when we clean up the processes - kill
      // all processes that were created, leave all processes that were
      // attached to running.  This should really go into the process class,
      // but I hesitate to do that this close to the release (3.0)
      // -nick (24-Mar-2000)
      int pid = theProc->getPid();
      bool wasAttachedTo = theProc->wasCreatedViaAttach();
#endif

#if defined(i386_unknown_linux2_0) || defined(alpha_dec_osf4_0) || defined(ia64_unknown_linux2_4)
      if (!wasAttachedTo) OS::osKill(pid);
#endif
      
      getProcMgr().removeProcess(theProc);
      delete theProc; // calls pd_process::~pd_process, which fries the shm seg
  }

  P_exit(status);
}

bool
RPC_undo_arg_list (string &flavor, unsigned argc, char **argv, 
		   string &machine, int &well_known_socket,int &termWin_port, int &flag, int &attpid)
{
  char *ptr;
  int c;
  extern char *optarg;
  bool err = false;
  const char optstring[] = "p:P:vVL:m:l:z:a:r:";

  // Defaults (for ones that make sense)
  machine = "localhost";
  flavor = "unix";
  flag = 2;
  well_known_socket = 0;
  termWin_port = 0;
  attpid = 0;
  bool stop = false;
  while ( (c = P_getopt(argc, argv, optstring)) != EOF && !stop)
    switch (c) {
    case 'p':
      // Port number to connect to for the Paradyn command port
      well_known_socket = P_strtol(optarg, &ptr, 10);
      if (ptr == optarg) err = true;
      break;
    case 'P':
      termWin_port = P_strtol(optarg, &ptr, 10);
      if (ptr == optarg) err = true;
      break;
    case 'v':
      // Obsolete argument
      err = true;
      break;
    case 'V':
      cout << V_id << endl;
      break;
    case 'L':
      // Debugging flag to specify runtime library
      process::paradynRT_name = optarg;
      break;
    case 'm':
      // Machine specification. We could do "machine:portname", but there are
      // two ports. 
      machine = optarg;
      break;
    case 'l':
      flag = P_strtol(optarg, &ptr, 10);
      if (ptr == optarg) err = true;
      break;
    case 'z':
      flavor = optarg;
      break;
    case 'a':
      attpid = P_strtol(optarg, &ptr, 10);
      break;
    case 'r':
      // We've hit the "runme" parameter. Stop processing
      stop = true;
      break;
    default:
      err = true;
      break;
    }
  if (err)
    return false;

  // verify required parameters
  // This define should be something like ENABLE_TERMWIN, but hey
#if !defined(i386_unknown_nt4_0)
  if (!well_known_socket && !termWin_port)
    return false;
#else
  if (!well_known_socket)
    return false;
#endif
  return true;
}

// PARADYND_DEBUG_XXX
static void initialize_debug_flag(void) {
  char *p;

  if ( (p=getenv("PARADYND_DEBUG_INFRPC")) ) {
    cerr << "Enabling inferior RPC debugging code" << endl;
    pd_debug_infrpc = 1;
  }

  if ( (p=getenv("PARADYND_DEBUG_CATCHUP")) ) {
    cerr << "Enabling catchup instrumentation debugging code" << endl;
    pd_debug_catchup = 1;
  }
}


static
void
PauseIfDesired( void )
{
	char *pdkill = getenv( "PARADYND_DEBUG" );
	if( pdkill && ( *pdkill == 'y' || *pdkill == 'Y' ) )
	{
		int pid = getpid();
		cerr << "breaking for debug in controllerMainLoop...pid=" << pid << endl;
#if defined(i386_unknown_nt4_0)
		DebugBreak();
#elif defined(sparc_sun_solaris2_4) || defined(i386_unknown_solaris2_5)
		bool bCont = false;
		while( !bCont )
		{
			sleep( 1 );
		}
#else
		kill(pid, SIGSTOP);
#endif
	}
}


#if !defined(i386_unknown_nt4_0)
static
void
InitSigTermHandler( void )
{
    struct sigaction act;

    // int i = 1;
    // while (i) {};

    act.sa_handler = (void (*)(int)) sigtermHandler;
    act.sa_flags   = 0;

    /* for AIX - default (non BSD) library does not restart - jkh 7/26/95 */
#if defined(SA_RESTART)
    act.sa_flags  |= SA_RESTART;
#endif

    sigfillset(&act.sa_mask);

    if (sigaction(SIGTERM, &act, 0) == -1) {
        perror("sigaction(SIGTERM)");
        abort();
    }
}
#endif // !defined(i386_unknown_nt4_0)

#if defined(i386_unknown_nt4_0)
static
void
InitWinsock( void )
{
    // Windows NT needs to initialize winsock library
    WORD wsversion = MAKEWORD(2,0);
    WSADATA wsadata;
    WSAStartup(wsversion, &wsadata);
}
#endif // defined(i386_unknown_nt4_0)




static
void
InitForMPI( char* argv[], const string& pd_machine )
{
	// Both IRIX and AIX MPI job-launchers will start paradynd,
	// which must report to paradyn
	// the pdRPC is allocated and reportSelf is called
	tp = new pdRPC(AF_INET, pd_known_socket_portnum, SOCK_STREAM, 
	     pd_machine, NULL, NULL, 2);
	assert(tp != NULL);

	tp->reportSelf(machine_name, argv[0], getpid(), "mpi");
}

static
void
InitManuallyStarted( char* argv[],  const string& pd_machine )
{
	bool reported = false;

	// report back to our front end
	tp = new pdRPC(AF_INET, pd_known_socket_portnum, SOCK_STREAM, 
					pd_machine, NULL, NULL, 2);
	assert(tp);
	if (!tp->net_obj())
	{
		cerr << "Failed to establish connection to Paradyn on "
			 << pd_machine << " port " << pd_known_socket_portnum << endl;
		cleanUpAndExit(-1);
	}
	tp->reportSelf(machine_name, argv[0], getpid(), pd_flavor);
	reported = true;

#if defined(PARADYND_PVM)
	// are we designated for monitoring a PVM application?
	if (pvm_running
		&& !PDYN_initForPVM (argv, pd_machine, pd_known_socket_portnum, 1))
	{
		cleanUpAndExit(-1);
	}
#endif // PARADYND_PVM
}



static
void
InitRemotelyStarted( char* argv[], const string& pd_machine, bool report )
{ 
  // we are a remote daemon started by rsh/rexec or some other
  // use socket to connect back to our front end
    
  // setup socket
  
  // We must get a connection with paradyn before starting any 
  // other daemons, or else one of the daemons we start 
  // (in PDYN_initForPVM), may get our connection.
  tp = new pdRPC(AF_INET, pd_known_socket_portnum, SOCK_STREAM, 
		 pd_machine, NULL, NULL, 2);
  assert( tp != NULL );
  
  // ??? is this the right thing to do? it was in the
  // non-PVM version of the code, but not in the PVM version
  // we decide whether to report if the cmdLine was empty or not
  if( report )
    {
      tp->reportSelf( machine_name, argv[0], getpid(), pd_flavor );
    }
}



static
void
InitLocallyStarted( char* []
#if PARADYND_PVM
                    argv
#endif
                    , const string& 
#ifdef PARADYND_PVM
                    pd_machine 
#endif
)
{
#ifdef PARADYND_PVM
	// check if we are designated to monitor a PVM application
	if( pvm_running && 
		!PDYN_initForPVM( argv, pd_machine, pd_known_socket_portnum, 1 ))
	{
		// TODO -- report error here
		cleanUpAndExit( -1 );
	}
#endif // PARADYND_PVM

	// connect to our front end using our stdout
	OS::osDisconnect();

#if !defined(i386_unknown_nt4_0)
	PDSOCKET sock = 0;
#else
	PDSOCKET sock = _get_osfhandle(0);
#endif // defined(i386_unknown_nt4_0)
	tp = new pdRPC(sock, NULL, NULL, 2);

	// note that we do not need to report to our front end in this case
}


#if defined(PARADYND_PVM)
static
void
InitForPVM( char* argv[], const string& pd_machine )
{
	// Check whether we are we are being created for a PVM application.
	// Also, check whether we are the first PVM paradynd or if were started 
	// by the first paradynd using pvm_spawn()
    int pvmParent = pvm_parent();

	if (pvmParent == PvmSysErr)
	{
		cout << "Unable to connect to PVM daemon; is PVM running?\n" << endl;
		cleanUpAndExit(-1);
	}
	else
	{
		pvm_running = true;
	}

    if (pvm_running && (pvmParent != PvmNoParent))
	{
		// we were started using pvm_spawn() by another paradynd
		if (!PDYN_initForPVM (argv, pd_machine, pd_known_socket_portnum, 0))
		{
			// TODO -- report error here
			cleanUpAndExit(-1);
		}

		// report to our front end
		tp = new pdRPC(AF_INET, pd_known_socket_portnum, SOCK_STREAM, 
						pd_machine, NULL, NULL, 2);
		assert(tp);

		tp->reportSelf (machine_name, argv[0], getpid(), "pvm");
    }
}
#endif // PARADYND_PVM


#if !defined(i386_unknown_nt4_0)
void sighup_handler( int ) {
   // we get SIGHUP when Paradyn closes our connection
   // we want to ignore this, and close 
}

void sighupInit() {
   // ensure that we don't die from SIGHUP when our connection
   // to Paradyn closes
   struct sigaction act;
   
   act.sa_handler = sighup_handler;
   act.sa_flags = 0;
   sigfillset( &act.sa_mask );
   
   if( sigaction( SIGHUP, &act, 0 ) == -1 )
   {
      cerr << "Failed to install SIGHUP handler: " << errno << endl;
      // this isn't a fatal error for us
   }
}
#endif

//
// Note: the pd_flag variable is set from the argument to the -l command
// line switch.  It has the following meaning:
//
// pd_flag == 0 => remote daemon started by Paradyn using rsh/rexec
// pd_flag == 1 => local daemon started by Paradyn using fork/exec
// pd_flag == 2 => daemon started manually
//


int
main( int argc, char* argv[] )
{
   PauseIfDesired();
   initialize_debug_flag();
   
#if !defined(i386_unknown_nt4_0)
   InitSigTermHandler();
#endif // defined(i386_unknown_nt4_0)
   
   
#ifdef DETACH_ON_THE_FLY
   initDetachOnTheFly();
#endif
  
#if defined(i386_unknown_nt4_0)
   InitWinsock();
#endif // defined(i386_unknown_nt4_0)
  
  
   //
   // process command line args passed in
   //
   process::programName = argv[0];
   bool aflag;
#ifdef DEBUG
   // Print command line args
   for (int j = 0; j < argc; j++)
      cerr << argv[j] << " ";
   cerr << endl;
#endif
   aflag = RPC_undo_arg_list (pd_flavor, argc, argv, pd_machine,
                              pd_known_socket_portnum,termWin_port, pd_flag, pd_attpid);
   if (!aflag || pd_debug)
   {
      if (!aflag)
      {
         cerr << "Invalid/incomplete command-line args:" << endl;
      }
      cerr << "   -z<flavor";
      if (pd_flavor.length())
      {
         cerr << "=" << pd_flavor;
      }
      cerr << "> -l<flag";
      if (pd_flag)
      {
         cerr << "=" << pd_flag;
      }
      cerr << "> -m<hostmachine";
      if (pd_machine.length()) 
      {
         cerr << "=" << pd_machine;
      }
      cerr << "> -p<hostport";
      if (pd_known_socket_portnum)
      {
         cerr << "=" << pd_known_socket_portnum;
      }
      cerr << "> -apid<attachpid";
      if (pd_attpid)
      {
         cerr << "=" << pd_attpid;
      }
      cerr << ">" << endl;
      if (process::paradynRT_name.length())
      {
         cerr << "   -L<library=" << process::paradynRT_name << ">" << endl;
      }
      if (!aflag)
      {
         cleanUpAndExit(-1);
      }
   }
   
#if !defined(i386_unknown_nt4_0)
   extern PDSOCKET connect_Svr(string machine,int port);
   PDSOCKET stdout_fd=INVALID_PDSOCKET;
   if ((stdout_fd = connect_Svr(pd_machine,termWin_port)) == INVALID_PDSOCKET)
      cleanUpAndExit(-1);
   if (write(stdout_fd,"from_paradynd\n",strlen("from_paradynd\n")) <= 0)
      cleanUpAndExit(-1);
   
   dup2(stdout_fd,1);
   dup2(stdout_fd,2);
#endif

#if !defined(i386_unknown_nt4_0)
   aflag = RPC_make_arg_list(process::arg_list,
                             pd_known_socket_portnum, termWin_port,pd_flag, 0,
                             pd_machine, true);
#else
   aflag = RPC_make_arg_list(process::arg_list,
                             pd_known_socket_portnum, pd_flag, 0,
                             pd_machine, true);
#endif 
   assert(aflag);
   string flav_arg(string("-z")+ pd_flavor);
   process::arg_list += flav_arg;
   machine_name = getNetworkName();
   
   //
   // See if we should fork an app process now.
   //

   // We want to find two things
   // First, get the current working dir (PWD)
   string* dir = new string(getenv("PWD"));

   // Second, put the inferior application and its command line
   // arguments into cmdLine. Basically, loop through argv until
   // we find -runme, and put everything after it into cmdLine.
   vector<string> cmdLine;
   unsigned int argNum = 0;
   while ((argNum < (unsigned int)argc) && (strcmp(argv[argNum], "-runme")))
   {
      argNum++;
   }
   // Okay, argNum is the command line argument which is "-runme" - skip it
   argNum++;
   // Copy everything from argNum to < argc
   // this is the command that is to be issued
   for (unsigned int i = argNum; i < (unsigned int)argc; i++)
   {
      cmdLine += argv[i];
   }
   // note - cmdLine could be empty here, if the -runme flag were not given
   

   // There are several ways that we might have been started.
   // We need to connect to Paradyn differently depending on which 
   // method was used.
   //
   // Use our own stdin if:
   //   started as local daemon by Paradyn using fork+exec
   //
   // Use a socket described in our command-line args if:
   //   started as remote daemon by rsh/rexec
   //   started manually on command line
   //   started by MPI
   //   started as PVM daemon by another Paradyn daemon using pvm_spawn() 
   //
    
   
   process::pdFlavor = pd_flavor;
#ifdef PDYN_DEBUG
   cerr << "pd_flavor: " << pd_flavor.c_str() << endl;
#endif


#ifdef PARADYND_PVM
   if (pd_flavor == string("pvm"))
   {
      InitForPVM( argv, pd_machine );
   }
#endif // PARADYND_PVM
   
   if( tp == NULL )
   {
      // we haven't yet reported to our front end
      
      if( pd_flavor == "mpi" )
      {
         InitForMPI( argv, pd_machine );
      }
      else if( pd_flag == 0 )
      {
         // we are a remote daemon started by rsh/rexec or some other
         InitRemotelyStarted( argv, pd_machine, (cmdLine.size() > 0) );
      }
      else if( pd_flag == 1 )
      {
         // we were started by a local front end using fork+exec
         InitLocallyStarted( argv, pd_machine );
      }
      else if( (pd_flag == 2)|| (pd_flag == 3))
      {
         // we were started manually (i.e., from the command line) -2
         // we were started manually by a daemon - 3
         InitManuallyStarted( argv, pd_machine );
      }
      
   }
   
   // by now, we should have a connection to our front end
   if( tp == NULL )
   {
      if( (pd_flag < 0) || (pd_flag > 3) )
      {
         cerr << "Paradyn daemon: invalid -l value " << pd_flag << " given." 
              << endl;
      }
      else
      {
         cerr << "Paradyn daemon: invalid command-line options seen" << endl;
      }
      cleanUpAndExit(-1);
   }
   assert( tp != NULL );
   
#if defined(MT_THREAD)
   statusLine(V_paradyndMT);
#else
   statusLine(V_paradynd);
#endif
   
   // Note -- it is important that this daemon receives all mdl info
   // before starting a process
   aflag = mdl_get_initial(pd_flavor, tp);
   assert(aflag);
   
   initLibraryFunctions();
#if defined(i386_unknown_linux2_0) || defined(ia64_unknown_linux2_4)
   sighupInit();
#endif
   if (!paradyn_init()) 
   {
      abort();
   }
   bool startByAttach = false;
   bool startByCreateAttach = false;
#ifdef mips_sgi_irix6_4
   struct utsname unameInfo;
   if ( P_uname(&unameInfo) == -1 )
   {
      perror("uname");
      return false;
   }
   
   // osName is used in irix.C and process.C
   osName = unameInfo.sysname;
   
   if ( pd_flavor == "mpi" && osName.prefixed_by("IRIX") )
   {
      if ( !execIrixMPIProcess(cmdLine) )
         return(0);
   }
   else
#endif
      
      // spawn the given process, if necessary
      if (cmdLine.size() && (pd_attpid==0))
      {
         vector<string> envp;
         // ignore return val (is this right?)
         extern int pd_createProcess(vector<string> &argv, 
                                     vector<string> &envp, string dir);
         pd_createProcess(cmdLine, envp, *dir); 
      } 
      else if (pd_attpid && (pd_flag==2))
      {
         // We attach after doing a last bit of initialization, below
         startByAttach = true;
      }else if (pd_attpid && (pd_flag==3))
      {
         // We attach to a just created application after doing a
         // last bit of initialization, below
         startByCreateAttach = true;
      }
   
   // Set up the trace socket. This is needed before we try
   // to attach to a process
   extern void setupTraceSocket();
   setupTraceSocket();
   if (startByAttach) {
      extern bool pd_attachProcess(const string &progpath, int pid, 
                                   int afterAttach);
      bool success = pd_attachProcess("", pd_attpid, 1);
      if (!success) return(-1);
   } else if (startByCreateAttach) {
      if (cmdLine.size()){
         AttachToCreatedProcess(pd_attpid,cmdLine[0]); 
      } else {
         AttachToCreatedProcess(pd_attpid,"");
      }
   }
   
   controllerMainLoop(true);
   return(0);
}
