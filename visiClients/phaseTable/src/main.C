/*
 * $Log: main.C,v $
 * Revision 1.3  1996/08/05 07:10:55  tamches
 * update for tcl 7.5
 *
 * Revision 1.2  1995/12/20 18:38:25  newhall
 * matherr.h does not need to be included by visis
 *
 * Revision 1.1  1995/12/15 22:01:54  tamches
 * first version of phaseTable
 *
 */

#include <stdio.h>
#include <signal.h>

#include "tcl.h"
#include "tk.h"
#include "tkTools.h"

#include "pdLogo.h"
#include "paradyn/xbm/logo.xbm"

extern Dg_Init(Tcl_Interp *interp);

Tcl_Interp *MainInterp;

int app_init() {
    if (Tcl_Init(MainInterp) == TCL_ERROR)
	return TCL_ERROR;

    if (Tk_Init(MainInterp) == TCL_ERROR)
	return TCL_ERROR;

    if (Dg_Init(MainInterp) == TCL_ERROR)
        return TCL_ERROR;

    // now install "makeLogo", etc:
    pdLogo::install_fixed_logo("paradynLogo", logo_bits, logo_width,
			       logo_height);

    tcl_cmd_installer createPdLogo(MainInterp, "makeLogo", pdLogo::makeLogoCommand,
				   (ClientData)Tk_MainWindow(MainInterp));

    // now initialize_tcl_sources created by tcl2c:
   extern int initialize_tcl_sources(Tcl_Interp *);
   if (TCL_OK != initialize_tcl_sources(MainInterp))
      tclpanic(MainInterp, "phaseTable: could not initialize_tcl_sources");

//    assert(TCL_OK == Tcl_EvalFile(MainInterp, "/p/paradyn/development/tamches/core/visiClients/phaseTable/tcl/phasetbl.tcl"));
    
    return TCL_OK;
}

int main(int, char **) {
//sigpause(0);
   MainInterp = Tcl_CreateInterp();
   assert(MainInterp);

   if (TCL_OK != app_init()) // formerly Tcl_AppInit()
      tclpanic(MainInterp, "PhaseTable: app_init() failed");

   Tk_MainLoop(); // returns when all tk windows are closed
}
