/*
 * Copyright (c) 1993, 1994 Barton P. Miller, Jeff Hollingsworth,
 *     Bruce Irvin, Jon Cargille, Krishna Kunchithapadam, Karen
 *     Karavanic, Tia Newhall, Mark Callaghan.  All rights reserved.
 * 
 * This software is furnished under the condition that it may not be
 * provided or otherwise made available to, or used by, any other
 * person, except as provided for by the terms of applicable license
 * agreements.  No title to or ownership of the software is hereby
 * transferred.  The name of the principals may not be used in any
 * advertising or publicity related to this software without specific,
 * written prior authorization.  Any use of this software must include
 * the above copyright notice.
 *
 */

/*
 * association.C - Manage mapping information (associations)
 *
 * $Log: association.C,v $
 * Revision 1.4  1994/11/02 10:59:43  markc
 * Replaced string-handles
 *
 * Revision 1.3  1994/09/22  01:32:26  markc
 * Made system includes extern"C"
 * Cast args for string functions
 *
 * Revision 1.2  1994/07/20  22:21:42  rbi
 * Small type change
 *
 * Revision 1.1  1994/06/27  21:29:11  rbi
 * Code for handling abstraction-specific mappings
 *
 */

extern "C" {
#include <stdio.h>
#include <stdlib.h>
}

#include "symtab.h"
#include "process.h"
#include "dyninstP.h"
#include "util.h"
#include "comm.h"
#include <strstream.h>
#include "main.h"

void newAssoc(process *proc, char *abstraction, char *type, char *key, 
	      char *value)
{
  unsigned faddr;
  pdFunction *func;

  /* 
   *  Call abstraction-specific translations here
   *  For example, an abstraction may want to 
   *  translate memory addresses to names using the symbol 
   *  table.
   */

  /* For TCL translate address to name */
  if (strcmp(type, "UserCommand") == 0) {
    /* Translate from string to address */
    sscanf(value, "%x", &faddr);
    func = (proc->symbols)->findFunctionByAddr(faddr);
    // TODO ? correct behavior
    if (!func)
      return;

    ostrstream os;

    assert(!(func->getSymbol() == (char*)NULL));
    os << func->getSymbol() << ends;
    char *temp = os.str();
    strcpy (value, temp);
    delete temp;
  }    
 
  tp->mappingInfoCallback(0, abstraction, type, key, value);
}










