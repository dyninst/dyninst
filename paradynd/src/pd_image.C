/*
 * Copyright (c) 1996 Barton P. Miller
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

// $Id: pd_image.C,v

#include "paradynd/src/pd_process.h"
#include "paradynd/src/pd_image.h"
#include "dyninstAPI/src/symtab.h"
#include "paradynd/src/pd_module.h"


pdvector<pd_image *> pd_image::all_pd_images;

// this function doesn't get called much right now
// if it's use becomes more frequest, we can make it faster by using
// a dictionary
pd_image *pd_image::get_pd_image(image *dyn_img) {
   for(unsigned i=0; i<all_pd_images.size(); i++) {
      pd_image *pd_img = all_pd_images[i];
      if(pd_img->get_dyn_image() == dyn_img)
         return pd_img;
   }
   return NULL;
}

pd_image::pd_image(image *d_image, pd_process *p_proc) :
   dyn_image(d_image), parent_proc(p_proc)
{
   all_pd_images.push_back(this);
   pdvector<module *> *mods = parent_proc->get_dyn_process()->getAllModules();
  
   for (unsigned int m = 0; m < mods->size(); m++) {
      pdmodule *curr = (pdmodule *) (*mods)[m];
      pd_module *pd_mod = new pd_module(curr);
      pd_modules.push_back(pd_mod);
   }
}

void pd_image::FillInCallGraphStatic(pd_process *proc) {
   pdstring pds;
   pdstring buffer;

   for(unsigned i=0; i<pd_modules.size(); i++) {
      pd_module *curmod = pd_modules[i];
      buffer = "building call graph module: " + curmod->fileName();
      statusLine(buffer.c_str());

      // if env var set and curmod is the first module (the user's program)
      // then print the loops for all the functions in this module.
      bool printLoops = false;
      if (char * pl = getenv("PARADYND_PRINTLOOPS")) {
	  printLoops = (i==0);
      }

      curmod->FillInCallGraphStatic(proc->get_dyn_process(), printLoops);
   }
}

int pd_image::getAddressWidth() {
   return dyn_image->getObject().getAddressWidth();
}

pdstring pd_image::get_file() const {
   return dyn_image->file();
}

