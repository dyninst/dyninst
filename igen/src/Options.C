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

// Options.C

#include "Options.h"
#include <stdio.h>

pdstring Options::make_ptrs(unsigned count) {
  static char buffer[100];
  assert(count < 100);

  buffer[0] = (char)0;
  for (unsigned i=0; i<count; i++)
    buffer[i] = '*';
  buffer[count] = '\0';
  return buffer;
}

pdstring Options::qual_to_unqual(const pdstring type_name) {
  assert(Options::all_types.defines(type_name));
  type_defn *td = Options::all_types[type_name];
  return (td->unqual_name());
}

pdstring Options::set_dir_decode() {
  return (Options::ml->set_dir_decode());
  //  return (pdstring("setDirDecode()"));
}
pdstring Options::set_dir_encode() {
  return (Options::ml->set_dir_encode());
  // return (pdstring("setDirEncode()"));
}

pdstring Options::error_state(bool braces,
                            unsigned nspaces,
                            const pdstring &err_name, const pdstring &return_value) {
  pdstring spacesm3;
  for (unsigned lcv=0; lcv < nspaces-3; ++lcv)
    spacesm3 += " ";
  const pdstring spaces = spacesm3 + "   ";
  
  pdstring result;
  if (braces)
    result += "{\n";
  result += spaces + "IGEN_ERR_ASSERT;\n" +
    spaces + "set_err_state(" + err_name + ");\n";
    if (Options::ml->name() != "mrnet")
      {
	result += spaces + "handle_error();\n" ;
      }
    else
      {
	result +=spaces + "handle_error_mrnet();\n" ;
      }
  result +=spaces + "return " + return_value + ";\n";
  if (braces)
    result += spacesm3 + "}\n";

  return result;
}

pdstring Options::gen_name() {
  char number[20];
  sprintf(number, "%d", var_count_++);
  return (pdstring("var_") + number);
}

pdstring Options::allocate_stl_type(pdstring stl_type, pdstring element_name,
                                    const unsigned star_count,
                                    const bool in_lib) {
  type_defn *ign = new type_defn(stl_type, element_name, star_count, in_lib);
  assert(ign);
  Options::all_types[ign->name()] = ign;
  Options::vec_types += ign;
  return (ign->name());
}

pdstring Options::allocate_type(const pdstring &name, bool is_class,
                  bool is_abstract,
			      bool is_derived,
                  bool is_virtual,
                  const pdstring &parent,
			      const type_defn::type_type &typ,
			      bool can_point, bool in_lib,
			      pdvector<arg*> *arglist, const pdstring &ignore_text,
			      const pdstring &bundle_name) {
  return (Options::add_type(name,
                            is_class,
                            is_abstract,
                            is_derived,
                            is_virtual,
                            parent, typ, can_point,
			    in_lib, arglist, ignore_text, bundle_name));
}

pdstring Options::allocate_type(const pdstring &name, bool is_class,
			      const type_defn::type_type &typ,
			      bool can_point, bool in_lib) {
  return Options::add_type(name, is_class, typ, can_point, in_lib);
}


pdstring Options::add_type(const pdstring name, const bool is_class, const bool is_abstract,
			 const bool is_derived,
             const bool is_virtual,
             const pdstring parent,
			 const type_defn::type_type &type,
			 const bool can_point, const bool in_lib,
			 pdvector<arg*> *arglist, const pdstring ignore_text,
			 const pdstring bundler_name) {
  type_defn *ign = new type_defn(name, is_class, is_abstract,
                 is_derived,
                 is_virtual,
                 parent, type,
				 arglist, can_point, in_lib,
				 ignore_text, bundler_name);
  assert(ign);
  Options::all_types[ign->name()] = ign;
  Options::vec_types += ign;
  return (ign->name());
}

pdstring Options::add_type(const pdstring &name, bool is_class,
			 const type_defn::type_type &type,
			 const bool can_point, const bool in_lib) {
  type_defn *ign = new type_defn(name, is_class, type, can_point, in_lib);
   assert(ign);

   Options::all_types[ign->name()] = ign;
   Options::vec_types += ign;
   return (ign->name());
}
			    
void Options::dump_types() {
   for (dictionary_hash_iter<pdstring, type_defn*> dhi=Options::all_types.begin(); dhi != Options::all_types.end(); dhi++)
      dhi.currval()->dump_type();
}

