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
          spaces + "set_err_state(" + err_name + ");\n" +
          spaces + "handle_error();\n" +
          spaces + "return " + return_value + ";\n";
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

