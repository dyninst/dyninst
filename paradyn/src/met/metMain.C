/*
 * Copyright (c) 1996 Barton P. Miller
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

/*
 * $Log: metMain.C,v $
 * Revision 1.36  1997/06/07 21:01:24  newhall
 * replaced exclude_func and exclude_lib with exclude_node
 *
 * Revision 1.35  1997/06/03 13:50:57  naim
 * Removing cm5d from source code - naim
 *
 * Revision 1.34  1997/05/23 23:04:08  mjrg
 * Windows NT port
 *
 * Revision 1.33  1996/11/26 16:07:17  naim
 * Fixing asserts - naim
 *
 * Revision 1.32  1996/08/16 21:12:21  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.31  1996/04/19 18:28:32  naim
 * Adding a procedure that will be called when we want to add a new process,
 * as it is done using the "paradyn process" command - naim
 *
 * Revision 1.30  1996/04/04  21:55:24  newhall
 * added limit option to visi definition
 *
 * Revision 1.29  1996/04/03  14:25:59  naim
 * Eliminating "simd" from the daemon's menu for the meantime - naim
 *
 * Revision 1.28  1996/02/16  20:23:57  tamches
 * fixed compile error
 *
 * Revision 1.27  1996/02/16 20:12:32  tamches
 * start_process now calls expand_tilde_pathname
 *
 * Revision 1.26  1996/02/16 16:33:37  naim
 * Checking error conditions in PCL file - naim
 *
 * Revision 1.25  1995/12/15  22:30:04  mjrg
 * Merged paradynd and paradyndPVM
 * Get module name for functions from symbol table in solaris
 * Fixed code generation for multiple instrumentation statements
 * Change syntax of MDL resource lists
 *
 * Revision 1.24  1995/11/13 14:58:36  naim
 * Adding "mode" option to the Metric Description Language to allow specificacion
 * of developer mode for metrics (default mode is "normal") - naim
 *
 */

#define GLOBAL_CONFIG_FILE "/paradyn.rc"
#define LOCAL_CONFIG_FILE "/.paradynrc"
#define PARADYN_ROOT "PARADYN_ROOT"


#include "paradyn/src/met/metParse.h"
#include "../TCthread/tunableConst.h"
#include "paradyn/src/met/metricExt.h"

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "paradyn/src/pdMain/paradyn.h"
#include "util/h/rpcUtil.h"
#include "util/h/pathName.h"
#include "paradyn/src/DMthread/DMinclude.h"
#include "paradyn/src/DMthread/DMdaemon.h"

extern int yyparse();
extern int yyrestart(FILE *);
extern appState PDapplicState;

static int open_N_parse(string& file);

// open the config file and parse it
// return -1 on failure to open file
// else return yyparse result

bool metDoDaemon();
bool metDoVisi();
bool metDoProcess();
bool metDoTunable();

static int open_N_parse (string& file)
{
  int res;
  FILE *f;
  static int been_here = 0;

  f = fopen (file.string_of(), "r");
  if (f) {
    if (!been_here) { 
      been_here = 1;
      yyin = f;
      res = yyparse();
      fclose(f);
      return res;
    } else {
      res = yyrestart(f);
      res = yyparse();
      fclose(f);
      return res;
    }
  }
  return -1;
}

// parse the 3 files (system, user, application)
bool metMain(string &userFile)
{
  // return yyparse();
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
      fprintf(stderr,"Error: can't open file '%s'.\n", userFile.string_of());
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
    dataMgr->defineDaemon("paradynd", NULL, NULL, "pvmd", NULL, "pvm");
    dataMgr->defineDaemon("paradynd", NULL, NULL, "defd", NULL, "unix");
    dataMgr->defineDaemon("paradynd", NULL, NULL, "winntd", NULL, "winnt");
    been_done = true;
  }
  unsigned size=daemonMet::allDaemons.size();
  for (unsigned u=0; u<size; u++) {
    dataMgr->defineDaemon(daemonMet::allDaemons[u]->command().string_of(),
			 daemonMet::allDaemons[u]->execDir().string_of(),
			 daemonMet::allDaemons[u]->user().string_of(),
			 daemonMet::allDaemons[u]->name().string_of(),
			 daemonMet::allDaemons[u]->host().string_of(),
			 daemonMet::allDaemons[u]->flavor().string_of());
    delete daemonMet::allDaemons[u];
  }
  daemonMet::allDaemons.resize(0);
  return true;
}

static void add_visi(visiMet *the_vm)
{
  vector<string> argv;
  bool aflag;
  aflag=(RPCgetArg(argv, the_vm->command().string_of()));
  assert(aflag);

  // the strings created here are used, not copied in the VM
  vmMgr->VMAddNewVisualization(the_vm->name().string_of(), &argv, 
			the_vm->force(),the_vm->limit(), NULL, 0);
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
  vector<string> argv;

  string directory;
  if (the_ps->command().length()) {
    bool aflag;
    aflag=(RPCgetArg(argv, the_ps->command().string_of()));
    assert(aflag);
    directory = expand_tilde_pathname(the_ps->execDir()); // see util lib
  }
  else {
    string msg;
    msg = string("Process \"") + the_ps->name() + 
	  string("\": command line is missing in PCL file.");
    uiMgr->showError(89,P_strdup(msg.string_of()));
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
//  if(dataMgr->addExecutable(the_ps->host().string_of(), 
//			    the_ps->user().string_of(),
//			    the_ps->daemon().string_of(), 
//			    directory.string_of(),
//			    &argv)) {
//    PDapplicState=appRunning;
//    dataMgr->pauseApplication();
//  }
}

bool metDoProcess()
{
  unsigned size = processMet::allProcs.size();
  for (unsigned u=0; u<size; u++) {
    start_process(processMet::allProcs[u]);
    delete processMet::allProcs[u];
  }
  return true;
}

static void set_tunable (tunableMet *the_ts)
{
  if (!tunableConstantRegistry::existsTunableConstant(the_ts->name().string_of()))
     return;

  if (tunableConstantRegistry::getTunableConstantType(the_ts->name().string_of()) ==
      tunableBoolean) {
     if (!the_ts->useBvalue()) {
        // type mismatch?
        return;
     }

     tunableConstantRegistry::setBoolTunableConstant(the_ts->name().string_of(),
						     (bool) the_ts->Bvalue());
  }
  else {
     if (the_ts->useBvalue()) {
        // type mismatch?
        return;
     }

     tunableConstantRegistry::setFloatTunableConstant(the_ts->name().string_of(),
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
