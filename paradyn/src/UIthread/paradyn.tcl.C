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

/* $Id: paradyn.tcl.C,v 1.106 2005/02/15 17:43:58 legendre Exp $
   This code implements the tcl "paradyn" command.  
   See the README file for command descriptions.
*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "common/h/headers.h"
#include "UIglobals.h"
#include "paradyn/src/DMthread/DMinclude.h"
#include "../TCthread/tunableConst.h"
#include "VM.thread.CLNT.h"
#include "pdthread/h/thread.h"
#include "../pdMain/paradyn.h"
#include "abstractions.h"
#include "whereAxisTcl.h"
#include "shgPhases.h"

#include "common/h/pathName.h" // expand_tilde_pathname

#include "Status.h"
#include "pdutil/h/TclTools.h"
#include "ParadynTkGUI.h"


extern bool detachApplication(bool);

extern abstractions* theAbstractions;

status_line *app_name=NULL;
extern status_line *app_status;






void
ParadynTkGUI::DisablePAUSEandRUN( void )
{
  if (Tcl_VarEval(interp,"changeApplicState 2",0)==TCL_ERROR) {
    pdstring msg = pdstring("Tcl interpreter failed in routine changeApplicState: ");
    msg += pdstring(Tcl_GetStringResult(interp));
    uiMgr->showError(83, P_strdup(msg.c_str()));
  }
}

void
ParadynTkGUI::enablePauseOrRun( void )
{
  pdstring msg = "Tcl interpreter failed in routine changeApplicState: ";
  if (GetAppState()==appRunning) {
    if (Tcl_VarEval(interp,"changeApplicState 1",0)==TCL_ERROR) {
      msg += pdstring(Tcl_GetStringResult(interp));
      showError(83, P_strdup(msg.c_str()));
    }   
  }
  else {
    if (Tcl_VarEval(interp,"changeApplicState 0",0)==TCL_ERROR) {
      msg += pdstring(Tcl_GetStringResult(interp));
      showError(83, P_strdup(msg.c_str()));
    }
  }
}

int
ParadynTkGUI::ParadynPauseCmd( int, TCLCONST char **)
{
  // Called by mainMenu.tcl when the PAUSE button is clicked on.
  
  // First, disable the PAUSE button, so we can't click on it twice.
  // Note that we won't enable the RUN button just yet...we wait until
  // the pause has been processed.
  myTclEval(interp, ".parent.buttons.2 configure -state disabled");

  //sleep(1);
  dataMgr->pauseApplication();

  myTclEval(interp, ".parent.buttons.1 configure -state normal");
  
  return TCL_OK;
}

int
ParadynTkGUI::ParadynContCmd( int, TCLCONST char **)
{
  // Called by mainMenu.tcl when the RUN button is clicked on.

  // First, we disable the RUN button so it can't be clicked on again.

  myTclEval(interp, ".parent.buttons.1 configure -state disabled");

  // NOTE: we don't enable the PAUSE button just yet...we wait until
  // the (synchronous) igen call has completed, to avoid a race condition.

  if (!dataMgr->continueApplication())
     cerr << "warning: dataMgr->continueApplication() failed" << endl;

  // Okay, now it's safe to enable the PAUSE button.
  myTclEval(interp, ".parent.buttons.2 configure -state normal");

  return TCL_OK;
}

int
ParadynTkGUI::ParadynStatusCmd( int, TCLCONST char **)
{
  dataMgr->printStatus();
  return TCL_OK;
}

int
ParadynTkGUI::ParadynMetricsCmd( int, TCLCONST char **)
{
  pdvector<pdstring> *ml = dataMgr->getAvailableMetrics(false);
  for (unsigned i=0; i < ml->size(); i++)
    Tcl_AppendElement(interp, const_cast<char*>((*ml)[i].c_str()));
  delete ml;
  return TCL_OK;
}


int
ParadynTkGUI::ParadynDaemonsCmd( int, TCLCONST char **)
{
  pdvector<pdstring> *dl = dataMgr->getAvailableDaemons();
  for (unsigned i=0; i < dl->size(); i++)
    Tcl_AppendElement(interp, const_cast<char*>((*dl)[i].c_str()));
  delete dl;
  return TCL_OK;
}


int
ParadynTkGUI::ParadynResourcesCmd( int, TCLCONST char **)
{
  dataMgr->printResources();
  return TCL_OK;
}

int
ParadynTkGUI::ParadynListCmd( int, TCLCONST char **)
{
  dataMgr->printResources();

  pdvector<pdstring> *ml = dataMgr->getAvailableMetrics(false);
  for (unsigned i=0; i < ml->size(); i++) {
    cout << ((*ml)[i]).c_str() << endl;
  }

  cout << "CONSTANTS" << endl;

  pdvector<tunableBooleanConstant> allBoolConstants = tunableConstantRegistry::getAllBoolTunableConstants();
  for (unsigned boollcv = 0; boollcv < allBoolConstants.size(); boollcv++) {
     tunableBooleanConstant &tbc = allBoolConstants[boollcv];
     cout << tbc.getName() << " = ";
     if (tbc.getValue())
        cout << "true" << endl;
     else
        cout << "false" << endl;
  }

  pdvector<tunableFloatConstant> allFloatConstants = tunableConstantRegistry::getAllFloatTunableConstants();
  for (unsigned floatlcv = 0; floatlcv < allFloatConstants.size(); floatlcv++) {
     tunableFloatConstant &tfc = allFloatConstants[floatlcv];

     cout << tfc.getName() << " = " << tfc.getValue() << endl;
  }

  timeLength bwidth;
  dataMgr->getCurrentBucketWidth(&bwidth);
  cout << "bucketWidth = " << bwidth << endl;
  cout << "number of buckets = " << dataMgr->getMaxBins() << endl;
  dataMgr->printDaemons();
  return TCL_OK;
}

int
ParadynTkGUI::ParadynDetachCmd( int, TCLCONST char **)
{
  dataMgr->detachApplication(true);
  return TCL_OK;
}

metricHandle *
StrToMetHandle (char *mstr)
{
  metricHandle *mh = new metricHandle;
  if (sscanf (mstr, "%u", mh) <= 0) {
    delete mh;
    return (metricHandle *) NULL;
  }
  else return mh;
}

char *
MetHandleToStr (metricHandle mh)
{
  char *result = new char[12];
  sprintf (result, "%u", mh);
  return result;
}

int
ParadynTkGUI::ParadynGetTotalCmd( int argc, TCLCONST char *argv[])
{
  metricHandle *met;
  metricInstInfo *mi;
  pdSample val;

  if (argc < 2) {
    Tcl_SetObjResult(interp,
        Tcl_NewStringObj("USAGE: gettotal <metid>", -1));
    return TCL_ERROR;
  }

  if (!(met = dataMgr->findMetric (argv[1]))) {
    Tcl_AppendElement (interp, "invalid metric identifier");
    return TCL_ERROR;
  }
  
  bool found = uim_enabled.find_with_key(*met, &mi);
  if (! found) {
    Tcl_AppendResult (interp, "unable to find metric ", MetHandleToStr(*met),
		      (char *)NULL);
    delete met;
    return TCL_ERROR;
  }
  else {
    std::ostringstream resstr;
    dataMgr->getTotValue(mi->mi_id, &val);
    resstr << val << std::ends;
    SetInterpResult(interp, resstr);
    delete met;
  }  
  return TCL_OK;
}

int
ParadynTkGUI::ParadynPrintCmd( int, TCLCONST char *argv[])
{
  std::ostringstream resstr;

  if (argv[1][0] == 'm') {   // print metric
    pdSample val;
    metricInstInfo *mi;
    metricHandle *met;

    if (! (met = dataMgr->findMetric (argv[2]))) {
      Tcl_AppendElement (interp, "Invalid metric");
      return TCL_ERROR;
    }
    bool found = uim_enabled.find_with_key(*met, &mi);

    if(! found) {
      resstr << "unable to find metric " << argv[2] << "\n" << std::ends;
      SetInterpResult(interp, resstr);
      delete met;
      return TCL_ERROR;
     } else {
      dataMgr->getMetricValue(mi->mi_id, &val);
      cout << "metric " << dataMgr->getMetricName(*met) << ", val = " 
	   << val << endl;
    }
  } else {
    resstr << "Unknown option: paradyn print " << argv[1] << "\n" << std::ends;
    SetInterpResult(interp, resstr);
    return TCL_ERROR;
  }
  return TCL_OK;
}

void processUsage()
{
  printf("USAGE: process <-user user> <-machine machine> "
         "<-daemon daemon> <-dir directory> <-MPItype mpitype> \"command\"\n");
}

/****
 * Process
 * Calls data manager service "addExecutable".  
 * Returns TCL_OK or TCL_ERROR
 *
 * Note: when there is an error, we should store specific error codes
 *       someplace (presumably, the interpreter's result).  Why?  Because,
 *       right now, any time there's an error in starting up, tcl code puts
 *       up an error dialog box that is so generic as to be nearly useless
 *       to the user.
 */
int
ParadynTkGUI::ParadynAttachCmd( int argc, TCLCONST char* argv[])
{
   const char* user = NULL;
   const char* machine = NULL;
   const char* paradynd = NULL;
   int afterattach = 0; // 0 --> as is, 1 --> pause, 2 --> run

   // cmd gives the path to the executable...used only to read the symbol table.
   const char* cmd = NULL; // program name

   const char* pidstr = NULL;

   for (int i=1; i < argc-1; i++) {
      if (0==strcmp("-user", argv[i]) && i+1 < argc) {
	 user = argv[++i];
      }
      else if (0==strcmp("-machine", argv[i]) && i+1 < argc) {
	 machine = argv[++i];
      }
      else if (0==strcmp("-daemon", argv[i]) && i+1 < argc) {
	 paradynd = argv[++i];
      }
      else if (0==strcmp("-command", argv[i]) && i+1 < argc) {
	 cmd = argv[++i];
      }
      else if (0==strcmp("-pid", argv[i]) && i+1 < argc) {
	 pidstr = argv[++i];
      }
      else if (0==strcmp("-afterattach", argv[i]) && i+1 < argc) {
	 const char *afterattachstr = argv[++i];
	 afterattach = atoi(afterattachstr);
	 if (afterattach < 0 || afterattach > 2) {
	    Tcl_SetResult(interp, "paradyn attach: bad -afterattach value: ", TCL_STATIC);
	    Tcl_AppendResult(interp, afterattachstr, NULL);
	    cerr << Tcl_GetStringResult(interp) << endl;
	    return TCL_ERROR;
	 }
      }
      else {
	 Tcl_SetResult(interp, "paradyn attach: unrecognized option, or option missing required argument: ", TCL_STATIC);
	 Tcl_AppendResult(interp, argv[i], NULL);
	 cerr << Tcl_GetStringResult(interp) << endl;
	 return TCL_ERROR;
      }
   }

   if (pidstr == NULL && cmd == NULL) {
      Tcl_SetResult(interp, "paradyn attach: at least one of the -pid and -command options are required", TCL_STATIC);
      cerr << Tcl_GetStringResult(interp) << endl;
      return TCL_ERROR;
   }

   if (!app_name)
      app_name = new status_line("Application name");

   pdstring theMessage;
   if (cmd)
      theMessage += pdstring("program: ") + cmd + " ";

   if (machine)
      theMessage += pdstring("machine: ") + machine + " ";
   
   if (user)
      theMessage += pdstring("user: ") + user + " ";

   if (paradynd)
      theMessage += pdstring("daemon: ") + paradynd + " ";

   app_name->message(theMessage.c_str());

   if (!app_status) {
      app_status = new status_line("Application status");
   }
  
   // Disabling PAUSE and RUN during attach can help avoid deadlocks.
   DisablePAUSEandRUN();

   // Note: the following is not an igen call to paradynd...just to the DM thread
   if (!dataMgr->attach(machine, user, cmd, pidstr, paradynd, afterattach)) {
      Tcl_SetResult(interp, "", TCL_STATIC);
      return TCL_ERROR;
   }

   return TCL_OK;
}

int
ParadynTkGUI::ParadynProcessCmd( int argc, TCLCONST char *argv[])
{
  const char *user = NULL;
  const char *machine = NULL;
  const char *paradynd = NULL;
  const char *mpitype = NULL;
  pdstring idir;
  int i;
  
  for (i=1; i < argc-1; i++) {
    if (!strcmp("-user", argv[i])) {
      if (i+1 == argc) {
	processUsage();
	return TCL_ERROR;
      }
      user = argv[++i];
    } else if (!strcmp("-machine", argv[i])) {
      if (i+1 == argc) {
	processUsage();
	return TCL_ERROR;
      }
      machine = argv[++i];
    } else if (!strcmp("-daemon", argv[i])) {
      if (i+1 == argc) {
	processUsage();
	return TCL_ERROR;
      }
      paradynd = argv[++i];
    } else if (!strcmp("-dir", argv[i])) {
      if (i+1 == argc) {
	processUsage();
	return TCL_ERROR;
      }
      idir = argv[++i];
    } else if (!strcmp("-MPItype", argv[i])) {
      if (i+1 == argc) {
	processUsage();
	return TCL_ERROR;
      }
      mpitype = argv[++i];
    } else if (argv[i][0] != '-') {
      break;
    } else {
      processUsage();
      return TCL_ERROR;
    }
  }

  if (!app_name) {
    app_name = new status_line("Application name");
  }

  pdstring machine_name;
  if (machine) machine_name=machine;
  else if (default_host.length()) machine_name="(default_host)";
  else machine_name="(local_host)";

  static char tmp_buf[1024];
  sprintf(tmp_buf, "program: %s, machine: %s, user: %s, daemon: %s",
	  argv[i], machine_name.c_str(), user?user:"(self)",
	  paradynd?paradynd:"(defd)");
  app_name->message(tmp_buf);

  if (!app_status) {
    app_status = new status_line("Application status");
  }
 
  pdvector<pdstring> *av = new pdvector<pdstring>();
  unsigned ve=i;
  while (argv[ve]) {
    *av += argv[ve];
    ve++;
  }

  // We disabled PAUSE and RUN buttons to avoid problems. If any of these
  // keys is pressed while defining a process, we end in a deadlock - naim
  DisablePAUSEandRUN();

  // At this point, we take a look at "idir"; if it starts with ~some_user_name,
  // then we alter "idir".  In the spirit of Tcl_TildeSubst (tclGlob.c).
  // The only reason we don't use Tcl_TildeSubst is because it uses Tcl_DStringFree,
  // etc., where we much prefer to use the pdstring class.

  pdstring dir = expand_tilde_pathname(idir); // idir --> "initial dir"

  // Note: the following is not an igen call to paradynd...just to the DM thread.
  char *m = machine ? strdup(machine) : NULL;
  char *u = user ? strdup(user) : NULL;
  char *p = paradynd ? strdup(paradynd) : NULL;
  char *d = dir.length() ? strdup(dir.c_str()) : NULL;
  char *mpi = mpitype ? strdup(mpitype) : NULL;
  dataMgr->addExecutable(m, u, p, d, mpi, av);
  return TCL_OK;
}

//
//  disable  <metid>
//
int
ParadynTkGUI::ParadynDisableCmd( int argc, TCLCONST char* argv[])
{
  metricHandle *met;
  metricInstInfo *mi;
  std::ostringstream resstr;

  // Hold Everything!
  dataMgr->pauseApplication ();

  if (argc < 2) {
    resstr << "USAGE: disable <metid>" << std::ends;
    SetInterpResult(interp, resstr);
    return TCL_ERROR;
  }

  if (! (met = dataMgr->findMetric(argv[1]))) {
    resstr << "Invalid metric " << argv[1] << std::ends;
    SetInterpResult(interp, resstr);
    return TCL_ERROR;
  }

  bool found = uim_enabled.find_with_key(*met, &mi);

  if(! found) {
    resstr << "unable to find metric " << MetHandleToStr(*met) << "\n"
           << std::ends; 
    SetInterpResult(interp, resstr);
    delete met;
    return TCL_ERROR;
  }
  else {
    // TODO: phase type should be entered as a command arg 
    dataMgr->disableDataCollection(GetPerfStreamHandle(),
                                    0,
                                    mi->mi_id,
                                    GlobalPhase);
    delete met;
  }
  return TCL_OK;
}

//
//  enable <metric> ?<resource>? ...
//    returns metric id
//
int
ParadynTkGUI::ParadynEnableCmd( int argc, TCLCONST char* argv[] )
{
  metricHandle *met;
  metricInstInfo *mi;
  pdvector<resourceHandle> *resList;
  std::ostringstream resstr;


  // Hold Everything!
  dataMgr->pauseApplication ();

  // Build a resource list from the tcl list
  if (argc == 2)
    resList = dataMgr->getRootResources();
  else {
    TCLCONST char **argsv;
    int argsc;
    resourceHandle *res;

    if (Tcl_SplitList(interp, argv[2], &argsc, &argsv) != TCL_OK) {
      printf("Error parsing resource list '%s'", argv[2]);
      return TCL_ERROR;
    }

    resList = new pdvector<resourceHandle>;
    cout << "enable request for ";
    for (int i = 0; i < argsc; i++) {
      res = dataMgr->findResource(argsv[i]);
      cout << argsv[i] << " ";
      resList += *res;
    }
    cout << endl;
    Tcl_Free((char*)argsv);
  }

  // Now check the metric
  met = dataMgr->findMetric (argv[1]);
  if (!met) {
    resstr << "metric " << argv[1] << " is not defined\n" << std::ends;
    SetInterpResult(interp, resstr);
    delete resList;
    return TCL_ERROR;
  }
  else {

    // Finally enable the data collection
    // TODO: phaseType, and persistent flags should be command args
    
    pdvector<metric_focus_pair> *request = new pdvector<metric_focus_pair>;
    metric_focus_pair new_request_entry(*met,*resList);
    *request += new_request_entry;
    assert(request->size() == 1);
    // 0 is used as the second parameter for non-trace use 
    dataMgr->enableDataRequest(GetPerfStreamHandle(),
                                0, request, 0,
                                GlobalPhase,0,0,0,0);

    // KLUDGE: wait for async response from DM
    bool ready=false;
    pdvector<metricInstInfo> *response;
    // wait for response from DM
    while(!ready){
      T_dataManager::T_enableDataCallback* rval = NULL;
 	  T_dataManager::message_tags waitTag;
	  tag_t tag = T_dataManager::enableDataCallback_REQ;
	  thread_t from;
	  int err = msg_poll(&from, &tag, true);
	  assert(err != THR_ERR);
	  if (dataMgr->isValidTag((T_dataManager::message_tags)tag)) {
	      waitTag = dataMgr->waitLoop(true,
			(T_dataManager::message_tags)tag, (void**)&rval);
              if(waitTag == T_dataManager::enableDataCallback_REQ){
                ready = true;
                response = rval->response;
                rval->response = 0;
                delete rval;
	      }
	      else {
		  cout << "error UI wait data enable resp:tag invalid" << endl;
		  assert(0);
	      }
	  }
	  else{
	      cout << "error UI wait data enable resp:tag invalid" << endl;
	      assert(0);
	  }
    } // while(!ready)
    mi = 0;
    // if this MI was successfully enabled
    if(response && (*response)[0].successfully_enabled) {
	  mi = new metricInstInfo;
	  mi->successfully_enabled = (*response)[0].successfully_enabled;
	  mi->mi_id = (*response)[0].mi_id;
	  mi->m_id = (*response)[0].m_id;
	  mi->r_id = (*response)[0].r_id;
	  mi->metric_name = (*response)[0].metric_name;
	  mi->metric_units = (*response)[0].metric_units;
	  mi->focus_name = (*response)[0].focus_name;
	  mi->units_type = (*response)[0].units_type;
    }

    if (mi) {
      uim_enabled.push_front(mi, mi->mi_id);
      resstr << MetHandleToStr (mi->mi_id) << std::ends;
      SetInterpResult(interp, resstr);
      printf ("metric %s, id = %s\n", argv[1], MetHandleToStr(mi->mi_id));
    } else {
      resstr << "can't enable metric " << argv[1] << " for focus \n"
             << std::ends;
      SetInterpResult(interp, resstr);
      return TCL_ERROR;
    }
  }
  return TCL_OK;
}

int
ParadynTkGUI::ParadynCoreCmd( int argc, TCLCONST char* argv[])
{
  int pid;

  if (argc != 2) {
    printf("usage: paradyn core <pid>\n");
    return TCL_ERROR;
  }
  if (sscanf(argv[1],"%d",&pid) != 1) {
    printf("usage: paradyn core <pid>\n");
    return TCL_ERROR;
  }

  dataMgr->coreProcess(pid);
  return TCL_OK;
}

int
ParadynTkGUI::ParadynSetCmd( int argc, TCLCONST char* argv[] )
{
  // args: <tunable-name> <new-val>
  if (argc != 3) {
    std::ostringstream resstr;

    resstr << "USAGE: " << argv[0] << " <variable> <value>" << std::ends;
    SetInterpResult(interp, resstr);
    return TCL_ERROR;
  }

  if (!tunableConstantRegistry::existsTunableConstant(argv[1])) {
     cout << "Tunable constant " << argv[1] << " does not exist; cannot set its value to " << argv[2] << endl;
     return TCL_ERROR;
  }
  
  if (tunableConstantRegistry::getTunableConstantType(argv[1]) == tunableBoolean) {
     int boolVal;
     if (TCL_ERROR == Tcl_GetBoolean(interp, argv[2], &boolVal))
        return TCL_ERROR;
     else {
        tunableConstantRegistry::setBoolTunableConstant(argv[1], (bool)boolVal);
//        cout << "tunable boolean constant " << argv[1] << " set to " << boolVal << endl;
     }
  }
  else {
     double doubleVal;
     if (TCL_ERROR == Tcl_GetDouble(interp, argv[2], &doubleVal))
        return TCL_ERROR;
     else {
        tunableConstantRegistry::setFloatTunableConstant(argv[1], (float)doubleVal);
//        cout << "tunable float constant " << argv[1] << " set to " << doubleVal << endl;
     }
  }

  return TCL_OK;
}

int
ParadynTkGUI::ParadynWaSetAbstraction( int argc, TCLCONST char* argv[] )
{
   if (argc != 2) {
      cerr << "ParadynWaSetAbstraction: wrong # args" << endl;
      return TCL_ERROR;
   }
   
   assert(0==strcmp(argv[0], "waSetAbstraction"));
   pdstring absName = pdstring(argv[1]);

   int menuIndex = theAbstractions->name2index(absName);
      // -1 if not found

   if (menuIndex == -1) {
      cout << "paradyn waSetAbstraction: could not change the abstraction to \"" << absName << "\" because it does not (yet?) exist" << endl;
      return TCL_ERROR;
   }

   menuIndex++; // tcl menus are base 1 (0 is reserved for tearoff)

   pdstring commandStr = theAbstractions->getAbsMenuName() + " invoke " +
                       pdstring(menuIndex);
   cout << "invoking menu item " << menuIndex << endl;

   if( Tcl_EvalObjEx( interp,
                        Tcl_NewStringObj( commandStr.c_str(), -1 ),
                        TCL_EVAL_DIRECT ) != TCL_OK )
   {
      cerr << Tcl_GetStringResult(interp) << endl;
      exit(5);
   }

   return TCL_OK;
}

int ParadynWaSelectUnselect(Tcl_Interp *interp,
			    const char *name,
			    bool selectFlag) {
   if (!theAbstractions->existsCurrent())
      return TCL_ERROR;

   const bool found = theAbstractions->
                      selectUnSelectFromFullPathName(name, selectFlag);
   if (!found) {
      if (selectFlag)
         cout << "paradyn waSelect: ";
      else
         cout << "paradyn waUnselect: ";
      cout << "could not find the item: " << name << endl;
   }
   else
      initiateWhereAxisRedraw(interp, true); // whereAxisTcl.C

   return TCL_OK;
}

int
ParadynTkGUI::ParadynWaSelect( int argc, TCLCONST char* argv[] )
{
   if (argc != 2) {
      cerr << "ParadynWaSelect: too many arguments" << endl;
      return TCL_ERROR;
   }

   assert(0==strcmp(argv[0], "waSelect"));
   return ParadynWaSelectUnselect(interp, argv[1], true);
}

int
ParadynTkGUI::ParadynWaUnSelect( int argc, TCLCONST char* argv[] )
{
   if (argc != 2) {
      cerr << "ParadynWaUnselect: too many arguments" << endl;
      return TCL_ERROR;
   }

   assert(0==strcmp(argv[0], "waUnselect"));
   return ParadynWaSelectUnselect(interp, argv[1], false);
}

int
ParadynTkGUI::ParadynApplicationDefinedCmd( int, TCLCONST char**)
{
   // returns true iff an application has been defined
   setResultBool(interp, dataMgr->applicationDefined());
   return TCL_OK;
}
				
int
ParadynTkGUI::ParadynSuppressCmd( int argc, TCLCONST char* argv[] )
{
  bool suppressInst, suppressChildren = false;

  if (argc != 3) {
    printf("Usage: paradyn suppress <search|inst|searchChildren> <resource list>\n");
    return TCL_ERROR;
  }
  if (!strcmp(argv[1], "search")) {
    suppressInst = false;
    suppressChildren = false;
  } else if (!strcmp(argv[1], "inst")) {
    suppressInst = true;
  } else if (!strcmp(argv[1], "searchChildren")) {
    suppressInst = false;
    suppressChildren = true;
  } else {
    printf("Usage: paradyn suppress <search|inst|searchChildren> <resource list>\n");
    return TCL_ERROR;
  }

  {
    TCLCONST char **argsv;
    int argsc;
    resourceHandle *res;
    
    if (Tcl_SplitList(interp, argv[2], &argsc, &argsv) != TCL_OK) {
      printf("Error parsing resource list '%s'", argv[2]);
      return TCL_ERROR;
    }
    
    cout << "suppress request for ";
    for (int i = 0; i < argsc; i++) {
      res = dataMgr->findResource (argsv[i]);
      cout << argsv[i];

      if (res == NULL) {
         cerr << "sorry, data manager could not findResource" << endl;
         return TCL_ERROR;
      }

      if (suppressInst) {
	dataMgr->setResourceInstSuppress(*res, true);
      } else {
	if (suppressChildren)
	  dataMgr->setResourceSearchChildrenSuppress(*res, true);
	else
	  dataMgr->setResourceSearchSuppress(*res, true);
      }
      delete res;
    }
    cout << endl;
    Tcl_Free((char*)argsv);
    return TCL_OK;
  }
}

int
ParadynTkGUI::ParadynVisiCmd( int argc, TCLCONST char* argv[] )
{
  std::ostringstream resstr;

//
//  @begin(barf)
//    This should be automated with a visi command 
//    dictionary or at least a switch statement.  --rbi
//  @end(barf)
//
  if (argc < 2) {
    resstr << "USAGE: visi [kill <ivalue>|create <ivalue>|info|active<cmd>]"
           << std::ends;
    SetInterpResult(interp, resstr);
    return TCL_ERROR;
  }
  if (argv[1][0] == 'a') {
    pdvector<VM_activeVisiInfo> *temp;

    temp = vmMgr->VMActiveVisis();
    for (unsigned i=0; i < temp->size(); i++) {
      printf("active_info %d: name %s TypeId %d visiNum = %d\n",i,
	     ((*temp)[i]).name.c_str(),
	     ((*temp)[i]).visiTypeId,((*temp)[i]).visiNum);
    }
    delete temp;
  }
  else if (argv[1][0] == 'i') {
      pdvector<VM_visiInfo> *visi_info;

      visi_info = vmMgr->VMAvailableVisis();
      for (unsigned i=0; i < visi_info->size();i++) {
	printf("visi %d: name %s visiTypeId %d\n",i,
	       ((*visi_info)[i]).name.c_str(), 
	       ((*visi_info)[i]).visiTypeId);
      }
      delete visi_info;
    } 
  else if (argv[1][0] == 'c') {
    int i;
    int j;
    if (Tcl_GetInt (interp, argv[2], &i) != TCL_OK) 
      return TCL_ERROR;
    if (Tcl_GetInt (interp, argv[3], &j) != TCL_OK) 
      return TCL_ERROR;
    if(j == 1){
        vmMgr->VMCreateVisi(1,-1,i,CurrentPhase,NULL); 
    }
    else {
        vmMgr->VMCreateVisi(1,-1,i,GlobalPhase,NULL); 
    }
  } 
  else if (argv[1][0] == 'k') {
    int i;
    if (Tcl_GetInt (interp, argv[2], &i) != TCL_OK) 
      return TCL_ERROR;
    vmMgr->VMDestroyVisi(i);
  } 
  else {
    resstr << "USAGE: visi [kill <ivalue>|create <ivalue>|info|active<cmd>]"
           << std::ends;
    SetInterpResult(interp, resstr);
    return TCL_ERROR;
  }
  return TCL_OK;
}

int
ParadynTkGUI::ParadynSaveCmd( int argc, TCLCONST char* argv[] )
{
  std::ostringstream resstr;

  if (argc == 4) {
    if (!strcmp(argv[1], "data")) {
      // "save data [global|phase|all] <dirname>" 
      switch (argv[2][0]) {
      case 'a':
	dataMgr->saveAllData(argv[3], All);
	break;
      case 'g':
	dataMgr->saveAllData(argv[3], Global);
	break;
      case 'p':
	dataMgr->saveAllData(argv[3], Phase);
	break;
      default:
	resstr << "USAGE: save data [global|phase|all] <dirname>\n" << std::ends;
    SetInterpResult(interp, resstr);
	return TCL_ERROR;
      }
      return TCL_OK;
    } else if (!strcmp(argv[1], "resources")) {
      // "save resources all <filename>"
      dataMgr->saveAllResources(argv[3]);
      return TCL_OK;
    } else if (!strcmp(argv[1], "shg")) {
      // "save shg [global|phase]"
      if (!strcmp(argv[2], "phase"))
        perfConsult->saveSHG(argv[3], 0);   // save phase shg(s)
      else
        perfConsult->saveSHG(argv[3], 1);   // save global shg
      return TCL_OK;
    }
  }
  resstr << "USAGE: save data [global|phase|all] <dirname>\n save resources all <file>\n save shg [global|phase|all] <dirname>\n" << std::ends;
  SetInterpResult(interp, resstr);
  return TCL_ERROR;
}

int
ParadynTkGUI::ParadynDaemonStartInfoCmd( int, TCLCONST char** )
{
  dataMgr->displayDaemonStartInfo();
  return TCL_OK;
}

int
ParadynTkGUI::ParadynGeneralInfoCmd( int, TCLCONST char**)
{
  dataMgr->displayParadynGeneralInfo();
  return TCL_OK;
}

int
ParadynTkGUI::ParadynLicenseInfoCmd(int, TCLCONST char**)
{
  dataMgr->displayParadynLicenseInfo();
  return TCL_OK;
}

int
ParadynTkGUI::ParadynReleaseInfoCmd(int, TCLCONST char**)
{
  dataMgr->displayParadynReleaseInfo();
  return TCL_OK;
}

int
ParadynTkGUI::ParadynVersionInfoCmd(int, TCLCONST char**)
{
  dataMgr->displayParadynVersionInfo();
  return TCL_OK;
}

