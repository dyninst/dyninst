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

// pdLogo.C

/* $Id: pdLogo.C,v 1.10 2002/07/03 23:13:47 schendel Exp $ */

#include "pdLogo.h"
// needed since NT is doing some form of implicit template instantiation
#if defined(i386_unknown_nt4_0)
#include "common/src/Dictionary.C"
#endif

dictionary_hash<string, pdLogo::logoStruct> pdLogo::all_installed_logos(string::hash,
                                                                       9);
dictionary_hash<string, pdLogo *> pdLogo::all_logos(string::hash, 9);

bool pdLogo::tryFirst() {
   XGCValues values;

   if (theLogo != None)
      return true;

   if (Tk_WindowId(theTkWindow) == 0)
      return false; // not yet ready

	// obtain a graphics context that specifies the correct
	// foreground and background colors for our destination 
	// drawable
	values.foreground = foregroundColor->pixel;
	values.background = backgroundColor->pixel;
	copyGC = Tk_GetGC(theTkWindow,
						GCForeground | GCBackground,
						&values);

	// create a bitmap from the data
	Tk_DefineBitmap( interp, Tk_GetUid( "ParadynLogo" ),
		(char*)theLogoData.rawData,
		theLogoData.width,
		theLogoData.height );
	theLogo = Tk_GetBitmap( interp, theTkWindow, Tk_GetUid( "ParadynLogo" ) );
   assert(theLogo);

   return true;
}

void pdLogo::draw() {
   if (!tryFirst())
      return;

   installer.install(this); // calls real_draw() when idle
}

void pdLogo::real_draw(ClientData cd) {
   pdLogo *pthis = (pdLogo *)cd;
   assert(pthis);

   XCopyPlane(Tk_Display(pthis->theTkWindow),
	     pthis->theLogo, // src drawable
	     Tk_WindowId(pthis->theTkWindow), // dest drawable
	     pthis->copyGC,
	     0, 0, // src x, y
	     pthis->theLogoData.width, pthis->theLogoData.height,
	     pthis->borderPix, pthis->borderPix, // dest x, y
		 1 // plane - we copy from a bitmap
	     );
}

/* ********************************************************** */

pdLogo::pdLogo(Tcl_Interp *iInterp, Tk_Window iTkWindow,
	       void *iRawData, unsigned iWidth, unsigned iHeight) :
		    installer(&pdLogo::real_draw) {
   theLogoData.rawData = iRawData;
   theLogoData.width = iWidth;
   theLogoData.height = iHeight;
   
   interp = iInterp;
   theTkWindow = iTkWindow;
   theTkWindowName = string(Tk_PathName(theTkWindow));
   theDisplay = Tk_Display(theTkWindow);

   myTclEval(interp, theTkWindowName + " cget -borderwidth");
   borderPix = atoi(Tcl_GetStringResult(interp));

   myTclEval(interp, theTkWindowName + " cget -highlightcolor");
   const string foregroundColorName = string(Tcl_GetStringResult(interp));

   myTclEval(interp, theTkWindowName + " cget -background");
   const string backgroundColorName = string(Tcl_GetStringResult(interp));

   theLogo = None;

   foregroundColor = Tk_GetColor(interp, theTkWindow,
				 Tk_GetUid(foregroundColorName.c_str()));
   assert(foregroundColor);

   backgroundColor = Tk_GetColor(interp, theTkWindow,
				 Tk_GetUid(backgroundColorName.c_str()));
   assert(backgroundColor);

   copyGC = None;
}

pdLogo::~pdLogo() {
   if (copyGC != None)
   {
      Tk_FreeGC(theDisplay, copyGC);
   }

   if (theLogo)
   {
		Tk_FreeBitmap(theDisplay, theLogo);
   }

   Tk_FreeColor(foregroundColor);
   Tk_FreeColor(backgroundColor);
}

/* ******************************************************************* */

int pdLogo::install_fixed_logo(const string &str,
			       void *rawData, unsigned width, unsigned height) {
   // we have no uninstall_fixed_logo since there's not big need for it.
   if (all_installed_logos.defines(str)) {
      cout << "install_fixed_logo (" << str << "): already installed! (ignoring)" << endl;
      return TCL_OK;
   }
   logoStruct l;
   l.rawData = rawData;
   l.width = width;
   l.height = height;;
   all_installed_logos[str] = l;

   return TCL_OK;
}

int pdLogo::makeLogoCommand(ClientData cd, Tcl_Interp *interp,
			    int argc, char **argv) {
   // args: [0] (implicit) the command name
   // 1) tk window name   2) logo name (previously installed via install_fixed_logo)
   // 3) relief           4) border pix       5) color
   // NOTE: ClientData must point to a Tk_Window created by Tk_CreateMainWindow()
   if (argc != 6) {
      cerr << argv[0] << " requires 5 args (found " << argc-1 << ")" << endl;
      exit(5);
   }
   
   assert(argc == 6);
   const string theTkWindowName = argv[1];
   const string theLogoName     = argv[2];
   const string reliefString    = argv[3];
   unsigned borderPix           = atoi(argv[4]);
   const string colorString     = argv[5];

   if (!all_installed_logos.defines(theLogoName)) {
      cout << argv[0] << ": sorry, the logo " << theLogoName << " hasn't been installed" << endl;
      exit(5);
   }
   if (all_logos.defines(theTkWindowName)) {
      cout << argv[0] << " -- sorry, the tk window name " << theTkWindowName << " is already in the dictionary (didn't flush on window close?)" << endl;
      exit(5);
   }

   Tk_Window rootTkWindow = (Tk_Window)cd;

   // Obtain the logo from the installed-logos dictionary:
   assert(all_installed_logos.defines(theLogoName));
   logoStruct &l = all_installed_logos[theLogoName];

   // create a frame widget:
   myTclEval(interp, string("frame ") + theTkWindowName +
	     " -width " + string(l.width + 2*borderPix) +
	     " -height " + string(l.height + 2*borderPix) +
	     " -relief " + reliefString +
	     " -borderwidth " + string(borderPix) +
	     " -highlightcolor " + colorString);

   // At last, we can obtain the Tk_Window
   Tk_Window theTkWindow = Tk_NameToWindow(interp, 
                 const_cast<char*>(theTkWindowName.c_str()), rootTkWindow);
   if (theTkWindow == NULL) {
      cout << "pdLogo::createCommand() -- sorry, window " << theTkWindowName <<
	      " does not exist (could not create properly?)" << endl;
      exit(5);
   }

   pdLogo *theLogo = new pdLogo(interp, theTkWindow,
				l.rawData, l.width, l.height);
      // constructor does nothing terribly interesting
   assert(theLogo);

   all_logos[theTkWindowName] = theLogo;

   // now create the bindings.  We call the tk "bind" command.  This requires
   // us to do 2 Tcl_CreateCommand()'s.  We do this here if the commands
   // don't already exist.
   // the commands are: "pdLogoDrawCommand" and "pdLogoDestroyCommand"
   Tcl_CmdInfo dummy;
   if (0==Tcl_GetCommandInfo(interp, "pdLogoDrawCommand", &dummy))
      Tcl_CreateCommand(interp, "pdLogoDrawCommand", &pdLogo::drawCallback,
			NULL, NULL);
   if (0==Tcl_GetCommandInfo(interp, "pdLogoDestroyCommand", &dummy))
      Tcl_CreateCommand(interp, "pdLogoDestroyCommand", &pdLogo::destroyCallback,
			NULL, NULL);

   const string bindStringPrefix = string("bind ") + theTkWindowName + " ";
   myTclEval(interp, bindStringPrefix + "<Configure> {pdLogoDrawCommand %W}");
   myTclEval(interp, bindStringPrefix + "<Expose> {pdLogoDrawCommand %W}");
   myTclEval(interp, bindStringPrefix + "<Destroy> {pdLogoDestroyCommand %W}");
   return TCL_OK;      
}

int pdLogo::drawCallback(ClientData, Tcl_Interp *,
			 int argc, char **argv) {
   assert(argc == 2);

   const string theTkWindowName = argv[1];

   assert(all_logos.defines(theTkWindowName));
   pdLogo *pthis = all_logos[theTkWindowName];
   assert(pthis);

   pthis->draw();

   return TCL_OK;
}

int pdLogo::destroyCallback(ClientData, Tcl_Interp *,
			    int argc, char **argv) {
   // makeLogoCommand rigs things s.t. this is called when the window enclosing
   // the logo is destroyed.  sent 1 arg: the tk window name
   assert(argc == 2);

   const string theTkWindowName = argv[1];
   
   assert(all_logos.defines(theTkWindowName));

   pdLogo *pthis = all_logos[theTkWindowName];
   assert(pthis);
   delete pthis; // destructor does nothing interesting

   all_logos.undef(theTkWindowName); // can't use pthis->theTkWindowName, which
      // has just been deallocated!

   return TCL_OK;
}
