
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
 * Revision 1.16  1995/03/30 15:32:37  jcargill
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

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "paradyn/src/pdMain/paradyn.h"
#include "util/h/rpcUtil.h"

extern int yyparse();
extern int yyrestart(FILE *);

int open_N_parse(char *file);

// open the config file and parse it
// return -1 on failure to open file
// else return yyparse result

int open_N_parse (char *file)
{
  int res;
  FILE *f;
  static int been_here = 0;

  f = fopen (file, "r");
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

char *makeName(char *prefix, char *suffix)
{
  char *result;
  int len;

  if (!prefix || !suffix)
    return ((char*) 0);

  len = strlen(prefix) + strlen(suffix) + 2;

  result = new char[len];
  if (!result)
    return ((char*) 0);

  strcpy(result, prefix);
  strcat(result, suffix);
  return result;
}

// parse the 3 files (system, user, application)

int metMain(char *userFile)
{
  // return yyparse();
  int yy1=0, yy2=0, yy3=0;
  char *home, *proot, *fname, *cwd;

  // empty the lists
  tunableMet::allTunables.removeAll();
  daemonMet::allDaemons.removeAll();
  visiMet::allVisis.removeAll();
  processMet::allProcs.removeAll();

  proot = getenv("PARADYN_ROOT");
  if (proot) {
    fname = makeName(proot, "/Paradynrc");
    yy1 = open_N_parse(fname);
    delete [] fname;
  } else {
    cwd = getenv("cwd");
    if (cwd) {
      fname = makeName(cwd, "/Paradynrc");
      yy1 = open_N_parse(fname);
      delete [] fname;
    }
  }

  home = getenv("HOME");
  if (home) {
    fname = makeName(home, "/.Paradynrc");
    yy2 = open_N_parse(fname);
    delete [] fname;
  }

  if (userFile) {
    yy3 = open_N_parse(userFile);
  }

  return (yy2 + yy1 + yy3);
}

static void define_daemon (daemonMet *the_dm)
{
  // daemons cannot define any arguments
  // just use the first word in the command 'sentence' for exec
  vector<string> argv;
  assert(RPCgetArg(argv, the_dm->command));
  string program = argv[0];

  if (!dataMgr->defineDaemon(context, program.string_of(), the_dm->execDir,
			     the_dm->user, the_dm->name, the_dm->host,
			     the_dm->flavor))
    ; // print error message
}

int metDoDaemon()
{
  daemonMet def;
  static int been_done=0;
  List<daemonMet*> walk;

  // the default daemons
  if (!been_done) {
    char nmStr[20] = "pvmd", cmdStr[20] = "paradyndPVM";
    def.name = nmStr;
    def.command = cmdStr;
    def.flavor = metPVM;
    define_daemon(&def);
    
    strcpy(nmStr, "defd");
    strcpy(cmdStr, "paradynd");
    def.flavor = metUNIX;
    define_daemon(&def);

    strcpy(nmStr, "cm5d");
    strcpy(cmdStr, "paradyndCM5");
    def.flavor = metCM5;
    define_daemon(&def);

    strcpy(nmStr, "simd");
    strcpy(cmdStr, "paradyndSIM");
    def.flavor = metUNIX;
    define_daemon(&def);
    def.command = 0; def.name =0;
    been_done = 1;
  }

  for (walk=daemonMet::allDaemons; *walk; walk++)
    define_daemon(*walk);
  return 1;
}


static void add_visi(visiMet *the_vm)
{
  vector<string> argv;
  assert(RPCgetArg(argv, the_vm->command));
  int argc = argv.size();
  char **av = new char*[argc+1];
  av[argc] = NULL;
  // TODO -- is there a memory leak here
  for (unsigned ve=0; ve<argc; ve++)
    av[ve] = P_strdup(argv[ve].string_of());

  // the strings created here are used, not copied in the VM
  vmMgr->VMAddNewVisualization(the_vm->name, argc, av, the_vm->force, NULL, 0);
}

int metDoVisi()
{
  visiMet vm;
  static int been_done = 0;
  List<visiMet*> walk;

/*
  if (!been_done) {
    char nmStr[20] = "HISTOGRAM_REALTIME", cmdStr[10] = "rthist";
    vm.name = nmStr;
    vm.command = cmdStr;
    add_visi(&vm);
    vm.command=0; vm.name=0;
    been_done = 1;
  }
*/

  for (walk=visiMet::allVisis; *walk; walk++)
    add_visi(*walk);
  return 1;
}

static void start_process(processMet *the_ps)
{
  vector<string> argv;
  assert(RPCgetArg(argv, the_ps->command));

  if (!dataMgr->addExecutable(context,
			      the_ps->host,
			      the_ps->user,
			      the_ps->daemon,
			      the_ps->execDir,
			      &argv))
    ; // print error message
}

int metDoProcess()
{
  List<processMet*> walk;
  for (walk=processMet::allProcs; *walk; walk++) 
    start_process(*walk);
  return 1;
}

void set_tunable (tunableMet *the_ts)
{
  if (!tunableConstantRegistry::existsTunableConstant(the_ts->name))
     return;

  if (tunableConstantRegistry::getTunableConstantType(the_ts->name) == tunableBoolean) {
     if (!the_ts->useBvalue) {
        // type mismatch?
        return;
     }

     tunableConstantRegistry::setBoolTunableConstant(the_ts->name, (bool)the_ts->Bvalue);
  }
  else {
     if (the_ts->useBvalue) {
        // type mismatch?
        return;
     }

     tunableConstantRegistry::setFloatTunableConstant(the_ts->name, the_ts->Fvalue);
  }
  
//  tunableConstant *curr = tunableConstant::findTunableConstant(the_ts->name);
//  if (curr == NULL)
//     return;
//
//  if ((curr->getType() == tunableFloat) && !the_ts->useBvalue) {
//    tunableFloatConstant *fConst = (tunableFloatConstant*) curr;
//    if (!fConst->setValue(the_ts->Fvalue)) {
//      ; // error
//    }
//  } else if ((curr->getType() == tunableBoolean) && the_ts->useBvalue) {
//    tunableBooleanConstant *bConst = (tunableBooleanConstant*) curr;
//    if (!bConst->setValue((Boolean)the_ts->Bvalue)) {
//       ; // error 
//    }
//  } else {
//    ;
//    // unknown tunableConst type or type mismatch
//  }

}

int metDoTunable()
{
  List<tunableMet*> walk;

  for (walk=tunableMet::allTunables; *walk; walk++)
    set_tunable(*walk);
  return 1;
}
