/*
 * Copyright (c) 1996-2000 Barton P. Miller
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
// PDGraph.C
//
// Definition of PDGraph class.
// A PDGraph object is the C++ portion of a custom Tcl widget to
// support the drawing of time-based histograms.
//
// Note that the implementation of the PDGraph class is separated into
// several files.  This file contains the implementation of the PDGraph 
// class itself, along with the implementation of the application domain 
// classes nested in the PDGraph object.  The custom Tcl subwidgets are
// implemented in other files:
//
//   Class                File
//   -----                ----
//   PDGraph::TimeAxisW   PDGTimeAxis.C
//   PDGraph::ValueAxisW  PDGValueAxis.C
//   PDGraph::DataW       PDGData.C
//
//---------------------------------------------------------------------------
// $Id: PDGraph.C,v 1.16 2002/02/15 18:35:24 pcroth Exp $
//---------------------------------------------------------------------------
#include <limits.h>
#include <iostream.h>
#if defined(i386_unknown_nt4_0)
#  include <strstrea.h>
#else
#  include <strstream.h>
#endif
#include <math.h>

#include "common/h/String.h"
#include <assert.h>
#include <string.h>
#include "tcl.h"
#include "tk.h"

#include "common/h/Dictionary.h"
#include "PDGraph.h"
#include "paradyn/src/UIthread/minmax.h"
#include "pdutil/h/makenan.h"

#define ZOOM_THUMB_SIZE             (0.2)
#define ZOOM_UNIT_SIZE              (0.05)
#define PAN_UNIT_SIZE               (0.05)



//---------------------------------------------------------------------------
// utility functions for use in this file
//---------------------------------------------------------------------------

// FocusToPosition
// PositionToFocus
//
// Converts between a Paradyn focus (the amount of 
// the Histogram shown, as a fraction) and the Tcl scrollbar
// thumb position (the fraction of the distance from the top of
// the scrollbar to the bottom)
inline
static 
double FocusToPosition( double f )
{
    return f - ZOOM_THUMB_SIZE;
}


inline
static
double  PositionToFocus( double p )
{
    return p + ZOOM_THUMB_SIZE;
}




//
// We need some mechanism for accessing the instance data for a widget
// when all we are given is a Tk_Window handle.  We could use an implementation
// secret (that one of the members of the Tk_Window structure is a piece
// of client data; most built-in widgets use this client data to point
// to their instance data).  Many other GUI environments (e.g., 
// Motif and Windows) use this method to associate the instance
// data with the GUI representation of a widget.  If there is a documented
// mechanism for doing this in Tk, it is non-obvious.
//
// We chose to use an external mapping table because (a) the need
// to lookup C++ objects from Tk_Windows occurs infrequently, especially
// if we retain the information in the objects that need it, and (b)
// we had a mapping class available in the dictionary_hash class.
//
dictionary_hash<Tk_Window,ClientData> PDGraph::winInstDataMap( HashTkWindow );




//---------------------------------------------------------------------------
// PDGraph data
//---------------------------------------------------------------------------

#if !defined(i386_unknown_nt4_0)
#  define    DEF_PDGRAPH_FOREGROUND    "Black"
#  define    DEF_PDGRAPH_BACKGROUND    "#d9d9d9"
#  define    DEF_PDGRAPH_FONT        "Helvetica 12"
#else // !defined(i386_unknown_nt4_0)
#  define    DEF_PDGRAPH_FOREGROUND    "SystemButtonText"
#  define    DEF_PDGRAPH_BACKGROUND    "SystemButtonFace"
#  define    DEF_PDGRAPH_FONT        "{MS Sans Serif} 8"
#endif // !defined(i386_unknown_nt4_0)


#define DEF_LINE_COLORS             \
"blue yellow red turquoise1 DarkViolet magenta {medium sea green} orange2"

#define DEF_LINE_PATTERNS           \
    "solid gray50 gray75 gray25"

// Tk options supported by the PDGraph widget
Tk_ConfigSpec PDGraph::configSpecs[] =
{
    {
        TK_CONFIG_BORDER,
        "-background",
        "background",
        "Background",
        DEF_PDGRAPH_BACKGROUND,
        Tk_Offset(PDGraph, bgBorder),
        TK_CONFIG_COLOR_ONLY,
        NULL
    },
    {
        TK_CONFIG_BORDER,
        "-background",
        "background",
        "Background",
        "white",
        Tk_Offset(PDGraph, bgBorder),
        TK_CONFIG_MONO_ONLY,
        NULL
    },
    {
        TK_CONFIG_SYNONYM,
        "-bd",
        "borderWidth",
        NULL,
        NULL,
        0, 
        0,
        NULL
    },
    {
        TK_CONFIG_SYNONYM, 
        "-bg", 
        "background",
        NULL,
        NULL, 
        0, 
        0,
        NULL
    },
    {
        TK_CONFIG_PIXELS, 
        "-borderwidth", 
        "borderWidth", 
        "BorderWidth",
        "2", 
        Tk_Offset(PDGraph, borderWidth), 
        0,
        NULL
    },
    {
        TK_CONFIG_SYNONYM, 
        "-fg", 
        "foreground",  
        NULL,
        NULL, 
        0, 
        0,
        NULL
    },
    {
        TK_CONFIG_BORDER, 
        "-foreground", 
        "foreground", 
        "Foreground",
        DEF_PDGRAPH_FOREGROUND,
        Tk_Offset(PDGraph, fgBorder), 
        TK_CONFIG_COLOR_ONLY,
        NULL
    },
    {
        TK_CONFIG_BORDER, 
        "-foreground", 
        "foreground", 
        "Foreground",
        "black", 
        Tk_Offset(PDGraph, fgBorder), 
        TK_CONFIG_MONO_ONLY,
        NULL
    },
    {
        TK_CONFIG_RELIEF, 
        "-relief", 
        "relief", 
        "Relief",
        "raised", 
        Tk_Offset(PDGraph, relief), 
        0,
        NULL
    },
    {
        TK_CONFIG_FONT,
        "-font",
        "font",
        "Font",
        DEF_PDGRAPH_FONT,
        Tk_Offset( PDGraph, font ),
        0,
        NULL
    },
    {
        TK_CONFIG_STRING,
        "-lineColors",
        "lineColors",
        "LineColors",
        DEF_LINE_COLORS,
        Tk_Offset( PDGraph, lineColors ),
        0,
        NULL
    },
    {
        TK_CONFIG_STRING,
        "-linePatterns",
        "linePatterns",
        "LinePatterns",
        DEF_LINE_PATTERNS,
        Tk_Offset( PDGraph, linePatterns ),
        0,
        NULL
    },
    {
        TK_CONFIG_BOOLEAN, 
        "-dbl", 
        "doubleBuffer", 
        "DoubleBuffer",
        "true", 
        Tk_Offset(PDGraph, doubleBuffer), 
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




//---------------------------------------------------------------------------
// PDGraph methods
//---------------------------------------------------------------------------

PDGraph::PDGraph( void )
 :  timeAxis( NULL ),
    valAxis( NULL ),
    dataw( NULL ),
    tkwin( NULL ),
    tkdisplay( NULL ),
    legendWin( NULL ),
    interp( NULL ),
    widgetCmd( NULL ),
    borderWidth( 0 ),
    bgBorder( NULL ),
    fgBorder( NULL ),
    relief( TK_RELIEF_FLAT ),
    gc( None ),
    font( NULL ),
    lineColors( NULL ),
    linePatterns( NULL ),
    doubleBuffer( 1 ),        // true
    redrawPending( false )
{
}



PDGraph::~PDGraph( void )
{
    if( tkdisplay != NULL )
    {
        Tk_FreeOptions(configSpecs, (char *)this, tkdisplay, 0);
        if(gc != None)
        {
            Tk_FreeGC( tkdisplay, gc);
        }

        Curve::ReleaseLineSpecs( tkdisplay );
    }

    if( lineColors != NULL )
    {
        Tcl_Free( lineColors );
        lineColors = NULL;
    }

    if( linePatterns != NULL )
    {
        Tcl_Free( linePatterns );
        linePatterns = NULL;
    }
}


// InitTclTk - initialize the Tcl/Tk members for the widget
//
int
PDGraph::InitTclTk( Tcl_Interp* interp, Tk_Window mwin, int argc, char* argv[] )
{
    ClientData cd;
    ostrstream wpathstr;
    bool found;


    // save the given interpreter
    this->interp = interp;

    
    // build a frame to enclose our widgets
    wpathstr << argv[1] << ends;
    Tk_Window tmpMainW = Tk_CreateWindowFromPath( interp,
                                                    mwin,
                                                    wpathstr.str(),
                                                    NULL );
    wpathstr.rdbuf()->freeze( 0 );
    wpathstr.seekp( 0 );
    if( tmpMainW == NULL )
    {
        return TCL_ERROR;
    }
    Tk_SetClass( tmpMainW, "Pdgraph" );


    // build and arrange subwidgets -
    // call out to Tcl proc to handle this
    ostrstream cmdstr;
    
    cmdstr << "::PDGraph::init " << argv[1] << ends;
    Tcl_Obj* cmdobj = Tcl_NewStringObj( cmdstr.str(), -1 );
    cmdstr.rdbuf()->freeze( 0 );
    cmdstr.seekp( 0 );
    if( Tcl_EvalObj( interp, cmdobj ) != TCL_OK )
    {
        Tk_DestroyWindow( tmpMainW );
        return TCL_ERROR;
    }

    // obtain access to our subwidgets
    wpathstr << argv[1] << ".legend" << ends;
    Tk_Window tmpLegendW = Tk_NameToWindow( interp, wpathstr.str(), mwin );
    wpathstr.rdbuf()->freeze( 0 );
    wpathstr.seekp( 0 );
    if( tmpLegendW == NULL )
    {
        Tk_DestroyWindow( tmpMainW );
        return TCL_ERROR;
    }

    wpathstr << argv[1] << ".valaxis" << ends;
    Tk_Window valAxisWin = Tk_NameToWindow( interp, wpathstr.str(), mwin );
    wpathstr.rdbuf()->freeze( 0 );
    wpathstr.seekp( 0 );
    if( valAxisWin == NULL )
    {
        Tk_DestroyWindow( tmpMainW );
        return TCL_ERROR;
    }
    found = winInstDataMap.find( valAxisWin, cd );
    assert( found );
    valAxis = (ValueAxisW*)cd;

    wpathstr << argv[1] << ".timeaxis" << ends;
    Tk_Window timeAxisWin = Tk_NameToWindow( interp, wpathstr.str(), mwin );
    wpathstr.rdbuf()->freeze( 0 );
    wpathstr.seekp( 0 );
    if( timeAxisWin == NULL )
    {
        Tk_DestroyWindow( tmpMainW );
        return TCL_ERROR;
    }
    found = winInstDataMap.find( timeAxisWin, cd );
    assert( found );
    timeAxis = (TimeAxisW*)cd;

    wpathstr << argv[1] << ".data" << ends;
    Tk_Window dataWin = Tk_NameToWindow( interp, wpathstr.str(), mwin );
    wpathstr.rdbuf()->freeze( 0 );
    wpathstr.seekp( 0 );
    if( dataWin == NULL )
    {
        Tk_DestroyWindow( tmpMainW );
        return TCL_ERROR;
    }
    found = winInstDataMap.find( dataWin, cd );
    assert( found );
    dataw = (DataW*)cd;

    // now that we've successfully created our subwindows,
    // keep hold of them
    tkwin = tmpMainW;
    tkdisplay = Tk_Display(tkwin);
    legendWin = tmpLegendW;
   
    
    // register an instance command with the interpreter
    // so our widget can handle its own commands
    widgetCmd = Tcl_CreateCommand(interp,
        Tk_PathName(tmpMainW),
        PDGraph::InstanceCmdCB,
        (ClientData)this,
        PDGraph::InstanceCmdDeletedCB);

    Tk_CreateEventHandler(tkwin,
        ExposureMask|StructureNotifyMask,
        PDGraph::EventCB,
        (ClientData)this);

    // handle any configuration options given
    if( Configure( interp, argc - 2, argv + 2, 0 ) != TCL_OK )
    {
        Tk_DestroyWindow(tkwin);
        return TCL_ERROR;
    }

    // ensure we have graphics contexts to use
    if( Curve::InitLineSpecs( interp, mwin, lineColors, linePatterns, fgBorder ) != TCL_OK )
    {
        Tk_DestroyWindow( tkwin );
        return TCL_ERROR;
    }

    // intialize the C++ objects for our subwidgets
    timeAxis->InitCPP( this );
    valAxis->InitCPP( this, timeAxis->DetermineHeight() + 1);
    dataw->InitCPP( this, valAxis->DetermineLabelHeight() );

    // register our association of Tk_Window and instance data
    winInstDataMap[tkwin] = (ClientData)this;


    cmdstr << "::PDGraph::Legend::init_font " << argv[1] << ends;
    cmdobj = Tcl_NewStringObj( cmdstr.str(), -1 );
    cmdstr.rdbuf()->freeze( 0 );
    cmdstr.seekp( 0 );
    if( Tcl_EvalObj( interp, cmdobj ) != TCL_OK )
    {
        Tk_DestroyWindow( tmpMainW );
        return TCL_ERROR;
    }

    return TCL_OK;
}




// ClassCmdCB - implementation of "pdgraph" Tcl command
//
int
PDGraph::ClassCmdCB( ClientData cd, Tcl_Interp* interp, int argc, char* argv[] )
{
    Tk_Window mwin = (Tk_Window)cd;


    // validate command argument count
    if (argc < 2)
    {
        ostrstream estr;

        estr << "wrong # args: should be \""
            << argv[0]
            << " pathName ?options?\""
            << ends;
        Tcl_SetObjResult( interp, Tcl_NewStringObj( estr.str(), -1 ));
        estr.rdbuf()->freeze( 0 );

        return TCL_ERROR;
    }

    // obtain an object to represent the widget
    PDGraph* pGraph = new PDGraph;

    // initialize the object
    if( pGraph->InitTclTk( interp, mwin, argc, argv ) != TCL_OK )
    {
        delete pGraph;
        return TCL_ERROR;
    }

    // set interpreter result to pathname associated with main window
    Tcl_SetObjResult( interp,
                        Tcl_NewStringObj( Tk_PathName( pGraph->tkwin ), -1 ));
    
    return TCL_OK;
}



// ZoomTo - zoom the graph to the indicate position
//
int
PDGraph::ZoomTo( double position )
{
    int ret = TCL_OK;


    assert( (position >= 0.0) && (position <= 1.0 - ZOOM_THUMB_SIZE) );

    // determine new focus and start values
    double newfocus = PositionToFocus( position );
    double newstart = visScopeInfo.start + (visScopeInfo.focus - newfocus) / 2;

    // guard against going outside the bounds for Tk's scrollbar positions
    if( newstart < 0.0 )
    {
        newstart = 0.0;
    }
    if( newstart + newfocus > 1.0 )
    {
        newfocus = (1.0 - newstart);
    }

    assert( (newstart >= 0.0) && (newstart <= 1.0) );
    assert( (newstart + newfocus) <= 1.0 );
    visScopeInfo.focus = newfocus;
    visScopeInfo.start = newstart;

    // update the subwindows that need to know about
    // the change - (in this case, the data window
    // only, since the time axis already knows about
    // the change)
    dataw->UpdateConfiguration();

    // make sure that the display gets updated
    RequestRedraw();
    timeAxis->RequestRedraw();

    // set the interpreter's result so that the Tcl script can update the
    // scrollbar positions accordingly
    ostrstream ostr;

    // make the result hold the zoom scrollbar position, followed by the
    // pan scrollbar position
    ostr << position << " " << position + ZOOM_THUMB_SIZE << " " 
        << newstart << " " << newstart + newfocus               
        << ends;
    Tcl_SetObjResult( interp, Tcl_NewStringObj( ostr.str(), -1 ));
    ostr.rdbuf()->freeze( 0 );

    return ret;
}


// PanTo - pan the graph to the indicated position
//
int
PDGraph::PanTo( double position )
{
    int ret = TCL_OK;


    // guard against going outside the bounds for Tk's scrollbar positions
    if( position < 0.0 )
    {
        position = 0.0;
    }
    if( (position + visScopeInfo.focus) > 1.0 )
    {
        visScopeInfo.focus = (1.0 - position);
    }

    // update the focus and start to reflect the new visible configuration
    visScopeInfo.start = position;

    // update the subwindows that need to know about
    // the change - (in this case, the data window
    // only, since the time axis already knows about
    // the change)
    dataw->UpdateConfiguration();

    // ...post an eventual redraw request
    RequestRedraw();
    timeAxis->RequestRedraw();

    // return our positions, so our container can update its scrollbars
    ostrstream resstr;
    resstr << position << " " << position + visScopeInfo.focus << ends;
    Tcl_SetObjResult( interp, Tcl_NewStringObj( resstr.str(), -1 ));
    resstr.rdbuf()->freeze( 0 );

    return ret;
}


// HandleGetSelectedCommand - respond to the get_selected Tcl command
// on a pdgraph instance.  Builds a list of curve IDs for selected items.
//
int
PDGraph::HandleGetSelectedCommand( int argc, char* argv[] )
{
    ostrstream rstr;        // result stream
    int ret = TCL_OK;


    if( argc == 2 )
    {
        // determine the selected curves
        // (ask legend to give us a list of the selection)
        ostrstream cmdstr;

        cmdstr << "::PDGraph::Legend::get_selected "
            << Tk_PathName( legendWin )
            << ends;
        ret = Tcl_EvalObj( interp, Tcl_NewStringObj( cmdstr.str(), -1 ));
        cmdstr.rdbuf()->freeze( 0 );
    }
    else
    {
        // handle cases where we couldn't parse the command
        // provide some usage information
        rstr << "wrong # getselected args: should be '"
            << argv[0]
            << " getselected'"
            << ends;

        Tcl_SetObjResult( interp, Tcl_NewStringObj( rstr.str(), -1 ));
        rstr.rdbuf()->freeze( 0 );

        ret = TCL_ERROR;
    }

    return ret;
}



// HandleSmoothCommand - Responds to the smooth and unsmooth commands
//
int
PDGraph::HandleSmoothCommand( int argc, char* argv[], bool smooth )
{
    ostrstream rstr;        // result stream
    int ret = TCL_OK;
    vector<Group*>    checkGroups;    // groups that need max value rechecks


    if( argc == 3 )
    {
        // extract the items out of the list
        int objc;
        Tcl_Obj** objv;

        ret = Tcl_ListObjGetElements( interp,
                                        Tcl_NewStringObj( argv[2], -1 ),
                                        &objc, &objv );
        if( ret == TCL_OK )
        {
            int i;

            // for each curve mentioned in the list
            for( i = 0; i < objc; i++ )
            {
                long lval;

                //convert the item (a string) to a number (a curveID)
                ret = Tcl_GetLongFromObj( interp, objv[i], &lval );
                if( ret == TCL_OK )
                {
                    // make the change to the curve
                    assert( curves[lval] != NULL );
                    if( smooth )
                    {
                        curves[lval]->Smooth();
                    }
                    else
                    {
                        curves[lval]->Unsmooth();
                    }

                    // add curve's group to list of groups whose
                    // max values need to be rechecked
                    checkGroups += curves[lval]->group;

                    // update the legend with the curve's new name
                    ostrstream cmdstr;

                    cmdstr << "::PDGraph::Legend::update_item_name "
                        << Tk_PathName( legendWin ) << " "
                        << lval << " "
                        << '\"' << curves[lval]->GetName() << '\"'
                        << ends;
                    ret = Tcl_EvalObj( interp,
                                        Tcl_NewStringObj( cmdstr.str(), -1 ) );
                    cmdstr.rdbuf()->freeze( 0 );
                    if( ret != TCL_OK )
                    {
                        break;
                    }
                }
                else
                {
                    break;
                }
            }

            // determine new axis max values
            //
            // (By smoothing or unsmoothing curves, we may have updated
            // the max value shown on a curve.  Unfortunately, we can't
            // just consider the curves we changed, since the smoothing/
            // unsmoothing operation may have caused a data point for
            // another curve to become the new maximum value.  So we check
            // each curve associated with each axis that had a curve that
            // was smoothed or unsmoothed.)
            unsigned int g;
            for( g = 0; g < checkGroups.size(); g++ )
            {
                Group* grp = checkGroups[g];
                assert( grp != NULL );

                double newMax = 0.0;
                unsigned int j;

                // check for new max value for curves of this group
                for( j = 0; j < grp->curves.size(); j++ )
                {
                    if( grp->curves[j]->GetMaxActiveValue() > newMax )
                    {
                        newMax = grp->curves[j]->GetMaxActiveValue();
                    }
                }

                // now update with the new max value
                UpdateAxisMaxValue( grp->axis, newMax );
            }
        }

        // update our display based on our new configuration
        dataw->UpdateConfiguration();
    }
    else
    {
        // handle cases where we couldn't parse the command
        rstr << "wrong # "
            << (smooth ? "smooth " : "unsmooth ")
            << "args: should be '"
            << argv[0]
            << " remove <cid_list>'"
            << ends;

        Tcl_SetObjResult( interp, Tcl_NewStringObj( rstr.str(), -1 ) );
        rstr.rdbuf()->freeze( 0 );

        ret = TCL_ERROR;
    }
    return ret;
}



// HandleShowCommand - respond to the show and hide commands
//
int
PDGraph::HandleShowCommand( int argc, char* argv[], bool show )
{
    ostrstream rstr;        // result stream
    int ret = TCL_OK;

    if( argc == 3 )
    {
        // extract the items out of the list
        int objc;
        Tcl_Obj** objv;

        ret = Tcl_ListObjGetElements( interp,
                                        Tcl_NewStringObj( argv[2], -1 ),
                                        &objc, &objv );
        if( ret == TCL_OK )
        {
            int i;

            // for each curve mentioned in the list
            for( i = 0; i < objc; i++ )
            {
                long lval;

                //convert the item (a string) to a number (a curveID)
                ret = Tcl_GetLongFromObj( interp, objv[i], &lval );
                if( ret == TCL_OK )
                {
                    assert( curves[lval] != NULL );

                    if( show )
                    {
                        curves[lval]->Show();
                    }
                    else
                    {
                        curves[lval]->Hide();
                    }
                }
                else
                {
                    break;
                }
            }

            // update our display
            UpdateGeometry();
            dataw->UpdateConfiguration();
        }
    }
    else
    {
        // handle cases where we couldn't parse the command
        rstr << "wrong # "
            << (show ? "show " : "hide ")
            << "args: should be '"
            << argv[0]
            << " remove <cid_list>'"
            << ends;

        Tcl_SetObjResult( interp, Tcl_NewStringObj( rstr.str(), -1 ) );
        rstr.rdbuf()->freeze( 0 );

        ret = TCL_ERROR;
    }

    return ret;

}



// HandleRemoveCommand - respond to the remove command by removing
// the indicated curves
//
int
PDGraph::HandleRemoveCommand( int argc, char* argv[] )
{
    ostrstream rstr;        // result stream
    int ret = TCL_OK;

    if( argc == 3 )
    {
        // extract the items out of the list
        int objc;
        Tcl_Obj** objv;

        ret = Tcl_ListObjGetElements( interp,
                                        Tcl_NewStringObj( argv[2], -1 ),
                                        &objc, &objv );
        if( ret == TCL_OK )
        {
            int i;

            // for each curve mentioned in the list
            for( i = 0; i < objc; i++ )
            {
                long cid;

                //convert the item (a string) to a number (a curveID)
                ret = Tcl_GetLongFromObj( interp, objv[i], &cid );
                if( ret == TCL_OK )
                {
                    ostrstream cmdstr;

                    cmdstr << "::PDGraph::Legend::remove_item "
                        << Tk_PathName( legendWin )
                        << " "
                        << cid
                        << ends;
                    ret = Tcl_EvalObj( interp,
                                        Tcl_NewStringObj( cmdstr.str(), -1 ));
                    cmdstr.rdbuf()->freeze( 0 );
                    if( ret == TCL_OK )
                    {
                        // remove the curve
                        if( curves[cid] != NULL )
                        {
                            Remove( curves[cid] );
                            curves[cid] = NULL;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
            }

            // update our display
            UpdateGeometry();
            dataw->UpdateConfiguration();
        }
    }
    else
    {
        // handle cases where we couldn't parse the command
        rstr << "wrong # remove args: should be '"
            << argv[0]
            << " remove <cid_list>'"
            << ends;

        Tcl_SetObjResult( interp, Tcl_NewStringObj( rstr.str(), -1 ));
        rstr.rdbuf()->freeze( 0 );

        ret = TCL_ERROR;
    }

    return ret;
}



// HandleZoomCommand - respond to the zoom command by validating command
// and zooming graph if needed
//
int
PDGraph::HandleZoomCommand( int argc, char* argv[] )
{
    int ret = TCL_OK;
    ostrstream estr;
    double position = 0.0;
    bool ok = true;


    if( argc > 2 )
    {
        if( !strcmp( argv[2], "scroll" ) )
        {
            // it is a scroll +/- 1 page or unit
            if( argc == 5 )
            {
                // attempt to interpre the units
                if( !strcmp( argv[4], "pages" ) )
                {
                    // determine where to zoom to
                    if( !strcmp( argv[3], "1" ) )
                    {
                        // page up
                        position = FocusToPosition( visScopeInfo.focus ) + 
                                                    ZOOM_THUMB_SIZE;
                    }
                    else if( !strcmp( argv[3], "-1" ) )
                    {
                        // page down
                        position = FocusToPosition( visScopeInfo.focus ) - 
                                                    ZOOM_THUMB_SIZE;
                    }
                    else
                    {
                        // indicate malformed request
                        estr << "bad zoom option";
                        ok = false;
                    }
                }
                else if( !strcmp( argv[4], "units" ) )
                {
                    // determine where to zoom to
                    if( !strcmp( argv[3], "1" ) )
                    {
                        // unit up
                        position = FocusToPosition( visScopeInfo.focus ) + 
                                                    ZOOM_UNIT_SIZE;
                    }
                    else if( !strcmp( argv[3], "-1" ) )
                    {
                        // unit down
                        position = FocusToPosition( visScopeInfo.focus ) - 
                                                    ZOOM_UNIT_SIZE;
                    }
                    else
                    {
                        // indicate malformed request
                        estr << "bad zoom option";
                        ok = false;
                    }
                }
                else
                {
                    // indicate reason for failure
                    estr << "bad zoom option";
                    ok = false;
                }
            }
            else
            {
                // indicate reason for failure
                estr << "wrong # zoom args";
                ok = false;
            }
        }
        else if( !strcmp( argv[2], "moveto" ) )
        {
            // it is a scroll to specific position
            if( argc == 4 )
            {
                // determine where to zoom to
                position = atof( argv[3] );
            }
            else
            {
                estr << "wrong # zoom args";
                ok = false;
            }
        }
        else
        {
            // indicate bad zoom option
            estr << "bad zoom option";
            ok = false;
        }
    }
    else
    {
        // we can't tell which zoom variant they intended
        estr << "wrong # zoom args";
        ok = false;
    }


    if( ok )
    {
        // validate position range
        if( position < 0.0 )
        {
            position = 0.0;
        }
        else if( position > 1.0 - ZOOM_THUMB_SIZE )
        {
            position = 1.0 - ZOOM_THUMB_SIZE;
        }

        // move to new position
        ret = ZoomTo( position );
    }
    else
    {
        // handle cases where we couldn't parse the command
        // provide some usage information
        estr << ": should be '"
            << argv[0]
            << " zoom scroll <number> <units>' or '"
            << argv[0]
            << " zoom moveto <position>'"
            << ends;

        // set the result
        Tcl_SetObjResult( interp, Tcl_NewStringObj( estr.str(), -1 ));
        estr.rdbuf()->freeze( 0 );

        ret = TCL_ERROR;
    }

    return ret;
}


// HandlePanCommand - respond to the pan command by validating params and
// panning graph if needed
//
int PDGraph::HandlePanCommand( int argc, char* argv[] )
{
    int ret = TCL_OK;
    ostrstream estr;
    double position = 0.0;
    bool ok = true;


    if( argc > 2 )
    {
        if( !strcmp( argv[2], "scroll" ) )
        {
            // it is a scroll +/- 1 page or unit
            if( argc == 5 )
            {
                // attempt to interpret the units
                if( !strcmp( argv[4], "pages" ) )
                {
                    // determine where to move to
                    if( !strcmp( argv[3], "1" ) )
                    {
                        // page left
                        position = visScopeInfo.start + visScopeInfo.focus;
                    }
                    else if( !strcmp( argv[3], "-1" ) )
                    {
                        // page right
                        position = visScopeInfo.start - visScopeInfo.focus;
                    }
                    else
                    {
                        // indicate malformed request
                        ok = false;
                        estr << "bad pan option";
                    }
                }
                else if( !strcmp( argv[4], "units" ) )
                {
                    // determine where to move to
                    if( !strcmp( argv[3], "1" ) )
                    {
                        // unit up
                        position = visScopeInfo.start + PAN_UNIT_SIZE;
                    }
                    else if( !strcmp( argv[3], "-1" ) )
                    {
                        // unit down
                        position = visScopeInfo.start - PAN_UNIT_SIZE;
                    }
                    else
                    {
                        // indicate malformed request
                        estr << "bad pan option";
                        ok = false;
                    }
                }
                else
                {
                    // indicate reason for failure
                    ok = false;
                    estr << "bad pan option";
                }
            }
            else
            {
                // indicate reason for failure
                estr << "wrong # pan args";
                ok = false;
            }
        }
        else if( !strcmp( argv[2], "moveto" ) )
        {
            // it is a scroll to specific position
            if( argc == 4 )
            {
                // determine where to move to
                position = atof( argv[3] );
            }
            else
            {
                estr << "wrong # pan args";
                ok = false;
            }
        }
        else
        {
            estr << "bad pan option";
            ok = false;
        }
    }
    else
    {
        // we can't tell which variant they intended
        estr << "wrong # pan args";
        ok = false;
    }


    if( ok )
    {
        // validate position range
        if( position < 0.0 )
        {
            position = 0.0;
        }
        else if( (position + visScopeInfo.focus) > 1.0 )
        {
            position -= (position + visScopeInfo.focus - 1.0);
        }

        // move to new position
        ret = PanTo( position );
    }
    else
    {
        // handle cases where we couldn't parse the command
        // by providing some usage information
        estr << ": should be '"
            << argv[0]
            << " pan scroll <number> <units>' or '"
            << argv[0]
            << " pan moveto <position>'"
            << ends;

        // set the result
        Tcl_SetObjResult( interp, Tcl_NewStringObj( estr.str(), -1 ));
        estr.rdbuf()->freeze( 0 );

        ret = TCL_ERROR;
    }

    return ret;
}



// HandleCmd - respond to an instance command.
// This method acts as a switchboard to route commands to their correct
// handling methods
//
int
PDGraph::HandleCmd( Tcl_Interp* interp, int argc, char* argv[] )
{
    int result = TCL_OK;
    size_t length;
    char c;


    // verify argument count
    if( argc < 2 )
    {
        ostrstream ostr;

        ostr << "wrong # args: should be \""
            << argv[0]
            << " option ?arg arg ...?\""
            << ends;
        Tcl_SetObjResult( interp, Tcl_NewStringObj( ostr.str(), -1 ));
        ostr.rdbuf()->freeze( 0 );

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
            ostrstream ostr;

            ostr << "wrong # args: should be \""
                << argv[0]
                << " cget option\""
                << ends;
            Tcl_SetObjResult( interp, Tcl_NewStringObj( ostr.str(), -1 ));
            ostr.rdbuf()->freeze( 0 );

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
    else if( (c == 'p') && !strncmp( "pan", argv[1], length ) )
    {
        // handle a change in the pan position
        result = HandlePanCommand( argc, argv );
    }
    else if( (c == 'z') && !strncmp( "zoom", argv[1], length ) )
    {
        // handle a change in the zoom position
        result = HandleZoomCommand( argc, argv );
    }
    else if( (c == 'g') && !strncmp( "getselected", argv[1], length ) )
    {
        // handle request for selection
        result = HandleGetSelectedCommand( argc, argv );
    }
    else if( (c == 's') && !strncmp( "smooth", argv[1], length ) )
    {
        result = HandleSmoothCommand( argc, argv, true );
    }
    else if( (c == 'u') && !strncmp( "unsmooth", argv[1], length ) )
    {
        result = HandleSmoothCommand( argc, argv, false );
    }
    else if( (c == 'h') && !strncmp( "hide", argv[1], length ) )
    {
        result = HandleShowCommand( argc, argv, false );
    }
    else if( (c == 's') && !strncmp( "show", argv[1], length ) )
    {
        result = HandleShowCommand( argc, argv, true );
    }
    else if( (c == 'r') && !strncmp( "remove", argv[1], length ) )
    {
        result = HandleRemoveCommand( argc, argv );
    }
    else
    {
        ostrstream ostr;

        ostr << "bad option \""
            << argv[1]
            << "\": must be cget, configure, zoom, pan, getselected, smooth, unsmooth, or remove"
            << ends;
        Tcl_SetObjResult( interp, Tcl_NewStringObj( ostr.str(), -1 ));
        ostr.rdbuf()->freeze( 0 );

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



// Configure - handle Tcl configuration of the widget from the
// given arguments
//
int
PDGraph::Configure(Tcl_Interp* interp, int argc, char* argv[], int flags )
{
    // handle Tk options
    if( Tk_ConfigureWidget(interp,
        tkwin,
        configSpecs,
        argc, argv,
        (char *)this, flags) != TCL_OK)
    {
        return TCL_ERROR;
    }

    // set the background for our windows based on our 
    // (possibly) updated background option
    Tk_SetWindowBackground(tkwin, Tk_3DBorderColor(bgBorder)->pixel);

    // create a graphics context for drawing
    if( gc == None )
    {
        XGCValues gcValues;
        unsigned long mask = GCFunction | GCGraphicsExposures | GCFont | 
             GCForeground | GCBackground;

        gcValues.foreground = Tk_3DBorderColor(fgBorder)->pixel;
        gcValues.background = Tk_3DBorderColor(bgBorder)->pixel;
        gcValues.function = GXcopy;
        gcValues.graphics_exposures = False;
        gcValues.font = Tk_FontId( font );
        gc = Tk_GetGC( tkwin, mask, &gcValues );
    }

    // register our desired geometry
    Tk_GeometryRequest(tkwin, 200, 150);
    Tk_SetInternalBorder(tkwin, borderWidth);

    // update our font metrics
    Tk_GetFontMetrics( font, &fontm );

    // arrange for ourselves to be redisplayed with
    // our new configuration
    RequestRedraw();

    return TCL_OK;
}


// HandleConfigureNotification - respond to a notification that
// the widget's configuration has changed
//
void
PDGraph::HandleConfigureNotification( void )
{
    // allow the subwindows to refigure their respective geometry,
    // in case it changed with the new configuration
    UpdateGeometry();

    // schedule a redraw if needed
    RequestRedraw();
}


// HandleEvent - respond to the given X event
//
void
PDGraph::HandleEvent( XEvent* eventPtr )
{
    switch( eventPtr->type )
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
        Tcl_DeleteCommandFromToken(interp, widgetCmd);

        // cancel any pending redraw as unnecessary
        CancelRedraw();

        // schedule our own termination
        Tcl_EventuallyFree((ClientData)this, PDGraph::DestroyCB);

        break;

    default:
        // we don't handle the other event types
        ;
    }
}


// HandleInstanceCommandDeleted - respond to situation when
// instance command has been deleted
void
PDGraph::HandleInstanceCmdDeleted( void ) 
{
    // destroy the widget in case it hasn't been destroyed already
    if( tkwin != NULL )
    {
        Tk_Window savedwin = tkwin;

        // indicate that we're on our way out
        tkwin = NULL;

        // now destroy our windows - this will
        // eventually result in our own destruction
        Tk_DestroyWindow( savedwin );
    }
}



// DrawBorder - handle the drawing of our border to our Tk window
void
PDGraph::DrawBorder( void ) const
{
    Pixmap pm = None;
    Drawable d;

    if(!Tk_IsMapped(tkwin))
    {
        return;
    }

    // draw into offscreen pixmap if desired
    if( doubleBuffer )
    {
        pm = Tk_GetPixmap(tkdisplay,
            Tk_WindowId(tkwin),
            Tk_Width(tkwin), Tk_Height(tkwin),
            DefaultDepthOfScreen(Tk_Screen(tkwin)));
        d = pm;
    }
    else
    {
        d = Tk_WindowId(tkwin);
    }

    // draw the desired border
    Tk_Fill3DRectangle(tkwin,
        d,
        bgBorder,
        0, 0,
        Tk_Width(tkwin), Tk_Height(tkwin),
        borderWidth, relief);


    // copy from the offscreen pixmap if needed
    if( doubleBuffer )
    {
        XCopyArea(tkdisplay,
            pm,
            Tk_WindowId(tkwin),
            gc,
            0, 0,
            Tk_Width(tkwin), Tk_Height(tkwin),
            0, 0);
        Tk_FreePixmap(tkdisplay, pm);
    }
}


// Draw - output a representation of our data to our window
// (Mostly, this work is handled directly by the subwidgets.)
void
PDGraph::Draw( void )
{
    // indicate that we're doing our update
    redrawPending = false;

    // draw our border, if needed
    if( borderWidth > 0 )
    {
        DrawBorder();
    }
}


// InstallClassCommand - installs the "pdgraph" command
// into the given Tcl interpreter
//
int
PDGraph::InstallClassCommand( Tcl_Interp* interp )
{
    // install our class command
    Tcl_Command cmd = Tcl_CreateCommand( interp,
                "pdgraph",
                PDGraph::ClassCmdCB,
                (ClientData)Tk_MainWindow( interp ),
                NULL );
    if( cmd == NULL )
    {
        return TCL_ERROR;
    }

    // install our subwidget class commands
    if( PDGraph::TimeAxisW::InstallClassCommand( interp ) != TCL_OK )
    {
        Tcl_DeleteCommandFromToken( interp, cmd );
        return TCL_ERROR;
    }

    if( PDGraph::ValueAxisW::InstallClassCommand( interp ) != TCL_OK )
    {
        Tcl_DeleteCommandFromToken( interp, cmd );
        return TCL_ERROR;
    }

    if( PDGraph::DataW::InstallClassCommand( interp ) != TCL_OK )
    {
        Tcl_DeleteCommandFromToken( interp, cmd );
        return TCL_ERROR;
    }
    return TCL_OK;
}



// UpdateHistogramInfo - updates the graph's idea about the
// current characteristics of the histogram
//
void
PDGraph::UpdateHistogramInfo(double startTimestamp,
                              unsigned int nBuckets,
                              double bucketWidth)
{
    assert( startTimestamp >= histInfo.startTimestamp );
    assert( bucketWidth > 0.0 );
    assert( nBuckets > 0 );


    // check whether we're changing the time base
    bool changingTimeBase = !((bucketWidth == histInfo.bucketWidth) &&
                             (startTimestamp == histInfo.startTimestamp ));

    // short circuit if we're not changing anything
    if( !changingTimeBase && (nBuckets == histInfo.nBuckets) )
    {
        return;
    }

    // update histogram characteristics
    histInfo.startTimestamp = startTimestamp;
    histInfo.nBuckets = nBuckets;
    histInfo.bucketWidth = bucketWidth;

    // update timeAxis based on the new information
    timeAxis->UpdateConfiguration();

    // reset curves in preparation for new data
    for( unsigned int idx = 0; idx < curves.size(); idx++ )
    {
        if( curves[idx] != NULL )
        {
            curves[idx]->nPoints = 0;
        }
    }

    // redraw all curves to reflect new histogram characteristics
    RequestRedraw();
    timeAxis->RequestRedraw();
    dataw->RequestRedraw();
}


// AddCurve - add a curve to the graph with the given metric and
// resource names
//
int
PDGraph::AddCurve( const char* metricName,
                    const char* metricLabel,
                    const char* resourceName )
{
    unsigned int idx = 0;


    // find the group (if any) that represents this metric
    // a match is based on the metric label
    PDGraph::Group* group = FindGroup( metricLabel );
    if( group == NULL )
    {
        // build a new object to represent group
        group = new Group( metricLabel );
        assert( group != NULL );

        // keep hold of the new object
        groups += group;
    }
    assert( group != NULL );

    // obtain an object to represent the curve
    PDGraph::Curve* curve = new PDGraph::Curve( metricName,
                                                resourceName,
                                                histInfo.nBuckets );
    group->Add( curve );

    // find a location in the curve vector for the new curve
    while( idx < curves.size() )
    {
        if( curves[idx] == NULL )
        {
            curves[idx] = curve;
            break;
        }
        idx++;
    }

    // grow the curve vector if necessary
    if( idx == curves.size() )
    {
        curves += curve;
    }
    CurveID cid = idx;

    
    // now count the number of curves we know about
    unsigned int nCurves = 0;
    for( idx = 0; idx < curves.size(); idx++ )
    {
        if( curves[idx] != NULL )
        {
            nCurves++;
        }
    }

    // update the legend
    ostrstream cmdstr;


    cmdstr << "::PDGraph::Legend::add_item "
        << Tk_PathName( legendWin ) << " "  // legend window name
        << nCurves - 1 << " "               // curve index within legend
        << cid << " "                       // curve ID (note - not the same as index)
        << '{' << curve->GetName() << "} "    // curve metric name
        << '\"' << Tk_NameOfColor( curve->GetLineSpec().color ) << "\" " // curve line color
        << '\"' << curve->GetLineSpec().stippleName << '\"'     // curve line stipple
        << ends;
    if( Tcl_EvalObj( interp, Tcl_NewStringObj( cmdstr.str(), -1 ) ) != TCL_OK )
    {
        // release the curve
        Remove( curve );

        // release hold on curve
        curves[cid] = NULL;

        // indicate failure to add
        cid = -1;
    }
    cmdstr.rdbuf()->freeze( 0 );

    // update our subwindow geometry to make space for the new curve
    UpdateGeometry();

    return cid;
}




// SetCurveData - associate data with the indicated curve
//
void
PDGraph::SetCurveData( CurveID cid,
                       int firstSample,
                       int nSamples,
                       const float* data )
{
    assert( cid != -1 );
    assert( ((unsigned)cid) < curves.size() );

    // access the indicated curve
    PDGraph::Curve* curve = curves[cid];
    assert( curve != NULL );

    // check for overflow
    if( firstSample + nSamples > (int)histInfo.nBuckets )
    {
        // TODO - how to indicate error of exceeding specified max sample ?
        return;
    }

    // update curve's data with the new data
    curve->SetData( firstSample, nSamples, data );

    // handle drawing of new curve points
    if( curve->isVisible )
    {
        Group* group = curve->group;

        // draw new points for the curve...

        // ...check if we found a new maximum value so we can rescale...
        if( curve->GetMaxActiveValue() > group->axis->maxValue )
        {
            UpdateAxisMaxValue( group->axis, curve->GetMaxActiveValue() );
        }
        else
        {
            dataw->HandleNewData( curve, firstSample, nSamples );
        }
    }
}



// UpdateAxisMaxValue - updates the maximum value of an axis, and 
// updates the display to reflect the new max value
// 
void
PDGraph::UpdateAxisMaxValue( ValueAxis* axis, double maxValue )
{
    // we found a new maximum Y value
    // update the entire group's view of the new max value and 
    // recompute the characteristics of the axis
    axis->maxValue = maxValue;

    // determine the maxmimum number of ticks we
    // can support in the given axis height
    //
    // Note that we go to great lengths to be sure
    // that we do our "max ticks" calculation as
    // signed integer arithmetic, so we can recognize
    // the case when the window is too short.
    int valAxisHeight = (Tk_IsMapped( valAxis->GetWindow() ) ? 
                            Tk_Height( valAxis->GetWindow() ) : 
                            Tk_ReqHeight( valAxis->GetWindow() ));
    int nTicksMax = (valAxisHeight - 
            ((int)timeAxis->DetermineHeight()) - 
            ((int)valAxis->DetermineLabelHeight())) / fontm.linespace;
    if( nTicksMax <= 0 )
    {
        // this could happen if we've not yet mapped the
        // value axis window, or if the window has
        // become too short
        //
        // we still want to compute an interval layout,
        // so we force the max number of ticks to a 
        // "reasonable" value
        nTicksMax = 5;
    }
    axis->ComputeIntervals( nTicksMax );

    // allow subwindows to recompute its visual information
    // based on new configuration
    dataw->UpdateConfiguration();

    // update the display
    RequestRedraw();
    valAxis->RequestRedraw();
}

// UpdateGeometry - update the geometry of the subwidgets based on
// the information held in the C++ objects
//
void
PDGraph::UpdateGeometry( void )
{
    // update value axes width
    // (width required is based on number of groups with visible curves)
    unsigned int widthRequired = 2;
    unsigned int gid;
    for( gid = 0; gid < groups.size(); gid++ )
    {
        if( groups[gid]->IsVisible() )
        {
            widthRequired += valAxis->DetermineWidth( groups[gid]->axis );
        }
    }
    Tk_GeometryRequest( valAxis->GetWindow(),
                        widthRequired,
                        Tk_Height( valAxis->GetWindow() ) );
}


// FindGroup - looks up the Group with the given metric label as its title
// if no group is so titled, returns NULL
//
PDGraph::Group*
PDGraph::FindGroup( const char* metricLabel ) const
{
    // find the group (if any) that represents this metric
    // a match is based on the metric label
    PDGraph::Group* group = NULL;
    unsigned int gid;
    for( gid = 0; gid < groups.size(); gid++ )
    {
        assert( groups[gid] != NULL );
        if( groups[gid]->axis->title == metricLabel )
        {
            // we've found a match on the metric - 
            // the new curve belongs in this group
            group = groups[gid];
            break;
        }
    }

    return group;
}





// Remove - Removes the given group from the set of groups known to
// the graph
//
void
PDGraph::Remove( PDGraph::Group* group )
{
    unsigned int gid;

    // find the group
    for( gid = 0; gid < groups.size(); gid++ )
    {
        assert( groups[gid] != NULL );
        if( groups[gid] == group )
        {
            // shift other group entries over removed entry
            while( gid < (groups.size() - 1))
            {
                groups[gid] = groups[gid+1];
                gid++;
            }
            groups.resize( groups.size() - 1 );

            // we're done
            break;
        }
    }
}


// Remove - Removes the given curve from the set of curves known to the
// graph.  If the removal of the curve results in a group becoming empty,
// also handles removal of the associated group.
//
void
PDGraph::Remove( PDGraph::Curve* curve )
{
    // release the curve object
    curve->group->Remove( curve );
    if( curve->group->curves.size() == 0 )
    {
        Remove( curve->group );
        delete curve->group;
    }
    delete curve;
}


// Looks up client data associated with the given Tk window.
// See comments by the declaration of the winInstDataMap member.
bool
PDGraph::FindInstanceData( char* name, Tcl_Interp* interp, ClientData& cd )
{
    Tk_Window tkwin = Tk_NameToWindow( interp, name, Tk_MainWindow( interp ) );
    bool found = false;


    if( tkwin != NULL )
    {
        found = winInstDataMap.find( tkwin, cd );
    }
    
    return found;
}



//---------------------------------------------------------------------------
// class callback functions
//---------------------------------------------------------------------------

int
PDGraph::InstanceCmdCB( ClientData cd, Tcl_Interp* interp, int argc, char* argv[] )
{
    PDGraph* pGraph = (PDGraph*)cd;
    return pGraph->HandleCmd( interp, argc, argv );
}


void
PDGraph::DrawCB(ClientData clientData)
{
    PDGraph* pGraph = (PDGraph*)clientData;
    pGraph->Draw();
}



void
PDGraph::EventCB(ClientData cd, XEvent* eventPtr)
{
    PDGraph* pGraph = (PDGraph*)cd;
    pGraph->HandleEvent( eventPtr );
}




void
PDGraph::DestroyCB(char* cd)
{
    PDGraph* pGraph = (PDGraph*) cd;
    delete pGraph;
}



void
PDGraph::InstanceCmdDeletedCB( ClientData cd )
{
    PDGraph* pGraph = (PDGraph*)cd;
    pGraph->HandleInstanceCmdDeleted();
}



//---------------------------------------------------------------------------
// PDGraph::Group implementation
//---------------------------------------------------------------------------

PDGraph::Group::Group( const char* title )
  : axis( new PDGraph::ValueAxis( title, 0, 0.0001, 10 ) ),
    nVisible( 0 )
{
}



PDGraph::Group::~Group( void )
{
}


// Add - add a curve to the group
//
void
PDGraph::Group::Add( PDGraph::Curve* curve )
{
    // keep hold of this curve
    curves += curve;
    curve->group = this;

    if( curve->isVisible )
    {
        nVisible++;
    }
}


// Remove - remove a curve from the group
//
void
PDGraph::Group::Remove( PDGraph::Curve* curve )
{
    unsigned int idx;


    // find the curve
    for( idx = 0; idx < curves.size(); idx++ )
    {
        if( curves[idx] == curve )
        {
            // we found our curve -
            // remove by shifting items over empty space
            while( idx < (curves.size() - 1) )
            {
                curves[idx] = curves[idx+1];
                
                idx++;
            }

            // now shrink the vector
            curves.resize( curves.size() - 1 );
            break;
        }
    }
}


//---------------------------------------------------------------------------
// PDGraph::Curve implementation
//---------------------------------------------------------------------------
const unsigned int PDGraph::Curve::smoothingWindowSize  = 8;

vector<PDGraph::Curve::LineSpec> PDGraph::Curve::lineColorSpecs;
vector<PDGraph::Curve::LineSpec> PDGraph::Curve::linePatternSpecs;
unsigned int PDGraph::Curve::nextLineSpecIdx = 0;

#ifndef DBL_MIN
#define DBL_MIN 1e-37
#endif

PDGraph::Curve::Curve( const char* metricName,
                       const char* resourceName,
                       unsigned int maxPoints )
  : isSmoothed( true ),
    nPoints( 0 ),
    pts( NULL ),
    spts( NULL ),
    group( NULL ),
    maxActiveValue( DBL_MIN ),
    xpts( NULL ),
    lineSpecIdx( nextLineSpecIdx++ ),
    useColor( true ),
    isVisible( true )
{
    // construct name of curve
    name = metricName;
    name += "<";
    name += resourceName;
    name += ">";

    // obtain space for our data
    pts = new double[maxPoints];
    for(unsigned i=0; i<maxPoints; i++) {
      pts[i] = make_Nan();
    }
    spts = new double[maxPoints];
    for(unsigned j=0; j<maxPoints; j++) {
      spts[j] = make_Nan();
    }
    xpts = new XPoint[maxPoints];
}


PDGraph::Curve::~Curve( void )
{
    delete[] pts;
    delete[] spts;
    delete[] xpts;
}


// ComputeXPoints - computes the coordinates to be used to represent
// the given subset of our currently active data within the given
// onscreen rectangle.  Also takes into account the
// current start and focus values to adjust to the current zoom 
// and pan settings of the graph.
//
void
PDGraph::Curve::ComputeXPoints( unsigned int startIdx,
                                unsigned int nxPoints,
                                XRectangle& rect,
                                double start,
                                double focus,
                                unsigned int nBuckets )
{
    double axisMaxValue = group->axis->nIntervals * group->axis->intervalWidth;
    double* data = GetActiveData();
    short ybase = rect.y + rect.height;
    unsigned int i;


    for( i = 0; i < nxPoints; i++ )
    {
        int idx = startIdx + i;

	double numer = static_cast<double>(idx) - (start * nBuckets);
	double denom = focus * nBuckets;
	double extraD = (numer / denom) * static_cast<double>(rect.width);
	int extraI = static_cast<int>(extraD);
	// the compilers past 2.95.3 are having problems with converting
        // directly from a double to a short
	short extraS = static_cast<short>(extraI);
        xpts[idx].x = rect.x + extraS;

        if( !isnan(data[idx]) )
        {
            // compute onscreen location for datapoint
            if( axisMaxValue != 0 )
            {
	      double extra = (data[idx] / axisMaxValue) * 
		             static_cast<double>(rect.height);
	      int extraI = static_cast<int>(extra);
	      // the compilers past 2.95.3 are having problems with converting
	      // directly from a double to a short
	      short extraS = static_cast<short>(extraI);
	      xpts[idx].y = ybase - extraS;
            }
            else
            {
                // y axis max is zero - to avoid division by zero,
                // we peg the value to zero
                xpts[idx].y = ybase;
            }
        }
        else
        {
            // peg undefined values to zero
            xpts[idx].y = ybase;
        }
    }
}


// SetData - associates the given data with our curve,
// starting at the indicated data point index
//
void
PDGraph::Curve::SetData( unsigned int startIdx,
                         unsigned int nNewPoints,
                         const float* data )
{
    // update our data with the new data
    unsigned int i;
    for( i = 0; i < nNewPoints; i++ )
    {
        pts[startIdx + i] = data[i];
    }

    // update notion of number of points owned by the curve
    if( startIdx + nNewPoints > nPoints )
    {
        nPoints = startIdx + nNewPoints;
    }

    // smooth data if desired
    if( isSmoothed )
    {
        ComputeSmoothedData( 0, nPoints, smoothingWindowSize );
    }

    // check for a new maximum value in the active data
    UpdateMaxActiveValue( startIdx );
}



// ComputeSmoothedData - using the given smoothing window size,
// computes the smoothed data points for the current set of raw data
// within the given data point interval
//
void
PDGraph::Curve::ComputeSmoothedData( unsigned int firstSample,
                                        unsigned int lastSample,
                                        unsigned int winSize )
{
    unsigned int i;
    unsigned int j;
    unsigned int k;

    // handle startup case where we don't have
    // winSize samples before first smoothed value
    if (firstSample <= winSize)
    {
        // search forward for next valid data
        while( (firstSample <= winSize) && (isnan(pts[firstSample])))
        {
            spts[firstSample] = pts[firstSample];
            firstSample++;
        }

        i = firstSample;
        while((i <= winSize) && (!isnan(pts[i])))
        {
            double sum = 0;

            for(j = firstSample; j <= i; j++)
            {
                sum += pts[j];
            }
            spts[i] = sum / (i+1);
            i++;
        }
        firstSample = i;
        k = i;
    }
    else
    {
        k = firstSample;
    }

    while(k <= lastSample)
    {
        double sum = 0;

        // search forward for next valid data
        j = k - winSize;

        while( (j <= k) && (isnan(pts[j])))
        {
            spts[j] = pts[j];
            j++;
        }

        // compute average value over window
	int nptsInAvg = 0;
        while( (j <= k) && (!isnan(pts[j])))
        {
            sum += pts[j];
            j++;
	    nptsInAvg++;
        }
	if(nptsInAvg > 0) 
	  spts[k] = sum / nptsInAvg;
        k++;
    }
}


// Draw - Draw a representation of the indicated subset of current data
// to the given Display and Drawable.
//
void
PDGraph::Curve::Draw( Display* disp,
                      Drawable d,
                      unsigned int startIdx,
                      unsigned int nPointsToDraw ) const
{
    assert( (startIdx + nPointsToDraw - 1) < nPoints );

    unsigned int idx = startIdx;
    while( idx < startIdx + nPointsToDraw )
    {
        // find next non-NaN point to draw
        while( (idx < startIdx + nPointsToDraw) && isnan( pts[idx] ) )
        {
            idx++;
        }
        if( idx == startIdx + nPointsToDraw )
        {
            // we're done
            break;
        }
        assert( !isnan(pts[idx]) );

        // start segment correctly with endpoint or
        // connected to previous data
        if( (idx == 0) || isnan( pts[idx - 1] ) )
        {
            DrawEndpoint( disp, d, xpts[idx] );
        }
        else if( idx != 0 )
        {
            // connect line to previous point
            XDrawLine( disp, d, GetLineSpec().gc,
                        xpts[idx-1].x, xpts[idx-1].y,
                        xpts[idx].x, xpts[idx].y );
        }
        idx++;

        while( (idx < startIdx + nPointsToDraw) && !isnan( pts[idx] ) )
        {
            // draw line to connect to previous point
            XDrawLine( disp, d, GetLineSpec().gc,
                        xpts[idx-1].x, xpts[idx-1].y,
                        xpts[idx].x, xpts[idx].y );

            idx++;
        }

        // draw an endpoint at the current point if needed
        if( idx < (startIdx + nPointsToDraw - 1) )
        {
            assert( isnan( pts[idx] ) );
            assert( !isnan( pts[idx-1] ));

            DrawEndpoint( disp, d, xpts[idx-1] );
        }
    }
}



// InitLineSpecs - try to obtain a set of line specifications from
// the Tk option data
//
int
PDGraph::Curve::InitLineSpecs( Tcl_Interp* interp,
                               Tk_Window win,
                               char* lineColors,
                               char* linePatterns,
                               Tk_3DBorder fgBorder )
{
    unsigned int nColors = 0;
    int ret = TCL_OK;


    // we want an easy way to parse the line specification
    // since we don't have a simple string tokenizer available,
    // we convert the string into a Tcl list object, and extract
    // the list items.  Unfortunately, we can't build a Tcl list
    // directly from the C string, so we take an intermediate step
    // through a Tcl string object.
    assert( lineColors != NULL );
    Tcl_Obj* lcObj = Tcl_NewStringObj( lineColors, -1 );
    if( lcObj != NULL )
    {
        int objc;
        Tcl_Obj** objv;

        ret = Tcl_ListObjGetElements( interp, lcObj, &objc, &objv );
        if( ret == TCL_OK )
        {
            if( objc > 0 )
            {
                int i;
            
                for( i = 0; i < objc; i++ )
                {
                    char* colorName = Tcl_GetStringFromObj( objv[i], NULL );
                    if( colorName != NULL )
                    {
                        // attempt to obtain a GC for the given color
                        XColor* color = Tk_GetColor( interp, win, colorName );
                        if( color != NULL )
                        {
                            XGCValues gcv;

                            gcv.foreground = color->pixel;
                            lineColorSpecs +=
                                LineSpec( Tk_GetGC( win, GCForeground, &gcv ),
                                                    color,
                                                    "solid" );

                            nColors++;
                        }
                    }
                }
            }
            else
            {
                // no line colors were specified
                Tcl_SetObjResult( interp, 
                        Tcl_NewStringObj( "no line colors specified", -1 ) );
                ret = TCL_ERROR;
            }
        }
    }

    if( (ret == TCL_OK) && (nColors == 0) )
    {
        // we weren't able to obtain any color line specifications
        Tcl_SetObjResult( interp, Tcl_NewStringObj( "unable to parse line color specification", -1 ) );
        ret = TCL_ERROR;
    }

    if( ret != TCL_OK )
    {
        return ret;
    }

#if READY
    // note - the black-and-white code is currently disabled
    // the problem is in the drawing of the lines within the
    // legend widget with the indicated stipple.  Apparently
    // there is a bug in the canvas widget in Tk 8.0.5 (and
    // anecdotal evidence says that it hasn't been fixed in
    // Tk 8.1) that keeps from correctly handling the stipple
    // option.
    //
    // A potential workaround is to use window canvas items
    // instead of line items, so as to have complete control
    // over the drawing of the line.
    // 
    unsigned int nPatterns = 0;

    // now handle black and white (pattern) specs
    assert( linePatterns != NULL );
    Tcl_Obj* lpObj = Tcl_NewStringObj( linePatterns, -1 );
    if( lpObj != NULL )
    {
        int objc;
        Tcl_Obj** objv;

        ret = Tcl_ListObjGetElements( interp, lpObj, &objc, &objv );
        if( ret == TCL_OK )
        {
            if( objc > 0 )
            {
                int i;
            
                for( i = 0; i < objc; i++ )
                {
                    char* pattName = Tcl_GetStringFromObj( objv[i], NULL );
                    if( pattName != NULL )
                    {
                        // get the foreground color of the given window
                        XColor* color = Tk_3DBorderColor(fgBorder);


                        // obtain the pixmap for the given stipple
                        bool useStipple = !strcmp( pattName, "solid" );
                        Pixmap stipple = NULL;

                        if( useStipple )
                        {
                            stipple = Tk_GetBitmap( interp, win, Tk_GetUid( pattName ) );
                        }

                        // attempt to obtain a GC for the given color and pattern
                        if( (color != NULL) && ((stipple != NULL) || !useStipple) )
                        {
                            XGCValues gcv;
                            unsigned long valMask = GCForeground;

                            gcv.foreground = color->pixel;
                            if( useStipple )
                            {
                                gcv.stipple = stipple;
                                valMask |= GCStipple;
                            }
                            
                            linePatternSpecs +=
                                LineSpec( Tk_GetGC( win, valMask, &gcv ),
                                                    color,
                                                    pattName );

                            nPatterns++;
                        }

                        if( stipple != NULL )
                        {
                            Tk_FreeBitmap( Tk_Display( win ), stipple );
                        }
                    }
                }
            }
            else
            {
                // no line colors were specified
                Tcl_SetObjResult( interp, 
                        Tcl_NewStringObj( "no line patterns specified", -1 ) );
                ret = TCL_ERROR;
            }
        }
    }

    if( (ret == TCL_OK) && (nPatterns == 0) )
    {
        // we weren't able to obtain any line pattern specifications
        Tcl_SetObjResult( interp,
            Tcl_NewStringObj( "unable to parse line pattern specification", -1 ) );
        ret = TCL_ERROR;
    }
#endif // READY

    return ret;
}



// ReleaseLineSpecs - release line specification resources
//
void
PDGraph::Curve::ReleaseLineSpecs( Display* disp )
{
    unsigned int i;

    for( i = 0; i < lineColorSpecs.size(); i++ )
    {
        Tk_FreeGC( disp, lineColorSpecs[i].gc );
        Tk_FreeColor( lineColorSpecs[i].color );
    }
    lineColorSpecs.resize( 0 );

    for( i = 0; i < linePatternSpecs.size(); i++ )
    {
        Tk_FreeGC( disp, linePatternSpecs[i].gc );
        Tk_FreeColor( linePatternSpecs[i].color );
    }
    linePatternSpecs.resize( 0 );
}


// Smooth - Indicates that we should use smoothed data as our
// active data.  If we are not already smoothed, computes smoothed data.
//
void
PDGraph::Curve::Smooth( void )
{
    if( !isSmoothed )
    {
        isSmoothed = true;

        // ensure we have up-to-date smoothed data
        ComputeSmoothedData( 0, nPoints, smoothingWindowSize );

        // reset the max value of the active data
        maxActiveValue = DBL_MIN;
        UpdateMaxActiveValue( 0 );
    }
}




// Unsmooth - Indicates that we should use raw data as our active data.
//
void
PDGraph::Curve::Unsmooth( void )
{
    if( isSmoothed )
    {
        isSmoothed = false;

        // reset the max value of the active data
        maxActiveValue = DBL_MIN;
        UpdateMaxActiveValue( 0 );
    }
}



// UpdateMaxActiveValue - determines the max value from the active
// data set, considering all data values with index i or larger
//
void
PDGraph::Curve::UpdateMaxActiveValue( unsigned int startIdx )
{
    unsigned int i;


    double* pts = GetActiveData();
    for( i = startIdx; i < nPoints; i++ )
    {
        if( pts[i] > maxActiveValue )
        {
            maxActiveValue = pts[i];
        }
    }
}





//---------------------------------------------------------------------------
// PDGraph::ValueAxis impelementation
//---------------------------------------------------------------------------

// ComputeIntervals - given the maximum number of ticks
// allowed, computes a visually appealing number of intervals
// for the Axis
//
void
PDGraph::ValueAxis::ComputeIntervals( unsigned int nTicksMax )
{
    // exact number of intervals needed for (nIntervalsExact * vInterval == maxValue)
    register double nIntervalsExact;
    register unsigned int nDigitsBefore;    // digits before the decimal point
    register unsigned int nDigitsAfter;     // digits after the decimal point
    unsigned int precision;


    if(maxValue <= 0.00000001)
    {
        nIntervalsExact = 0.99;
        intervalWidth = 1.0;
    }
    else if(maxValue < 0.0)
    {
        nIntervalsExact = -maxValue;
        intervalWidth = -1.0;
    }
    else
    {
        nIntervalsExact = maxValue;
        intervalWidth = 1.0;
    }
    nDigitsAfter = 0;
    if (nIntervalsExact < 1.0)
    {
        nDigitsBefore = 1;
        while (nIntervalsExact < 0.1)
        {
            nIntervalsExact *= 10.0;
            intervalWidth /= 10.0;
            nDigitsAfter++;
        }
    }
    else
    {
        nDigitsBefore = 0;
        while (nIntervalsExact >= 1.0)
        {
            nIntervalsExact /= 10.0;
            intervalWidth *= 10.0;
            nDigitsAfter--;
            nDigitsBefore++;
        }
    }
    precision = 0;
    while (1 + (unsigned int)(3.0 * nIntervalsExact) < nTicksMax)
    {
        if (1 + (unsigned int)(6.0 * nIntervalsExact) < nTicksMax)
            if (1 + (unsigned int)(15.0 * nIntervalsExact) < nTicksMax)
            {
                intervalWidth /= 10.0;
                nIntervalsExact *= 10.0;
            }
            else
            {
                intervalWidth /= 5.0;
                nIntervalsExact *= 5.0;
            }
        else
        {
            intervalWidth /= 2.0;
            nIntervalsExact *= 2.0;
        }
        precision++;
        nDigitsAfter++;
    }
    nIntervals = 1 + (unsigned int)nIntervalsExact;
    if(maxValue == (nIntervals - 1) * intervalWidth)
    {
        nIntervals--;
    }

    // determine correct format to use for labels
    if( (nDigitsBefore + (nDigitsAfter > 0 ? nDigitsAfter + 1 : 0)) > precision + 5)
    {
        sprintf(labelFormat, "%%%d.%de", 0, precision - 1);
    }
    else if(nDigitsAfter > 0)
    {
        sprintf(labelFormat, "%%%d.%df", 0, nDigitsAfter);
    }
    else
    {
        sprintf(labelFormat, "%%%d.0f", 0);
    }
}

//---------------------------------------------------------------------------
// explicit template instantiations needed by this class
//---------------------------------------------------------------------------
#include "paradyn/src/UIthread/minmax.C"
#include "common/src/Dictionary.C"

template class dictionary_hash<Tk_Window, ClientData>;
template class vector<dictionary_hash<Tk_Window, ClientData>::entry>;
template class vector<Tk_Window>;

template class vector<PDGraph::Curve*>;
template class vector<PDGraph::Curve::LineSpec>;
template class vector<PDGraph::Group*>;

template class vector<unsigned int>;
template class vector<void*>;

