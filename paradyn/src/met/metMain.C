
/* metMain contains the functions that are called by the parser to support the
 * configuration language.  The functions are:
 *     metProcess(..) - build the process list
 *     metDaemon (..) - build the daemon list
 *     metTunable(..) - build the tunable constant list
 *     metVisi(..) - build the visi list
 *
 *     metDoProcess(..) - start a process
 *     metDoDaemon (..) - start a daemon
 *     metDoTunable(..) - set a tunable constant value
 *     metDoVisi(..) - declare a visi
 */

/*
 * $Log: metMain.C,v $
 * Revision 1.2  1994/07/07 13:10:41  markc
 * Turned off debugging printfs.
 *
 * Revision 1.1  1994/07/07  03:25:25  markc
 * Configuration language parser.
 *
 */

#include "paradyn/src/met/metParse.h"
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "util/h/list.h"
#include "paradyn/src/pdMain/paradyn.h"
#include "util/h/tunableConst.h"

extern "C" {
int gethostname(char*, int);
}

#define CONFIG_NAME "sample.psdl"
#define CONFIG_SLASH "/sample.psdl"
#define ROOT_NAME "/p/paradyn/sample.psdl"

extern int yyparse();
extern int yyrestart(FILE *);

int open_N_parse(char *file);

List<tunableStruct*> globalTunable;
List<processStruct*> globalProcess;
List<visiStruct*> globalVisi;
List<daemonStruct*> globalDaemon;

// open the config file and parse it
// return -1 on failure to open file
// else return yyparse result

int open_N_parse (char *file)
{
  int res;
  FILE *f;
  static int been_here = 0;

  f = fopen (file, "r");
  if (f)
    {
      if (!been_here)
	{ 
	  been_here = 1;
          yyin = f;
          res = yyparse();
          fclose(f);
          return res;
        }
      else
	{
          res = yyrestart(f);
	  res = yyparse();
	  fclose(f);
	  return res;
	}
    }
  return -1;
}

// 
static char *convert_local(char *old_host)
{
  char holder[100];

  if (!old_host)
      return ((char*) 0);

  if (!strcmp("localhost", old_host))
    {
      if(!gethostname(holder, 99))
	return(strdup(holder));
      else
	return(old_host);
    }
  else
    return strdup(old_host);
}


// parse the 3 files (system, user, application)

int metMain()
{
  // return yyparse();
  int yy1, yy2, yy3, hlen;
  char *home, *homecat;

  home = getenv("HOME");

  globalTunable.removeAll();
  globalProcess.removeAll();
  globalDaemon.removeAll();

  yy1 = open_N_parse(ROOT_NAME);
  if (home)
    {
      hlen = strlen(home);
      hlen += (1 + strlen(CONFIG_NAME));  
      homecat = new char[hlen];
      strcpy(homecat, home);
      strcat(homecat, CONFIG_SLASH);
      yy2 = open_N_parse(homecat);
    }
  else
    yy2 = 0;

  yy3 = open_N_parse(CONFIG_NAME);

  return (yy1 + yy2 + yy3);
}

void dumpProcess(processStruct ps)
{
  List<char*> tempList;
  int i=0;
  char *val;

  printf("DUMPING THE PROCESS\n");
  printf("    COMMAND: %s\n", ps.command ? ps.command : " ");
  printf("    NAME:    %s\n", ps.name ? ps.name : " ");

  printf("    ARGS:  ");
  for (tempList=ps.args; val = *tempList; tempList++)
    printf("arg %d: %s  ", i++, val);

  printf("    HOST:    %s\n", ps.host ? ps.host : " ");
  printf("    DAEMON:  %s\n", ps.daemon ? ps.daemon : " ");
  printf("    FLAVOR:  %d\n", ps.flavor);
}

int metProcess (processStruct ps)
{
  processStruct *new_ps;

  printf("metProcess\n");
  if (!ps.command || !ps.host || !ps.daemon)
    {
      printf("for a process, command, daemon, and host must be defined\n");
      dumpProcess(ps);
      return -1;
    }
  else
    {
      new_ps = new processStruct;
      new_ps->name = ps.name;
      new_ps->command = ps.command;
      new_ps->args = ps.args;
      new_ps->host = convert_local(ps.host);
      new_ps->daemon  = ps.daemon;
      new_ps->flavor = ps.flavor;
      globalProcess.add(new_ps);
      // dumpProcess(ps);
      return 0;
    }
}

void dumpVisi (visiStruct vs)
{
  List<char*> tempList;
  int i=0;
  char *val;

  printf("DUMPING THE VISI\n");
  printf("    COMMAND: %s\n", vs.command ? vs.command : " ");
  printf("    NAME:    %s\n", vs.name ? vs.name : " ");

  printf("    ARGS:  ");
  for (tempList=vs.args; val = *tempList; tempList++)
    printf("arg %d: %s  ", i++, val);

  printf("    HOST:    %s\n", vs.host ? vs.host : " ");
}

int metVisi(visiStruct vs)
{
  visiStruct *new_vs;

  printf("metVisi\n");
  if (!vs.command || !vs.name)
    {
      printf("for a visi, command and name must be defined\n");
      dumpVisi(vs);
      return -1;
    }
  else
    {
      new_vs = new visiStruct;
      new_vs->name = vs.name;
      new_vs->command = vs.command;
      new_vs->args = vs.args;
      new_vs->host = convert_local(vs.host);
      globalVisi.add(new_vs);
      // dumpVisi(vs);
      return 0;
    }
}
void dumpTunable (char *name, float value)
{
  printf("DUMPING THE TUNABLE\n");
  printf("    NAME:  %s\n", name ? name : " ");
  printf("    VALUE: %f\n", value);
}

int metTunable (char *name, float value)
{
  tunableStruct *ts;

  printf("metTunable\n");
  if (!name)
    {
      printf("for a tunable constant, name must be defined\n");
      dumpTunable(name, value);
      return -1;
    }
  else
    {
      ts = new tunableStruct;
      ts->name = name;
      ts->value = value;
      globalTunable.add(ts);
      // dumpTunable(name, value);
      return 0;
    }
}

void dumpDaemon (daemonStruct ds)
{
  printf("DUMPING THE DAEMON\n");
  printf("    COMMAND: %s\n", ds.command ? ds.command : " ");
  printf("    NAME:    %s\n", ds.name ? ds.name : " ");
  printf("    HOST:    %s\n", ds.host ? ds.host : " ");
  printf("    FLAVOR:  %d\n", ds.flavor);
}

int metDaemon (daemonStruct ds)
{
  daemonStruct *new_ds;

  printf("metDaemon\n");
  if (!ds.command || !ds.host)
    {
      printf("for a daemon, command and host must be defined\n");
      dumpDaemon(ds);
      return -1;
    }
  else
    { 
      new_ds =  new daemonStruct;
      new_ds->name = ds.name;
      new_ds->command = ds.command;
      new_ds->host = convert_local(ds.host);
      new_ds->flavor = ds.flavor;
      globalDaemon.add(new_ds);
      // dumpDaemon(ds);
      return 0;
    }
}

int metDoDaemon()
{
  daemonStruct *the_ds;
  List<daemonStruct*> dl;

  for (dl = globalDaemon; the_ds = *dl; dl++)
    {
      // dumpDaemon(*the_ds);
      if (dataMgr->addDaemon(context, the_ds->host, (char *) 0,
			     the_ds->command))
	; //printf("Start daemon %s on %s succeeded\n", the_ds->command,
	   //    the_ds->host);
      else
	; //printf("Start daemon %s on %s failed\n", the_ds->command,
	   //    the_ds->host);
    }
  return 0;
}

int metDoVisi()
{
  visiStruct *the_vs;
  List<visiStruct*> vl;
  List <char*> tempList;
  int argc, i;
  char **argv;
  char *val;

  for (vl = globalVisi; the_vs = *vl; vl++)
    {
      argc = the_vs->args.count() + 1;
      argv = new char*[argc+1];
      argv[argc] = 0;
      i = 1;
      argv[0] = strdup(the_vs->command);
      for (tempList = the_vs->args; val = *tempList; tempList++)
	{
	  argv[i] = strdup(val);
	  i++;
	}
      // dumpVisi(*the_vs);
      vmMgr->VMAddNewVisualization(the_vs->name, argc, argv);
      for (i=0; i<argc; ++i)
       if (argv[i]) delete argv[i];
      delete argv;
    }
  return 0;
}

int metDoProcess()
{
  processStruct *the_ps;
  List<processStruct*> pl;
  List <char*> tempList;
  int argc, i;
  char **argv;
  char *val;

  for (pl = globalProcess; the_ps = *pl; pl++)
    {
      argc = the_ps->args.count() + 1;
      argv = new char*[argc+1];
      argv[argc] = 0;
      i = 1;
      argv[0] = strdup(the_ps->command);
      for (tempList = the_ps->args; val = *tempList; tempList++)
	{
	  argv[i] = strdup(val);
	  i++;
	}
      // dumpProcess(*the_ps);
      if (dataMgr->addExecutable(context, the_ps->host, (char*) 0,
				 the_ps->daemon, argc, argv))
	; // printf("Start process %s succeeded on %s\n", the_ps->command,
	   //    the_ps->host);
      else
	; //printf("Start process %s failed on %s\n", the_ps->command,
	   //    the_ps->host);
      
      for (i=0; i<argc; ++i)
       if (argv[i]) delete argv[i];
      delete argv;
    }
  return 0;
}

int metDoTunable()
{
  tunableStruct *the_ts;
  List<tunableStruct*> tl;
  tunableConstant* curr;
  char *sp;

  if (!tunableConstant::allConstants)
    return -1;

  for (tl = globalTunable; the_ts = *tl; tl++)
    {
      // dumpTunable(the_ts->name, the_ts->value);
      sp = tunableConstant::pool->findAndAdd(the_ts->name);
      curr = tunableConstant::allConstants->find(sp);

      if (curr)
	{
	  if (!curr->setValue(the_ts->value))
	    ; // printf("Can't set value of tunable constant\n");
	  else
	    ; //printf("Set Tunable Constant: %s = %f\n", the_ts->name,
	//	   the_ts->value);
	}
    }
  return 0;
}
