/*
 * Copyright (c) 1996-1999 Barton P. Miller
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

// $Id: main.C,v 1.8 2000/07/28 17:23:01 pcroth Exp $

#include <stdio.h>
#include <signal.h>

#include "common/h/headers.h"

#include "tcl.h"
#include "tk.h"
#include "tkTools.h"

#include "pdLogo.h"
#include "paradyn/xbm/logo.xbm"

#include "common/h/Ident.h"
extern "C" const char V_phaseTable[];
Ident V_id(V_phaseTable,"Paradyn");
extern "C" const char V_libpdutil[];
Ident V_Uid(V_libpdutil,"Paradyn");
extern "C" const char V_libvisi[];
Ident V_Vid(V_libvisi,"Paradyn");

extern int Dg_Init(Tcl_Interp *interp);

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

   return 0;
}
