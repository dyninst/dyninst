/*
 * Copyright (c) 1996-2003 Barton P. Miller
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

// $Id: main.C,v 1.21 2003/07/18 15:45:40 schendel Exp $

#include <assert.h>
#include <stdlib.h>
#include <iostream>

#include "common/h/headers.h"
#include "pdutil/h/pdsocket.h"
#include "pdutil/h/pddesc.h"
#include "common/h/Types.h" // Address
#include "common/h/String.h" // Address
#include "visi/h/visualization.h"
#include "visiClients/auxiliary/h/NoSoloVisiMsg.h"

#include "tcl.h"
#include "tk.h"
#include "tkTools.h"

#include "pdLogo.h"
#include "paradyn/xbm/logo.xbm"

#include "dg2.h"

#include "tableVisi.h"
#include "tableVisiTcl.h"

#include "common/h/Ident.h"
extern "C" const char V_tableVisi[];
Ident V_id(V_tableVisi,"Paradyn");
extern "C" const char V_libpdutil[];
Ident V_Uid(V_libpdutil,"Paradyn");
extern "C" const char V_libvisi[];
Ident V_Vid(V_libvisi,"Paradyn");

Tcl_Interp *mainInterp;

tableVisi *theTableVisi; // our main data structure
bool xsynch_flag=false;

void visiFdReadableHandler(ClientData, int) {
   // Installed as a file-handler routine for whenever data arrives over
   // the socket returned from visi_Init()
   if (visi_callback() == -1)
      exit(0);
}

int main(int argc, char **argv) {

    bool sawParadynFlag = false;

    for( unsigned int i = 1; i < argc; i++ )
    {
        if( strcmp(argv[i], "--debug" ) == 0 )
        {
          xsynch_flag = true;
#if !defined(i386_unknown_nt4_0)
          cout << "tableVisi at sigpause...pid=" << getpid() << endl;
          sigpause(0);
#endif // !defined(i386_unknown_nt4_0)
        }
        else if( strcmp( argv[i], "--paradyn" ) == 0 )
        {
            sawParadynFlag = true;
        }
        else
        {
            Tcl_Panic( "unrecognized argument seen", NULL );
        }
    }

    if( !sawParadynFlag )
    {
        ShowNoSoloVisiMessage( argv[0] );
    }

   mainInterp = Tcl_CreateInterp();
   assert(mainInterp);

#if !defined(i386_unknown_nt4_0)
   if (xsynch_flag) {
      cout << "xsynching..." << endl;
      XSynchronize(Tk_Display(Tk_MainWindow(mainInterp)), 1);
   }
#endif // !defined(i386_unknown_nt4_0)

   if (TCL_OK != Tcl_Init(mainInterp))
      tclpanic(mainInterp, "Could not Tcl_Init");

   if (TCL_OK != Tk_Init(mainInterp))
      tclpanic(mainInterp, "Could not Tk_Init");

   if (TCL_OK != Dg2_Init(mainInterp))
      tclpanic(mainInterp, "Could not Dg2_Init");

   PDSOCKET visi_sock = visi_Init();
   if (visi_sock < 0)
      Tcl_Panic("failed to initialize w/ visi lib", NULL);

   Tcl_SetVar(mainInterp, "tcl_interactive", "0", TCL_GLOBAL_ONLY);

   // Install our new tcl commands here:
   installTableVisiCommands(mainInterp);

   if (visi_RegistrationCallback(ADDMETRICSRESOURCES,Dg2AddMetricsCallback)!=0)
      Tcl_Panic("Dg2_Init() -- couldn't install ADDMETRICSRESOURCES callback",
        NULL);

   if (visi_RegistrationCallback(DATAVALUES, Dg2NewDataCallback) != 0)
      Tcl_Panic("Dg2_Init() -- couldn't install DATAVALUES callback", NULL);

   if (visi_RegistrationCallback(PARADYNEXITED, Dg2ParadynExitedCallback) != 0)
       panic("Dg2_Init() -- couldn't install PARADYNEXITED callback");

//   if (visi_RegistrationCallback(PHASEDATA, Dg2PhaseDataCallback) != 0)
//      panic("Dg2_Init() -- couldn't install PHASEINFO callback");

	// install a handler to notify us when there is data to be read
	Tcl_Channel visi_chan = 
	  Tcl_MakeTcpClientChannel((ClientData)(Address)(PDDESC)visi_sock);
	Tcl_CreateChannelHandler(visi_chan,
				 TCL_READABLE,
				 (Tcl_FileProc*)visiFdReadableHandler,
				 0);

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
   theTableVisi = NULL;

   return 0;
}
