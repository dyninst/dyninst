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
 * shgDisplay.h
 *
 * $Log: shgDisplay.h,v $
 * Revision 1.1  1995/10/05 04:32:01  karavan
 * Initial implementation.
 *
 */

#ifndef _shgDisplay_h
#define _shgDisplay_h

#include <string.h>
#include "dag.h"

// hash function for dictionary members
inline unsigned uihsh(const unsigned &ptr) {
  return (ptr >> 2);
}

// 
// shgDisplay
// 
// Contains everything related to GUI display for one Perf Consultant
// search.  We assume there may be more than one, although only one 
// per phase plus optionally one for global search.  Perf Consultant 
// uses shgDisplay.id as shgToken for all update calls (this used to 
// be a dag id), including text status update.
//
class shgDisplay {
  friend int shgShortExplainCmd (ClientData clientData, 
                Tcl_Interp *interp, 
                int argc, 
                char *argv[]);

 public:
  shgDisplay (const char *name, int phase);
  int initDisplay();
  // for token lookups
  int getToken() {return (int)id;}
  int getDagToken() {return (int)dagToken;}
  int addNode (nodeIdType nodeID, int styleID, 
	       char *label, char *shgname, int flags);
  int addEdge (nodeIdType srcID, nodeIdType dstID, int styleID);
  int configureNode (nodeIdType nodeID, char *label, int styleID);

  void updateStatus (const char *newinfo);
  const char *getWinTitle() {return winTitle.string_of();}
  static dictionary_hash<unsigned, shgDisplay*> AllSearchDisplays;
 private:
  // graph display
  dag *shgdag;    
  // token for graph display (needed to cross interfaces)
  unsigned dagToken;
  // window for entire PC display
  string winName;
  // token for PC display, used in PC/UI calls
  unsigned id;
  static unsigned nextID;
  // What's displayed at the top of the Search Display window
  string winTitle;
  // unique identifer for phase, NOT same as DM id's 
  int phaseID;
  dictionary_hash<unsigned, string*> AllNodeFullNames;
};

#endif
