
/*
 * $Log: metTester.C,v $
 * Revision 1.1  1994/07/07 03:25:30  markc
 * Configuration language parser.
 *
 */

#include "paradyn/src/met/metParse.h"
#include <stdio.h>

int yyparse();

int metMain()
{
  return yyparse();
}

main()
{
    metMain();
}

void dumpProcess(processStruct ps)
{ 
  int i=0;
  List<char*> tempList;
  char *val;

  printf("DUMPING THE PROCESS\n");
  printf("    COMMAND: %s\n", ps.command ? ps.command : " ");
  printf("    NAME:    %s\n", ps.name ? ps.name : " ");

  printf("    ARGS:  ");
  for (tempList=ps.args; val = *tempList; tempList++)
    printf("arg %d: %s   ", i++, val);

  printf("    HOST:    %s\n", ps.host ? ps.host : " ");
  printf("    DAEMON:  %s\n", ps.daemon ? ps.daemon : " ");
  printf("    FLAVOR:  %d\n", ps.flavor);
}

int metProcess (processStruct ps)
{
  printf("metProcess\n");
  if (!ps.command || !ps.host || !ps.daemon)
    {
      printf("ERROR: for a process, command, daemon and host must be defined\n");
      dumpProcess(ps);
      return -1;
    }
  else
    {
      printf("PROCESS defined correctly\n");
      dumpProcess(ps);
      return (0);
    }
}

void dumpVisi(visiStruct vs)
{ 
  int i=0;
  List<char*> tempList;
  char *val;

  printf("DUMPING THE VISI\n");
  printf("    COMMAND: %s\n", vs.command ? vs.command : " ");
  printf("    NAME:    %s\n", vs.name ? vs.name : " ");

  printf("    ARGS:  ");
  for (tempList=vs.args; val = *tempList; tempList++)
    printf("arg %d: %s   ", i++, val);

  printf("    HOST:    %s\n", vs.host ? vs.host : " ");
}

int metVisi (visiStruct vs)
{
  printf("metProcess\n");
  if (!vs.command || !vs.name)
    {
      printf("ERROR: for a visi, command, and name must be defined\n");
      dumpVisi(vs);
      return -1;
    }
  else
    {
      printf("VISI defined correctly\n");
      dumpVisi(vs);
      return (0);
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
  printf("metTunable\n");
  if (!name)
    {
      printf("for a tunable constant, name must be defined\n");
      dumpTunable(name, value);
      return -1;
    }
  else
    {
      printf("TUNABLE CONSTANT defined correctly\n");
      dumpTunable(name, value);
      return (0);
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
  printf("metDaemon\n");
  if (!ds.command || !ds.host)
    {
      printf("for a daemon, command and host must be defined\n");
      dumpDaemon(ds);
      return -1;
    }
  else
    {
      printf("DAEMON defined correctly\n");
      dumpDaemon(ds);
      return (0);
    }
}

