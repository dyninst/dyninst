/*
 * Copyright (c) 1996-2003 Barton P. Miller
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

// $Id: barChartDriver.C,v 1.24 2003/04/15 18:09:54 pcroth Exp $

#include <assert.h>
#include <stdlib.h>
#include <iostream.h>

#include "common/h/Ident.h"
extern "C" const char V_barChart[];
Ident V_id(V_barChart,"Paradyn");
extern "C" const char V_libpdutil[];
Ident V_Uid(V_libpdutil,"Paradyn");
extern "C" const char V_libvisi[];
Ident V_Vid(V_libvisi,"Paradyn");


#include "tcl.h"
#include "tk.h"
#include "tkTools.h"

#include "pdLogo.h"
#include "paradyn/xbm/logo.xbm"

#include "dg2.h"
#include "barChartTcl.h"
#include "barChartUtil.h"
#include "visiClients/auxiliary/h/Export.h"
#include "visiClients/auxiliary/h/NoSoloVisiMsg.h"


// variables used in this file
Tcl_Interp *MainInterp = NULL; // needed by Dg
bool xsynchFlag = false;

// prototypes of functions used in this file
int initialize_tcl_sources(Tcl_Interp *);


void panic(const char *msg) {
   cerr << msg << endl;
   exit(5);
}

int main(int argc, char **argv) {

    // parse our command line
    bool sawParadynFlag = false;
    const char* progName = argv[0];

   for (argc--, argv++; argc > 0; argc--, argv++) {
      if (0==strcmp(*argv, "--xsynch"))
         xsynchFlag = true;
      else if (0==strcmp(*argv, "--debug")) {
         xsynchFlag = true;
#if !defined(i386_unknown_nt4_0)
	 cout << "barChart pid " << getpid() << " at sigpause..." << endl;
	 sigpause(0);
#endif // !defined(i386_unknown_nt4_0)
      }
      else if( strcmp( *argv, "--paradyn" ) == 0 )
      {
        sawParadynFlag = true;
      }
      else {
         cerr << "barChart: unrecognized argument \"" << *argv << "\"" << endl;
         return 5;
      }
   }

   if( !sawParadynFlag )
   {
        ShowNoSoloVisiMessage( progName );
   }


   MainInterp = Tcl_CreateInterp();
   assert(MainInterp);

   if (TCL_OK != Tcl_Init(MainInterp))
      tclpanic(MainInterp, "Could not Tcl_Init()");

   if (TCL_OK != Tk_Init(MainInterp))
      tclpanic(MainInterp, "Could not Tk_Init");

   if (TCL_OK != Dg2_Init(MainInterp))
      tclpanic(MainInterp, "Could not Dg2_Init()");

   Tcl_SetVar(MainInterp, "tcl_interactive", "0", TCL_GLOBAL_ONLY);

   // Install our new tcl commands
   Tcl_CreateCommand(MainInterp, "resizeCallback", resizeCallbackCommand,
		     NULL, // ClientData
		     deleteDummyProc);

   Tcl_CreateCommand(MainInterp, "exposeCallback", exposeCallbackCommand,
		     NULL, // ClientData
		     deleteDummyProc);

   Tcl_CreateCommand(MainInterp, "resourcesAxisHasChanged",
		     resourcesAxisHasChangedCommand,
		     NULL, // ClientData
		     deleteDummyProc);

   Tcl_CreateCommand(MainInterp, "metricsAxisHasChanged", metricsAxisHasChangedCommand,
		     NULL, // ClientData
		     deleteDummyProc);

   Tcl_CreateCommand(MainInterp, "newScrollPosition", newScrollPositionCommand,
		     NULL, // ClientData
		     deleteDummyProc);

   Tcl_CreateCommand(MainInterp, "dataFormatHasChanged", dataFormatHasChangedCommand,
		     NULL, // ClientData
		     deleteDummyProc);

   Tcl_CreateCommand(MainInterp, "rethinkIndirectResourcesCallback",
		     rethinkIndirectResourcesCommand,
		     NULL, // ClientData
		     deleteDummyProc);

   Tcl_CreateCommand(MainInterp, "getMetricColorName",
		     getMetricColorNameCommand,
		     NULL, // ClientData
		     deleteDummyProc);

   Tcl_CreateCommand(MainInterp, "long2shortFocusName", long2shortFocusNameCommand,
		     NULL, // ClientData
		     deleteDummyProc);

   Tcl_CreateCommand(MainInterp, "newMetricMaxValCallback", newMetricMaxValCallbackCommand,
		     NULL, // ClientData
		     deleteDummyProc);

   Tcl_CreateCommand(MainInterp, "launchBarChart", launchBarChartCommand,
		     NULL, // ClientData
		     deleteLaunchBarChartCommand);

   //Create commands for export functionality
   Tcl_CreateObjCommand(MainInterp, "get_subscribed_mrpairs",
	       get_subscribed_mrpairs, NULL, NULL);
   Tcl_CreateObjCommand(MainInterp, "DoExport", DoExport, NULL, NULL);
 
   // Krishna's tcl2c stuff gets loaded here:
   // (initialize_tcl_sources() was auto-generated by 'tcl2c')
   if (TCL_OK != initialize_tcl_sources(MainInterp))
      tclpanic(MainInterp, "barChart: could not initialize_tcl_sources");

//if (TCL_OK != Tcl_EvalFile(MainInterp, "/p/paradyn/development/tamches/core/visiClients/barchart/barChart.tcl"))
//   tclpanic(MainInterp, "could not source barChart.tcl manually");

   pdLogo::install_fixed_logo("paradynLogo", logo_bits, logo_width, logo_height);
   tcl_cmd_installer createPdLogo(MainInterp, "makeLogo",
				  pdLogo::makeLogoCommand,
				  (ClientData)Tk_MainWindow(MainInterp));

   myTclEval(MainInterp, "init_barchart_window");
   myTclEval(MainInterp, "Initialize");

   Tk_MainLoop(); // returns when all tk windows are closed

   // shouldn't we be doing some cleanup here?

   return 0;
}
