/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

/* 
 * UIwhere.C
 * code related to displaying the where axes lives here
 */

/* $Log: UIwhere.C,v $
/* Revision 1.18  1996/08/16 21:06:50  tamches
/* updated copyright for release 1.1
/*
 * Revision 1.17  1996/08/02 19:08:40  tamches
 * tclclean.h --> tcl.h
 *
 * Revision 1.16  1995/12/01 06:39:03  tamches
 * removed some warnings (tclclean.h; tkclean.h)
 *
 * Revision 1.15  1995/11/06 19:26:41  tamches
 * removed some warnings
 *
 * Revision 1.14  1995/10/17 20:49:12  tamches
 * Where axis changes, e.g. abstractions* instead of
 * abstractions<resourceHandle>*.
 *
 * Revision 1.13  1995/08/04 19:13:56  tamches
 * Added a status line for 'rethinking' after receiving data (whethere batch
 * mode or not)
 *
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
 */

#include "tcl.h"
#include "tk.h"

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
void resourceAddedCB (perfStreamHandle,
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
  if (!inBatchMode)
     ui_status->message("receiving where axis item");

  extern abstractions *theAbstractions;
  assert(theAbstractions);

  abstractions &theAbs = *theAbstractions;
  string theAbstractionName = abs;
  whereAxis &theWhereAxis = theAbs[theAbstractionName];
     // may create a where axis!

  const char *nameLastPart = strrchr(name, '/');
  assert(nameLastPart);
  nameLastPart++;

  theWhereAxis.addItem(nameLastPart, parent, newResource,
		       false, // don't rethink graphics
		       false // don't resort (if not in batch mode, code below
		             // will do that, so don't worry)
		       );

  if (!inBatchMode) {
     ui_status->message("Rethinking after a non-batch receive");

     theWhereAxis.recursiveDoneAddingChildren(true); // true --> resort
     theWhereAxis.resize(true);
        // super-expensive operation...rethinks EVERY node's
        // listbox & children dimensions.  Actually, this only needs
        // to be done for the just-added node and all its ancestors.
     initiateWhereAxisRedraw(interp, true);

     ui_status->message("ready");
  }
}
