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
 * Revision 1.1  1994/06/27 21:29:11  rbi
 * Code for handling abstraction-specific mappings
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "symtab.h"
#include "process.h"
#include "dyninstP.h"
#include "util.h"
#include "comm.h"

extern pdRPC *tp;

void newAssoc(process *proc, char *abstraction, char *type, char *key, 
	      char *value)
{
  unsigned faddr;
  function *func;

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
    func = findFunctionByAddr(proc->symbols, faddr);
    strcpy (value, func->symTabName);
  }    
 
  tp->mappingInfoCallback(0, abstraction, type, key, value);
}
