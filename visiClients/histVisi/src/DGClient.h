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
//----------------------------------------------------------------------------
//
// DGClient.h
//
// Declaration of DGClient class.
// A DGClient is the implementation of a Tcl command that allows
// access to the Visi library from Tcl code.
//
//----------------------------------------------------------------------------
// $Id: DGClient.h,v 1.2 2004/03/23 01:12:48 eli Exp $
//----------------------------------------------------------------------------
#ifndef DGCLIENT_H
#define DGCLIENT_H



class DGClient
{
private:
    struct CommandInfo
    {
       const char* name;
       unsigned char nArgs;
       int (DGClient::*handler)( Tcl_Interp* interp, int argc, char* argv[] );
    };

    static  CommandInfo cmdInfo[];    // mapping of command names to handlers


    // CommandCB - Tcl command function, acting as switchboard to route
    // commands to actual handler
    static  int CommandCB( ClientData cd,
                           Tcl_Interp* interp,
                           int argc, char* argv[] );


    // handlers for DataGrid commands
    int HandleAggregate( Tcl_Interp* interp, int argc, char* argv[] );
    int HandleBinWidth( Tcl_Interp* interp, int argc, char* argv[] );
    int HandleFirstBucket( Tcl_Interp* interp, int argc, char* argv[] );
    int HandleLastBucket( Tcl_Interp* interp, int argc, char* argv[] );
    int HandleMetricName( Tcl_Interp* interp, int argc, char* argv[] );
    int HandleMetricUnits( Tcl_Interp* interp, int argc, char* argv[] );
    int HandleMetricAverageUnits( Tcl_Interp* interp, int argc, char* argv[] );
    int HandleMetricSumUnits( Tcl_Interp* interp, int argc, char* argv[] );
    int HandleNumBins( Tcl_Interp* interp, int argc, char* argv[] );
    int HandleNumMetrics( Tcl_Interp* interp, int argc, char* argv[] );
    int HandleNumResources( Tcl_Interp* interp, int argc, char* argv[] );
    int HandleResourceName( Tcl_Interp* interp, int argc, char* argv[] );
    int HandleStartStream( Tcl_Interp* interp, int argc, char* argv[] );
    int HandleStopStream( Tcl_Interp* interp, int argc, char* argv[] );
    int HandleSum( Tcl_Interp* interp, int argc, char* argv[] );
    int HandleValid( Tcl_Interp* interp, int argc, char* argv[] );
    int HandleEnabled( Tcl_Interp* interp, int argc, char* argv[] );
    int HandleValue( Tcl_Interp* interp, int argc, char* argv[] );


public:
    // Init - sets up the 'dg' command in the given Tcl interpreter.
    int Init( Tcl_Interp* interp );
};

#endif // DGCLIENT_H

