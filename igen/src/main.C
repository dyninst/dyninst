/*
 * Copyright (c) 1996-1998 Barton P. Miller
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

// $Id: main.C,v 1.52 2000/07/28 17:21:40 pcroth Exp $

#include "parse.h"
#include <iostream.h>
#include <stdio.h>

#include "common/h/Ident.h"
extern "C" const char V_igen[];
Ident V_id(V_igen,"Paradyn");
extern "C" const char V_libpdutil[];
Ident V_Uid(V_libpdutil,"Paradyn");

bool Options::dont_gen_handle_err = false;
dictionary_hash<string, type_defn*> Options::all_types(string::hash);
vector<type_defn*> Options::vec_types;
vector<message_layer*> Options::all_ml;
vector<Options::stl_data> Options::stl_types;
vector<string> Options::forward_decls;

extern FILE *yyin;
extern int yyparse();

static bool set_ml(const string);
static void init_ml();
static void open_interface_file(int argc, char *argv[]);
static void open_output_files();
static void do_opts(int argc, char *argv[]);
static void usage();
static void init_header_files();
static void end_header_files();
static void init_types();

interface_spec *Options::current_interface;

ifstream Options::input;
ofstream Options::dot_h;
ofstream Options::dot_c;
ofstream Options::clnt_dot_h;
ofstream Options::clnt_dot_c;
ofstream Options::srvr_dot_h;
ofstream Options::temp_dot_c;
ofstream Options::srvr_dot_c;
string Options::file_base_;
string Options::input_file_;
bool Options::profile_;
unsigned Options::var_count_ = 0;
Options::mem_type Options::mem_;
message_layer *Options::ml;
bool Options::stl_seen = false;

static void usage() {
  cerr << "igen " << "[-prof] -xdr | -thread | -pvm [-header | -code]\n";
  cerr << "      [-ignore | -detect | -handle] [-input <file>] <fileName>\n";
  cerr << "  CODE OPTIONS\n";
  cerr << "    -xdr     -->  produce code for sockets/xdr\n";
  cerr << "    -pvm     -->  produce code for PVM\n";
  cerr << "    -thread  -->  produce code for threads\n";
  cerr << "    -sas     -->  produce code for unknown threading systems\n";
  cerr << "    -mas     -->  produce code for unknown multi-process message passing\n";
  cerr << "  DATA OPTIONS\n";
  cerr << "    -input <file> --> file that supplies data for unknown systems\n";
  cerr << "  MARSHALLING OPTIONS\n";
  cerr << "     currently unsupported\n";
  cerr << "    -ignore  -->  don't detect pointers to the same memory\n";
  cerr << "    -detect  -->  detect (and assert) duplicate pointers\n";
  cerr << "    -handle  -->  handle duplicate\n";
  cerr << "  PROFILE OPTIONS\n";
  cerr << "     currently unsupported\n";
  cerr << "    -prof    -->  generate profile code\n";
}

static void do_opts(int argc, char *argv[]) {
  Options::current_interface = NULL;

  for (int i=0; i<argc; i++) {
    if (!strcmp("-pvm", argv[i])) {
      if (!set_ml("pvm")) {
	cerr << "Message layer 'pvm' unknown\n";
	exit(-1);
      }
    } else if (!strcmp("-no_err", argv[i])) {
      Options::dont_gen_handle_err = true;
    } else if (!strcmp("-xdr", argv[i])) {
      if (!set_ml("xdr")) {
	cerr << "Message layer 'xdr' unknown\n";
	exit(-1);
      }
    } else if (!strcmp("-rpc", argv[i])) {
      if (!set_ml("rpc")) {
	cerr << "Message layer 'rpc' unknown\n";
	exit(-1);
      }
    } else if (!strcmp("-sas", argv[i])) {

    } else if (!strcmp("-mas", argv[i])) {

    } else if (!strcmp("-prof", argv[i])) {
      cerr << "-prof is currently unsupported, bye\n"; exit(-1);
    } else if (!strcmp("-thread", argv[i])) {
      if (!set_ml("thread")) {
	cerr << "Message layer 'thread' unknown\n";
	exit(-1);
      }
    } else if (!strcmp("-ignore", argv[i])) {
      cerr << "-ignore is currently unsupported, bye\n"; exit(-1);
    } else if (!strcmp("-detect", argv[i])) {
      cerr << "-detect is currently unsupported, bye\n"; exit(-1);
    } else if (!strcmp("-handle", argv[i])) {
      cerr << "-handle is currently unsupported, bye\n"; exit(-1);
    } else if (!strcmp("-input", argv[i])) {
      if (!argv[i+1]) {
	cerr << "-input specified with no filename, goodbye!\n";
	exit(-1);
      }
      Options::set_input_file(argv[i+1]);
    } else if (!strcmp("-V", argv[i])) {
        cerr << V_igen << endl;         // version information
    } else if (!strcmp("-usage", argv[i]) || !strcmp("-help",argv[i])) {
        usage();
        exit(0);
    } else if (!strncmp("-", argv[i], 1)) {
        cerr << "Unexpected flag: " << argv[i] << endl;
        usage();
        exit(-1);
    }
  }

  if ((Options::ml==NULL) || (Options::ml->med() == message_layer::Med_none)) {
    cerr << "No message layer defined\n";
    usage();
    exit(-1);
  }

  char *buffer = strdup(argv[argc-1]);
  char *temp = strrchr(buffer, '/');
  if (temp)
    temp++;
  else
    temp = buffer;

  char *temp2 = strstr(temp, ".I");
  if (!temp2) {
    cerr << "The file " << argv[argc-1] << " must end with the suffix '.I' \n";
    usage();
    exit(-1);
  }
  *(temp2) = '\0';
  Options::set_file_base(temp);
  free(buffer);
  
  if ((Options::input_file()).length()) {
    Options::input.open((Options::input_file()).string_of(), ios::in);
    if (!Options::input.good()) {
      cerr << "Could not open " << Options::input_file() << " for input, goodbye\n";
      usage();
      exit(-1);
    }
  }
}

static void open_interface_file(int argc, char *argv[]) {

  if (argc < 2) {
    cerr << "Not enough arguments\n";
    usage();
    exit(-1);
  }

  if (!(yyin = fopen(argv[argc-1], "r"))) {
    cerr << "Unable to open " << argv[argc-1] << endl;
    usage();
    exit(-1);
  }
}

static void open_output_files() {
  string base(Options::file_base() + "." + Options::ml->name() + ".");
  string cpp_base(Options::file_base() + "_" + Options::ml->name() + "_");

  string dump_to = base + "temp.C";
  Options::temp_dot_c.open(dump_to.string_of(), ios::out);
  if (!Options::temp_dot_c.good()) {
    cerr << "Could not open " << dump_to << " for output\n";
    exit(-1);
  }

  dump_to = base + "h";
  Options::dot_h.open(dump_to.string_of(), ios::out);
  if (!Options::dot_h.good()) {
    cerr << "Could not open " << dump_to << " for output\n";
    exit(-1);
  }

  dump_to = base + "C";
  Options::dot_c.open(dump_to.string_of(), ios::out);
  if (!Options::dot_c.good()) {
    cerr << "Could not open " << dump_to << " for output\n";
    exit(-1);
  }

  dump_to = base + "SRVR.h";
  Options::srvr_dot_h.open(dump_to.string_of(), ios::out);
  if (!Options::srvr_dot_h.good()) {
    cerr << "Could not open " << dump_to << " for output\n";
    exit(-1);
  }

  dump_to = base + "SRVR.C";
  Options::srvr_dot_c.open(dump_to.string_of(), ios::out);
  if (!Options::srvr_dot_c.good()) {
    cerr << "Could not open " << dump_to << " for output\n";
    exit(-1);
  }

  dump_to = base + "CLNT.C";
  Options::clnt_dot_c.open(dump_to.string_of(), ios::out);
  if (!Options::clnt_dot_c.good()) {
    cerr << "Could not open " << dump_to << " for output\n";
    exit(-1);
  }

  dump_to = base + "CLNT.h";
  Options::clnt_dot_h.open(dump_to.string_of(), ios::out);
  if (!Options::clnt_dot_h.good()) {
    cerr << "Could not open " << dump_to << " for output\n";
    exit(-1);
  }
}

static void init_header_files() {
  string base(Options::file_base() + "." + Options::ml->name() + ".");
  string cpp_base(Options::file_base() + "_" + Options::ml->name() + "_");
  string ident("// Automatically generated by " 
        + V_id.Suite() + "/" + V_id.Component() + " "
        + V_id.Release() + V_id.BuildNum() + V_id.Revision());

  Options::temp_dot_c << ident << endl;
  Options::temp_dot_c << "#pragma implementation \"Vector.h\"\n";
  Options::temp_dot_c << "#include \"common/h/Vector.h\"\n";
//  Options::temp_dot_c << "#pragma implementation \"Queue.h\"\n";
//  Options::temp_dot_c << "#include \"common/h/Queue.h\"\n";
  Options::temp_dot_c << "#pragma implementation \"" << base << "h\"\n";
  Options::temp_dot_c << "#include \"" << base << "h\"\n";
  Options::temp_dot_c << "#include \"common/h/String.h\"\n";
/* trace data streams */
  Options::temp_dot_c << "#include \"pdutil/h/ByteArray.h\"\n";

  Options::dot_h << ident << endl;
  Options::dot_h << "#ifndef " << cpp_base << "BASE_H\n";
  Options::dot_h << "#define " << cpp_base << "BASE_H\n";

  Options::dot_h << "#include \"pdutil/h/rpcUtil.h\"\n";
  Options::dot_h << Options::ml->includes() << endl;

  Options::dot_h << "#ifdef IGEN_ERR_ASSERT_ON\n";
  Options::dot_h << "#ifndef IGEN_ERR_ASSERT\n";
  Options::dot_h << "#define IGEN_ERR_ASSERT assert(0)\n";
  Options::dot_h << "#endif\n";
  Options::dot_h << "#else\n";
  Options::dot_h << "#define IGEN_ERR_ASSERT\n";
  Options::dot_h << "#endif\n\n";

  Options::dot_h << "\n // Errors that can occur internal to igen\n";
  Options::dot_h << "#ifndef _IGEN_ERR_DEF\n";
  Options::dot_h << "#define _IGEN_ERR_DEF\n";
  Options::dot_h << "typedef enum e_IGEN_ERR {\n";
  Options::dot_h << "                       igen_no_err=0,\n";
  Options::dot_h << "                       igen_encode_err,\n";
  Options::dot_h << "                       igen_decode_err,\n";
  Options::dot_h << "                       igen_send_err,\n";
  Options::dot_h << "                       igen_read_err,\n";
  Options::dot_h << "                       igen_request_err,\n";
  Options::dot_h << "                       igen_call_err,\n";
  Options::dot_h << "                       igen_proto_err\n";
  Options::dot_h << "                       }  IGEN_ERR;\n\n";
  Options::dot_h << "#endif\n";

  Options::dot_c << ident << endl;
  Options::dot_c << "#include \"" << base << "h\"\n";

  Options::clnt_dot_h << ident << endl;
  Options::clnt_dot_h << "#ifndef " << cpp_base << "CLNT_H\n";
  Options::clnt_dot_h << "#define " << cpp_base << "CLNT_H\n";
  Options::clnt_dot_h << "#include \"" << base << "h\"\n";

//  Options::clnt_dot_h << "#include \"common/h/Queue.h\"\n";

  Options::srvr_dot_h << ident << endl;
  Options::srvr_dot_h << "#ifndef " << cpp_base << "SRVR_H\n";
  Options::srvr_dot_h << "#define " << cpp_base << "SRVR_H\n";
  Options::srvr_dot_h << "#include \"" << base << "h\"\n";
//  Options::srvr_dot_h << "#include \"common/h/Queue.h\"\n";

  Options::clnt_dot_c << ident << endl;
  Options::clnt_dot_c << "#include \"" << base << "CLNT.h\"\n";
  Options::srvr_dot_c << ident << endl;
  Options::srvr_dot_c << "#include \"" << base << "SRVR.h\"\n";
}

static void end_header_files() {
  Options::dot_h << "\n#endif\n";
  Options::dot_h << endl;

  Options::clnt_dot_h << "#endif\n" << endl;
  Options::srvr_dot_h << "#endif\n" << endl;
}

static void close_files() {
  Options::srvr_dot_h.close();
  Options::clnt_dot_h.close();
  Options::dot_c.close();
  Options::dot_h.close();

  Options::srvr_dot_c.close();
  Options::clnt_dot_c.close();
}

static void init_types() {
  Options::stl_data sd;
  sd.name = "vector"; sd.include_file = "common/h/Vector.h";
  sd.need_include = false; sd.pragma_name = "Vector.h";
  Options::stl_types += sd;
  Options::el_data el;
  el.name = "string"; el.type = "string"; el.stars = 0;
  Options::stl_types[0].elements += el;

  Options::add_type("int", false, false, false, "", type_defn::TYPE_SCALAR, false,
		    true, NULL);
  Options::add_type("double", false, false, false, "", type_defn::TYPE_SCALAR, false,
		    true, NULL);
  Options::add_type("void", false, false, false, "", type_defn::TYPE_SCALAR, false, true,
		    NULL);
  Options::add_type("bool", false, false, false, "", type_defn::TYPE_SCALAR, false, true,
		    NULL, "", "Boolean");
  Options::add_type("u_int", false, false, false, "", type_defn::TYPE_SCALAR, false, true,
		    NULL);
  Options::add_type("float", false, false, false, "", type_defn::TYPE_SCALAR, false, true,
		    NULL);
  Options::add_type("u_char", false, false, false, "", type_defn::TYPE_SCALAR, false, true,
		    NULL);
  Options::add_type("char", false, false, false, "", type_defn::TYPE_SCALAR, false, true,
		    NULL);
  Options::add_type("string", false, false, false, "", type_defn::TYPE_SCALAR, false, true,
		    NULL, "", "string_pd");
/* trace data streams */
  Options::add_type("byteArray", false, false, false, "", type_defn::TYPE_SCALAR, false, true,
                    NULL, "", "byteArray_pd");
}

static bool set_ml(const string ml_name) {
  for (unsigned i=0; i<Options::all_ml.size(); i++) {
    if (Options::all_ml[i]->name() == ml_name) {
      Options::ml = Options::all_ml[i];
      return true;
    }
  }
  return false;
}

static void init_ml() {
  message_layer * xdr_ml = new message_layer("xdr",
					     message_layer::Med_xdr,
					     "P_xdr_",
					     "bool_t",
					     "*",
					     "XDR",
					     "*",
					     message_layer::AS_many,
					     "FALSE",
					     "TRUE",
					     "x_op",
					     "XDR_ENCODE",
					     "XDR_DECODE", 
					     "XDR_FREE",
					     "XDRrpc",
						 "unsigned int",
					     "P_xdrrec_endofrecord(net_obj(), TRUE)",
					     true,
					     "P_xdrrec_skiprecord(net_obj())",
					     "XXX_RECV",
					     " ",
					     true,
					     "setDirEncode()",
					     "setDirDecode()",
					     true);
  
  message_layer * thrd_ml = new message_layer("thread",
					      message_layer::Med_thread,
					      "P_thread_",
					      "int",
					      "*",
					      "XDR",
					      "*",
					      message_layer::AS_one,
					      "THR_ERR",
					      "THR_OKAY",
					      "x_op",
					      "XX_ENCODE",
					      "XX_DECODE", 
					      "XX_FREE",
					      "THREADrpc",
						  "tag_t",
					      "msg_send",
					      true,
					      "XXX_skip",
					      "msg_recv",
				      "#include \"thread/h/thread.h\"\n",
					      false,
					      " ",
					      " ",
					      false);


  message_layer * rpc_ml = new message_layer("rpc",
					     message_layer::Med_rpc,
					     "P_rpc_",
					     "bool",
					     "*",
					     "RPC",
					     "*",
					     message_layer::AS_many,
					     "false",
					     "true",
					     "x_op",
					     "RPC::ENCODE",
					     "RPC::DECODE", 
					     "RPC::FREE",
					     "RPCrpc",
					     "unsigned",
					     "P_xdrrec_endofrecord(net_obj(), TRUE)",
					     true,
					     "P_xdrrec_skiprecord(net_obj())",
					     "XXX_RECV",
					     " ",
					     true,
					     "setDirEncode()",
					     "setDirDecode()",
					     true);

  /*
  message_layer pvm_ml("pvm", message_layer::Med_pvm, "pvm_", "bool_t", "*", "XDR", "*",
		       message_layer::AS_many, "FALSE", "TRUE");
  message_layer tcp_ml("tcp", message_layer::Med_other, "", "bool_t", "*", "XDR", "*",
		       message_layer::AS_many, "false", "true");
		       */
  Options::all_ml += xdr_ml; Options::all_ml += thrd_ml; Options::all_ml += rpc_ml;
}

int main(int argc, char *argv[]) {
  init_ml();
  init_types();
  do_opts(argc, argv);
  open_interface_file(argc, argv);
  open_output_files();
  init_header_files();
  yyparse();

  if (!Options::current_interface) {
    cerr << "no interface defined\n";
    exit(-1);
  }

  Options::current_interface->gen_interface();
  end_header_files();
  close_files();
  return(0);
}

void dump_to_dot_h(const char *msg) {
  Options::dot_h << msg << endl;
}

string Options::make_ptrs(unsigned count) {
  static char buffer[100];
  assert(count < 100);

  buffer[0] = (char)0;
  for (unsigned i=0; i<count; i++)
    buffer[i] = '*';
  buffer[count] = '\0';
  return buffer;
}

string arg::deref(const bool use_ref) const {
  if (stars_)
    return pointers_;
  else if (is_ref_ && use_ref)
    return "&";
  else
    return "";
}

string arg::type(const bool use_const, const bool local) const {
  if (use_const)
    return ((constant_ ? string("const ") : string("")) + type_ + deref(local));
  else
    return (type_ + deref(local));
}

string arg::gen_bundler_name() const {
  string suffix;
  switch (stars_) {
  case 0: break;
  case 1: suffix = "_PTR"; break;
  default: abort();
  }
  return ((Options::all_types[base_type()])->gen_bundler_name() + suffix);
}


void arg::gen_bundler(ofstream &out_stream, const string obj_name,
		      const string data_name) const
{
  out_stream <<
    (Options::all_types[base_type()])->gen_bundler_call(obj_name,
							data_name+name_, stars_);
}

arg::arg(const string *type, const unsigned star_count,
	 const bool b, const string *name, const bool is_ref) 
: pointers_(Options::make_ptrs(star_count)), type_(*type),
  constant_(b), stars_(star_count), is_ref_(is_ref) {

  if(is_ref_ && star_count) {
    cerr << "Cannot use pointers with a reference, good bye\n";
    exit(0);
  }

  if (name)
    name_ = *name;
  else
    name_ = Options::gen_name();

  if (!(Options::all_types[type_])->can_point() &&
      (star_count>1) &&
      (Options::ml->address_space() == message_layer::AS_many)) {
    cerr << "Sorry, pointers not handled for this type, goodbye\n";
    cerr << "Type = " << *type << "  name = " << *name << endl;
    exit(-1);
  }
  if (star_count)
    (Options::all_types[type_])->set_pointer_used();
}

bool arg::tag_bundle_send(ofstream &out_stream, const string bundle_val,
			  const string tag_val, const string return_value) const
{
  if (Options::ml->address_space() == message_layer::AS_many)
    return (tag_bundle_send_many(out_stream, bundle_val,
				 tag_val, return_value));
  else
    return (tag_bundle_send_one(out_stream, bundle_val,
				tag_val, return_value));
}

bool arg::tag_bundle_send_many(ofstream &out_stream, const string bundle_val,
			       const string tag_val, const string return_value) const
{
  // set direction encode
  out_stream << Options::set_dir_encode() << ";\n";

  // bundle individual args
  out_stream << "if (!" << Options::ml->read_tag("net_obj()", tag_val) << "\n";

  if (type_ != "void") {
    out_stream << " || !" <<
      (Options::all_types[type_])->gen_bundler_call("net_obj()",
						    string("&")+bundle_val, stars_)
	<< "\n";
  }
  
  out_stream << ") ";
  out_stream << Options::error_state("igen_encode_err", return_value);

  // send message
  out_stream << "if (!" << Options::ml->send_message() << ") ";
  out_stream << Options::error_state("igen_send_err", return_value);

  return true;
}

bool arg::tag_bundle_send_one(ofstream &, const string,
			  const string, const string) const 
{
  abort();
  return false;
}

string type_defn::qual_id() const { return (Options::type_prefix()+unqual_id());}

bool type_defn::assign_to(const string prefix, const vector<arg*> &alist,
			  ofstream &out_stream) const
{
  assert (alist.size() == arglist_.size());
  for (unsigned i=0; i<alist.size(); i++) 
    out_stream << prefix << arglist_[i]->name() << " = " << alist[i]->name() << ";\n";
  return true;
}

bool type_defn::is_same_type(vector<arg*> *arglist) const {
  assert(arglist);
  if (arglist_.size() != arglist->size())
    return false;

  for (unsigned i=0; i<arglist_.size(); i++) {
    if (arglist_[i]->type() != (*arglist)[i]->type())
      return false;
  }
  return true;
}

string type_defn::gen_bundler_name() const {
  return (prefix_ +
	  Options::ml->bundler_prefix() +
	  bundle_name_);
}

string type_defn::gen_bundler_call(const string obj, const string data,
				   const unsigned pointer_count) const
{
  string pointer_suffix;
  switch (pointer_count) {
  case 0: pointer_suffix = ""; break;
  case 1: pointer_suffix = "_PTR"; break;
  default: assert(0);
  }

  if (!is_stl())
    return (gen_bundler_name() + pointer_suffix + "(" + obj + ", " + data + ")");
  else 
    return (gen_bundler_name() + pointer_suffix + "(" + obj + ", " + data + ", " +
	    stl_arg_->gen_bundler_name() + ", (" + stl_arg_->type() + "*)NULL)");
}

string type_defn::dump_args(const string data_name, const string sep) const {
  string ret("");

  if (my_type_ == type_defn::TYPE_SCALAR) {
    if (name_ != "void")
      ret = data_name;
  } else if (is_stl()) {
    ret = data_name;
  } else {
    switch (arglist_.size()) {
    case 0:
      break;
    case 1:
      ret = data_name;
      break;
    default:
      for (unsigned i=0; i<arglist_.size(); i++) {
	ret += string(data_name + sep + arglist_[i]->name());
	if (i < (arglist_.size() - 1))
	  ret += string(", ");
      }
      break;
    }
  }
  return ret;
}

type_defn::type_defn(string stl_name, string element_name, const unsigned ct,
		     const bool in_lib)
: my_type_(type_defn::TYPE_COMPLEX), 
  in_lib_(in_lib), is_stl_(true),
  pointer_used_(false), can_point_(true), is_class_(false), is_derived_(false) 
{
  string ptrs = Options::make_ptrs(ct);
  string waste="dummy";
  name_ = stl_name + "<" + element_name + ptrs + ">";
  stl_arg_ = new arg(&element_name, ct, false, &waste, false);
  if (Options::all_types.defines(name_)) {
    cerr << name_ << " is already defined\n";
    exit(-1);
  }
  prefix_ = Options::type_class() + "_";
  unqual_name_ = name_;
  bundle_name_ = "stl";
}

static string process_ignore(const string txt) {
  if (txt.length() < 17) return "";
  char *buffer = P_strdup(txt.string_of());
  char *temp = buffer;

  temp[strlen(temp) - 8] = (char) 0;
  temp += 8;
  string s = temp;
  free(buffer);
  return s;
}

type_defn::type_defn(const string name, const bool is_cl, const bool is_abs,
		     const bool is_der, const string par, 
		     const type_type type,
		     vector<arg*> *arglist, const bool can_point, 
		     bool in_lib, const string ignore, const string bundle_name)
: my_type_(type), bundle_name_(bundle_name), in_lib_(in_lib),
  is_stl_(false), pointer_used_(false), can_point_(can_point),
  ignore_(process_ignore(ignore)),
  is_class_(is_cl), is_abstract_(is_abs), is_derived_(is_der) 
{
  stl_arg_ = NULL;
  if (Options::all_types.defines(name)) {
    cerr << name << " is already defined\n";
    exit(-1);
  }
  if (in_lib || Options::ml->AS_one) {
    name_ = name;
    prefix_ = "";
  } else {
    name_ = Options::type_prefix() + name;
    prefix_ = Options::type_prefix();
  }
  unqual_name_ = name;

  if (!bundle_name_.length())
    bundle_name_ = unqual_name_;
  
  if (arglist) {
    for (unsigned i=0; i< arglist->size(); i++)
      arglist_ += (*arglist)[i];
  }
  if (is_derived_) {
    parent_ = Options::type_prefix() + par;
    if (!Options::all_types.defines(parent_)) {
      cerr << "Parent " << par << " not defined\n";
      exit(0);
    } else {
      type_defn *p = Options::all_types[parent_];
      assert(p);
      p->add_kid(name_);
    }
  }
}

void type_defn::add_kid(const string kid_name) { kids_ += kid_name; }

/* Convert a type name with qualified names to a names with unqualified names.
   E.g. vector<T_dyninstRPC::mdl_expr*> to vector<mdl_expr*>
   This is needed to compile the igen output with Visual C++, which
   does not accept the qualified names (in the example above, it complains
   that T_dyninstRPC is undefined).
*/
string unqual_type(const string &type) {
  string ret;
  const char *t = type.string_of();

  while (1) {
    const char *p = strstr(t, Options::type_prefix().string_of());
    if (!p) {
      ret += t;
      return ret;
    }
    ret += string(t, p-t);
    t = p + Options::type_prefix().length();
  }
}


bool type_defn::gen_class(const string, ofstream &out_stream) {

  if (is_class())
    out_stream << "class "; 
  else
    out_stream << "struct ";
  out_stream << unqual_name();
  if (is_derived()) 
    out_stream << " : public " << Options::qual_to_unqual(parent()) << " ";
  out_stream << "{ \n";
  if (is_class()) {
    out_stream << " public: \n ";
    out_stream << unqual_name() << "();\n";
  }
  
  for (unsigned i=0; i<arglist_.size(); i++) 
    out_stream << unqual_type(arglist_[i]->type(true)) << " " << arglist_[i]->name() << ";\n";

  if (ignore_.length())
    out_stream << "\n" << ignore_ << endl;

  if (is_class()) {
    out_stream << "public:\n";
    if (has_kids())
      out_stream << "virtual ";
    out_stream << Options::ml->bundler_return_type() << " marshall "
      << "(" << Options::ml->marshall_obj()
	<< " " << Options::ml->marshall_obj_ptr() << " obj);\n";
    if (has_kids())
      out_stream << "virtual ";
    out_stream << "class_ids getId() { return " << unqual_id() << ";}\n";
  }

  out_stream << "};\n";

  if (!is_class()) 
    out_stream << "typedef struct " << unqual_name() << " " << unqual_name()
      << ";\n";
  return true;
}

bool type_defn::gen_bundler_ptr(const string class_prefix, const string bundler_prefix,
				ofstream &out_c, ofstream &out_h) const 
{
  bool do_it = false;
  if (pointer_used())
    do_it = true;
  else if (is_class()) {
    const type_defn *curr_defn = this;
    while (curr_defn->is_derived()) {
      if (!Options::all_types.defines(curr_defn->parent())) {
	cerr << "Parent " << curr_defn->parent() << " should be defined\n";
	assert(0);
      }
      curr_defn = Options::all_types[curr_defn->parent()];
      if (curr_defn->pointer_used()) {
	do_it = true;
	break;
      }
    }
  }

  // if (!do_it)
  // return true;

  if (is_class()) 
    return (gen_bundler_ptr_class(class_prefix, bundler_prefix, out_c, out_h));
  else
    return (gen_bundler_ptr_struct(class_prefix, bundler_prefix, out_c, out_h));
}

bool type_defn::gen_bundler_ptr_struct(const string class_prefix, const string bundler_prefix,
				ofstream &out_c, ofstream &out_h) const 
{
  out_h << "static " << Options::ml->bundler_return_type() << " "
    << bundler_prefix << unqual_name() << "_PTR" << "(" << Options::ml->marshall_obj()
      << " " << Options::ml->marshall_obj_ptr() << " obj, "
	<< unqual_name() << Options::ml->marshall_data_ptr() << "*"
	  << " data);\n";

  out_c << Options::ml->bundler_return_type() << " "
    << class_prefix << bundler_prefix << unqual_name() << "_PTR"
      << "(" << Options::ml->marshall_obj()
	<< " " << Options::ml->marshall_obj_ptr() << " obj, "
	  << name() << Options::ml->marshall_data_ptr() << "*" << " data) { \n";
  out_c << "assert(obj);\n";
  out_c << "assert(" << Options::obj_ptr() << " != " << Options::ml->free_const() << ");\n";
  out_c << "bool is_null(false);\n";
  out_c << "switch (" << Options::obj_ptr() << ") {\n";
  out_c << "case " << Options::ml->pack_const() << ":\n";
  out_c << "if (!*data) is_null=true;\n";
  out_c << "if (!" << Options::all_types["bool"]->gen_bundler_call("obj", "&is_null", 0)
    << ") return " << Options::ml->bundle_fail() << ";\n";
  out_c << "if (!*data) return " << Options::ml->bundle_ok() << ";\n";
  out_c << "break;\n";
  out_c << "case " << Options::ml->unpack_const() << ":\n";
  out_c << "if (!" << Options::all_types["bool"]->gen_bundler_call("obj", "&is_null", 0)
    << ") return " << Options::ml->bundle_fail() << ";\n";
  out_c << "if (is_null) { *data = NULL; return " << Options::ml->bundle_ok() << ";}\n";
  out_c << "*data = new " << name() << ";\n";
  out_c << "if (!*data) return " << Options::ml->bundle_fail() << ";\n";
  out_c << "break;\n";
  out_c << "default: \n return " << Options::ml->bundle_fail() << ";\n";
  out_c << "}\n";
  out_c << "return " << class_prefix << bundler_prefix << unqual_name() << "(obj, *data);\n";
  out_c << "}\n";
  return true;
}

bool type_defn::gen_bundler_ptr_class(const string class_prefix, const string bundler_prefix,
				ofstream &out_c, ofstream &out_h) const 
{
  out_h << "static " << Options::ml->bundler_return_type() << " "
    << bundler_prefix << unqual_name() << "_PTR" << "(" << Options::ml->marshall_obj()
      << " " << Options::ml->marshall_obj_ptr() << " obj, "
	<< unqual_name() << Options::ml->marshall_data_ptr() << "*" << " data);\n";

  out_c << Options::ml->bundler_return_type() << " "  << class_prefix
    << bundler_prefix << unqual_name() << "_PTR" << "("
      << Options::ml->marshall_obj() << " " << Options::ml->marshall_obj_ptr()
	<< " obj, ";
  out_c << class_prefix << unqual_name() << Options::ml->marshall_data_ptr() << "*" 
    << " data) {\n";
  out_c << "assert(obj);\n";
  out_c << "assert(" << Options::obj_ptr() << " != " << Options::ml->free_const()
    << ");\n";
  out_c << "bool is_null = (*data == NULL);\n";
  out_c << "unsigned tag;\n";
  out_c << "if (" << Options::obj_ptr() << " == " << Options::ml->pack_const()
    << ") {\n";
  out_c << "if (!*data) tag = " << qual_id() << ";\n";
  out_c << "else tag = (unsigned) (*data)->getId();\n";
  out_c << "}\n";
  out_c << "if (!" << Options::all_types["bool"]->gen_bundler_call("obj", 
								   "&is_null",
								   0)
    << ") return " << Options::ml->bundle_fail() << ";\n";
  out_c << "if (!" << Options::all_types["u_int"]->gen_bundler_call("obj",
								    "&tag",
								    0)
    << ") return " << Options::ml->bundle_fail() << ";\n";

  out_c << "if (" << Options::obj_ptr() << " == " << Options::ml->unpack_const()
    << ") {\n";
  out_c << "if (is_null) { *data = NULL; return " << Options::ml->bundle_ok() << ";}\n";
  out_c << "switch (tag) {\n";
  if (!is_abstract()) {
    out_c << "case " << qual_id() << ":\n";
    out_c << "*data = new " << name() << ";\n";
    out_c << "break;\n";
  }
  
  recursive_dump_kids(this, out_c);

  out_c << "default: \n return " << Options::ml->bundle_fail() << ";\n";
  out_c << "};\n";
  out_c << "if (!*data) return " << Options::ml->bundle_fail() << ";\n";
  out_c << "} else if (" << Options::obj_ptr() << " == "
    << Options::ml->pack_const() << ") {\n";
  out_c << "if (!*data) return " << Options::ml->bundle_ok() << ";\n";
  out_c << "} else  \n return " << Options::ml->bundle_fail() << ";\n";
  out_c << "return ((*data)->marshall(obj));\n";
  out_c << "}\n";

  out_c << Options::ml->bundler_return_type() << " " 
    << name() << "::"
      << "marshall("
	<< Options::ml->marshall_obj()
	  << " " << Options::ml->marshall_obj_ptr() << " obj) {\n";

  for (unsigned i=0; i<arglist_.size(); i++) {
    out_c << " if (!";
    arglist_[i]->gen_bundler(out_c, "obj", "&");
    out_c << ")\n";
    out_c << "return " << Options::ml->bundle_fail() << ";\n";
  }
  const type_defn *curr_defn = this;
  while (curr_defn->is_derived()) {
    if (!Options::all_types.defines(curr_defn->parent())) {
      cerr << "Parent " << curr_defn->parent() << " should be defined\n";
      assert(0);
    }
    curr_defn = Options::all_types[curr_defn->parent()];
    assert(curr_defn);
    for (unsigned ui=0; ui<curr_defn->arglist_.size(); ui++) {
      out_c << " if (!";
      arglist_[ui]->gen_bundler(out_c, "obj", "&");
      out_c << ")\n";
      out_c << "return " << Options::ml->bundle_fail() << ";\n";
    }
  }
  out_c << "return " << Options::ml->bundle_ok() << " ;\n";
  out_c << "}\n";
  return true;
}

bool type_defn::gen_bundler_body(const string bundler_prefix, const string class_prefix,
				 ofstream &out_stream) const
{
  if (is_class()) 
    return (gen_bundler_body_class(bundler_prefix, class_prefix, out_stream));
  else
    return (gen_bundler_body_struct(bundler_prefix, class_prefix, out_stream));
}
				 
bool type_defn::gen_bundler_body_struct(const string bundler_prefix, const string class_prefix,
				 ofstream &out_stream) const
{
  gen_bundler_sig(false, class_prefix, bundler_prefix, out_stream);
  out_stream << "{\n";
  out_stream << "assert(obj);\n";
  out_stream << "assert(" << Options::obj_ptr() << " != "
    << Options::ml->free_const() << ");\n";

  for (unsigned i=0; i<arglist_.size(); i++) {
    out_stream << " if (!";
    arglist_[i]->gen_bundler(out_stream, "obj", "&data->");
    out_stream << ")\n";
    out_stream << "return " << Options::ml->bundle_fail() << ";\n";
  }
  
  out_stream << "return " << Options::ml->bundle_ok() << " ;\n}\n";
  return true;
}

bool type_defn::gen_bundler_body_class(const string bundler_prefix, const string class_prefix,
				 ofstream &out_stream) const
{
  gen_bundler_sig(false, class_prefix, bundler_prefix, out_stream);
  out_stream << "{\n";
  out_stream << "assert(obj);\n";
  out_stream << "assert(" << Options::obj_ptr() << " != "
    << Options::ml->free_const() << ");\n";
  out_stream << "return (data->marshall(obj));\n";
  out_stream << "}\n";
  return true;
}

bool type_defn::gen_bundler_sig(const bool &print_extern, const string class_prefix,
				const string &bundler_prefix,
				ofstream &out_stream) const
{
  if (print_extern) 
    out_stream << "static ";

  out_stream << Options::ml->bundler_return_type() << " ";

  if (!print_extern) 
    out_stream << class_prefix;

  out_stream << bundler_prefix
    << unqual_name() << "(" << Options::ml->marshall_obj() << " "
      << Options::ml->marshall_obj_ptr() << " obj, ";

  if (!print_extern) 
    out_stream << class_prefix;

  out_stream << unqual_name() << Options::ml->marshall_data_ptr() << " data)";
    
  if (print_extern)
    out_stream << ";";
  out_stream << endl;
  return true;
}

void type_defn::dump_type() {
  cerr << "Type " << name_ << "  in_lib=" << in_lib_ << endl;
  for (unsigned i=0; i<arglist_.size(); i++)
    cerr << "   " << arglist_[i]->type() << "    "  << arglist_[i]->name() << endl;
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

string Options::error_state(const string err_name, const string return_value) {
  return (string("{\n ") +
	  "IGEN_ERR_ASSERT;\n " +
	  "set_err_state(" + err_name + ");\n " +
	  "handle_error();\n " +
	  "return " + return_value + ";\n " +
	  "}\n");
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

string Options::allocate_type(const string name, const bool is_class, const bool is_abstract,
			      const bool is_derived, const string parent,
			      const type_defn::type_type &typ,
			      const bool can_point, const bool &in_lib,
			      vector<arg*> *arglist, const string ignore_text,
			      const string bundle_name) {
  return (Options::add_type(name, is_class, is_abstract, is_derived, parent, typ, can_point,
			    in_lib, arglist, ignore_text, bundle_name));
}


string Options::add_type(const string name, const bool is_class, const bool is_abstract,
			 const bool is_derived, const string parent,
			 const type_defn::type_type &type,
			 const bool can_point, const bool in_lib,
			 vector<arg*> *arglist, const string ignore_text,
			 const string bundler_name) {
  type_defn *ign = new type_defn(name, is_class, is_abstract, is_derived, parent, type,
				 arglist, can_point,
				 in_lib, ignore_text, bundler_name);
  assert(ign);
  Options::all_types[ign->name()] = ign;
  Options::vec_types += ign;
  return (ign->name());
}
			    

message_layer::message_layer(const string nm, const medium md, const string bp,
			     const string brt, const string mdp, const string mo,
			     const string mop, const AS as, const string bfail,
			     const string bok, const string dir_f, const string pack_f,
			     const string unpack_f, const string free_f,
			     const string rpc_par, const string tag_type,
				 const string send_msg,
			     const bool r_used, const string skip_msg,
			     const string recv_msg, const string incs,
			     const bool do_serial, 
			     const string enc, const string dec,
			     const bool do_skip)
: name_(nm), med_(md), bundler_prefix_(bp), bundler_return_type_(brt),
  marshall_data_ptr_(mdp), marshall_obj_(mo), marshall_obj_ptr_(mop), address_space_(as),
  bundle_fail_(bfail), bundle_ok_(bok), dir_field_(dir_f), pack_const_(pack_f), 
  unpack_const_(unpack_f), free_const_(free_f), rpc_parent_(rpc_par),
  tag_type_(tag_type),
  send_message_(send_msg), records_used_(r_used), skip_message_(skip_msg),
  recv_message_(recv_msg), incs_(incs), serial_(do_serial), 
  encode_(enc), decode_(dec), skip_(do_skip)
{
  
}

string message_layer::read_tag(const string obj_name, const string tag_s) const {
  return(Options::ml->bundler_prefix() + "u_int(" + obj_name + ", (u_int*) &" + tag_s + ")");
}

void Options::dump_types() {
   for (dictionary_hash_iter<string, type_defn*> dhi=Options::all_types; dhi; dhi++)
      dhi.currval()->dump_type();
}

string remote_func::request_tag(bool unqual) const {
  return((unqual ? string("") : Options::type_prefix()) + name() + "_REQ");}

string remote_func::response_tag(bool unqual) const {
  return((unqual ? string("") : Options::type_prefix()) + name() + "_RESP");}

remote_func::remote_func(const string name, vector<arg*> *arglist, const call_type &ct,
			 const bool &is_v, const arg &return_arg, const bool do_free)
: name_(name), function_type_(ct), is_virtual_(is_v),
  call_sig_(arglist, name_), return_arg_(return_arg), do_free_(do_free)
{
}

// If I am generating code for the server, I don't handle async upcalls since
// these are calls that the server makes, not receives
// And the opposite applies for the client
bool remote_func::handle_request(ofstream &out_stream, const bool srvr, bool special) const {
  if (is_srvr_call()) {
    if (srvr) return false;
  } else {
    if (!srvr) return false;
  }
  out_stream << "case " << request_tag() << ":\n";
  out_stream << "{\n";
  if (call_sig_.type() != "void") {
    if (Options::ml->address_space() == message_layer::AS_many) {
      out_stream << call_sig_.type() << " message;\n";
      out_stream << "if (!" <<
	call_sig_.gen_bundler_call("net_obj()", "&message") << ") ";
      out_stream << Options::error_state("igen_decode_err",
					 Options::type_prefix() + "error");
      if (special)
        out_stream << "if (buffer) { *(" << call_sig_.type() << " *)buffer = message; break; }\n";
    }
  }

  if (Options::ml->address_space() == message_layer::AS_one) {
    if (special)
      out_stream << "if (buffer) { *(" << Options::type_prefix() << "msg_buf *)buffer = KLUDGE_msg_buf; break; }\n";
  }

  if (return_arg_.base_type() != "void") 
    out_stream << return_arg_.type( true ) << " ret = ";
  out_stream << name() << "(";

  if (Options::ml->address_space() == message_layer::AS_one) {
    out_stream << call_sig_.dump_args(string("KLUDGE_msg_buf.")+name()+"_call", ".");
  } else {
    out_stream << call_sig_.dump_args("message", ".");
  }

  out_stream << ")" << ";\n";

  // reply
  if (Options::ml->address_space() == message_layer::AS_many) {
    if (!is_async_call()) {
      out_stream << "unsigned ret_tag = " << response_tag() << ";\n";
      return_arg_.tag_bundle_send(out_stream, "ret", "ret_tag", 
				  Options::type_prefix() + "error");
    }
  } else {
    if (!is_async_call()) {
      string size, msg;
      out_stream << "val = msg_send(getRequestingThread(), " << response_tag();
      if (return_arg_.type() == "void") {
	size = ", 0"; msg = ", (void*)NULL";
      } else {
	size = ", sizeof(ret)"; msg = ", (void*) &ret";
      }
      out_stream << msg << size << ");\n";
    }
  }

  // only attempt to free the return value, the user should free the call_sig args
  // only if there is a return arg
  if ((Options::ml->address_space() != message_layer::AS_one) && do_free() &&
      (return_arg_.base_type() != "void")) {
    out_stream << "delete ret;\n";
  }

  out_stream << "}\n";
  out_stream << "break;\n";
  return true;
}

bool remote_func::free_async(ofstream &out_stream, const bool srvr) const {
  if (is_srvr_call()) {
    if (srvr) return false;
  } else {
    if (!srvr) return false;
  }
  if (!is_async_call()) return false;
  out_stream << "case " << request_tag() << ":\n";
  out_stream << "{\n";
  if (call_sig_.type() != "void") {
    out_stream << call_sig_.type() << " *message = (" << call_sig_.type() <<
      "*)(item->data_ptr);\n";
  }

  out_stream << name() << "(";
  out_stream << call_sig_.dump_args("(*message)", ".");
  // out_stream << (Options::all_types[call_sig_.base_type()])->dump_args("(*message)", ".");
  out_stream << ");\n";

  if (call_sig_.type() != "void") {
    out_stream << "assert(item->data_ptr);\n";
    out_stream << "delete (" << call_sig_.type() << "*) (item->data_ptr);\n";
  }

  out_stream << "}\n";

  out_stream << "break;\n";
  return true;
}

// If I am generating code for the server, I don't handle async upcalls since
// these are calls that the server makes, not receives
// And the opposite applies for the client
bool remote_func::save_async_request(ofstream &out_stream, const bool srvr) const {
  if (is_srvr_call()) {
    if (srvr) return false;
  } else {
    if (!srvr) return false;
  }
  if (!is_async_call()) return false;

  out_stream << "case " << request_tag() << ":\n";
  out_stream << "{\n";

  if (call_sig_.type() != "void") {
    out_stream << call_sig_.type() << " *message = new " << call_sig_.type()
      << ";\n" << "if (!"
	<< call_sig_.gen_bundler_call("net_obj()", "message") << ") ";
    out_stream << Options::error_state("igen_decode_err", "false");
  }
  out_stream << Options::type_prefix()
    << "buf_struct *buffer = new " << Options::type_prefix() << "buf_struct;\n";
  out_stream << "assert(buffer);\n";
  out_stream << "buffer->data_type = " << request_tag() << ";\n";

  if (call_sig_.type() != "void")
    out_stream << "buffer->data_ptr = (void*) message" << ";\n";
  else
    out_stream << "buffer->data_ptr = NULL;\n";

  out_stream << "async_buffer += buffer;\n";
  out_stream << "}\n";
  out_stream << "break;\n";
  return true;
}

bool remote_func::gen_async_struct(ofstream &) const { return true; }

bool remote_func::gen_signature(ofstream &out_stream, const bool &hdr,
				const bool server) const
{
  if (hdr)
    if ((server && !is_srvr_call()) ||
        (!server && is_srvr_call()))
      if (is_virtual())
         out_stream << " virtual ";

  out_stream << return_arg_.type(true, true) << " ";

  if (!hdr)
    out_stream << Options::current_interface->gen_class_prefix(is_srvr_call());

  out_stream << name() << "(";
  call_sig_.gen_sig(out_stream);
  out_stream << ")" << (hdr ? ";\n" : "");
  return true;
}

bool remote_func::gen_stub(ofstream &out_srvr, ofstream &out_clnt) const 
{
  switch (function_type_) {
  case async_upcall:
    gen_stub_helper(out_srvr, out_clnt, true);
    break;
  case async_call:
  case sync_call:
    gen_stub_helper(out_srvr, out_clnt, false);
    break;
  default:
    abort();
  }
  return true;
}

bool remote_func::gen_stub_helper(ofstream &out_srvr, ofstream &out_clnt,
				  const bool srvr) const 
{
  if (Options::ml->address_space() == message_layer::AS_one) 
    return (gen_stub_helper_one(out_srvr, out_clnt, srvr));
  else
    return (gen_stub_helper_many(out_srvr, out_clnt, srvr));
	    
}

bool remote_func::gen_stub_helper_many(ofstream &out_srvr, ofstream &out_clnt,
				       const bool srvr) const
{
  ofstream& out_str = (srvr ? out_srvr : out_clnt);

  gen_signature(out_str, false, srvr);
  out_str << "{\n";
  if (!is_async_call() && !is_void()) 
    out_str << return_arg_.type() << " ret_arg;\n";

  out_str << "if (get_err_state() != igen_no_err) {\n"
    << "IGEN_ERR_ASSERT;\nreturn " << return_value() << ";\n}\n";
  
  if (srvr) {
    out_srvr << "if (!getVersionVerifyDone()) {\n";
    out_srvr << "if (!verify_protocol()) ";
    out_srvr << Options::error_state("igen_proto_err", return_value());
    out_srvr << "\n}\n";
  }

  out_str << Options::ml->tag_type() << " tag = " << request_tag() << ";\n";
  call_sig_.tag_bundle_send(out_str, return_value(),
			    request_tag());

  if (!is_async_call()) {
    out_str << "if (!awaitResponse(" << response_tag() << ")) "
      << Options::error_state("igen_read_err", return_value());

    // set direction decode 
    if (!is_void()) {
    // out_str << Options::set_dir_decode() << ";\n";

      // decode something
      out_str << "if(!"
	<< (Options::all_types[return_arg_.base_type()])->gen_bundler_call("net_obj()",
								      "&ret_arg",
								      return_arg_.stars())
	  << ") ";
      out_str << Options::error_state("igen_decode_err", return_value());
    }

    out_str << "return " << return_value() << ";\n";
  }

  out_str << "}\n";
  return true;
}

bool remote_func::gen_stub_helper_one(ofstream &out_srvr, ofstream &out_clnt,
				      const bool srvr) const
{
  ofstream& out_str = (srvr ? out_srvr : out_clnt);

  gen_signature(out_str, false, srvr);
  out_str << "{\n";
  if (!is_async_call() && !is_void()) 
    out_str << return_arg_.type() << " ret_arg;\n";

  out_str << "if (get_err_state() != igen_no_err) {\n"
    << "IGEN_ERR_ASSERT;\nreturn " << return_value() << ";\n}\n";
  
  call_sig_.tag_bundle_send(out_str, return_value(),
			    request_tag());

  if (!is_async_call()) {
    if (return_arg_.type() != "void") {
      out_str << "unsigned len = sizeof(ret_arg);\n";
    } else
      out_str << "unsigned len = 0;\n";

	out_str << "thread_t tid = THR_TID_UNSPEC;\n";
    out_str << Options::ml->tag_type() << " tag = " << response_tag() << ";\n";

    string rb;
    string lb;
    if (return_arg_.type() != "void") {
      rb = "(void*)&ret_arg, ";
      lb = "sizeof(ret_arg)";
    } else {
      rb = "(void*) NULL, ";
      lb = "0";
    }

    out_str << "if (" << Options::ml->recv_message()
      << "(&tid, &tag, " << rb << "&len) == " << Options::ml->bundle_fail() << ") ";
    out_str << Options::error_state("igen_read_err", return_value());
    
    out_str << "if (len != " << lb << ") ";
    out_str << Options::error_state("igen_read_err", return_value());

    out_str << "if (tag != " << response_tag() << ") ";
    out_str << Options::error_state("igen_read_err", return_value());

    out_str << "return " << return_value() << ";\n";
  }

  out_str << "}\n";
  return true;
}

signature::signature(vector<arg*> *alist, const string rf_name) : is_const_(false), stars_(0) {
  assert(alist);
  for (unsigned i=0; i<alist->size(); i++) 
    args += (*alist)[i];

  switch (args.size()) {
  case 0: type_ = "void"; break;
  case 1:
    type_ = args[0]->base_type();
    stars_ = args[0]->stars();
    is_const_ = args[0]->is_const();
    break;
  default:
    type_defn *td = NULL;
    bool match = false;
    for (dictionary_hash_iter<string, type_defn*> tdi=Options::all_types;
         tdi && !match; tdi++) {
       td = tdi.currval();
       match = td->is_same_type(alist);
    }
    
    if (match) {
      type_ = td->name();
      for (unsigned q=0; q<args.size(); ++q)
	delete args[q];
      args.resize(0);
      args = td->copy_args();
      // get names here
    } else {
      type_ = Options::allocate_type(string("T_") + rf_name, false, false,
				     false, "",
				     type_defn::TYPE_COMPLEX,
				     true, false, &args);
    }
    break;
  }
}

string signature::type(const bool use_c) const {
  string ret;
  if (use_c && is_const_)
    ret = "const ";
  ret += type_;
  ret += Options::make_ptrs(stars_);
  return ret;
}

string signature::gen_bundler_call(const string obj_nm, const string data_nm) const {
  return ((Options::all_types[base_type()])->gen_bundler_call(obj_nm,
							      data_nm, stars_));
}

bool signature::tag_bundle_send(ofstream &out_stream, const string return_value,
				const string req_tag) const
{
  if (Options::ml->address_space() == message_layer::AS_many)
    return (tag_bundle_send_many(out_stream, return_value, req_tag));
  else
    return (tag_bundle_send_one(out_stream, return_value, req_tag));
}

bool signature::tag_bundle_send_many(ofstream &out_stream, const string return_value,
				     const string) const
{
  // set direction encode
  out_stream << Options::set_dir_encode() << ";\n";

  // bundle individual args
  out_stream << "if (!" << Options::ml->read_tag("net_obj()", "tag") << "\n";

  for (unsigned i=0; i<args.size(); i++) {
    out_stream << " || !";
    args[i]->gen_bundler(out_stream, "net_obj()", "&");
    out_stream << "\n";
  }
  
  out_stream << ") ";
  out_stream << Options::error_state("igen_encode_err", return_value);

  // send message
  out_stream << "if (!" << Options::ml->send_message() << ") ";
  out_stream << Options::error_state("igen_send_err", return_value);

  return true;
}

bool signature::tag_bundle_send_one(ofstream &out_stream, const string return_value,
				    const string req_tag) const
{
  switch (args.size()) {
  case 0:
    break;
  case 1:
    out_stream << type(true) << " send_buffer = ";
    out_stream << args[0]->name() << ";\n";
    break;
  default:
    out_stream << type(true) << " send_buffer;\n";
    (Options::all_types[type()])->assign_to("send_buffer.", args, out_stream);
    break;
  }

  string sb;
  if (type() != "void")
    sb = "(void*) &send_buffer, sizeof(send_buffer));\n";
  else
    sb = "(void*) NULL, 0);\n";

  out_stream << Options::ml->bundler_return_type() << " res = "
    << Options::ml->send_message() << "(net_obj(), " << req_tag
      << ", " << sb;

  out_stream << "if (res == " << Options::ml->bundle_fail() << ") ";
  out_stream << Options::error_state("igen_send_err", return_value);

  return true;
}

string signature::dump_args(const string data_name, const string sep) const {
  string ret;
  switch (args.size()) {
  case 0:
    // out_stream << "void";
    return "";
  case 1:
    return data_name;
  default:
    for (unsigned i=0; i<args.size(); i++) {
      ret += data_name + sep + args[i]->name();
      if (i < (args.size() - 1))
	ret += ", ";
    }
    return ret;
  }
}

bool signature::gen_sig(ofstream &out_stream) const {
  switch (args.size()) {
  case 0:
    out_stream << "void";
    break;
  case 1:
  default:
    for (unsigned i=0; i<args.size(); i++) {
      out_stream << args[i]->type(true, true) << " " << args[i]->name();
      if (i < (args.size()-1))
	out_stream << ", ";
    }
    break;
  }
  return true;
}

bool signature::arg_struct(ofstream &out_stream) const {
  for (unsigned i=0; i<args.size(); i++)
    out_stream << args[i]->type() << " " << args[i]->name() << ";\n";
  return true;
}

void recursive_dump_kids(const type_defn *curr_defn, ofstream &output) {
  for (unsigned u=0; u<curr_defn->kids_.size(); u++) {
    string kid_name = curr_defn->kids_[u];
    if (!Options::all_types.defines(kid_name)) {
      cerr << "Child: " << kid_name << " should be defined\n";
      assert(0);
    }    
    const type_defn *kid_defn = Options::all_types[kid_name];
    output << "case " << kid_defn->qual_id() << ":\n";
    output << "*data = new " << kid_defn->name() << ";\n";
    output << "break;\n";
    recursive_dump_kids(kid_defn, output);
  }
}
