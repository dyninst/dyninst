// main.C
// Ariel Tamches

/*
 * $Log: main.C,v $
 * Revision 1.2  1995/11/08 21:17:07  naim
 * Adding matherr exception handler function to avoid error message when
 * computing the "not a number" (NaN) value - naim
 *
 * Revision 1.1  1995/11/04  00:44:36  tamches
 * First version of new table visi
 *
 */

#include <assert.h>
#include <stdlib.h>
#include <iostream.h>
#include "tclclean.h"
#include "tkclean.h"
#include "tkTools.h"

#include "dg2.h"
#include "tableVisi.h"
#include "tableVisiTcl.h"
#include "util/h/matherr.h"

Tcl_Interp *mainInterp;
Tk_Window   mainTkWindow;

void panic(const char *msg) {
   cerr << msg << endl;
   exit(5);
}

tableVisi *theTableVisi; // our main data structure
bool xsynch_flag=false;

int main(int argc, char **argv) {
   if (argc==2 && 0==strcmp(argv[1], "--debug")) {
      xsynch_flag = true;
      cout << "tableVisi at sigpause...pid=" << getpid() << endl;
      sigpause(0);
      argc = 1;
   }

   // tableVisi requires no arguments, thanks to tcl2c.
   if (argc == 3)
      cout << "tableVisi notice: argc should no longer be 3 (ignoring args)" << endl;

   if (argc == 2 || argc>3)
      panic("tableVisi: incorrect #args (should be none)");

   mainInterp = Tcl_CreateInterp();
   assert(mainInterp);

   mainTkWindow = Tk_CreateMainWindow(mainInterp, NULL,
				      "tableVisi",
				      "TableVisi");
   if (mainTkWindow == NULL)
      tclpanic(mainInterp, "Could not Tk_CreateMainWindow()");

   if (xsynch_flag) {
      cout << "xsynching..." << endl;
      XSynchronize(Tk_Display(mainTkWindow), 1);
   }

   if (TCL_OK != Tcl_Init(mainInterp))
      tclpanic(mainInterp, "Could not Tcl_Init");

   if (TCL_OK != Tk_Init(mainInterp))
      tclpanic(mainInterp, "Could not Tk_Init");

   if (TCL_OK != Dg2_Init(mainInterp))
      tclpanic(mainInterp, "Could not Dg2_Init");

   Tcl_SetVar(mainInterp, "tcl_interactive", "0", TCL_GLOBAL_ONLY);

   // Install our new tcl commands here:
   installTableVisiCommands(mainInterp);

   // Krishna's tcl2c stuff:
   extern int initialize_tcl_sources(Tcl_Interp *);
   if (TCL_OK != initialize_tcl_sources(mainInterp))
      tclpanic(mainInterp, "tableVisi: could not initialize_tcl_sources");

//if (Tcl_EvalFile(mainInterp, "/p/paradyn/development/tamches/core/visiClients/tableVisi/tcl/tableVisi.tcl") != TCL_OK)
//   tclpanic(mainInterp, "could not open tableVisi.tcl");

   // Create our main data structure:
   theTableVisi = new tableVisi(mainInterp,
				Tk_NameToWindow(mainInterp, ".body", mainTkWindow),
				"*-Helvetica-*-r-*-14-*", // metric font
				"*-Helvetica-*-r-*-12-*", // metric units font
				"*-Helvetica-*-r-*-14-*", // focus font
				"*-Helvetica-*-r-*-12-*", // cell font
				"lightBlue", // line color
				"blue", // metric color
				"black", // metric units color
				"tomato", // focus color
				"black", // cell color
				"gray", // background color
				2 // initial # sig figs
				);
   assert(theTableVisi);

   Tk_MainLoop(); // returns when all tk windows are closed

   delete theTableVisi;

   // cleanup (uninstall tcl commands, etc.):
   unInstallTableVisiCommands(mainInterp);
}
