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
//----------------------------------------------------------------------------
//
// main.C
//
// Main program for the rthist executable.
//
//----------------------------------------------------------------------------
// $Id: main.C,v 1.8 2003/06/27 17:59:33 pcroth Exp $
//----------------------------------------------------------------------------
#include "common/h/headers.h"

#include "tcl.h"
#include "tk.h"
#include "tkTools.h"

#include "common/h/Ident.h"

#include "visi/h/visiTypes.h"
#include "common/h/Vector.h"

#include "Histogram.h"
#include "DGClient.h"
#include "HistVisi.h"
#include "visiClients/auxiliary/h/NoSoloVisiMsg.h"



//---------------------------------------------------------------------------
// program data
//---------------------------------------------------------------------------
extern "C" const char V_rthist[];
Ident V_Pid(V_rthist,"Paradyn");
extern "C" const char V_libpdutil[];
Ident V_Uid(V_libpdutil,"Paradyn");
extern "C" const char V_libvisi[];
Ident V_Vid(V_libvisi,"Paradyn");


//---------------------------------------------------------------------------
// main program
//---------------------------------------------------------------------------

int
main( int argc, char* argv[] )
{
    int ret = 0;

    bool sawParadynFlag = false;
    for( int i = 0; i < argc; i++ )
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

    // set up Tcl and Tk
    Tcl_FindExecutable( argv[0] );
    Tcl_Interp* interp = Tcl_CreateInterp();
    assert( interp != NULL );

    if( Tcl_Init( interp ) != TCL_OK )
    {
        tclpanic( interp, "Failed to initialize Tcl." );
    }

    if( Tk_Init( interp ) != TCL_OK )
    {
        tclpanic( interp, "Failed to initialize Tk." );
    }
    Tcl_SetVar( interp, "tcl_interactive", "0", TCL_GLOBAL_ONLY );
	Tk_SetClass( Tk_MainWindow(interp), Tk_GetUid("Pdhistvisi") );

#if defined(DEBUG_PAUSE)
    Tcl_EvalObj( interp,
        Tcl_NewStringObj( "tk_messageBox -message \"Press OK to continue\" -title RTHist -type ok", -1 ));
#endif // defined(DEBUG_PAUSE)

    // set up our application object
    HistVisi* phist = new HistVisi( interp );
    if( phist->Init() )
    {
        // start handling events
        phist->Run();
    }

    // cleanup
    delete phist;
    Tcl_DeleteInterp( interp );

    return ret;
}
