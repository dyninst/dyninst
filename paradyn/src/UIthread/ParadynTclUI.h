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

// $Id: ParadynTclUI.h,v 1.1 2003/09/05 19:14:20 pcroth Exp $
#ifndef PARADYNTCLUI_H
#define PARADYNTCLUI_H

#include "ParadynUI.h"


class ParadynTclUI : public ParadynUI
{
private:
    struct CmdInfo
    {
        const char* name;
        int (ParadynTclUI::*func)( int, TCLCONST char* argv[] );
    };

    static struct CmdInfo pd_Cmds[];
    static struct CmdInfo uimpd_Cmds[];

    int maxErrorNumber;
    Tcl_Obj* stdinCmdObj;
    thread_t stdin_tid;

    static void TclPromptCallback( bool newVal );

    // Tcl commands
    static int UimpdCmd(ClientData clientData, 
                Tcl_Interp *interp, 
                int argc, 
                TCLCONST char *argv[]);
    static int ParadynCmd(ClientData clientData, 
                  Tcl_Interp *interp, 
                  int argc, 
                  TCLCONST char *argv[]);

    void InstallStdinHandler( void );
    void UninstallStdinHandler( void );
    void ShowPrompt( bool continuation );
    void HandleStdinInput( void );

protected:
    Tcl_Interp* interp;

    virtual bool IsDoneHandlingEvents( void ) const;
    virtual bool HandleEvent( thread_t mtid, tag_t mtag );

    virtual void showTclPrompt( bool newVal );
    virtual void readStartupFile( const char* script );
	virtual void showError(int errCode, const char *errString);
	virtual void updateStatusLine(const char* sl_name, const char *msg);
	virtual void createStatusLine(const char* sl_name);
	virtual void createProcessStatusLine(const char* sl_name);
	virtual void destroyStatusLine(const char* sl_name);

    // "uimpd" Tcl command implementation
    virtual int SendVisiSelectionsCmd( int argc, TCLCONST char* argv[] ) = NULL;
    virtual int ProcessVisiSelectionsCmd( int argc, TCLCONST char* argv[] ) = NULL;
    virtual int TclTunableCmd( int argc, TCLCONST char* argv[] ) = NULL;
    virtual int ShowErrorCmd( int argc, TCLCONST char* argv[] ) = NULL;
    virtual int StartPhaseCmd( int argc, TCLCONST char* argv[] ) = NULL;

    // "paradyn" Tcl command implementation
    virtual int ParadynApplicationDefinedCmd( int argc, TCLCONST char* argv[] ) = NULL;
    virtual int ParadynAttachCmd( int argc, TCLCONST char* argv[] ) = NULL;
    virtual int ParadynPauseCmd( int argc, TCLCONST char* argv[] ) = NULL;
    virtual int ParadynContCmd( int argc, TCLCONST char* argv[] ) = NULL;
    virtual int ParadynStatusCmd( int argc, TCLCONST char* argv[] ) = NULL;
    virtual int ParadynListCmd( int argc, TCLCONST char* argv[] ) = NULL;
    virtual int ParadynDaemonsCmd( int argc, TCLCONST char* argv[] ) = NULL;
    virtual int ParadynDetachCmd( int argc, TCLCONST char* argv[] ) = NULL;
    virtual int ParadynDisableCmd( int argc, TCLCONST char* argv[] ) = NULL;
    virtual int ParadynEnableCmd( int argc, TCLCONST char* argv[] ) = NULL;
    virtual int ParadynGetTotalCmd( int argc, TCLCONST char* argv[] ) = NULL;
    virtual int ParadynMetricsCmd( int argc, TCLCONST char* argv[] ) = NULL;
    virtual int ParadynPrintCmd( int argc, TCLCONST char* argv[] ) = NULL;
    virtual int ParadynProcessCmd( int argc, TCLCONST char* argv[] ) = NULL;
    virtual int ParadynResourcesCmd( int argc, TCLCONST char* argv[] ) = NULL;
    virtual int ParadynSetCmd( int argc, TCLCONST char* argv[] ) = NULL;
    virtual int ParadynSaveCmd( int argc, TCLCONST char* argv[] ) = NULL;
    virtual int ParadynCoreCmd( int argc, TCLCONST char* argv[] ) = NULL;
    virtual int ParadynSuppressCmd( int argc, TCLCONST char* argv[] ) = NULL;
    virtual int ParadynVisiCmd( int argc, TCLCONST char* argv[] ) = NULL;
    virtual int ParadynWaSetAbstraction( int argc, TCLCONST char* argv[] ) = NULL;
    virtual int ParadynWaSelect( int argc, TCLCONST char* argv[] ) = NULL;
    virtual int ParadynWaUnSelect( int argc, TCLCONST char* argv[] ) = NULL;
    virtual int ParadynDaemonStartInfoCmd( int argc, TCLCONST char* argv[] ) = NULL;
    virtual int ParadynGeneralInfoCmd( int argc, TCLCONST char* argv[] ) = NULL;
    virtual int ParadynLicenseInfoCmd( int argc, TCLCONST char* argv[] ) = NULL;
    virtual int ParadynReleaseInfoCmd( int argc, TCLCONST char* argv[] ) = NULL;
    virtual int ParadynVersionInfoCmd( int argc, TCLCONST char* argv[] ) = NULL;

public:
    ParadynTclUI( pdstring progName );
    virtual ~ParadynTclUI( void );

    virtual bool Init( void );
};

#endif // PARADYNTCLUI_H
