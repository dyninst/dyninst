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

/*
 * UIpublic.C : exported services of the User Interface Manager thread 
 *              of Paradyn
 */
 
/* $Id: UIpublic.C,v 1.84 2003/09/05 19:14:20 pcroth Exp $
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "UI.thread.CLNT.h"
#include "UI.thread.SRVR.h"
#include "UI.thread.h"
#include "pdthread/h/thread.h"
#include "UIglobals.h"
#include "../pdMain/paradyn.h"

#include "shgPhases.h"
#include "shgTcl.h"

#include "callGraphs.h"
#include "callGraphTcl.h"


void
UIMUser::chosenMetricsandResources
          (chooseMandRCBFunc cb,
	   pdvector<metric_focus_pair> *pairList)
{
  (cb) (pairList);
}
	
// 
// Startup File
void 
UIM::readStartupFile(const char* /* script */)
{
    // should be pure virtual 
    assert( false );
}

void
UIM::showError(int /* errCode */, const char* /* errString */)
{
    assert( false );
}

void
UIM::updateStatusLine(const char* /* sl_name */, const char* /* msg */)
{
    // should be pure virtual
    assert( false );
}

void
UIM::createStatusLine(const char* /* sl_name */)
{
    // should be pure virtual
    assert( false );
}

void
UIM::createProcessStatusLine(const char* /* sl_name */)
{
    // should be pure virtual
    assert( false );
}

void
UIM::destroyStatusLine(const char* /* sl_name */)
{
    // should be pure virtual
    assert( false );
}

void UIM::enablePauseOrRun( void )
{
    // should be pure virtual 
    assert( false );
}


void
UIM::chooseMetricsandResources(chooseMandRCBFunc /* cb */,
                                pdvector<metric_focus_pair>* /* pairlist */ )
{
    // should be pure virtual
    assert( false );
}


void
UIM::threadExiting( void )
{
    // should be pure virtual
    assert( false );
}

void
UIM::newPhaseNotification(unsigned /* ph */,
                            const char* /* name */,
                            bool /* with_new_pc */)
{
    // should be pure virtual
    assert( false );
}


void
UIM::callGraphProgramAddedCB(unsigned /* programId */,
                                resourceHandle /* newId */, 
				                const char* /* executableName */, 
				                const char* /* shortName */,
                                const char* /* longName */)
{
    // should be pure virtual
    assert( false );
}

void
UIM::updateStatusDisplay (int /* dagid */, pdstring* /* info */)
{
    // should be pure virtual
    assert( false );
}


void 
UIM::DAGaddNode(int /* dagID */,
                    unsigned /* nodeID */,
                    int /* styleID */, 
                    const char* /* label */,
                    const char* /* shgname */,
                    int /* flags */)
{
    // should be pure virtual
    assert( false );
}

void
UIM::CGaddNode(int /* pid */,
                resourceHandle /* parent */,
                resourceHandle /* newNode */,
                const char* /* shortName */,
                const char* /* fullName */, 
                bool /* recursiveFlag */,
                bool /* shadowFlag */)
{
    // should be pure virtual
    assert( false );
}

void
UIM::CGDoneAddingNodesForNow(int /* pid */)
{
    // should be pure virtual
    assert( false );
}

void
UIM::DAGaddBatchOfEdges(int /* dagID */,
                        pdvector<uiSHGrequest>* /* requests */,
                        unsigned /* numRequests */)
{
    // should be pure virtual
    assert( false );
}

void 
UIM::DAGaddEdge(int /* dagID */,
                unsigned /* srcID */, 
                unsigned /* dstID */,
                int /* styleID */, 
		        const char* /* label */ )
{
    // should be pure virtual
    assert( false );
}

void 
UIM::DAGconfigNode(int /* dagID */,
                    unsigned /* nodeID */,
                    int /* styleID */ )
{
    // should be pure virtual
    assert( false );
}


void
UIM::DAGinactivateEntireSearch(int /* dagID */)
{
    // should be pure virtual
    assert( false );
}

void
UIM::requestNodeInfoCallback(unsigned /* phaseID */,
                                int /* nodeID */, 
                                shg_node_info* /* theInfo */,
                                bool /* ok */)
{
    // should be pure virtual
    assert( false );
}

void
UIM::ProcessCmd(pdstring* /* args */)
{
    // should be pure virtual
    assert( false );
}

void 
UIM::registerValidVisis (pdvector<VM_visiInfo>* /* via */)
{
    // should be pure virtual
    assert( false );
}


void 
UIM::allDataSaved(bool /* succeeded */)
{
    // should be pure virtual
    assert( false );
}

void 
UIM::resourcesSaved(bool /* succeeded */)
{
    // should be pure virtual
    assert( false );
}

void 
UIM::shgSaved (bool /* succeeded */)
{
    // should be pure virtual
    assert( false );
}

void
UIM::CloseTkConnection( void )
{
    // should be pure virtual
    assert( false );
}


void
UIM::showWhereAxisTips(bool /* newValue */)
{
    // should be pure virtual
    assert( false );
}


void
UIM::showSHGKey(bool /* newValue */)
{
    // should be pure virtual
    assert( false );
}


void
UIM::showSHGTips(bool /* newValue */)
{
    // should be pure virtual
    assert( false );
}



void
UIM::showSHGTrueNodes(bool /* show */)
{
    // should be pure virtual
    assert( false );
}



void
UIM::showSHGFalseNodes(bool /* show */)
{
    // should be pure virtual
    assert( false );
}


void
UIM::showSHGUnknownNodes(bool /* show */)
{
    // should be pure virtual
    assert( false );
}


void
UIM::showSHGNeverExpandedNodes(bool /* show */)
{
    // should be pure virtual
    assert( false );
}



void
UIM::showSHGActiveNodes(bool /* show */)
{
    // should be pure virtual
    assert( false );
}



void
UIM::showSHGInactiveNodes(bool /* show */)
{
    // should be pure virtual
    assert( false );
}



void
UIM::showSHGShadowNodes(bool /* show */)
{
    // should be pure virtual
    assert( false );
}


void
UIM::setDeveloperMode(bool /* newValue */)
{
    // should be pure virtual
    assert(false);
}


void
UIM::showTclPrompt( bool /* show */ )
{
    // should be pure virtual
    assert( false );
}

void
UIM::DMready( void )
{
    // should be pure virtual
    assert( false );
}

