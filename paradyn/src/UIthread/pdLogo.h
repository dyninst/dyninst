// pdLogo.h

/*
 * $Log: pdLogo.h,v $
 * Revision 1.3  1996/08/05 07:30:31  tamches
 * removed a warning
 *
 * Revision 1.2  1995/12/26 19:58:17  tamches
 * tclclean/tkclean --> tcl/tk.h; cleaned up include of String.h w.r.t. directory
 *
 * Revision 1.1  1995/11/29 00:19:35  tamches
 * first version of pdLogo
 *
 */

#ifndef _PD_LOGO_H_
#define _PD_LOGO_H_

#include <assert.h>
#include <iostream.h>

#include "tcl.h"
#include "tk.h"
#include "tkTools.h"

#ifdef PARADYN
#include "util/h/DictionaryLite.h"
#include "util/h/String.h"
#else
#include "DictionaryLite.h"
#include "String.h"
#endif

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

   static dictionary_lite<string, logoStruct> all_installed_logos;
   static dictionary_lite<string, pdLogo *> all_logos;
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
			   int argc, char **argv);
      // makeLogoCommand rigs things s.t. this is called when the window enclosing
      // the logo needs redrawing.  sent 1 arg: the tk window name

   static int destroyCallback(ClientData pthis, Tcl_Interp *interp,
                              int argc, char **argv);
      // makeLogoCommand rigs things s.t. this is called when the window enclosing
      // the logo is destroyed.  sent 1 arg: the tk window name

 public:
   static int install_fixed_logo(const string &str,
				 void *rawData, unsigned width, unsigned height);
      // we have no uninstall_fixed_logo since there's not big need for it.
      // creates the tcl command "createPdLogo" (rigged to makeLogoCommand, below)
      // if necessary.

   static int makeLogoCommand(ClientData cd, Tcl_Interp *interp,
			       int argc, char **argv);
      // args: [0] (implicit) the command name
      // 1) tk window name   2) logo name (previously installed via install_fixed_logo)
      // 3) relief           4) border pix       5) color
      // NOTE: ClientData must point to a Tk_Window created by Tk_CreateMainWindow()
};

#endif
