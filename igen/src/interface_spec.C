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

#include "parse.h"

interface_spec::interface_spec(const string *name, const unsigned &b,
			       const unsigned &v)
: name_(*name), base_(b), version_(v),
  client_prefix_(*name+"User::"), server_prefix_(*name+"::"), 
  client_name_(*name + "User"), server_name_(*name), all_functions_(string::hash) 
{
}


interface_spec::~interface_spec() {

}

bool interface_spec::gen_dtor_hdr(ofstream &out_stream, const bool &server,
				  const bool &hdr) const
{
  if (!hdr)
    return true;

  out_stream << "virtual ~"
      << gen_class_name(server) << "() {  } \n";

  return true;
}

bool interface_spec::gen_ctor_hdr(ofstream &out_stream, const bool &server) const {
  if (Options::ml->address_space() == message_layer::AS_many) {
    gen_ctor_1(out_stream, server, true);
    gen_ctor_2(out_stream, server, true);
    gen_ctor_3(out_stream, server, true);
  } else {
    gen_ctor_4(out_stream, server, true);
  }

  if (server) {
    if (Options::ml->address_space() == message_layer::AS_many) {
      out_stream << "private:\n";
      out_stream << "bool verify_protocol(bool tag_read=false);\n";
      out_stream << "public:\n";
    }
  } else {
    if (Options::ml->address_space() == message_layer::AS_many) {
      out_stream << "private:\n";
      out_stream << "virtual bool verifyProtocolAndVersion();\n";
      out_stream << "public:\n";
      out_stream << "bool errorConditionFound;\n";
    }
  }

  if (Options::ml->address_space() == message_layer::AS_many) {
    out_stream << Options::type_prefix() << "message_tags"
      << " waitLoop(void);\n";
  } else {
    out_stream << Options::type_prefix() << "message_tags"
      << " waitLoop(" 
	<< "bool specific, "
	  << Options::type_prefix() << "message_tags mt,void *buffer=NULL);\n";
  }
  out_stream << "bool isValidTag(const " 
    << Options::type_prefix() << "message_tags &tag) {\n";
  out_stream << "return((tag >= " << Options::type_prefix() << "verify) && "
    << "(tag <= " << Options::type_prefix() << "last));\n}\n";

  if (Options::ml->address_space() == message_layer::AS_one) {
    out_stream << "// KLUDGE ALERT -- can't use a class member for msg_buf because this class \n";
    out_stream << "// instance is being abused -- multiplexed by several threads \n";
  }

  out_stream << Options::type_prefix() << "message_tags switch_on("
    << Options::type_prefix() << "message_tags m";
  if (Options::ml->address_space() == message_layer::AS_one) 
    out_stream << ", " << Options::type_prefix() << "msg_buf& KLUDGE";
  out_stream << ", void *buffer=NULL);\n";

  if (Options::ml->serial()) {
    out_stream << "// returns true if any requests are buffered\n";
    out_stream << "bool buffered_requests() {\n"
      << "return (async_buffer.size());\n}\n";

    out_stream << "// Wait until this mesage tag is received, or is found buffered\n";
    out_stream << "bool wait_for(" << Options::type_prefix() << "message_tags m);\n";
    out_stream << "bool wait_for_and_read(" << Options::type_prefix() << "message_tags m, void *buffer);\n";

    out_stream << "bool is_buffered(" << Options::type_prefix() << "message_tags m) {\n";
    out_stream << "unsigned size = async_buffer.size();\n";
    out_stream << "for (unsigned u=head; u<size; u++) \n";
    out_stream << "if (async_buffer[u]->data_type == m) return true;\n";
    out_stream << "return false;\n}\n";

    out_stream << "// Handles any buffered request\n";
    out_stream << Options::type_prefix() << "message_tags process_buffered();\n";

    out_stream << "private:\n";
    out_stream << Options::type_prefix() << "message_tags process_buffered(unsigned dex);\n";
    out_stream << "bool awaitResponse(const " << Options::type_prefix() << "message_tags &, bool wait=true, bool *response=NULL);\n";
    out_stream << Options::type_prefix() << "message_tags delete_buffer(unsigned dex);\n";
    out_stream << "public:\n";
  }
  return true;
}

bool interface_spec::gen_prelude(ofstream &out_stream, const bool &server) const {
  out_stream << "\nclass " << gen_class_name(server) << ":" 
    << " public RPCBase, public " << Options::ml->rpc_parent() << "{\npublic:\n";
  gen_ctor_hdr(out_stream, server);
  gen_dtor_hdr(out_stream, server, true);

  out_stream << "\n // Begin passed through text from interface file\n";
  if (server) {
    for (unsigned ts=0; ts<server_ignore.size(); ++ts)
      out_stream << server_ignore[ts] << endl;
  } else {
    for (unsigned ts=0; ts<client_ignore.size(); ++ts)
      out_stream << client_ignore[ts] << endl;
  }
  out_stream << "// End passed through text from interface file\n";

  return true;
}


bool interface_spec::gen_stl_bundler(ofstream &out_h, ofstream &/*out_c*/) const {
  if (Options::ml->address_space() == message_layer::AS_one) 
    return true;

  out_h << "#if defined(external_templates)\n#pragma interface\n#endif\n"; 

  out_h << "template <class Cont, class Elem, class Bund_Func> \ninline\n";
  out_h << Options::ml->bundler_return_type() << " " 
    << Options::type_class() << "_"
    << Options::ml->bundler_prefix() << "stl("
      << Options::ml->marshall_obj() << Options::ml->marshall_obj_ptr() 
	<< " obj, Cont *data, Bund_Func Func, Elem *ptr) {\n";
 
  out_h << "assert(obj);\n";
  out_h << "assert(" << Options::obj_ptr() << " != "
    << Options::ml->free_const() << ");\n";

  out_h << "if (" << Options::obj_ptr() << " == "
    << Options::ml->unpack_const() << ") {\n";
  out_h << "unsigned index=0;\n";
  out_h << "unsigned count;\n";
  out_h << "if (!" << Options::ml->read_tag("obj", "count") << ")\n"
    << " return FALSE;\n";

  out_h << "while (index < count) {\n ";
  out_h << "Elem element;\n";
  out_h << "if (Func(obj, &element)) {\n";
  out_h << "(*data) += element;\n";
  out_h << "index++;\n";
  out_h << "} else { \n";
  out_h << "break;\n";
  out_h << "}\n}\n";
  out_h << "if (index == count) return "
    << Options::ml->bundle_ok() << "; else return " 
      << Options::ml->bundle_fail() << ";\n";
  out_h << "} ";

  out_h << "else {\n";
  out_h << "unsigned index=0;\n";
  out_h << "unsigned count=data->size();\n";
  out_h << "if (!" << Options::ml->read_tag("obj", "count") << ")\n"
    << " return FALSE;\n";
  out_h << "while ((index <count) && ";
  out_h << "Func(obj, &((*data)[index]))) index++;\n";
  out_h << "if (index == count) return TRUE; else return FALSE;\n";
  out_h << "}\n}\n";
  return true;
}

bool interface_spec::gen_stl_bundler_ptr(ofstream &out_h, ofstream &/*out_c*/) const {
  if (Options::ml->address_space() == message_layer::AS_one) 
    return true;

  out_h << "template <class Cont, class Elem, class Bund_Func> \ninline\n";
  out_h << Options::ml->bundler_return_type() << " " 
    << Options::type_class() << "_"
    << Options::ml->bundler_prefix() << "stl_PTR("
      << Options::ml->marshall_obj() << Options::ml->marshall_obj_ptr() 
	<< " obj, Cont **data, Bund_Func Func, Elem *ptr) {\n";
 
  out_h << "assert(obj);\n";
  out_h << "assert(" << Options::obj_ptr() << " != "
    << Options::ml->free_const() << ");\n";

  out_h << "bool is_null = (*data == NULL);\n";
  out_h << "if (!P_xdr_Boolean(obj, &is_null)) return " << Options::ml->bundle_fail()
    << ";\n";

  out_h << "if (" << Options::obj_ptr() << " == "
    << Options::ml->unpack_const() << ") {\n";
  out_h << "if (is_null) { *data = NULL; return " << Options::ml->bundle_ok() << "; }\n";
  out_h << "*data = new Cont;\n";
  out_h << "if (!*data) return " << Options::ml->bundle_fail() << ";\n";
  out_h << "return "
    << Options::type_class() << "_"
    << Options::ml->bundler_prefix() << "stl(obj, *data, Func, ptr);\n";
  out_h << "} ";

  out_h << "else {\n";
  out_h << "if (!*data) return " << Options::ml->bundle_ok() << ";\n";
  out_h << "return "
         << Options::type_class() << "_"
          <<  Options::ml->bundler_prefix() << "stl(obj, *data, Func, ptr);\n";
  out_h << "}\n}\n";
  return true;
}

bool interface_spec::gen_scope(ofstream &out_h, ofstream &out_c) const {

  out_h << "class " << Options::type_class() << "  {\npublic:\n";

  type_defn *td; string s;

  bool first = true;
  out_h << "typedef enum {\n ";
  for (unsigned u=0; u<Options::vec_types.size(); u++) {
    td = Options::vec_types[u];
    if (td->is_class()) {
      if (!first)
	out_h << ", " << td->unqual_id();
      else {
	out_h << td->unqual_id() << " = " << base()+2001;
	first=false;
      }
    }
  }
  out_h << "} class_ids;\n";

  for (unsigned u1=0; u1<Options::vec_types.size(); u1++) {
    td = Options::vec_types[u1];
    if (!td->is_in_library() && !td->is_stl()) {
      td->gen_class(Options::ml->bundler_prefix(), out_h);
      if (Options::ml->address_space() == message_layer::AS_many) {
	td->gen_bundler_sig(true, Options::type_prefix(), 
			    Options::ml->bundler_prefix(), out_h);
	td->gen_bundler_body(Options::ml->bundler_prefix(), Options::type_prefix(),
			     out_c);
	td->gen_bundler_ptr(Options::type_prefix(), Options::ml->bundler_prefix(),
			    out_c, out_h);

      }
    }
  }

  remote_func *rf; 
  dictionary_hash_iter<string, remote_func*> rfi(all_functions_);  
  if (Options::ml->address_space() == message_layer::AS_one) {
    out_h << "union msg_buf { \n";
    while (rfi.next(s, rf)) {
      if (rf->sig_type() != "void")
	out_h << rf->sig_type(true) << " " << rf->name() << "_call;\n";
      if (rf->ret_type() != "void")
	out_h << rf->ret_type(true) << " " << rf->name() << "_req;\n";
    }
    out_h << "};\n";
  }

  out_h << "enum message_tags {\n";
  out_h << "verify = " << base() << ",\n";

  rfi.reset();
  while (rfi.next(s, rf)) 
    out_h << rf->request_tag(true) << ", " << rf->response_tag(true) << ",\n";
  out_h << "error, " << "last }; \n typedef enum message_tags message_tags;\n";

  if (Options::ml->serial()) {
    out_h << "typedef struct buf_struct {\n";
    out_h << "void *data_ptr;\n";
    out_h << "message_tags data_type;\n";
    out_h << "} buf_struct;\n";
  }

  out_h << "};\n";
  return true;
}

bool interface_spec::gen_inlines(ofstream &/*out_stream*/, const bool &/*server*/) const {
	    
  return true;
}

bool interface_spec::gen_header(ofstream &out_stream, const bool &server) const {
  gen_prelude(out_stream, server);

  string s; remote_func *rf;
  dictionary_hash_iter<string, remote_func*> dhi(all_functions_);
  while (dhi.next(s, rf))
    rf->gen_signature(out_stream, true, server);

  out_stream << "\nprotected:\n virtual void handle_error();\n";
  if (Options::ml->serial()) {
    out_stream << "vector<" << Options::type_prefix()
      << "buf_struct*> async_buffer;\n";
    out_stream << "unsigned head;\n";
  } 
  if (Options::ml->address_space() == message_layer::AS_one)
    out_stream << "// " << Options::type_prefix() << "msg_buf msg_buf;\n";


  out_stream << "};\n";

  gen_inlines(out_stream, server);

  return true;
}

bool interface_spec::gen_ctor_helper(ofstream &out_stream, const bool &server) const {
  if (!server) {
//    out_stream << "if (!opened()) assert(0);\n";
//    out_stream << "if (!verifyProtocolAndVersion()) assert(0);\n";
    out_stream << "if (!opened())\n";
    out_stream << "  errorConditionFound=true;\n";
    out_stream << "else {\n";
    out_stream << "  if (!verifyProtocolAndVersion())\n";
    out_stream << "    errorConditionFound=true;\n";
    out_stream << "  else\n"; 
    out_stream << "    errorConditionFound=false;\n";
    out_stream << "}\n";
  }
  return true;
}

bool interface_spec::gen_ctor_4(ofstream &out_stream, const bool &server,
				const bool &hdr) const
{
  if (!hdr)
    return true;

  out_stream << (!hdr ? gen_class_prefix(server) : string(""))
    << gen_class_name(server) 
      << "(const unsigned tid) :\n ";
  out_stream << " RPCBase(igen_no_err, 0), \n"
    << Options::ml->rpc_parent() << "(tid)";
  if (Options::ml->serial()) out_stream << ", head(0) ";
  out_stream << " { } \n";
  return true;
}

bool interface_spec::gen_ctor_1(ofstream &out_stream, const bool &server,
				const bool &hdr) const {
  out_stream << (!hdr ? gen_class_prefix(server) : string(""))
    << gen_class_name(server) 
      << "(int use_fd, xdr_rd_func r, xdr_wr_func w, const int nblock)";
  if (hdr) {
    out_stream << ";\n";
    return true;
  }
  
  out_stream << "\n: RPCBase(igen_no_err, 0),\n"
    << Options::ml->rpc_parent() << "(use_fd, r, w, nblock) ";
  if (Options::ml->serial()) out_stream << " , head(0) ";
  out_stream << "\n";
  return true;
}

bool interface_spec::gen_ctor_2(ofstream &out_stream, const bool &server,
				const bool &hdr) const
{
  out_stream << (!hdr ? gen_class_prefix(server) : string("")) << gen_class_name(server)
    << "(int family, int port, int type, const string host, xdr_rd_func r, xdr_wr_func w, int nblock)";

  if (hdr) {
    out_stream << ";\n";
    return true;
  }
  out_stream << "\n: RPCBase(igen_no_err, 0),\n"
    << Options::ml->rpc_parent() << "(family, port, type, host, r, w, nblock)";
  if (Options::ml->serial()) out_stream << ", head(0)";
  out_stream << "\n";
  return true;
}

bool interface_spec::gen_ctor_3(ofstream &out_stream, const bool &server,
				const bool &hdr) const
{
  out_stream << (!hdr ? gen_class_prefix(server) : string("")) << gen_class_name(server)
    << "(const string machine, const string login, const string program, xdr_rd_func rf, xdr_wr_func wf, vector<string> &args, int nblock, int port_fd)";

  if (hdr) {
    out_stream << ";\n";
    return true;
  }
  out_stream << "\n: RPCBase(igen_no_err, 0),\n"
    << Options::ml->rpc_parent() << "(machine, login, program, rf, wf, args, nblock, port_fd)";
  if (Options::ml->serial()) out_stream << ", head(0) ";
  out_stream << "\n";
 
  return true;
}

bool interface_spec::gen_ctor_body(ofstream &out_stream, const bool &server) const {
  if (Options::ml->address_space() == message_layer::AS_many) {
    gen_ctor_1(out_stream, server, false);
    out_stream << "{\n";
    gen_ctor_helper(out_stream, server);
    out_stream << "}\n";

    gen_ctor_2(out_stream, server, false);
    out_stream << "{\n";
    gen_ctor_helper(out_stream, server);
    out_stream << "}\n";

    gen_ctor_3(out_stream, server, false);
    out_stream << "{\n";
    gen_ctor_helper(out_stream, server);
    out_stream << "}\n";
  } else {
    // do nothing
  }
  return true;
}

bool interface_spec::gen_dtor_body(ofstream &/*out_stream*/, const bool &/*server*/) const {
  // gen_dtor_hdr(out_stream, server, false);
  // out_stream << "{ }\n";
  return true;
}

bool interface_spec::gen_server_verify(ofstream &out_stream) const {

  if (!Options::dont_gen_handle_err) {
    out_stream << "void " << gen_class_prefix(true) << "handle_error() {\n";
    out_stream << "cerr << \"Error not handled, exiting\" << endl;\n";
    out_stream << "IGEN_ERR_ASSERT;\n";
    out_stream << "exit(-1);\n";
    out_stream << "}\n\n";
  }

  if (Options::ml->address_space() == message_layer::AS_one) 
    return true;

  out_stream << "bool " << gen_class_prefix(true) << "verify_protocol(bool tag_read) {\n";
  out_stream << "unsigned tag;\n";
  out_stream << "if (!tag_read) {\n";
  out_stream << Options::set_dir_decode() << ";\n";

  if (Options::ml->skip()) {
    out_stream << "if (!" << Options::ml->skip_message() << ") ";
    out_stream << Options::error_state("igen_read_err", "false");
  }
  out_stream << "if (!" << Options::ml->bundler_prefix() << "u_int(net_obj(), &tag)) ";
  out_stream << Options::error_state("igen_proto_err", "false");
  out_stream << "if (tag != " << Options::type_prefix() << "verify) ";
  out_stream << Options::error_state("igen_proto_err", "false");
  out_stream << "}\n";

  out_stream << Options::set_dir_encode() << ";\n";
  out_stream << "tag = " << Options::type_prefix() << "verify;\n";
  out_stream << "unsigned version = " << version() << ";\n";
  out_stream << "string name = " << "\"" << name() << "\";\n";
  out_stream << "if (!" << Options::ml->bundler_prefix()
    << "u_int(net_obj(), &tag) || \n !" << Options::ml->bundler_prefix() 
      << "string_pd(net_obj(), &name) || \n !" << Options::ml->bundler_prefix()
	<< "u_int(net_obj(), &version)) ";
  out_stream << Options::error_state("igen_encode_err", "false");

  out_stream << "if (!" << Options::ml->send_message() << ") ";
  out_stream << Options::error_state("igen_send_err", "false");
  out_stream << "setVersionVerifyDone();\n";
  out_stream << "return true;\n}\n";

  return true;
}

bool interface_spec::gen_client_verify(ofstream &out_stream) const {

  if (!Options::dont_gen_handle_err) {
    out_stream << "void " << gen_class_prefix(false) << "handle_error() {\n";
    out_stream << "cerr << \"Error condition found - handle_error\" << endl;\n";
    out_stream << "}\n\n";
  }

  if (Options::ml->address_space() == message_layer::AS_many) {
    out_stream << "bool  " << gen_class_prefix(false)
      << "verifyProtocolAndVersion() {\n";

    out_stream << "unsigned tag = " << Options::type_prefix() << "verify;\n";
    out_stream << Options::set_dir_encode() << ";\n";
    out_stream << "if (!" << Options::ml->bundler_prefix()
      << "u_int(net_obj(), &tag)) ";
    out_stream << Options::error_state("igen_encode_err", "false");

    out_stream << "if (!" << Options::ml->send_message() << ") ";
    out_stream << Options::error_state("igen_send_err", "false");

    out_stream << "if (!awaitResponse(" << Options::type_prefix() << "verify)) ";
    out_stream << Options::error_state("igen_proto_err", "false");

    out_stream << Options::set_dir_decode() << ";\n";
    out_stream << "string proto;\n";
    out_stream << "if (!" << Options::ml->bundler_prefix()
      << "string_pd(net_obj(), &proto) || \n !" 
	<< Options::ml->bundler_prefix() << "u_int(net_obj(), &tag)) {\n";
    out_stream << "set_err_state(igen_proto_err);\n";
    out_stream << "cerr << \"Protocol verify - bad response from server\" << endl;";
    out_stream << "handle_error(); exit(-1); return false;\n}\n";
    
    out_stream << "if ((tag != " << version() << "|| proto != \""
      << name() << "\")) {\n";
    out_stream << "cerr << \" Protocol " << name() << " version " << version()
      << " expected\" << endl;";
    out_stream << "cerr << \" Found Protocol \" << proto << \" version \" << tag << endl;";
    out_stream << "set_err_state(igen_proto_err);\n";

    //out_stream << "handle_error();\nexit(-1); return false;\n}\n";
    out_stream << "handle_error(); return false;\n}\n";
       // removed the exit(-1) so the caller can look at errorCondition flag
       // and do the polite thing.  Of course this raises a danger; the caller
       // must not forget to check! --ari 03/96

    out_stream << "return true;\n}\n";
  }
  return true;
}

bool interface_spec::gen_interface() const {

  gen_scope(Options::dot_h, Options::dot_c);

  gen_header(Options::clnt_dot_h, false);
  gen_header(Options::srvr_dot_h, true);

  gen_ctor_body(Options::srvr_dot_c, true);
  gen_ctor_body(Options::clnt_dot_c, false);

  gen_dtor_body(Options::srvr_dot_c, true);
  gen_dtor_body(Options::clnt_dot_c, false);

  remote_func *rf; string s;
  dictionary_hash_iter<string, remote_func*> rfi(all_functions_);
  while (rfi.next(s, rf))
    rf->gen_stub(Options::srvr_dot_c, Options::clnt_dot_c);

  gen_await_response(Options::clnt_dot_c, false);
  gen_await_response(Options::srvr_dot_c, true);
  gen_wait_loop(Options::clnt_dot_c, false);
  gen_wait_loop(Options::srvr_dot_c, true);
  gen_process_buffered(Options::srvr_dot_c, true);
  gen_process_buffered(Options::clnt_dot_c, false);
  gen_client_verify(Options::clnt_dot_c);
  gen_server_verify(Options::srvr_dot_c);

  if (Options::stl_seen) {
    gen_stl_bundler(Options::dot_h, Options::dot_c);
    gen_stl_bundler_ptr(Options::dot_h, Options::dot_c);
    gen_stl_temps();
  }

  return true;
}

// I am assuming that Vector and Queue have already been included
bool interface_spec::gen_stl_temps() const {

  if (Options::ml->address_space() == message_layer::AS_many) 
    Options::temp_dot_c << "template class queue<"
      << Options::type_prefix() << "buf_struct*>;\n";
  
  for (unsigned stl_index=0; stl_index < Options::stl_types.size(); stl_index++) {
    if (Options::stl_types[stl_index].elements.size()) {
      if (Options::stl_types[stl_index].need_include) {
	Options::temp_dot_c <<  "#pragma implementation \"" 
	  << Options::stl_types[stl_index].pragma_name << "\"\n";
	Options::temp_dot_c << "#include "
	  << Options::stl_types[stl_index].include_file << "\n";
      }
      for (unsigned el_index=0; el_index < Options::stl_types[stl_index].elements.size();
	   el_index++) {
	Options::temp_dot_c << "template class " <<
	  Options::stl_types[stl_index].name << "<"
	    << Options::stl_types[stl_index].elements[el_index].name << ">;\n";
      }
    }
  }

  if (Options::ml->address_space() == message_layer::AS_one)
    return true;

  for (unsigned stl_index1=0; stl_index1 < Options::stl_types.size(); stl_index1++) {
    for (unsigned el_index=0; el_index < Options::stl_types[stl_index1].elements.size();
	 el_index++) {
      Options::temp_dot_c << "template "
	<< Options::ml->bundler_return_type() << " "
	  << Options::type_class() << "_"
	  << Options::ml->bundler_prefix() << "stl";
      Options::temp_dot_c << "(" 
	<< Options::ml->marshall_obj() << Options::ml->marshall_obj_ptr() << ", "
        << Options::stl_types[stl_index1].name
	<< "<"
	<< Options::stl_types[stl_index1].elements[el_index].name
	<< ">*, "
	<< Options::ml->bundler_return_type() << " (*)("
	<< Options::ml->marshall_obj()
	<< Options::ml->marshall_obj_ptr() << ", "
	<< Options::stl_types[stl_index1].elements[el_index].name << "*), "
	<< Options::stl_types[stl_index1].elements[el_index].name
	<< "*);\n";
    }
  }

  for (unsigned stl_index2=0; stl_index2 < Options::stl_types.size(); stl_index2++) {
    for (unsigned el_index=0; el_index < Options::stl_types[stl_index2].elements.size();
	 el_index++) {
      Options::temp_dot_c << "template "
	<< Options::ml->bundler_return_type() << " "
	<< Options::type_class() << "_"
	  << Options::ml->bundler_prefix() << "stl";
      Options::temp_dot_c << "_PTR";
      Options::temp_dot_c << "(" 
	<< Options::ml->marshall_obj() << Options::ml->marshall_obj_ptr() << ", "
        << Options::stl_types[stl_index2].name
	<< "<"
	<< Options::stl_types[stl_index2].elements[el_index].name
	<< ">**, "
	<< Options::ml->bundler_return_type() << " (*)("
	<< Options::ml->marshall_obj()
	<< Options::ml->marshall_obj_ptr() << ", "
	<< Options::stl_types[stl_index2].elements[el_index].name << "*), "
	<< Options::stl_types[stl_index2].elements[el_index].name
	<< "*);\n";
    }
  }
  
  return true;
}

bool interface_spec::gen_process_buffered(ofstream &out_stream, const bool &srvr) const {
  if (!Options::ml->serial())
    return true;

  out_stream << Options::type_prefix() << "message_tags " << gen_class_prefix(srvr)
    << "process_buffered() {\n";
  out_stream << "if (!async_buffer.size()) return "
    << Options::type_prefix() << "error;\n";

  out_stream << "return (delete_buffer(head));\n";
  out_stream << "}\n";

  out_stream << Options::type_prefix() << "message_tags " << gen_class_prefix(srvr)
    << "process_buffered(unsigned index) {\n";
  out_stream << "if (!async_buffer.size()) return "
    << Options::type_prefix() << "error;\n";
  out_stream << "return (delete_buffer(index));\n";
  out_stream << "}\n";

  out_stream << Options::type_prefix() << "message_tags "
    << gen_class_prefix(srvr) << "delete_buffer(unsigned index) {\n";
  out_stream << "unsigned asize = async_buffer.size();\n";
  out_stream << "assert(index < asize);\n";
  out_stream << Options::type_prefix() << "buf_struct *item = async_buffer[index];\n";

  out_stream << "unsigned elems_left = asize - head - 1;\n";
  out_stream << "if (index == head) \n head++;\n";
  out_stream << "else if (index == (asize - 1))   \n";
  out_stream << "   async_buffer.resize(asize - 1); \n";
  out_stream << "else {   \n";
  out_stream << "   async_buffer[index] = async_buffer[asize - 1]; \n";
  out_stream << "   async_buffer.resize(asize - 1);   \n";
  out_stream << "}\n";

  out_stream << "if (!elems_left) {    \n";
  out_stream << "head = 0; async_buffer.resize(0);   \n";
  out_stream << "}\n";
  out_stream << "assert((!async_buffer.size() && !head) || (elems_left == (async_buffer.size()-head)));\n";
  out_stream << Options::type_prefix() << "message_tags tag = item->data_type;\n";
  out_stream << "switch (tag) {\n";
  
  remote_func *rf; string s;
  dictionary_hash_iter<string, remote_func*> rfi(all_functions_);
  while (rfi.next(s, rf))
    rf->free_async(out_stream, srvr);
  
  out_stream << "default:\n";
  out_stream << "IGEN_ERR_ASSERT;\n";
  out_stream << "set_err_state(igen_request_err);\n";
  out_stream << "return " << Options::type_prefix() << "error" << ";\n";
  out_stream << "break;\n";
  out_stream << "}\n";

  out_stream << "delete item;\n";
  out_stream << "return tag;\n";
  out_stream << "}\n";

  return true;
}

bool interface_spec::gen_await_response(ofstream &out_stream, const bool srvr) const {
  if (!Options::ml->serial())
    return true;

  out_stream << "bool " << gen_class_prefix(srvr)
    << "awaitResponse(const " << Options::type_prefix()
      << "message_tags &target_tag, bool wait, bool *response) {\n";
  out_stream << "unsigned tag;\n";
  out_stream << "if (!wait) *response = false;\n";
  out_stream << "if (get_err_state() != igen_no_err) return false;\n";
  out_stream << Options::set_dir_decode() << ";\n";

  out_stream << "do {\n";

  if (Options::ml->records_used()) {
    out_stream << "if (!" << Options::ml->skip_message() << ") ";
    out_stream << Options::error_state("igen_read_err", "false");
  }

  out_stream << "if (!" << Options::ml->read_tag("net_obj()", "tag") << ") ";
  out_stream << Options::error_state("igen_read_err", "false");

  out_stream << "if (tag == target_tag) {\n";
  out_stream << "  if (!wait) *response = true;\n";
  out_stream << "  return true;\n";
  out_stream << "}\n";  

  out_stream << "switch (tag) {\n";
  
  remote_func *rf; string s;
  dictionary_hash_iter<string, remote_func*> rfi(all_functions_);
  while (rfi.next(s, rf))
    rf->save_async_request(out_stream, srvr);
  out_stream << "default:\n";
  out_stream << "set_err_state(igen_request_err);\n";
  out_stream << "IGEN_ERR_ASSERT;\n";
  out_stream << "return false;\n";
  out_stream << "}\n";

  out_stream << "} while(wait);\n";
  out_stream << "assert(0);\n";
  out_stream << "return false;\n";
  out_stream << "}\n";
  return true;
}

bool interface_spec::gen_wait_loop(ofstream &out_stream, const bool srvr) const {

  out_stream << Options::type_prefix() << "message_tags "
    << gen_class_prefix(srvr) << "waitLoop(";
  
  if (Options::ml->address_space() == message_layer::AS_many) {
    out_stream << "void) {\n";
  } else {
    out_stream << " bool specific, "
      << Options::type_prefix() << "message_tags mt, void *buffer) {\n"; 
  }

  out_stream << Options::type_prefix() << "message_tags tag;\n";

  out_stream << "if (get_err_state() != igen_no_err) return "
    << Options::type_prefix() << "error;\n";

  out_stream << Options::set_dir_decode() << ";\n";

  if (Options::ml->records_used()) {
    if (Options::ml->skip()) {
      out_stream << "if (!" << Options::ml->skip_message() << ") ";
      out_stream << Options::error_state("igen_read_err",
					 Options::type_prefix()+"error");
    }
  }

  if (Options::ml->address_space() != message_layer::AS_one) {
    out_stream << "if (!" << Options::ml->read_tag("net_obj()", "tag") << ") ";
    out_stream << Options::error_state("igen_read_err",
			 Options::type_prefix()+"error");
    out_stream << "return switch_on(tag);\n"; 
  } else {
    out_stream << "if (!specific) \n";
    out_stream << "tag = " << Options::type_prefix() << "message_tags(MSG_TAG_ANY);\n";
    out_stream << "else \n";
    out_stream << "tag = mt;\n";

    out_stream << Options::type_prefix() << "msg_buf KLUDGE_msg_buf;\n";    
    out_stream << "// unsigned len = sizeof(msg_buf);\n";
    out_stream << "unsigned len = sizeof(KLUDGE_msg_buf);\n";
    out_stream << "// setRequestingThread(msg_recv((unsigned*)&tag, &msg_buf, &len));\n";
    out_stream << "setRequestingThread(msg_recv((unsigned*)&tag, &KLUDGE_msg_buf, &len));\n";
    out_stream << "if (getRequestingThread() == THR_ERR) ";
    out_stream << Options::error_state("igen_read_err", Options::type_prefix()+"error");
    out_stream << "return switch_on(tag, KLUDGE_msg_buf, buffer);\n"; 
  }


  out_stream << "}\n";

  out_stream << Options::type_prefix() << "message_tags " << gen_class_prefix(srvr)
    << "switch_on(" << Options::type_prefix() << "message_tags tag";
  if (Options::ml->address_space() == message_layer::AS_one) 
    out_stream << ", " << Options::type_prefix() << "msg_buf& KLUDGE_msg_buf";
  out_stream << ", void *buffer) {\n";

  if (Options::ml->address_space() == message_layer::AS_one) {
    out_stream << "int val = THR_OKAY;\n";
  }

  out_stream << "switch (tag) {\n";

  if (srvr && (Options::ml->address_space() != message_layer::AS_one)) {
    out_stream << "case " << Options::type_prefix() << "verify:\n";
    out_stream << "if (!verify_protocol(true)) ";
    out_stream << Options::error_state("igen_proto_err", Options::type_prefix()+"error");
    out_stream << "return tag;\n";
  }

  remote_func *rf; string s;
  dictionary_hash_iter<string, remote_func*> rfi(all_functions_);
  while (rfi.next(s, rf))
    rf->handle_request(out_stream, srvr, true);

  out_stream << "default:\n";
  out_stream << "set_err_state(igen_request_err);\n";
  out_stream << "IGEN_ERR_ASSERT;\n";
  out_stream << "return " << Options::type_prefix() << "error;\n";
  out_stream << "}\n";

  if (Options::ml->address_space() == message_layer::AS_one) {
    out_stream << "if (val != THR_OKAY) ";
    out_stream <<
      Options::error_state("igen_read_err", Options::type_prefix()+"error");
  }

  out_stream << "return tag;\n";
  out_stream << "}\n";

  if (Options::ml->address_space() != message_layer::AS_one) {
    out_stream << "bool " << gen_class_prefix(srvr)
      << "wait_for(" << Options::type_prefix() << "message_tags tag) {\n";

    out_stream << "unsigned size = async_buffer.size();\n";
    out_stream << "for (unsigned u=head; u<size; u++) \n";
    out_stream << "if (async_buffer[u]->data_type == tag) {\n";
    out_stream << "if (process_buffered(u) != tag) \n return false;\n";
    out_stream << "else \n return true;\n";
    out_stream << "}\n";

    out_stream << "if (!awaitResponse(tag)) ";
    out_stream << Options::error_state("igen_proto_err", "false");
    
    out_stream << "if (!switch_on(tag)) \n return false;\n";
    out_stream << "else \n return true;\n";
    out_stream << "}\n";

    // New procedure required for async enableData requests
    out_stream << "bool " << gen_class_prefix(srvr)
      << "wait_for_and_read(" << Options::type_prefix() 
      << "message_tags tag, void *buffer) {\n";
    out_stream << "  bool response;\n";
    out_stream << "  if (awaitResponse(tag,false,&response)) {\n";
    out_stream << "    if (response && switch_on(tag,buffer))\n";
    out_stream << "      return(true);\n";
    out_stream << "  }\n";
    out_stream << "  return(false);\n";    
    out_stream << "}\n";   
  }
  
  return true;
}

bool interface_spec::new_remote_func(const string *name, vector<arg*> *arglist,
				     const remote_func::call_type &callT,
				     const bool &is_virtual, const arg &return_arg,
				     const bool do_free) {
  assert(name);
  remote_func *rf = new remote_func(*name, arglist, callT, is_virtual,
				    return_arg, do_free);
  assert(rf);
  all_functions_[*name] = rf;
  return true;
}

bool interface_spec::are_bundlers_generated() const {
  return (Options::ml->address_space() == message_layer::AS_many);
}

void interface_spec::ignore(bool is_srvr, char *text) {
  if (!text || (P_strlen(text) < 17)) return;
  char *buffer = P_strdup(text);
  char *temp = buffer;
  temp[strlen(temp) - 8] = (char) 0;
  temp += 8;
  if (is_srvr)
    server_ignore += temp;
  else
    client_ignore += temp;
  delete buffer;
}
