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

// $Id: ParadynTkGUI.h,v 1.8 2005/03/11 00:38:09 legendre Exp $
#ifndef PARADYNTKGUI_H
#define PARADYNTKGUI_H

#include "tk.h"
#include "ParadynTclUI.h"
#include "shgPhases.h"


class ParadynTkGUI : public ParadynTclUI
{
private:
    thread_t xtid;   // tid assigned to bound X socket or Windows message queue
    bool haveSeenFirstWhereAxisWindow;
    bool haveSeenFirstCallGraphWindow;
    bool haveSeenFirstShgWindow;
    int batchModeCounter;
    appState PDapplicState;     // used to update run/pause buttons  
    bool all_metrics_set_yet;
    dictionary_hash<unsigned, pdstring> all_metric_names; // met-id (not index!) to name

    // The following two variables tell the shg which phase to try to
    // activate when _first_ opening the shg window.  We initialize it
    // to the well-known values for the "current phase" which is
    // created on startup.
    int latest_detected_new_phase_id;
    const char* latest_detected_new_phase_name;


    static void ShowWhereAxisTipsCallback( bool newVal );
    static void HideWhereAxisRetiredResCallback(bool);
    static void ShowShgKeyCallback( bool newVal );
    static void ShowShgTipsCallback( bool newVal );
    static void ShowShgTrueCallback( bool show );
    static void ShowShgFalseCallback( bool show );
    static void ShowShgUnknownCallback( bool show );
    static void ShowShgNeverCallback( bool show );
    static void ShowShgActiveCallback( bool show );
    static void ShowShgInactiveCallback( bool show );
    static void ShowShgShadowCallback( bool show );
    static void UseGUITermWinCallback( bool useGUI );

    void DoPendingTkEvents( void );
    void DisablePAUSEandRUN( void );
    void tcShgShowGeneric(shg::changeType ct, bool show);



protected:
    virtual bool IsDoneHandlingEvents( void ) const;
    virtual void DoPendingWork( void );
    virtual bool HandleEvent( thread_t mtid, tag_t mtag );

    // performance stream handlers
    virtual void ResourceBatchChanged( perfStreamHandle, batchMode mode );
    virtual void ApplicStateChanged( perfStreamHandle h, appState s );
    virtual void ResourceAdded(perfStreamHandle h,
                        resourceHandle parent, 
                        resourceHandle newResource,
                        const char *name,
                        const char *abs);

    virtual void ResourceRetired(perfStreamHandle h,
                                    resourceHandle uniqueID, 
                                 const char* name);

    virtual void ResourceUpdated(perfStreamHandle, resourceHandle, 
                     const char *, const char *);

    // igen interface implementation
	virtual void chooseMetricsandResources(chooseMandRCBFunc cb,
			                           pdvector<metric_focus_pair>* pairList);
	virtual void registerValidVisis (pdvector<VM_visiInfo> *allVisis);
	virtual void threadExiting( void );
	virtual void enablePauseOrRun( void );
	virtual void ProcessCmd(pdstring *arguments);
	virtual void allDataSaved(bool succeeded);
	virtual void resourcesSaved(bool succeeded);
    virtual void shgSaved (bool succeeded);
	virtual void updateStatusDisplay (int token, pdstring *item);
	virtual void newPhaseNotification(unsigned phaseID,
                                        const char *phname, 
					                    bool with_new_pc);
	virtual void DAGaddNode(int dagID, unsigned nodeID, int styleID, 
			const char *label, const char *shgname, int root);
	virtual void DAGaddEdge (int dagID, unsigned srcID, 
			unsigned dstID,
			int styleID, // why vs. where refinement
			const char *label // only used for shadow nodes; else NULL
			);
	virtual void DAGaddBatchOfEdges(int dagID, pdvector<uiSHGrequest> *requests,
				       unsigned numRequests);
	virtual void DAGconfigNode (int dagID, unsigned nodeID, int styleID);
	virtual void DAGinactivateEntireSearch(int dagID);
        
    virtual void requestNodeInfoCallback(unsigned phaseID, int nodeID, 
                                            shg_node_info *theInfo, bool ok);
	virtual void CGaddNode(int pid, resourceHandle parent, 
	                      resourceHandle newNode, const char *shortName, 
                              const char *fullName, bool isRecursive,
                              bool shadowNode);
	virtual void callGraphProgramAddedCB(unsigned programId,
                                            resourceHandle newId,
                                            const char *executableName,
                                            const char *shortName,  
                                            const char *fullName); 
	virtual void CGDoneAddingNodesForNow(int programId);
	virtual void CloseTkConnection( void );
	virtual void showWhereAxisTips( bool val );
	virtual void showSHGKey( bool val );
	virtual void showSHGTips( bool val );
	virtual void showSHGTrueNodes( bool val );
	virtual void showSHGFalseNodes( bool val );
	virtual void showSHGUnknownNodes( bool val );
	virtual void showSHGNeverExpandedNodes( bool val );
	virtual void showSHGActiveNodes( bool val );
	virtual void showSHGInactiveNodes( bool val );
	virtual void showSHGShadowNodes( bool val );
    virtual void setDeveloperMode( bool newVal );

    // "uimpd" Tcl command implementation
    virtual int SendVisiSelectionsCmd( int argc, TCLCONST char* argv[] );
    virtual int ProcessVisiSelectionsCmd( int argc, TCLCONST char* argv[] );
    virtual int TclTunableCmd( int argc, TCLCONST char* argv[] );
    virtual int ShowErrorCmd( int argc, TCLCONST char* argv[] );
    virtual int StartPhaseCmd( int argc, TCLCONST char* argv[] );

    // "paradyn" Tcl command implementation
    virtual int ParadynApplicationDefinedCmd( int argc, TCLCONST char* argv[] );
    virtual int ParadynAttachCmd( int argc, TCLCONST char* argv[] );
    virtual int ParadynPauseCmd( int argc, TCLCONST char* argv[] );
    virtual int ParadynContCmd( int argc, TCLCONST char* argv[] );
    virtual int ParadynStatusCmd( int argc, TCLCONST char* argv[] );
    virtual int ParadynListCmd( int argc, TCLCONST char* argv[] );
    virtual int ParadynDaemonsCmd( int argc, TCLCONST char* argv[] );
    virtual int ParadynDetachCmd( int argc, TCLCONST char* argv[] );
    virtual int ParadynDisableCmd( int argc, TCLCONST char* argv[] );
    virtual int ParadynEnableCmd( int argc, TCLCONST char* argv[] );
    virtual int ParadynGetTotalCmd( int argc, TCLCONST char* argv[] );
    virtual int ParadynMetricsCmd( int argc, TCLCONST char* argv[] );
    virtual int ParadynPrintCmd( int argc, TCLCONST char* argv[] );
    virtual int ParadynProcessCmd( int argc, TCLCONST char* argv[] );
    virtual int ParadynResourcesCmd( int argc, TCLCONST char* argv[] );
    virtual int ParadynSetCmd( int argc, TCLCONST char* argv[] );
    virtual int ParadynSaveCmd( int argc, TCLCONST char* argv[] );
    virtual int ParadynCoreCmd( int argc, TCLCONST char* argv[] );
    virtual int ParadynSuppressCmd( int argc, TCLCONST char* argv[] );
    virtual int ParadynVisiCmd( int argc, TCLCONST char* argv[] );
    virtual int ParadynWaSetAbstraction( int argc, TCLCONST char* argv[] );
    virtual int ParadynWaSelect( int argc, TCLCONST char* argv[] );
    virtual int ParadynWaUnSelect( int argc, TCLCONST char* argv[] );
    virtual int ParadynDaemonStartInfoCmd( int argc, TCLCONST char* argv[] );
    virtual int ParadynGeneralInfoCmd( int argc, TCLCONST char* argv[] );
    virtual int ParadynLicenseInfoCmd( int argc, TCLCONST char* argv[] );
    virtual int ParadynReleaseInfoCmd( int argc, TCLCONST char* argv[] );
    virtual int ParadynVersionInfoCmd( int argc, TCLCONST char* argv[] );

public:
    ParadynTkGUI( thread_t _mainTid, pdstring _progName );
    virtual ~ParadynTkGUI( void );

    bool IsInBatchMode( void ) const        { return (batchModeCounter > 0); }
    appState GetAppState( void ) const      { return PDapplicState; }

    bool TryFirstWhereAxisWindow( void );
    bool HaveSeenFirstWhereAxisWindow( void ) const
        { return haveSeenFirstWhereAxisWindow; }

    bool TryFirstCallGraphWindow( void );
    bool HaveSeenFirstCallGraphWindow( void ) const
        { return haveSeenFirstCallGraphWindow; }

    bool TryFirstShgWindow( void );
    bool HaveSeenFirstShgWindow( void ) const
        { return haveSeenFirstShgWindow; }

    virtual bool Init( void );

    // igen interface methods
    virtual void DMready( void );
};

#endif // PARADYNTKGUI_H
