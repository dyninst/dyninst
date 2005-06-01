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

// $Id: pd_image.h,v


#if !defined(__PD_IMAGE__)
#define __PD_IMAGE__

#include "common/h/Vector.h"

class image;
class pd_process;
class pd_module;

class pd_image {
   friend void addLibraryCallback(BPatch_thread *, BPatch_module *, bool);
   static pdvector<pd_image *> all_pd_images;

 public:

   static pd_image *get_pd_image(BPatch_module *dyn_module);
   pd_module *get_pd_module(BPatch_module *dyn_module);

   pd_image(BPatch_image *d_image, pd_process *p_proc);
   ~pd_image();

   BPatch_image *get_dyn_image() {  return appImage; }
   pd_process *getParentProc() {return parent_proc;}
   pdstring name() const { return _name; }

   // report statically determinable caller-callee relationship to paradyn....
   void FillInCallGraphStatic(pd_process *p);

   pdstring get_file() const;
   int getAddressWidth();


   BPatch_Vector<BPatch_function *> *getIncludedFunctions();
   BPatch_Vector<BPatch_function *> *getIncludedFunctions(BPatch_module *mod);
   pdvector<BPatch_module *> *getIncludedModules(pdvector<BPatch_module *>*buf);
   BPatch_module *findModule(const pdstring &mod_name, bool check_excluded);
   bool hasModule(BPatch_module *mod);

 private:

   bool addModule(BPatch_module *mod);

   BPatch_image *appImage;
   pd_process *parent_proc;
   BPatch_Vector<BPatch_function *> *some_funcs; // included functions
   pdvector <pd_module *> some_mods; // included modules
   pdvector <pd_module *> all_mods;

   pdstring _name;
   pdstring _fname;
};


#endif

