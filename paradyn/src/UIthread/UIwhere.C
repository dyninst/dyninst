/*
 * Copyright (c) 1996-1998 Barton P. Miller
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

/* $Id: UIwhere.C,v 1.28 2003/07/15 22:46:02 schendel Exp $ */

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
void resourceAddedCB(perfStreamHandle, resourceHandle parent, 
                     resourceHandle newResource, const char *name,
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
  pdstring theAbstractionName = abs;
  whereAxis &theWhereAxis = theAbs[theAbstractionName];
     // may create a where axis!

  // Strip away all path components up to our name. Look for '/', but
  // be careful not to strip away "operator/".
  const char *nameLastPart = strstr(name, "operator/");
  if (nameLastPart == 0) {
      nameLastPart = strrchr(name, '/');
  }
  else {
#ifdef FIXME_AFTER_4
      cerr << "WARNING: Change the whereAxis separator from '/' to something\n"
	   << "else. Otherwise, operator/ needs to be special-cased\n";
#endif
      // Scan back from the current point looking for '/'. Can't use
      // strrchr, since it always scans from the end of the string.
      while (nameLastPart != name && *nameLastPart != '/') {
	  nameLastPart--;
      }
  }

#ifdef i386_unknown_nt4_0
  // under Windows, we may find our Code resources
  // are paths containing backslashes as separators
  // instead of forward slashes.  The strrchr on
  // the forward slash gets us to the path name, now
  // we need to extract the last component of the path.
  const char* backslashNameLastPart = strrchr(nameLastPart, '\\');
  if( backslashNameLastPart != NULL )
  {
    nameLastPart = backslashNameLastPart;
  }
#endif // i386_unknown_nt4_0

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

void resourceRetiredCB(perfStreamHandle handle, resourceHandle uniqueID, 
                       const char *name, const char *abs) {
   extern abstractions *theAbstractions;
   assert(theAbstractions);
   
   abstractions &theAbs = *theAbstractions;
   pdstring theAbstractionName = abs;
   whereAxis &theWhereAxis = theAbs[theAbstractionName];
   // may create a where axis!

   theWhereAxis.retireItem(uniqueID);
   initiateWhereAxisRedraw(interp, true);
}


