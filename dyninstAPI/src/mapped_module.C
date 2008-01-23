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

// $Id: mapped_module.C,v 1.25 2008/01/23 14:45:51 jaw Exp $

#include "dyninstAPI/src/mapped_module.h"
#include "dyninstAPI/src/mapped_object.h"
#include "dyninstAPI/src/symtab.h"
#include "common/h/String.h"
#include "dyninstAPI/src/debug.h"
#include "symtabAPI/src/Object.h"
#include "process.h"
#include <iomanip>

bool mapped_module::truncateLineFilenames = true;

const pdvector<int_function *> &mapped_module::getAllFunctions() 
{
   pdvector<image_func *> pdfuncs;
   internal_mod_->getFunctions(pdfuncs);

   if (everyUniqueFunction.size() == pdfuncs.size())
      return everyUniqueFunction;

   for (unsigned i = 0; i < pdfuncs.size(); i++) {
      // Will auto-create (and add to this module)
      obj()->findFunction(pdfuncs[i]);
   }
   assert(everyUniqueFunction.size() == pdfuncs.size());

   return everyUniqueFunction;
}

const pdvector<int_variable *> &mapped_module::getAllVariables() 
{
   pdvector<image_variable *> img_vars;
   internal_mod_->getVariables(img_vars);

   if (everyUniqueVariable.size() == img_vars.size())
      return everyUniqueVariable;

   for (unsigned i = 0; i < img_vars.size(); i++) {
      obj()->findVariable(img_vars[i]);
   }

   return everyUniqueVariable;
}

// We rely on the mapped_object for pretty much everything...
void mapped_module::addFunction(int_function *func) 
{
   // Just the everything vector... the by-name lists are
   // kept in the mapped_object and filtered if we do a lookup.
   everyUniqueFunction.push_back(func);
}

void mapped_module::addVariable(int_variable *var) 
{
   everyUniqueVariable.push_back(var);
}

const string &mapped_module::fileName() const 
{
   return pmod()->fileName(); 
}

const string &mapped_module::fullName() const 
{
   return pmod()->fullName(); 
}

mapped_object *mapped_module::obj() const 
{
   return obj_;
}

bool mapped_module::isNativeCompiler() const 
{
   // This should probably be per-module info at some point; some
   // .o's might be compiled native, and others not.
   return pmod()->mod()->exec()->isNativeCompiler();
}

supportedLanguages mapped_module::language() const 
{
   return pmod()->language(); 
}

bool mapped_module::findFuncVectorByMangled(const pdstring &funcname,
      pdvector<int_function *> &funcs)
{
   // For efficiency sake, we grab the image vector and strip out the
   // functions we want.
   // We could also keep them all in modules and ditch the image-wide search; 
   // the problem is that BPatch goes by module and internal goes by image. 
   unsigned orig_size = funcs.size();

   const pdvector<int_function *> *obj_funcs = obj()->findFuncVectorByMangled(funcname);
   if (!obj_funcs) {
      return false;
   }

   for (unsigned i = 0; i < obj_funcs->size(); i++) {
      if ((*obj_funcs)[i]->mod() == this)
         funcs.push_back((*obj_funcs)[i]);
   }

   return funcs.size() > orig_size;
}

bool mapped_module::findFuncVectorByPretty(const pdstring &funcname,
      pdvector<int_function *> &funcs)
{
   // For efficiency sake, we grab the image vector and strip out the
   // functions we want.
   // We could also keep them all in modules and ditch the image-wide search; 
   // the problem is that BPatch goes by module and internal goes by image. 
   unsigned orig_size = funcs.size();

   const pdvector<int_function *> *obj_funcs = obj()->findFuncVectorByPretty(funcname);
   if (!obj_funcs) return false;

   for (unsigned i = 0; i < obj_funcs->size(); i++) {
      if ((*obj_funcs)[i]->mod() == this)
         funcs.push_back((*obj_funcs)[i]);
   }
   return funcs.size() > orig_size;
}


pdmodule *mapped_module::pmod() const 
{
   return internal_mod_;
}

void mapped_module::dumpMangled(pdstring prefix) const 
{
   // No reason to have this process specific... it just dumps
   // function names.
   pmod()->dumpMangled(prefix);
}

mapped_module::mapped_module(mapped_object *obj,
      pdmodule *pdmod) :
   internal_mod_(pdmod),
   obj_(obj),
   lineInfoValid_(false)
{
}

mapped_module *mapped_module::createMappedModule(mapped_object *obj,
      pdmodule *pdmod) 
{
   assert(obj);
   assert(pdmod);
   assert(pdmod->imExec() == obj->parse_img());
   mapped_module *mod = new mapped_module(obj, pdmod);
   // Do things?

   return mod;
}

// BPatch loves the mapped_module, but we pass it up to the image (since
// that occupies a range of memory; modules can be scattered all around it).
codeRange *mapped_module::findCodeRangeByAddress(const Address &addr)  
{
   return obj()->findCodeRangeByAddress(addr);
}

int_function *mapped_module::findFuncByAddr(const Address &addr)  
{
   return obj()->findFuncByAddr(addr);
}


pdstring mapped_module::processDirectories(const pdstring &fn) const 
{
   // This is black magic... assume Todd (I think) knew what
   // he was doing....
   if(fn == "")
      return "";

   if(!strstr(fn.c_str(),"/./") &&
         !strstr(fn.c_str(),"/../"))
      return fn;

   pdstring ret;
   char suffix[10] = "";
   char prefix[10] = "";
   char* pPath = new char[strlen(fn.c_str())+1];

   strcpy(pPath,fn.c_str());

   if(pPath[0] == '/')
      strcpy(prefix, "/");
   else
      strcpy(prefix, "");

   if(pPath[strlen(pPath)-1] == '/')
      strcpy(suffix, "/");
   else
      strcpy(suffix, "");

   int count = 0;
   char* pPathLocs[1024];
   char* p = strtok(pPath,"/");
   while(p){
      if(!strcmp(p,".")){
         p = strtok(NULL,"/");
         continue;
      }
      else if(!strcmp(p,"..")){
         count--;
         if(((count < 0) && (*prefix != '/')) || 
               ((count >= 0) && !strcmp(pPathLocs[count],"..")))
         {
            count++;
            pPathLocs[count++] = p;
         }
         if(count < 0) count = 0;
      }
      else
         pPathLocs[count++] = p;

      p = strtok(NULL,"/");
   }

   ret += prefix;
   for(int i=0;i<count;i++){
      ret += pPathLocs[i];
      if(i != (count-1))
         ret += "/";
   }
   ret += suffix;

   delete[] pPath;
   return ret;
}

AddressSpace *mapped_module::proc() const 
{
   return obj()->proc(); 
}

LineInformation *mapped_module::getLineInformation() 
{
   LineInformation *li = NULL;

  if (!lineInfoValid_) {
     pdmodule *pm = pmod();
     if (!pm) {
        fprintf(stderr, "%s[%d]:FIXME\n", FILE__, __LINE__);
        return NULL;
     }
     Module *m = pm->mod();
     if (!m) {
        fprintf(stderr, "%s[%d]:FIXME\n", FILE__, __LINE__);
        return NULL;
     }

     li = m->getLineInformation();

     if (li)
        lineInfoValid_ = true;
     else {
        //  Hmmm...  apparently failure is ok here?
        //fprintf(stderr, "%s[%d]:  failed to parse line information for module %s\n", FILE__, __LINE__, fileName().c_str());
     }
  }

  lineInfoValid_ = true;
  return li;
}


