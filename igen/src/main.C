/*
 * Copyright (c) 1996-2003 Barton P. Miller
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

// $Id: main.C,v 1.59 2003/06/17 17:16:24 pcroth Exp $

/*
 * Note: AIX 5.1
 * If igen is not compiled and linked with the -pthread flag (gcc > 3.1)
 * it dies with a segfault in the initialization of libstdc++
 * -- bernat, JAN2003
 */

#include "parse.h"
#include "Options.h"
#include "type_defn.h"
#include <iostream.h>
#include <stdio.h>
#include "common/h/Ident.h"

extern "C" const char V_igen[];
Ident V_id(V_igen,"Paradyn");

bool Options::dont_gen_handle_err = false;
dictionary_hash<string, type_defn*> Options::all_types(string::hash);
pdvector<type_defn*> Options::vec_types;
pdvector<message_layer*> Options::all_ml;
pdvector<Options::stl_data> Options::stl_types;
pdvector<string> Options::forward_decls;

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
bool Options::shortnames_ = false;
unsigned Options::var_count_ = 0;
Options::mem_type Options::mem_;
message_layer *Options::ml;
bool Options::stl_seen = false;

static void usage() {
  cerr << "igen [-prof] [-shortnames] -xdr | -thread | -pvm [-header | -code]\n";
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
  cerr << "  MISCELLANEOUS OPTIONS\n";
  cerr << "    -shortnames   -->  produce code such that subclasses not generated as members of main class\n";
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
    } else if (!strcmp("-shortnames", argv[i])) {
      Options::set_shortnames(true);
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
#if defined(i386_unknown_nt4_0)
  // support either forward or backslashes (or mixed) in the input pathname
  char* btemp = strrchr(buffer, '\\');
  char* ftemp = strrchr(buffer, '/');
  char* temp = ((btemp > ftemp) ? btemp : ftemp);
#else
  char *temp = strrchr(buffer, '/');
#endif // defined(i386_unknown_nt4_0)
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
    Options::input.open((Options::input_file()).c_str(), ios::in);
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
  Options::temp_dot_c.open(dump_to.c_str(), ios::out);
  if (!Options::temp_dot_c.good()) {
    cerr << "Could not open " << dump_to << " for output\n";
    exit(-1);
  }

  dump_to = base + "h";
  Options::dot_h.open(dump_to.c_str(), ios::out);
  if (!Options::dot_h.good()) {
    cerr << "Could not open " << dump_to << " for output\n";
    exit(-1);
  }

  dump_to = base + "C";
  Options::dot_c.open(dump_to.c_str(), ios::out);
  if (!Options::dot_c.good()) {
    cerr << "Could not open " << dump_to << " for output\n";
    exit(-1);
  }

  dump_to = base + "SRVR.h";
  Options::srvr_dot_h.open(dump_to.c_str(), ios::out);
  if (!Options::srvr_dot_h.good()) {
    cerr << "Could not open " << dump_to << " for output\n";
    exit(-1);
  }

  dump_to = base + "SRVR.C";
  Options::srvr_dot_c.open(dump_to.c_str(), ios::out);
  if (!Options::srvr_dot_c.good()) {
    cerr << "Could not open " << dump_to << " for output\n";
    exit(-1);
  }

  dump_to = base + "CLNT.C";
  Options::clnt_dot_c.open(dump_to.c_str(), ios::out);
  if (!Options::clnt_dot_c.good()) {
    cerr << "Could not open " << dump_to << " for output\n";
    exit(-1);
  }

  dump_to = base + "CLNT.h";
  Options::clnt_dot_h.open(dump_to.c_str(), ios::out);
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

  Options::srvr_dot_h << ident << endl;
  Options::srvr_dot_h << "#ifndef " << cpp_base << "SRVR_H\n";
  Options::srvr_dot_h << "#define " << cpp_base << "SRVR_H\n";
  Options::srvr_dot_h << "#include \"" << base << "h\"\n";

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
  sd.name = "pdvector"; sd.include_file = "common/h/Vector.h";
  sd.need_include = false; sd.pragma_name = "Vector.h";
  Options::stl_types += sd;
  Options::el_data el;
  el.name = "string"; el.type = "string"; el.stars = 0;
  Options::stl_types[0].elements += el;

  Options::add_type("kptr_t", false, false, false, false, "", type_defn::TYPE_SCALAR, false,
		    true, /*false,*/ NULL);
  Options::add_type("dptr_t", false, false, false, false, "", type_defn::TYPE_SCALAR, false,
		    true, /*false,*/ NULL);
  Options::add_type("uint32_t", false, false, false, false, "", type_defn::TYPE_SCALAR, false,
		    true, /*false,*/ NULL);
  Options::add_type("uint64_t", false, false, false, false, "", type_defn::TYPE_SCALAR, false,
		    true, /*false,*/ NULL);
  Options::add_type("int", false, false, false, false, "", type_defn::TYPE_SCALAR, false,
		    true, /*false,*/ NULL);
  Options::add_type("double", false, false, false, false, "", type_defn::TYPE_SCALAR, false,
		    true, /*false,*/ NULL);
  Options::add_type("void", false, false, false, false, "", type_defn::TYPE_SCALAR, false,
                    true, /*false,*/ NULL);
  Options::add_type("bool", false, false, false, false, "", type_defn::TYPE_SCALAR, false,
                    true, /*false,*/ NULL, "", "Boolean");
  Options::add_type("u_int", false, false, false, false, "", type_defn::TYPE_SCALAR, false,
                    true, /*false,*/ NULL);
  Options::add_type("unsigned", false, false, false, false, "", type_defn::TYPE_SCALAR,
                    false, true, /*false,*/ NULL);
  Options::add_type("u_long", false, false, false, false, "", type_defn::TYPE_SCALAR,
                    false, true, /*false,*/ NULL);
  Options::add_type("u_longlong_t", false, false, false, false, "", type_defn::TYPE_SCALAR,
                    false, true, /*false,*/ NULL);
  Options::add_type("float", false, false, false, false, "", type_defn::TYPE_SCALAR, false,
                    true, /*false,*/ NULL);
  Options::add_type("u_char", false, false, false, false, "", type_defn::TYPE_SCALAR, false,
                    true, /*false,*/ NULL);
  Options::add_type("char", false, false, false, false, "", type_defn::TYPE_SCALAR, false,
                    true, /*false,*/ NULL);
  Options::add_type("u_short", false, false, false, false, "", type_defn::TYPE_SCALAR, false,
                    true, /*false,*/ NULL);
  Options::add_type("short", false, false, false, false, "", type_defn::TYPE_SCALAR, false,
                    true, /*false,*/ NULL);
  Options::add_type("string", false, false, false, false, "", type_defn::TYPE_SCALAR, false,
                     true, /*false,*/ NULL, "", "string_pd");
  Options::add_type("crope", false, false, false, false, "", type_defn::TYPE_SCALAR, false,
                     true, /*false,*/ NULL, "", "crope_pd");
/* trace data streams */
  Options::add_type("byteArray", false, false, false, false, "", type_defn::TYPE_SCALAR, false,
		    true, /*false,*/ NULL, "", "byteArray_pd");
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
					     "bool", // return type of bundler fns
					     "*",
					     "XDR",
					     "*",
					     message_layer::AS_many,
					     "false",
					     "true",
					     "x_op",
					     "XDR_ENCODE",
					     "XDR_DECODE", 
					     "XDR_FREE",
					     "XDRrpc",
						 "unsigned int",
					     "xdrrec_endofrecord(net_obj(), TRUE)",
					     true,
					     "xdrrec_skiprecord(net_obj())",
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
				      "#include \"pdthread/h/thread.h\"\n",
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
					     "xdrrec_endofrecord(net_obj(), TRUE)",
					     true,
					     "xdrrec_skiprecord(net_obj())",
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
  Options::dot_h << msg;
}

void recursive_dump_kids(const type_defn *curr_defn, ofstream &output) {
  for (unsigned u=0; u<curr_defn->kids_.size(); u++) {
    string kid_name = curr_defn->kids_[u];
    if (!Options::all_types.defines(kid_name)) {
      cerr << "Child: " << kid_name << " should be defined\n";
      assert(0);
    }    
    const type_defn *kid_defn = Options::all_types[kid_name];
    output << "    case " << kid_defn->qual_id() << ":\n";
    output << "      data = new " << kid_defn->name() << ";\n";
    output << "      break;\n";
    recursive_dump_kids(kid_defn, output);
  }
}
