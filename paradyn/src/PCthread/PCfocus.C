/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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
 * PCfocus.C
 * 
 * Initializing focus info needed by PC from DM's resource hierarchies.
 * 
 * $Log: PCfocus.C,v $
 * Revision 1.7  2004/03/23 01:12:27  eli
 * Updated copyright string
 *
 * Revision 1.6  2002/12/20 07:50:02  jaw
 * This commit fully changes the class name of "vector" to "pdvector".
 *
 * A nice upshot is the removal of a bunch of code previously under the flag
 * USE_STL_VECTOR, which is no longer necessary in many cases where a
 * functional difference between common/h/Vector.h and stl::vector was
 * causing a crash.
 *
 * Generally speaking, Dyninst and Paradyn now use pdvector exclusively.
 * This commit DOES NOT cover the USE_STL_VECTOR flag, which will now
 * substitute stl::vector for BPatch_Vector only.  This is currently, to
 * the best of my knowledge, only used by DPCL.  This will be updated and
 * tested in a future commit.
 *
 * The purpose of this, again, is to create a further semantic difference
 * between two functionally different classes (which both have the same
 * [nearly] interface).
 *
 * Revision 1.5  2000/03/06 21:41:22  zhichen
 * Moved /Process hierarchy to /Machine hierarchy.
 *
 * Revision 1.4  1997/03/29 02:04:18  sec
 * Changed /MsgTag to /Message in MDL/paradyn.rc
 *
 * Revision 1.3  1996/08/16 21:03:28  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.2  1996/02/22 18:30:34  karavan
 * cleanup after dataMgr calls; explicitly cast all NULLs to keep
 * AIX happy
 *
 * Revision 1.1  1996/02/02 02:06:34  karavan
 * A baby Performance Consultant is born!
 *
 */

#include <assert.h>
#include "PCintern.h"

//
// known, "base" resources -- these are invariant across applications
//
resourceHandle RootResource;
resourceHandle SyncObject;
resourceHandle Procedures;
resourceHandle Machines;
resourceHandle Locks;
resourceHandle Barriers;
resourceHandle Semaphores;
resourceHandle Messages;

// focus formed from topmost node of each resource hierarchy
focus topLevelFocus;


void initResources()
{
  // only toplevel "base" resources are known to PC 
  // get names <-> handles here

  // first, get handles for base resources guaranteed to exist for every application
  //
  resourceHandle *h = dataMgr->getRootResource();
  assert (h != NULL); 
  RootResource = *h;
  delete h;

  pdvector<resourceHandle> *testr = dataMgr->getRootResources();
  assert (testr != NULL);
  topLevelFocus = dataMgr->getResourceList(testr);
  delete testr;

  h = dataMgr->findResource ("/SyncObject");
  assert (h != NULL);  
  SyncObject = *h;
  delete h;

  h = dataMgr->findResource ("/Code");
  assert (h != NULL); 
  Procedures = *h;
  delete h;

  h = dataMgr->findResource ("/Machine");
  assert (h != NULL); 
  Machines = *h;
  delete h;
  
  //
  // now try toplevels not guaranteed to exist for every application
  //
  h = dataMgr->findResource ("/SpinLock");
  if (h != NULL) {
    Locks = *h;
    delete h;
  }

  h = dataMgr->findResource ("/Barrier");
  if (h != NULL) {
    Barriers = *h;
    delete h;
  }

  h = dataMgr->findResource ("/Semaphore");
  if (h != NULL) {
    Semaphores = *h;
    delete h;
  }

  h = dataMgr->findResource ("/Message");
  if (h != NULL) {
    Messages = *h;
    delete h;
  }

}


