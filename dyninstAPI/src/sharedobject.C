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

// $Id: sharedobject.C,v 1.25 2005/01/18 18:34:16 bernat Exp $

#include "dyninstAPI/src/sharedobject.h"

#define FS_FIELD_SEPERATOR '/'

shared_object::shared_object():
  desc(0),
  name(0),
  short_name(0),
  base_addr(0),
  processed(false),
  mapped(false),
  objs_image(0),
  dlopenUsed(false),
  proc_(NULL)
{
}
shared_object::shared_object(pdstring &n, Address b, bool p,bool m, bool i, image *d, process *proc):
  name(n), 
  base_addr(b),
  processed(p),
  mapped(m),
  objs_image(d),
  dlopenUsed(false), // ccw 8 mar 2004
  proc_(proc)
{ 
  desc = new fileDescriptor(n, b);
  set_short_name();
  dirty_=false; 
  dirtyCalled_ = false;
}

shared_object::shared_object(fileDescriptor *f,
                             bool p, bool m, bool i, image *d, process *proc):
        desc(f),
        name(f->file()), 
        base_addr(f->addr()),
        processed(p),
        mapped(m),
        objs_image(d),
        dlopenUsed(false),
	proc_(proc)
{ 
    set_short_name();
    dirty_=false;
	dirtyCalled_ = false;
}

shared_object::shared_object(const shared_object &s, process *child) :
  codeRange(),
  desc(s.desc),
  name(s.name),
  short_name(s.short_name),
  base_addr(s.base_addr),
  processed(s.processed),
  mapped(s.mapped),
  dirty_(s.dirty_),
  dirtyCalled_(s.dirtyCalled_),
  dlopenUsed(s.dlopenUsed),
  proc_(child)
{
  // Reference counting...
  objs_image = s.objs_image->clone();
}

shared_object::~shared_object() {
  if (objs_image) {
    objs_image->cleanProcessSpecific(proc_);
    image::removeImage(objs_image);
  }
}

// TODO: this should probably not be a shared_object method, but since
// for now it is only used by shared_objects it is
// from a string that is a complete path name to a function in a module
// (ie. "/usr/lib/libc.so.1/write") return a string with the function
// part removed.  return 0 on error
char *shared_object::getModulePart(pdstring &full_path_name) {

    char *whole_name = P_strdup(full_path_name.c_str());
    char *next=0;
    char *last=next;
    if((last = P_strrchr(whole_name, '/'))){
        next = whole_name;
        for(u_int i=0;(next!=last)&&(i<full_path_name.length()); i++){
	    next++;
	    if(next == last){
		u_int size = i+2;
	        char *temp_str = new char[size];
	        if(P_strncpy(temp_str,whole_name,size-1)){
                    temp_str[size-1] = '\0';
		    delete whole_name;
		    return temp_str;
		    temp_str = 0;
	    } }
        }
    }
    delete whole_name;
    return 0;
}

pdmodule *shared_object::findModule(pdstring m_name)
{
   if(objs_image) {
     return (objs_image->findModule(m_name));
   }
   return 0;
}

// fill in "short_name" data member.  Use last component of "name" data
// member with FS_FIELD_SEPERATOR ("/") as field seperator....
void shared_object::set_short_name() {
    const char *name_string = name.c_str();
    const char *ptr = strrchr(name_string, FS_FIELD_SEPERATOR);
    if (ptr != NULL) {
       short_name = ptr+1;
    } else {
       short_name = pdstring("");
    }
}

pd_Function *shared_object::findOnlyOneFunction(const pdstring &funcname) 
{
  if (funcname.c_str() == 0) return NULL;
  if(objs_image) {
    return (objs_image->findOnlyOneFunction(funcname));
  }
  return NULL;
} 


pd_Function *shared_object::findOnlyOneFunctionFromAll(const pdstring &funcname) 
{
  if (funcname.c_str() == 0) return NULL;
  if(objs_image) {
    return (objs_image->findOnlyOneFunctionFromAll(funcname));
  }
  return NULL;
} 

pdvector<pd_Function *> *shared_object::findFuncVectorByPretty(const pdstring &funcname) 
{
  if (funcname.c_str() == 0) return NULL;
  if(objs_image) {
    return (objs_image->findFuncVectorByPretty(funcname));
  }
  return NULL;
} 

pd_Function *shared_object::findFuncByMangled(const pdstring &funcname) 
{
  if (funcname.c_str() == 0) return NULL;
  if(objs_image) {
    return (objs_image->findFuncByMangled(funcname));
  }
  return NULL;
} 

const pdvector<pd_Function *> *shared_object::getAllFunctions(){
  if(objs_image) {
    // previously objs_image->mdlNormal....
    return (&(objs_image->getAllFunctions()));
  }
  return 0;
}





