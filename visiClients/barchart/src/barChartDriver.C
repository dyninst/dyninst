// barChartDriver.C

/* $Log: barChartDriver.C,v $
/* Revision 1.5  1995/07/06 18:55:31  tamches
/* Now contains a main program suitable for tk4.0
/*
 * Revision 1.4  1994/10/13  00:52:36  tamches
 * Minor additions to support a new command related to sorting
 * of resources
 *
 * Revision 1.3  1994/10/10  23:08:43  tamches
 * preliminary changes on the way to swapping the x and y axes
 *
 * Revision 1.2  1994/09/29  20:05:36  tamches
 * minor cvs fixes
 *
 * Revision 1.1  1994/09/29  19:50:51  tamches
 * initial implementation.
 * entrypoint for barchart C++ program.  we create new tcl
 * commands (which will eventually call back to barChartTcl.C
 * and from there barChart.C) and then launch barChart.tcl.
 *
*/

#include <assert.h>
#include <stdlib.h>
#include <iostream.h>

#include <tcl.h>
#include <tk.h>
#include "dg2.h"
#include "barChartTcl.h"

void panic(char *msg) {
   cerr << msg << endl;
   exit(5);
}

Tcl_Interp *MainInterp = NULL; // needed by Dg

int main(int argc, char **argv) {
   // argc should be 3.  The first argv is of course the program name.
   // The second is "-f".  The third is a full-path-name to the barchart tcl file.
   if (argc != 3)
      panic("barChart: argc should be 3");

   char *tclFileName = argv[2];

   MainInterp = Tcl_CreateInterp();
   assert(MainInterp);

   Tk_Window mainTkWindow = Tk_CreateMainWindow(MainInterp, NULL,
						"barChart",
						"BarChart"
						);
   if (mainTkWindow == NULL) {
      cerr << "Could not Tk_CreateMainWindow()!" << endl;
      cerr << MainInterp->result << endl;
      exit(5);
   }

   if (TCL_OK != Tcl_Init(MainInterp)) {
      cerr << "Could not Tcl_Init: " << MainInterp->result << endl;
      exit(5);
   }

   if (TCL_OK != Tk_Init(MainInterp)) {
      cerr << "Could not Tk_Init: " << MainInterp->result << endl;
      exit(5);
   }

   if (TCL_OK != Dg2_Init(MainInterp)) {
      cerr << "Could not Dg2_Init: " << MainInterp->result << endl;
      exit(5);
   }

   tcl_RcFileName = "~/.wishrc";
   Tcl_SetVar(MainInterp, "tcl_interactive", "0", TCL_GLOBAL_ONLY);

   // Install our new tcl commands
   Tcl_CreateCommand(MainInterp, "resizeCallback", resizeCallbackCommand,
		     NULL, // ClientData
		     deleteDummyProc);

   Tcl_CreateCommand(MainInterp, "exposeCallback", exposeCallbackCommand,
		     NULL, // ClientData
		     deleteDummyProc);

   Tcl_CreateCommand(MainInterp, "resourcesAxisHasChanged", resourcesAxisHasChangedCommand,
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

   Tcl_CreateCommand(MainInterp, "rethinkIndirectResourcesCallback", rethinkIndirectResourcesCommand,
		     NULL, // ClientData
		     deleteDummyProc);

   Tcl_CreateCommand(MainInterp, "launchBarChart", launchBarChartCommand,
		     NULL, // ClientData
		     deleteLaunchBarChartCommand);

   if (TCL_ERROR == Tcl_EvalFile(MainInterp, tclFileName)) {
      cerr << "barChart: Tcl_EvalFile() on \"" << tclFileName << "\" failed." << endl;
      cerr << MainInterp->result << endl;
      exit(5);
   }

   Tk_MainLoop(); // returns when all tk windows are closed

   
}
