/*
 * Copyright (c) 1996 Barton P. Miller
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

/*
 *  tclVisi.C -- This file handles the bare essentials of tcl 
 *        application initialization.  Essentially, it implements the
 *        Tcl_AppInit() function.
 *
 *  $Log: tclVisi.C,v $
 *  Revision 1.8  1996/08/16 21:37:40  tamches
 *  updated copyright for release 1.1
 *
 *  Revision 1.7  1996/08/05 07:14:01  tamches
 *  update for tcl 7.5
 *
 *  Revision 1.6  1995/12/20 18:37:00  newhall
 *  matherr.h does not need to be included by visis
 *
 *  Revision 1.5  1995/12/01 06:42:56  tamches
 *  removed warnings (tclclean.h; tkclean.h)
 *  included new logo code (pdLogo.h)
 *
 *  Revision 1.4  1995/11/08 21:16:56  naim
 *  Adding matherr exception handler function to avoid error message when
 *  computing the "not a number" (NaN) value - naim
 *
 * Revision 1.3  1995/07/06  01:55:46  newhall
 * update for Tcl-7.4, Tk-4.0
 *
 */

#include <stdio.h>
#include <signal.h>

#include "tcl.h"
#include "tk.h"

#include "pdLogo.h"
#include "paradyn/xbm/logo.xbm"

extern Dg_Init(Tcl_Interp *interp);

Tcl_Interp *MainInterp;

int Tcl_AppInit(Tcl_Interp *interp) {
    Tk_Window main;

    MainInterp = interp;

    main = Tk_MainWindow(interp);

//    if (Blt_Init(interp) == TCL_ERROR)
//	return TCL_ERROR;

    if (Dg_Init(interp) == TCL_ERROR)
        return TCL_ERROR;

    if (Tcl_Init(interp) == TCL_ERROR)
	return TCL_ERROR;

    if (Tk_Init(interp) == TCL_ERROR)
	return TCL_ERROR;

    // now install "makeLogo", etc:
    pdLogo::install_fixed_logo("paradynLogo", logo_bits, logo_width,
			       logo_height);
    tcl_cmd_installer createPdLogo(interp, "makeLogo", pdLogo::makeLogoCommand,
				   (ClientData)main);

    return TCL_OK;
}

int main(int argc,char **argv){
//sigpause(0);
    Tk_Main(argc,argv,Tcl_AppInit);
    return 0;
}

