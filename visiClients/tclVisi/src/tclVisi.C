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

/*
 *  tclVisi.C -- This file handles the bare essentials of tcl application
 *     initialization.  Essentially, it implements the Tcl_AppInit() function.
 *
 *  $Id: tclVisi.C,v 1.18 2004/03/23 22:23:57 pcroth Exp $
 */

#include <stdio.h>
#include <signal.h>

#include "common/h/headers.h"

#include "tcl.h"
#include "tk.h"
#include "visiClients/auxiliary/h/NoSoloVisiMsg.h"

#include "pdLogo.h"
#include "paradyn/xbm/logo.xbm"

#include "common/h/Ident.h"
extern "C" const char V_tclVisi[];
Ident V_id(V_tclVisi,"Paradyn");
extern "C" const char V_libpdutil[];
Ident V_Uid(V_libpdutil,"Paradyn");
extern "C" const char V_libvisi[];
Ident V_Vid(V_libvisi,"Paradyn");

extern int Dg_Init(Tcl_Interp *interp);

Tcl_Interp *MainInterp;

int My_AppInit(Tcl_Interp *interp) {
    Tk_Window main;

    MainInterp = interp;

    main = Tk_MainWindow(interp);

//    if (Blt_Init(interp) == TCL_ERROR)
//	return TCL_ERROR;

    if (Dg_Init(interp) == TCL_ERROR)
        return TCL_ERROR;

    if (Tcl_Init(interp) == TCL_ERROR)
	return TCL_ERROR;

    // Set argv0 before we do any other Tk program initialization because
    // Tk takes the main window's class and instance name from argv0
    // We set it to "paradyn" instead of "termwin" so that we can 
    // set resources for all paradyn-related windows with the same root.
    Tcl_SetVar( interp,
                "argv0", 
                "paradyn",
                TCL_GLOBAL_ONLY );

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

    bool sawParadynFlag = false;
    for( unsigned int i = 1; i < argc; i++ )
    {
        if( strcmp( argv[i], "--paradyn" ) == 0 )
        {
            sawParadynFlag = true;
        }
    }

    if( !sawParadynFlag )
    {
        ShowNoSoloVisiMessage( argv[0] );
    }

//sigpause(0);
    // Note: Tk_Main calls Tcl_FindExecutable before creating the
    // interpreter.  If you build a visi that does not use Tk_Main,
    // be sure to call Tcl_FindExecutable( argv[0] ) before 
    // creating the interpreter.
    Tk_Main(argc,argv,My_AppInit);
    return 0;
}

