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

// $Id: ParadynTclUI.C,v 1.2 2004/03/20 20:44:47 pcroth Exp $
#include "tcl.h"
#include "tkTools.h"
#include "paradyn/src/pdMain/paradyn.h"
#include "paradyn/src/TCthread/tunableConst.h"
#include "pdutil/h/TclTools.h"
#include "ParadynTclUI.h"
#include "UIglobals.h"


Tcl_HashTable UIMMsgReplyTbl;


struct ParadynTclUI::CmdInfo ParadynTclUI::uimpd_Cmds[] =
{
    {"sendVisiSelections", &ParadynTclUI::SendVisiSelectionsCmd},
    {"processVisiSelection", &ParadynTclUI::ProcessVisiSelectionsCmd},
    {"tclTunable", &ParadynTclUI::TclTunableCmd},
    {"showError", &ParadynTclUI::ShowErrorCmd},
    {"startPhase", &ParadynTclUI::StartPhaseCmd},
    { NULL, NULL}
};


struct ParadynTclUI::CmdInfo ParadynTclUI::pd_Cmds[] =
{
    {"applicationDefined", &ParadynTclUI::ParadynApplicationDefinedCmd},
    {"attach", &ParadynTclUI::ParadynAttachCmd},
    {"pause", &ParadynTclUI::ParadynPauseCmd},
    {"cont", &ParadynTclUI::ParadynContCmd},
    {"status", &ParadynTclUI::ParadynStatusCmd},
    {"list", &ParadynTclUI::ParadynListCmd},
    {"daemons", &ParadynTclUI::ParadynDaemonsCmd},
    {"detach", &ParadynTclUI::ParadynDetachCmd},
    {"disable", &ParadynTclUI::ParadynDisableCmd},
    {"enable", &ParadynTclUI::ParadynEnableCmd},
    {"gettotal", &ParadynTclUI::ParadynGetTotalCmd},
    {"metrics", &ParadynTclUI::ParadynMetricsCmd},
    {"print", &ParadynTclUI::ParadynPrintCmd},
    {"process", &ParadynTclUI::ParadynProcessCmd},
    {"resources", &ParadynTclUI::ParadynResourcesCmd},
    {"set", &ParadynTclUI::ParadynSetCmd},
    {"save", &ParadynTclUI::ParadynSaveCmd},
    {"core", &ParadynTclUI::ParadynCoreCmd},
    {"suppress", &ParadynTclUI::ParadynSuppressCmd},
    {"visi", &ParadynTclUI::ParadynVisiCmd},
    {"waSetAbstraction", &ParadynTclUI::ParadynWaSetAbstraction},
    {"waSelect", &ParadynTclUI::ParadynWaSelect},
    {"waUnselect", &ParadynTclUI::ParadynWaUnSelect},
    {"daemonStartInfo", &ParadynTclUI::ParadynDaemonStartInfoCmd},
    {"generalInfo", &ParadynTclUI::ParadynGeneralInfoCmd},
    {"licenseInfo", &ParadynTclUI::ParadynLicenseInfoCmd},
    {"releaseInfo", &ParadynTclUI::ParadynReleaseInfoCmd},
    {"versionInfo", &ParadynTclUI::ParadynVersionInfoCmd},
    {NULL, NULL}
};



ParadynTclUI::ParadynTclUI( pdstring progName )
  : ParadynUI( progName ),
    maxErrorNumber( -1 ),
    stdinCmdObj( NULL ),
    stdin_tid( THR_TID_UNSPEC ),
    interp( NULL )
{
}


ParadynTclUI::~ParadynTclUI( void )
{
    Tcl_DeleteInterp( interp );
    interp = NULL;
}


bool
ParadynTclUI::Init( void )
{
    // initialize our base class
    if( !ParadynUI::Init() )
    {
        return false;
    }

    Tcl_FindExecutable( GetProgramName().c_str() );
	interp = Tcl_CreateInterp();

    // do our own initialization
    if( Tcl_Init( interp ) == TCL_ERROR )
    {
        Panic( "Tcl_Init() failed (perhaps TCL_LIBRARY is not set?)" );
    }

	Tcl_SetVar(interp, "tcl_interactive", "0", TCL_GLOBAL_ONLY);

    // set argv0 before we do any other program initialization because
    // Tk takes the main window's class and instance name from argv0
    Tcl_SetVar( interp,
                "argv0", 
                GetProgramName().c_str(),
                TCL_GLOBAL_ONLY );


    // initialize number of errors read in from error database 
    Tcl_VarEval (interp, "getNumPdErrors", (char *)NULL);
    maxErrorNumber = atoi(Tcl_GetStringResult(interp));

    tunableBooleanConstantDeclarator* tcHideTcl = 
        new tunableBooleanConstantDeclarator("tclPrompt",
        "Allow access to a command-line prompt accepting arbitrary tcl"
        " commands in the Paradyn process.",
        false, // default value
        TclPromptCallback,
        developerConstant);

    // initialize hash table for async call replies
    Tcl_InitHashTable(&UIMMsgReplyTbl, TCL_ONE_WORD_KEYS);

    // Add internal UIM command to the tcl interpreter.
    Tcl_CreateCommand(interp, "uimpd", 
                        UimpdCmd,
                        (ClientData)this,
                        NULL );

    // add Paradyn tcl command to active interpreter
    Tcl_CreateCommand(interp, "paradyn",
                        ParadynCmd,
                        (ClientData)this,
                        NULL);

    // TODO you must do this after processing the PCL input files
    // TODO does the callback take care of this anyway?
    tunableBooleanConstant showPrompt = 
        tunableConstantRegistry::findBoolTunableConstant("tclPrompt");
    if( showPrompt.getValue() )
    {
        InstallStdinHandler();
    }

    return true;
}


bool
ParadynTclUI::IsDoneHandlingEvents( void ) const
{
#if READY
#else
   return false;
#endif // READY
}

bool
ParadynTclUI::HandleEvent( thread_t mtid, tag_t mtag )
{
    bool ret = false;

#if !defined(i386_unknown_nt4_0)
    if( mtag == MSG_TAG_FILE )
    {
        // there is input on a bound file
        // we only support one bound file - stdin
        HandleStdinInput();
        ret = true;

        // let the thread library know we've handled the input
        clear_ready_file( thr_file( mtid ) );
    }
#endif // defined(i386_unknown_nt4_0)

    if( !ret )
    {
        // let our base class try to handle it
        ret = ParadynUI::HandleEvent( mtid, mtag );
    }

    return ret;
}



void
ParadynTclUI::showTclPrompt( bool show )
{
#if !defined(i386_unknown_nt4_0)
    if( show )
    {
        // install our input handler
        InstallStdinHandler();

        // bind to stdin so that the thread library responds to input
        // on the command line

        //cout << "binding to stdin:" << endl;
        msg_bind_file(fileno(stdin),
            1, // we dequeue messages ourselves - we just want notificaton
            NULL,
            NULL,
            &stdin_tid );
   }
   else
   {
      //cout << "deleting file handler:" << endl;
      UninstallStdinHandler();
   }
#else
    // TODO - handle this mechanism (or similar) under Windows NT
#endif // !defined(i386_unknown_nt4_0)
}


// InstallStdinHandler - install a channel handler for stdin to read
// commands and execute them when a complete command is recognized
//
void
ParadynTclUI::InstallStdinHandler( void )
{
    if( stdinCmdObj != NULL )
    {
        Tcl_SetStringObj( stdinCmdObj, "", 0 );
    }
    else
    {
        stdinCmdObj = Tcl_NewStringObj( "", -1 );

        // we need to hold onto this string object 
        Tcl_IncrRefCount( stdinCmdObj );
    }
    if( IsStdinTTY() )
    {
        ShowPrompt( false );
    }
}




// UninstallStdinHandler - remove the channel handler for stdin
// used on exit or in response to tclPrompt changing to false
//
void
ParadynTclUI::UninstallStdinHandler( void )
{
    Tcl_Channel ochan = Tcl_GetStdChannel( TCL_STDOUT );


    // dump something to show that the prompt is no longer valid
    Tcl_WriteChars( ochan, "<done>", -1 );
    Tcl_Flush( ochan );

    // unbind thread library from stdin
    assert( stdin_tid != THR_TID_UNSPEC );
    msg_unbind( stdin_tid );
    stdin_tid = THR_TID_UNSPEC;
}


// standard input channel handler, called when there is input on stdin
void
ParadynTclUI::HandleStdinInput( void )
{
    Tcl_Channel ichan = Tcl_GetStdChannel( TCL_STDIN );
    bool continuation = false;


    // read the available input, appending to any existing input
    Tcl_GetsObj( ichan, stdinCmdObj );

    // check if we have a complete command
    if( Tcl_GetCharLength( stdinCmdObj ) > 0 )
    {
        if( Tcl_CommandComplete( Tcl_GetStringFromObj( stdinCmdObj, NULL )) ) 
        {
            // issue the command
            int evalRes = Tcl_EvalObjEx( interp, stdinCmdObj, TCL_EVAL_DIRECT );

            // output the result
            Tcl_Channel ochan = Tcl_GetStdChannel( (evalRes == TCL_OK) ? 
                                                    TCL_STDOUT : TCL_STDERR );
            Tcl_WriteChars( ochan, Tcl_GetStringResult( interp ), -1 );
            Tcl_WriteChars( ochan, "\n", -1 );
            Tcl_Flush( Tcl_GetStdChannel( TCL_STDOUT ) );

            // clear our old command
            continuation = false;    // we had a complete command
            Tcl_SetObjLength( stdinCmdObj, 0 );
        }
        else
        {
            // we've already saved the input for later completion
            continuation = true;
        }
    }
    else if( Tcl_Eof( ichan ) )
    {
        // we've reached EOF on the stdin file - stop looking for input
        if( !IsStdinTTY() )
        {
            UninstallStdinHandler();
        }
    }

    // show prompt if needed
    if( IsStdinTTY() )
    {
        ShowPrompt( continuation );
    }
}


void
ParadynTclUI::ShowPrompt( bool continuation )
{
    Tcl_Channel ochan = Tcl_GetStdChannel( TCL_STDOUT );

    // issue an appropriate prompt
    Tcl_WriteChars( ochan, continuation ? " +> " : "pd> ", -1 );
    Tcl_Flush( ochan );
}


void
ParadynTclUI::TclPromptCallback( bool show )
{
    // we make this call through the client-side interface because
    // this callback may be executed in a thread other than the UI thread
	uiMgr->showTclPrompt( show );
}



void
ParadynTclUI::readStartupFile( const char* script )
{
    if (script != NULL)
    {
        pdstring tcommand = pdstring("source \"") + pdstring(script) + "\"";

        if( Tcl_EvalObjEx( interp,
                            Tcl_NewStringObj( tcommand.c_str(), -1 ),
                            TCL_EVAL_DIRECT ) != TCL_OK )
        {
            uiMgr->showError(24, "");
        }
    }
}

void
ParadynTclUI::showError( int errCode, const char* errString )
{
    // errString -- custom error info string to be printed
    // Note that UIthread code can make the call to the tcl
    // routine "showError" directly; no need to call us.

    pdstring tcommand = pdstring("showError ") + 
                            pdstring(errCode) + 
                            pdstring(" ");
    if (errString == NULL || errString[0] == '\0')
       tcommand += pdstring("\"\"");
    else
       tcommand += pdstring("{") + pdstring(errString) + pdstring("}");
    myTclEval(interp, tcommand);
}



int
ParadynTclUI::ParadynCmd(ClientData cd, 
                            Tcl_Interp *interp, 
                            int argc, 
                            TCLCONST char *argv[])
{
    std::ostringstream resstr;
    int i;

    if(argc < 2)
    {
        resstr << "USAGE: " << argv[0] << " <cmd>" << std::ends;
        SetInterpResult(interp, resstr);
        return TCL_ERROR;
    }

    for(i = 0; pd_Cmds[i].name; i++)
    {
        if(strcmp(pd_Cmds[i].name,argv[1]) == 0)
        {
            ParadynTclUI* ui = static_cast<ParadynTclUI*>( cd );
            return (ui->*(pd_Cmds[i].func))( argc-1, argv+1 );
        }
    }

    resstr << "unknown paradyn cmd '" << argv[1] << "'" << std::ends;
    SetInterpResult(interp, resstr);
    return TCL_ERROR;  
}


int
ParadynTclUI::UimpdCmd(ClientData cd,
                        Tcl_Interp* interp, 
                        int argc, 
                        TCLCONST char *argv[])
{
    int i;
    std::ostringstream resstr;


    if(argc < 2)
    {
        resstr << "USAGE: " << argv[0] << " <cmd>" << std::ends;
        SetInterpResult(interp, resstr);
        return TCL_ERROR;
    }

    for(i = 0; uimpd_Cmds[i].name; i++)
    {
        if(strcmp(uimpd_Cmds[i].name,argv[1]) == 0)
        {
            // TODO temporary - change in the client data
            ParadynTclUI* ui = (ParadynTclUI*)( cd );
            return (ui->*(uimpd_Cmds[i].func))( argc - 1, argv+1 );
        }
    }

    resstr << "unknown UIM cmd '" << argv[1] << "'" << std::ends;
    SetInterpResult(interp, resstr);
    return TCL_ERROR;  
}


void
ParadynTclUI::updateStatusLine(const char* sl_name, const char *msg)
{
    status_line *status = status_line::find(sl_name);
    assert(status);
    
    status->message(msg);
}

void
ParadynTclUI::createStatusLine(const char* sl_name)
{
    new status_line(sl_name);
}

void
ParadynTclUI::createProcessStatusLine(const char* sl_name)
{
    new status_line(sl_name, status_line::PROCESS);
}

void
ParadynTclUI::destroyStatusLine(const char* sl_name)
{
    status_line *status = status_line::find(sl_name);
    assert(status);

    delete status;
}

