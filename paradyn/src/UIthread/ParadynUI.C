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

// $Id: ParadynUI.C,v 1.4 2004/07/14 18:24:15 eli Exp $
#include "UIglobals.h"
#include "common/h/Ident.h"
#include "../pdMain/paradyn.h"
#include "paradyn/src/TCthread/tunableConst.h"
#include "ParadynUI.h"


extern "C" const char V_paradyn[];
const Ident ParadynUI::V_id(V_paradyn,"Paradyn");
extern "C" const char V_libpdutil[];
const Ident ParadynUI::V_Uid(V_libpdutil,"Paradyn");
extern "C" const char V_libpdthread[];
const Ident ParadynUI::V_Tid(V_libpdthread,"Paradyn");



ParadynUI::ParadynUI( thread_t _mainTid, pdstring _progName )
  : UIM( _mainTid ),
    stdinIsTTY( false ),
    inDeveloperMode( false ),
    ps_handle( 0 ),
    progName( _progName )
{
    // nothing else to do
}

ParadynUI::~ParadynUI( void )
{
}


bool
ParadynUI::Init( void )
{
    bool ret = true;

    // determine if our stdin is a TTY
    // (or if we're reading stdin from a file)
#if !defined(i386_unknown_nt4_0)
    stdinIsTTY = isatty( 0 );
#else
    stdinIsTTY = _isatty( _fileno( stdin ) );
#endif // defined(i386_unknown_nt4_0)

    // our tunable constants
	tunableBooleanConstantDeclarator* tcInDeveloperMode = 
        new tunableBooleanConstantDeclarator("developerMode",
		"Allow access to all tunable constants, including those limited to developer mode.  (Use with caution)",
		false, 	// intial value
		DeveloperModeCallback,
		userConstant);

    return ret;
}



void
ParadynUI::DoMainLoop( void )
{
    while( !IsDoneHandlingEvents() )
    {
        // allow UI to do any pending work between event processing
        DoPendingWork();

        // get the next message
        thread_t mtid = THR_TID_UNSPEC;
        tag_t mtag = MSG_TAG_ANY;
        msg_poll( &mtid, &mtag, 1 );    // 1 => this is a blocking call

        // dispatch the message
        bool handled = HandleEvent( mtid, mtag );
        if( !handled )
        {
            // indicate the error
            // TODO how to indicate the error
            cerr << "failed to handle event, tid = " << mtid
                << ", tag = " << mtag 
                << endl;
            Tcl_Panic((TCLCONST char*)"ui main loop: neither dataMgr nor uim_server report isValidTag() of true", NULL);
            break;
        }
    }
}


bool
ParadynUI::HandleEvent( thread_t /* mtid */, tag_t mtag )
{
    bool ret = false;

    // check if it is a message for the UIM igen object
    if( isValidTag((T_UI::message_tags)mtag) )
    {
        // The message is a request to the UIM igen object
        if( waitLoop(true, (T_UI::message_tags)mtag) != T_UI::error )
        {
            ret = true;
        }
        else
        {
            // TODO respond better
            assert(0);
        }
    }
    else if( dataMgr->isValidTag((T_dataManager::message_tags)mtag) )
    {
        // it is an upcall from the DM thread
        if( dataMgr->waitLoop(true, (T_dataManager::message_tags)mtag) !=
                T_dataManager::error )
        {
            ret = true;
        }
        else
        {
            // TODO respond better
            assert(0);
        }
    }

    return ret;
}



void
ParadynUI::setDeveloperMode( bool newVal )
{
	inDeveloperMode = newVal;
}

void
ParadynUI::DeveloperModeCallback( bool newVal )
{
    // we make this call through the client-side interface because
    // this callback may be executed in a thread other than the UI thread
	uiMgr->setDeveloperMode( newVal );
}


void
ParadynUI::Panic( pdstring msg )
{
#if !defined(i386_unknown_nt4_0)
    fprintf( stderr, "PANIC: %s\n", msg.c_str() );
#else
	MessageBox( NULL, msg.c_str(), "Fatal", MB_ICONSTOP | MB_OK );
#endif // !defined(i386_unknown_nt4_0)
    exit( -1 );
}



void
ParadynUI::DMready( void )
{
    // create our "new resource" performance stream
    assert( ps_handle == 0 );

    controlCallback controlFuncs;
    controlFuncs.rFunc = ResourceAddedCallback;
    controlFuncs.retireFunc = ResourceRetiredCallback;
    controlFuncs.mFunc = NULL;
    controlFuncs.fFunc = NULL;
    controlFuncs.sFunc = ApplicStateChangedCallback;
    controlFuncs.bFunc = ResourceBatchChangedCallback;
    controlFuncs.pFunc = NULL;
    controlFuncs.eFunc = EnableDataResponseCallback;

    dataCallback dataFunc;
    dataFunc.sample = NULL;

    ps_handle = dataMgr->createPerformanceStream(Sample,
                                                        dataFunc,
                                                        controlFuncs);
}


void
ParadynUI::ResourceBatchChangedCallback( perfStreamHandle h, batchMode mode)
{
    pdui->ResourceBatchChanged( h, mode );
}



/* 
 *  This callback function invoked by dataManager whenever a new 
 *  resource has been defined.  Maintains where axis display.
 *  Creates dag for abstraction if none exists.
 */
void
ParadynUI::ResourceAddedCallback(perfStreamHandle h,
                                    resourceHandle parent, 
                                    resourceHandle newResource,
                                    const char *name,
                                    const char *abs)
{
    pdui->ResourceAdded( h, parent, newResource, name, abs );
}


void
ParadynUI::ResourceRetiredCallback(perfStreamHandle h,
                                    resourceHandle uniqueID, 
                                   const char* name)
{
    pdui->ResourceRetired( h, uniqueID, name);
}


// Currently this routine never executes because of a kludge
// that receives the response message from the DM before a call to this
// routine is made.  This routine must still be registered with the DM on
// createPerformanceStream, otherwise the DM will not send the response
// message.  If the UI is changed so that the call to getPredictedDataCost
// is handled in a truely asynchronous manner, then this routine should
// contain the code to handle the upcall from the DM
void
ParadynUI::EnableDataResponseCallback(pdvector<metricInstInfo>*,
                                            u_int, u_int)
{
    assert( false );
}


void
ParadynUI::ApplicStateChangedCallback( perfStreamHandle h,
                                        appState s )
{
    pdui->ApplicStateChanged( h, s );
}


