
/*
 * $Log: metTester.C,v $
 * Revision 1.2  1994/08/22 15:53:32  markc
 * Config language version 2.
 *
 * Revision 1.1  1994/07/07  03:25:30  markc
 * Configuration language parser.
 *
 */

#include "paradyn/src/met/metParse.h"
#include <stdio.h>

int yyparse();

int metMain(char *userFile)
{
  return yyparse();
}

main()
{
  metMain((char*) 0);
  
  tunableMet::dumpAll();
  processMet::dumpAll();
  visiMet::dumpAll();
  daemonMet::dumpAll();
  stringList::dumpAll();
}


