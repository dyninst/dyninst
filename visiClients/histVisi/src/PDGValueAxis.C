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
// PDGValueAxis.C
//
// Definition of PDGraph::ValueAxisW class.
// A PDGraph object is the C++ portion of a custom Tcl widget to
// support the drawing of time-based histograms.  The ValueAxisW nested
// class is the C++ portion of the value axis widget subwindow of a PDGraph 
// megawidget.
//
//---------------------------------------------------------------------------
// $Id: PDGValueAxis.C,v 1.9 2004/03/23 01:12:48 eli Exp $
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
#include "paradyn/src/UIthread/minmax.h"


const int PDGraph::ValueAxisW::tickLen        = 4;
const int PDGraph::ValueAxisW::tickPad        = 2;


Tk_ConfigSpec PDGraph::ValueAxisW::configSpecs[] =
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



// InstallClassCommand - installs the class Tcl command into the given 
// interpreter.
//
int
PDGraph::ValueAxisW::InstallClassCommand( Tcl_Interp* interp )
{
    int ret = TCL_OK;

    Tcl_Command cmd = Tcl_CreateCommand( interp,
            "pdgraph_valueaxis",
            ValueAxisW::ClassCmdCB,
            Tk_MainWindow( interp ),
            NULL );
    if( cmd == NULL )
    {
        ret = TCL_ERROR;
    }
    return ret;
}



int
PDGraph::ValueAxisW::ClassCmdCB( ClientData cd,
                                    Tcl_Interp* interp,
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
    ValueAxisW* va = new ValueAxisW;
    if( va->InitTclTk( interp, mwin, argc, argv ) != TCL_OK )
    {
        delete va;
        return TCL_ERROR;
    }

    // set interpreter result to pathname associated with main window
    Tcl_SetObjResult( interp, Tcl_NewStringObj( Tk_PathName( va->tkwin ), -1 ));

    return TCL_OK;
}


// InitTclTk - initialize the Tcl/Tk members for the widget
//
int
PDGraph::ValueAxisW::InitTclTk( Tcl_Interp* interp,
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
    Tk_SetClass( tkwin, "Pdgraph_valueaxis" );

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
PDGraph::ValueAxisW::InitCPP( PDGraph* pdgraph, unsigned int taHeight )
{
    g = pdgraph;
    timeAxisHeight = taHeight;
}



// Configure - handle Tcl configuration of the widget from the
// given arguments
//
int
PDGraph::ValueAxisW::Configure( Tcl_Interp* interp,
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
PDGraph::ValueAxisW::Draw( void )
{
    redrawPending = false;

    // avoid attempting to draw if we aren't yet mapped
    if (!Tk_IsMapped(tkwin))
    {
        return;
    }

    Pixmap pm = None;
    Drawable d;

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

    // draw axis for each group
    const pdvector<PDGraph::Group*>& groups = g->GetGroups();
    unsigned int xRight = Tk_Width(tkwin);   // right edge of value axis window
    unsigned int i;
    for( i = 0; i < groups.size(); i++ )
    {
        if( groups[i]->IsVisible() )
        {
            // determine location of value axis
            XRectangle vaRect;
            unsigned int axisWidth = DetermineWidth( groups[i]->axis );
            
            vaRect.x = xRight - axisWidth;
            vaRect.y = 0;
            vaRect.width = xRight;
            vaRect.height = Tk_Height(tkwin);
        
            // draw the value axis
            DrawAxis( d, groups[i]->axis, vaRect );

            xRight -= axisWidth;
        }
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





// DetermineWidth - computes width of value axis; width
// is computed using the current set of visible curves
//
unsigned int
PDGraph::ValueAxisW::DetermineWidth( const ValueAxis* axis ) const
{
    std::ostringstream vstr;

    // ...first find width of units string...
    int unitsStrLen = strlen( axis->title.c_str() );
    int unitsStrWidth = Tk_TextWidth( g->GetFont(),
                                        axis->title.c_str(),
                                        unitsStrLen );

    // ...then width of label strings...
    vstr << axis->maxValue << std::ends;
    int valStrLen = strlen( vstr.str().c_str() );
    int valStrWidth = Tk_TextWidth( g->GetFont(), vstr.str().c_str(), valStrLen );

    // ...and find the longer of the two.
    unsigned int axisWidth = tickLen + 16 + max( unitsStrWidth, valStrWidth );
    vstr.seekp( 0 );

    return axisWidth;
}



// DrawAxis - output a representation of the given ValueAxis to
// our window at the location indicated by rect
//
void
PDGraph::ValueAxisW::DrawAxis( Drawable d, ValueAxis* axis, XRectangle& rect )
{
    std::ostringstream vstr;
    unsigned int labelHeight = DetermineLabelHeight();
    unsigned int i;


    // determine our width
    unsigned int axisWidth = DetermineWidth( axis );
    unsigned int unitsStrLen = strlen( axis->title.c_str() );
    unsigned int unitsStrWidth = Tk_TextWidth( g->GetFont(), axis->title.c_str(), unitsStrLen );

    // draw units string
    Tk_DrawChars( Tk_Display(tkwin), d,
        g->GetDrawGC(),
        g->GetFont(),
        axis->title.c_str(), unitsStrLen,
        rect.x + axisWidth - unitsStrWidth - tickLen - 4,
        rect.y + labelHeight - (g->GetFontMetrics().linespace) );

    // draw axis line, ticks, and labels
    XDrawLine( Tk_Display(tkwin), d,
        g->GetDrawGC(),
        rect.x + axisWidth, rect.y + labelHeight,
        rect.x + axisWidth, rect.y + rect.height - timeAxisHeight - 1);

    for( i = 0; i <= axis->nIntervals; i++ )
    {
        // draw tick
        double userFlNum = i * (1.0 / axis->nIntervals);
	int heightNum = rect.height - labelHeight - timeAxisHeight;
        double yTickD = rect.y + rect.height - timeAxisHeight - 
	                (userFlNum * heightNum);
	int yTickI = static_cast<int>(yTickD);
	// the compilers past 2.95.3 are having problems with converting 
	// directly from a double to a short
	short yTick = static_cast<short>(yTickI);

        XDrawLine( Tk_Display(tkwin), d,
            g->GetDrawGC(),
            rect.x + axisWidth - tickLen, yTick,
            rect.x + axisWidth, yTick );
        
        // draw tick label
        double vTick = axis->minValue + i * axis->intervalWidth;
        vstr << vTick << std::ends;
        int labStrLen = strlen( vstr.str().c_str() );
        int labWidth = Tk_TextWidth( g->GetFont(), vstr.str().c_str(), labStrLen );

        Tk_DrawChars( Tk_Display(tkwin), d,
            g->GetDrawGC(),
            g->GetFont(),
            vstr.str().c_str(), labStrLen,
            rect.x + axisWidth - tickLen - labWidth - 4,
            yTick + (g->GetFontMetrics().linespace / 2) );

        vstr.seekp( 0 );
    }
}


// HandleConfigureNotification - respond to a notification that
// the widget's configuration has changed
//
void
PDGraph::ValueAxisW::HandleConfigureNotification( void )
{
    if( Tk_IsMapped(tkwin) )
    {
        const pdvector<PDGraph::Group*>& groups = g->GetGroups();
        unsigned int i = 0;

        for( i = 0; i < groups.size(); i++ )
        {
            // determine the maxmimum number of ticks we
            // can support in the given axis height
            //
            // Note that we go to great lengths to be sure
            // that we do our "max ticks" calculation as
            // signed integer arithmetic, so we can recognize
            // the case when the window is too short
            int nTicksMax = (Tk_Height(tkwin) - 
                    ((int)timeAxisHeight) - 
                    ((int)DetermineLabelHeight())) / g->GetFontMetrics().linespace;
            if( nTicksMax <= 0 )
            {
                // this could happen if we've not yet mapped the
                // value axis window, of ir the window has become
                // too short
                //
                // we still want ot compute an interval layout,
                // so we force teh maxnumber of ticks to a 
                // "reasonable" value
                nTicksMax = 5;
            }
            groups[i]->axis->ComputeIntervals( nTicksMax );
        }
    }

    RequestRedraw();
}


// HandleEvent - respond to the given X event
//
void
PDGraph::ValueAxisW::HandleEvent( XEvent* ev )
{
    switch( ev->type )
    {
    case Expose:
        // schedule a redraw if needed
        RequestRedraw();
        break;

    case ConfigureNotify:
        HandleConfigureNotification();
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
            Tcl_EventuallyFree((ClientData)this, ValueAxisW::DestroyCB);
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
PDGraph::ValueAxisW::HandleCommand( Tcl_Interp* interp,
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
            Tcl_SetObjResult( interp, Tcl_NewStringObj( estr.str().c_str(), -1 ));

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
PDGraph::ValueAxisW::HandleInstanceCommandDeleted( void ) 
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
// PDGraph::ValueAxisW callbacks
//---------------------------------------------------------------------------

int
PDGraph::ValueAxisW::InstanceCmdCB( ClientData cd, Tcl_Interp* interp,
                                    int argc, TCLCONST char* argv[] )
{
    ValueAxisW* va = (ValueAxisW*)cd;
    return va->HandleCommand( interp, argc, argv );
}


void
PDGraph::ValueAxisW::DrawCB(ClientData clientData)
{
    ValueAxisW* va = (ValueAxisW*)clientData;
    va->Draw();
}


void
PDGraph::ValueAxisW::EventCB( ClientData cd, XEvent* eventPtr )
{
    ValueAxisW* va = (ValueAxisW*)cd;
    va->HandleEvent( eventPtr );
}


void
PDGraph::ValueAxisW::DestroyCB( char* cd )
{
    ValueAxisW* va = (ValueAxisW*)cd;
    delete va;
}



void
PDGraph::ValueAxisW::InstanceCmdDeletedCB( ClientData cd )
{
    ValueAxisW* va = (ValueAxisW*)cd;
    va->HandleInstanceCommandDeleted();
}


//---------------------------------------------------------------------------
// explicit template instantiations needed by this class
//---------------------------------------------------------------------------
#include "paradyn/src/UIthread/minmax.C"

template  int max( const int, const int );

