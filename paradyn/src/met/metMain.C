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

/* metMain contains the functions that are called by the parser to support the
 * configuration language.  The functions are:
 *
 *     metDoProcess(..) - start a process
 *     metDoDaemon (..) - start a daemon
 *     metDoTunable(..) - set a tunable constant value
 *     metDoVisi(..) - declare a visi
 */

// $Id: metMain.C,v 1.51 2003/06/09 21:47:14 pcroth Exp $

#define GLOBAL_CONFIG_FILE "/paradyn.rc"
#define LOCAL_CONFIG_FILE "/.paradynrc"
#define PARADYN_ROOT "PARADYN_ROOT"

#include "mdl/h/mdlParse.h"
#include "../TCthread/tunableConst.h"
#include "paradyn/src/met/metricExt.h"

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#if !defined(i386_unknown_nt4_0)
#include <unistd.h>
#endif // !defined(i386_unknown_nt4_0)

#include "paradyn/src/pdMain/paradyn.h"
#include "pdutil/h/rpcUtil.h"
#include "common/h/pathName.h"
#include "paradyn/src/DMthread/DMinclude.h"
#include "paradyn/src/DMthread/DMdaemon.h"

pdvector<int> mdl_files;
extern appState PDapplicState;

static int open_N_parse(string& file);

// open the config file and parse it
// return -1 on failure to open file
// else return mdlparse result

bool metDoDaemon();
bool metDoVisi();
bool metDoProcess();
bool metDoTunable();

static int open_N_parse (string& file)
{
  FILE *f;
  static int been_here = 0;
  int ret = -1;

  f = fopen (file.c_str(), "r");
  if (f) {
    mdl_files.push_back( fileno(f) );
    if (!been_here) { 
      been_here = 1;
      mdlin = f;
      ret = mdlparse();
      fclose(f);
    } else {
      mdlrestart(f);
      ret = mdlparse();
      fclose(f);
    }
  }
  return ret;
}

// parse the 3 files (system, user, application)
bool metMain(string &userFile)
{
  int yy1=0, yy2, yy3;
  char *home, *proot, *cwd;
  string fname;

  mdl_init();
 
//  const string rcFileExtensionName="paradyn.rc";
     // formerly Paradynrc_NEW --ari

  proot = getenv(PARADYN_ROOT);
  if (proot) {
    fname = string(proot) + GLOBAL_CONFIG_FILE;
    yy1 = open_N_parse(fname);
  } else {
    // note: we should use getwd() instead --ari
    // (although it's not standard C in the sense that it's not
    //  in the K & R book's appendix)
    cwd = getenv("PWD");
    if (cwd) {
      fname = string(cwd) + GLOBAL_CONFIG_FILE;
      yy1 = open_N_parse(fname);
    } else yy1 = -1;
  }

  home = getenv("HOME");
  if (home) {
    fname = string(home) + LOCAL_CONFIG_FILE;
    yy2 = open_N_parse(fname);
  } else yy2 = -1;

  if (userFile.length()) {
    yy3 = open_N_parse(userFile);
    if (yy3 == -1) {
      fprintf(stderr,"Error: can't open file '%s'.\n", userFile.c_str());
      exit(-1);
    }
  }
  else yy3 = -1;

  if (yy1 < 0 && yy2 < 0 && yy3 < 0) {
    fprintf(stderr,"Error: can't find any configuration files.\nParadyn looks for configuration files in the following places:\n\t$PARADYN_ROOT/paradyn.rc or $PWD/paradyn.rc\n\t$HOME/.paradynrc\n");
    exit(-1);
  }

  // take actions based on the parsed configuration files

  // metDoDaemon();
  // metDoTunable();
  // metDoProcess();
  // metDoVisi();
  bool mdl_res = mdl_apply();
  if(mdl_res) {
      mdl_res = mdl_check_node_constraints();
  }
  return(mdl_res);
}

bool metDoDaemon()
{
  static bool been_done=0;
  // the default daemons
  if (!been_done) {
    dataMgr->defineDaemon("paradynd", NULL, NULL, "defd", NULL, NULL, "unix");
    dataMgr->defineDaemon("paradynd", NULL, NULL, "winntd", NULL,NULL,"winnt");
    dataMgr->defineDaemon("paradyndMT", NULL, NULL, "mt_d", NULL, NULL,"unix");
    been_done = true;
  }
  unsigned size=daemonMet::allDaemons.size();
  for (unsigned u=0; u<size; u++) {
    dataMgr->defineDaemon(daemonMet::allDaemons[u]->command().c_str(),
			 daemonMet::allDaemons[u]->execDir().c_str(),
			 daemonMet::allDaemons[u]->user().c_str(),
			 daemonMet::allDaemons[u]->name().c_str(),
			 daemonMet::allDaemons[u]->host().c_str(),
			 daemonMet::allDaemons[u]->remoteShell().c_str(),
			 daemonMet::allDaemons[u]->flavor().c_str());
    delete daemonMet::allDaemons[u];
  }
  daemonMet::allDaemons.resize(0);
  return true;
}

static void add_visi(visiMet *the_vm)
{
  pdvector<string> argv;
  bool aflag;
  aflag=(RPCgetArg(argv, the_vm->command().c_str()));
  assert(aflag);

  // the strings created here are used, not copied in the VM
  vmMgr->VMAddNewVisualization(the_vm->name().c_str(), &argv, 
			the_vm->force(),the_vm->limit(),the_vm->metfocus());
}


unsigned metVisiSize(){
  return(visiMet::allVisis.size());
}

visiMet *metgetVisi(unsigned i){
   
   if(i < visiMet::allVisis.size()){
       return(visiMet::allVisis[i]);
   }
   return 0;
}


bool metDoVisi()
{
  unsigned size = visiMet::allVisis.size();

  for (unsigned u=0; u<size; u++) {
    add_visi(visiMet::allVisis[u]);
    delete visiMet::allVisis[u];
  }
  return true;
}

static void start_process(processMet *the_ps)
{
  pdvector<string> argv;

  string directory;
  if (the_ps->command().length()) {
    bool aflag;
    aflag=(RPCgetArg(argv, the_ps->command().c_str()));
    assert(aflag);
    directory = expand_tilde_pathname(the_ps->execDir()); // see util lib
  }
  else {
    string msg;
    msg = string("Process \"") + the_ps->name() + 
	  string("\": command line is missing in PCL file.");
    uiMgr->showError(89,P_strdup(msg.c_str()));
    return;
  }

  string *arguments;
  arguments = new string;
  if (the_ps->user().length()) {
    *arguments += string("-user ");
    *arguments += the_ps->user();
  }
  if (the_ps->host().length()) { 
    *arguments += string(" -machine ");
    *arguments += the_ps->host();
  }
  if (directory.length()) {
    *arguments += string(" -dir ");
    *arguments += directory;
  }
  if (the_ps->daemon().length()) {
    *arguments += string(" -daemon ");
    *arguments += the_ps->daemon();
  }
  for (unsigned i=0;i<argv.size();i++) { 
    *arguments += string(" ");
    *arguments += argv[i];
  }
  uiMgr->ProcessCmd(arguments);

//
// The code bellow is no longer necessary because we are starting this process
// in the same way as in the "Define A Process" window - naim
//
//  if(dataMgr->addExecutable(the_ps->host().c_str(), 
//			    the_ps->user().c_str(),
//			    the_ps->daemon().c_str(), 
//			    directory.c_str(),
//			    &argv)) {
//    PDapplicState=appRunning;
//    dataMgr->pauseApplication();
//  }
}

bool metDoProcess()
{
    return processMet::doInitProcess();
}

bool processMet::doInitProcess() {
    unsigned size = allProcs.size();
    for (unsigned u=0; u<size; u++) {
	if( allProcs[u]->autoStart() ) {
	    start_process( allProcs[u] );
	    delete allProcs[u];
	    allProcs[u] = NULL;
	}
    }
    return true;
}

void metCheckDaemonProcess( const string &host ) {
    processMet::checkDaemonProcess( host );
}

void processMet::checkDaemonProcess( const string &host ) {
    unsigned size = allProcs.size();
    //cerr << "Checking for non-started processes for host " << host << endl;
    for (unsigned u=0; u<size; u++) {
	if( allProcs[u] && allProcs[u]->host() == host ) {
	    //cerr << " - Starting process " << allProcs[u]->name() << endl;
	    start_process( allProcs[u] );
	    delete allProcs[u];
	    allProcs[u] = NULL;
	}
    }	
}

static void set_tunable (tunableMet *the_ts)
{
  if (!tunableConstantRegistry::existsTunableConstant(the_ts->name().c_str()))
     return;

  if (tunableConstantRegistry::getTunableConstantType(the_ts->name().c_str()) ==
      tunableBoolean) {
     if (!the_ts->useBvalue()) {
        // type mismatch?
        return;
     }

     tunableConstantRegistry::setBoolTunableConstant(the_ts->name().c_str(),
						     the_ts->Bvalue());
  }
  else {
     if (the_ts->useBvalue()) {
        // type mismatch?
        return;
     }

     tunableConstantRegistry::setFloatTunableConstant(the_ts->name().c_str(),
						      the_ts->Fvalue());
  }
}
  
bool metDoTunable()
{
  unsigned size = tunableMet::allTunables.size();
  for (unsigned u=0; u<size; u++)
    set_tunable(tunableMet::allTunables[u]);
  return 1;
}
