/*
 * Copyright (c) 1996-1999 Barton P. Miller
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
// DGClient.C
//
// Definition of DGClient class.
// A DGClient is the implementation of a Tcl command that allows
// access to the Visi library from Tcl code.
//
//----------------------------------------------------------------------------
// $Id: DGClient.C,v 1.7 2002/04/09 18:06:04 mjbrim Exp $
//----------------------------------------------------------------------------
#include <iostream.h>

#include "common/h/headers.h"

#include "tcl.h"
#include "tk.h"

#include "DGClient.h"
#include "pdutil/h/pdsocket.h"
#include "visi/h/visualization.h"
#include "pdutil/h/TclTools.h"

struct DGClient::CommandInfo DGClient::cmdInfo[] =
{
    {"aggregate",         2,  &DGClient::HandleAggregate},
    {"binwidth",          0,  &DGClient::HandleBinWidth},
    {"firstbucket",       2,  &DGClient::HandleFirstBucket},
    {"lastbucket",        2,  &DGClient::HandleLastBucket},
    {"metricname",        1,  &DGClient::HandleMetricName},
    {"metricunits",       1,  &DGClient::HandleMetricUnits},
    {"metricaveunits",    1,  &DGClient::HandleMetricAverageUnits},
    {"metricsumunits",    1,  &DGClient::HandleMetricSumUnits},
    {"numbins",           0,  &DGClient::HandleNumBins},
    {"nummetrics",        0,  &DGClient::HandleNumMetrics},
    {"numresources",      0,  &DGClient::HandleNumResources},
    {"resourcename",      1,  &DGClient::HandleResourceName},
    {"start",             2,  &DGClient::HandleStartStream},
    {"stop",              2,  &DGClient::HandleStopStream},
    {"sum",               2,  &DGClient::HandleSum},
    {"valid",             2,  &DGClient::HandleValid},
    {"enabled",           2,  &DGClient::HandleEnabled},
    {"value",             3,  &DGClient::HandleValue},
    {NULL,                0,  NULL}
};



// Init - sets up the 'dg' Tcl command
int
DGClient::Init( Tcl_Interp* interp )
{
    int ret = TCL_OK;

    // provide access to the visi library from Tcl code
    if( Tcl_CreateCommand( interp,
                            "dg",
                            (Tcl_CmdProc*)CommandCB,
                            (ClientData)this,
                            NULL ) == NULL )
    {
        ret = TCL_ERROR;
    }

    return ret;
}




// CommandCB - Tcl command function, acting as switchboard to route
// commands to actual handler
int
DGClient::CommandCB( ClientData cd,
                        Tcl_Interp* interp,
                        int argc, char* argv[] )
{
    int ret = TCL_OK;


    // skip the 'dg' argument
    argc--;
    argv++;

    if( argc == 0 )
    {
        ostrstream estr;
        estr << "USAGE: dg <option> [args...]\n" << ends;
        SetInterpResult( interp, estr );
        return TCL_ERROR;
    }

    CommandInfo* ci = cmdInfo;
    DGClient* pclient = (DGClient*)cd;
    assert( pclient != NULL );

    while( ci->name != NULL )
    {
        if( !strcmp( ci->name, argv[0] ) )
        {
            // we found the called command
            // did we get the correct number of arguments?
            if( ci->nArgs == (argc - 1) )
            {
                // we got the correct number of arguments
                // make the call through the method function pointer
                ret = (pclient->*(ci->handler))( interp, argc, argv );
            }
            else
            {
                // we didn't get the expected number of arguments
                ostrstream estr;
                estr << argv[0]
                    << ": wrong number of args (" << argc - 1
                    << ").  Should be " << ci->nArgs << '\n'
                    << ends;
                SetInterpResult( interp, estr );

                ret = TCL_ERROR;
            }
            break;
        }

        ci++;
    }

    if( (ret == TCL_OK) && (ci->name == NULL) )
    {
        // we didn't find the command to execute
        ostrstream estr;

        estr << "unknown option (" << argv[0] << ")\n" << ends;
        SetInterpResult( interp, estr );

        ret = TCL_ERROR;
    }

    return ret;
}




//----------------------------------------------------------------------------
// Handle*
// Handlers for dg commands
//----------------------------------------------------------------------------

int
DGClient::HandleAggregate( Tcl_Interp* interp, int /* argc */, char* argv[] )
{
    ostrstream ostr;

    int m = atoi(argv[2]);
    int r = atoi(argv[3]);

    ostr << visi_AverageValue( m, r ) << ends;
    SetInterpResult( interp, ostr );

    return TCL_OK;
}

int
DGClient::HandleBinWidth( Tcl_Interp* interp, int /* argc */, char** /* argv */ )
{
    ostrstream ostr;

    ostr << visi_BucketWidth() << ends;
    SetInterpResult( interp, ostr );

    return TCL_OK;
}


int
DGClient::HandleFirstBucket( Tcl_Interp* interp, int /* argc */, char* argv[] )
{
    ostrstream ostr;
    int m = atoi(argv[2]);
    int r = atoi(argv[3]);

    ostr << visi_FirstValidBucket( m, r ) << ends;
    SetInterpResult( interp, ostr );

    return TCL_OK;
}

int
DGClient::HandleLastBucket( Tcl_Interp* interp, int /* argc */, char* argv[] )
{
    ostrstream ostr;

    int m = atoi(argv[2]);
    int r = atoi(argv[3]);

    ostr << visi_LastBucketFilled( m, r ) << ends;
    SetInterpResult( interp, ostr );

    return TCL_OK;
}


int
DGClient::HandleMetricName( Tcl_Interp* interp, int /* argc */, char* argv[] )
{
    ostrstream ostr;

    int m = atoi(argv[2]);

    ostr << visi_MetricName( m ) << ends;
    SetInterpResult( interp, ostr );

    return TCL_OK;
}


int
DGClient::HandleMetricUnits( Tcl_Interp* interp, int /* argc */, char* argv[] )
{
    ostrstream ostr;

    int m = atoi(argv[2]);

    ostr << visi_MetricLabel( m ) << ends;
    SetInterpResult( interp, ostr );

    return TCL_OK;
}


int
DGClient::HandleMetricAverageUnits( Tcl_Interp* interp, int /* argc */, char* argv[] )
{
    ostrstream ostr;

    int m = atoi(argv[2]);

    ostr << visi_MetricAveLabel( m ) << ends;
    SetInterpResult( interp, ostr );

    return TCL_OK;
}


int
DGClient::HandleMetricSumUnits( Tcl_Interp* interp, int /* argc */, char* argv[] )
{
    ostrstream ostr;

    int m = atoi(argv[2]);

    ostr << visi_MetricSumLabel(m) << ends;
    SetInterpResult( interp, ostr );

    return TCL_OK;
}


int
DGClient::HandleNumBins( Tcl_Interp* interp, int /* argc */, char** /* argv */ )
{
    ostrstream ostr;

    ostr << visi_NumBuckets() << ends;
    SetInterpResult( interp, ostr );

    return TCL_OK;
}


int
DGClient::HandleNumMetrics( Tcl_Interp* interp, int /* argc */, char** /* argv */ )
{
    ostrstream ostr;

    ostr << visi_NumMetrics() << ends;
    SetInterpResult( interp, ostr );

    return TCL_OK;
}


int
DGClient::HandleNumResources( Tcl_Interp* interp, int /* argc */, char** /* argv */ )
{
    ostrstream ostr;

    ostr << visi_NumResources() << ends;
    SetInterpResult( interp, ostr );

    return TCL_OK;
}



int
DGClient::HandleResourceName( Tcl_Interp* interp, int /* argc */, char** argv )
{
    ostrstream ostr;

    int r = atoi(argv[2]);

    ostr << visi_ResourceName( r ) << ends;
    SetInterpResult( interp, ostr );

    return TCL_OK;
}



int
DGClient::HandleStartStream( Tcl_Interp* /* interp */, int /* argc */, char* argv[] )
{
    visi_GetMetsRes(argv[2],0); 
    return TCL_OK;
}


int
DGClient::HandleStopStream( Tcl_Interp* /* interp */, int /* argc */, char* argv[] )
{
    int m = atoi(argv[2]);
    int r = atoi(argv[3]);
    visi_StopMetRes(m, r);
    return TCL_OK;
}


int
DGClient::HandleSum( Tcl_Interp* interp, int /* argc */, char* argv[] )
{
    ostrstream ostr;

    int m = atoi(argv[2]);
    int r = atoi(argv[3]);
    ostr << visi_SumValue( m, r ) << ends;
    SetInterpResult( interp, ostr );

    return TCL_OK;
}


int
DGClient::HandleValid( Tcl_Interp* interp, int /* argc */, char* argv[] )
{
    ostrstream ostr;

    int m = atoi(argv[2]);
    int r = atoi(argv[3]);

    ostr << visi_Valid( m, r ) << ends;
    SetInterpResult( interp, ostr );

    return TCL_OK;
}

int
DGClient::HandleEnabled( Tcl_Interp* interp, int /* argc */, char* argv[] )
{
    ostrstream ostr;

    int m = atoi(argv[2]);
    int r = atoi(argv[3]);

    ostr << visi_Enabled(m, r) << ends;
    SetInterpResult( interp, ostr );

    return TCL_OK;
}


int
DGClient::HandleValue( Tcl_Interp* interp, int /* argc */, char* argv[] )
{
    ostrstream ostr;

    int m = atoi(argv[2]);
    int r = atoi(argv[3]);
    int buck = atoi(argv[4]);

    ostr << visi_DataValue( m, r, buck ) << ends;
    SetInterpResult( interp, ostr );

    return TCL_OK;
}

