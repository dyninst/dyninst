/*
 *  tclVisi.C -- This file handles the bare essentials of tcl 
 *        application initialization.  Essentially, it implements the
 *        Tcl_AppInit() function.
 *
 *  $Log: tclVisi.C,v $
 *  Revision 1.1  1994/05/31 21:05:51  rbi
 *  Initial version of tclVisi and tabVis
 *
 */
#include <stdio.h>
#include <tcl.h>
#include <tk.h>

extern Dg_Init(Tcl_Interp *interp);

extern "C" {
  int Blt_Init(Tcl_Interp *interp);
}

Tcl_Interp *MainInterp;

int
Tcl_AppInit(Tcl_Interp *interp)
{
    Tk_Window main;

    MainInterp = interp;

    main = Tk_MainWindow(interp);

    if (Blt_Init(interp) == TCL_ERROR) {
	return TCL_ERROR;
    }
    if (Dg_Init(interp) == TCL_ERROR) {
        return TCL_ERROR;
    }
    if (Tcl_Init(interp) == TCL_ERROR) {
	return TCL_ERROR;
    }
    if (Tk_Init(interp) == TCL_ERROR) {
	return TCL_ERROR;
    }

    tcl_RcFileName = "~/.wishrc";

    return TCL_OK;
}

