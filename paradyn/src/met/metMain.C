
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
 * Revision 1.17  1995/05/18 10:58:29  markc
 * mdl
 *
 * Revision 1.16  1995/03/30  15:32:37  jcargill
 * Fixed a minor UMR purify turned up
 *
 * Revision 1.15  1995/02/27  18:59:02  tamches
 * The use of tunable constants has changed to reflect the new
 * "tunableConstantRegistry" class and the new TCthread.
 *
 * Revision 1.14  1995/02/16  08:24:18  markc
 * Changed Boolean to bool.
 * Changed calls to igen functions to use strings/vectors rather than
 * char*'s/arrays
 *
 * Revision 1.13  1995/02/07  21:59:52  newhall
 * added a force option to the visualization definition, this specifies
 * if the visi should be started before metric/focus menuing
 * removed compiler warnings
 *
 * Revision 1.12  1994/12/21  07:38:43  tamches
 * Removed uses of tunableConstant::allConstants, which became a private
 * class variable.
 *
 * Revision 1.11  1994/12/21  05:50:15  tamches
 * Used the new tunableConstant::findTunableConstant() instead of
 * manually tinkering with tunable constant internal vrbles, which
 * is no longer allowed.
 *
 * Revision 1.10  1994/11/03  02:46:47  krisna
 * removed bare prototype for gethostname
 *
 * Revision 1.9  1994/10/10  02:52:51  newhall
 * removed the default visi: HISTOGRAM_REALTIME
 *
 * Revision 1.8  1994/09/25  01:55:08  newhall
 * changed arguments to VMAddNewVisualization
 *
 * Revision 1.7  1994/09/22  01:22:05  markc
 * Set default args
 *
 * Revision 1.6  1994/08/22  15:53:23  markc
 * Config language version 2.
 *
 * Revision 1.2  1994/07/07  13:10:41  markc
 * Turned off debugging printfs.
 *
 * Revision 1.1  1994/07/07  03:25:25  markc
 * Configuration language parser.
 *
 */

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
#include "paradyn/src/DMthread/DMinternals.h"

extern int yyparse();
extern int yyrestart(FILE *);

static int open_N_parse(string& file);

// open the config file and parse it
// return -1 on failure to open file
// else return yyparse result

static bool metDoDaemon(applicationContext *appCon);
static bool metDoVisi();
static bool metDoProcess(applicationContext *appCon);
static bool metDoTunable();

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
bool metMain(applicationContext *appCon, string &userFile)
{
  // return yyparse();
  int yy1=0, yy2, yy3;
  char *home, *proot, *cwd;
  string fname;

  mdl_init();

  proot = getenv("PARADYN_ROOT");
  if (proot) {
    fname = string(proot) + "/Paradynrc_NEW";
    yy1 = open_N_parse(fname);
  } else {
    cwd = getenv("cwd");
    if (cwd) {
      fname = string(cwd) + "/Paradynrc_NEW";
      yy1 = open_N_parse(fname);
    }
  }

  home = getenv("HOME");
  if (home) {
    fname = string(home) + "/.Paradynrc_NEW";
    yy2 = open_N_parse(fname);
  }

  if (userFile.length())
    yy3 = open_N_parse(userFile);

  // take actions based on the parsed configuration files
  metDoDaemon(appCon);
  metDoTunable();
  metDoProcess(appCon);
  metDoVisi();

  bool mdl_res = mdl_apply();

  return true;
}

static bool metDoDaemon(applicationContext *appCon)
{
  static bool been_done=0;
  // the default daemons
  if (!been_done) {
    appCon->defineDaemon("paradyndPVM", NULL, NULL, "pvmd", NULL, "pvm");
    appCon->defineDaemon("paradynd", NULL, NULL, "defd", NULL, "unix");
    // TODO -- should cm5d be defined
    appCon->defineDaemon("paradynd", NULL, NULL, "cm5d", NULL, "cm5");
    appCon->defineDaemon("simd", NULL, NULL, "simd", NULL, "unix");
    been_done = true;
  }
  unsigned size=daemonMet::allDaemons.size();
  for (unsigned u=0; u<size; u++) {
    appCon->defineDaemon(daemonMet::allDaemons[u]->command().string_of(),
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
  assert(RPCgetArg(argv, the_vm->command().string_of()));

  // the strings created here are used, not copied in the VM
  vmMgr->VMAddNewVisualization(the_vm->name().string_of(), &argv, the_vm->force(),
			       NULL, 0);
}

static bool metDoVisi()
{
  unsigned size = visiMet::allVisis.size();

  for (unsigned u=0; u<size; u++) {
    add_visi(visiMet::allVisis[u]);
    delete visiMet::allVisis[u];
  }
  return true;
}

static void start_process(processMet *the_ps, applicationContext *appCon)
{
  vector<string> argv;
  assert(RPCgetArg(argv, the_ps->command().string_of()));

  appCon->addExecutable(the_ps->host().string_of(), the_ps->user().string_of(),
			the_ps->daemon().string_of(), the_ps->execDir().string_of(),
			&argv);
}

static bool metDoProcess(applicationContext *appCon)
{
  unsigned size = processMet::allProcs.size();
  for (unsigned u=0; u<size; u++) {
    start_process(processMet::allProcs[u], appCon);
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
  
static bool metDoTunable()
{
  unsigned size = tunableMet::allTunables.size();
  for (unsigned u=0; u<size; u++)
    set_tunable(tunableMet::allTunables[u]);
  return 1;
}
