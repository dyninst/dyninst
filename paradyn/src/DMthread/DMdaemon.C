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

/*
 * $Id: DMdaemon.C,v 1.135 2003/06/17 17:54:50 pcroth Exp $
 * method functions for paradynDaemon and daemonEntry classes
 */
#include "paradyn/src/pdMain/paradyn.h"
#include <assert.h>
#include <stdio.h>
#include <malloc.h>

extern "C" {
#include <rpc/types.h>
#include <rpc/xdr.h>
}
#include <string.h>
#include <sys/types.h>
#include <ctype.h>
#include <limits.h>

#include "pdthread/h/thread.h"

#include "dataManager.thread.h"
#include "dyninstRPC.xdr.CLNT.h"
#include "DMdaemon.h"
#include "paradyn/src/TCthread/tunableConst.h"
#include "paradyn/src/UIthread/Status.h"
#include "DMmetric.h"
#include "paradyn/src/met/metricExt.h"
#include "pdutil/h/rpcUtil.h"
#include "pdutil/h/pdDebugOstream.h"
#include "common/h/timing.h"
#include "DMmetricFocusReq.h"


// TEMP this should be part of a class def.
string DMstatus="Data Manager";
bool DMstatus_initialized = false;

extern unsigned enable_pd_samplevalue_debug;
extern pdvector<string> mdl_files;

#if ENABLE_DEBUG_CERR == 1
#define sampleVal_cerr if (enable_pd_samplevalue_debug) cerr
#else
#define sampleVal_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

// This is from met/metMain.C
void metCheckDaemonProcess( const string & );


pdvector<paradynDaemon::MPICHWrapperInfo> paradynDaemon::wrappers;
dictionary_hash<string, pdvector<paradynDaemon*> > paradynDaemon::daemonsByHost( string::hash );


// change a char* that points to "" to point to NULL
// NULL is used to signify "NO ARGUMENT"
// NULL is easier to detect than "" (which needs a strlen to detect)
/*
static void fixArg(char *&argToFix)
{
  if (argToFix && !strlen(argToFix))
    argToFix = (char*) 0;
}
*/


static appState applicationState = appPaused; // status of the application.


const string daemonEntry::getRemoteShellString() const {
   if(remote_shell.length() > 0) {
      return remote_shell;
   } else {
      const char *rshCmd = getRshCommand();
      return string(rshCmd);
   }
}

// Called whenever a program is ready to run (both programs started by paradyn
// and programs forked by other programs). QUESTION: What about when a program does
// an exec syscall?
//
// The new program inherits all enabled metrics, and if the application is 
// running, the new program must run too.
// 
// Called by the igen routine newProgramCallbackFunc()

bool paradynDaemon::addRunningProgram (int pid,
				       const pdvector<string> &paradynd_argv,
				       paradynDaemon *daemon,
				       bool calledFromExec,
				       bool isInitiallyRunning) {
    executable *exec = NULL;
    
    if (calledFromExec) {
        for (unsigned i=0; i < programs.size(); i++) {
            if ((int) programs[i]->pid == pid && programs[i]->controlPath == daemon) {
	     exec = programs[i];
	     break;
            }
        }
        assert(exec);
        
        // exec() doesn't change the pid, but paradynd_argv changes (big deal).
        exec->argv = paradynd_argv;
        
        // fall through (we probably want to execute the daemon->continueProcess())
    }
    else {
        // the non-exec (the normal) case follows:
        exec = new executable (pid, paradynd_argv, daemon);
        programs += exec;
        ++procRunning;
        
        // the following propagates mi's to the new process IF it's the only
        // process on the daemon.  Otherwise, the daemon can and does propagate
        // on its own.  We don't call it in the exec case (above) since we know it
        // wouldn't do anything.
        daemon->propagateMetrics();
    }
    
    // Now... do we run the application or not? First, check what the frontend
    // thinks is the case
    if (applicationState == appRunning) {
        daemon->continueProcess(pid);
        uiMgr->enablePauseOrRun();
    }
    else {
        // Daemon is handling pause/run, we just need to update our state
        switch(daemon->afterAttach_) {
      case 0:
          // Leave as is... key off the isInitiallyRunning parameter
          if (isInitiallyRunning)
              uiMgr->enablePauseOrRun();
          break;
      case 1:
          break;
      case 2:
          // Run
          uiMgr->enablePauseOrRun();
          break;
        }
    }
    return true;
}

//
// add a new paradyn daemon
// called when a new paradynd contacts the advertised socket
//
bool paradynDaemon::addDaemon (PDSOCKET sock)
{
  // constructor adds new daemon to dictionary allDaemons
  paradynDaemon *new_daemon = new paradynDaemon (sock);

  if (new_daemon->errorConditionFound) {
    //TODO: "something" has to be done for a proper clean up - naim
    uiMgr->showError(6,"");
    return(false);
  }

//
// KLUDGE:  set socket buffer size to 64k to avoid write-write deadlock
//              between paradyn and paradynd
//
#if defined(sparc_sun_sunos4_1_3) || defined(hppa1_1_hp_hpux)
   int num_bytes =0;
   int size = sizeof(num_bytes);
   num_bytes = 32768;
   if(setsockopt(new_daemon->get_sock(),SOL_SOCKET,SO_SNDBUF,(char *)&num_bytes ,size) < 0){
      perror("setsocketopt SND_BUF error");
   }
#endif

  msg_bind_socket (new_daemon->get_sock(), true, (int(*)(void*)) xdrrec_eof,
		     (void*)new_daemon->net_obj(), &(new_daemon->stid));
  assert(new_daemon);

  // The pid is reported later in an upcall
  return (true);
}

//  TODO : fix this so that it really does clean up state
// Dispose of daemon state.
//    This is called because someone wants to kill a daemon, or the daemon
//       died and we need to cleanup after it.
//
void paradynDaemon::removeDaemon(paradynDaemon *d, bool informUser)
{

    if (informUser) {
      string msg;
      msg = string("paradynd has died on host <") + d->getMachineName()
	    + string(">");
      uiMgr->showError(5, P_strdup(msg.c_str()));
    }

    d->dead = true;

#ifdef notdef
    executable *e;
    List<executable*> progs;
    daemons.remove(d);

    //
    // Delete executables running on the dead paradyn daemon.
    //
    for (progs = programs; e = *progs; progs++) {
       if (e->controlPath == d) {
	   programs.remove(e);
	   delete(e);
       }
    }

#endif

    // tell the thread package to ignore the fd to the daemon.
    msg_unbind(d->getSocketTid());
}

// get the current time of a daemon, to adjust for clock differences.
//
//       DM'                FE                DM
//       ^
//       |  } skew' (neg)                                  
//       ------------------------------------------------------
//       |                  |                     } skew
//       |                  |                  ^
//       |                  |  t1              |
//       |                  |                  |  
//       | dt'              |  FEt             |  dt
//       |                  |                  |
//       
//       delay  =  dt' - t1  =  dt - t1   
//       
//                      (verify by looking at distances on graph)
//       FEt  =  dt' + skew'  =  t1 + delay  =  dt + skew  
//
//                      (by algebra)
//       skew  = t1 + delay - dt   =  t1 - dt + delay
//       skew' = t1 + delay - dt'

void paradynDaemon::calc_FE_DMN_Times(timeLength *networkDelay,
				      timeLength *timeAdjustment) {
  timeStamp t1 = getCurrentTime();
  timeStamp dt = timeStamp(getTime(), timeUnit::sec(), timeBase::bStd());
  timeStamp t2 = getCurrentTime();
  timeLength delay = (t2 - t1) / 2.0;
  if(networkDelay != NULL)
    *networkDelay = delay;
  if(timeAdjustment != NULL)
    *timeAdjustment = t1 - dt + delay;
}

// will update the time adjustment for this daemon with the average
// skew value
// returns the maximum network delay 
timeLength paradynDaemon::updateTimeAdjustment(const int samplesToTake) {
  sampleVal_cerr << "  updateTimeAdjustment (" << this << "):\n";
  timeLength totalSkew   = timeLength::Zero();
  timeLength maxNetDelay = timeLength::Zero();

  for(int i=0; i<samplesToTake; i++) {
    timeLength delay, skew;
    calc_FE_DMN_Times(&delay, &skew);
    totalSkew += skew;
    if(delay > maxNetDelay)
      maxNetDelay = delay;
    sampleVal_cerr << "    curSkew: " << skew << ", curDelay: " << delay 
		   << "\n";
  }
  timeLength avgSkew = totalSkew / samplesToTake;
  setTimeFactor(avgSkew);
  sampleVal_cerr << "**setting timeFactor: " << avgSkew << "\n";
  if(!getMaxNetworkDelay().isInitialized() || 
     maxNetDelay > getMaxNetworkDelay()) {
    setMaxNetworkDelay(maxNetDelay);
    sampleVal_cerr << "**setMaxNetDelay: " << maxNetDelay << "\n";
  }

  return maxNetDelay;
}


void
paradynDaemon::SendMDLFiles( void )
{
    pdvector<T_dyninstRPC::rawMDL> mdlBufs;
    pdvector<void*> mappedFileAddrs;
    pdvector<unsigned long> mappedFileLengths;

#if defined(i386_unknown_nt4_0)
    pdvector<HANDLE> mdl_fds;
    pdvector<HANDLE> mdl_maps;
#else
    pdvector<FILE*> mdl_fds;
#endif // defined(i386_unknown_nt4_0)


    for( pdvector<string>::const_iterator iter = mdl_files.begin();
            iter != mdl_files.end();
            iter++ )
    {
        string curFileName = *iter;

        //
        // map the current file into our address space
        //
#if defined(i386_unknown_nt4_0)
        HANDLE hFile = CreateFile( curFileName.c_str(),
                                    GENERIC_READ,
                                    FILE_SHARE_READ,
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_FLAG_SEQUENTIAL_SCAN,
                                    NULL );
        if( hFile == INVALID_HANDLE_VALUE )
        {
            fprintf( stderr, "FE: warning: failed to re-open MDL file \"%s\"\n",
                curFileName.c_str() );
            continue;
        }
        HANDLE hMap = CreateFileMapping( hFile,
                                            NULL,
                                            PAGE_READONLY,
                                            0, 0,
                                            NULL );
        if( hMap == NULL )
        {
            fprintf( stderr, "FE: warning: failed to create file mapping for MDL file \"%s\": %x\n",
                curFileName.c_str(), GetLastError() );
            continue;
        }
        mdl_maps.push_back( hMap );
        LPVOID mapAddr = MapViewOfFile( hMap,
                                        FILE_MAP_READ,
                                        0, 0,
                                        0 );
        if( mapAddr == NULL )
        {
            fprintf( stderr, "FE: warning: failed to map file into address space MDL file \"%s\": %x\n",
                curFileName.c_str(), GetLastError() );
            // warning - failed to map MDL file into address space
            continue;
        }
        mappedFileAddrs.push_back( mapAddr );
        DWORD mapLen = GetFileSize( hFile, NULL );
        mappedFileLengths.push_back( mapLen );

        string buf( (const char*)mapAddr, mapLen );
        mdlBufs.push_back( T_dyninstRPC::rawMDL( buf ) );

#else
        FILE* fp = fopen( curFileName.c_str(), "r" );
        if( fp == NULL )
        {
            // warning - we were unable to open an MDL file 
            // that we recently opened and read
            continue;
        }
        mdl_fds.push_back( fp );

        // determine file size
        struct stat fileInfo;
        if( fstat( fileno( fp ), &fileInfo ) == -1 )
        {
            cerr << "FE: failed to stat MDL file: "
                << errno
                << endl;
            continue;
        }

        // map the current file into our address space
        void* mapAddr = mmap( NULL,     // don't care where
                                fileInfo.st_size,     // entire file
                                PROT_READ,  // file protections
                                MAP_PRIVATE,
                                fileno( fp ),     // file to map
                                0 );    // start at the beginning
        if( mapAddr != NULL )
        {
            mappedFileAddrs.push_back( mapAddr );
            mappedFileLengths.push_back( fileInfo.st_size );

            string buf( (const char*)mapAddr, fileInfo.st_size );
            mdlBufs.push_back( T_dyninstRPC::rawMDL( buf ) );
        }
        else
        {
            cerr << "FE: failed to map MDL file into address space: "
                << errno
                << endl;
        }
#endif // defined(i386_unknown_nt4_0)
    }

    if( mdlBufs.size() > 0 )
    {
        // deliver the files' contents to our daemons
        send_mdl( mdlBufs );
    }
    else
    {
        cerr << "FE: failed to send any MDL files - none to send" << endl;
    }

    //
    // We're done with the MDL files - unmap and release them
    //
#if defined(i386_unknown_nt4_0)
    for( pdvector<void*>::const_iterator mapIter = mappedFileAddrs.begin();
                mapIter != mappedFileAddrs.end();
                mapIter++ )
    {
        UnmapViewOfFile( *mapIter );
    }
    for( pdvector<HANDLE>::const_iterator mappingIter = mdl_maps.begin();
            mappingIter != mdl_maps.end();
            mappingIter++ )
    {
        CloseHandle( *mappingIter );
    }
    for( pdvector<HANDLE>::const_iterator fileIter = mdl_fds.begin();
            fileIter != mdl_fds.end();
            fileIter++ )
    {
        CloseHandle( *fileIter );
    }
#else
    for( unsigned int i = 0; i < mdlBufs.size(); i++ )
    {
        munmap( mappedFileAddrs[i], mappedFileLengths[i] );
    }
    for( pdvector<FILE*>::const_iterator iter = mdl_fds.begin();
            iter != mdl_fds.end();
            iter++ )
    {
        fclose( *iter );
    }
#endif // defined(i386_unknown_nt4_0)
}



//
// add a new daemon
// check to see if a daemon that matches the function args exists
// if it does exist, return a pointer to it
// otherwise, create a new daemon
//
paradynDaemon *paradynDaemon::getDaemonHelper(const string &machine, 
					      const string &login, 
					      const string &name) {

   //cerr << "paradynDaemon::getDaemonHelper(machine=" << machine << ")\n";

    string m = machine; 
    // fill in machine name to default_host if empty
    if (!m.length()) {
        if (default_host.length()) {
            m = getNetworkName(default_host);
            //cerr << "Using default host <" << m << ">" << endl;
        } else {
            m = getNetworkName();
            //cerr << "Using local host <" << m << ">" << endl;
        }
    } else {
            m = getNetworkName(m);
            //cerr << "Using given host <" << m << ">" << endl;
    }

    // find out if we have a paradynd on this machine+login+paradynd
    paradynDaemon *pd;
    bool foundSimilarDaemon = false;
    for(unsigned i = 0; i < paradynDaemon::allDaemons.size(); i++){
        pd = paradynDaemon::allDaemons[i];
        if ((!m.c_str() || (pd->machine == m)) && 
           (!login.c_str() || (pd->login == login))) {
           if((name.c_str() && (pd->name == name))) {
              //cerr << "Returning an existing daemon match!" << endl;
              return (pd);     
           } else 
              foundSimilarDaemon = true;
        }
    }

    if(foundSimilarDaemon) {
       cerr << "Warning, found an existing daemon on requested machine \"" << m
            << "\" and with requested login \"" << login << "\", but the"
            << " requested daemon flavor \"" << name << "\" doesn't match." 
            << endl;
    }

    // find a matching entry in the dictionary, and start it
    daemonEntry *def = findEntry(m, name);
    if (!def) {
	if (name.length()) {
	  string msg = string("Paradyn daemon \"") + name + string("\" not defined.");
	  uiMgr->showError(90,P_strdup(msg.c_str()));
        }
	else {
	  uiMgr->showError(91,"");
        }
	return ((paradynDaemon*) 0);
    }

    char statusLine[256];
    sprintf(statusLine, "Starting daemon on %s",m.c_str());
    uiMgr->updateStatusLine(DMstatus.c_str(),P_strdup(statusLine));

    string flav_arg(string("-z") + def->getFlavor());
    unsigned asize = paradynDaemon::args.size();
    paradynDaemon::args += flav_arg;

    pd = new paradynDaemon(m, login, def->getCommandString(), 
			   def->getRemoteShellString(), def->getNameString(),
			   def->getFlavorString());

    if (pd->errorConditionFound) {
      //TODO: "something" has to be done for a proper clean up - naim
      string msg;
      msg=string("Cannot create daemon process on host \"") + m + string("\"");
      uiMgr->showError(84,P_strdup(msg.c_str())); 
      return((paradynDaemon*) 0);
    }

#if defined(sparc_sun_sunos4_1_3) || defined(hppa1_1_hp_hpux)
   int num_bytes =0;
   int nb_size = sizeof(num_bytes);
   num_bytes = 32768;
   if(setsockopt(pd->get_desc(),SOL_SOCKET,SO_SNDBUF,(char *)&num_bytes ,nb_size) < 0){
      perror("setsocketopt SND_BUF error");
   }
#endif

    paradynDaemon::args.resize(asize);
    uiMgr->updateStatusLine(DMstatus.c_str(),P_strdup("ready"));

    if (pd->get_sock() == PDSOCKET_ERROR) {
        uiMgr->showError (6, "");
        return((paradynDaemon*) 0);
    }

   // Send the initial metrics, constraints, and other neato things
   pd->SendMDLFiles();
   // Send the initial metrics, constraints, and other neato things
   pdvector<T_dyninstRPC::metricInfo> info = pd->getAvailableMetrics();
   unsigned size = info.size();
   for (unsigned u=0; u<size; u++)
	addMetric(info[u]);

   
    pd->updateTimeAdjustment();

    msg_bind_socket(pd->get_sock(), true, (int(*)(void*))xdrrec_eof,
		     (void*) pd->net_obj(), &(pd->stid));

    return (pd);
}

//  
// add a new daemon, unless a daemon is already running on that machine
// with the same machine, login, and program
//
bool paradynDaemon::getDaemon (const string &machine, 
			       const string &login,
			       const string &name){

    if (!getDaemonHelper(machine, login, name)){
        return false;
    }
    return true;
}

//
// define a new entry for the daemon dictionary, or change an existing entry
//
bool paradynDaemon::defineDaemon (const char *c, const char *d,
				  const char *l, const char *n,
				  const char *m, const char *r, const char *f) {

  if(!n || !c)
      return false;
  string command = c;
  string dir = d;
  string login = l;
  string name = n;
  string machine = m;
  string remote_shell = r;
  string flavor = f;

  daemonEntry *newE;
  for(unsigned i=0; i < allEntries.size(); i++){
      newE = allEntries[i];
      if(newE->getNameString() == name){
          if (newE->setAll(machine, command, name, login, dir, remote_shell, flavor))
              return true;
          else 
              return false;
      }
  }

  newE = new daemonEntry(machine, command, name, login, dir, remote_shell, flavor);
  if (!newE)
      return false;
  allEntries += newE;
  return true;
}


daemonEntry *paradynDaemon::findEntry(const string &, 
				      const string &n) {

    // if (!n) return ((daemonEntry*) 0);
    for(unsigned i=0; i < allEntries.size(); i++){
        daemonEntry *newE = allEntries[i];
        if(newE->getNameString() == n){
	   return(newE);
        }
    }
    return ((daemonEntry*) 0);

}

void paradynDaemon::tellDaemonsOfResource(u_int parent, u_int my_id, 
					  const char *name, unsigned type) {
    paradynDaemon *pd;
    for(unsigned i = 0; i < paradynDaemon::allDaemons.size(); i++){
        pd = paradynDaemon::allDaemons[i];
	pd->addResource(parent,my_id,name, type);
    }

}

void paradynDaemon::printEntries()
{
    daemonEntry *entry;
    for(unsigned i=0; i < allEntries.size(); i++){
        entry = allEntries[i];
	entry->print();
    }
}

void paradynDaemon::print() 
{
  cout << "DAEMON\n";
  cout << "  name: " << name << endl;
  cout << "  command: " << command << endl;
  cout << "  login: " << login << endl;
  cout << "  machine: " << machine << endl;
  cout << "  flavor: " << flavor << endl;
}

void paradynDaemon::printDaemons()
{
    paradynDaemon *pd;
  cout << "ACTIVE DAEMONS\n";
    for(unsigned i = 0; i < paradynDaemon::allDaemons.size(); i++){
        pd = paradynDaemon::allDaemons[i];
	pd->print();
    }
}

void paradynDaemon::printPrograms()
{
    executable *entry;
    for(unsigned i=0; i < programs.size(); i++){
	entry = programs[i];
	cout << "PROGRAM ENTRY\n";
	cout << "pid : " << entry->pid << endl;
	entry->controlPath->print();
    }
}

//
// Return list of names of defined daemons.  
//
pdvector<string> *paradynDaemon::getAvailableDaemons()
{
    pdvector<string> *names = new pdvector<string>;

    daemonEntry *entry;
    for(unsigned i=0; i < allEntries.size(); i++){
        entry = allEntries[i];
	*names += entry->getName();
    }
    return(names);
}

// For a given machine name, find the appropriate paradynd structure(s).
pdvector<paradynDaemon*> paradynDaemon::machineName2Daemon(const string &mach) {
   pdvector<paradynDaemon*> v;
   for (unsigned i=0; i < allDaemons.size(); i++) {
      paradynDaemon *theDaemon = allDaemons[i];
      if (theDaemon->getMachineName() == mach)
        v += theDaemon;
   }
   return v;
}

static bool hostIsLocal(const string &machine)
{
	return (machine.length() == 0 || machine == "localhost" || 
		getNetworkName(machine) == getNetworkName() ||
		getNetworkAddr(machine) == getNetworkAddr());
}

static bool execPOE(const string /* &machine*/, const string /* &login */,
                    const string /* &name   */, const string         &dir,
                    const pdvector<string> &argv, const pdvector<string>  args,
                    daemonEntry          *de)
{
	unsigned i;


  char **s = (char**) malloc(sizeof(char*) * (args.size() + argv.size() + 5));
  char   t[1024];
  assert(s != NULL);

  s[0] = strdup("poe");
  s[1] = strdup(de->getCommand());

  for (i = 0; i < args.size(); i++)
    s[i+2] = (strcmp(args[i].c_str(), "-l1")) 
             ? strdup(args[i].c_str()) : strdup("-l0");

  sprintf(t, "-z%s", de->getFlavor());
  s[args.size()+2] = strdup(t);
  s[args.size()+3] = strdup("-runme");
      
  for (i = 0; i < argv.size(); i++) 
    s[args.size()+4+i] = (i || strcmp(argv[i].c_str(), "poe"))
                         ? strdup(argv[i].c_str()) : strdup("");

  s[args.size()+argv.size()+4] = NULL;

                      //IBM POE sets remote directory same as current directory
  if (dir.length() && (P_chdir(dir.c_str()) < 0)) 
    cerr << "cannot chdir to " << dir.c_str() << ": " 
         << sys_errlist[errno] << endl;

  int execRetVal = execvp(s[0], s);

  // Close Tk X connection to avoid conflicts with parent
  uiMgr->CloseTkConnection();

  if ( execRetVal == -1 )
  {
    if ( errno == ENOENT )
      cerr << "Could not find executable." << endl;
    else
      cerr << "Could not start MPI on local host." << endl;
  }
  
  P__exit(-1);
  return(false);
}


static bool rshPOE(const string         &machine, const string         &login,
                   const string      /* &name*/,  const string         &dir,
                   const pdvector<string> &argv,    const pdvector<string>  args,
                   daemonEntry          *de)
{
  unsigned i;
  char *s[6];
  char  t[1024];
  string appPath;
  string pathResult;
  string path = getenv("PATH");

  string remoteShell = de->getRemoteShellString();
  assert(remoteShell.length() > 0);
  s[0] = strdup(remoteShell.c_str());

  s[1] = strdup(machine.c_str());
  s[2] = (login.length()) ? strdup("-l"):              strdup("");
  s[3] = (login.length()) ? strdup(login.c_str()): strdup("");

  sprintf(t, "(");

  if (dir.length()) strcat(strcat(strcat(t, "cd "), dir.c_str()), "; ");

  strcat(strcat(strcat(t, "poe "), de->getCommand()), " ");

  for (i = 0; i < args.size(); i++)
    strcat(strcat(t, (strcmp(args[i].c_str(), "-l1")) 
                     ? args[i].c_str() : "-l0"), " ");

  strcat(strcat(strcat(t, "-z"), de->getFlavor()), " -runme ");
      
  for (i = 0; i < argv.size(); i++) 
    strcat(strcat(t, (i || strcmp(argv[i].c_str(), "poe"))
                     ? argv[i].c_str() : ""), " ");

  strcat(t, ")");

  s[4] = strdup(t);
  s[5] = NULL;

  int execRetVal = execvp(s[0], s);

  // Close Tk X connection to avoid conflicts with parent
  uiMgr->CloseTkConnection();

  if ( execRetVal == -1 )
  {
    if ( errno == ENOENT )
      cerr << "Could not find executable to start remote shell." << endl;
    else
      cerr << "Could not start MPI on remote host." << endl;
  }
  
  P__exit(-1);
  return(false);
}


static bool startPOE(const string         &machine, const string         &login,
                     const string         &name,    const string         &dir,
                     const pdvector<string> &argv,    const pdvector<string>  args,
		     daemonEntry          *de)
{
#if !defined(i386_unknown_nt4_0)
    uiMgr->updateStatusLine(DMstatus.c_str(),   "ready");

  if (fork()) return(true);

  if (hostIsLocal(machine))
    return(execPOE(machine, login, name, dir, argv, args, de));
  else
    return( rshPOE(machine, login, name, dir, argv, args, de));
#else // !defined(i386_unknown_nt4_0)
	// TODO - implement this?
	return false;
#endif // !defined(i386_unknown_nt4_0)
}


bool getIrixMPICommandLine(const pdvector<string> argv, const pdvector<string> args,
              daemonEntry* de, pdvector<string>& cmdLineVec, string& programNames)
{
  //  This parsing implemented for mpirun IRIX 6.5, SGI MPI 3.2.0.0

  typedef struct arg_entry {
        
    char* argFlag;
    bool hasValue;
    bool definedBehavior;
    bool supported;
    
  } ArgEntry;
                
  const unsigned int globalArgNo = 14;
        
  ArgEntry globalArgs[globalArgNo] =
  {
    {  "-a", true, true, true },
    {  "-array", true, true, true },
    {  "-cpr", false, false, true },
    {  "-d", true, true, true },
    {  "-dir", true, true, true },
    {  "-f", true, false, true },
    {  "-file", true, false, true },
    {  "-h", false, false, false },
    {  "-help", false, false, false },
    {  "-miser", false, true, true },
    {  "-p", true, true, true },
    {  "-prefix", true, true, true },
    {  "-v", false, true, true },
    {  "-verbose", false, true, true }
  };
        
        
  const unsigned int localArgNo = 4;
        
  ArgEntry localArgs[localArgNo] =
  {
    {  "-f", true, false, true },
    {  "-file", true, false, true },
    {  "-np", true, true, true },
    {  "-nt", true, true, true }
  };

  unsigned int i = 0, j = 0;
  pdvector<string> progNameVec;
  
  //  parse past mpirun
  bool mpirunFound = false;
  do 
  {
    cmdLineVec += argv[i];
    
    mpirunFound = (P_strstr(argv[i].c_str(), "mpirun") > 0);
    i++;
  }
  while ( !mpirunFound && i < argv.size());

  if ( !mpirunFound )
  {
    uiMgr->showError(113, "Unable to find mpirun command.");
    return false;
  }
  
  //  parse global options
  bool flagFound = true;

  while ( i < argv.size() && flagFound )
  {
    flagFound = false;

    for ( j = 0; j < globalArgNo && !flagFound; j++ )
    {
      if ( P_strcmp(argv[i].c_str(), globalArgs[j].argFlag) == 0 )
      {
        if ( !globalArgs[j].supported )
        {
          string message = "Paradyn does not support use of mpirun flag ";
          message += globalArgs[j].argFlag;
          uiMgr->showError(113, P_strdup(message.c_str()));
          
          return false;
        }
        
        if ( !globalArgs[j].definedBehavior )
        {
          string message = "Behavior undefined with mpirun option ";
          message += globalArgs[j].argFlag;
          uiMgr->showError(114, P_strdup(message.c_str()));
        }
        
        flagFound = true;
        cmdLineVec += argv[i];
        i++;

        if ( globalArgs[j].hasValue )
        {
          cmdLineVec += argv[i];
          i++;
        }
      }
    }
  }

  while ( i < argv.size() )
  {
    //  We have reached the first entry
    //  The following arguments could be host list, process count, application,
    //    or application argument

    //  Is the argument of the first entry a number?
    //  If so, assume that this is a process count and not
    //    a hostname

    bool isNumber = true;
    unsigned int len;
    
    len = argv[i].length();

    for ( j = 0; j < len && isNumber; j++ )
    {
      if ( !isdigit(argv[i][j]) )
        isNumber = false;
    }

    if ( isNumber )  
    {
      //  this must be a process count
      cmdLineVec += argv[i];
      i++;
    }

    if ( !isNumber )
    {
      //  If not a number, then this argument can either be
      //  a local flag or a hostname.  If it is a local flag,
      //  then there is no hostlist for this entry (it is for
      //  the local host).

      // are there any local options?
      //  This can consist of:
      //    -f[ile] filename
      //    -np number
      //    -nt number
        
      bool flagFound = false;

      for ( j = 0; j < localArgNo && !flagFound; j++ )
      {
        if ( strcmp(argv[i].c_str(), localArgs[j].argFlag) == 0 )
        {
          if ( !localArgs[j].supported )
          {
            string message = "Paradyn does not support use of mpirun flag ";
            message += globalArgs[j].argFlag;
            uiMgr->showError(113, P_strdup(message.c_str()));
            return false;
          }
          
          if ( !localArgs[j].definedBehavior )
          {
            string message = "Behavior undefined with mpirun option ";
            message += globalArgs[j].argFlag;
            uiMgr->showError(114, P_strdup(message.c_str()));
          }

          flagFound = true;
          
          cmdLineVec += argv[i];
          i++;
        
          if ( localArgs[j].hasValue )
          {
            cmdLineVec += argv[i];
            i++;
          }
        }
      }
        
      //  If the first arg in the entry is neither a number or a local flag,
      //  then this must be a hostlist.
      //    hostlist can be constructed in the following ways:
      //      "hostname1," "hostname2"
      //      "hostname1" ",hostname2"
      //      "hostname1" "," "hostname2"
      //      "hostname1,hostname2"

      if ( !flagFound )
      {
        bool inHostList = false;
        char* commaPtr;
        do
        {
          // If curr arg contains ",", then it is a hostname
          // If last char is ',' or next arg starts with ',', then still in hostlist
          commaPtr = strrchr(argv[i].c_str(), ',');

          if ( (commaPtr && argv[i][argv[i].length()-1] == ',') ||
               ( (i+1) < argv.size() && argv[i+1][0] == ',') )
          {
            inHostList = true;
          }
          else
            inHostList = false;

          cmdLineVec += argv[i];
          i++;
        
        } while ( inHostList );

        // If there aren't anymore args, the command is invalid!
        if ( i >= argv.size() )
        {
            uiMgr->showError(113, "Unable to parse mpirun command line.");
            return false;
        }
        
        // Now redo the number/flag-checking
        isNumber = true;
        len = argv[i].length();

        for ( j = 0; j < len; j++ )
        {
          if ( !isdigit(argv[i][j]) )
            isNumber = false;
        }

        if ( isNumber )  
        {
          //  this must be a process count
          cmdLineVec += argv[i];
          i++;
        }
        else
        {
          flagFound = false;
          
          for ( j = 0; j < localArgNo && !flagFound; j++ )
          {
            if ( strcmp(argv[i].c_str(), localArgs[j].argFlag) == 0 )
            {
              if ( !localArgs[j].supported )
              {
                string message = "Paradyn does not support use of mpirun flag ";
                message += globalArgs[j].argFlag;
                uiMgr->showError(113, P_strdup(message.c_str()));
                return false;
              }
              
              flagFound = true;
              cmdLineVec += argv[i];
              i++;
        
              if ( localArgs[j].hasValue )
              {
                cmdLineVec += argv[i];
                i++;
              }
            }
          }

          if ( !flagFound )  // No process count!  Error parsing command.
          {
            uiMgr->showError(113, "Unable to parse mpirun command line.");
            return false;
          }
        }
      }
    }

    // If there aren't anymore args, the command is invalid!
    if ( i >= argv.size() )
    {
      uiMgr->showError(113, "Unable to determine executable in mpirun command line.");
      return false;
    }

    //  the current argument is an application, so insert daemon command & args
    cmdLineVec += de->getCommand();
    
    for (j = 0; j < args.size(); j++)
      cmdLineVec += strcmp(args[j].c_str(), "-l1") ? args[j] : "-l0";

    cmdLineVec += string("-z") + de->getFlavor();
    cmdLineVec += "-runme";
    
    bool progNameFound = false;
    for ( j = 0; j < progNameVec.size(); j++ )
    {
      if ( progNameVec[j] == argv[i] )
        progNameFound = true;
    }

    if ( !progNameFound )
      progNameVec += argv[i];

    cmdLineVec += argv[i];
    
    i++;

    //  Go past program arguments to find the next entry
    while ( i < argv.size() && strcmp(argv[i].c_str(), ":") )
    {
      cmdLineVec += argv[i];
      i++;
    }

    if ( i < argv.size() && !strcmp(argv[i].c_str(), ":") )
    {
      cmdLineVec += argv[i];
      i++;
    }
  }

  for ( j = 0; j < progNameVec.size(); j++ )
  {
    if ( programNames.length() )
      programNames += ", ";
    programNames += progNameVec[j];
  }

  return true;
}


static bool execIrixMPI(const string &dir, pdvector<string>& cmdLineVec)
{
  unsigned int j;
  
  const int cmdLineVecSize = 1000;
  char **s = new char*[cmdLineVecSize]();

  for ( j = 0; j < cmdLineVec.size(); j++ ) {
    s[j] = P_strdup(cmdLineVec[j].c_str());
  }

  s[j] = NULL;
  
  if (dir.length()) {
      // mpirun cds to "current directory" based on PWD environment variable
      const int dirLength = 300;
      char* buf = new char[dirLength]();
      sprintf(buf, "PWD=%s",dir.c_str());
      putenv(buf);
  }

  int execRetVal = execvp(s[0], s);

  // Close Tk X connection to avoid conflicts with parent
  uiMgr->CloseTkConnection();

  if ( execRetVal == -1 )
  {
    if ( errno == ENOENT )
      cerr << "Could not find executable." << endl;
    else
      cerr << "Could not start MPI on local host." << endl;
  }
  
  P__exit(-1);
  return(false);
}


static bool rshIrixMPI(const string &machine, const string &login,
                       const string &dir, daemonEntry* de,
                       pdvector<string>& cmdLineVec)
{
  char *s[6];
  char  t[1024];
  unsigned int i;
  
  //  create rsh command
  string remoteShell = de->getRemoteShellString();
  assert(remoteShell.length() > 0);
  s[0] = strdup(remoteShell.c_str());

  s[1] = strdup(machine.c_str());
  s[2] = (login.length()) ? strdup("-l"):              strdup("");
  s[3] = (login.length()) ? strdup(login.c_str()): strdup("");

  strcpy(t, "(");

  if (dir.length())
  {
    strcat(t, "cd ");
    strcat(t, dir.c_str());
    strcat(t, "; ");
  }

  for ( i = 0; i < cmdLineVec.size(); i++ )
  {
    strcat(t, cmdLineVec[i].c_str());
    strcat(t, " ");
  }
  
  strcat(t, ")");

  s[4] = strdup(t);
  s[5] = NULL;

  int execRetVal = execvp(s[0], s);

  // Close Tk X connection to avoid conflicts with parent
  uiMgr->CloseTkConnection();

  if ( execRetVal == -1 )
  {
    if ( errno == ENOENT )
      cerr << "Could not find executable to start remote shell." << endl;
    else
      cerr << "Could not start MPI on remote host." << endl;
  }
  
  P__exit(-1);
  return(false);
}


static bool startIrixMPI(const string     &machine, const string         &login,
                     const string         &name,    const string         &dir,
                     const pdvector<string> &argv,    const pdvector<string>  args,
		     daemonEntry          *de)
{
#if !defined(i386_unknown_nt4_0)
    uiMgr->updateStatusLine(DMstatus.c_str(),   "ready");

   string programNames;
   pdvector<string> cmdLineVec;
   
   if ( !getIrixMPICommandLine(argv, args, de, cmdLineVec, programNames) )
     return false;
   else
   {
      string newStatus;
      newStatus = string("program: ") + programNames + " ";
      newStatus += string("machine: ");
      if ( machine.length() )
         newStatus += machine;
      else
         newStatus += "(local_host)";
      newStatus += " ";
      newStatus += string("user: ");
      if ( login.length() )
         newStatus += login;
      else
         newStatus += "(self)";
      newStatus += " ";
      newStatus += string("daemon: ") + name + " ";

      extern status_line* app_name;
      uiMgr->updateStatus(app_name, P_strdup(newStatus.c_str()));
   }
   
   if (fork()) return(true);

   if (hostIsLocal(machine))
      return(execIrixMPI(dir, cmdLineVec));
   else
      return(rshIrixMPI(machine, login, dir, de, cmdLineVec));
#else // !defined(i386_unknown_nt4_0)
   // TODO - implement this?
   return false;
#endif // !defined(i386_unknown_nt4_0)
}

/*
  Pick a unique name for the wrapper
*/
string mpichNameWrapper( const string& dir )
{
   string rv;
	

   // We need to put the wrapper in a file that is (a) accessible
   // on every machine that will participate in the MPI job and 
   // (b) is unique on each of those machines
   //
   // Whether we are creating this file on the local or remote machine,
   // our format for the wrapper filename is
   //
   //   pdd-<name>-<pid>-<usec>
   //
   // where:
   //   <name> is the network name of this machine (the one running the
   //     front end process
   //   <pid> is the process ID of the front-end process on that machine
   //   <usec> is the result of gettimeofday() expressed in microseconds
   // 
   rv += dir;
   rv += "/";
   rv += "pdd-";
   rv += getNetworkName();
   rv += "-";
   rv += string(getpid());
   rv += "-";

   struct timeval tv;
   gettimeofday( &tv, NULL );
   rv += string(tv.tv_sec * 1000000 + tv.tv_usec);
   
   return rv;
}


#if !defined(i386_unknown_nt4_0)

/*
  Perform a cleanup when the frontend exits
*/
bool
mpichUnlinkWrappers()
{
   bool rv=true;
   for (unsigned int i=0; i< paradynDaemon::wrappers.size(); i++) {
      paradynDaemon::MPICHWrapperInfo& wrapper = paradynDaemon::wrappers[i];
      
      if(wrapper.isLocal) {
	 // remove the local wrapper file
	 if (remove(wrapper.filename.c_str()) < 0) { // Best effort
	    cerr << "mpichUnlinkWrappers: Failed to remove " 
		 << wrapper.filename << ": " << strerror(errno) << endl;
	    rv=false;
	 }
      } else {
	 // remove the remote wrapper file
	 bool removeFailed = false;
	 int pid = fork();
	 if(pid > 0) {
	    // we're the parent - wait for our child's remove command to
	    // finish
	    int rmStatus;
	    if( (waitpid( pid, &rmStatus, 0 ) != pid) ||
		(rmStatus != 0) ) {
	       removeFailed = true;
	       rv=false;
	    }
	 } else if(pid == 0) {
	    // we're the child -
	    // exec the remote command to remove the wrapper file
	    string cmd;
	    
	    cmd += wrapper.remoteShell;
	    cmd += " ";
	    cmd += wrapper.remoteMachine;
	    cmd += " /bin/rm ";
	    cmd += wrapper.filename;
	    execl("/bin/sh", "sh", "-c", cmd.c_str(), NULL );
	    
	    // if we reached here, our exec failed
	    removeFailed = true;
	 }
	 else {
	    // our fork failed
	    removeFailed = true;
	    rv=false;
	 }
	 
	 if(removeFailed) {
	    cerr << "mpichUnlinkWrappers: Failed to remove " 
		 << wrapper.filename << ": " << strerror(errno) << endl;
	 }
      }
   }
   return rv;
}


bool writeMPICHWrapper(int fd, const string& buffer )
{
    if (lseek(fd, 0, SEEK_SET) == (off_t)-1) {
        perror("Failed to write MPI wrapper (seek)");
        return false;
    }
    if (write(fd, buffer.c_str(), strlen(buffer.c_str())) !=
        strlen(buffer.c_str())) {
        perror("Failed to write MPI wrapper (write)");
        return false;
    }
    if (fchmod(fd, S_IRWXU | S_IRWXG | S_IRWXO) == -1) {
        perror("Failed to write MPI wrapper (chmod)");
        return false;
    }
    if (close(fd) == -1) {
        perror("Failed to write MPI wrapper (close)");
        return false;
    }
   return true;
}


/*
  Create a script file which will start paradynd with appropriate
  parameters in a given directory. MPICH hides the user cmdline from
  slave processes until the MPI_Init() moment, so we pass these parameters
  in a script

  If successful, adds an entry to paradynDaemon::wrappers.
*/
bool mpichCreateWrapper(const string& machine, bool localMachine,
			const string& script, const char *dir,
			const string& app_name, const pdvector<string> args,
			daemonEntry *de,
                        bool has_explicit_wd)
{
    const char* shellspec = "#!/bin/sh\n";
    string buffer;
    unsigned int j;

    assert( (dir != NULL) && (strlen(dir) != 0) );
    
    // dump the shell specifier
    buffer = string(shellspec);

    // Set up for using the correct working directory for the process.
    // We have to be careful to respect the user's desire if
    // the user has explicitly specified a working directory for the process
    // If they have, we recognized it when we parsed the command line
    // and we don't mess with the process arguments as given by mpirun.
    // If they haven't, we need to modify the "working directory" argument
    // specified by mpirun to ensure it is the directory specified by
    // the user via the Paradyn PCL file or the Paradyn GUI.
    buffer += (string("cd ") + string(dir) + 
                string("\nPWD=") + string(dir) + string("; export PWD\n"));
    if( !has_explicit_wd )
    {
        // The user didn't specify a working directory in the mpirun
        // command, so we need to munge the MPI process arguments to use the
        // directory given via Paradyn.
        // This method works for the non-MPD MPICH device, which uses
        // the p4wd specifier to give its working directory.
        // TODO how does MPD specify its working directory?
        buffer += (string("substdir=") + string(dir) + "\n");
        buffer += string("appargs=`echo $* | sed \"s,p4wd [ ]*[^ ]*,p4wd $substdir,\"`\n");
    }
    else
    {
        buffer += string("appargs=$*\n");
    }

    // dump the daemon command
    buffer += string(de->getCommand());
    for (j=0; j < args.size(); j++) {
        if (!strcmp(args[j].c_str(), "-l1")) {
            buffer += " -l0";
        } else {
            buffer += string(" ") + args[j];
        }
    }
    
    buffer += (string(" -z") + string(de->getFlavor()));

    // Next add the arguments that define the process to be created.
    buffer += (string(" -runme ") + app_name + string(" $appargs\n"));
    
    if(localMachine) {
        // the file named by script is our wrapper - write the script to the
        // wrapper file
        int fd = open(script.c_str(), O_WRONLY | O_CREAT);
        if( !writeMPICHWrapper(fd, buffer)) {
            return false;
        }      
        paradynDaemon::wrappers += paradynDaemon::MPICHWrapperInfo(script);
        return true;
   }
   

   assert(! localMachine);
   // the file named by script is the wrapper's remote name - we write the
   // script to a local temporary file so as to copy it to the remote system
   char templ[40] = "pd.wrapper.XXXXXX";
   int tempfd = mkstemp(templ);
   // mkstemp also writes the name into temp
   string localWrapper = templ;   
   if( !writeMPICHWrapper(tempfd, buffer)) {
      return false;
   }
   
   // try to copy the file to the desired location on the remote system
   // ideally, we'd use execCmd() to execute this copy command on the
   // local system, but execCmd has some side effects we'd rather not try
   // to work around
   bool copySucceeded = true;
   int pid = fork();
   if(pid > 0) {
      // we are the parent - 
      // wait for our child process (which will do the copy) to complete
       int copyStatus;
       if(waitpid( pid, &copyStatus, 0 ) != pid ) {
           cerr << "mpichCreateWrapper: Failed to copy temporary local "
                << "wrapper to remote system: " 
                << strerror(errno) << endl;
           copySucceeded = false;
       }
       else {
           if(copyStatus != 0) {
               copySucceeded = false;
           }
       }
   } else if(pid == 0) {
       // we are the child - we exec our copy command...
       
       // ...build the command string...
       string cmd;
       cmd += de->getRemoteShellString();
       cmd += " ";
       cmd += machine;		// name of the remote system
       cmd += " cat - \">\" ";
       cmd += script;			// name of wrapper on remote system
       cmd += " \";\" chmod 755 ";
       cmd += script;
       cmd += " < ";
       cmd += localWrapper;	// name of wrapper on local system
       
       // ...exec a shell to execute it
       execl("/bin/sh", "sh", "-c", cmd.c_str(), NULL);
       copySucceeded = false;	// if we got here, the exec failed
   } else {
       cerr << "mpichCreateWrapper: fork failed." << strerror(errno) << endl;
       copySucceeded = false;
   }
   
   // release the local temporary wrapper file
   if(remove(localWrapper.c_str()) < 0) {
      cerr << "mpichCreateWrapper: Failed to remove temporary local wrapper"
	   << " " << localWrapper << ": " << strerror(errno) << endl;
   }
   
   if(copySucceeded) {
      // keep info around till later about script that needs to be removed
      paradynDaemon::wrappers += paradynDaemon::MPICHWrapperInfo(script, 
					  machine, de->getRemoteShellString());
   }

   if(!copySucceeded) {
      return false;
   }

   return true;
}

extern void appendParsedString(pdvector<string> &strList, const string &str);

/*
  Handle remote startup case
*/
void mpichRemote(const string &machine, const string &login, 
		 const char *cwd, daemonEntry *de, 
		 pdvector<string> &params)
{
	string rsh = de->getRemoteShellString();
	assert(rsh.length() > 0);
	appendParsedString(params, rsh);

	if (login.length() != 0) {
		params += string("-l");
		params += login;
	}
    if( machine.length() != 0 )
    {
        params += machine;
    }
    else
    {
        params += "localhost";
    }
	params += string("cd");
	params += string(cwd);
	params += string(";");
}
	
struct known_arguments {
	char* name;
	bool has_value;
	bool supported;
};

/*
  Split the command line into three parts:
  (mpirun arguments, the application name, application parameters)
  Insert the script name instead of the application name
*/
bool mpichParseCmdline(const string& script, const pdvector<string> &argv,
		       string& app_name,
                        pdvector<string> &params,
                        bool& has_explicit_wd)
{
   const unsigned int NKEYS = 35;
   struct known_arguments known[NKEYS] = {
      {"-arch", true, true},           {"-h", false, false},
      {"-machine", true, true},        {"-machinefile", true, true},
      {"-np", true, true},             {"-nolocal", false, true},
      {"-stdin", true, true},          {"-t", false, false},
      {"-v", false, true},             {"-dbx", false, false},
      {"-gdb", false, false},          {"-xxgdb", false, false},
      {"-tv", false, false},           {"-batch", true, true},
      {"-stdout", true, true},         {"-stderr", true, true},
      {"-nexuspg", true, true},        {"-nexusdb", true, true},
      {"-e", false, true},             {"-pg", false, true},
      {"-leave_pg", false, true},      {"-p4pg", true, false},
      {"-tcppg", true, false},         {"-p4ssport", true, true},
      {"-p4wd", true, true},
      {"-mvhome", false, false},       {"-mvback", true, false},
      {"-maxtime", true, true},        {"-nopoll", false, true},
      {"-mem", true, true},            {"-cpu", true, true},
      {"-cac", true, true},            {"-paragontype", true, true},
      {"-paragonname", true, true},    {"-paragonpn", true, true}
   };

   bool app = false, found_mpirun = false;
   unsigned int i = 0, k;

   while (i < argv.size() && !found_mpirun) {
      found_mpirun = (strstr(argv[i].c_str(), "mpirun") != 0);
      params += argv[i++];
   }

   if (!found_mpirun) {
      uiMgr->showError(113, "Expected: \"mpirun <command>\"");
      return false;
   }

   has_explicit_wd = false;
   while (!app && i < argv.size()) {
      app = true;
      
      for (k=0; k<NKEYS; k++) {
	 if (argv[i] == known[k].name) {	    
	    app = false;
	    
	    if (!known[k].supported) {
	       string msg = string("Argument \"") + string(known[k].name) + 
		            string("\" is not supported");
	       uiMgr->showError(113, strdup(msg.c_str()));
	       return false;
	    }

        // check whether the user explicitly specified a working directory
        if( (argv[i] == "-p4wd") )
        {
            has_explicit_wd = true;
        }
	    
	    params += argv[i++];
	    
	    if (known[k].has_value) {
	       // Skip the next arg
	       if (i >= argv.size()) {
		  uiMgr->showError(113, "MPICH command line parse error");
		  return false;
	       }
	       params += argv[i++];
	    }
	    break;
	 }
      }
   }

   if (!app) {
      uiMgr->showError(113, "MPICH command line parse error");
      return false;
   }

   params += script;
   app_name = argv[i++];
   
   for (; i < argv.size(); i++) {
      params += argv[i];
   }
   
   return true;
}

/*
  Initiate the MPICH startup process: start the master application
  under paradynd
*/
static bool startMPICH(const string &machine, const string &login,
		       const string &/*name*/, const string &dir,
		       const pdvector<string> &argv, const pdvector<string> &args,
		       daemonEntry *de)
{
	string app_name;
	pdvector<string> params;
	unsigned int i;
	char cwd[PATH_MAX];
	char **s;
	bool localMachine = hostIsLocal(machine);

    uiMgr->updateStatusLine(DMstatus.c_str(),   "ready");
   
	if (dir.length()) {
	   strcpy(cwd, dir.c_str());
	} else {
	   getcwd(cwd, PATH_MAX);
	}

    if( !localMachine || (login.length() > 0) )
    {
        // run the mpirun command via rsh/ssh
        // either because target host is remote, or
        // because target process' user is likely not us
        //
        // Note that this is conservative: the user may have
        // specified the same login as the one used to create
        // the front-end process.  We expect the common case
        // to be that the user explicitly specifies a user name
        // iff it is different from the one they used to start
        // the front end.
        mpichRemote(machine, login, cwd, de, params);
	}

	string script = mpichNameWrapper( cwd );

        bool has_explicit_wd = false;
        bool parseRet = mpichParseCmdline(script,
                                        argv,
                                        app_name,
                                        params,
                                        has_explicit_wd);
	if (!parseRet) {
	   return false;
	}

        bool createRet = mpichCreateWrapper(machine,
                                            localMachine,
                                            script,
                                            cwd,
                                            app_name, 
				            args,
                                            de,
                                            has_explicit_wd);
	if (!createRet) {
	   return false;
	}

	if ((s = (char **)malloc((params.size() + 1) * sizeof(char *))) == 0) {
	   uiMgr->showError(113, "Out of memory");
	   return false;
	}

	for (i=0; i<params.size(); i++) {
	   s[i] = strdup(params[i].c_str());
	}
	s[i] = 0;


	if (fork()) {
	   return true;
	}


	// Close Tk X connection to avoid conflicts with parent
	uiMgr->CloseTkConnection();

	if (execvp(s[0], s) < 0) {
	   uiMgr->showError(113, "Failed to start MPICH");
	}
	return false;
}
#endif // !defined(i386_unknown_nt4_0)

// TODO: fix this
//
// add a new executable (binary) to a program.
//
bool paradynDaemon::newExecutable(const string &machineArg,
				  const string &login,
				  const string &name, 
				  const string &dir, 
				  const pdvector<string> &argv){
   string machine = machineArg;

   if (! DMstatus_initialized) {
       uiMgr->createStatusLine(DMstatus.c_str());
       DMstatus_initialized = true;
   }

   daemonEntry *def = findEntry(machine, name) ;
   if (!def) {
      if (name.length()) {
         string msg = string("Paradyn daemon \"") + name + string("\" not defined.");
         uiMgr->showError(90,P_strdup(msg.c_str()));
      }
      else {
         uiMgr->showError(91,"");
      }
      return false;
   }

   if (!machine.length()) {
      if (default_host.length()) {
	 string m = getNetworkName(default_host);
         if( m.length() == 0 )
         {
             string msg = string("Host \"") + default_host + 
                            string("\" (given as default_host) not found.");
             uiMgr->showError(90,P_strdup(msg.c_str()));
             return false;
         }
	 machine = m;
      }
   }
   else if (getNetworkName(machine) == "") {
         string msg = string("Host \"") + machine + string("\" not found.");
         uiMgr->showError(90,P_strdup(msg.c_str()));
         return false;
   }

   if ( def->getFlavorString() == "mpi" )
   {
#if defined(i386_unknown_nt4_0)
     string message = "Paradyn does not yet support MPI applications started from OS WinNT";
         
     uiMgr->showError(113, strdup(message.c_str()));
     return false;
#else
      string os;

      if (hostIsLocal(machine))
      {
          struct utsname unameInfo;
          if ( P_uname(&unameInfo) == -1 )
          {
              perror("uname");
              return false;
          }

          os = unameInfo.sysname;
      }
      else
      {
          // get OS name through remote uname	 
          char comm[256];
          FILE* commStream;
          string remoteShell;

          remoteShell = def->getRemoteShellString();
	  assert(remoteShell.length() > 0);
          sprintf(comm, "%s%s%s %s uname -s", 
                  remoteShell.c_str(),
                  login.length() ? " -l " : "",
                  login.length() ? login.c_str() : "",
                  machine.c_str());

          commStream = P_popen(comm, "r");

          bool foundOS = false;
      
          if ( commStream )
          {
              if ( !P_fgets(comm, 256, commStream) )
                  fclose(commStream);
              else
              {
                  os = comm;
                  fclose(commStream);
                  foundOS = true;
              }
          }
      
          if ( !foundOS )
          {
              uiMgr->showError(113, "Could not determine OS on remote host.");
              return false;
          }
      }

      if ( os.prefixed_by("IRIX") )
      {
         return(startIrixMPI(machine, login, name, dir, argv, args, def));
      }
      else if ( os.prefixed_by ("AIX") )
      {
         return(startPOE(machine, login, name, dir, argv, args, def));
      }
      else
      {
         return(startMPICH(machine, login, name, dir, argv, args, def));
      }
#endif
   }

   paradynDaemon *daemon;
   if ((daemon=getDaemonHelper(machine, login, name)) == (paradynDaemon*) NULL)
      return false;

   performanceStream::ResourceBatchMode(batchStart);
   int pid = daemon->addExecutable(argv, dir);
   performanceStream::ResourceBatchMode(batchEnd);

   // did the application get started ok?
   if (pid > 0 && !daemon->did_error_occur()) {
#ifdef notdef
      executable *exec = new executable(pid, argv, daemon);
      paradynDaemon::programs += exec;
      ++procRunning;
#endif
      return (true);
   } else {
      return(false);
   }
}

bool paradynDaemon::attachStub(const string &machine,
			       const string &userName,
			       const string &cmd, // program name (full path)
			       int the_pid,
			       const string &daemonName,
			       int afterAttach // 0 --> as is, 1 --> pause, 2 --> run
			       ) {
  // Note: by this time, both the RUN and PAUSE buttons have been disabled in the
  // user interface...

   if (! DMstatus_initialized) {
       uiMgr->createStatusLine(DMstatus.c_str());
       DMstatus_initialized = true;
   }

  paradynDaemon *daemon = getDaemonHelper(machine, userName, daemonName);
  if (daemon == NULL)
      return false;

  daemon->afterAttach_ = afterAttach;

  performanceStream::ResourceBatchMode(batchStart);
  bool success = daemon->attach(cmd, the_pid, afterAttach);
  performanceStream::ResourceBatchMode(batchEnd);

  if (daemon->did_error_occur())
     return false;

  if (!success)
     return false;

  return true; // success
}

//
// start the programs running.
//
bool paradynDaemon::startApplication()
{
    executable *prog;
    for(unsigned i=0; i < programs.size(); i++){
	prog = programs[i];
        prog->controlPath->startProgram(prog->pid);   
    }
    return(true);
}

//
// pause all processes.
//
bool paradynDaemon::pauseAll()
{
    paradynDaemon *pd;
    for(unsigned i = 0; i < paradynDaemon::allDaemons.size(); i++){
        pd = paradynDaemon::allDaemons[i];
	pd->pauseApplication();
    }
    // tell perf streams about change.
    performanceStream::notifyAllChange(appPaused);
    applicationState = appPaused;
    return(true);
}

//
// pause one processes.
//
bool paradynDaemon::pauseProcess(unsigned pid)
{
    executable *exec = 0;
    for(unsigned i=0; i < programs.size(); i++){
	exec = programs[i];
	if(exec->pid == pid)
	    break;
        exec = 0;
    }
    if (exec) {
        exec->controlPath->pauseProgram(exec->pid);
        return(true); 
    } else
	return (false);
}

//Send message to each daemon to instrument the dynamic call
//sites in function "func_name"
bool paradynDaemon::AllMonitorDynamicCallSites(string func_name){
  paradynDaemon *pd;
  for(unsigned int i = 0; i < paradynDaemon::allDaemons.size(); i++)
    {
      pd = paradynDaemon::allDaemons[i];
      pd->MonitorDynamicCallSites(func_name);
    }

  return true;
}

//
// continue all processes.
//
bool paradynDaemon::continueAll()
{
    paradynDaemon *pd;

    if (programs.size() == 0)
       return false; // no program to pause

    if (procRunning == 0)
       return false;

    for(int i = paradynDaemon::allDaemons.size()-1; i >= 0; i--) 
    {
        pd = paradynDaemon::allDaemons[i];
	pd->continueApplication();
    }
    // tell perf streams about change.
    performanceStream::notifyAllChange(appRunning);
    applicationState = appRunning;
    return(true);
}

//
// continue one processes.
//
bool paradynDaemon::continueProcess(unsigned pid)
{
    executable *exec = 0;
    for(unsigned i=0; i < programs.size(); i++){
	exec = programs[i];
	if(exec->pid == pid && exec->controlPath == this)
	    break;
        exec = 0;
    }
    if (exec) {
        exec->controlPath->continueProgram(exec->pid);
        return(true); 
    } else
	return (false);
}

//
// detach the paradyn tool from a running program.  This should clean all
//   of the dynamic instrumentation that has been inserted.
//
bool paradynDaemon::detachApplication(bool pause)
{
    executable *exec = 0;
    for(unsigned i=0; i < programs.size(); i++){
	exec = programs[i];
	exec->controlPath->detachProgram(exec->pid,pause);
    }
    return(true);
}

//
// print the status of each process.  This is used mostly for debugging.
//
void paradynDaemon::printStatus()
{
    executable *exec = 0;
    for(unsigned i=0; i < programs.size(); i++){
	exec = programs[i];
        string status = exec->controlPath->getStatus(exec->pid);
	    if (!exec->controlPath->did_error_occur()) {
	        cout << status << endl;
	    }
    }
}

//
// Cause the passed process id to dump a core file.  This is also used for
//    debugging.
// If pid = -1, all processes will dump core files.
//
void paradynDaemon::dumpCore(int pid)
{
    executable *exec = 0;
    for(unsigned i=0; i < programs.size(); i++){
	exec = programs[i];
        if ((exec->pid == (unsigned)pid) || (pid == -1)) {
	    exec->controlPath->coreProcess(exec->pid);
	    printf("found process and coreing it\n");
        }
    }
}


bool paradynDaemon::setInstSuppress(resource *res, bool newValue)
{
    bool ret = false;
    paradynDaemon *pd;
    for(unsigned i = 0; i < paradynDaemon::allDaemons.size(); i++){
        pd = paradynDaemon::allDaemons[i];
        ret |= pd->setTracking(res->getHandle(), newValue);
    }
    return(ret);
}

//
// signal from daemon that is is about to start or end a set 
// of new resource definitions
//
void paradynDaemon::resourceBatchMode(bool onNow){
    int prev = count;
    if (onNow) {
	count++;
    } else {
	assert(count > 0);
	count--;
    }

    if (count == 0) {
	for(u_int i=0; i < allDaemons.size(); i++){
            (allDaemons[i])->reportResources();
	}
        performanceStream::ResourceBatchMode(batchEnd);
    } else if (!prev) {
        performanceStream::ResourceBatchMode(batchStart);
    }
}

extern void ps_retiredResource(string resource_name);

void paradynDaemon::retiredResource(string resource_name) {
   ps_retiredResource(resource_name);
}

//
//  reportResources:  send new resource ids to daemon
//
void  paradynDaemon::reportResources(){
    assert(newResourceTempIds.size() == newResourceHandles.size());
    resourceInfoResponse(newResourceTempIds, newResourceHandles);
    newResourceTempIds.resize(0);
    newResourceHandles.resize(0);
}

bool paradynDaemon::isMonitoringProcess(int pid) {
   for(unsigned i=0; i<pidsThatAreMonitored.size(); i++) {
      if(pidsThatAreMonitored[i] == pid)
         return true;
   }
   return false;
}

void paradynDaemon::addProcessInfo(const pdvector<string> &resource_name) {
   // need at least /Machine, <machine>, process
   if(resource_name.size() < 3)
      return;
   
   const string &process_str = resource_name[2];
   string proc_name;
   int pid;
   bool result = resource::splitProcessResourceStr(process_str, NULL, &pid);

   if(result == true) {
      // it's a resource for a process
      pidsThatAreMonitored.push_back(pid);
   }
}

//
// upcall from paradynd reporting new resource
//
void paradynDaemon::resourceInfoCallback(u_int temporaryId,
                                         pdvector<string> resource_name,
                                         string abstr, u_int type) {

   if(resource_name.size() > 0) {
      const char *rstr = resource_name[0].c_str();
      if(rstr && !strcmp(rstr, "Machine")) {
         addProcessInfo(resource_name);
      }
   }

   resourceHandle r = resource::createResource(temporaryId, resource_name, 
                                               abstr, type);
   if(!count){
      if (r != temporaryId) {
         pdvector<u_int>tempIds; pdvector<u_int>rIds;
         tempIds += temporaryId; rIds += r;
         resourceInfoResponse(tempIds, rIds);
      }
   }
   else {
      if (r != temporaryId) {
         newResourceTempIds += temporaryId;
         newResourceHandles += r;
         assert(newResourceTempIds.size() == newResourceHandles.size());
      }
   }
}

void paradynDaemon::severalResourceInfoCallback(pdvector<T_dyninstRPC::resourceInfoCallbackStruct> items) {
   for (unsigned lcv=0; lcv < items.size(); lcv++)
      resourceInfoCallback(items[lcv].temporaryId,
			   items[lcv].resource_name,
			   items[lcv].abstraction,
			   items[lcv].type);
}


//
// Get the expected delay (as a fraction of the running program) for the passed
//   resource list (focus) and metric.
//
void paradynDaemon::getPredictedDataCostCall(perfStreamHandle ps_handle,
				      metricHandle m_handle,
				      resourceListHandle rl_handle,
				      resourceList *rl, 
				      metric *m,
				      u_int clientID)
{
    if(rl && m){
        pdvector<u_int> focus;
        bool aflag;
	aflag=rl->convertToIDList(focus);
	assert(aflag);
        const char *metName = m->getName();
        assert(metName);
        u_int requestId;
        if(performanceStream::addPredCostRequest(ps_handle,requestId,m_handle,
				rl_handle, paradynDaemon::allDaemons.size())){
            paradynDaemon *pd;
            for(unsigned i = 0; i < paradynDaemon::allDaemons.size(); i++){
                pd = paradynDaemon::allDaemons[i];
	        pd->getPredictedDataCost(ps_handle,requestId,focus, 
					 metName,clientID);
            }
	    return;
        }
    }
    // TODO: change this to do the right thing
    // this should make the response upcall to the correct calling thread
    // perfConsult->getPredictedDataCostCallbackPC(0,0.0);
    assert(0);
}

void filter_based_on_process_in_focus(
                                const pdvector<paradynDaemon *> &daemon_list,
                                pdvector<paradynDaemon *> *to_list,
                                resourceList *focus_resources)
{
   // if there are multiple daemons on the same machine and if the focus
   // specifies a process, then we need to refine this list of daemons down
   // to the daemon that is monitoring the specified process

   int pid;
   bool result = focus_resources->getProcessReferredTo(NULL, &pid);
   if(result == true) {
      // the metric-focus is specific to a particular process      

      for(unsigned i=0; i<daemon_list.size(); i++) {
         paradynDaemon *cur_dmn = daemon_list[i];
         if(cur_dmn->isMonitoringProcess(pid)) {
            (*to_list).push_back(cur_dmn);
         }
      }
   } else {
      // it's not a process specific focus, so add all daemons on
      // machine that was specified in the focus
      for(unsigned i=0; i<daemon_list.size(); i++) {
         paradynDaemon *cur_dmn = daemon_list[i];                     
         (*to_list).push_back(cur_dmn);
      }
   }
}

void strip_duplicate_daemons(const pdvector<paradynDaemon *> &daemon_list,
                             pdvector<paradynDaemon *> *to_list) {
    to_list->clear();
    
    if (daemon_list.size() == 0)
        return;
    
    to_list->push_back(daemon_list[0]);
    
    for (u_int i = 1; i < daemon_list.size(); i++) {
        paradynDaemon *cur_dmn = daemon_list[i];
        bool found = false;
        
        for (u_int j = 0; j < to_list->size(); j++)
            if ((*to_list)[j] == cur_dmn ||
                (*to_list)[j]->get_id() == cur_dmn->get_id()) {
                found = true;
                break;
            }
        if (!found)
            to_list->push_back(cur_dmn);
    }
}

// if whole_prog_focus is false, then the relevant daemons are in daemon_subset
void paradynDaemon::getMatchingDaemons(pdvector<metricInstance *> *miVec,
                                  pdvector<paradynDaemon *> *matching_daemons)
{
   bool whole_prog_focus = false;
   pdvector<paradynDaemon*> daemon_subset; // which daemons to send request
   
   for(u_int i=0; i < miVec->size(); i++) {
      // check to see if this focus is refined on the machine
      // or process heirarcy, if so then add the approp. daemon
      // to the matching_daemons, else set whole_prog_focus to true
       string machine_name;
       resourceList *focus_resources = (*miVec)[i]->getresourceList(); 
       assert(focus_resources);
       // focus is refined on machine or process heirarchy 
       if(focus_resources->getMachineNameReferredTo(machine_name)){
           // get the daemon corr. to this focus and add it
           // to the list of daemons
           pdvector<paradynDaemon*> vpd = 
           paradynDaemon::machineName2Daemon(machine_name);
           assert(vpd.size());
           
           // Add daemons into daemon_subset
           filter_based_on_process_in_focus(vpd, &daemon_subset, focus_resources);
       }
       else {  // focus is not refined on process or machine 
           whole_prog_focus = true;
           // As soon as this is true, we add all the daemons to the list.
           break;
       }
   }

   if(whole_prog_focus) {
       // Don't need to prune, as there's only one daemon
       (*matching_daemons).clear();
       for(unsigned i=0; i<paradynDaemon::allDaemons.size(); i++)
           (*matching_daemons).push_back(paradynDaemon::allDaemons[i]);
   }
   else {
       // Prune the list we generated, since there might be duplicates
       strip_duplicate_daemons(daemon_subset, matching_daemons);
   }
   
   
   assert(matching_daemons->size() <= paradynDaemon::allDaemons.size());
}


// propagateMetrics:
// called when a new process is started, to propagate all enabled metrics to
// the new process.  (QUESTION: should this include when a process makes
// an exec syscall, thus 'starting' another process?)
// Metrics are propagated only if the new process is the only process running 
// on a daemon (this is why we don't need the pid here). If there are already
// other processes running on a daemon, than it is up to the daemon to do the
// propagation (we can't do it here because the daemon has to do the aggregation).
// Calling this function has no effect if there are no metrics enabled.
void paradynDaemon::propagateMetrics() {
   pdvector<metricInstanceHandle> allMIHs = metricInstance::allMetricInstances.keys();

   for (unsigned i = 0; i < allMIHs.size(); i++) {
      metricInstance *mi = metricInstance::getMI(allMIHs[i]);
      if (!mi->isEnabled()) {
         metricFocusReq_Val::attachToOutstandingRequest(mi, this);
         continue;
      }

      // first we must find if the daemon already has this metric enabled for
      // some process. In this case, we don't need to do anything, the
      // daemon will do the propagation by itself.
      bool found = false;
      for (unsigned j = 0; j < mi->components.size(); j++) {
         if (mi->components[j]->getDaemon() == this) {
            found = true;
            break;
         }
      }

      if (found) {
         continue; // we don't enable this mi; let paradynd do it
      }

      resourceListHandle r_handle = mi->getFocusHandle();
      metricHandle m_handle = mi->getMetricHandle();
      resourceList *rl = resourceList::getFocus(r_handle);
      metric *m = metric::getMetric(m_handle);

      pdvector<u_int> vs;
      bool aflag = rl->convertToIDList(vs);
      assert(aflag);

      T_dyninstRPC::instResponse resp =
         enableDataCollection2(vs, (const char *) m->getName(), mi->id,
                               this->id );
      
      inst_insert_result_t status = inst_insert_result_t(resp.rinfo[0].status);
      if(did_error_occur())
         continue;

      if(status == inst_insert_deferred) {
         // This shouldn't happen since when we propagate metricInstances to
         // new machines (ie. call enableDataCollection2 above), the process
         // on that machine hasn't yet been started.  At this time, a
         // metric-focus can only be deferred if the process is running
         // (amongst other conditions).
         cerr << "WARNING: failed to propagate metric-focus "
              << mi->getMetricName() << " / " << mi->getFocusName()
              << " to machine\n   " << getMachineName() 
              << " since metric-focus was deferred by daemon\n";
      } else if(status == inst_insert_success) {
         component *comp = new component(this, resp.rinfo[0].mi_id, mi);
         if (!mi->addComponent(comp)) {
            cout << "internal error in paradynDaemon::addRunningProgram" 
                 << endl;
            abort();
         }
      }
      // do nothing if request failed
   }
}


bool paradynDaemon::setDefaultArgs(char *&name)
{
  if (!name)
    name = strdup("defd");
  if (name)
    return true;
  else 
    return false;
}


bool daemonEntry::setAll (const string &m, const string &c, const string &n,
			  const string &l, const string &d, const string &r, const string &f)
{
  if(!n.c_str() || !c.c_str())
      return false;

  if (m.c_str()) machine = m;
  if (c.c_str()) command = c;
  if (n.c_str()) name = n;
  if (l.c_str()) login = l;
  if (d.c_str()) dir = d;
  if (r.c_str()) remote_shell = r;
  if (f.c_str()) flavor = f;

  return true;
}
void daemonEntry::print() 
{
  cout << "DAEMON ENTRY\n";
  cout << "  name: " << name << endl;
  cout << "  command: " << command << endl;
  cout << "  dir: " << dir << endl;
  cout << "  login: " << login << endl;
  cout << "  machine: " << machine << endl;
  cout << "  remote_shell: " << remote_shell << endl;
  cout << "  flavor: " << flavor << endl;
}

#ifdef notdef
int paradynDaemon::read(const void* handle, char *buf, const int len) {
  assert(0);
  int ret, err;

  assert(len > 0);
  assert((int)handle<200);
  assert((int)handle >= 0);
  static pdvector<unsigned> fd_vect(200);

  // must handle the msg_bind_buffered call here because xdr_read will be
  // called in the constructor for paradynDaemon, before the previous call
  // to msg_bind had been called

  if (!fd_vect[(unsigned)handle]) {
    paradynDaemon *pd;
    for(unsigned i = 0; i < paradynDaemon::allDaemons.size(); i++){
        pd = paradynDaemon::allDaemons[i];
        if(pd->get_sock() == (int)handle)
	    break;
    }
    if (!(pd))
      return -1;

    msg_bind((int)handle, true, (int(*)(void*))xdrrec_eof,
		      (void*)(pd)->net_obj(), &xxx);
    fd_vect[(unsigned)handle] = 1;
  }

  do {
    tag_t tag = MSG_TAG_FILE;
	thread_t ready_tid;

    do
		ready_tid = THR_TID_UNSPEC;
		err = msg_poll(&ready_tid, &tag, true);
	while ((err != THR_ERR) && (ready_tid != (thread_t)handle))
    
    if (ready_tid == (thread_t) handle) {
      errno = 0;
      ret = P_read((int)handle, buf, len);
    } else 
      return -1;
  } while (ret < 0 && errno == EINTR);

  if (ret <= 0)
    return (-1);
  else
    return ret;
}
#endif // notdef

void paradynDaemon::setDaemonStartTime(int, double startTime) {
  timeStamp firstT(startTime, timeUnit::sec(), timeBase::bStd());
  if(!getStartTime().isInitialized() || firstT < getStartTime()) {
    //attemptUpdateAggDelay();
    setStartTime(firstT);
  }
  setEarliestStartTime(getAdjustedTime(firstT));
}

void paradynDaemon::setEarliestStartTime(timeStamp f) {
  if(! earliestStartTime.isInitialized()) earliestStartTime = f;
  else if(f < earliestStartTime)  earliestStartTime = f;
}

void paradynDaemon::setInitialActualValueFE(int mid, double initActVal) {
  sampleVal_cerr << "paradynDaemon::setInitialActualValueFE- mid: " << mid
		 << ", val: " << initActVal << "\n";

  metricInstance *mi = NULL;
  bool found = metricInstance::allMetricInstances.find(mid, mi);
  assert(found);
  assert(mi);

  // ---------------------------------------------------------------
  // The following code converts the old format received through the
  // visis into the new format.  This switchover can be eliminated
  // once the visis are converted.
  metric *met = metric::getMetric(mi->getMetricHandle());
  double multiplier;
  if(met->getStyle() == SampledFunction) {
    multiplier = 1.0;
  } else {
    multiplier = 1000000000.0; 
  }
  // ---------------------------------------------------------------

  // find the right component.
  component *part = NULL;
  for(unsigned i=0; i < mi->components.size(); i++) {
    if(mi->components[i]->daemon == this) {
      part = mi->components[i];
      break;
    }
  }
  assert(part != NULL);

  aggComponent *aggComp = part->sample;  
  pdSample value = pdSample(static_cast<int64_t>(initActVal * multiplier));
  aggComp->setInitialActualValue(value);
}

paradynDaemon::paradynDaemon(const string &m, const string &u, const string &c,
			   const string &r, const string &n, const string &f)
: dynRPCUser(m, u, c, r, NULL, NULL, args, 1, dataManager::sock_desc),
  machine(m), login(u), command(c), name(n), flavor(f), afterAttach_(0), activeMids(uiHash)
{
  if (!this->errorConditionFound) {
    // No problems found in order to create this new daemon process - naim
    assert(m.length());
    assert(c.length());
    assert(n.length());
    assert(f.length());

    // if c includes a pathname, lose the pathname
    const char *loc = P_strrchr(c.c_str(), '/');
    if (loc) {
      loc = loc + 1;
      command = loc;
    }

    if (machine.suffixed_by(local_domain)) {
        const unsigned namelength = machine.length()-local_domain.length()-1;
        const string localname = machine.substr(0,namelength);
        status = localname;
    } else {
        status = machine;
    }

    // add a unique-ifier to the status line name
    // so that we don't place status messages for all daemons running
    // on teh same host into the same status line in the GUI
    {
        char idxbuf[16];
        sprintf( idxbuf, "%d", paradynDaemon::daemonsByHost[status].size() );

        string sfx = ((string)("[")) + idxbuf + "]";
        paradynDaemon::daemonsByHost[status].push_back( this );

        // now make "status" unique
        status += sfx;
    }
    uiMgr->createProcessStatusLine(status.c_str());
    
    paradynDaemon *pd = this;
    paradynDaemon::allDaemons+=pd;
    id = paradynDaemon::allDaemons.size()-1;
    assert(paradynDaemon::allDaemons.size() > id); 

    metCheckDaemonProcess(machine);
  }
  // else...we leave "errorConditionFound" for the caller to check...
  //        don't forget to check!
}

// machine, name, command, flavor and login are set via a callback
paradynDaemon::paradynDaemon(PDSOCKET use_sock)
: dynRPCUser(use_sock, NULL, NULL, 1), flavor(0), afterAttach_(0), activeMids(uiHash) {
  if (!this->errorConditionFound) {
    // No problems found in order to create this new daemon process - naim 
    paradynDaemon *pd = this;
    paradynDaemon::allDaemons += pd;
    id = paradynDaemon::allDaemons.size()-1;
  }
  // else...we leave "errorConditionFound" for the caller to check...
  //        don't forget to check!
}

bool our_print_sample_arrival = false;
void printSampleArrivalCallback(bool newVal) {
   our_print_sample_arrival = newVal;
}

// batched version of sampleCallbackFunc
void paradynDaemon::batchSampleDataCallbackFunc(int ,
		pdvector<T_dyninstRPC::batch_buffer_entry> theBatchBuffer)
{
    sampleVal_cerr << "batchSampleDataCallbackFunc(), burst size: " 
		   << theBatchBuffer.size() << "   earliestFirstTime: " 
		   << getEarliestStartTime() << "\n";
    // Go through every item in the batch buffer we've just received and
    // process it.
    for (unsigned index=0; index < theBatchBuffer.size(); index++) {
	const T_dyninstRPC::batch_buffer_entry &entry = theBatchBuffer[index] ; 
	unsigned mid             = entry.mid ;
        // Okay, the sample is not an error; let's process it.
	metricInstance *mi;
        bool found = activeMids.find(mid, mi);
	if (!found) {
	  // this can occur due to asynchrony of enable or disable requests
	  // so just ignore the data
	  if (our_print_sample_arrival)
	    cout << "ignoring that sample since it's no longer active" << endl;
	  continue;
        }
       	assert(mi);

	timeStamp startTimeStamp = 
	  timeStamp(entry.startTimeStamp, timeUnit::sec(), timeBase::bStd());
	timeStamp endTimeStamp   =
	  timeStamp(entry.endTimeStamp, timeUnit::sec(), timeBase::bStd());

	// ---------------------------------------------------------------
	// The following code converts the old format received through the
	// visis into the new format.  This switchover can be eliminated
	// once the visis are converted.
	metric *met = metric::getMetric(mi->getMetricHandle());
	double multiplier;
	if(met->getStyle() == SampledFunction) {
	  multiplier = 1.0;
	} else {
	  multiplier = 1000000000.0;
	}
	// ---------------------------------------------------------------
	pdSample value= pdSample(static_cast<int64_t>(entry.value*multiplier));

	if (our_print_sample_arrival) {
	  cerr << "[" << index << "] mid " << mid << "-   from: " 
	       << startTimeStamp << "  to: " << endTimeStamp << "   val: "
	       << value << "  machine: " << machine.c_str() << "\n";
	}
	
	timeStamp newStartTime = this->getAdjustedTime(startTimeStamp);
	timeStamp newEndTime   = this->getAdjustedTime(endTimeStamp);

	if (our_print_sample_arrival) {
	  cerr << "(after adj) [" << index << "] mid " << mid << "-   from: " 
	       << newStartTime << "  to: " << newEndTime << "   val: "
	       << value << "  machine: " << machine.c_str() << "\n";
	}
	
	mi->updateComponent(this, newStartTime, newEndTime, value);
	mi->doAggregation();
    }
}

// trace data streams
void paradynDaemon::batchTraceDataCallbackFunc(int ,
                pdvector<T_dyninstRPC::trace_batch_buffer_entry> theTraceBatchBuffer)
{
    // get the earliest first time that had been reported by any paradyn
    // daemon to use as the base (0) time
    // assert(getEarliestFirstTime());

  // Just for debugging:
  //fprintf(stderr, "in DMdaemon.C, burst size = %d\n", theTraceBatchBuffer.size());

    // Go through every item in the batch buffer we've just received and
    // process it.
    for (unsigned index =0; index < theTraceBatchBuffer.size(); index++) {
        T_dyninstRPC::trace_batch_buffer_entry &entry = theTraceBatchBuffer[index] ;

        unsigned mid          = entry.mid ;
        unsigned length       = entry.length;

        if (our_print_sample_arrival) {
            cout << "mid " << mid << " : length = " << length << "\n";
        }

        // Okay, the sample is not an error; let's process it.
        metricInstance *mi;
        bool found = activeMids.find(mid, mi);
        if (!found) {
           // this can occur due to asynchrony of enable or disable requests
           // so just ignore the data
          continue;
        }
        assert(mi);
        byteArray *localTraceData = new byteArray(entry.traceRecord.getArray(),
        length);
        mi->sendTraceData(localTraceData->getArray(),length);

        delete localTraceData;

    } // the main for loop
}

//
// paradyn daemon should never go away.  This represents an error state
//    due to a paradynd being killed for some reason.
//
// TODO -- handle this better
paradynDaemon::~paradynDaemon() {

#ifdef notdef
    metricInstance *mi;
    HTable<metricInstance*> curr;

    allDaemons.remove(this);

    // remove the metric ID as required.
    for (curr = activeMids; mi = *curr; curr++) {
	mi->parts.remove(this);
	mi->components.remove(this);
    }
#endif
    printf("Inconsistant state\n");
    abort();
}

//
// When an error is determined on an igen call, this function is
// called, since the default error handler will exit, and we don't
// want paradyn to exit.
//
void paradynDaemon::handle_error()
{
   removeDaemon(this, true);
}

//
// When a paradynd is started remotely, ie not by paradyn, this upcall
// reports the information for that paradynd to paradyn
//
// This must set command, name, machine and flavor fields
// (pid no longer used --ari)
//
void 
paradynDaemon::reportSelf (string m, string p, int /*pid*/, string flav)
{
  flavor = flav;
  if (!m.length() || !p.length()) {
    removeDaemon(this, true);
    printf("paradyn daemon reported bad info, removed\n");
    // error
  } else {
    machine = m.c_str();
    command = p.c_str();

    if (machine.suffixed_by(local_domain)) {
        const unsigned namelength = machine.length()-local_domain.length()-1;
        const string localname = machine.substr(0,namelength);
        status = localname;
    } else
        status = machine;

    // add a unique-ifier to the status line name
    // so that we don't place status messages for all daemons running
    // on teh same host into the same status line in the GUI
    {
        char idxbuf[16];
        sprintf( idxbuf, "%d", paradynDaemon::daemonsByHost[status].size() );

        string sfx = ((string)("[")) + idxbuf + "]";
        paradynDaemon::daemonsByHost[status].push_back( this );

        // now make "status" unique
        status += sfx;
    }
    uiMgr->createProcessStatusLine(status.c_str());

    if(flavor == "pvm") {
      name = "pvmd";
    } else if(flavor == "unix") {
      name = "defd";
    } else if(flavor == "mpi") {
      name = "mpid";
    } else if (flavor == "winnt") {
      name = "winntd";
    } else {
      name = flavor;
    }

	metCheckDaemonProcess( machine );
  }

  // Send the initial metrics, constraints, and other neato things
  SendMDLFiles();
  pdvector<T_dyninstRPC::metricInfo> info = this->getAvailableMetrics();
  unsigned size = info.size();
  for (unsigned u=0; u<size; u++)
      addMetric(info[u]);

  this->updateTimeAdjustment();

#if !defined(NO_SYNC_REPORT_SELF)
    // tell the daemon we're done with this call
    // it can go off and ignore its event loop for awhile
    reportSelfDone();
#endif // !defined(NO_SYNC_REPORT_SELF)

  return;
}

//
// When a paradynd reports status, send the status to the user
//
void 
paradynDaemon::reportStatus (string line)
{
    uiMgr->updateStatusLine(status.c_str(), P_strdup(line.c_str()));
}

/***
 This call is used by a daemon to report a change in the status of a process
 such as when the process exits.
 When one process exits, we just decrement procRunning, a counter of the number
 of processes running. If procRunning is zero, there are no more processes running,
 and the status of the application is set to appExited.
***/
void
paradynDaemon::processStatus(int pid, u_int stat) {
  if (stat == procExited) { // process exited
    for(unsigned i=0; i < programs.size(); i++) {
      if ((programs[i]->pid == static_cast<unsigned>(pid)) && 
	  programs[i]->controlPath == this) {
	programs[i]->exited = true;
	if (--procRunning == 0)  performanceStream::notifyAllChange(appExited);
	int totalProcs, numExited;
	getProcStats(&totalProcs, &numExited);
	if(totalProcs == numExited)
	  reportStatus("application exited");
	else if(totalProcs > 1 && numExited>0) {
	  string msg;
	  msg = string("application running, ") + string(numExited) + " of " + 
	    string(totalProcs) + " processes exited\n";
	  reportStatus(msg.c_str());
	}
	break;
      }
    }
  }
}

void paradynDaemon::getProcStats(int *numProcsForDmn, int *numProcsExited) {
  int numProcs = 0, numExited = 0;

  for(unsigned i=0; i < programs.size(); i++) {
    executable *entry = programs[i];
    if(entry->controlPath==this) {
      numProcs++;
      if(entry->exited==true)
	numExited++;
    }
  }
  *numProcsForDmn = numProcs;
  *numProcsExited = numExited;
}

// Called by a daemon when there is no more data to be sent for a metric
// instance (because the processes have exited).
void
paradynDaemon::endOfDataCollection(int mid) {
  sampleVal_cerr << "endOfDataCollection-  mid: " << mid << "\n";

    if(activeMids.defines(mid)){
        metricInstance *mi = activeMids[mid];
	assert(mi);
	mi->removeComponent(this);
    }
    else{  // check if this mid is for a disabled metric 
        bool found = false;
        for (unsigned ve=0; ve<disabledMids.size(); ve++) {
            if ((int) disabledMids[ve] == mid) {
 	        found = true;
	        break;
 	    }
        }
	if (!found) {
	    cout << "Ending data collection for unknown metric" << endl;
	    uiMgr->showError (2, "Ending data collection for unknown metric");
	}
    }
}
