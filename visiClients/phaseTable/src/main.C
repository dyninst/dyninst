/*
 * $Log: main.C,v $
 * Revision 1.1  1995/12/15 22:01:54  tamches
 * first version of phaseTable
 *
 */

#include <stdio.h>
#include <signal.h>

#include "tclclean.h"
#include "tkclean.h"
#include "tkTools.h"

#include "pdLogo.h"
#include "paradyn/xbm/logo.xbm"

#include "util/h/matherr.h"

extern Dg_Init(Tcl_Interp *interp);

Tcl_Interp *MainInterp;

int app_init() {
    Tk_Window main = Tk_MainWindow(MainInterp);

    if (Dg_Init(MainInterp) == TCL_ERROR)
        return TCL_ERROR;

    if (Tcl_Init(MainInterp) == TCL_ERROR)
	return TCL_ERROR;

    if (Tk_Init(MainInterp) == TCL_ERROR)
	return TCL_ERROR;

    tcl_RcFileName = "~/.wishrc";

    // now install "makeLogo", etc:
    pdLogo::install_fixed_logo("paradynLogo", logo_bits, logo_width,
			       logo_height);
    tcl_cmd_installer createPdLogo(MainInterp, "makeLogo", pdLogo::makeLogoCommand,
				   (ClientData)main);

    // now initialize_tcl_sources created by tcl2c:
    extern int initialize_tcl_sources(Tcl_Interp *);
    if (TCL_OK != initialize_tcl_sources(MainInterp))
       tclpanic(MainInterp, "phaseTable: could not initialize_tcl_sources");
    
    return TCL_OK;
}

int main(int, char **) {
   MainInterp = Tcl_CreateInterp();
   assert(MainInterp);

   Tk_Window mainTkWindow = Tk_CreateMainWindow(MainInterp, NULL,
						"phaseTable",
						"PhaseTable");
   if (mainTkWindow == NULL)
      tclpanic(MainInterp, "phaseTable: could not Tk_CreateMainWindow");

   if (TCL_OK != app_init()) // formerly Tcl_AppInit()
      tclpanic(MainInterp, "PhaseTable: app_init() failed");

   Tk_MainLoop(); // returns when all tk windows are closed
}

