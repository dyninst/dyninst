
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
 * PCfocus.C
 * 
 * Initializing focus info needed by PC from DM's resource hierarchies.
 * 
 * $Log: PCfocus.C,v $
 * Revision 1.1  1996/02/02 02:06:34  karavan
 * A baby Performance Consultant is born!
 *
 */

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "PCintern.h"

//
// known, "base" resources -- these are invariant across applications
//
resourceHandle RootResource;
resourceHandle SyncObject;
resourceHandle Procedures;
resourceHandle Processes;
resourceHandle Machines;
resourceHandle Locks;
resourceHandle Barriers;
resourceHandle Semaphores;
resourceHandle MsgTags;

// focus formed from topmost node of each resource hierarchy
focus topLevelFocus;


// returns 0 if internal error which is fatal to PC
int initResources()
{
  // only toplevel "base" resources are known to PC 
  // get names <-> handles here

  resourceHandle *h = dataMgr->getRootResource();
  assert (h != NULL); 
  RootResource = *h;

  h = dataMgr->findResource ("/SyncObject");
  if (h != NULL) 
    SyncObject = *h;
  h = dataMgr->findResource ("/Code");
  assert (h != NULL); // top level resources undefined is bad news
  Procedures = *h;
  h = dataMgr->findResource ("/Process");
  if (h != NULL) 
    Processes = *h;
  h = dataMgr->findResource ("/Machine");
  if (h != NULL) 
    Machines = *h;
  h = dataMgr->findResource ("/SpinLock");
  if (h != NULL)
    Locks = *h;
  h = dataMgr->findResource ("/Barrier");
  if (h != NULL)
    Barriers = *h;
  h = dataMgr->findResource ("/Semaphore");
  if (h != NULL)
    Semaphores = *h;
  h = dataMgr->findResource ("/MsgTag");
  if (h != NULL)
    MsgTags = *h;

  vector<resourceHandle> *testr = dataMgr->getRootResources();
  assert (testr != NULL);
  topLevelFocus = dataMgr->getResourceList(testr);
  return 1;
}


