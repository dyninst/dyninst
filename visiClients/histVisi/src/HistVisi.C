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
//---------------------------------------------------------------------------
// 
// HistVisi.C
//
// Definition of HistVisi class.
// A HistVisi represents the Paradyn histogram visi.
//
//---------------------------------------------------------------------------
// $Id: HistVisi.C,v 1.6 2001/08/23 14:44:51 schendel Exp $
//---------------------------------------------------------------------------
#include <limits.h>
#include "common/h/headers.h"
#include "pdutilOld/h/pdsocket.h"
#include "pdutilOld/h/pddesc.h"

#include "tcl.h"
#include "tk.h"
#include "tkTools.h"

#include "common/h/Dictionary.h"
#include "Histogram.h"
#include "PDGraph.h"
#include "DGClient.h"

#include "visi/h/visiTypes.h"
#include "visi/h/visualization.h"
#include "common/h/Vector.h"
#include "HistVisi.h"

#include "pdLogo.h"
#include "paradyn/xbm/logo.xbm"



//---------------------------------------------------------------------------
// prototypes of functions used in this file
//---------------------------------------------------------------------------
int    initialize_tcl_sources( Tcl_Interp* interp );


//---------------------------------------------------------------------------
// class data
//---------------------------------------------------------------------------

HistVisi::VisiLibEvent HistVisi::libEventInfo[] =
{
    { ADDMETRICSRESOURCES,    AddMetricsAndResourcesCB },
    { DATAVALUES,             DataCB },
    { FOLD,                   FoldCB },
    { PHASESTART,             PhaseStartCB },
    { PHASEEND,               PhaseEndCB },
    { PHASEDATA,              PhaseDataCB },
    { PARADYNEXITED,          ParadynExitedCB },
    { (visi_msgTag)0,         NULL }
};


HistVisi* HistVisi::histVisi = NULL;    // singleton HistVisi object
const unsigned int    HistVisi::maxCurves = 10;



//---------------------------------------------------------------------------
// methods implementation
//---------------------------------------------------------------------------

HistVisi::HistVisi( Tcl_Interp* interp_ )
  : interp( interp_ ),
    visiLibInitialized( false ),
    graph( NULL ),
    dg( new DGClient )
{
    histVisi = this;
}


HistVisi::~HistVisi( void )
{
    if( visiLibInitialized )
    {
        visi_QuitVisi();
    }

    delete dg;
}



// Init - initializes the HistVisi object.
//
bool
HistVisi::Init( void )
{
    assert( interp != NULL );

    bool ret = false;
    if( InitUI() &&
        InitParadynConnection() )
    {
        ret = true;
    }

    return ret;
}




// InitUI - handles initialization of the HistVisi user interface.
//
bool
HistVisi::InitUI( void )
{
    // load the custom histogram widget
    Tcl_StaticPackage( NULL, "Pdhist", Pdhist_Init, NULL );
    if( Tcl_EvalObj(interp, Tcl_NewStringObj( "load {} Pdhist", -1)) != TCL_OK)
    {
        ostrstream msgstr;
        ostrstream cmdstr;

        msgstr << "Failed to initialize Tcl user interface extensions:\n"
                << Tcl_GetStringResult( interp )
                << ends;

        cmdstr << "tk_messageBox -icon error -message {"
                << msgstr.str()
                << "} -title Error -type ok"
                << ends;
        Tcl_EvalObj( interp, Tcl_NewStringObj( cmdstr.str(), -1 ) );

        msgstr.rdbuf()->freeze( 0 );
        cmdstr.rdbuf()->freeze( 0 );

        return false;
    }

    // set up for use of the Paradyn logo
    pdLogo::install_fixed_logo("paradynLogo",
                                logo_bits,
                                logo_width, logo_height);
    tcl_cmd_installer logoCmd(interp, "makeLogo",
                                pdLogo::makeLogoCommand,
                                (ClientData)Tk_MainWindow(interp));

    // define our user interface
    if( (initialize_tcl_sources( interp ) != TCL_OK) ||
        (Tcl_EvalObj(interp, Tcl_NewStringObj("::RTHist::init", -1)) != TCL_OK))
    {
        ostrstream msgstr;
        ostrstream cmdstr;

        msgstr << "Failed to initialize the Histogram's user interface:\n"
                << Tcl_GetStringResult( interp )
                << ends;

        cmdstr << "tk_messageBox -icon error -message {"
                << msgstr.str()
                << "} -title Error -type ok"
                << ends;
        Tcl_EvalObj( interp, Tcl_NewStringObj( cmdstr.str(), -1 ));

        cmdstr.rdbuf()->freeze( 0 );
        msgstr.rdbuf()->freeze( 0 );

        return false;
    }

	Tk_SetClass( Tk_MainWindow(interp), Tk_GetUid( "Pdhistvisi" ) );
    
    // set up any Tcl commands that support the UI
    Tcl_Command cmd = Tcl_CreateObjCommand( interp,
        "rthist_handle_remove_internal",
        HandleRemoveCB,
        (ClientData)this,
        NULL );
    if( cmd == NULL )
    {
        ostrstream msgstr;
        ostrstream cmdstr;

        msgstr << "Failed to initialize the Histogram's user interface:\n"
                << Tcl_GetStringResult( interp )
                << ends;

        cmdstr << "tk_messageBox -icon error -message {"
                << msgstr.str()
                << "} -title Error -type ok"
                << ends;
        Tcl_EvalObj( interp, Tcl_NewStringObj( cmdstr.str(), -1 ));

        cmdstr.rdbuf()->freeze( 0 );
        msgstr.rdbuf()->freeze( 0 );

        return false;
    }

    // obtain access to our graph object
    ClientData cd;
    bool found = PDGraph::FindInstanceData( ".hist.graph", interp, cd );
    assert( found  );
    graph = (PDGraph*)cd;

    return true;
}




// InitParadynConnection - handles initialization of the data connection
// to the Paradyn front end.
//
bool
HistVisi::InitParadynConnection( void )
{
    // establish connection with Paradyn
    PDSOCKET sock = visi_Init();
    if( sock == PDSOCKET_ERROR )
    {
        return false;
    }
    visiLibInitialized = true;

    // set up callbacks for visi lib events
    unsigned i = 0;
    while( libEventInfo[i].cb != NULL )
    {
        if( visi_RegistrationCallback( libEventInfo[i].evt,
                                       libEventInfo[i].cb ) != 0 )
        {
            return false;
        }
        i++;
    }

    // initialize the DGClient interface, so that our Tcl scripts
    // have access to the visi library
    dg->Init( interp );

    // set up callback for handling available data on visi socket
    Tcl_Channel chan = Tcl_MakeTcpClientChannel( (ClientData)sock );
    Tcl_CreateChannelHandler( chan,
                                TCL_READABLE,
                                (Tcl_FileProc*)HandleParadynInput,
                                0);
    return true;
}


//
// HandleParadynInput
//
// Callback for handling data from Paradyn on visi channel
//
void
HistVisi::HandleParadynInput( void*, int*, unsigned long* )
{
    if( visi_callback() == -1 )
    {
        // TODO - how to indicate to user?
        exit( 0 );
    }
}



// UpdatePhaseName - updates the phase name if necessary
void
HistVisi::UpdatePhaseName( void )
{
	static bool done = false;

	if( !done )
	{
		// update our phase label
		int pHandle = visi_GetMyPhaseHandle();
		if( pHandle >= -1 )
		{
			const char* pname = visi_GetMyPhaseName();
			if( pname != NULL )
			{
				ostrstream msgstr;
				ostrstream cmdstr;

				cmdstr << "::RTHist::update_phase_name " << pname << ends;
				Tcl_EvalObj( interp, Tcl_NewStringObj( cmdstr.str(), -1 ));

				cmdstr.rdbuf()->freeze( 0 );

				done = true;
			}
		}
	}
}



// AddMetricResourcePair - activates an association between the
// given metric and resource (as represented by their visi library indices)
//
bool
HistVisi::AddMetricResourcePair(int midx, int ridx)
{
    bool added = false;


	UpdatePhaseName();


    assert( visi_Enabled(midx, ridx) );
    assert( visi_GetUserData(midx, ridx) == NULL );

    // update the characteristics of the histogram
    graph->UpdateHistogramInfo( visi_GetStartTime(),
        visi_NumBuckets(),
        visi_BucketWidth() );

    // build an object to represent the curve
    MetricResourcePair* pair = new MetricResourcePair(midx, ridx);

    // add a curve representing the pair to our graph
    assert( graph != NULL );
    pair->curveId = graph->AddCurve( pair->mname.string_of(),
                                visi_MetricLabel( midx ),
                                pair->rname.string_of() );

    if( pair->curveId>=0)
    {
        // associate the object with this metric/resource pair
        visi_SetUserData(midx, ridx, pair);
        mrpairs += pair;
	
	int firstBucket = visi_FirstValidBucket(midx, ridx);
	int lastBucket = visi_LastBucketFilled( midx, ridx );       
	int nSamples = lastBucket - firstBucket + 1;
	if(! (firstBucket >= 0 && visi_NumBuckets()>0))  nSamples = 0;
	float *data = NULL;
	if(nSamples > 0) {
	  data = new float[visi_NumBuckets() + 1];
	  int resI = visi_DataValues(midx, ridx, data, firstBucket,lastBucket);
	  bool result = (resI == 1 ? true : false);
	  if(result==false)  {  delete data;  data=NULL; }
	}

	if(nSamples > 0 && data!=NULL) {
	  graph->SetCurveData( pair->curveId, firstBucket, nSamples, data );
	  pair->lastFilledBucket = lastBucket;
	}
	delete data;
    }
    else
    {
        // TODO - how to indicate failure to add pair to graph?
        delete pair;
    }
    return added;
}



// RemoveCurve, HandleRemoveCurveCB - implementation of a Tcl
// command, to allow us to respond in C++ code when a curve is
// removed using the user interface
//
int
HistVisi::RemoveCurve( Tcl_Interp* interp, int objc, Tcl_Obj* CONST objv[] )
{
    int ret = TCL_OK;


    if( objc == 2 )
    {
        long curveId;

        // extract the curve Id from the argument
        ret = Tcl_ExprLongObj( interp, objv[1], &curveId );
        if( ret == TCL_OK )
        {
            // find the associated metric resource pair
            for( unsigned int i = 0; i < mrpairs.size(); i++ )
            {
                if( mrpairs[i]->curveId == curveId )
                {
                    // we found the indicated curve,
                    // so get rid of it -
                    // shift elements over the now-removed item
                    MetricResourcePair* pair = mrpairs[i];
                    for( unsigned int j = i + 1; j < mrpairs.size(); j++ )
                    {
                        mrpairs[j-1] = mrpairs[j];
                    }
                    mrpairs.resize( mrpairs.size() - 1 );
                    
                    // tell the visi library to stop sending data
                    // for the pair
                    visi_SetUserData( pair->midx, pair->ridx, NULL);
                    visi_StopMetRes( pair->midx, pair->ridx );

                    // release the object
                    delete pair;

                    break;
               }
            }
        }
    }
    else
    {
        ostrstream rstr;

        // indicate the error
        rstr << "wrong # rthist_handle_remove_internal args: should be '"
            << objv[0]
            << " <curveId>'"
            << ends;

        Tcl_SetObjResult( interp, Tcl_NewStringObj( rstr.str(), -1 ) );
        rstr.rdbuf()->freeze( 0 );

        ret = TCL_ERROR;
    }

    return ret;
}


int
HistVisi::HandleRemoveCB( ClientData cd, Tcl_Interp* interp, int objc, Tcl_Obj* CONST objv[] )
{
    return ((HistVisi*)cd)->RemoveCurve( interp, objc, objv );
}



//---------------------------------------------------------------------------
// Callbacks for Visi library events
//---------------------------------------------------------------------------

//
// AddMetricsAndResourcesCB
// DataCB
// FoldCB
// PhaseStartCB
// PhaseEndCB
// PhaseDataCB
// ParadynExitedCB
//
// Turn around and call the object method to do the real work.
//
int
HistVisi::AddMetricsAndResourcesCB( int lastBucket )
{
    assert( histVisi != NULL );
    return histVisi->HandleAddMetricsAndResources( lastBucket );
}

int
HistVisi::DataCB( int lastBucket )
{
    assert( histVisi != NULL );
    return histVisi->HandleData( lastBucket, false );
}

int
HistVisi::FoldCB( int lastBucket )
{
    assert( histVisi != NULL );
    return histVisi->HandleData( lastBucket, true );
}

int
HistVisi::PhaseStartCB( int lastBucket )
{
    assert( histVisi != NULL );
    return histVisi->HandlePhaseStart( lastBucket );
}

int
HistVisi::PhaseEndCB( int lastBucket )
{
    assert( histVisi != NULL );
    return histVisi->HandlePhaseEnd( lastBucket );
}

int
HistVisi::PhaseDataCB( int lastBucket )
{
    assert( histVisi != NULL );
    return histVisi->HandlePhaseData( lastBucket );
}

int
HistVisi::ParadynExitedCB( int lastBucket )
{
    assert( histVisi != NULL );
    return histVisi->HandleParadynTermination( lastBucket );
}


//
// HandleAddMetricsAndResources
//
// Takes care of adding new resources to the visi's notion of the
// current resource set
//
int
HistVisi::HandleAddMetricsAndResources( int /* lastBucket */ )
{
    int nMetrics = visi_NumMetrics();
    int nResources = visi_NumResources();
    int m, r;
    string skippedPairs;
    unsigned int nSkipped = 0;


    // discover newly-added metric-resource pairs
    for( m = 0; m < nMetrics; m++ )
    {
        for( r = 0; r < nResources; r++ )
        {
            // determine if this is a newly-enabled pair
            if( visi_Enabled(m, r) && (visi_GetUserData(m, r) == NULL) )
            {
                if( mrpairs.size() < maxCurves )
                {
                    // add a new curve for the metric/resource
                    if( AddMetricResourcePair(m, r) )
                    {
                        assert( visi_GetUserData(m, r) != NULL );
                    }
                }
                else
                {
                    // we have no more room...

                    // ...stop data for this metric/resource...
                    visi_SetUserData(m, r, NULL);
                    visi_StopMetRes(m, r);

                    // ...add its name to our list of skipped metric/resource pairs
                    skippedPairs += visi_MetricName(m);
                    skippedPairs += "<";
                    skippedPairs += visi_ResourceName(r);
                    skippedPairs += ">  ";
                }
            }
        }
    }

    // indicate any curves that had to be skipped
    if( nSkipped > 0 )
    {
        ostrstream ostr;

        ostr << "Maximum number of curves has been exceeded.  Only "
             << maxCurves
             << " out of "
             << maxCurves + nSkipped
             << " can be displayed at the same time.  The following metrics are not begin displayed: "
             << skippedPairs
             << ends;

        visi_showErrorVisiCallback( ostr.str() );
        ostr.rdbuf()->freeze( 0 );
    }

    return 0;
}


//
// HandleData
//
// Takes care of incoming performance data
//
int
HistVisi::HandleData( int /* lastBucket */, bool isFold )
{
    int nMetrics = visi_NumMetrics();
    int nResources = visi_NumResources();
    int m, r;


	// update phase name 
	UpdatePhaseName();

    // update histogram characteristics
    graph->UpdateHistogramInfo( visi_GetStartTime(),
        visi_NumBuckets(),
        visi_BucketWidth() );

    for( m = 0; m < nMetrics; m++ ) {
        for( r = 0; r < nResources; r++ ) {
            if( !visi_Valid( m, r ) ) {
                // invalid metric-resource pair - skip to next pair
                continue;
            }

            // access the histogram data for this pair
            MetricResourcePair* pair = 
	            (MetricResourcePair*)visi_GetUserData( m, r );
      
            if( pair == NULL ) {
                // this curve was previously deleted and now has new data -
                // re-add the curve and try again
                if( AddMetricResourcePair( m, r ) ) {
                    pair = (MetricResourcePair*)visi_GetUserData( m, r );
                    assert( pair != NULL );
                }
		else continue;
            }

            // obtain the curve data
	    int firstBucket, lastBucket;
	    if(isFold) {
	      firstBucket = 0;
	      lastBucket  = visi_NumBuckets() - 1;
	    } else if(pair->lastFilledBucket == -1) {  // no buckets filled yet
	      lastBucket = visi_LastBucketFilled(m, r);
	      firstBucket = visi_FirstValidBucket(m, r);
	    } else {
	      lastBucket = visi_LastBucketFilled(m, r);
	      firstBucket = pair->lastFilledBucket + 1;
	    }
	    int nSamples = lastBucket - firstBucket + 1;
	    float *data = new float[visi_NumBuckets() + 1];

	    int resI = visi_DataValues( m, r, data, firstBucket, lastBucket);
	    bool result = (resI == 1 ? true : false);
	    if(result == false)  nSamples = 0;  // that is don't draw anything

            // update the display with the new data
	    if(nSamples>0) {
	      //cerr << "calling SetCurveData- metric: " << m 
	      //   << ", firstBucket: " << firstBucket << ", nSamples: " 
	      //   << nSamples << ", data[]: " << data[firstBucket] << "\n";
	      graph->SetCurveData( pair->curveId, firstBucket, 
				   nSamples, &(data[firstBucket]));
	      pair->lastFilledBucket = visi_LastBucketFilled(m,r);
            }
	    delete data;
        }
    }

    return 0;
}



//
// HandlePhaseStart
//
// Takes care of a start-of-phase notification from the visi library
//
int
HistVisi::HandlePhaseStart( int )
{
    // TODO - anything to do for the Histogram visi?
    return 0;
}



//
// HandlePhaseEnd
//
// Takes care of an start-of-phase notification from the visi library
//
int
HistVisi::HandlePhaseEnd( int )
{
    // TODO - anything to do for the Histogram visi?
    return 0;
}



//
// HandlePhaseData
//
// Takes care of phase information from the visi library
//
int
HistVisi::HandlePhaseData( int )
{
    // TODO - anything to do for the Histogram visi?
    return 0;
}


//
// HandleParadynExited
//
// Responds to termination of the connection between the front end
// and the visi.
//
int
HistVisi::HandleParadynTermination( int )
{
    int ret = TCL_OK;
    int keep = 0;           // note - we don't use a bool here because
                            // we pass this to the Tcl_GetBooleanFromObj call


    // determine whether we are to keep on Paradyn exit
    Tcl_Obj* varObj = Tcl_ObjGetVar2( interp,
                                        Tcl_NewStringObj( "::RTHist::is_kept_on_paradyn_exit", -1 ),
                                        NULL,
                                        0 );
    if( varObj != NULL )
    {
        ret = Tcl_GetBooleanFromObj( interp, varObj, &keep );
    }
    else
    {
        ret = TCL_ERROR;
    }

    if( (ret == TCL_OK) && !keep )
    {
        ret = Tcl_EvalObj( interp, Tcl_NewStringObj( "::RTHist::shutdown", -1 ) );
    }

    return ret;
}


//---------------------------------------------------------------------------
// HistVisi::MetricResourcePair implementation
//---------------------------------------------------------------------------

HistVisi::MetricResourcePair::MetricResourcePair( int metricId, int resourceId )
  : midx( metricId ),
    ridx( resourceId ),
    lastFilledBucket( -1 )
{
    const char* name = visi_MetricName(midx);
    if( name != NULL )
    {
        mname = name;
    }
    name = visi_ResourceName(ridx);
    if( name != NULL )
    {
        rname = name;
    }
}



//---------------------------------------------------------------------------
// explicit template instantiations needed by this class
//---------------------------------------------------------------------------
template class vector<HistVisi::MetricResourcePair*>;

