/*
 *  tclVisi.C -- This file handles the bare essentials of tcl 
 *        application initialization.  Essentially, it implements the
 *        Tcl_AppInit() function.
 *
 *  $Log: tclVisi.C,v $
 *  Revision 1.3  1995/07/06 01:55:46  newhall
 *  update for Tcl-7.4, Tk-4.0
 *
 * Revision 1.2  1994/11/08  00:19:41  tamches
 * commented out blt-ish influences
 *
 * Revision 1.1  1994/05/31  21:05:51  rbi
 * Initial version of tclVisi and tabVis
 *
 */
#include <stdio.h>
#include <signal.h>
#include <tcl.h>
#include <tk.h>

extern Dg_Init(Tcl_Interp *interp);

//extern "C" {
//  int Blt_Init(Tcl_Interp *interp);
//}

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

    tcl_RcFileName = "~/.wishrc";

    return TCL_OK;
}

main(int argc,char **argv){

    Tk_Main(argc,argv,Tcl_AppInit);

}

