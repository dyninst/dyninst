// Options.C

#include "Options.h"
#include <stdio.h>

string Options::make_ptrs(unsigned count) {
  static char buffer[100];
  assert(count < 100);

  buffer[0] = (char)0;
  for (unsigned i=0; i<count; i++)
    buffer[i] = '*';
  buffer[count] = '\0';
  return buffer;
}

string Options::qual_to_unqual(const string type_name) {
  assert(Options::all_types.defines(type_name));
  type_defn *td = Options::all_types[type_name];
  return (td->unqual_name());
}

string Options::set_dir_decode() {
  return (Options::ml->set_dir_decode());
  //  return (string("setDirDecode()"));
}
string Options::set_dir_encode() {
  return (Options::ml->set_dir_encode());
  // return (string("setDirEncode()"));
}

string Options::error_state(bool braces,
                            unsigned nspaces,
                            const string &err_name, const string &return_value) {
   string spacesm3;
   for (unsigned lcv=0; lcv < nspaces-3; ++lcv)
      spacesm3 += " ";
   const string spaces = spacesm3 + "   ";

   string result;
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

string Options::gen_name() {
  char number[20];
  sprintf(number, "%d", var_count_++);
  return (string("var_") + number);
}

string Options::allocate_stl_type(string stl_type, string element_name,
				  const unsigned star_count, const bool in_lib) {
  type_defn *ign = new type_defn(stl_type, element_name, star_count, in_lib);
  assert(ign);
  Options::all_types[ign->name()] = ign;
  Options::vec_types += ign;
  return (ign->name());
}

string Options::allocate_type(const string &name, bool is_class, bool is_abstract,
			      bool is_derived, const string &parent,
			      const type_defn::type_type &typ,
			      bool can_point, bool in_lib,
			      vector<arg*> *arglist, const string &ignore_text,
			      const string &bundle_name) {
  return (Options::add_type(name, is_class, is_abstract, is_derived, parent, typ, can_point,
			    in_lib, arglist, ignore_text, bundle_name));
}

string Options::allocate_type(const string &name, bool is_class,
			      const type_defn::type_type &typ,
			      bool can_point, bool in_lib) {
  return Options::add_type(name, is_class, typ, can_point, in_lib);
}


string Options::add_type(const string name, const bool is_class, const bool is_abstract,
			 const bool is_derived, const string parent,
			 const type_defn::type_type &type,
			 const bool can_point, const bool in_lib,
			 vector<arg*> *arglist, const string ignore_text,
			 const string bundler_name) {
  type_defn *ign = new type_defn(name, is_class, is_abstract, is_derived, parent, type,
				 arglist, can_point, in_lib,
				 ignore_text, bundler_name);
  assert(ign);
  Options::all_types[ign->name()] = ign;
  Options::vec_types += ign;
  return (ign->name());
}

string Options::add_type(const string &name, bool is_class,
			 const type_defn::type_type &type,
			 const bool can_point, const bool in_lib) {
  type_defn *ign = new type_defn(name, is_class, type, can_point, in_lib);
   assert(ign);

   Options::all_types[ign->name()] = ign;
   Options::vec_types += ign;
   return (ign->name());
}
			    
void Options::dump_types() {
   for (dictionary_hash_iter<string, type_defn*> dhi=Options::all_types.begin(); dhi != Options::all_types.end(); dhi++)
      dhi.currval()->dump_type();
}

