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

// main.C
// Ariel Tamches

/*
 * $Log: main.C,v $
 * Revision 1.11  1997/09/24 19:32:04  tamches
 * Tcl_GetFile() no longer used in tcl 8.0
 *
 * Revision 1.10  1996/08/16 21:36:55  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.9  1996/08/05 07:11:55  tamches
 * update for tcl 7.5
 *
 * Revision 1.8  1996/01/17 18:31:38  newhall
 * changes due to new visiLib
 *
 * Revision 1.7  1995/12/22 22:37:25  tamches
 * highlight background color is new
 *
 * Revision 1.6  1995/12/20 18:37:16  newhall
 * matherr.h does not need to be included by visis
 *
 * Revision 1.5  1995/11/29 00:43:33  tamches
 * paradyn logo is now hard-coded
 *
 */

#include <assert.h>
#include <stdlib.h>
#include <iostream.h>
#include "tcl.h"
#include "tk.h"
#include "tkTools.h"

#include "pdLogo.h"
#include "paradyn/xbm/logo.xbm"

#include "visi/h/visualization.h"
#include "dg2.h"

#include "tableVisi.h"
#include "tableVisiTcl.h"

Tcl_Interp *mainInterp;

void panic(const char *msg) {
   cerr << msg << endl;
   exit(5);
}

tableVisi *theTableVisi; // our main data structure
bool xsynch_flag=false;

void visiFdReadableHandler(ClientData, int) {
   // Installed as a file-handler routine for whenever data arrives over
   // the socket returned from visi_Init()
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

   if (xsynch_flag) {
      cout << "xsynching..." << endl;
      XSynchronize(Tk_Display(Tk_MainWindow(mainInterp)), 1);
   }

   if (TCL_OK != Tcl_Init(mainInterp))
      tclpanic(mainInterp, "Could not Tcl_Init");

   if (TCL_OK != Tk_Init(mainInterp))
      tclpanic(mainInterp, "Could not Tk_Init");

   if (TCL_OK != Dg2_Init(mainInterp))
      tclpanic(mainInterp, "Could not Dg2_Init");

   int fd = visi_Init();
   if (fd < 0)
      panic("failed to initialize w/ visi lib");

   Tcl_SetVar(mainInterp, "tcl_interactive", "0", TCL_GLOBAL_ONLY);

   // Install our new tcl commands here:
   installTableVisiCommands(mainInterp);

   if (visi_RegistrationCallback(ADDMETRICSRESOURCES,Dg2AddMetricsCallback)!=0)
      panic("Dg2_Init() -- couldn't install ADDMETRICSRESOURCES callback");

   if (visi_RegistrationCallback(DATAVALUES, Dg2NewDataCallback) != 0)
      panic("Dg2_Init() -- couldn't install DATAVALUES callback");

//   if (visi_RegistrationCallback(PHASEDATA, Dg2PhaseDataCallback) != 0)
//      panic("Dg2_Init() -- couldn't install PHASEINFO callback");

   // new with tcl 7.5: the Tcl_File type instead of int is passed to Tk_CreateFileHandler
   Tcl_CreateFileHandler(fd, TCL_READABLE, visiFdReadableHandler, 0);

   // Krishna's tcl2c stuff:
   extern int initialize_tcl_sources(Tcl_Interp *);
   if (TCL_OK != initialize_tcl_sources(mainInterp))
      tclpanic(mainInterp, "tableVisi: could not initialize_tcl_sources");

//if (Tcl_EvalFile(mainInterp, "/p/paradyn/development/tamches/core/visiClients/tableVisi/tcl/tableVisi.tcl") != TCL_OK)
//   tclpanic(mainInterp, "could not open tableVisi.tcl");

   pdLogo::install_fixed_logo("paradynLogo", logo_bits, logo_width, logo_height);
   tcl_cmd_installer createPdLogo(mainInterp, "makeLogo", pdLogo::makeLogoCommand,
				  (ClientData)Tk_MainWindow(mainInterp));

   myTclEval(mainInterp, "initializeTableVisi");

   // Create our main data structure:
   theTableVisi = new tableVisi(mainInterp,
				Tk_NameToWindow(mainInterp, ".body",
						Tk_MainWindow(mainInterp)),
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
				"lightGray", // highlight background color
				3 // initial # sig figs
				);
   assert(theTableVisi);

   Tk_MainLoop(); // returns when all tk windows are closed

   delete theTableVisi;
}
