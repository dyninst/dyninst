// main.C
// Ariel Tamches

/*
 * $Log: main.C,v $
 * Revision 1.6  1995/12/20 18:37:16  newhall
 * matherr.h does not need to be included by visis
 *
 * Revision 1.5  1995/11/29 00:43:33  tamches
 * paradyn logo is now hard-coded
 *
 * Revision 1.4  1995/11/18 08:34:45  tamches
 * initial # of sig figs is now 3
 *
 * Revision 1.3  1995/11/08 21:53:57  tamches
 * changed focus color from tomato to maroon3
 * cleaner initialization since dg2.C now strictly handles just the "Dg" tcl cmd
 *
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

#include "pdLogo.h"
#include "paradyn/xbm/logo.xbm"

#include "visi/h/visualization.h"
#include "dg2.h"

#include "tableVisi.h"
#include "tableVisiTcl.h"

Tcl_Interp *mainInterp;
Tk_Window   mainTkWindow;

void panic(const char *msg) {
   cerr << msg << endl;
   exit(5);
}

tableVisi *theTableVisi; // our main data structure
bool xsynch_flag=false;

void visiFdReadableHandler(ClientData, int) {
   // Installed as a file-handler routine for whenever data arrives over
   // the socket returned from VisiInit()
   if (visi_callback() == -1)
      exit(0);
}

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

   int fd = VisiInit();
   if (fd < 0)
      panic("failed to initialize w/ visi lib");

   Tcl_SetVar(mainInterp, "tcl_interactive", "0", TCL_GLOBAL_ONLY);

   // Install our new tcl commands here:
   installTableVisiCommands(mainInterp);

   if (RegistrationCallback(ADDMETRICSRESOURCES, Dg2AddMetricsCallback) != 0)
      panic("Dg2_Init() -- couldn't install ADDMETRICSRESOURCES callback");

   if (RegistrationCallback(DATAVALUES, Dg2NewDataCallback) != 0)
      panic("Dg2_Init() -- couldn't install DATAVALUES callback");

   Tk_CreateFileHandler(fd, TK_READABLE, visiFdReadableHandler, 0);

   // Krishna's tcl2c stuff:
   extern int initialize_tcl_sources(Tcl_Interp *);
   if (TCL_OK != initialize_tcl_sources(mainInterp))
      tclpanic(mainInterp, "tableVisi: could not initialize_tcl_sources");

//if (Tcl_EvalFile(mainInterp, "/p/paradyn/development/tamches/core/visiClients/tableVisi/tcl/tableVisi.tcl") != TCL_OK)
//   tclpanic(mainInterp, "could not open tableVisi.tcl");

   pdLogo::install_fixed_logo("paradynLogo", logo_bits, logo_width, logo_height);
   tcl_cmd_installer createPdLogo(mainInterp, "makeLogo", pdLogo::makeLogoCommand,
				  (ClientData)mainTkWindow);

   myTclEval(mainInterp, "initializeTableVisi");

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
				"maroon3", // focus color
				"black", // cell color
				"gray", // background color
				3 // initial # sig figs
				);
   assert(theTableVisi);

   Tk_MainLoop(); // returns when all tk windows are closed

   delete theTableVisi;
}
