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
// HistVisi.h
//
// Declaration of HistVisi class.
// A HistVisi represents the Paradyn histogram visi.
//
//---------------------------------------------------------------------------
// $Id: HistVisi.h,v 1.3 2001/11/07 05:03:26 darnold Exp $
//---------------------------------------------------------------------------
#ifndef HISTVISI_H
#define HISTVISI_H


class PDGraph;

class HistVisi
{
private:
    // VisiLibEvent - used for table-driven registration of our
    // handlers with the visi library
    //
    struct VisiLibEvent
    {
        visi_msgTag    evt;
        int (*cb)(int);
    };


    // MetricResourcePair - structs of this type are used to
    // denote the metric-resource pairs whose data is currently
    // being shown in the histogram display.
    //
    struct MetricResourcePair
    {
        int midx;
        int ridx;
        string mname;
        string rname;
        int lastFilledBucket;

        // graph-related information
        int curveId;

        MetricResourcePair(int midx, int ridx);
    };


    static    VisiLibEvent libEventInfo[];    // visi lib event handlers
    static    HistVisi* histVisi;             // singleton visi object
    static    const unsigned int maxCurves;   // max allowable curves

    Tcl_Interp* interp;    
    bool visiLibInitialized;                  // is visi library initialized?
    vector<MetricResourcePair*> mrpairs;      // active metric-resource pairs
    PDGraph* graph;                           // custom graph object
    DGClient* dg;                             // object providing access to
                                              // visi library from Tcl code


    // HandleParadynInput - Tcl channel callback, called in response
    // to new input from Paradyn
    //
    static    void    HandleParadynInput( void*, int*, unsigned long* );

    // InitUI - handles initialization of the HistVisi user interface.
    //
    bool    InitUI( void );

    // InitParadynConnection - handles initialization of the data connection
    // to the Paradyn front end.
    //
    bool    InitParadynConnection( void );

    // AddMetricResourcePair - activates an association between the
    // given metric and resource (as represented by their visi library indices)
    //
    bool    AddMetricResourcePair( int midx, int ridx );

    // chan -- used so that TCL event loop can probe PDSOCKET used by
    //         visiLib for interactions with the front end
    Tcl_Channel chan;

    // Handle* - respond to the corresponding visi library event
    //
    int    HandleAddMetricsAndResources( int lastBucket );
    int    HandleData( int lastBucket, bool isFold );
    int    HandlePhaseStart( int lastBucket );
    int    HandlePhaseEnd( int lastBucket );
    int    HandlePhaseData( int lastBucket );
    int HandleParadynTermination( int lastBucket );

    // visilib event callbacks - these handlers turn around and call
    // the corresponding Handle* object methods
    //
    static    int    AddMetricsAndResourcesCB( int );
    static    int    DataCB( int );
    static    int    FoldCB( int );
    static    int    PhaseStartCB( int );
    static    int    PhaseEndCB( int );
    static    int    PhaseDataCB( int );
    static  int ParadynExitedCB( int );

    // RemoveCurve, HandleRemoveCurveCB - implementation of a Tcl
    // command, to allow us to respond in C++ code when a curve is
    // removed using the user interface
    //
    int RemoveCurve( Tcl_Interp* interp, int objc, Tcl_Obj* CONST objv[] );
    static  int HandleRemoveCB( ClientData cd,
                                Tcl_Interp* interp,
                                int objc,
                                Tcl_Obj* CONST objv[] );

	// UpdatePhaseName - updates the phase label in the UI
	void	UpdatePhaseName( void );

public:
    HistVisi( Tcl_Interp* interp );
    ~HistVisi( void );

    // Init - initializes the HistVisi object.
    bool    Init( void );

    // Run - begin handling events
    void    Run( void )                { Tk_MainLoop(); }
};

#endif // HISTVISI_H

