/* 
 * UIwhere.C
 * code related to displaying the where axes lives here
 */
/* $Log: UIwhere.C,v $
/* Revision 1.13  1995/08/04 19:13:56  tamches
/* Added a status line for 'rethinking' after receiving data (whethere batch
/* mode or not)
/*
 * Revision 1.12  1995/07/24  21:31:30  tamches
 * removed some obsolete code related to the old where axis
 *
 * Revision 1.11  1995/07/17  05:07:33  tamches
 * Drastic changes related to the new where axis...most of the good stuff
 * is now in different files.
 *
 * Revision 1.10  1995/06/02  20:50:38  newhall
 * made code compatable with new DM interface
 *
 * Revision 1.8  1995/02/16  08:20:50  markc
 * Changed Boolean to bool
 * Changed wait loop code for igen messages
 *
 * Revision 1.7  1995/01/26  17:59:03  jcargill
 * Changed igen-generated include files to new naming convention; fixed
 * some bugs compiling with gcc-2.6.3.
 *
 * Revision 1.6  1994/11/04  20:11:45  karavan
 * changed the name of some frames in the main window, affecting status
 * and resource Display frame parents.
 *
 * Revision 1.5  1994/11/03  19:55:55  karavan
 * added call to dag::setRowSpacing to improve appearance of resource displays
 *
 * Revision 1.4  1994/11/03  06:41:19  karavan
 * took out those pesty debug printfs
 *
 * Revision 1.3  1994/11/03  06:16:16  karavan
 * status display and where axis added to main window and the look cleaned
 * up a little bit.  Added option to ResourceDisplayObj class to specify
 * a parent window for an RDO with the constructor.
 *
 * Revision 1.2  1994/11/01  05:44:24  karavan
 * changed resource selection process to support multiple focus selection
 * on a single display
 *
 * Revision 1.1  1994/10/25  17:58:43  karavan
 * Added support for Resource Display Objects, which display multiple resource
 * Abstractions
 * */

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

#include "UIglobals.h" // UIM_BatchMode
#include "dataManager.thread.h"
#include "paradyn/src/DMthread/DMinclude.h"

#include "whereAxis.h"
#include "abstractions.h"
#include "whereAxisTcl.h"

/* 
 *  resourceAddedCB
 *  This callback function invoked by dataManager whenever a new 
 *  resource has been defined.  Maintains where axis display.
 *  Creates dag for abstraction if none exists.
*/
int numResourceAddedCBSoFar = 0;
void resourceAddedCB (perfStreamHandle handle, 
		      resourceHandle parent, 
		      resourceHandle newResource, 
		      const char *name,
		      const char *abs)
{

#if UIM_DEBUG
  printf ("resourceAddedCB %s\n", name);
#endif

  numResourceAddedCBSoFar++;

//  cout << numResourceAddedCBSoFar << " " << abs << ":" << name
//       << " [" << newResource << "] [" << parent << "]" << endl;

  const bool inBatchMode = (UIM_BatchMode > 0);
  if (!inBatchMode) {
     ui_status->message("receiving where axis item");
  }

  extern abstractions<resourceHandle> *theAbstractions;
  assert(theAbstractions);

  abstractions<resourceHandle> &theAbs = *theAbstractions;
  string theAbstractionName = abs;
  whereAxis<resourceHandle> &theWhereAxis = theAbs[theAbstractionName];
     // may create a where axis!

  char *nameLastPart = strrchr(name, '/');
  assert(nameLastPart);
  nameLastPart++;

  theWhereAxis.addItem(nameLastPart, parent, newResource,
		       false, // don't rethink graphics
		       false // don't resort (if not in batch mode, code below
		             // will do that, so don't worry)
		       );

  if (!inBatchMode) {
     ui_status->message("Rethinking after a non-batch receive");

     theWhereAxis.recursiveDoneAddingChildren();
     theWhereAxis.resize(true);
        // super-expensive operation...rethinks EVERY node's
        // listbox & children dimensions.  Actually, this only needs
        // to be done for the just-added node and all its ancestors.
     initiateWhereAxisRedraw(interp, true);

     ui_status->message("ready");
  }
}
