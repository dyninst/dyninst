// Options.h
// Moved here by Ariel Tamches, from parse.h, into its own .[hC] file combo
// for clarity.

#ifndef _OPTIONS_H_
#define _OPTIONS_H_

#include "common/h/String.h"
#include "type_defn.h"
#include "interface_spec.h"
#include "message_layer.h"

class Options {
public:
  typedef enum { Mem_ignore, Mem_detect, Mem_handle } mem_type;

  static string make_ptrs(unsigned count);
  static string qual_to_unqual(const string type);
  static string file_base() { return file_base_;}
  static void set_file_base(const string f) { file_base_ = f;}
  static string input_file() { return input_file_;}
  static void set_input_file(const string f) { input_file_ = f;}

  static bool profile() { return profile_;}
  static void set_profile(const bool b) { profile_ = b;}
  static bool shortnames() { return shortnames_;}
  static void set_shortnames(const bool b) { shortnames_ = b;}
  static mem_type mem() { return mem_;}
  static void set_mem(const mem_type m) { mem_ = m;}

  static string error_state(bool braces,
                            unsigned nspaces,
                            const string &err_name, const string &return_value);
  static interface_spec *current_interface;
  static dictionary_hash<string, type_defn*> all_types;
  static pdvector<type_defn*> vec_types;
  static pdvector<message_layer*> all_ml;
  static message_layer *ml;

  static void ignore(string &s, bool is_server);
  static pdvector<string> client_ignores;
  static pdvector<string> server_ignores;
  static pdvector<string> forward_decls;

  typedef struct el_data {
    string type;
    unsigned stars;
    string name;
  } el_data;

  typedef struct stl_data {
    string include_file;
    string name;
    bool need_include;
    string pragma_name;
    pdvector<el_data> elements;
  } stl_data;
  static pdvector<stl_data> stl_types;

  static ifstream input;
  static ofstream dot_h;
  static ofstream dot_c;
  static ofstream clnt_dot_h;
  static ofstream clnt_dot_c;
  static ofstream srvr_dot_h;
  static ofstream srvr_dot_c;
  static ofstream temp_dot_c;

  static void dump_types();

  static string allocate_stl_type(string stl_type, string element_name,
				  const unsigned star_count, const bool in_lib);
  static string allocate_type(const string &name, bool is_class,
                              bool is_abstract,
			                  bool is_derived,
                              bool is_virtual,
                  const string &parent,
			      const type_defn::type_type &typ,
			      bool can_point, bool in_lib,
			      pdvector<arg*> *arglist=NULL, const string &ignore_text="",
			      const string &bundle_name="");
   static string allocate_type(const string &name, const bool isClass,
                               const type_defn::type_type &typ,
                               bool can_point, bool in_lib);
                               

  static string add_type(const string name,
                        const bool is_class,
                        const bool is_abstract,
			            const bool is_derived,
                        const bool is_virtual,
                        const string parent,
			 const type_defn::type_type &type, 
			 const bool can_point, const bool in_lib, 
			 pdvector<arg*> *arglist=NULL, const string ignore="",
			 const string bundler_name="");
   static string add_type(const string &name, bool is_class,
                          const type_defn::type_type &type, 
                          const bool can_point, const bool in_lib);


  static string obj() { return (string("obj"));}
  static string obj_ptr() { return (string("obj->") + Options::ml->dir_field());}
  static string gen_name();
  static string set_dir_encode();
  static string set_dir_decode();
  static string type_class() {return (string("T_") + file_base_);}
  static string type_prefix() { return (type_class() + "::");}
  static bool types_defined(const string name) {
    return (all_types.defines(name) || all_types.defines(type_prefix()+name));
  }
  static string get_type(const string name) {
    if (all_types.defines(name))
      return name;
    else if (all_types.defines(type_prefix()+name))
      return (type_prefix()+name);
    else {
      abort();
      return((char *)NULL); // some compilers complain if we don't return a value
    }
  }
  static bool stl_seen;
  static bool dont_gen_handle_err;

private:
  static string file_base_;
  static string input_file_;
  static bool profile_;
  static bool shortnames_;
  static mem_type mem_;
  static unsigned var_count_;
};

#endif


