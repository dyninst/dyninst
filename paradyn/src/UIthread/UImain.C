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

// $Id: UImain.C,v 1.114 2004/06/21 19:37:42 pcroth Exp $

/* UImain.C
 *    This is the main routine for the User Interface Manager thread, 
 *    called at thread creation.
 */

#include "common/h/headers.h"

#if !defined(i386_unknown_nt4_0)
#include <sys/param.h>
#endif // !defined(i386_unknown_nt4_0)

#include "UIglobals.h" 
#include "paradyn/src/DMthread/DMinclude.h"
#include "dataManager.thread.h"
#include "pdthread/h/thread.h"
#include "../pdMain/paradyn.h"
#include "tkTools.h"
#include "pdutil/h/TclTools.h"
#include "ParadynTkGUI.h"


extern "C" const char V_paradyn[];


//----------------------------------------------------------------------------
// variables "global" to the UI thread
//----------------------------------------------------------------------------T
ListWithKey<metricInstInfo *, metricInstanceHandle> uim_enabled;
int UIMMsgTokenID = 0;

ParadynUI* pdui = NULL;


void*
UImain( void* args )
{
    PARADYN_DEBUG(("%s\n",V_paradyn));
    thr_name("UIM");
    UIthreadArgs* uiargs = (UIthreadArgs*)args;
    assert( uiargs != NULL );

    // create the UI
#if READY
    // create actual type of GUI
#else
    pdui = new ParadynTkGUI( uiargs->mainTid, uiargs->progName );
#endif // READY
    if( pdui->Init() )
    {
        // let main thread know we're initialized and starting to handle events
        msg_send( uiargs->mainTid, MSG_TAG_UIM_READY, NULL, 0 );

        // start handling events
        pdui->DoMainLoop();
    }

    // cleanup
    delete pdui;
    pdui = NULL;

    return (void*)0;
}



