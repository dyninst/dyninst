// barChartDriver.C

/* $Log: barChartDriver.C,v $
/* Revision 1.2  1994/09/29 20:05:36  tamches
/* minor cvs fixes
/*
 * Revision 1.1  1994/09/29  19:50:51  tamches
 * initial implementation.
 * entrypoint for barchart C++ program.  we create new tcl
 * commands (which will eventually call back to barChartTcl.C
 * and from there barChart.C) and then launch barChart.tcl.
 *
*/

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

extern "C" {
   int Blt_Init(Tcl_Interp *);
}

Tcl_Interp *MainInterp;

int Tcl_AppInit(Tcl_Interp *interp) {
   // tk library's main() will call this routine for us.  We must perform all
   // needed initializations here...

   MainInterp = interp; // needed by Dg

   if (TCL_ERROR == Tcl_Init(interp))
      return TCL_ERROR;

   if (TCL_ERROR == Tk_Init(interp))
      return TCL_ERROR;

//   if (TCL_ERROR == Blt_Init(interp))
//      // for blt_drag n' drop
//      return TCL_ERROR;

   if (TCL_ERROR == Dg2_Init(interp))
      // installs lots of tcl and C++ callbacks
      return TCL_ERROR;

   tcl_RcFileName = "~/.wishrc";

   // Install our new tcl commands
   Tcl_CreateCommand(interp, "resizeCallback", resizeCallbackCommand,
		     NULL, // ClientData
		     deleteDummyProc);

   Tcl_CreateCommand(interp, "exposeCallback", exposeCallbackCommand,
		     NULL, // ClientData
		     deleteDummyProc);

   Tcl_CreateCommand(interp, "xAxisHasChanged", xAxisHasChangedCommand,
		     NULL, // ClientData
		     deleteDummyProc);

   Tcl_CreateCommand(interp, "yAxisHasChanged", yAxisHasChangedCommand,
		     NULL, // ClientData
		     deleteDummyProc);

   Tcl_CreateCommand(interp, "newScrollPosition", newScrollPositionCommand,
		     NULL, // ClientData
		     deleteDummyProc);

   Tcl_CreateCommand(interp, "dataFormatHasChanged", dataFormatHasChangedCommand,
		     NULL, // ClientData
		     deleteDummyProc);

   Tcl_CreateCommand(interp, "launchBarChart", launchBarChartCommand,
		     NULL, // ClientData
		     deleteLaunchBarChartCommand);

   return TCL_OK;
}

// we use main() from the tk library, which is rigged to call
// int Tcl_AppInit(Tcl_Interp *) which gives us a change to install
// new tcl commands.

extern int main(int argc, char **argv);
int (*dummy)(int, char**) = main; // make use of main() to ensure it gets linked in...
