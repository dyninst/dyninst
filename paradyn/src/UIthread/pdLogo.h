/*
 * Copyright (c) 1996-1998 Barton P. Miller
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

// pdLogo.h

/* $Id: pdLogo.h,v 1.9 2003/06/20 02:12:19 pcroth Exp $ */

#ifndef _PD_LOGO_H_
#define _PD_LOGO_H_

#include <assert.h>
#include <iostream.h>


#include "common/h/Dictionary.h"
#include "common/h/String.h"
#include "tkTools.h"

class pdLogo {
 private:
   friend class foo; // placate compiler

   struct logoStruct {
      void *rawData; unsigned width; unsigned height;
      bool operator==(const logoStruct &other) const {
         return rawData == other.rawData;
      }
      logoStruct() {} // required by Pair.h
      logoStruct(int) {} // required by Pair.h
   };
   logoStruct theLogoData;

   Tcl_Interp *interp;
   Tk_Window theTkWindow;
   string theTkWindowName;
      // needed since in the destructor, Tk_PathName() may return junk
      // since the window may (probably?) have been deleted already.
   Display *theDisplay;
      // needed in destructor, when Tk_Display(theTkWindow)
      // may no longer be defined, since the tk window will have been deleted.
   Pixmap theLogo; // "None" until tryFirst() returns true
   XColor *foregroundColor, *backgroundColor;
   GC copyGC; // used only in XCopyArea()
   unsigned borderPix;

   tkInstallIdle installer;

   static dictionary_hash<string, logoStruct> all_installed_logos;
   static dictionary_hash<string, pdLogo *> all_logos;
   static tcl_cmd_installer createPdLogo; // tkTools.C

 private:
   bool tryFirst();
   void draw();
   static void real_draw(ClientData cd);

   // constructor and destructor do nothing terribly interesting
   pdLogo(Tcl_Interp *, Tk_Window,
	  void *rawData, unsigned width, unsigned height);
  ~pdLogo();

   static int drawCallback(ClientData pthis, Tcl_Interp *interp,
			   int argc, TCLCONST char **argv);
      // makeLogoCommand rigs things s.t. this is called when the window enclosing
      // the logo needs redrawing.  sent 1 arg: the tk window name

   static int destroyCallback(ClientData pthis, Tcl_Interp *interp,
                              int argc, TCLCONST char **argv);
      // makeLogoCommand rigs things s.t. this is called when the window enclosing
      // the logo is destroyed.  sent 1 arg: the tk window name

 public:
   static int install_fixed_logo(const string &str,
				 void *rawData, unsigned width, unsigned height);
      // we have no uninstall_fixed_logo since there's not big need for it.
      // creates the tcl command "createPdLogo" (rigged to makeLogoCommand, below)
      // if necessary.

   static int makeLogoCommand(ClientData cd, Tcl_Interp *interp,
			       int argc, TCLCONST char **argv);
      // args: [0] (implicit) the command name
      // 1) tk window name   2) logo name (previously installed via install_fixed_logo)
      // 3) relief           4) border pix       5) color
      // NOTE: ClientData must point to a Tk_Window created by Tk_CreateMainWindow()
};

#endif
