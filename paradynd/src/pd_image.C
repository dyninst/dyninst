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

// $Id: pd_image.C,v

#include "paradynd/src/pd_process.h"
#include "paradynd/src/pd_image.h"
#include "paradynd/src/pd_module.h"


pdvector<pd_image *> pd_image::all_pd_images;
extern bool mdl_get_lib_constraints(pdvector<pdstring> &);
static dictionary_hash<pdstring, pdstring> func_constraint_hash(pdstring::hash);
static bool func_constraint_hash_loaded = false;
bool cache_func_constraint_hash();
bool function_is_excluded(BPatch_function *f, pdstring modname);
bool module_is_excluded(BPatch_module *m);

// this function doesn't get called much right now
// if it's use becomes more frequest, we can make it faster by using
// a dictionary
pd_image *pd_image::get_pd_image(BPatch_module *dyn_mod) {
   for(unsigned i=0; i<all_pd_images.size(); i++) {
      pd_image *pd_img = all_pd_images[i];
      if(pd_img->hasModule(dyn_mod))
         return pd_img;
   }
   return NULL;
}
pd_module *pd_image::get_pd_module(BPatch_module *dyn_mod) {
   for(unsigned i=0; i<all_mods.size(); i++) {
      pd_module *pd_mod = all_mods[i];
      if(pd_mod->get_dyn_module() == dyn_mod)
         return pd_mod;
   }
   return NULL;
}

#define NAME_LEN 512
pd_image::pd_image(BPatch_image *d_image, pd_process *p_proc) :
   appImage(d_image), parent_proc(p_proc)
{
   all_pd_images.push_back(this);
   BPatch_Vector<BPatch_module *> *mods = parent_proc->getAllModules();

   for (unsigned int m = 0; m < mods->size(); m++) {
      BPatch_module *curr = (BPatch_module *) (*mods)[m];
      addModule(curr);
   }

   char namebuf[NAME_LEN];
   d_image->getProgramName(namebuf, NAME_LEN);
   _name = pdstring(namebuf);
   d_image->getProgramFileName(namebuf, NAME_LEN);
   _fname = pdstring(namebuf);
   //fprintf(stderr, "%s[%d]:  new pd_image: '%s'/'%s'\n", 
   //        __FILE__, __LINE__, _name.c_str(), _fname.c_str());
}


pd_image::~pd_image() {
   for(unsigned i=0; i<all_mods.size(); i++) {
      delete all_mods[i];
   }
}

bool pd_image::addModule(BPatch_module *mod)
{
   pd_module *pd_mod = new pd_module(mod);
   all_mods.push_back(pd_mod);
     if (!pd_mod->isExcluded())
       some_mods.push_back(pd_mod);

  return true;
}

bool pd_image::hasModule(BPatch_module *mod)
{
  for (unsigned int m = 0; m < all_mods.size(); m++)
    if (mod == all_mods[m]->get_dyn_module()) return true;

  return false;
}

void pd_image::FillInCallGraphStatic(pd_process *p) 
{
   pdstring pds;
   pdstring buffer;

   for(unsigned i=0; i<all_mods.size(); i++) {
      pd_module *curmod = all_mods[i];
      buffer = "building call graph module: " + curmod->fileName();
      statusLine(buffer.c_str());

      curmod->FillInCallGraphStatic(p);
   }
}

int pd_image::getAddressWidth() {
   if (!all_mods.size()) return -1;
   return all_mods[0]->get_dyn_module()->getAddressWidth();
}

pdstring pd_image::get_file() const {
   return _fname;
}

BPatch_Vector<BPatch_function *> *pd_image::getIncludedFunctions()
{
  BPatch_Vector<BPatch_function *> *ret = new BPatch_Vector<BPatch_function *>();
  for (unsigned int i = 0; i < some_mods.size(); ++i) {
    BPatch_Vector<BPatch_function *> *temp;
    temp = getIncludedFunctions(some_mods[i]->get_dyn_module());
    if (NULL == temp) continue;
    (*ret) += (*temp);
  }
  return ret;
}

BPatch_Vector<BPatch_function *> *
pd_image::getIncludedFunctions(BPatch_module *mod)
{
  pd_module *pdm = get_pd_module(mod);
  if (NULL == pdm) return NULL;
  return pdm->getIncludedFunctions();
}

pdvector<BPatch_module *> *pd_image::getIncludedModules(pdvector<BPatch_module *> *buf)
{
  for (unsigned int i = 0; i < some_mods.size(); ++i) {
    buf->push_back(some_mods[i]->get_dyn_module());
  }
  return buf;
}

BPatch_module *pd_image::findModule(const pdstring &mod_name, bool check_excluded)
{
  pdvector<pd_module *> &modlist = check_excluded ? some_mods : all_mods;
  for (unsigned int i = 0; i < modlist.size(); ++i) {
     char buf[512];
     modlist[i]->get_dyn_module()->getName(buf, 512);
     if (!strcmp(buf, mod_name.c_str())) 
       return modlist[i]->get_dyn_module();
  }
  return NULL;
}


bool function_is_excluded(BPatch_function *f, pdstring module_name)
{
  char fnamebuf[2048];
  f->getName(fnamebuf, 2048);
  pdstring full_name = module_name + pdstring("/") + pdstring(fnamebuf);

  if (!func_constraint_hash_loaded) {
      if (!cache_func_constraint_hash()) {
          return FALSE;
      }
  }

  if (func_constraint_hash.defines(full_name)) {
      return TRUE;
  }
  return FALSE;
}

bool module_is_excluded(BPatch_module *m)
{
  char mnamebuf[512];
  m->getName(mnamebuf, 512);

  if (!func_constraint_hash_loaded) {
      if (!cache_func_constraint_hash()) {
          return FALSE;
      }
  }

  if (func_constraint_hash.defines(mnamebuf)) {
      return TRUE;
  }
  return FALSE;
}

bool cache_func_constraint_hash() {

    // strings holding exclude constraints....
    pdvector<pdstring> func_constraints;
    // if unble to get list of excluded functions, assume all functions
    //  are NOT excluded!!!!
    if(mdl_get_lib_constraints(func_constraints) == FALSE) {
        return FALSE;
    }
    func_constraint_hash_loaded = TRUE;

    unsigned i;
    for(i=0;i<func_constraints.size();i++) {
        func_constraint_hash[func_constraints[i]] = func_constraints[i];
    }
    return TRUE;
}

