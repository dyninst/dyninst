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
//---------------------------------------------------------------------------
// 
// PDGraph.h
//
// Declaration of PDGraph class.
// A PDGraph object is the C++ portion of a custom Tcl widget to
// support the drawing of time-based histograms.
//
//---------------------------------------------------------------------------
// $Id: PDGraph.h,v 1.7 2003/06/27 17:59:33 pcroth Exp $
//---------------------------------------------------------------------------
#ifndef PDGRAPH_H
#define PDGRAPH_H


// Tk_ConfigureWidget expects offsets to variables within a structure.
// Having these variables as members of a class causes problems in
// finding the offsets of the members within an object instance.
// So, we specify Tk data in separate structs, which are then 
// used as members of the corresponding class.
struct SubWindowTkData
{
    int         width;            // window width
    int         height;           // window height
};


struct GraphTkData
{
    Tk_3DBorder bgBorder;       // background border
    Tk_3DBorder fgBorder;       // foreground border
    int         borderWidth;    // width of 3D border around widget
    int         relief;         // relief for main window
    Tk_Font     font;           // font to use when drawing axes, etc.
    char*       lineColors;     // string holding allowable line color names
    char*       linePatterns;   // string holding allowable line pattern names
    int         doubleBuffer;   // draw into offscreen pixmap?
};




class PDGraph
{
private:

    // Axis
    // Struct for holding basic information about an axis
    //
    struct Axis
    {
        double minValue;            // min value of axis
        double maxValue;            // max value of axis
        unsigned int nIntervals;    // number of intervals to use


        Axis( double minVal = 0.0,
              double maxVal = 1.0,
              unsigned int numIntervals = 1 )
            : minValue( minVal ),
              maxValue( maxVal ),
              nIntervals( numIntervals )
        {}

        // GetIntervalWidth - computes the width of each axis interval
        double    GetIntervalWidth( void ) const
                {
                    return (maxValue - minValue) / nIntervals;
                }
    };


    // ValueAxis
    // An Axis that knows about the additional information needed by
    // a Value axis.
    //
    struct ValueAxis : public Axis
    {
        string  title;
        double  intervalWidth;      // value increment across each interval
        char    labelFormat[32];

        ValueAxis( const char* title_,
                    double minVal = 0.0,
                    double maxVal = 0.0,
                    unsigned int nIntervals = 1 )
            : Axis( minVal, maxVal, nIntervals ),
              title( title_ ),
              intervalWidth( (maxVal - minVal) / nIntervals )
        {}

        // ComputeIntervals - given the maximum number of ticks
        // allowed, computes a visually appealing number of intervals
        // for the Axis
        //
        void    ComputeIntervals( unsigned int nTicksMax );
    };


    // Group
    // Object representing a collection of Curves that share the same
    // value axis.
    // 
    struct Curve;
    struct Group
    {
        ValueAxis* axis;         // Y axis for the group of curves
        int nVisible;            // number of curves currently visible
        pdvector<Curve*> curves;   // curves belonging to the group


        Group( const char* title );
        ~Group( void );

        // Add - Add a curve to the group
        void    Add( Curve* curve );

        // Remove - Remove a curve from the group
        void    Remove( Curve* curve );

        // UpdateMaxValue - respond to the situation where the data for one of
        // the curves in our group has observed a new maximum value
        void    UpdateMaxValue( double val );

        // IsVisible - indicates whether any of the group's curves are
        // currently visible
        bool    IsVisible( void ) const     { return (nVisible > 0); }
    };


    // Curve
    // Object representing a curve in the PDGraph.
    struct Curve
    {
    private:
        // LineSpec
        // Struct containing specification for the line that will
        // represent the curve's data in the graph
        struct LineSpec
        {
            GC      gc;
            XColor* color;
            char*   stippleName;

            LineSpec( void )
              : gc( None ),
                color( NULL ),
                stippleName( NULL )
            {}

            LineSpec( GC gc_, XColor* color_, char* stippleName_ )
                : gc( gc_ ),
                  color( color_ ),
                  stippleName( stippleName_ )
            {}
        };

        static  pdvector<LineSpec>    lineColorSpecs;     // allowable line specs
        static  pdvector<LineSpec>    linePatternSpecs;
        static  unsigned int        nextLineSpecIdx;    // next spec to use


        // DrawEndpoint - draws a curve endpoint at the indicated location
        void    DrawEndpoint( Display* disp,
                                Drawable d,
                                const XPoint& pt ) const
                {
                    XFillRectangle( disp, d, GetLineSpec().gc,
                                    pt.x-1, pt.y-1, 4, 4 );
                }

		// UpdateMaxActiveValue - determines the max value from the active
		// data set, considering all data values with index i or larger
		void	UpdateMaxActiveValue( unsigned int startIdx );

    public:
        // domain data
        static const unsigned int   smoothingWindowSize;    // number of points
                                                            // to consider per 
                                                            // smoothed point

        string  name;           // curve name
        bool    isSmoothed;     // is curve data being smoothed?

        unsigned int nPoints;   // number of data points
        double* pts;            // curve data
        double* spts;           // smoothed curve data
        Group*  group;          // group to which curve belongs

		double	maxActiveValue;	// max value of the active data

        // GUI data
        XPoint* xpts;           // X points corresponding to data
        unsigned int lineSpecIdx; // index of line spec to use when drawing data
        bool    useColor;       // is curve to be shown in color?
        bool    isVisible;      // is curve to be shown on the display?

        
        // InitLineSpecs - try to obtain a set of line specifications
        static  int     InitLineSpecs( Tcl_Interp* interp,
                                        Tk_Window win,
                                        char* lineColors,
                                        char* linePatterns,
                                        Tk_3DBorder fg);

        // ReleaseLineSpecs - release line specification resources
        static  void    ReleaseLineSpecs( Display* disp );


        // constructors
        Curve( const char* metricName,
                const char* resourceName,
                unsigned int maxBuckets );
        ~Curve( void );

   
        // GetLineSpec - retrieve the line spec that should be used to
        // draw our data
        LineSpec&   GetLineSpec( void ) const
            {
                return ( useColor ?
                            lineColorSpecs[lineSpecIdx % lineColorSpecs.size()]
                            : linePatternSpecs[lineSpecIdx % linePatternSpecs.size()] );
            }

        // GetActiveData - access the data that should be shown on the display
        // (raw or smoothed, depending on the user's choice)
        double* GetActiveData( void ) const
            {
                return ( isSmoothed ? spts : pts );
            }


		// GetMaxActiveValue - determine the maximum value of the 
		// active data set
		double GetMaxActiveValue( void ) const
			{
				return maxActiveValue;
			}

        // GetName - obtain a string representation of the name that should
        // be used for the curve.  If the active data is the smoothed data,
        // the name is suffixed with "(smoothed)"
        string  GetName( void ) const
            {
                return (isSmoothed ? (name + " (smoothed)") : name);
            }        


        // SetData - associates the given data with our curve,
        // starting at the indicated data point index
        //
        void    SetData( unsigned int startIdx,
                         unsigned int nPoints,
                         const float* data );


        // ComputeSmoothedData - using the given smoothing window size,
        // computes the smoothed data points for the current set of raw data
        // within the given data point interval
        //
        void    ComputeSmoothedData( unsigned int firstSample,
                                        unsigned int lastSample,
                                        unsigned int winSize );

        // ComputeXPoints - computes the coordinates to be used to represent
        // the given subset of our currently active data within the given
        // onscreen rectangle.  Also takes into account the
        // current start and focus values to adjust to the current zoom 
        // and pan settings of the graph.
        //
        void    ComputeXPoints( unsigned int startIdx,
                                unsigned int nPoints,
                                XRectangle& rect,
                                double start,
                                double focus,
                                unsigned int nBuckets );

        // ComputeXPoints - computes the coordinates to be used to represent
        // all of our currently active data within the given onscreen rectangle.
        // Also takes into account the current start and focus values to adjust
        // the to the current zoom and pan settingsof the graph.
        //
        void    ComputeXPoints( XRectangle& rect,
                                double start,
                                double focus,
                                unsigned int nBuckets )
                {
                    ComputeXPoints( 0, nPoints, rect, start, focus, nBuckets );
                }

    
        // Draw - Draw a representation of the indicated subset of current data
        // to the given Display and Drawable.
        //
        void    Draw( Display* disp,
                        Drawable d,
                        unsigned int startIdx,
                        unsigned int nPointsToDraw ) const;

        // Draw - Draws a representation of all of our current data to
        // the given Display and Drawable
        //
        void    Draw( Display* disp, Drawable d )
                {
                    Draw( disp, d, 0, nPoints );
                }

        // Smooth - Indicates that we should use smoothed data as our
        // active data.  If we are not already smoothed, computes smoothed data.
        //
        void    Smooth( void );


        // Unsmooth - Indicates that we should use raw data as our active data.
        void    Unsmooth( void );


        // Show - indicate that our data should be visible (drawn in the graph)
        void    Show( void )
                {
                    if( !isVisible )
                    {
                        isVisible = true;
                        group->nVisible++;
                    }
                }

        // Hide - indicate that our data should not be visible in the graph.
        void    Hide( void )
                {
                    if( isVisible )
                    {
                        isVisible = false;
                        group->nVisible--;
                    }
                }
    };


    // HistogramInfo
    // Struct holding information about the current characteristics of the
    // data histogram.
    //
    struct HistogramInfo
    {
        double          startTimestamp;     // timestamp of leading edge of histogram
        unsigned int    nBuckets;           // number of histogram buckets
        double          bucketWidth;        // width of histogram buckets

        HistogramInfo( void )
          : startTimestamp( 0 ),
            nBuckets( 1 ),
            bucketWidth( 1 )
        {}
    };


    // VisualScopeInfo
    // Struct holding information about the current zoom and pan settings
    // of the graph
    //
    struct VisualScopeInfo
    {
        double  start;      // fraction from start to left
        double  focus;      // fraction of histogram shown

        VisualScopeInfo( void )
          : start( 0.0 ),
            focus( 1.0 )
        {}
    };


    // ValueAxisW
    // C++ object associated with the pdgraph_valueaxis Tcl widget.
    //
    class ValueAxisW
    {
        static    const int   tickLen;        // length of axis ticks (pixels)
        static    const int   tickPad;        // amount of tick padding (pixels)

        PDGraph*    g;

        static  Tk_ConfigSpec   configSpecs[];    // configuration descriptions
        SubWindowTkData wdata;      // Tk configuration data

        Tk_Window   tkwin;          // token for widget's (main) window
        Tcl_Command widgetCmd;      // token for widget's instance command
        bool        redrawPending;  // is redraw pending for widget?
        unsigned int    timeAxisHeight; // height of time axis

        // Tcl callbacks
        static  int     ClassCmdCB( ClientData cd, Tcl_Interp* interp,
                                    int argc, TCLCONST char* argv[] );
        static  int     InstanceCmdCB( ClientData cd, Tcl_Interp* interp,
                                    int argc, TCLCONST char* argv[] );
        static  void    InstanceCmdDeletedCB( ClientData cd );
        static  void    DrawCB( ClientData cd );
        static  void    EventCB( ClientData cd, XEvent* eventPtr );
        static  void    DestroyCB( char* cd );


        // construction/destruction
        ValueAxisW( void )
            : g( NULL ),
              tkwin( NULL ),
              widgetCmd( NULL ),
              redrawPending( false ),
              timeAxisHeight( 0 )
        {
            wdata.width = 0;
            wdata.height = 0;
        }
        ~ValueAxisW( void ) {}


        // DetermineLabelHeight - computes height to be reserved for
        // axis labels
        //
        int DetermineLabelHeight( void ) const
            {
                return 3 * g->GetFontMetrics().linespace;
            }


        // DetermineWidth - computes width of value axis; width
        // is computed using the current set of visible curves
        unsigned int    DetermineWidth( const ValueAxis* axis ) const;


        // GetWindow - access the Tk window associated with the Value Axis
        Tk_Window       GetWindow( void )   { return tkwin; }


        // InitTclTk - initialize the Tcl/Tk members for the widget
        int     InitTclTk( Tcl_Interp* interp,
                            Tk_Window mainWin,
                            int argc,
                            TCLCONST char* argv[] );

        // InitCPP - initialize the "application-domain" members
        void    InitCPP( PDGraph* pdgraph, unsigned int timeAxisHeight );


        // Configure - handle Tcl configuration of the widget from the
        // given arguments
        int     Configure( Tcl_Interp* interp,
                            int argc,
                            TCLCONST char* argv[],
                            int flags );

        // Draw - output a representation of ourselves to our window
        void    Draw( void );


        // DrawAxis - output a representation of the given ValueAxis to
        // our window at the location indicated by rect
        void    DrawAxis( Drawable d, ValueAxis* axis, XRectangle& rect );


        // RequestRedraw - request that a redraw be performed at some point
        // in the future
        //
        void    RequestRedraw( void )
                {
                    if( !redrawPending )
                    {
                        redrawPending = true;
                        Tcl_DoWhenIdle( ValueAxisW::DrawCB, (ClientData)this );
                    }
                }

        // CancelRedraw - cancel any pending redraw requests
        //
        void    CancelRedraw( void )
                {
                    if( redrawPending )
                    {
                        Tcl_CancelIdleCall( ValueAxisW::DrawCB, (ClientData)this );
                        redrawPending = false;
                    }
                }


        // HandleEvent - respond to the given X event
        void    HandleEvent( XEvent* ev );

        // HandleCommand - respond to Tcl instance command
        int     HandleCommand( Tcl_Interp* interp,
                                int argc, TCLCONST char* argv[] );

        // HandleInstanceCommandDeleted - respond to situation when
        // instance command has been deleted
        void    HandleInstanceCommandDeleted( void );

        // HandleConfigureNotification - respond to a notification that
        // the widget's configuration has changed
        void    HandleConfigureNotification( void );

    public:
        // InstallClassCommand - installs the "pdgraph_valueaxis" command
        // into the given Tcl interpreter
        static  int InstallClassCommand( Tcl_Interp* interp );

        friend class PDGraph;
    };



    // TimeAxisW
    // C++ object associated with the pdgraph_timeaxis Tcl widget.
    //
    class TimeAxisW : public Axis
    {
    private:
        enum TimeUnits
        {
            tuSeconds,
            tuMinutes,
            tuHours
        };

        static    const char*    secondsLabel;
        static    const char*    minutesLabel;
        static    const char*    hoursLabel;
        static    const char*    tuSecondsFormat;
        static    const char*    tuMinutesFormat;
        static    const char*    tuHoursFormat;
        static    const int      tickLen;
        static    const int      tickPad;
        static    const int      kMaxSeconds;
        static    const int      kMaxMinutes;

        TimeUnits    units;        // units currently in use
        PDGraph*    g;

        static  Tk_ConfigSpec   configSpecs[];    // configuration specifications

        Tk_Window   tkwin;
        Tcl_Command widgetCmd;      // token for widget's instance command
        bool        redrawPending;  // is a redraw pending for our widget?
        SubWindowTkData wdata;      // Tk configuration data


        // Tcl callbacks
        static  int     ClassCmdCB( ClientData cd, Tcl_Interp* interp,
                                    int argc, TCLCONST char* argv[] );
        static  int     InstanceCmdCB( ClientData cd, Tcl_Interp* interp,
                                    int argc, TCLCONST char* argv[] );
        static  void    InstanceCmdDeletedCB( ClientData cd );
        static  void    DrawCB( ClientData cd );
        static  void    EventCB( ClientData cd, XEvent* eventPtr );
        static  void    DestroyCB( char* cd );


        // construction/destruction
        TimeAxisW( void )
            : Axis( 0, 1, 10 ),
              units( tuSeconds ),
              g( NULL ),
              tkwin( NULL ),
              widgetCmd( NULL ),
              redrawPending( false )
        {
            wdata.width = 0;
            wdata.height = 0;
        }
        ~TimeAxisW( void )   {}


        // GetWindow - access the Tk window associated with the Value Axis
        Tk_Window       GetWindow( void )   { return tkwin; }


        // DetermineHeight - computes the height required by the time axis
        //
        int DetermineHeight( void ) const
            {
                return 2 * g->GetFontMetrics().linespace +    // units,tick labels
                        tickLen + tickPad;                  // for ticks
            }

        // FindUnitsLabel - determine correct units label to use,
        // depending on currently-active units
        //
        const char* FindUnitsLabel( void ) const;


        // InitTclTk - initialize the Tcl/Tk members for the widget
        int     InitTclTk( Tcl_Interp* interp,
                            Tk_Window mainWin,
                            int argc,
                            TCLCONST char* argv[] );

        // InitCPP - initialize the "application-domain" members
        void    InitCPP( PDGraph* pdgraph );


        // Configure - handle Tcl configuration of the widget from the
        // given arguments
        int     Configure( Tcl_Interp* interp,
                            int argc,
                            TCLCONST char* argv[],
                            int flags );

        // Draw - output a representation of ourselves to our window
        void    Draw( void );


        // UpdateConfiguration - updates the current location of tick marks
        // based on the current PDGraph widget configuration
        //
        void    UpdateConfiguration( void );


        // RequestRedraw - request that a redraw be performed at some point
        // in the future
        //
        void    RequestRedraw( void )
                {
                    if( !redrawPending )
                    {
                        redrawPending = true;
                        Tcl_DoWhenIdle( TimeAxisW::DrawCB, (ClientData)this );
                    }
                }

        // CancelRedraw - cancel any pending redraw requests
        //
        void    CancelRedraw( void )
                {
                    if( redrawPending )
                    {
                        Tcl_CancelIdleCall( TimeAxisW::DrawCB, (ClientData)this );
                        redrawPending = false;
                    }
                }

        // HandleEvent - respond to the given X event
        //
        void    HandleEvent( XEvent* ev );

        // HandleCommand - respond to Tcl instance command
        //
        int     HandleCommand( Tcl_Interp* interp,
                                int argc, TCLCONST char* argv[] );

        // HandleInstanceCommandDeleted - respond to situation when
        // instance command has been deleted
        //
        void    HandleInstanceCommandDeleted( void );

    public:
        // InstallClassCommand - installs the "pdgraph_valueaxis" command
        // into the given Tcl interpreter
        static  int InstallClassCommand( Tcl_Interp* interp );

        friend class PDGraph;
    };


    class DataW
    {
    private:
        // domain data
        PDGraph*    g;

        // Tcl/Tk data
        static  Tk_ConfigSpec   configSpecs[];

        Tk_Window   tkwin;
        Tcl_Command widgetCmd;      // token for widget's instance command
        bool        redrawPending;  // is a redraw pending for our widget?
        unsigned int    valLabelHeight;    // height of value axis labels
        SubWindowTkData   wdata;    // Tk configuration data


        // Tcl callbacks
        static  int     ClassCmdCB( ClientData cd, 
                                    Tcl_Interp* interp,
                                    int argc, TCLCONST char* argv[] );
        static  int     InstanceCmdCB( ClientData cd,
                                    Tcl_Interp* interp,
                                    int argc, TCLCONST char* argv[] );
        static  void    InstanceCmdDeletedCB( ClientData cd );
        static  void    DrawCB( ClientData cd );
        static  void    EventCB( ClientData cd, XEvent* eventPtr );
        static  void    DestroyCB( char* cd );


        // construction/destruction
        DataW( void )
            : g( NULL ),
              tkwin( NULL ),
              widgetCmd( NULL ),
              redrawPending( false ),
              valLabelHeight( 0 )
        {
            wdata.width = 0;
            wdata.height = 0;
        }
        ~DataW( void )   {}




        // GetWindow - access the Tk window associated with the Value Axis
        Tk_Window       GetWindow( void )   { return tkwin; }


        // InitTclTk - initialize the Tcl/Tk members for the widget
        int     InitTclTk( Tcl_Interp* interp,
                            Tk_Window mainWin,
                            int argc,
                            TCLCONST char* argv[] );

        // InitCPP - initialize the "application-domain" members
        void    InitCPP( PDGraph* pdgraph, unsigned int labelHeight );


        // Configure - handle Tcl configuration of the widget from the
        // given arguments
        int     Configure( Tcl_Interp* interp,
                            int argc,
                            TCLCONST char* argv[],
                            int flags );

        // Draw - output a representation of ourselves to our window
        void    Draw( void );


        // UpdateConfiguration - updates the current location of tick marks
        // based on the current PDGraph widget configuration
        //
        void    UpdateConfiguration( void );


        // HandleNewData - respond to observed new data for the given curve
        void    HandleNewData( Curve* curve,
                                unsigned int firstSampleIdx,
                                unsigned int nSamples );

        // RequestRedraw - request that a redraw be performed at some point
        // in the future
        //
        void    RequestRedraw( void )
                {
                    if( !redrawPending )
                    {
                        redrawPending = true;
                        Tcl_DoWhenIdle( DataW::DrawCB, (ClientData)this );
                    }
                }

        // CancelRedraw - cancel any pending redraws
        //
        void    CancelRedraw( void )
                {
                    if( redrawPending )
                    {
                        Tcl_CancelIdleCall( DataW::DrawCB, (ClientData)this );
                        redrawPending = false;
                    }
                }

        // HandleEvent - respond to the given X event
        //
        void    HandleEvent( XEvent* ev );

        // HandleCommand - respond to Tcl instance command
        //
        int     HandleCommand( Tcl_Interp* interp,
                                int argc, TCLCONST char* argv[] );

        // HandleInstanceCommandDeleted - respond to situation when
        // instance command has been deleted
        //
        void    HandleInstanceCommandDeleted( void );

    public:
        // InstallClassCommand - installs the "pdgraph_valueaxis" command
        // into the given Tcl interpreter
        static  int InstallClassCommand( Tcl_Interp* interp );

        friend class PDGraph;
    };


private:
    pdvector<Curve*>    curves;        // set of curves known to the graph
    pdvector<Group*>    groups;        // set of groups known to the graph
    TimeAxisW*      timeAxis;        // the graph's time axis subwidget
    ValueAxisW*     valAxis;         // the graph's value axis subwidget
    DataW*          dataw;           // the graph's data subwidget
    HistogramInfo   histInfo;        // current histogram configuration
    VisualScopeInfo visScopeInfo;    // current zoom and pan settings

    static  Tk_ConfigSpec configSpecs[];    // configuration specification
    static  dictionary_hash<Tk_Window, ClientData> winInstDataMap;
                                            // map associating Tk_Windows with
                                            // C++ objects, used to allow
                                            // us access to our subwidget's
                                            // C++ objects

    Tk_Window   tkwin;          // frame window around widget subwindows
    Display*    tkdisplay;      // display for the tkwin window
    Tk_Window   legendWin;      // window for graph legend

    Tcl_Interp* interp;         // Tcl interpreter associated with widget
    Tcl_Command widgetCmd;      // Token for widget's instance command

    GC gc;                      // graphics context for drawing
    Tk_FontMetrics    fontm;        // metrics for font
    GraphTkData       wdata;    // Tk configuration data
    bool redrawPending;         // redraw is pending?


    // construction/destruction
    PDGraph( void );
    ~PDGraph( void );


    // InitTclTk - initialize the Tcl/Tk members for the widget
    //
    int     InitTclTk( Tcl_Interp* interp, Tk_Window mainWin,
                        int argc, TCLCONST char* argv[] );

    // Configure - handle Tcl configuration of the widget from the
    // given arguments
    //
    int     Configure( Tcl_Interp* interp,
                        int argc,
                        TCLCONST char* argv[],
                        int flags );

    // Draw - output a representation of our data to our window
    // (Mostly, this work is handled directly by the subwidgets.)
    void    Draw( void );


    // DrawBorder - handle the drawing of our border to our Tk window
    void    DrawBorder( void ) const;

    // RequestRedraw - request that a redraw be performed at some point
    // in the future
    //
    void    RequestRedraw( void )
            {
                if( !redrawPending )
                {
                    redrawPending = true;
                    Tcl_DoWhenIdle( PDGraph::DrawCB, (ClientData)this );
                }
            }

    // CancelRedraw - cancel any pending redraw requests
    //
    void    CancelRedraw( void )
            {
                if( redrawPending )
                {
                    Tcl_CancelIdleCall( PDGraph::DrawCB, (ClientData)this );
                }
            }

    // HashTkWindow - dictionary_hash hash function, used to compute
    // hash values for Tk_Windows.
    //
    static  unsigned int    HashTkWindow( const Tk_Window& tkwin )
    {
        // the modulo operation handles the case on
        // systems (e.g., SGI) where a pointer is 64-bits wide
        // and an unsigned int is narrower, say 32-bits
        return (unsigned int)(((long)tkwin) % UINT_MAX);
    }


    // HandleConfigureNotification - responds to changes in configuration
    // of the graph widget
    //
    void    HandleConfigureNotification( void );

    // HandleEvent - responds to the given X event
    //
    void    HandleEvent( XEvent* ep );

    // HandleCmd - respond to a Tcl instance command
    // 
    int     HandleCmd( Tcl_Interp* interp, int argc, TCLCONST char* argv[] );

    // HandleInstanceCmdDeleted - respond to situation where instance
    // command has been deleted (indicating widget should go away)
    void    HandleInstanceCmdDeleted( void );


    // PanTo - pan the graph to the indicated position
    //
    int     PanTo( double position );

    // ZoomTo - zoom the graph to the indicate position
    //
    int     ZoomTo( double position );

    // UpdateGeometry - update the geometry of the subwidgets based on
    // the information held in the C++ objects
    //
    void    UpdateGeometry( void );


	// UpdateAxisMaxValue - updates the maximum value of an axis, and 
	// updates the display to reflect the new max value
	// 
	void UpdateAxisMaxValue( ValueAxis* axis, double maxValue );


    // FindGroup - looks up the Group with the given metric label as its title
    // if no group is so titled, returns NULL
    //
    Group*  FindGroup( const char* metricLabel ) const;


    // AddGroup - builds a new Group with the given metric label as its title
    Group*  AddGroup( const char* metricLabel );


    // Remove - Removes the given group from the set of groups known to
    // the graph
    //
    void    Remove( PDGraph::Group* group );

    // Remove - Removes the given curve from the set of curves known to the
    // graph.  If the removal of the curve results in a group becoming empty,
    // also handles removal of the associated group.
    //
    void    Remove( PDGraph::Curve* curve );


    // accessors
    Tcl_Interp* GetInterp( void )                   { return interp; }
    bool    IsDoubleBuffered( void ) const          { return wdata.doubleBuffer; }
    GC      GetDrawGC( void ) const                 { return gc; }
    Tk_3DBorder GetBackground( void ) const         { return wdata.bgBorder; }
    Tk_3DBorder GetForeground( void ) const         { return wdata.fgBorder; }
    const Tk_FontMetrics& GetFontMetrics( void ) const    { return fontm; }
    const Tk_Font& GetFont( void ) const            { return wdata.font; }
    const HistogramInfo&    GetHistogramInfo( void ) const  { return histInfo; }
    const VisualScopeInfo&  GetVisScopeInfo( void ) const   { return visScopeInfo; }
    const pdvector<Group*>&   GetGroups( void ) const { return groups; }
    const pdvector<Curve*>&   GetCurves( void ) const { return curves; }


    // Tcl callback routines
    static  int     ClassCmdCB( ClientData clientData,
                                Tcl_Interp* interp,
                                int argc, TCLCONST char* argv[] );
    static  int     InstanceCmdCB( ClientData cd,
                                Tcl_Interp* interp,
                                int argc, TCLCONST char* argv[] );
    static  void    InstanceCmdDeletedCB( ClientData cd );
    static  void    DrawCB( ClientData cd );
    static  void    EventCB( ClientData cd, XEvent* eventPtr );
    static  void    DestroyCB( char* cd );

    // Tcl callback handlers, called by the callback routines
    int     HandlePanCommand( int argc, TCLCONST char* argv[] );
    int     HandleZoomCommand( int argc, TCLCONST char* argv[] );
    int     HandleGetSelectedCommand( int argc, TCLCONST char* argv[] );
    int     HandleSmoothCommand( int argc, TCLCONST char* argv[], bool smooth );
    int     HandleShowCommand( int argc, TCLCONST char* argv[], bool show );
    int     HandleRemoveCommand( int argc, TCLCONST char* argv[] );

public:
    typedef int CurveID;       // nicer name for curve IDs


    // InstallClassCommand - installs the "pdgraph" command
    // into the given Tcl interpreter
    //
    static  int     InstallClassCommand( Tcl_Interp* interp );


    // FindInstanceData - looks up instance data associated with
    // for a given Tk window pathname.  Used as a toehold for
    // users of a graph widget to obtain access directly to the
    // C++ object associated with the graph
    // 
    static    bool    FindInstanceData( char* name,
                                        Tcl_Interp* interp,
                                        ClientData& cd );


    // UpdateHistogramInfo - updates the graph's idea about the
    // current characteristics of the histogram
    //
    void    UpdateHistogramInfo( double startTimestamp,
                            unsigned int nBuckets,
                            double bucketWidth );

    // AddCurve - add a curve to the graph with the given metric and
    // resource names
    //
    CurveID AddCurve( const char* metricName,
                            const char* metricLabel,
                            const char* resourceName );

    // SetCurveData - associate data with the indicated curve
    //
    void    SetCurveData( CurveID cid,
                            int firstSample,
                            int nSamples,
                            const float* data );


    friend struct Group;
    friend struct Curve;
    friend class ValueAxisW;
    friend class TimeAxisW;
    friend class DataW;
};

#endif // PDGRAPH_H

