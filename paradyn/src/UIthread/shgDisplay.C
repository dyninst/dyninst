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
 * shgDisplay.C
 * 
 * class shgDisplay
 *
 * $Log: shgDisplay.C,v $
 * Revision 1.1  1995/10/05 04:31:59  karavan
 * Initial implementation.
 *
 */

#include "UIglobals.h"
#include "../pdMain/paradyn.h"
#include "shgDisplay.h"

      /* unique id generator, hash table for token lookup */
unsigned shgDisplay::nextID = 0;
dictionary_hash<unsigned, shgDisplay*> 
     shgDisplay::AllSearchDisplays(uihsh);
//
// add a new string to the scrolling text window at the top of the 
// search display
//
void
shgDisplay::updateStatus (const char *newinfo)
{
  if (Tcl_VarEval (interp, "shgUpdateStatusLine ", winName.string_of(), 
		   " {",
		   (char *)newinfo, "}", (char *) NULL) == TCL_ERROR)
    fprintf (stderr, "status insert error: %s\n", interp->result);
}

//
// add a new node to this search display
//
int
shgDisplay::addNode (nodeIdType nodeID, int styleID, 
			char *label, char *shgname, int flags)
{
  int retVal;

  if (flags)
    retVal = shgdag->
      CreateNode (nodeID, 1, "root", styleID, (void *)NULL);
  else 
    retVal = shgdag->
      CreateNode (nodeID, 0, label, styleID, (void *)NULL);

  if (retVal != AOK)
    return -1;

     // store pointer to full node pathname and use last piece as label
  string *newname = new string(shgname);
  AllNodeFullNames[nodeID] = newname;
  return 1;
}

//
// add a new edge to this search display
//
int
shgDisplay::addEdge (nodeIdType srcID, nodeIdType dstID, int styleID)
{
  return (shgdag->AddEdge(srcID, dstID, styleID) == AOK);
}

//
// change style of existing node in search display
//
int 
shgDisplay::configureNode (nodeIdType nodeID, char *label, int styleID)
{
  return (shgdag->configureNode (nodeID, label, styleID) == AOK);
}

//
// note constructor does not actually create display window
//
shgDisplay::shgDisplay (const char *winT, // display at top of window
			int pID)          // search phase identifier
                                          //  (global = 0)
: winTitle(winT), phaseID(pID), AllNodeFullNames (uihsh)
{
  char win[20];

  // create dag 
  shgdag = new dag (interp);
  dagToken = (int) shgdag->getToken();

  sprintf (win, ".shg%02d", dagToken);
  winName = win;
  
  winTitle = winT;
  phaseID = pID;

  // add to list of all shg displays, indexed by shgid
  id = nextID;
  nextID++;
  AllSearchDisplays[id] = this;
}

//
// set up full GUI display for a single SHG and return token
//
int
shgDisplay::initDisplay ()
{
  char tcommand[256];

  // create toplevel window for shg display, buttons, etc.
  sprintf (tcommand, "initSHG %s %d {%s} %d", winName.string_of(),  
		   dagToken, winTitle.string_of(), id);
  if (Tcl_VarEval (interp, tcommand, (char *) NULL) == TCL_ERROR) {
    printf ("initSHG error: %s\n", interp->result);
    return -1;
  }

  // dag display
  string temp = winName;
  temp += ".dag";
  shgdag->createDisplay (temp.string_of());

  // add default bindings and styles for SHG
  sprintf (tcommand, "addDefaultShgBindingsAndStyles %s %d %d",
	   shgdag->getCanvasName(), dagToken, id);
  if (Tcl_VarEval (interp, tcommand, (char *) NULL) == TCL_ERROR) {
    printf ("initSHGbindingsAndStyles error: %s\n", interp->result);
    return -1;
  }
  return (int)id;
}



