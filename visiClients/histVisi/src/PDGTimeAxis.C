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
//---------------------------------------------------------------------------
// 
// PDGTimeAxis.C
//
// Definition of PDGraph::TimeAxisW class.
// A PDGraph object is the C++ portion of a custom Tcl widget to
// support the drawing of time-based histograms.  The TimeAxisW nested
// class is the C++ portion of the time axis widget subwindow of a PDGraph 
// megawidget.
//
//---------------------------------------------------------------------------
// $Id: PDGTimeAxis.C,v 1.10 2004/03/23 01:12:48 eli Exp $
//---------------------------------------------------------------------------
#include <limits.h>
#include <iostream>
#include <sstream>
#include "common/h/String.h"
#include <assert.h>
#include <string.h>
#include "tcl.h"
#include "tk.h"

#include "common/h/Dictionary.h"
#include "PDGraph.h"


Tk_ConfigSpec PDGraph::TimeAxisW::configSpecs[] =
{
    {
        TK_CONFIG_PIXELS, 
        "-width", 
        "width", 
        "Width",
        "50", 
        Tk_Offset(SubWindowTkData, width), 
        0,
        NULL
    },
    {
        TK_CONFIG_PIXELS, 
        "-height", 
        "height", 
        "Height",
        "50", 
        Tk_Offset(SubWindowTkData, height), 
        0,
        NULL
    },
    {
        TK_CONFIG_END, 
        NULL, 
        NULL, 
        NULL,
        NULL, 
        0, 
        0,
        NULL
    }
};

const char*    PDGraph::TimeAxisW::secondsLabel    = "Sec";
const char*    PDGraph::TimeAxisW::minutesLabel    = "Min:Sec";
const char*    PDGraph::TimeAxisW::hoursLabel    = "Hours:Min";
const char*    PDGraph::TimeAxisW::tuSecondsFormat    = "%.0f";
const char*    PDGraph::TimeAxisW::tuMinutesFormat    = "%u:%02u";
const char*    PDGraph::TimeAxisW::tuHoursFormat    = "%u:%02u";
const int   PDGraph::TimeAxisW::tickLen        = 4;
const int   PDGraph::TimeAxisW::tickPad        = 2;

const int   PDGraph::TimeAxisW::kMaxSeconds     = 200;
const int   PDGraph::TimeAxisW::kMaxMinutes     = 60 * 60;


// InstallClassCommand - installs the "pdgraph_timeaxis" command into the 
// given Tcl  interpreter.
//
//
int
PDGraph::TimeAxisW::InstallClassCommand( Tcl_Interp* interp )
{
    int ret = TCL_OK;

    Tcl_Command cmd = Tcl_CreateCommand( interp,
            "pdgraph_timeaxis",
            TimeAxisW::ClassCmdCB,
            Tk_MainWindow( interp ),
            NULL );
    if( cmd == NULL )
    {
        ret = TCL_ERROR;
    }
    return ret;
}



int
PDGraph::TimeAxisW::ClassCmdCB( ClientData cd, Tcl_Interp* interp,
                                int argc, TCLCONST char* argv[] )
{
    Tk_Window mwin = (Tk_Window)cd;

    // validate command argument count
    if( argc < 2 )
    {
        std::ostringstream estr;

        estr << "wrong # args: should be \""
            << argv[0]
            << " pathName ?options?\""
            << std::ends;
        Tcl_SetObjResult( interp, Tcl_NewStringObj( estr.str().c_str(), -1 ));

        return TCL_ERROR;
    }

    // obtain an object to represent the widget
    TimeAxisW* ta = new TimeAxisW;
    if( ta->InitTclTk( interp, mwin, argc, argv ) != TCL_OK )
    {
        delete ta;
        return TCL_ERROR;
    }

    // set interpreter result to pathname associated with main window
    Tcl_SetObjResult( interp, Tcl_NewStringObj( Tk_PathName( ta->tkwin ), -1 ));

    return TCL_OK;
}


// InitTclTk - initialize the Tcl/Tk members for the widget
//
int
PDGraph::TimeAxisW::InitTclTk( Tcl_Interp* interp, Tk_Window mwin,
                                int argc, TCLCONST char* argv[] )
{
    // create a window for the widget
    tkwin = Tk_CreateWindowFromPath( interp, mwin, argv[1], NULL );
    if( tkwin == NULL )
    {
        // initialization failed
        return TCL_ERROR;
    }
    Tk_SetClass( tkwin, "Pdgraph_timeaxis" );

    // register an instance command with the interpreter
    // so our widget can handle its own commands
    widgetCmd = Tcl_CreateCommand(interp,
        Tk_PathName(tkwin),
        InstanceCmdCB,
        (ClientData)this,
        InstanceCmdDeletedCB);

    Tk_CreateEventHandler(tkwin,
        ExposureMask|StructureNotifyMask,
        EventCB,
        (ClientData)this);

    // handle any configuration options given
    if( Configure( interp, argc - 2, argv + 2, 0 ) != TCL_OK )
    {
        Tk_DestroyWindow(tkwin);
        return TCL_ERROR;
    }

    // register our association of Tk_Window and instance data
    winInstDataMap[tkwin] = (ClientData)this;

    return TCL_OK;
}


// InitCPP - initialize the "application-domain" members
//
void
PDGraph::TimeAxisW::InitCPP( PDGraph* pdgraph )
{
    g = pdgraph;
}


// Configure - handle Tcl configuration of the widget from the
// given arguments
//
int
PDGraph::TimeAxisW::Configure( Tcl_Interp* interp,
                                int argc, TCLCONST char* argv[], int flags )
{
    // handle Tk options
    if( Tk_ConfigureWidget(interp,
        tkwin,
        configSpecs,
        argc, argv,
        (char *)&(this->wdata), flags) != TCL_OK)
    {
        return TCL_ERROR;
    }

    // register our desired geometry
    Tk_GeometryRequest( tkwin, wdata.width, wdata.height );

    // arrange for ourselves to be redisplayed with
    // our new configuration
    RequestRedraw();

    return TCL_OK;
}


// Draw - Draw a representation of our data to our window
//
void
PDGraph::TimeAxisW::Draw( void )
{
    Pixmap pm = None;
    Drawable d;


    // reset our pending redisplay flag
    redrawPending = false;

    // no need to draw if we aren't yet mapped
    if (!Tk_IsMapped(tkwin))
    {
        return;
    }

    // draw into offscreen pixmap if desired
    if( g->IsDoubleBuffered() )
    {
        pm = Tk_GetPixmap(Tk_Display(tkwin),
            Tk_WindowId(tkwin),
            Tk_Width(tkwin),
            Tk_Height(tkwin),
            DefaultDepthOfScreen(Tk_Screen(tkwin)));
        d = pm;
    }
    else
    {
        d = Tk_WindowId(tkwin);
    }

    // clear the background
    Tk_Fill3DRectangle(tkwin,
        d,
        g->GetBackground(),
        0, 0,
        Tk_Width(tkwin),
        Tk_Height(tkwin),
        0, TK_RELIEF_FLAT);

    //
    // draw a representation of our data
    //

    // draw our unit string...
    const char* unitsString = FindUnitsLabel();
    int unitsStringLen = strlen( unitsString );
    int unitsStringWidth = Tk_TextWidth( g->GetFont(), unitsString, unitsStringLen );

    Tk_DrawChars( Tk_Display( tkwin ), d,
        g->GetDrawGC(),
        g->GetFont(),
        unitsString,
        unitsStringLen,
        ((Tk_Width( tkwin ) - unitsStringWidth)/2),
		tickLen + tickPad + 2 * g->GetFontMetrics().linespace );

    // draw time axis ticks...

    // determine first tick value
    const PDGraph::VisualScopeInfo& visScopeInfo = g->GetVisScopeInfo();
    const PDGraph::HistogramInfo& histInfo = g->GetHistogramInfo();

    double vWidth = visScopeInfo.focus * histInfo.bucketWidth * histInfo.nBuckets;
    double vStart = minValue + visScopeInfo.start * histInfo.bucketWidth * histInfo.nBuckets;
    double vInterval = GetIntervalWidth();
    double vFirstTick = vStart;
    unsigned int i;
    for( i = 0; i < nIntervals; i++ )
    {
        if( minValue + i * vInterval >= vStart )
        {
            // minValue + i * vInterval is first to be shown
            vFirstTick = minValue + i * vInterval;
            break;
        }
    }

    // determine first tick position
    double xStart = ((vFirstTick - vStart) / vWidth) * Tk_Width(tkwin);
    double xInterval = (vInterval / vWidth) * Tk_Width(tkwin);
    assert( xInterval != 0 );

    // draw ticks
    double dx = xStart;
    int y = 0;
    double val = vFirstTick;
    while( dx <= Tk_Width( tkwin ) )
    {
        int x = (int)dx;


        // draw the current tick
        XDrawLine( Tk_Display( tkwin ), d, g->GetDrawGC(), x, y, x, y + tickLen );

        // draw the current tick's label
        char buf[32];
        switch( units )
        {
        case tuSeconds:
            sprintf( buf, tuSecondsFormat, val );
            break;

        case tuMinutes:
            {
                unsigned int minutes = (unsigned int)val / 60;
                unsigned int seconds = (unsigned int)val - minutes * 60;
                sprintf( buf, tuMinutesFormat, minutes, seconds );
            }
            break;

        case tuHours:
            {
                unsigned int hours = (unsigned int)(val / 3600);
                unsigned int minutes = (unsigned int)(val - hours * 3600) / 60;
                sprintf( buf, tuHoursFormat, hours, minutes );
            }
            break;

        default:
            // should never reach here
            assert( false );
        }
        int cxLabel;            // width of label, in pixels
        int bufLen = strlen( buf );
        Tk_MeasureChars( g->GetFont(), buf, bufLen, 0, 0, &cxLabel );
        Tk_DrawChars( Tk_Display(tkwin), d, g->GetDrawGC(),
            g->GetFont(),
            buf, bufLen,
            x - cxLabel / 2, y + tickLen + tickPad + g->GetFontMetrics().linespace );

        // advance to next tick
        dx += xInterval;
        val += vInterval;
    }

    // copy from offscreen pixmap if needed
    if( g->IsDoubleBuffered() )
    {
        XCopyArea(Tk_Display(tkwin),
            pm,
            Tk_WindowId(tkwin),
            g->GetDrawGC(),
            0, 0,
            (unsigned) Tk_Width(tkwin), (unsigned) Tk_Height(tkwin),
            0, 0);
        Tk_FreePixmap(Tk_Display(tkwin), pm);
    }
}




// UpdateConfiguration - updates the current location of tick marks
// based on the current PDGraph widget configuration
//
void
PDGraph::TimeAxisW::UpdateConfiguration( void )
{
    const PDGraph::HistogramInfo& histInfo = g->GetHistogramInfo();

    // update our notion of time from the (new) histogram info
    minValue = histInfo.startTimestamp;
    maxValue = histInfo.startTimestamp + histInfo.bucketWidth * histInfo.nBuckets;

    // determine (roughly) how many intervals
    // will be shown in the window...

    double timeVisible;     // visible width, in terms of seconds
    double intervalWidth;   // width of intervals, in seconds
    unsigned int nIntervalsShown;


	timeVisible = histInfo.bucketWidth * histInfo.nBuckets;
    intervalWidth = GetIntervalWidth();
    nIntervalsShown = (unsigned int)(timeVisible / intervalWidth);

    // ...adjust the number of intervals so that there
    // are somewhere between five and ten
    if( nIntervalsShown <= 5 )
    {
        // there will be too few intervals,
        // so make some more
        nIntervals *= 2;
    }
    else if( nIntervalsShown > 10 )
    {
        // there will be too many intervals,
        // so reduce the number
        nIntervals /= 2;
    }

    // update our label format
    if( maxValue <= kMaxSeconds )
    {
        units = tuSeconds;
    }
    else if( maxValue <= kMaxMinutes )
    {
        units = tuMinutes;
    }
    else
    {
        units = tuHours;
    }
}


// HandleEvent - respond to the given X event
//
void
PDGraph::TimeAxisW::HandleEvent( XEvent* ev )
{
    switch( ev->type )
    {
    case Expose:
        // schedule a redraw if needed
        RequestRedraw();
        break;

    case ConfigureNotify:
        if( tkwin != NULL )
        {
            Tk_GeometryRequest( tkwin, Tk_Width(tkwin), DetermineHeight() );
        }
        break;

    case DestroyNotify:
        // remove our instance command
        if( (tkwin != NULL) && (g != NULL) )
        {
            tkwin = NULL;
            Tcl_DeleteCommandFromToken( g->GetInterp(), widgetCmd);

            // cancel any pending redraw as unnecessary
            CancelRedraw();

            // schedule our own termination
            Tcl_EventuallyFree((ClientData)this, TimeAxisW::DestroyCB);
        }

        break;

    default:
        // we don't handle the other event types
        ;
    }
}


// HandleCommand - respond to Tcl instance command
//
int
PDGraph::TimeAxisW::HandleCommand( Tcl_Interp* interp,
                                    int argc, TCLCONST char* argv[] )
{
    int result = TCL_OK;
    size_t length;
    char c;


    // verify argument count
    if( argc < 2 )
    {
        std::ostringstream estr;

        estr << "wrong # args: should be \""
            << argv[0]
            << " option ?arg arg ...?\""
            << std::ends;
        Tcl_SetObjResult( interp, Tcl_NewStringObj( estr.str().c_str(), -1 ));

        return TCL_ERROR;
    }

    // bump our reference count so we can't be
    // deleted during this command processing
    Tcl_Preserve((ClientData)this);


    c = argv[1][0];
    length = strlen(argv[1]);
    if((c == 'c') && (strncmp(argv[1], "cget", length) == 0) && (length >= 2))
    {
        // handle a 'cget' command
        if (argc != 3)
        {
            std::ostringstream estr;

            estr << "wrong # args: should be \""
                << argv[0]
                << " cget option\""
                << std::ends;
            Tcl_SetObjResult( interp, Tcl_NewStringObj( estr.str().c_str(),
                                                        -1 ));

            goto error;
        }
        result = Tk_ConfigureValue( interp,
            tkwin,
            configSpecs,
            (char*)this,
            argv[2],
            0);
    }
    else if((c == 'c') && (strncmp(argv[1], "configure", length) == 0) && (length >= 2))
    {
        // handle a 'configure' command
        if( argc == 2 )
        {
            result = Tk_ConfigureInfo( interp,
                tkwin,
                configSpecs,
                (char*)this,
                NULL,
                0);
        }
        else if( argc == 3 )
        {
            result = Tk_ConfigureInfo( interp,
                tkwin,
                configSpecs,
                (char*)this,
                argv[2],
                0);
        }
        else
        {
            // handle any widget-specific configuration options
            result = Configure( interp, argc-2, argv+2, TK_CONFIG_ARGV_ONLY );
        }
    }
    else
    {
        std::ostringstream estr;

        estr << "bad option \""
            << argv[1]
            << "\": must be cget or configure"
            << std::ends;
        Tcl_SetObjResult( interp, Tcl_NewStringObj( estr.str().c_str(), -1 ));

        goto error;
    }

    // schedule a redraw to reflect our new configuration
    RequestRedraw();

    // drop our reference count
    Tcl_Release((ClientData)this);
    return result;

error:
    // drop our reference count
    Tcl_Release((ClientData)this);
    return TCL_ERROR;
}


// HandleInstanceCommandDeleted - respond to situation when
// instance command has been deleted
//
void
PDGraph::TimeAxisW::HandleInstanceCommandDeleted( void ) 
{
    // destroy the widget in case it hasn't been destroyed already
    if( tkwin != NULL )
    {
        Tk_Window saved = tkwin;

        // indicate that we're on our way out
        tkwin = NULL;

        // now destroy our windows - this will
        // eventually result in our own destruction
        Tk_DestroyWindow( saved );
    }
}

const char*
PDGraph::TimeAxisW::FindUnitsLabel( void ) const
{
    const char* unitsString = NULL;

    switch( units )
    {
    case tuSeconds:
        unitsString = secondsLabel;
        break;

    case tuMinutes:
        unitsString = minutesLabel;
        break;

    case tuHours:
        unitsString = hoursLabel;
        break;

    default:
        // should never reach here
        assert( false );
    }
    return unitsString;
}

//---------------------------------------------------------------------------
// PDGraph::TimeAxisW callbacks
//---------------------------------------------------------------------------

int
PDGraph::TimeAxisW::InstanceCmdCB( ClientData cd, Tcl_Interp* interp,
                                    int argc, TCLCONST char* argv[] )
{
    TimeAxisW* ta = (TimeAxisW*)cd;
    return ta->HandleCommand( interp, argc, argv );
}


void
PDGraph::TimeAxisW::DrawCB(ClientData clientData)
{
    TimeAxisW* ta = (TimeAxisW*)clientData;
    ta->Draw();
}


void
PDGraph::TimeAxisW::EventCB( ClientData cd, XEvent* eventPtr )
{
    TimeAxisW* ta = (TimeAxisW*)cd;
    ta->HandleEvent( eventPtr );
}


void
PDGraph::TimeAxisW::DestroyCB( char* cd )
{
    TimeAxisW* ta = (TimeAxisW*)cd;
    delete ta;
}



void
PDGraph::TimeAxisW::InstanceCmdDeletedCB( ClientData cd )
{
    TimeAxisW* ta = (TimeAxisW*)cd;
    ta->HandleInstanceCommandDeleted();
}

