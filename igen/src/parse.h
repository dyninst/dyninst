/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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

#ifndef PARSE_H
#define PARSE_H

/*
 * Parse.h - define the classes that are used in parsing an interface.
 */

#if defined(i386_unknown_nt4_0)
// XXX kludge for bison.simple
#include <malloc.h>
#define alloca _alloca
#endif

#include "common/h/String.h"
/* trace data streams */
#include "pdutil/h/ByteArray.h"
#include "common/h/Vector.h"
#include "common/h/Dictionary.h"
#include <fstream.h>

extern void dump_to_dot_h(const char*);

// forward decl
class type_defn;
class remote_func;
class signature;
class interface_spec;

class arg {
public:
  // gen_variable()
  arg(const string *type, const unsigned star_count, const bool is_const,
      const string *name, const bool is_ref);
  arg() { }
  ~arg() { }
  bool operator== (const arg &other) const { return (type_ == other.type()); }

  bool is_void() const { return (type_ == "void");}
  string gen_bundler_name() const;
  void gen_bundler(ofstream &outStream, const string obj_name,
		   const string data_name) const;

  string pointers() const { return pointers_; }
  string base_type() const { return type_;}
  string type(const bool use_const=false, const bool use_ref=false) const;
  string name() const { return name_; }
  bool is_const() const { return constant_;}
  bool tag_bundle_send(ofstream &out_stream, const string bundle_value, 
		       const string tag_value, const string return_value) const;
  unsigned stars() const { return stars_;}
  bool is_ref() const { return is_ref_;}
  string deref(const bool local) const;

private:
  string pointers_;
  string type_;
  string name_;
  bool constant_;
  unsigned stars_;
  bool is_ref_;

  bool tag_bundle_send_one(ofstream &out_stream, const string bundle_value, 
		       const string tag_value, const string return_value) const;
  bool tag_bundle_send_many(ofstream &out_stream, const string bundle_value, 
		       const string tag_value, const string return_value) const;
};

class type_defn {
public:
  friend void recursive_dump_kids(const type_defn *from, ofstream &output);

  typedef enum { TYPE_SCALAR, TYPE_COMPLEX } type_type;

  type_defn(string stl_name, string element_name, const unsigned star_count,
	    const bool in_lib);
  type_defn(const string name, const bool is_class, const bool is_abstract,
	    const bool is_derived, const string parent, 
	    const type_type type, vector<arg*> *arglist = NULL, 
	    const bool can_point=false, bool in_lib=false, const string ignore="",
	    const string bundle_name="");
  ~type_defn() { }

  string gen_bundler_name() const;
  string gen_bundler_call(const string obj_name, const string data_name,
			  const unsigned pointer_count) const;
  bool gen_bundler_body(const string bundler_prefix, const string class_prefix,
			ofstream &out_stream) const;
  bool gen_bundler_sig(const bool &print_extern, const string class_prefix,
		       const string &bundler_prefix, ofstream &out_stream) const;
  bool gen_bundler_ptr(const string bundler_prefix, const string class_prefix,
		       ofstream &out_c, ofstream &out_h) const;
  bool gen_class(const string bundler_prefix, ofstream &out_stream);

  string unqual_id() const { return (unqual_name_ + "_id");}
  string qual_id() const;
  string name() const { return name_;}
  string bundle_name() const { return bundle_name_;}
  type_type my_type() const { return my_type_;}
  bool is_in_library() const { return in_lib_;}
  bool operator== (const type_defn &other) const { return (other.name() == name_); }
  bool is_same_type(vector<arg*> *arglist) const;
  string dump_args(const string data_name, const string sep) const;
  void dump_type();
  string unqual_name() const { return unqual_name_;}
  bool is_stl() const { return is_stl_;}
  string prefix() const { return prefix_;}
  bool assign_to(const string prefix, const vector<arg*> &alist, ofstream &out_stream) const;
  bool pointer_used() const { return pointer_used_;}
  void set_pointer_used() { pointer_used_ = true;}
  bool can_point() const { return can_point_;}
  const vector<arg*> &copy_args() const { return (arglist_);}
  string ignore() const { return ignore_;}
  bool is_class() const { return is_class_;}
  bool is_abstract() const { return is_abstract_; }
  bool is_derived() const { return is_derived_;}
  string parent() const { return parent_;}
  void add_kid(const string kid_name);
  bool has_kids() const { return (kids_.size() > 0);}

private:
  type_type my_type_;
  string name_;
  string bundle_name_;
  bool in_lib_;
  vector<arg*> arglist_;
  string unqual_name_;
  bool is_stl_;
  string prefix_;
  bool pointer_used_;
  bool can_point_;
  arg *stl_arg_;
  string ignore_;
  bool is_class_;
  bool is_abstract_;
  bool is_derived_;
  string parent_;
  vector<string> kids_;

  bool gen_bundler_ptr_struct(const string bundler_prefix, const string class_prefix,
		       ofstream &out_c, ofstream &out_h) const;
  bool gen_bundler_ptr_class(const string bundler_prefix, const string class_prefix,
		       ofstream &out_c, ofstream &out_h) const;

  bool gen_bundler_body_class(const string bundler_prefix, const string class_prefix,
			ofstream &out_stream) const;
  bool gen_bundler_body_struct(const string bundler_prefix, const string class_prefix,
			ofstream &out_stream) const;
};

class signature {
public:
  signature(vector<arg*> *alist, const string rf_name);
  ~signature() { }

  string type(const bool use_bool=false) const;
  string base_type() const { return type_;}
  void type(const string t, const unsigned star);
  bool gen_sig(ofstream &out_stream) const;
  bool tag_bundle_send(ofstream &out_stream, const string return_value,
		       const string req_tag) const;
  bool tag_bundle_send_many(ofstream &out_stream, const string return_value,
			    const string req_tag) const;
  bool tag_bundle_send_one(ofstream &out_stream, const string return_value,
			   const string req_tag) const;
  bool arg_struct(ofstream &out_stream) const;
  string dump_args(const string message, const string sep) const;
  string gen_bundler_call(const string obj_name, const string data_name) const;

private:
  vector<arg*> args;
  string type_;
  bool is_const_;
  unsigned stars_;
};

class remote_func { 
public:
  typedef enum { async_upcall,      // from server to client, async
		 sync_call,         // from client to server, sync
		 async_call         // from client to server, async
		 } call_type; 

  remote_func(const string name, vector<arg*> *arglist, const call_type &ct,
	      const bool &is_v, const arg &return_arg, const bool do_free);
  ~remote_func() { }
  bool operator== (const remote_func &other) const { return (other.name() == name_);}

  bool gen_stub(ofstream &out_srvr, ofstream &out_clnt) const;
  bool gen_signature(ofstream &out_stream, const bool &hdr, const bool srvr) const;
  bool gen_async_struct(ofstream &out_stream) const;
  bool save_async_request(ofstream &out_stream, const bool srvr) const;
  bool free_async(ofstream &out_stream, const bool srvr) const;
  bool handle_request(ofstream &out_stream, const bool srvr, bool special=false) const;

  string request_tag(bool unqual=false) const;
  string response_tag(bool unqual=false) const;

  bool is_virtual() const { return is_virtual_;}
  bool is_void() const { return (return_arg_.is_void());}
  bool is_srvr_call() const { return (function_type_ == async_upcall);}
  bool is_async_call() const {
    return ((function_type_ == async_upcall) || (function_type_ == async_call));}

  string name() const { return name_; }
  call_type function_type() const { return function_type_;}
  string return_value() const
    { return ((is_async_call()||is_void()) ? string("") : string("ret_arg"));}
  bool do_free() const { return do_free_;}
  string sig_type(const bool use_const=false) const { return call_sig_.type(use_const);}
  string ret_type(const bool use_const=false) const { return return_arg_.type(use_const);}

private:
  string name_;
  call_type function_type_;
  bool is_virtual_;
  signature call_sig_;
  arg return_arg_;
  bool do_free_;

  bool gen_stub_helper(ofstream &out_srvr, ofstream &out_clnt,
		       const bool server) const;
  bool gen_stub_helper_many(ofstream &out_srvr,
			    ofstream &out_clnt, const bool server) const;
  bool gen_stub_helper_one(ofstream &out_srvr, ofstream &out_clnt,
			   const bool server) const;
};

class interface_spec {
public:
  interface_spec(const string *name, const unsigned &b, const unsigned &v);
  ~interface_spec();

  bool gen_interface() const;

  // TODO reverse arg list ?
  bool new_remote_func(const string *name, vector<arg*> *arglist,
		       const remote_func::call_type &callT,
		       const bool &is_virtual, const arg &return_arg,
		       const bool do_free);
  
  void ignore(bool is_srvr, char *text);
  bool are_bundlers_generated() const;
  string name() const { return name_;}
  unsigned base() const { return base_;}
  unsigned version() const { return version_;}

  string gen_class_name(const bool &server) const { 
    return (server ? server_name_ : client_name_); }
  string gen_class_prefix(const bool &server) const {
    return (server ? server_prefix_ : client_prefix_);}
  bool gen_process_buffered(ofstream &out_stream, const bool &srvr) const;
  bool gen_await_response(ofstream &out_stream, const bool srvr) const;
  bool gen_wait_loop(ofstream &out_stream, const bool srvr) const;
  bool gen_scope(ofstream &out_h, ofstream &out_c) const;
  bool gen_client_verify(ofstream &out_stream) const;
  bool gen_server_verify(ofstream &out_stream) const;

private:
  string name_;
  unsigned base_;
  unsigned version_;
  string client_prefix_;
  string server_prefix_;
  string client_name_;
  string server_name_;

  vector<string> client_ignore;
  vector<string> server_ignore;

  bool gen_stl_temps() const;
  bool gen_stl_bundler(ofstream &out_h, ofstream &out_c) const;
  bool gen_stl_bundler_ptr(ofstream &out_h, ofstream &out_c) const;
  bool gen_header(ofstream &out_stream, const bool &server) const;
  bool gen_inlines(ofstream &out_stream, const bool &server) const;
  bool gen_prelude(ofstream &out_stream, const bool &server) const;
  bool gen_dtor_hdr(ofstream &out_stream, const bool &server, const bool &hdr) const;
  bool gen_ctor_hdr(ofstream &out_stream, const bool &server) const;
  bool gen_dtor_body(ofstream &out_stream, const bool &server) const;
  bool gen_ctor_body(ofstream &out_stream, const bool &server) const;
  bool gen_ctor_helper(ofstream &out_stream, const bool &server) const;
  bool gen_ctor_1(ofstream &out_stream, const bool &server,
		  const bool &hdr) const;
  bool gen_ctor_2(ofstream &out_stream, const bool &server,
		  const bool &hdr) const;
  bool gen_ctor_3(ofstream &out_stream, const bool &server,
		  const bool &hdr) const;
  bool gen_ctor_4(ofstream &out_stream, const bool &server,
		  const bool &hdr) const;

  dictionary_hash<string, remote_func*> all_functions_;
};

class message_layer {
public:
  typedef enum { AS_one, AS_many, AS_none } AS;
  typedef enum { Med_xdr, Med_pvm, Med_thread, Med_other, Med_none, Med_rpc } medium;

  message_layer() { med_ = Med_none; }
  message_layer(const string) { }
  message_layer(const string nm, const medium md, const string bp, const string brt,
		const string mdp, const string mo, const string mop, const AS as,
		const string bfail, const string bok, const string dir_f,
		const string pack_f, const string unpack_f, const string free_c,
		const string rpc_par, const string tag_type,
		const string send_msg, const bool r_used,
		const string skip_msg, const string r_msg, const string incs,
		const bool do_serial, const string enc, const string dec,
		const bool do_skip);
  ~message_layer() { }
  bool operator== (const message_layer &other) const { return (name_ == other.name());}

  string name() const { return name_;}
  medium med() const { return med_;}
  string bundler_prefix() const { return bundler_prefix_;}
  string bundler_return_type() const { return bundler_return_type_;}
  string marshall_data_ptr() const { return marshall_data_ptr_;}
  string marshall_obj() const { return marshall_obj_;}  
  string marshall_obj_ptr() const { return marshall_obj_ptr_;}
  AS address_space() const { return address_space_;}
  string bundle_fail() const { return bundle_fail_;}
  string bundle_ok() const { return bundle_ok_;}
  string dir_field() const { return dir_field_;}
  string pack_const() const { return pack_const_;}
  string unpack_const() const { return unpack_const_;}
  string free_const() const { return free_const_;}
  string rpc_parent() const { return rpc_parent_;}
  string tag_type() const { return tag_type_;}
  string send_message() const { return send_message_;}
  string read_tag(const string obj_name, const string tag) const;
  bool records_used() const { return records_used_;}
  string skip_message() const { return skip_message_;}
  string recv_message() const { return recv_message_;}
  string includes() const { return incs_;}
  bool serial() const { return serial_;}
  string set_dir_decode() const { return decode_;}
  string set_dir_encode() const { return encode_;}
  bool skip() const { return skip_;}

private:
  string name_;
  medium med_;
  string bundler_prefix_;
  string bundler_return_type_;
  string dir_is_free_;
  string marshall_data_ptr_;
  string marshall_obj_;
  string marshall_obj_ptr_;
  AS address_space_;
  string bundle_fail_;
  string bundle_ok_;
  string dir_field_;
  string pack_const_;
  string unpack_const_;
  string free_const_;
  string rpc_parent_;
  string tag_type_;
  string send_message_;
  bool records_used_;
  string skip_message_;
  string recv_message_;
  string incs_;
  bool serial_;
  string encode_;
  string decode_;
  bool skip_;
};

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
  static mem_type mem() { return mem_;}
  static void set_mem(const mem_type m) { mem_ = m;}

  static string error_state(const string err_name, const string return_value);
  static interface_spec *current_interface;
  static dictionary_hash<string, type_defn*> all_types;
  static vector<type_defn*> vec_types;
  static vector<message_layer*> all_ml;
  static message_layer *ml;

  static void ignore(string &s, bool is_server);
  static vector<string> client_ignores;
  static vector<string> server_ignores;
  static vector<string> forward_decls;

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
    vector<el_data> elements;
  } stl_data;
  static vector<stl_data> stl_types;

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
  static string allocate_type(const string name, const bool is_class, const bool is_abstract,
			      const bool is_derived, const string parent,
			      const type_defn::type_type &typ,
			      const bool can_point, const bool &in_lib,
			      vector<arg*> *arglist=NULL, const string ignore_text="",
			      const string bundle_name="");

  static string add_type(const string name, const bool is_class, const bool is_abstract,
			 const bool is_derived, const string parent,
			 const type_defn::type_type &type, 
			 const bool can_point, const bool in_lib,
			 vector<arg*> *arglist=NULL, const string ignore="",
			 const string bundler_name="");

  static string obj() { return (string("obj"));}
  static string obj_ptr() { return (string("obj->") + Options::ml->dir_field());}
  static string gen_name();
  static string set_dir_encode();
  static string set_dir_decode();
  static string type_class() { return (string("T_") + file_base_);}
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
      return(NULL); // some compilers will complain if we don't return a value
    }
  }
  static bool stl_seen;
  static bool dont_gen_handle_err;

private:
  static string file_base_;
  static string input_file_;
  static bool profile_;
  static mem_type mem_;
  static unsigned var_count_;
};


typedef struct functype_data {
  remote_func::call_type call;
  bool is_virtual;
} Type_data;

typedef struct derived_data {
  bool is_derived;
  string *name;
} Derived_data;

typedef struct cl {
  bool b;
  bool abs;
} cl;

union parse_stack {
  string *cp;
  int i;
  unsigned u;
  float f;
  bool b;
  cl class_data;
  arg *args;
  functype_data fd;
  derived_data derived;
  vector<arg*> *arg_vector;
  interface_spec *spec;
  char *charp;
};

#endif
