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
// PDGData.C
//
// Definition of PDGraph::DataW class.
// A PDGraph object is the C++ portion of a custom Tcl widget to
// support the drawing of time-based histograms.  The DataW nested
// class is the C++ portion of the Data widget subwindow of a PDGraph 
// megawidget.
//
//---------------------------------------------------------------------------
// $Id: PDGData.C,v 1.8 2005/02/15 17:44:28 legendre Exp $
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


Tk_ConfigSpec PDGraph::DataW::configSpecs[] =
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


// InstallClassCommand - Installs the class Tcl command into the given 
// interpreter.
//
int
PDGraph::DataW::InstallClassCommand( Tcl_Interp* interp )
{
    int ret = TCL_OK;

    Tcl_Command cmd = Tcl_CreateCommand( interp,
            "pdgraph_data",
            DataW::ClassCmdCB,
            Tk_MainWindow( interp ),
            NULL );
    if( cmd == NULL )
    {
        ret = TCL_ERROR;
    }
    return ret;
}



int
PDGraph::DataW::ClassCmdCB( ClientData cd, Tcl_Interp* interp,
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
    DataW* dw = new DataW;
    if( dw->InitTclTk( interp, mwin, argc, argv ) != TCL_OK )
    {
        delete dw;
        return TCL_ERROR;
    }

    // set interpreter result to pathname associated with main window
    Tcl_SetObjResult( interp, Tcl_NewStringObj( Tk_PathName( dw->tkwin ), -1 ));

    return TCL_OK;
}


// InitTclTk - initialize the Tcl/Tk members for the widget
//
int
PDGraph::DataW::InitTclTk( Tcl_Interp* interp,
                            Tk_Window mwin, 
                            int argc, TCLCONST char* argv[] )
{
    // create a window for the widget
    tkwin = Tk_CreateWindowFromPath( interp, mwin, argv[1], NULL );
    if( tkwin == NULL )
    {
        // initialization failed
        return TCL_ERROR;
    }
    Tk_SetClass( tkwin, "Pdgraph_data" );

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
PDGraph::DataW::InitCPP( PDGraph* pdgraph, unsigned int labHeight )
{
    g = pdgraph;
    valLabelHeight = labHeight;
}



// Configure - handle Tcl configuration of the widget from the
// given arguments
//
int
PDGraph::DataW::Configure( Tcl_Interp* interp,
                            int argc, TCLCONST char* argv[],
                            int flags )
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


// UpdateConfiguration - updates the current location of datapoints
// based on the current PDGraph widget configuration
//
void
PDGraph::DataW::UpdateConfiguration( void )
{
    XRectangle dataRect;
	int tkwidth = Tk_Width( tkwin );
	int tkheight = Tk_Height( tkwin );

    // determine where we will draw
    dataRect.x = 0;
    dataRect.y = valLabelHeight;
    dataRect.width = tkwidth - 1;
    dataRect.height = tkheight - valLabelHeight - 1;

    // compute new X coordinates for all curves
    const pdvector<PDGraph::Curve*>& curves = g->GetCurves();
    unsigned int idx;

    for( idx = 0; idx < curves.size(); idx++ )
    {
        if( curves[idx] != NULL )
        {
            curves[idx]->ComputeXPoints( dataRect,
                                         g->GetVisScopeInfo().start, 
                                         g->GetVisScopeInfo().focus,
                                         g->GetHistogramInfo().nBuckets );
        }
    }

    // redisplay the curve data in its new location
    RequestRedraw();
}


// HandleNewData - respond to observed new data for the given curve
//
void
PDGraph::DataW::HandleNewData( PDGraph::Curve* curve,
                               unsigned int firstSampleIdx,
                               unsigned int nSamples )
{
    XRectangle dataRect;
	int tkwidth = Tk_Width( tkwin );
	int tkheight = Tk_Height( tkwin );

    // determine where we can draw
    dataRect.x = 0;
    dataRect.y = valLabelHeight;
    dataRect.width = tkwidth  - 1;
    dataRect.height = tkheight - valLabelHeight - 1;

    // compute GUI points for the indicated sample data only
    curve->ComputeXPoints( firstSampleIdx,
                            nSamples,
                            dataRect,
                            g->GetVisScopeInfo().start,
                            g->GetVisScopeInfo().focus,
                            g->GetHistogramInfo().nBuckets );

    // draw the new data
    //
    // Note that this method is called when the PDGraph
    // receives new data points.  This call can be due to "live"
    // data, but also due to a fold.  In the case of live data,
    // we just want to draw the new data points.  In the case of
    // a fold, we need to redraw our display.
    if( (firstSampleIdx == 0) || !Tk_IsMapped(tkwin) )
    {
        // it is a fold - schedule a redraw
        RequestRedraw();
    }
    else
    {
        // it is just new data - draw it directly
        curve->Draw( Tk_Display(tkwin),
					 Tk_WindowId(tkwin),
					 firstSampleIdx, nSamples );
    }
}



// Draw - Draw a representation of our data to our window
//
void
PDGraph::DataW::Draw( void )
{
    // reset our pending redraw flag, so
    // further requests aren't ignored in case
    // we aren't yet mapped
    redrawPending = false;

    // avoid attempting to draw if we are not yet mapped
    if (!Tk_IsMapped(tkwin))
    {
        return;
    }


    Pixmap pm = None;
    Drawable d;


    // draw into offscreen pixmap if desired
    if( g->IsDoubleBuffered() )
    {
        pm = Tk_GetPixmap( Tk_Display(tkwin),
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

    // draw a representation of our data...

    // frame the data area
    XDrawRectangle( Tk_Display(tkwin), pm,
                    g->GetDrawGC(),
                    0, valLabelHeight,
                    Tk_Width(tkwin) - 1, Tk_Height(tkwin) - valLabelHeight - 1);

    // draw the curve data
    const pdvector<PDGraph::Curve*>& curves = g->GetCurves();
    unsigned int idx;

    for( idx = 0; idx < curves.size(); idx++ )
    {
        Curve* c = curves[idx];

        if( (c != NULL) && c->isVisible && (c->nPoints > 0) )
        {
            // draw the curve
            c->Draw( Tk_Display(tkwin), pm );
        }
    }

    // copy from offscreen pixmap if needed
    if( g->IsDoubleBuffered() )
    {
        XCopyArea( Tk_Display(tkwin), pm,
            Tk_WindowId(tkwin),
            g->GetDrawGC(),
            0, 0,
            Tk_Width(tkwin), Tk_Height(tkwin),
            0, 0);
        Tk_FreePixmap( Tk_Display(tkwin), pm);
    }
}



// HandleEvent - respond to the given X event
//
void
PDGraph::DataW::HandleEvent( XEvent* ev )
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
            UpdateConfiguration();
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
            Tcl_EventuallyFree((ClientData)this, DataW::DestroyCB);
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
PDGraph::DataW::HandleCommand( Tcl_Interp* interp,
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
PDGraph::DataW::HandleInstanceCommandDeleted( void ) 
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

//---------------------------------------------------------------------------
// PDGraph::DataW callbacks
//---------------------------------------------------------------------------

int
PDGraph::DataW::InstanceCmdCB( ClientData cd, Tcl_Interp* interp,
                                int argc, TCLCONST char* argv[] )
{
    DataW* dw = (DataW*)cd;
    return dw->HandleCommand( interp, argc, argv );
}


void
PDGraph::DataW::DrawCB(ClientData clientData)
{
    DataW* dw = (DataW*)clientData;
    dw->Draw();
}


void
PDGraph::DataW::EventCB( ClientData cd, XEvent* eventPtr )
{
    DataW* dw = (DataW*)cd;
    dw->HandleEvent( eventPtr );
}


void
PDGraph::DataW::DestroyCB( char* cd )
{
    DataW* dw = (DataW*)cd;
    delete dw;
}

void
PDGraph::DataW::InstanceCmdDeletedCB( ClientData cd )
{
    DataW* dw = (DataW*)cd;
    dw->HandleInstanceCommandDeleted();
}

