/* 
 * UIwhere.C
 * code related to displaying the where axes lives here
 */
/* $Log: UIwhere.C,v $
/* Revision 1.11  1995/07/17 05:07:33  tamches
/* Drastic changes related to the new where axis...most of the good stuff
/* is now in different files.
/*
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

#include "string.h"
#include "UIglobals.h"
#include "dataManager.thread.h"
#include "paradyn/src/DMthread/DMinclude.h"
#include "../pdMain/paradyn.h"
//#include "dag.h"

#include "whereAxis.h"
#include "abstractions.h"
#include "whereAxisTcl.h"

//List<resourceDisplayObj *> resourceDisplayObj::allRDOs;
//tokenHandler tokenClerk; 
//int resourceDisplayObj::rdoCount = 0;
//List<stringHandle> uim_knownAbstractions;

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
//  List<resourceDisplayObj *> tmp;
#if UIM_DEBUG
  printf ("resourceAddedCB %s\n", name);
#endif

  numResourceAddedCBSoFar++;

//  cout << numResourceAddedCBSoFar << " " << abs << ":" << name
//       << " [" << newResource << "] [" << parent << "]" << endl;


//  char *aname = new char[strlen(abs)+1];
//  strcpy (aname, abs);
//  char *rname = new char[strlen(name)+1];
//  strcpy (rname, name);
//  // add this abstraction to all existing resourceDisplayObjs
//  tmp = resourceDisplayObj::allRDOs;
//  while (*tmp) {    
//    (*tmp)->addResource (newResource, parent, rname, aname);
//    tmp++;
//  }

  /* ******************************************************* */

  extern abstractions<resourceHandle> *theAbstractions;
  assert(theAbstractions);

  abstractions<resourceHandle> &theAbs = *theAbstractions;
  string theAbstractionName = abs;
  whereAxis<resourceHandle> &theWhereAxis = theAbs[theAbstractionName];
     // may create a where axis!

  const bool inBatchMode = (UIM_BatchMode > 0);
  char *nameLastPart = strrchr(name, '/');
  assert(nameLastPart);
  nameLastPart++;

  theWhereAxis.addItem(nameLastPart, parent, newResource,
		       false // don't rethink graphics
		       );

  if (!inBatchMode) {
     theWhereAxis.recursiveDoneAddingChildren();
     theWhereAxis.resize(true);
        // super-expensive operation...rethinks EVERY node's
        // listbox & children dimensions.  Actually, think only needs
        // to be done for the just-added node and all its ancestors.
     initiateWhereAxisRedraw(interp, true);
  }
}

/* initWhereAxis
 * initialization, including mapping to the screen, of a where axis
 * returns 0 if error, token otherwise
 */
//int initWhereAxis (dag *wheredag, stringHandle abs, int rdoToken, 
//		   int dagToken, char *pwin, int mapflag) 
//{
//  char tcommand[250];
//  char winname[100];
//  sprintf (winname, "%s.dag.dag%s", pwin, (char *)abs);
//  sprintf (tcommand, "initRDOdag %s %s", pwin, (char *)abs);
//  if (Tcl_VarEval (interp, tcommand, 0) == TCL_ERROR) {
//    printf ("NOWHEREDAG:: %s\n", interp->result);
//    return 0; 
//  }
//  wheredag->createDisplay (winname);
//  sprintf (tcommand, "addDefaultWhereSettings %s %d",
//           wheredag->getCanvasName(), dagToken);
//  if (Tcl_VarEval (interp, tcommand, 0) == TCL_ERROR) {
//    printf ("ERROR ADDING Where Bindings %s\n", interp->result);
//    return 0;
//  }
//  if (mapflag) {
//    sprintf (tcommand, "mapRDOdag %d %d %s %s", rdoToken, dagToken,
//	     pwin, (char *)abs);
//#if UIM_DEBUG
//    printf ("%s\n", tcommand);
//#endif
//    if (Tcl_VarEval (interp, tcommand, 0) == TCL_ERROR) {
//      printf ("CANTMAPDAG:: %s\n", interp->result);
//      return 0; 
//    }
//  }
//  return 1;
//}
//
//dag * 
//resourceDisplayObj::addAbstraction (char *newabs) 
//{
//  List<dag *> tptr;
//  int retVal, dagToken;
//  dag *newdag;
//
//  dag *tempdata;
//  for(tptr = dags; tempdata = *tptr; tptr++){
//      if(strcmp(newabs,tempdata->getAbstraction())){
//          return 0;
//      }
//  }
//  //  if (tptr.find ((void *) ah))
//  //    return (dag *) NULL;
//  newdag = new dag(interp);
//  newdag->setAbstraction(newabs);
//  numdags++;
//  dagToken = tokenClerk.getToken (newdag);
//#if UIM_DEBUG
//  printf ("adding %d to activeDags\n", dagToken);
//#endif
//  ActiveDags[dagToken] = newdag;
//
//  // initialize display for this abstraction 
//  retVal = initWhereAxis (newdag, newabs, token, dagToken, parentwin,
//			  (numdags == 1));
//  if (retVal < 0) {
//    printf ("Unable to initialize where axis display\n");
//    delete newdag;
//    return (dag *)NULL;
//  }
//  newdag->setRowSpacing(40);
//  //  dags.add (newdag, (void *) ah);
//  dags.add (newdag);
//  if (numdags == 1) 
//    topdag = newdag;
//  uim_knownAbstractions.add (newabs);
//  return newdag;
//}
//
//resourceDisplayObj::resourceDisplayObj (int baseflag, int &success, 
//					const char *pwin)
//{
//  numdags = 0;	
//  status = DISPLAYED;	
//  topdag = NULL;
//  base = baseflag;
//  token = rdoCount;
//  rdoCount++;
//  sprintf (parentwin, "%s", pwin);
//  sprintf (tbuf, "initRDO %d %s {Paradyn Where Axis Display} 0",
//	   token, parentwin);
//  if (Tcl_VarEval (interp, tbuf, 0) == TCL_ERROR) {
//    sprintf (tbuf, "Can't initialize RDO: %s", interp->result);
//    uim_server->showError(26, tbuf);
//    success = 0; 
//  } else {
//    allRDOs.add(this, (void *)token);
//    success = 1;
//  }
//}
//
//resourceDisplayObj::resourceDisplayObj (int baseflag, int &success) 
//{
//  numdags = 0;	
//  status = DISPLAYED;	
//  topdag = NULL;
//  base = baseflag;
//  token = rdoCount;
//  rdoCount++;
//  sprintf (parentwin, ".parent.where%d", token);
//
//  sprintf (tbuf, "initRDO %d %s {Paradyn Where Axis Display} 1",
//	   token, parentwin);
//  if (Tcl_VarEval (interp, tbuf, 0) == TCL_ERROR) {
//    sprintf (tbuf, "Can't initialize RDO: %s", interp->result);
//    uim_server->showError(26, tbuf);
//    success = 0; 
//  } else {
//    allRDOs.add(this, (void *)token);
//    success = 1;
//  }
//}
//
///* 
// * add a single resource to the appropriate dag within this rDO,
// * creating the dag if none found for the resource's abstraction
//*/
//void
//resourceDisplayObj::addResource (resourceHandle newres, resourceHandle parent, 
//				 char *name, char *abs)
//{ 
//  nodeIdType nodeID;
//  dag *adag;
//  char *label, *nptr;
//  List<dag *> tptr;
//  // tptr = dags;
//
//  bool found = false;
//  // find dag for this abstraction; if none, create one
//  for(tptr = dags; adag = *tptr; tptr++){
//      if(strcmp(abs,adag->getAbstraction()) == 0){
//          found = true;
//	  break;
//      }
//  }
//  if (!found) {
//    adag = this->addAbstraction(abs);
//  }
//
///*
//   adag = tptr.find ((void *) ah);
//  if (adag == NULL) {
//    adag = this->addAbstraction(abs, ah);
//#if UIM_DEBUG
//    printf ("addResource: abs name %s not found, added\n", (char *)abs);
//#endif
//  }
//*/
//
//  nodeID = (nodeIdType) newres;
//  nptr = P_strrchr(name, '/'); nptr++;
//  label = new char[strlen(nptr)+1];
//  strcpy (label, nptr);
//  if (parent == uim_rootRes) {
//    adag->CreateNode (nodeID, 1, label, 1, (void *)NULL);
//  }
//  else {
//    adag->CreateNode (nodeID, 0, label, 1, (void *)NULL);
//    adag->AddEdge ((nodeIdType) parent, nodeID, 1);
//  }
//}
//
//int
//resourceDisplayObj::cycle (char *oldab)
//{
//  int newtoken;
//  List<stringHandle> tmp;
//  stringHandle *firstab;
//  stringHandle *newab = NULL;
//  dag *newdag;
//
//  // if there's only one dag, do nothing
//  if (numdags <= 1)
//    return 1;
//  // locate next dag to display
//  // first get next abstraction
//  
//  tmp += uim_knownAbstractions;
//  firstab = (stringHandle *) *tmp;
//  while (*tmp) {    
//    if (!strcmp((char *)(*tmp), oldab)) {
//      tmp++;
//      if (*tmp)
//	newab = (stringHandle *) *tmp;
//      else
//	newab = firstab;
//      break;
//    }
//    tmp++;
//  }
//  if (newab == NULL)
//    return 0;
//
//  // get token for new dag
//  newdag = dags.find(newab);
//  topdag = newdag;
//  newtoken = tokenClerk.reportToken(newdag);
//  if (newtoken < 0)
//    return 0;
//  // change displayed dag to newdag
//  sprintf (tbuf, "unmapRDOdag %s %s", parentwin, (char *)oldab);
//#if UIM_DEBUG
//  printf ("%s\n", tbuf);
//#endif
//  if (Tcl_VarEval (interp, tbuf, 0) == TCL_ERROR) {
//    printf ("CANTUNMAPDAG:: %s\n", interp->result);
//    return 0; 
//  }
//  sprintf (tbuf, "mapRDOdag %d %d %s %s", token, newtoken,
//	   parentwin, (char *)newab);
//#if UIM_DEBUG
//  printf ("%s\n", tbuf);
//#endif
//  if (Tcl_VarEval (interp, tbuf, 0) == TCL_ERROR) {
//    printf ("CANTMAPDAG:: %s\n", interp->result);
//    return 0;
//  } 
//  return 1;
//}
// 
//int
//tokenHandler::getToken (void *obj) 
//{
//  tokenRec *newRec;
//  newRec = new tokenRec;
//  newRec->token = counter++;
//  newRec->object = obj;
//  store.add (newRec, (void *)newRec->token);
//  return newRec->token;
//}
//
//int
//tokenHandler::reportToken (void *obj)
//{
//  List<tokenRec *> tmp;
//  tmp = store;
//  while (*tmp) {
//    if ((*tmp)->object == obj)
//      return (*tmp)->token;
//    else 
//      tmp++;
//  }
//#if UIM_DEBUG
//  printf ("object not found by tokenclerk\n");
//#endif
//  return -1;
//}
//
//tokenRec *
//tokenHandler::translateToken (int token)
//{
//  return store.find ((void *)token);
//}
//
//bool
//tokenHandler::invalidate (int token)
//{
//  return store.remove((void *)token);
//}
//
//
//int initMainWhereDisplay ()
//{
//  resourceDisplayObj *newRec;
//  int alliswell = 0;
//  newRec = new resourceDisplayObj(1, alliswell, ".parent.where");
//  if (!alliswell) {
//    //handle error in constructor
//#if UIM_DEBUG
//    printf ("error in constructor\n");
//#endif
//    delete newRec;
//  }
//  return alliswell;
//}
