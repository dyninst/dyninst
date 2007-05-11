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

/*
 * $Id: DMdaemon.C,v 1.172 2007/05/11 21:47:32 legendre Exp $
 * method functions for paradynDaemon and daemonEntry classes
 */
#include "paradyn/src/pdMain/paradyn.h"
#include <assert.h>
#include <stdio.h>
#include <malloc.h>
#include "common/h/headers.h"
#include "pdutil/h/mdlParse.h"

template class pdvector<double>;
template class pdvector < pdvector<double> >;
template class pdvector< pdvector<T_dyninstRPC::metricInfo> >;
template class pdvector< T_dyninstRPC::instResponse >;
template class pdvector<T_dyninstRPC::daemonInfo>;
template class  pdvector <int>;



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
#include "dyninstRPC.mrnet.CLNT.h"
#include "DMdaemon.h"
#include "paradyn/src/TCthread/tunableConst.h"
#include "paradyn/src/UIthread/Status.h"
#include "DMmetric.h"
#include "paradyn/src/met/metricExt.h"
#include "pdutil/h/rpcUtil.h"
#include "pdutil/h/pdDebugOstream.h"
#include "common/h/timing.h"
#include "DMmetricFocusReq.h"
#include <sstream>
#include <iostream>
#include <fstream>

static int getNumberOfNodes( const pdvector<pdstring> &argv);

#if defined(os_aix)
static bool startPOE(const pdstring &, const pdstring &,
                     const pdstring &, const pdstring &,
                     const pdvector<pdstring> & , 
                     const pdvector<pdstring> , daemonEntry *);
#endif




//template class dictionary_hash<unsigned, paradynDaemon*>;
//template class pdvector < paradynDaemon* >;

// TEMP this should be part of a class def.
pdstring DMstatus="Data Manager";
bool DMstatus_initialized = false;

unsigned int num_dmns_to_report_resources=0;
unsigned int num_dmns_to_report_callgraph=0;

extern unsigned enable_pd_samplevalue_debug;
extern pdvector<pdstring> mdl_files;

#if ENABLE_DEBUG_CERR == 1
#define sampleVal_cerr if (enable_pd_samplevalue_debug) cerr
#else
#define sampleVal_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

// This is from met/metMain.C
void metCheckDaemonProcess( const pdstring & );

pdvector<paradynDaemon::MPICHWrapperInfo> paradynDaemon::wrappers;
dictionary_hash<pdstring, pdvector<paradynDaemon*> > paradynDaemon::daemonsByHost( pdstring::hash );
dictionary_hash<unsigned, paradynDaemon*> paradynDaemon::daemonsById( uiHash );


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


const pdstring daemonEntry::getRemoteShellString() const {
   if(remote_shell.length() > 0) {
      return remote_shell;
   } else {
      const char *rshCmd = getRshCommand();
      return pdstring(rshCmd);
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
				       const pdvector<pdstring> &paradynd_argv,
				       paradynDaemon *daemon,
				       bool calledFromExec,
				       bool isInitiallyRunning) {
   executable *exec = NULL;
   assert(daemon);    

   if (calledFromExec) 
   {
      for (unsigned i=0; i < programs.size(); i++)
      {
         if ((int) programs[i]->pid == pid && programs[i]->controlPath == daemon) 
	      {
            exec = programs[i];
            break;
	      }
      }
      assert(exec);
      
      // exec() doesn't change the pid, but paradynd_argv changes (big deal).
      exec->argv = paradynd_argv;
      
      // fall through (we probably want to execute the daemon->continueProcess())
   }
   else
   {
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
   if (applicationState == appRunning)
   {
      daemon->continueProcess(pid);
      uiMgr->enablePauseOrRun();
   }
   else 
   {
      switch(daemon->afterAttach_ ) 
      {
         case 0:
            // Leave as is... key off the isInitiallyRunning parameter
            if (isInitiallyRunning) 
            {
               daemon->continueProcess(pid);
               applicationState = appRunning;
               performanceStream::notifyAllChange(appRunning);
            }
            break;
         case 1:
            break;
         case 2:
            // Run
            daemon->continueProcess(pid);
            applicationState = appRunning;
            performanceStream::notifyAllChange(appRunning);
            break;
      }
   }
   return true;
}

//  TODO : fix this so that it really does clean up state
// Dispose of daemon state.
//    This is called because someone wants to kill a daemon, or the daemon
//       died and we need to cleanup after it.
//
void paradynDaemon::removeDaemon(paradynDaemon *d, bool informUser)
{

    if (informUser) {
       pdstring msg;
       msg = pdstring("paradynd has died on host <") + d->getMachineName()
             + pdstring(">");
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

  MRN::Stream * stream = network->new_Stream(communicator);
  pdvector<double> daemon_time = getTime( stream );
  delete stream;

  timeStamp dt = timeStamp(daemon_time[0], timeUnit::sec(), timeBase::bStd());

  timeStamp t2 = getCurrentTime();
  timeLength delay = (t2 - t1) / 2.0;
  if(networkDelay != NULL)
    *networkDelay = delay;
  if(timeAdjustment != NULL)
    *timeAdjustment = t1 - dt + delay;
}
#define Samples 25
//This function calculates the time adjustment for all daemons
bool paradynDaemon::updateTimeAdjustment( )
{

	if( allDaemons.size() == 0 )
		{
			return false;
    }

	//TODO: we shouldn't need to do this. there should be static igen funcs
	//      available for group communication.
	paradynDaemon * pd = allDaemons[0];
	MRN::Communicator * comm = pd->getNetwork()->get_BroadcastCommunicator();
	MRN::Stream * streamPhase1 = pd->getNetwork()->
		new_Stream( comm,
								MRN::TFILTER_SAVE_LOCAL_CLOCK_SKEW_UPSTREAM,
								MRN::SFILTER_WAITFORALL,
								MRN::TFILTER_SAVE_LOCAL_CLOCK_SKEW_DOWNSTREAM );
	
	MRN::Stream * streamPhase2  = pd->getNetwork()->new_Stream( comm, MRN::TFILTER_GET_CLOCK_SKEW );
	
	//Phase 1:
	// - Run N iterations of "SaveLocalClockSkew". The MRNet nodes
	//   will calculate clock skew amongst their immediate neighbors.
	// - Multiple iterations will result in smallest latency being used in
	//   skew calulation.
	//	pdvector <double> result;

	for( unsigned int i=0; i < Samples; i++ )
		{
			//Calculate send time and pass down.
			struct timeval sendTimeVal;
			gettimeofday( &sendTimeVal, NULL );

			double sendTime = ((double)sendTimeVal.tv_sec) + (((double)sendTimeVal.tv_usec)/1000000U);
			pd->save_LocalClockSkew( streamPhase1, sendTime, 1 );
	
			//Phase 2:
			// - Collect Cumulative Clock Skew
			//   - resulting vector contains skew b/n FE and all backend nodes.
			
			//pdvector < pdvector<double> > clock_skew_vector;
			//clock_skew_vector = pd->get_ClockSkew( streamPhase2, 1 );
			//assert ( clock_skew_vector[0].size() == allDaemons.size() );

			//for( unsigned int j = 0; j<allDaemons.size(); j++)
			//	{
			//		result.push_back(clock_skew_vector[0][j]);
			//	}
    }
	
	pdvector < pdvector<double> > clock_skew_vector;
	clock_skew_vector = pd->get_ClockSkew( streamPhase2, 1 );

	//assert ( clock_skew_vector[0].size() == allDaemons.size() );
	for(unsigned j = 0 ; j < clock_skew_vector[0].size(); j++)
		{
			timeLength len = timeLength(clock_skew_vector[0][j] , timeUnit::sec() );
			paradynDaemon::allDaemons[j]->setTimeFactor( len );
		}
	
	
	delete streamPhase1;
	delete streamPhase2;
		
	//Store clock skew results in respective daemon
	/*
	int counter = 0;

	for( unsigned int ii = 0; ii < allDaemons.size(); ii++)
		{
			// TODO this is NOT correct - we need to use
			// the time classes used by Paradyn instead of gettimeofday().
			//paradynDaemon::allDaemons[i]->
			//    setTimeFactor( timeLength( clock_skew_vector[i][0], timeUnit::sec() ) );
			double avgResult = -100000.0;
			for(unsigned k =0 ; k < Samples ; k++)
				{
					if(result[i*Samples +k] > avgResult)
						avgResult = result[i*Samples +k];
				}
			paradynDaemon::allDaemons[ii]->
				setTimeFactor( timeLength(avgResult/(double)Samples , timeUnit::sec() ) );
			fprintf(stderr,"Time factor daemon[%d] = %lf\n",ii,avgResult);
		}
	*/
	return true;
}

// will update the time adjustment for this daemon with the average
// skew value
// returns the maximum network delay 
timeLength paradynDaemon::updateTimeAdjustment2(const int samplesToTake) {
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
paradynDaemon::SendMDLFiles( MRN::Stream * stream )
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


    for( pdvector<pdstring>::const_iterator iter = mdl_files.begin();
            iter != mdl_files.end();
            iter++ )
    {
        pdstring curFileName = *iter;
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

        pdstring buf( (const char*)mapAddr, mapLen );
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

            pdstring buf( (const char*)mapAddr, fileInfo.st_size );
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
        send_mdl( stream, mdlBufs );
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

// add a new daemon
// check to see if a daemon that matches the function args exists
// if it does exist, return a pointer to it
// otherwise, call instantiateDaemon() to create a new daemon
paradynDaemon *paradynDaemon::getDaemon(const pdstring &machine, 
                                        const pdstring &login, 
                                        const pdstring &name )
{
	pdstring m = machine; 
	// fill in machine name to default_host if empty
	if (m.length() == 0) {
		if (default_host.length()) {
			m = getNetworkName(default_host);
		} else {
			m = getNetworkName();
		}
	} else {
		m = getNetworkName(m);
	}

	// look for existing machine+login+paradynd daemon
	paradynDaemon *pd=NULL;
	bool foundSimilarDaemon = false;
	for(unsigned i = 0; i < paradynDaemon::allDaemons.size(); i++){
		pd = paradynDaemon::allDaemons[i];

		if ((!m.c_str() || (pd->machine == m)) && 
            (!login.c_str() || (pd->login == login))) {

            if((name.c_str() && (pd->name == name))) {
                return (pd);     
            }
            else 
                foundSimilarDaemon = true;
        }
	}

	if(foundSimilarDaemon) {
		cerr << "Warning, found an existing daemon on requested machine \"" << m
				 << "\" and with requested login \"" << login << "\", but the"
				 << " requested daemon name \"" << name << "\" doesn't match." 
				 << endl;
        return pd;
	}

    return pd;
}

// define a new entry for the daemon dictionary, or change an existing entry
bool paradynDaemon::defineDaemon (const char *c, const char *d,
                                  const char *l, const char *n,
                                  const char *m, const char *r,
                                  const char *f, const char *t,
                                  const char *MPIt ) 
{    
    if (! DMstatus_initialized) {
        uiMgr->createStatusLine(DMstatus.c_str());
        DMstatus_initialized = true;
    }

    bool redefinition=false;
		
    if(!n || !c)
			return false;
		
    pdstring command = (c?c:"");
    pdstring dir = (d?d:"");
    pdstring login = (l?l:"");
    pdstring name = (n?n:"");
    pdstring machine = (m?m:"");
    pdstring remote_shell = (r?r:"");
    pdstring flavor = (f?f:"");
    pdstring mrnet_topology = (t?t:"");
    pdstring MPItype = (MPIt?MPIt:"");
    daemonEntry *newE;

    for(unsigned i=0; i < allEntries.size(); i++)
    {
       newE = allEntries[i];
       if(newE->getNameString() == name)
       {
          if (newE->setAll(machine, command, name, login, dir, remote_shell,
                           flavor, mrnet_topology, MPItype))
          {
             redefinition=true;
             break;
          }
          else
          {
             return false;
          }
       }
    }
    
    if( !redefinition ) {
      newE = new daemonEntry(machine, command, name, login, dir,
                             remote_shell, flavor, mrnet_topology,MPItype);
      if (!newE) {
        return false;
      }
    }
    allEntries += newE;

    return true;
}

bool paradynDaemon::instantiateMRNetforMPIDaemons(daemonEntry * ide,
                                                  unsigned int iprocess_count)
{
    MRN::Network * network=NULL;
    MRN::Network::LeafInfo **leafInfo=NULL;
    unsigned int nLeaves = 0;

    if(ide->getMRNetTopologyString().length() == 0) {
        pdstring mrnet_top_buf=getNetworkName()+":0 => ";

        for(unsigned int i = 0 ; i < iprocess_count ; i++) {
            char buf[256];
            snprintf(buf, 256, "mpidummy%d:%d ", i, i );
            mrnet_top_buf += buf;
        }
        mrnet_top_buf += ";";

        network = new MRN::Network(mrnet_top_buf.c_str(), false,
                                   &leafInfo, &nLeaves );
    }
    else{
        network = new MRN::Network(ide->getMRNetTopology(),
                                   &leafInfo, &nLeaves );
    }

    ide->setMRNetNetwork( network );
    ide->setMRNetLeafInfo( leafInfo );
    ide->setMRNetNumLeaves( nLeaves );
    return true;
}

bool paradynDaemon::instantiateDefaultDaemon( daemonEntry * ide,
                                              const pdvector <pdstring> * ihosts )
{
    MRN::Network * network;

    //first gather paradynd args into a argv vector;
    char ** argv;
    argv = (char **)malloc(sizeof(char*) * (args.size()+1) );
    for( unsigned int i = 0; i<paradynDaemon::args.size(); i++){
        argv[i] = strdup(paradynDaemon::args[i].c_str() );
        argv[i+1] = NULL;
    }

    if( ide->getMRNetTopologyString() == "" ){
        //create network using a automated topology buffer if necessary
        unsigned int count=1;
        
        pdstring mrnet_top_buf=getNetworkName()+":0 => ";
        
        for( unsigned int i=0; i < (*ihosts).size(); i++ ){
            char buf[256];
            
            snprintf(buf, 256, "%s:%d ", (*ihosts)[i].c_str(), count++ );
            mrnet_top_buf += buf;
        }
        
        mrnet_top_buf += ";";

        network = new MRN::Network (mrnet_top_buf.c_str(), false,
                                     ide->getCommand(), (const char **)argv);
    }
    else{
        network = new MRN::Network (ide->getMRNetTopology(),
                                    ide->getCommand(), (const char **)argv);
    }
    ide->setMRNetNetwork( network );

    for( unsigned int i=0; i<paradynDaemon::args.size(); i++){
        free(argv[i]);
    }

    return true;
}

bool paradynDaemon::instantiateMRNetforManualDaemon( )
{
    MRN::Network * network=NULL;
    MRN::Network::LeafInfo **leafInfo=NULL;
    unsigned int nLeaves = 0;

    pdstring mrnet_top_buf=getNetworkName()+":0 => dummyhost0:0 ;";

    network = new MRN::Network( mrnet_top_buf.c_str(), false,
                                &leafInfo, &nLeaves );

    //Print out paradynd command line and wait for daemon to connect
    char buf[1024];
    sprintf(buf,
            "To start a paradyn daemon, logon to chosen machine and "
            "run paradynd with the following arguments:\n\n"
            "\tparadynd -z<flavor> -l1 -P%d %s %d %d\n\n"
            "(where flavor is one of: unix, mpi, or winnt).\n",
            dataManager::termWin_port, leafInfo[0]->get_ParHost(),
            leafInfo[0]->get_ParPort(), leafInfo[0]->get_Rank() );

    uiMgr->showError(99, buf);

    if( network->connect_Backends() < 0 ) {
        fprintf(stderr,"MRN::Network::connect_Backends() failed\n");
        return false;
    }

    daemonEntry * de = new daemonEntry( );
    de->setMRNetNetwork( network );
    de->setMRNetLeafInfo( leafInfo );
    de->setMRNetNumLeaves( nLeaves );

    initializeDaemon( de, true );

    return true;
}

bool paradynDaemon::initializeDaemon(daemonEntry * de, bool started_manually)
{
    //Bind the socket that the MRNet front-end uses
    //Not an actual "network" bind, but just lets thr_mailbox::poll()
    // add the sockfd to its list of monitored activities.
    //We will read the connection for data ourself
    thread_t stid;
    int *mrnet_sockets;
    unsigned int num_mrnet_sockets;
    de->getMRNetNetwork()->get_SocketFd( &mrnet_sockets, &num_mrnet_sockets );
  
    for( unsigned int i=0; i<num_mrnet_sockets; i++) {
        msg_bind_socket (mrnet_sockets[i], true, NULL, NULL, &stid);
    }
		
    std::vector<MRN::EndPoint *> endpoints =
        de->getMRNetNetwork()->get_BroadcastCommunicator()->get_EndPoints();
		
    //for all endpoints in the network, create a paradynd object and
    //the constructor puts the daemon in the allDaemons list
    pdvector<T_dyninstRPC::daemonSetupStruc> di;
    paradynDaemon * pd = NULL;
    for( unsigned int j=0; j<endpoints.size(); j++) {
        pdstring daemon_machine = endpoints[j]->get_HostName() ;
        
        // /* DEBUG */ fprintf( stderr,
        //         "Creating paradynDaemon: host:%s, name:%s, flavor:%s\n",
        //         daemon_machine.c_str(), de->getName(), de->getFlavor() );
        pd = new paradynDaemon(de->getMRNetNetwork(), endpoints[j],
                               daemon_machine,
                               (pdstring&)de->getLoginString(),
                               (pdstring&)de->getNameString(),
                               (pdstring&)de->getFlavorString()); 
        T_dyninstRPC::daemonSetupStruc * dss =
            new T_dyninstRPC::daemonSetupStruc;
        (*dss).daemonId = pd->get_id();
        (*dss).daemonName = pd->getMachineName( );
        di.push_back(*dss);
    }
		
    MRN::Stream * eqcStream = de->getMRNetNetwork()->
        new_Stream( de->getMRNetNetwork()->get_BroadcastCommunicator(),
                    MRN::TFILTER_PD_UINT_EQ_CLASS);
		
    MRN::Stream * defStream = de->getMRNetNetwork()->
        new_Stream( de->getMRNetNetwork()->get_BroadcastCommunicator(),
                    MRN::TFILTER_NULL,MRN::SFILTER_DONTWAIT);
    pdvector <int> countStreams = pd->setDaemonDefaultStream(defStream);
		
    MRN::Stream * bcStream = de->getMRNetNetwork()->
        new_Stream( de->getMRNetNetwork()->get_BroadcastCommunicator(),
                    MRN::TFILTER_NULL);
		
    pdvector <T_dyninstRPC::daemonInfo> daemon_info =
        pd->getDaemonInfo( bcStream, di );
		
    for( unsigned ij =0 ; ij < daemon_info.size() ; ij++) {
        pd = paradynDaemon::allDaemons[ij];
        if( started_manually || ( de->getFlavorString() == "mpi") ) {
            if(ij != (unsigned)daemon_info[ij].d_id) {
                MRN::Communicator * temp =
                    paradynDaemon::allDaemons[daemon_info[ij].d_id]->
                    getCommunicator();
                MRN::Communicator * temp1 = paradynDaemon::allDaemons[ij]->
                    getCommunicator();
              
                paradynDaemon::allDaemons[daemon_info[ij].d_id]->setCommunicator(temp1);
                paradynDaemon::allDaemons[ij]->setCommunicator(temp);
            }

            pd->machine = daemon_info[daemon_info[ij].d_id].machine;
            fprintf(stderr, "daemon[%d] machine:%s\n", ij,pd->machine.c_str() );
            if (pd->machine.suffixed_by(local_domain)) {
                const unsigned namelength = pd->machine.length() - local_domain.length() - 1;
              const pdstring localname = pd->machine.substr(0,namelength);
              pd->status = localname + ":" + pdstring( pd->get_id() );
            } 
            else {
                pd->status = pd->machine + ":" + pdstring(pd->get_id() );
            }
            uiMgr->createProcessStatusLine(pd->status.c_str());
        }
    }

    //TODO: we shoud make this persistent for default async downcalls
    delete bcStream;
		
    paradynDaemon::allDaemons[0]->setEquivClassReportStream(eqcStream);

    // Send the initial metrics, constraints, and other neato things
    MRN::Stream * nbcStream = de->getMRNetNetwork()->
        new_Stream( de->getMRNetNetwork()->get_BroadcastCommunicator(),MRN::TFILTER_NULL);
    paradynDaemon::allDaemons[0]->SendMDLFiles(nbcStream);
    delete nbcStream;	 
		
    nbcStream = de->getMRNetNetwork()->
        new_Stream( de->getMRNetNetwork()->get_BroadcastCommunicator(),MRN::TFILTER_NULL);
    pdvector<pdvector<T_dyninstRPC::metricInfo> > info =
        paradynDaemon::allDaemons[0]->getAvailableMetrics(nbcStream);
		
    for (unsigned kl = 0 ; kl < info.size() ; kl ++) {
        unsigned size = info[kl].size();
        for (unsigned u=0; u<size; u++) {
            addMetric(info[kl][u]);
        }
    }		
    delete nbcStream;
    paradynDaemon::allDaemons[0]->updateTimeAdjustment();
    return true;
}

daemonEntry *paradynDaemon::findEntry(const pdstring &n)
{
    for(unsigned i=0; i < allEntries.size(); i++){
        daemonEntry *newE = allEntries[i];
        if(newE->getNameString() == n) {
            return(newE);
        }
    }

    return ((daemonEntry*) 0);
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
pdvector<pdstring> *paradynDaemon::getAvailableDaemons()
{
	pdvector<pdstring> *names = new pdvector<pdstring>;
	
	daemonEntry *entry;
	for(unsigned i=0; i < allEntries.size(); i++)
		{
			entry = allEntries[i];
			*names += entry->getName();
		}
	return(names);
}

// For a given machine name, find the appropriate paradynd structure(s).
pdvector<paradynDaemon*> paradynDaemon::machineName2Daemon(const pdstring &mach)
{
    pdvector<paradynDaemon*> v;
    for (unsigned i=0; i < allDaemons.size(); i++) {
        paradynDaemon *theDaemon = allDaemons[i];
        if (theDaemon->getMachineName() == mach)
            v += theDaemon;
    }
    return v;
}

static bool hostIsLocal(const pdstring &machine)
{
	return (machine.length() == 0 ||
            machine == "localhost" || 
            getNetworkName(machine) == getNetworkName() ||
            getNetworkAddr(machine) == getNetworkAddr());
}

#if defined(os_aix)
static bool execPOE(const pdstring /* &machine*/, const pdstring /* &login */,
                    const pdstring /* &name   */, const pdstring         &dir,
                    const pdvector<pdstring> &argv, const pdvector<pdstring>  args,
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
         << strerror(errno) << endl;

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


static bool rshPOE(const pdstring  &machine, const pdstring   &login,
                   const pdstring /* &name*/,  const pdstring &dir,
                   const pdvector<pdstring> &argv,
                   const pdvector<pdstring> args,
                   daemonEntry *de)
{
  unsigned i;
  char *s[6];
  char  t[1024];
  pdstring appPath;
  pdstring pathResult;
  pdstring path = getenv("PATH");

  pdstring remoteShell = de->getRemoteShellString();
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

static bool startPOE(const pdstring &machine, const pdstring &login,
                     const pdstring &name,    const pdstring &dir,
                     const pdvector<pdstring> &argv, 
                     const pdvector<pdstring>  args,
                     daemonEntry *de)
{
    uiMgr->updateStatusLine(DMstatus.c_str(),   "ready");

  if (fork()) return(true);

  if (hostIsLocal(machine))
    return(execPOE(machine, login, name, dir, argv, args, de));
  else
    return( rshPOE(machine, login, name, dir, argv, args, de));
}
#endif

/*
  Pick a unique name for the wrapper
*/
pdstring mpichNameWrapper( const pdstring& dir )
{
   pdstring rv;
	

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
   rv += pdstring(getpid());
   rv += "-";

   struct timeval tv;
   gettimeofday( &tv, NULL );
   rv += pdstring(tv.tv_sec * 1000000 + tv.tv_usec);
   
   return rv;
}


#if !defined(os_windows)

static bool startMPI( daemonEntry *,
                      const pdvector<pdstring> & ,
                      const pdvector<pdstring> & );

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
	    pdstring cmd;
	    
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


bool writeMPICHWrapper(int fd, const pdstring& buffer )
{

    if (lseek(fd, 0, SEEK_SET) == (off_t)-1) {
        perror("Failed to write MPI wrapper (seek)");
        return false;
    }
    if (fchmod(fd, S_IRWXU | S_IRWXG | S_IRWXO) == -1) {
        perror("Failed to write MPI wrapper (chmod)");
        return false;
    }
    if (write(fd, buffer.c_str(), strlen(buffer.c_str())) !=
        (ssize_t)(strlen(buffer.c_str()))) {
        perror("Failed to write MPI wrapper (write)");
        return false;
    }
    if (close(fd) == -1) {
        perror("Failed to write MPI wrapper (close)");
        return false;
    }
   return true;
}

void addMRNetInfoToScript(pdstring &script, MRN::Network::LeafInfo **leafInfo, 
                          int nLeaves)
{
   script += pdstring("\n");
   for (int i=0; i<nLeaves; i++)
   {
       script += pdstring("# MRN ") + pdstring(leafInfo[i]->get_ParHost()) + 
           pdstring(" ") + pdstring(leafInfo[i]->get_ParPort()) + pdstring(" ") + 
           pdstring(leafInfo[i]->get_Rank()) + pdstring("\n");
   }
}

/*
  Create a script file which will start paradynd with appropriate
  parameters in a given directory. MPICH hides the user cmdline from
  slave processes until the MPI_Init() moment, so we pass these parameters
  in a script

  If successful, adds an entry to paradynDaemon::wrappers.
*/
bool mpichCreateWrapper ( const pdstring& script, daemonEntry *de, 
                          const pdstring& app_name,
                          const pdvector<pdstring> args,
                          bool has_explicit_wd,
                          const pdstring & app_dir )
{
    pdstring host = de->getMachineString();

    char dir[PATH_MAX];
    if ( de->getDirString().length() ) {
        strcpy(dir, de->getDir() );
    }
    else {
        getcwd(dir, PATH_MAX);
    }

	const char* shellspec = "#!/bin/sh\n";
	pdstring buffer;
	unsigned int j;

	assert( (dir != NULL) && (strlen(dir) != 0) );
	
	// dump the shell specifier
	buffer = pdstring(shellspec);

	// Set up for using the correct working directory for the process.
	// We have to be careful to respect the user's desire if
	// the user has explicitly specified a working directory for the process
	// If they have, we recognized it when we parsed the command line
	// and we don't mess with the process arguments as given by mpirun.
	// If they haven't, we need to modify the "working directory" argument
	// specified by mpirun to ensure it is the directory specified by
	// the user via the Paradyn PCL file or the Paradyn GUI.
	buffer += (pdstring("cd ") + pdstring(dir) + 
						 pdstring("\nPWD=") + pdstring(dir) +
						 pdstring("; export PWD\n"));
	if( !has_explicit_wd )
    {
			// The user didn't specify a working directory in the mpirun
			// command, so we need to munge the MPI process arguments to use the
			// directory given via Paradyn.
			// This method works for the non-MPD MPICH device, which uses
			// the p4wd specifier to give its working directory.
			// TODO how does MPD specify its working directory?

			buffer += (pdstring("substdir=") + app_dir + "\n");

			buffer += pdstring("appargs=`echo $* | sed \"s,p4wd [ ]*[^ ]*,p4wd $substdir,\"`\n");
    }
	else
    {
			buffer += pdstring("appargs=$*\n");
    }
	
	// dump the daemon command
	buffer += pdstring(de->getCommand());
	for (j=0; j < args.size(); j++) 
		{
			if (!strcmp(args[j].c_str(), "-l1")) 
				{
					buffer += " -l0";
				} 
			else
				{
					buffer += pdstring(" ") + args[j];
				}
		}
    if( app_dir.length() > 0 ){
        buffer += (pdstring(" -d") + app_dir);
    }
	buffer += (pdstring(" -z") + pdstring(de->getFlavor()));
	buffer += (pdstring(" -M") + pdstring(de->getMPItype()));
    buffer += pdstring(" -N $0");
	bool useLoops = tunableConstantRegistry::findBoolTunableConstant("EnableLoops").getValue();
	if (useLoops)
		buffer += pdstring(" -o");
	// Next add the arguments that define the process to be created.
	buffer += (pdstring(" -runme ") + app_name + pdstring(" $appargs "));

   addMRNetInfoToScript(buffer, de->getMRNetLeafInfo(),
                        de->getMRNetNumLeaves());

   if( hostIsLocal(host) ) {
      // the file named by script is our wrapper - write the script to the
      // wrapper file
      int fd = open(script.c_str(), O_WRONLY | O_CREAT);
      if( fd == -1 ){
          fprintf(stderr, "open(\"%s\") failed.\n", script.c_str());
          perror("open()");
          exit(-1);
      }
      if( !writeMPICHWrapper(fd, buffer)) 
      {
         cerr << "Perhaps the current directory isn't writeable?" << endl;
         return false;
      }      
      paradynDaemon::wrappers += paradynDaemon::MPICHWrapperInfo(script);
      
      return true;
   }
   
	// the file named by script is the wrapper's remote name - we write the
	// script to a local temporary file so as to copy it to the remote system
	char templ[40] = "pd.wrapper.XXXXXX";
	int tempfd = mkstemp(templ);
	// mkstemp also writes the name into temp
	pdstring localWrapper = templ;   
	if( !writeMPICHWrapper(tempfd, buffer)) {
		return false;
	}
   
	// try to copy the file to the desired location on the remote system
	// ideally, we'd use execCmd() to execute this copy command on the
	// local system, but execCmd has some side effects we'd rather not try
	// to work around
	bool copySucceeded = true;
	int pid = fork();

	if(pid > 0) 
		{
			// we are the parent - 
			// wait for our child process (which will do the copy) to complete
			int copyStatus;
			if(waitpid( pid, &copyStatus, 0 ) != pid )
				{
					cerr << "mpichCreateWrapper: Failed to copy temporary local "
							 << "wrapper to remote system: " 
							 << strerror(errno) << endl;
					copySucceeded = false;
				}
		else 
			{
				if(copyStatus != 0)
					{
						copySucceeded = false;
					}
			}
		}
	else if(pid == 0) 
		{
			// we are the child - we exec our copy command...
			
			// ...build the command string...
			pdstring cmd;
			cmd += de->getRemoteShellString();
			cmd += " ";
			cmd += host;		// name of the remote system
			cmd += " cat - \">\" ";
			cmd += script;			// name of wrapper on remote system
			cmd += " \";\" chmod 755 ";
			cmd += script;
			cmd += " < ";
			cmd += localWrapper;	// name of wrapper on local system
			
			// ...exec a shell to execute it
			execl("/bin/sh", "sh", "-c", cmd.c_str(), NULL);
			copySucceeded = false;	// if we got here, the exec failed
		} 
	else
		{
			cerr << "mpichCreateWrapper: fork failed." << strerror(errno) << endl;
			copySucceeded = false;
		}

	// release the local temporary wrapper file
	if(remove(localWrapper.c_str()) < 0) 
		{
			cerr << "mpichCreateWrapper: Failed to remove temporary local wrapper"
					 << " " << localWrapper << ": " << strerror(errno) << endl;
		}
	if(copySucceeded) 
		{
			// keep info around till later about script that needs to be removed
			paradynDaemon::wrappers += paradynDaemon::MPICHWrapperInfo(script, 
																																 host, de->getRemoteShellString());
		}

	if(!copySucceeded) {
		return false;
	}
	return true;
} 


extern void appendParsedString(pdvector<pdstring> &strList, const pdstring &str);

/*
  Handle remote startup case
*/
void mpiRemote( daemonEntry *de, const char *daemon_dir,
                pdvector<pdstring> &params)
{
	pdstring rsh = de->getRemoteShellString();
	assert(rsh.length() > 0);
	appendParsedString(params, rsh);

	if (de->getLoginString().length() != 0) {
		params += pdstring("-l");
		params += de->getLoginString();
	}
    if( de->getMachineString().length() != 0 )
    {
        params += de->getMachineString();
    }
    else
    {
        params += "localhost";
    }
	params += pdstring("cd");
	params += pdstring(daemon_dir);
	params += pdstring(";");
}
	
//added member num_values to known_arguments, because sometimes
//in LAM the argument might have more than one thing following it
//e.g.  -ssi rpi usysv

struct known_arguments {
	char* name;
	bool has_value;
	bool supported;
        int num_values;
};

 //      added function
 //      returns true if all characters in char array are numbers
 //      returns false o.w.

bool allCharsNumeric( int len, const char * array){
   for(int c = 0; c < len; ++c){
       if(!isdigit(array[c]))
           return false;
   }
   return true;
}

//a function to parse out int ranges from strings like '10-200'
//len is how many chars before the '-' character
//returns true if the string represents a valid node spec
//returns false otherwise
bool lamParseNodeSpecDash(char * no,int len){
      int first;
      bool numeric = false;

      numeric = allCharsNumeric(len,no);
      if(!numeric)
          return false;
      first = atoi(no);
      int len2 =  strlen(no) - len - 1;  //get rid of chars before - and the -
      numeric = allCharsNumeric(len2,no+len+1);
       if(!numeric)
           return false;
       return true;
}

//a function to parse out nxxx or cxxx commandline args to mpirun
//returns false if the string doesn't match a valid node or cpu spec.
//then we assume that it's the name of their MPI program

bool lamParseNodeSpec(const pdstring& spec ){
  const char * temp;
  char *no = NULL;
  int k;
  char * ptr;
  bool good = false;

  temp = spec.c_str();
  no = (char *) malloc((spec.length() + 1) * sizeof(char));
  char * comma = strchr(temp,',');
  char * dash = strchr(temp,'-');

  // if nodespec like n0-23,27 or something crazy like that
  if(comma){
      k = comma - temp;
      strncpy(no,temp+1,k-1);
      no[k-1] = '\0';
      dash = strchr(no,'-');
      if(dash){
         good = lamParseNodeSpecDash(no,dash-no);
         if(!good)
	 {
	   free(no);
           return false;
	 }
      }
      else{
         good = allCharsNumeric(k-2,no);
         if(!good)
	 {
	   free(no);
           return false;
	 }
      }
      ptr = comma;           //hold the previous comma spot
      //loop through each comma delimited section and parse out node numbers
      for(comma = strchr(temp+k+1,','); ptr; comma = strchr(ptr+1,',')){
         if(comma)
            k = comma - ptr;
         else
            k = temp + strlen(temp) - ptr;  //length from comma to comma
         strncpy(no,ptr+1,k-1);             //get the string without commas
         no[k-1] = '\0';
         dash = strchr(no,'-');
         if(dash){
            good = lamParseNodeSpecDash(no,dash-no);
            if(!good)
	    {
	      free(no);
	      return false;
	    }
         }
         else {
            good = allCharsNumeric(k-2,no);
            if(!good)
	    {
	      free(no);
	      return false;
	    }
         }
         if(!comma)  //no more specs to parse out, whew
            break;
         ptr = comma;
      }
  }
// else  if like n10-20 only
  else if(dash){
      strncpy(no,temp+1,strlen(temp)-1);  //exclude 'n'  or 'c'
      no[strlen(temp)-1] = '\0';
      good = lamParseNodeSpecDash(no,dash-temp-1);
      if(!good)
      { 
	free(no);
	return false;
      }
  }
  // else nodespec like n10 only
  else{
      strcpy(no,temp+1);  //exclude 'n' or 'c'
      good = allCharsNumeric(strlen(temp)-2,no);
      if(!good) 
      {
	free(no);
	return false;
      }
  }
  return true;
}


/*
  Split the command line into three parts:
  (mpirun arguments, the application name, application parameters)
  Insert the script name instead of the application name
*/
bool lamParseCmdline(const pdstring& script, const pdvector<pdstring> &argv,
                       pdstring & app_name,
                       pdvector<pdstring> &params,
                       bool& has_explicit_wd )
{  
   const unsigned int NKEYS = 31;
   struct known_arguments known[NKEYS] = {
      {"-f", false, true, 0},           {"-c", true, true, 1},
      {"-np", true, true, 1},           {"-D", false, true, 0},
      {"-wd", true, true, 1},           {"-ger", false, true, 0},
      {"-nger", false, true, 0},        {"-sigs", false, true, 0},
      {"-nsigs", false, true, 0},       {"-ssi", true, true, 2},
      {"-nw", false, true, 0},          {"-w", false, true, 0},
      {"-nx", false, true, 0},          {"-ptr", false, true, 0},
      {"-npty", false, true, 0},        {"-s", true, true, 1},
      {"-t", false, true, 0},           {"-toff", false, true, 0},
      {"-ton", false, true, 0},         {"-tv", false, false, 0},
      {"-x", false, false, 0},          {"-p", true, true, 1},
      {"-sa", false, true, 0},          {"-sf", false, true, 0},
      {"C", false, true, 0},            {"P", false, true, 0},
      {"-h", false, false, 0},          {"-v", false, true,0},
      {"-O", false, true, 0},           {"-c2c", false, true, 0},
      {"N", false,true,0}		
   };
   bool app = false, found_mpirun = false;
   bool in_known = false;
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
      in_known = false;

      for (k=0; k<NKEYS; k++) {
         if (argv[i] == known[k].name) {
            app = false;
            in_known = true;

            if (!known[k].supported) {
               pdstring msg = pdstring("Argument \"") + pdstring(known[k].name) +
                            pdstring("\" is not supported");
              uiMgr->showError(113, strdup(msg.c_str()));
               return false;
            }

            // check whether the user explicitly specified a working directory
            if( (argv[i] == "-wd") || argv[i] == "-D") {
               has_explicit_wd = true;
            }

            params += argv[i++];

            if (known[k].has_value) {
               // Skip the next arg
               if (i >= argv.size()) {
                  uiMgr->showError(113, "LAM/MPI command line parse error");
                  return false;
               }
               //loop through args for the value
               for(int z = 0; z < known[k].num_values; ++z)
                 params += argv[i++];
            }
            break;
         }
      }//end for

      //check for n0 n1-5 etc -- node specification
      //OR check for c0 c1-5 etc  -- cpu specification
      // the reason for parsing it is so we know if the string is a node
      //spec, or is a cpu spec, or is the executable name
      if(!in_known){
            bool retVal = false;
            if(argv[i].prefixed_by("n") || argv[i].prefixed_by("c"))
               retVal= lamParseNodeSpec(argv[i]);
            if(retVal){
               app = false;  //if it was a node or spec, it's not the app
               params += argv[i++];
            }
      }
   }//end while

   if (!app) {
      uiMgr->showError(113, "LAM/MPI command line parse error");
      return false;
   }
   
   params += script;
   app_name += argv[i++] ;

   for (; i < argv.size(); i++) {
     params += argv[i];
   }

   return true;
}

bool mpichParseCmdline(const pdstring& script, const pdvector<pdstring> &argv,
                       pdstring& app_name,
                       pdvector<pdstring> &params,
                       bool& has_explicit_wd) 
{
   const unsigned int NKEYS = 39;
   struct known_arguments known[NKEYS] = {
      {"-arch", true, true, 1},           {"-h", false, false, 0},
      {"-machine", true, true, 1},        {"-machinefile", true, true, 1},
      {"-np", true, true, 1},             {"-nolocal", false, true, 0},
      {"-stdin", true, true, 1},          {"-t", false, false, 0},
      {"-v", false, true, 0},             {"-dbx", false, false, 0},
      {"-gdb", false, false, 0},          {"-xxgdb", false, false, 0},
      {"-tv", false, false, 0},           {"-batch", true, true, 1},
      {"-stdout", true, true, 1},         {"-stderr", true, true, 1},
      {"-nexuspg", true, true, 1},        {"-nexusdb", true, true, 1},
      {"-e", false, true, 0},             {"-pg", false, true, 0},
      {"-leave_pg", false, true, 0},      {"-p4pg", true, false, 1},
      {"-tcppg", true, false, 1},         {"-p4ssport", true, true, 1},
      {"-p4wd", true, true, 1},           {"-all-local", false, true, 1},
      {"-mvhome", false, false, 0},       {"-mvback", true, false, 1},
      {"-maxtime", true, true, 1},        {"-nopoll", false, true, 0},
      {"-mem", true, true, 1},            {"-cpu", true, true, 1},
      {"-cac", true, true, 1},            {"-paragontype", true, true, 1},
      {"-paragonname", true, true, 1},    {"-paragonpn", true, true, 1},
      {"-wdir", true, true, 1},//added for chp4_mpd MPICH - it's like p4wd
      {"-hf", true, true, 1},//MPICH2 - it's like machinefile
      {"-1", false, true, 0} //MPICH2 -start 1st proc non-locally
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
   while (!app && i < argv.size()) 
		 {
			 app = true;
      
			 for (k=0; k<NKEYS; k++)
				 {
					 if (argv[i] == known[k].name) 
						 {	    
							 app = false;
	    
							 if (!known[k].supported)
								 {
									 pdstring msg = pdstring("Argument \"") + pdstring(known[k].name) + 
										 pdstring("\" is not supported");
									 uiMgr->showError(113, strdup(msg.c_str()));
									 return false;
								 }

							 // check whether the user explicitly specified a working directory
							 if( (argv[i] == "-p4wd" || argv[i] == "-wdir") )
								 {
									 has_explicit_wd = true;
								 }
	    
							 params += argv[i++];
	    
							 if (known[k].has_value)
								 {
									 // Skip the next arg
									 if (i >= argv.size()) 
										 {
											 uiMgr->showError(113, "MPICH command line parse error");
											 return false;
										 }
									 //loop through args for the value
									 for(int z = 0; z < known[k].num_values; ++z)
										 {
											 params += argv[i++];
										 }
								 }
							 break;
						 }
				 }
		 }

   if (!app)
		 {
			 uiMgr->showError(113, ":MPICH command line parse error:");
			 return false;
		 }
	 
   params += script;
   app_name = argv[i++] ;
   
   for (; i < argv.size(); i++) 
   {
      params += argv[i];
   }
   return true;
}

bool slurmParseCmdline(const pdstring& script, const pdvector<pdstring> &argv,
                       pdstring& app_name,
                       pdvector<pdstring> &params,
                       bool& has_explicit_wd) 
{

   const unsigned int NKEYS = 74;
   struct known_arguments known[NKEYS] = {
      {"-n", true, true, 1}, {"--ntasks", true, true, 1},
      {"-c", true, true, 1}, {"--cpus-per-task", true, true, 1},
      {"-N", true, true, 1},  {"--nodes", true, true, 1},
      {"-r", true, true, 1}, {"--relative", true, true, 1},
      {"-p", true, true, 1}, {"--partition", true, true, 1},
      {"-t", true, true, 1}, {"--time", true, true, 1},
      {"-D", true, true, 1}, {"--chdir", true, true, 1},
      {"-I", false, true, 0},{"--immediate", false, true, 0},
      {"-k", false, true, 0},{"--no-kill", false, true, 0},
      {"-s", false, true, 0},{"--share", false, true, 0},
      {"-O", false, true, 0},{"--overcommit", false, true, 0},
      {"-T", true, true, 1}, {"--threads", true, true, 0},
      {"-l", false, true, 0},{"--label", false, true, 0},
      {"-u", false, true, 0},{"--unbuffered", false, true, 0},
      {"-m", true, true, 1}, {"--distribution", true, true, 1},
      {"-J", true, true, 1}, {"--job-name", true, true, 1},
      {"--mpi", true, true, 1}, {"--job-id", true, true, 1},
      {"-o", true, true, 1}, {"--output", true, true, 1},
      {"-i", true, true, 1}, {"--input", true, true, 1},
      {"-e", true, true, 1}, {"--error", true, true, 1},
      {"-b", false, true, 0},{"--batch", false, true, 0},
      {"-v", false, true, 0},{"--verbose", false, true, 0},
      {"-d", true, true, 1}, {"--slurmd-debug", true, true, 1},
      {"-W", true, true, 1}, {"--wait", true, true, 1},
      {"-q", false, true, 0},{"--quit-on-interrupt", false, true, 0},
      {"-X", false, true, 0},{"--disable-status", false, true, 0},
      {"-Q", false, true, 0},{"--quiet", false, true, 0},
      {"--uid", true, true, 1}, {"--gid", true, true, 1},
      {"--core", true, true, 1},
      //allocate just obtains resources and spawns a shell; not supported
      {"-A", true, false, 1}, {"--allocate", true, false, 1},
      //no-shell allocates resources and exits; not supported
      {"--no-shell", false, false, 0},
      //attach to, join with, steal from running job; not supported
      {"-a", true, false, 1}, {"--attach", true, false, 1},
      {"-j", false, false, 0}, {"--join", false, false, 0},
      {"--steal", false, false, 0},

      {"--mincpus", true, true, 1}, {"--mem", true, true, 1},
      {"--tmp", true, true, 1},
      {"-C", true, true, 1}, {"--constraint", true, true, 1},
      {"-w", true, true, 1}, {"--nodelist", true, true, 1},
      {"-x", true, true, 1}, {"--exclude", true, true, 1}
   };

   bool app = false, found_mpirun = false;
   unsigned int i = 0, k;

   while (i < argv.size() && !found_mpirun) {
      found_mpirun = (strstr(argv[i].c_str(), "srun") != 0);
      params += argv[i++];
   }

   if (!found_mpirun) {
      uiMgr->showError(113, "Expected: \"srun <command>\"");
      return false;
   }

   has_explicit_wd = false;
   while (!app && i < argv.size()) {
      app = true;

      for (k=0; k<NKEYS; k++) {
         if (argv[i] == known[k].name) {
            app = false;

            if (!known[k].supported) {
               pdstring msg = pdstring("Argument \"") + pdstring(known[k].name) + 
                         pdstring("\" to srun is not supported");
               cerr<<msg<<endl;
               uiMgr->showError(113, strdup(msg.c_str()));
               return false;
            }

        // check whether the user explicitly specified a working directory
        if( (argv[i] == "-D" || argv[i] == "--chdir") )
        {
            has_explicit_wd = true;
        }

            params += argv[i++];

            if (known[k].has_value) {
               // Skip the next arg
               if (i >= argv.size()) {
                  uiMgr->showError(113, "SLURM command line parse error");
                  return false;
               }
              //loop through args for the value
               for(int z = 0; z < known[k].num_values; ++z)
                  params += argv[i++];
            }
            break;
         }
      }
   }

   if (!app) {
      uiMgr->showError(113, "SLURM command line parse error");
      return false;
   }

   params += script;
   app_name = argv[i++] ;

   for (; i < argv.size(); i++) {
      params += argv[i];
   }

   return true;
}

/* loops through argv looking for an argument that 
   corresponds to an MPI start command, mpirun or srun.
   It returns the pdstring "mpirun" if mpirun is used and
   "SLURM" if srun is used. Otherwise, the empty string is 
   returned.
*/
pdstring mpiGetStartCmd(const pdvector<pdstring> & argv){
   
   bool found_mpirun = false;
   bool found_srun = false;
   unsigned i = 0;
   while (i < argv.size() ) {
      found_mpirun = (strstr(argv[i].c_str(), "mpirun") != 0);
      if (found_mpirun)
         return pdstring("mpirun");
      found_srun = (strstr(argv[i].c_str(), "srun") != 0);
      if (found_srun)
         return pdstring("SLURM");
      ++i;
   }
   return pdstring("");
}
static int getNumberOfNodes( const pdvector<pdstring> &argv)
{
	pdstring target;
	pdstring startUp = mpiGetStartCmd(argv);

	if (startUp == "srun")
		target = "-n";
	else
		target = "-np";

	bool found = false;
	for( unsigned int i=0; i < argv.size(); i++ ) {
        found = (strstr(argv[i].c_str(), target.c_str()) != 0);
        if (found) {
            assert( i != argv.size()-1 );
            return atoi(argv[i+1].c_str());
        }
   }
   return 1;
}


/*
  Initiate the MPICH startup process: start the master application
  under paradynd
*/
static bool startMPI( daemonEntry *de,
                      const pdvector<pdstring> &process_argv,
                      const pdvector<pdstring> &paradynd_args )
                     
{
    pdstring app_name;
    pdvector<pdstring> params;
    unsigned int i;
    char daemon_dir[PATH_MAX];
    
    pdvector<pdstring> Argv;
    //----------------------------------------------
    char* ar[64];
    char** ars;
    ars = ar;
    char *line = strdup(process_argv[1].c_str());
    char *orig_line = line;
    pdstring app_dir = process_argv[5];

    while (*line != '\0') {       /* if not the end of line ....... */ 
        while (*line == ' ' || *line == '\t' || *line == '\n') {
            *line++ = '\0';     /* replace white spaces with 0    */
        }
        *ars++ = line; /* save the argument position     */
        while (*line != '\0' && *line != ' ' && 
              *line != '\t' && *line != '\n') { 
            line++;             /* skip the argument until ...    */
        }
    }
    *ars = '\0';
    ars = ar;
    while(*ars !='\0') {
        Argv.push_back(*ars);
        *ars++;
    }
    free(orig_line);

    uiMgr->updateStatusLine(DMstatus.c_str(),   "ready");
   
    if ( de->getDirString().length() ) {
        strcpy(daemon_dir, de->getDir());
    }
    else {
        getcwd(daemon_dir, PATH_MAX);
    }
   
    // find out if we are using MPICH or LAM or SLURM
    pdstring whichMPI = de->getMPItypeString();
    if(whichMPI.length() == 0){
        whichMPI = pdstring("MPICH");
    }
   
    if( !hostIsLocal( de->getMachineString() )
        || ( de->getLoginString().length() > 0) ) {
        // run mpirun via rsh/ssh if using remote host or login specified
        mpiRemote(de, daemon_dir, params);
    }
	 

    pdstring script = mpichNameWrapper(daemon_dir);

    bool has_explicit_wd = false;
    bool parseRet = false;
    pdstring startUp = mpiGetStartCmd(Argv);
    if (startUp == ""){
        cerr<<"could not find MPI command"<<endl;
        uiMgr->showError(113, "Could not find MPI command, expected \"mpirun\" or \"srun\"");
        return false;
    }
    else if(startUp == "mpirun" && whichMPI == "MPICH") {
        parseRet = mpichParseCmdline(script, Argv, app_name, params, 
                                     has_explicit_wd);
    }
    else if(startUp == "mpirun" && whichMPI == "LAM") {
        parseRet = lamParseCmdline(script, Argv, app_name,  params, 
                                   has_explicit_wd);
    }
    else if(startUp == "SLURM") {
        parseRet = slurmParseCmdline(script, Argv, app_name,  params, 
                                     has_explicit_wd);
    }
    if (!parseRet) {
        return false;
    }
   
    bool createRet = mpichCreateWrapper(script, de, app_name, paradynd_args,
                                        has_explicit_wd, app_dir);
    if (!createRet) {
        return false;
    }
   
    char **cmd = new char *[ params.size() + 4 ];
    if ( cmd == NULL ) {
        fprintf(stderr, "Internal error: memory allocation failed.\n");
        exit(-1);
    }

    for (i=0; i<params.size(); i++) {
        if(params[i] != NULL)
            cmd[i] = strdup(params[i].c_str());
        else
            cmd[i] = strdup("");
    }
   
    cmd[i] = 0;

    if ( fork() )
        return true;

    // Close Tk X connection to avoid conflicts with parent
    uiMgr->CloseTkConnection();
   
    if (execvp(cmd[0], cmd) < 0) {
        uiMgr->showError(113, "Failed to start MPI");
    }
    return false;
}

bool paradynDaemon::startMPIDaemonsandApplication(daemonEntry * ide,
                                                  processMet* proc )
{
#if defined(os_aix)
    //startPOE(machine, login, name, dir, argv, args, def, leafInfo);
#else
    pdvector<pdstring> argv;
    argv.push_back( proc->name());
    argv.push_back( proc->command());
    argv.push_back( proc->daemon());
    argv.push_back( proc->host());
    argv.push_back( proc->user());
    argv.push_back( proc->execDir());
    if( proc->autoStart() )
        argv.push_back( pdstring("true"));
    else
        argv.push_back( pdstring("false"));
    
    startMPI(ide, argv, args);
#endif
				
    if( ide->getMRNetNetwork()->connect_Backends() < 0)
        fprintf(stderr,"MRN::Network::connect_Backends() failed\n");

    return true;
}

#endif // !defined(i386_unknown_nt4_0)

// add a new executable (binary) to a program.
// for non-mpi jobs, start mrnet with daemon(s) if not already started
//     - then add executable to the daemon
// for mpi jobs, this launches mrnet, and then the daemons and app. via mpirun 
bool paradynDaemon::newExecutable(const pdstring &ihost,
                                  const pdstring &iuser,
                                  const pdstring &idaemon_name, 
                                  const pdstring &idir,
                                  const pdvector <pdstring> &iargv)
{
    if ( !DMstatus_initialized) {
        uiMgr->createStatusLine(DMstatus.c_str());
        DMstatus_initialized = true;
    }

    //Do we have a daemon by this name?
    daemonEntry *de = findEntry( idaemon_name ) ;
    if (!de) {
        fprintf( stderr, "Paradyn daemon \"%s\" not defined.",
                 idaemon_name.c_str() );
        exit(-1);
    }

    if( de->getFlavorString() == "mpi" ){
#if defined(i386_unknown_nt4_0)
        fprintf(stderr, "MPI applications for WinNT not supported!");
        exit(-1);
#else
        //TODO: find process, fix getnumberofnodes to only parse command
        int process_count = getNumberOfNodes(iargv);
        instantiateMRNetforMPIDaemons( de, process_count );
        startMPIDaemonsandApplication( de, processMet::allProcs[0] );

        initializeDaemon(de);
#endif
    }
    else{
        //non-mpi application
        paradynDaemon *daemon = getDaemon( ihost, iuser, idaemon_name );

        if( daemon == NULL){
            pdvector < pdstring > hosts;
            hosts.push_back( ihost );
            instantiateDefaultDaemon( de, &hosts );
            if( de->getMRNetNetwork()->fail() ) {
                pdstring msg = pdstring("Failed: MRNet new network.");
                //TODO: create new error code
                uiMgr->showError(90,P_strdup(msg.c_str()));
                return false;
            }
    
            initializeDaemon(de);
        }

        //try again, now the daemon should now exist
        daemon = getDaemon( ihost, iuser, idaemon_name );
        assert(daemon);

        performanceStream::ResourceBatchMode(batchStart);

        MRN::Stream * stream =
            daemon->network->new_Stream( daemon->getCommunicator(),
                                         MRN::TFILTER_NULL);
        pdstring wdir;
        wdir = idir.c_str() ? idir : "";

        pdvector<int> pid = daemon->addExecutable(stream, iargv, wdir);
        delete stream;

        performanceStream::ResourceBatchMode(batchEnd);
        bool err_found = false;

        for(unsigned zz = 0 ; zz < pid.size(); zz++) {
            if (!(pid[zz] > 0 && !daemon->did_error_occur())) {
               return false;
            }
        }
    }

    return true;
}


bool paradynDaemon::attachStub(const pdstring &machine,
                               const pdstring &userName,
                               const pdstring &cmd, // program name (full path)
                               int the_pid,
                               const pdstring &daemonName,
                         int afterAttach // 0 --> as is, 1 --> pause, 2 --> run
                               ) {
  // Note: by this time, both the RUN and PAUSE buttons have been disabled in the
  // user interface...

   if (! DMstatus_initialized) {
       uiMgr->createStatusLine(DMstatus.c_str());
       DMstatus_initialized = true;
   }

   //Do we have a daemon by this name?
   daemonEntry *de = findEntry( daemonName ) ;
   if (!de) {
      fprintf( stderr, "Paradyn daemon \"%s\" not defined.",
               machine.c_str() );
      exit(-1);
   }

   paradynDaemon *daemon = getDaemon(machine, userName, daemonName);
   if( daemon == NULL){
      pdvector < pdstring > hosts;
      hosts.push_back( machine );
      instantiateDefaultDaemon( de, &hosts );
      if( de->getMRNetNetwork()->fail() ) {
         pdstring msg = pdstring("Failed: MRNet new network.");
         //TODO: create new error code
         uiMgr->showError(90,P_strdup(msg.c_str()));
         return false;
      }
      initializeDaemon(de);
   }

  daemon = getDaemon(machine, userName, daemonName);
  if (daemon == NULL)
      return false;

  daemon->afterAttach_ = afterAttach;
  performanceStream::ResourceBatchMode(batchStart);

  MRN::Stream * stream = daemon->network->new_Stream(daemon->network->get_BroadcastCommunicator(),MRN::TFILTER_NULL);

  pdvector<bool> success = daemon->attach(stream, cmd, the_pid, afterAttach);
  delete stream;

  performanceStream::ResourceBatchMode(batchEnd);

  if (daemon->did_error_occur())
     return false;

  if (!success[0])
     return false;

  return true; // success
}

//
// start the programs running.
//
bool paradynDaemon::startApplication()
{
   MRN::Stream * stream = programs[0]->controlPath ->network->new_Stream(programs[0]->controlPath->network->get_BroadcastCommunicator(),MRN::TFILTER_NULL);
    //TODO: is there a way to broadcast this request? What is pid used for?
    //for(unsigned i=0; i < programs.size(); i++){
   //MRN::Stream * stream = programs[i]->controlPath->network->
   //         new_Stream( programs[i]->controlPath->communicator ) ;

        programs[0]->controlPath->startProgram(stream, programs[0]->pid);   
	     delete stream;
	// }

    return(true);
}

//
// pause all processes.
//
bool paradynDaemon::pauseAll()
{
  MRN::Communicator *comm = paradynDaemon::allDaemons[0]->network->
    new_Communicator() ;
  for(unsigned i = 0; i < paradynDaemon::allDaemons.size(); i++){
    comm->add_EndPoint( paradynDaemon::allDaemons[i]->endpoint );
  }
  
  MRN::Stream * stream = paradynDaemon::allDaemons[0]->network->new_Stream( comm );
  
  //TODO: there should be a static func that doesn't need a daemon to
  //broadcast commands
  paradynDaemon::allDaemons[0]->pauseApplication(stream);
  
  delete stream;
  delete comm;
  
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
        MRN::Stream * stream = exec->controlPath->network->
            new_Stream( exec->controlPath->communicator );
        exec->controlPath->pauseProgram(stream, exec->pid);
        delete stream;
        return(true); 
    } else
	return (false);
}

//Send message to each daemon to instrument the dynamic call
//sites in function "func_name"
bool paradynDaemon::AllMonitorDynamicCallSites(pdstring func_name){
    //TODO: right now all broadcast funcs assume all daemons live on the
    //      same mrnet network (darnold)

    MRN::Communicator *comm = paradynDaemon::allDaemons[0]->network
        ->new_Communicator();
    for(unsigned i = 0; i < paradynDaemon::allDaemons.size(); i++){
        comm->add_EndPoint( paradynDaemon::allDaemons[i]->endpoint );
    }

    MRN::Stream * stream = paradynDaemon::allDaemons[0]->network->new_Stream( comm );

    //TODO: there should be a static func that doesn't need a daemon to
    //broadcast commands
	paradynDaemon::allDaemons[0]->MonitorDynamicCallSites(stream, func_name);

    delete stream;
    delete comm;


    return true;
}

//
// continue all processes.
//
bool paradynDaemon::continueAll()
{
    if (programs.size() == 0)
       return false; // no program to pause

    if (procRunning == 0)
       return false;

    //MRN::Stream * stream = programs[0]->controlPath->network->new_Stream(programs[0]->controlPath->network->get_BroadcastCommunicator(),MRN::TFILTER_NULL);


    MRN::Communicator *comm = paradynDaemon::allDaemons[0]->network
        ->new_Communicator();
    for(unsigned i = 0; i < paradynDaemon::allDaemons.size(); i++){
        comm->add_EndPoint( paradynDaemon::allDaemons[i]->endpoint );
    }

    MRN::Stream * stream = paradynDaemon::allDaemons[0]->network->new_Stream( comm );

    //TODO: there should be a static func that doesn't need a daemon to
    //broadcast commands
    paradynDaemon::allDaemons[0]->continueApplication(stream);
    delete stream;
    delete comm;

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
        MRN::Stream * stream = exec->controlPath->network->
            new_Stream( exec->controlPath->communicator );
        exec->controlPath->continueProgram(stream, exec->pid);
        delete stream;
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
   MRN::Stream * stream = programs[0]->controlPath->network->new_Stream(programs[0]->controlPath->network->get_BroadcastCommunicator(),MRN::TFILTER_NULL);

   paradynDaemon::allDaemons[0]->detachProgram(stream, 0,pause);
   delete stream;
/*
    for(unsigned i=0; i < programs.size(); i++){
        MRN::Stream * stream = programs[i]->controlPath->network->
            new_Stream( programs[i]->controlPath->communicator );
        programs[i]->controlPath->detachProgram(stream, programs[i]->pid,pause);
        delete stream;
    }
*/
    return(true);
}

//
// print the status of each process.  This is used mostly for debugging.
//
void paradynDaemon::printStatus()
{
    for(unsigned i=0; i < programs.size(); i++){
        MRN::Stream * stream = programs[i]->controlPath->network->
            new_Stream( programs[i]->controlPath->communicator );
        pdvector<pdstring> status = programs[i]->controlPath->
            getStatus(stream, programs[i]->pid);
        delete stream;
        if (!programs[i]->controlPath->did_error_occur()) {
            cout << status[0] << endl;
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
    for(unsigned i=0; i < programs.size(); i++){
        if ((programs[i]->pid == (unsigned)pid) || (pid == -1)) {
            MRN::Stream * stream = programs[i]->controlPath->network->
                new_Stream( programs[i]->controlPath->communicator );
            programs[i]->controlPath->coreProcess(stream, programs[i]->pid);
            delete stream;
        }
    }
}


bool paradynDaemon::setInstSuppress(resource *res, bool newValue)
{
    MRN::Communicator * comm = paradynDaemon::allDaemons[0]->network->
        new_Communicator();
    for(unsigned i = 0; i < paradynDaemon::allDaemons.size(); i++){
        comm->add_EndPoint( paradynDaemon::allDaemons[i]->endpoint );
    }
    MRN::Stream * stream = paradynDaemon::allDaemons[0]->network->
        new_Stream( comm );

    pdvector <bool> ret = paradynDaemon::allDaemons[0]->
        setTracking(stream, res->getHandle(), newValue);

    delete stream;
    delete comm;

    bool retval = false;
    for (unsigned int ii=0; ii<ret.size(); ii++){
        retval |= ret[ii];
    }
    return(retval);
}

//
// signal from daemon that is is about to start or end a set 
// of new resource definitions
//
void paradynDaemon::resourceBatchMode(MRN::Stream *, bool onNow)
{

  int prev = countSync;
  if (onNow)
    {
      countSync++;
    }
  else
    {
			assert(countSync > 0);
      countSync--;
    }
  
  if (countSync == 0) 
    {
      for(u_int i=0; i < allDaemons.size(); i++)
				{
					(allDaemons[i])->reportResources();
				}
      performanceStream::ResourceBatchMode(batchEnd);
    }
  else if (!prev)
    {
      performanceStream::ResourceBatchMode(batchStart);
    }
}

extern void ps_retiredResource(pdstring resource_name);

void paradynDaemon::retiredResource(MRN::Stream *, pdstring resource_name) {
   ps_retiredResource(resource_name);
}


//
//  reportResources:  send new resource ids to daemon
//
void  paradynDaemon::reportResources()
{
	
  assert(newResourceTempIds.size() == newResourceHandles.size());
	
	if(newResourceTempIds.size() > 0 )
		{
			paradynDaemon * pd = paradynDaemon::allDaemons[0];
			MRN::Stream * stream =  pd->getNetwork()->new_Stream( pd->getNetwork()->get_BroadcastCommunicator(),MRN::TFILTER_NULL);
			pd->resourceInfoResponse(stream, newResourceTempIds, newResourceHandles);
			delete stream;
			newResourceTempIds.resize(0);
			newResourceHandles.resize(0);
		}
}

bool paradynDaemon::isMonitoringProcess(int pid) {
   for(unsigned i=0; i<pidsThatAreMonitored.size(); i++) {
      if(pidsThatAreMonitored[i] == pid)
         return true;
   }
   return false;
}

void paradynDaemon::addProcessInfo(const pdvector<pdstring> &resource_name) {
   // need at least /Machine, <machine>, process
   if(resource_name.size() < 3)
      return;
   
   const pdstring &process_str = resource_name[2];
   pdstring proc_name;
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

void paradynDaemon::resourceEquivClassReportCallback(MRN::Stream * /*s*/, 
						     pdvector<T_dyninstRPC::equiv_class_entry> eqclasses )
{
    if(eqclasses.size() == 0)
        return;

    MRN::Communicator * comm = allDaemons[0]->getNetwork()->new_Communicator();

    extern unsigned num_dmns_to_report_resources;

    for(unsigned i = 0 ; i < eqclasses.size() ; i++) {
        comm->add_EndPoint(  paradynDaemon::allDaemons[eqclasses[i].class_rep]->endpoint );
        num_dmns_to_report_resources++;
    }
    MRN::Stream * stream = network->new_Stream( comm ,MRN::TFILTER_NULL);
    reportInitialResources(stream);
    delete stream;
    delete comm;
}
//
// upcall from paradynd reporting new resource
//
void paradynDaemon::resourceInfoCallback(MRN::Stream * /*s*/,
                                         u_int temporaryId,
                                         pdvector<pdstring> resource_name,
                                         pdstring /* abstr */,
                                         u_int type,
                                         u_int mdlType)
{
	
	if(resource_name.size() > 0) 
		{
			const char *rstr = resource_name[0].c_str();
			if(rstr && !strcmp(rstr, "Machine")) 
				{
					addProcessInfo(resource_name);
				}
		}
	
	resource* r = resource::create(resource_name,
																 (ResourceType)type,
																 mdlType,
																 temporaryId);
	if(!countSync)
		{
			if (r->getHandle() != temporaryId)
				{
					pdvector<u_int>tempIds;
					pdvector<u_int>rIds;
					tempIds.push_back( temporaryId );
					rIds.push_back( r->getHandle() );
					MRN::Communicator * comm = network->new_Communicator();
					comm->add_EndPoint( endpoint );
					MRN::Stream * stream = network->new_Stream( comm );
					resourceInfoResponse(stream,tempIds, rIds);
					delete stream;
				}
		}
	else
		{
      if (r->getHandle() != temporaryId) 
				{
					newResourceTempIds.push_back( temporaryId );
					newResourceHandles.push_back( r->getHandle() );
					assert(newResourceTempIds.size() == newResourceHandles.size());
				}
		}
}


void paradynDaemon::severalResourceInfoCallback(MRN::Stream *s,
						pdvector<T_dyninstRPC::resourceInfoCallbackStruct> items)
{
    for (unsigned lcv=0; lcv < items.size(); lcv++)
        resourceInfoCallback(s,
                             items[lcv].temporaryId,
                             items[lcv].resource_name,
                             items[lcv].abstraction,
                             items[lcv].type,
                             items[lcv].mdlType);
}


// - update resource - added this function
// upcall from paradynd reporting resource update
//
void paradynDaemon::resourceUpdateCallback( MRN::Stream* ,
																						pdvector<pdstring> resource_name,
																						pdvector<pdstring> display_name,
																						pdstring abstraction)
{
	resource::update(resource_name,display_name, abstraction);	
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
   if(rl && m)
      {
         pdvector<u_int> focus;
         bool aflag;
         aflag=rl->convertToIDList(focus);
         assert(aflag);
         const char *metName = m->getName();
         assert(metName);

         pdstring focus_machine("none");
         rl->getMachineNameReferredTo(focus_machine);
         pdstring focus_proc("none");
         int focus_pid = 0;
         rl->getProcessReferredTo(&focus_proc, &focus_pid);

         // pdstring debug_focus = pdstring(m->getName());
//          for(unsigned i = 0; i < focus.size(); i++)
//             debug_focus += pdstring(" ") + pdstring(focus[i]);
//          fprintf(stderr, "paradynDaemon::getPredictedDataCostCall() - metfoc is %s for machine %s, process %s (%d)\n", 
//                  debug_focus.c_str(), focus_machine.c_str(), 
//                  focus_proc.c_str(), focus_pid);

         pdvector<unsigned> target_daemons;
         for(unsigned i = 0; i < paradynDaemon::allDaemons.size(); i++)
         {
            paradynDaemon *pd = paradynDaemon::allDaemons[i];
            if(strcmp(focus_machine.c_str(), "none")) {
               // focus is machine specific
               if(focus_machine == pd->getMachineName()) {
                  // machine match
                  if(focus_pid) {
                     // focus is process specific
                     if(pd->isMonitoringProcess(focus_pid)) {
                        // daemon is monitoring process focus
                        target_daemons.push_back(i);
                     }
                  }
                  else {
                     // focus not process specific
                     target_daemons.push_back(i);
                  }
               }
            }
            else
               target_daemons.push_back(i);
         }

         u_int requestId;
         if(performanceStream::addPredCostRequest(ps_handle, requestId,
                                                  m_handle, rl_handle,
                                                  target_daemons.size()))
            {
               
               MRN::Communicator * comm = paradynDaemon::allDaemons[0]->network
                  ->new_Communicator();
               for(unsigned j = 0; j < target_daemons.size(); j++)
               {
                  unsigned ndx = target_daemons[j];
                  comm->add_EndPoint( paradynDaemon::allDaemons[ndx]->endpoint );
               }
               MRN::Stream * stream = paradynDaemon::allDaemons[0]->network->
                  new_Stream( comm );
               paradynDaemon::allDaemons[0]->getPredictedDataCost(stream, ps_handle,requestId,focus, 
                                                                  metName,clientID);
               delete stream;
               delete comm;
            }
         return;
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
                             pdvector<paradynDaemon *> *to_list) 
{
  to_list->clear();
    
  if (daemon_list.size() == 0)
    return;
  
  to_list->push_back(daemon_list[0]);
  
  for (u_int i = 1; i < daemon_list.size(); i++)
    {
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
void paradynDaemon::findMatchingDaemons(metricInstance *mi, 
             pdvector<paradynDaemon *> *matching_daemons)
{
  pdvector<paradynDaemon*> daemon_subset; // which daemons to send request

  // check to see if this focus is refined on the machine
  // or process heirarcy, if so then add the approp. daemon
  // to the matching_daemons, else set whole_prog_focus to true
  pdstring machine_name;

  resourceList *focus_resources = mi->getresourceList(); 
  assert(focus_resources);
  //focus is refined on machine or process heirarchy 
  if(focus_resources->getMachineNameReferredTo(machine_name))
    {
      // get the daemon corr. to this focus and add it
      // to the list of daemons
        pdvector<paradynDaemon*> vpd = 
            paradynDaemon::machineName2Daemon(machine_name);
        assert(vpd.size());

      // Add daemons into daemon_subset
      filter_based_on_process_in_focus(vpd, &daemon_subset, focus_resources);

      // Prune the list we generated, since there might be duplicates
      strip_duplicate_daemons(daemon_subset, matching_daemons);

    }
  else 
    {  // focus is not refined on process or machine 
      // Don't need to prune, as there's only one daemon
      matching_daemons->clear();
      for(unsigned i=0; i<paradynDaemon::allDaemons.size(); i++)
	{
	  matching_daemons->push_back(paradynDaemon::allDaemons[i]);
	}
    }
    
  assert(matching_daemons->size() <= paradynDaemon::allDaemons.size());
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
       pdstring machine_name;
       resourceList *focus_resources = (*miVec)[i]->getresourceList(); 
       assert(focus_resources);
       // focus is refined on machine or process heirarchy 
       if(focus_resources->getMachineNameReferredTo(machine_name)){
           // get the daemon corr. to this focus and add it
           // to the list of daemons
           pdvector<paradynDaemon*> vpd = paradynDaemon::machineName2Daemon(machine_name);

					 // fprintf(stderr, "machineName2Daemon(\"%s\") => size %d\n",
					 //      machine_name.c_str(), vpd.size() );

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
  
  for (unsigned i = 0; i < allMIHs.size(); i++) 
    {
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
      
      //MRN::Stream * stream = network->new_Stream( network->get_BroadcastCommunicator(),TFILTER_NULL);
      MRN::Stream * stream = network->new_Stream( communicator );
      
      pdvector< T_dyninstRPC::instResponse > resp =
				enableDataCollection2(stream, vs, (const char *) m->getName(), mi->id,
			      this->id );
      delete stream;
      
      inst_insert_result_t status;
      //std::vector<MRN::EndPoint *> endpoints = network->
	//get_BroadcastCommunicator()->get_EndPoints();
      //      for( unsigned int j=0; j<endpoints.size(); j++) 
      //	{
      //	  status = inst_insert_result_t(resp[j].rinfo[0].status);
      status = inst_insert_result_t(resp[0].rinfo[0].status);
      //	}
      if(did_error_occur()){
         continue;
      }    
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
	component *comp = new component(this, resp[0].rinfo[0].mi_id, mi);
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


bool daemonEntry::setAll (const pdstring &m, const pdstring &c,
                          const pdstring &n, const pdstring &l,
                          const pdstring &d, const pdstring &r,
													const pdstring &f,
                          const pdstring &t, const pdstring &MPIt)
{
  if(!n.c_str() || !c.c_str())
      return false;

  if (m.c_str()) machine = m;
  if (c.c_str()) command = c;
  if (n.c_str()) name = n;
  if (l.c_str()) login = l;
  if (d.c_str()) dir = d;
  if (r.c_str()) remote_shell = r;
  if (MPIt.c_str()) MPItype = MPIt;
  if (f.c_str()) flavor = f;
  if (t.c_str()) mrnet_topology = t;

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

void paradynDaemon::setDaemonStartTime(MRN::Stream *, int, double startTime)
{
  timeStamp temp = 
    timeStamp(startTime, timeUnit::sec(), timeBase::bStd());
  cerr << std::flush;

  timeStamp firstT(startTime, timeUnit::sec(), timeBase::bStd());

  if(!getStartTime().isInitialized() || firstT < getStartTime()) 
    {
      //attemptUpdateAggDelay();
      setStartTime(firstT);
    }
  setEarliestStartTime(getAdjustedTime(firstT));
}

void paradynDaemon::setEarliestStartTime(timeStamp f) 
{
  if(! earliestStartTime.isInitialized()) 
    earliestStartTime = f;
  else if(f < earliestStartTime) 
    earliestStartTime = f;
  cerr << std::flush;
}

void paradynDaemon::setInitialActualValueFE(MRN::Stream *, int mid, double initActVal) {

  metricInstance *mi = NULL;
  bool found = metricInstance::allMetricInstances.find(mid, mi);
  if (!found)
    return; //We may have deleted the metric, while call was still in the pipe

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

paradynDaemon::paradynDaemon(MRN::Network *net, MRN::EndPoint *e, pdstring &m,
                             pdstring &l, pdstring &n, pdstring &f)
    :dynRPCUser(),
     network(net), endpoint(e), machine(m), login(l), name(n), flavor(f),
     activeMids(uiHash)
{
    communicator = net->new_Communicator();
    communicator->add_EndPoint( endpoint );

    machine = getNetworkName(machine);

    if (machine.suffixed_by(local_domain)) {
        const unsigned namelength = machine.length()-local_domain.length()-1;
        const pdstring localname = machine.substr(0,namelength);
        status = localname;
    } else {
        status = machine;
    }

    paradynDaemon::allDaemons += this;
    id = paradynDaemon::allDaemons.size()-1;

    paradynDaemon::daemonsById[ id ] = this;

    status += ":" + pdstring( id ) ;
    if(flavor != "mpi")
        uiMgr->createProcessStatusLine(status.c_str());
}

paradynDaemon::paradynDaemon(const pdstring &m, const pdstring &u,
                             const pdstring &c, const pdstring &r,
                             const pdstring &n, const pdstring &f)
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
        const pdstring localname = machine.substr(0,namelength);
        status = localname;
    } else {
        status = machine;
    }

    // add a unique-ifier to the status line name
    // so that we don't place status messages for all daemons running
    // on the same host into the same status line in the GUI
    {
        char idxbuf[16];
        snprintf( idxbuf, 16, "%d", paradynDaemon::daemonsByHost[status].size() );

        pdstring sfx = ((pdstring)("[")) + idxbuf + "]";
        paradynDaemon::daemonsByHost[status].push_back( this );

        // now make "status" unique
        status += sfx;
    }
		if(flavor != "mpi")
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

bool our_print_sample_arrival = false;
void printSampleArrivalCallback(bool newVal) {
   our_print_sample_arrival = newVal;
}

// batched version of sampleCallbackFunc
void paradynDaemon::batchSampleDataCallbackFunc(MRN::Stream *, int ,
																								pdvector<T_dyninstRPC::batch_buffer_entry> theBatchBuffer)
{
    sampleVal_cerr << "batchSampleDataCallbackFunc(), burst size: " 
                   << theBatchBuffer.size() << "   earliestFirstTime: " 
                   << getEarliestStartTime() << "\n";
    // Process every item in the batch buffer we've just received and

    for (unsigned index=0; index < theBatchBuffer.size(); index++) {
        const T_dyninstRPC::batch_buffer_entry &entry = theBatchBuffer[index] ; 
        unsigned mid = entry.mid ;
        // Okay, the sample is not an error; let's process it.
        metricInstance *mi;
        bool found = activeMids.find(mid, mi);
        if (!found) {
            // this can occur due to asynchrony of enable or disable requests
            // so just ignore the data
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
        } 
        else {
            multiplier = 1000000000.0;
        }
        // ---------------------------------------------------------------
        pdSample value= pdSample(static_cast<int64_t>(entry.value*multiplier));
        
        timeStamp newStartTime = this->getAdjustedTime(startTimeStamp);
        timeStamp newEndTime   = this->getAdjustedTime(endTimeStamp);
      
        mi->updateComponent(this, newStartTime, newEndTime, value);

        mi->doAggregation();
    }
}

// trace data streams
void paradynDaemon::batchTraceDataCallbackFunc(MRN::Stream *, int ,
                pdvector<T_dyninstRPC::trace_batch_buffer_entry> theTraceBatchBuffer)
{
    // get the earliest first time that had been reported by any paradyn
    // daemon to use as the base (0) time
    // assert(getEarliestFirstTime());

    // Go through every item in the batch buffer we've just received and
    // process it.
    for (unsigned index =0; index < theTraceBatchBuffer.size(); index++) {
        T_dyninstRPC::trace_batch_buffer_entry &entry = theTraceBatchBuffer[index] ;

        unsigned mid          = entry.mid ;
        unsigned length       = entry.length;

        if (our_print_sample_arrival) {
            cout << "mid " << mid << " : length = " << length << "\n"<<std::flush;
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
    fprintf(stderr, "Inconsistant state\n");
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
paradynDaemon::reportSelf(MRN::Stream *stream, pdstring m, pdstring p, int /*pid*/, pdstring flav)
{

  flavor = flav;
  if (!m.length() || !p.length()) {
    removeDaemon(this, true);
    fprintf(stderr, "paradyn daemon reported bad info, removed\n");
    // error
  } else {
    machine = m.c_str();
    command = p.c_str();

    if (machine.suffixed_by(local_domain)) {
        const unsigned namelength = machine.length()-local_domain.length()-1;
        const pdstring localname = machine.substr(0,namelength);
        status = localname;
    } else
        status = machine;

    // add a unique-ifier to the status line name
    // so that we don't place status messages for all daemons running
    // on the same host into the same status line in the GUI
    {
        char idxbuf[16];
        snprintf( idxbuf, 16, "%d", paradynDaemon::daemonsByHost[status].size() );

        pdstring sfx = ((pdstring)("[")) + idxbuf + "]";
        paradynDaemon::daemonsByHost[status].push_back( this );

        // now make "status" unique
        status += sfx;
    }

    uiMgr->createProcessStatusLine(status.c_str());

    if(flavor == "unix") {
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

  MRN::Stream * stream1 = programs[0]->controlPath->network->new_Stream(programs[0]->controlPath->network->get_BroadcastCommunicator(),MRN::TFILTER_NULL);
  // Send the initial metrics, constraints, and other neato things
  SendMDLFiles(stream1);
  delete stream1;


  //  Stream * stream2 = network->new_Stream(communicator);
  MRN::Stream * stream2 = programs[0]->controlPath->network->new_Stream(programs[0]->controlPath->network->get_BroadcastCommunicator(),MRN::TFILTER_NULL);

  pdvector<pdvector<T_dyninstRPC::metricInfo> > info = this->getAvailableMetrics(stream2);
  for(unsigned k = 0 ; k < info.size(); k++)
    {
      unsigned size = info[k].size();
      for (unsigned u=0; u<size; u++)
	{
	  addMetric(info[k][u]);
	}
    }
  delete stream2;
  //this->updateTimeAdjustment();

#if !defined(NO_SYNC_REPORT_SELF)
    // tell the daemon we're done with this call
    // it can go off and ignore its event loop for awhile
  reportSelfDone(stream);
  //delete stream;
#endif // !defined(NO_SYNC_REPORT_SELF)

  return;
}

//
// When a paradynd reports status, send the status to the user
//
void 
paradynDaemon::reportStatus(MRN::Stream *, pdstring line)
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
paradynDaemon::processStatus(MRN::Stream *s, int pid, u_int stat) {
   if (stat == procExited) { // process exited
      for(unsigned i=0; i < programs.size(); i++) {
         if ((programs[i]->pid == static_cast<unsigned>(pid)) && 
             programs[i]->controlPath == this)
         {
            programs[i]->exited = true;
            if (--procRunning == 0)
               performanceStream::notifyAllChange(appExited);
            int totalProcs, numExited;
            getProcStats(&totalProcs, &numExited);
            if(totalProcs == numExited)
               reportStatus(s, "application exited");
            else if(totalProcs > 1 && numExited>0) {
               pdstring msg;
               msg = pdstring("application running, ") + pdstring(numExited) +
                     " of " + pdstring(totalProcs) + " processes exited\n";
               reportStatus(s, msg.c_str());
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
paradynDaemon::endOfDataCollection(MRN::Stream *, int mid) {
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

void paradynDaemon::resourceReportsDone( MRN::Stream *,int )
{
    extern unsigned num_dmns_to_report_resources;
    num_dmns_to_report_resources--;
    if( num_dmns_to_report_resources == 0 ){
        MRN::Stream * stream = network->new_Stream( network->get_BroadcastCommunicator() );
        reportCallGraphEquivClass( stream);
        delete stream;
    }
}

void paradynDaemon::callGraphEquivClassReportCallback(MRN::Stream*, pdvector<T_dyninstRPC::equiv_class_entry> eqclasses )
{
    if(eqclasses.size() == 0)
        return;

    MRN::Communicator * comm = allDaemons[0]->getNetwork()->new_Communicator();

    extern unsigned num_dmns_to_report_callgraph;

    for(unsigned i = 0 ; i < eqclasses.size() ; i++) {
        num_dmns_to_report_callgraph++;
        comm->add_EndPoint(  paradynDaemon::allDaemons[eqclasses[i].class_rep]->endpoint );
    }
  
  MRN::Stream * stream = network->new_Stream( comm ,MRN::TFILTER_NULL);
  reportStaticCallgraph(stream);
  delete stream;
  delete comm;
}
