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

#include "interface_spec.h"
#include "parse.h"
#include "Options.h"

// utility functions used from main.C
extern pdstring	unqual_type( const pdstring& type );

interface_spec::interface_spec(const pdstring *name, const unsigned &b,
			       const unsigned &v)
: name_(*name), base_(b), version_(v),
  client_prefix_(*name+"User::"), server_prefix_(*name+"::"), 
  client_name_(*name + "User"), server_name_(*name), all_functions_(pdstring::hash) 
{ }

interface_spec::~interface_spec() { }

bool interface_spec::gen_dtor_hdr(ofstream &out_stream, bool server,
				  bool hdr) const {
  if (!hdr) return true;
  out_stream << "virtual ~" << gen_class_name(server) << "() {  } \n";
  return true;
}

bool interface_spec::gen_ctor_hdr(ofstream &out_stream, bool server) const {
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
      out_stream << "  bool verify_protocol(bool tag_read=false);\n";
      out_stream << "public:\n";
    }
  } else {
    if (Options::ml->address_space() == message_layer::AS_many) {
      out_stream << "private:\n";
      out_stream << "  virtual bool verifyProtocolAndVersion();\n";
      out_stream << "public:\n";
      out_stream << "  bool errorConditionFound;\n";
    }
  }

  if (Options::ml->address_space() == message_layer::AS_many) {
    out_stream << "  " << Options::type_prefix() << "message_tags"
	       << " waitLoop(void);\n";
  } else {
    out_stream << "  " << Options::type_prefix() << "message_tags"
	       << " waitLoop(" 
	       << "bool specific, "
	       << Options::type_prefix() << "message_tags mt,void** buffer=NULL);\n";
  }
  out_stream << "  bool isValidTag(const " 
	     << Options::type_prefix() << "message_tags &tag) {\n";
  out_stream << "  return((tag >= " << Options::type_prefix() << "verify) && "
	     << "(tag <= " << Options::type_prefix() << "last));\n}\n";

  out_stream << "  " << Options::type_prefix() << "message_tags switch_on("
	     << Options::type_prefix() << "message_tags m";
  if (Options::ml->address_space() == message_layer::AS_one) 
  {
    out_stream << ", void* msgBuf";
  }
  out_stream << ", void** buffer=NULL);\n";

  if (Options::ml->serial()) {
    out_stream << "  // returns true if any requests are buffered\n";
    out_stream << "  bool buffered_requests() {\n"
	       << "    return (async_buffer.size() > 0);\n}\n";

    out_stream << "  // Wait until this mesage tag is received, or is found buffered\n";
    out_stream << "  bool wait_for(" << Options::type_prefix() << "message_tags m);\n";
    out_stream << "  bool wait_for_and_read(" << Options::type_prefix() << "message_tags m, void** buffer);\n";

    out_stream << "  bool is_buffered(" << Options::type_prefix() << "message_tags m) {\n";
    out_stream << "  unsigned size = async_buffer.size();\n";
    out_stream << "  for (unsigned u=head; u<size; u++) \n";
    out_stream << "    if (async_buffer[u]->data_type == m) return true;\n";
    out_stream << "    return false;\n   }\n";

    out_stream << "  // Handles any buffered request\n";
    out_stream << "  "
	       << Options::type_prefix() << "message_tags process_buffered();\n";

    out_stream << "private:\n";
    out_stream << "  " 
	       << Options::type_prefix()
	       << "message_tags process_buffered(unsigned dex);\n";
    out_stream << "  bool awaitResponse(const " << Options::type_prefix()
	       << "message_tags &, bool wait=true, bool *response=NULL);\n";
    out_stream << "   " << Options::type_prefix()
	       << "message_tags delete_buffer(unsigned dex);\n";
    out_stream << "public:\n";
  }
  return true;
}

bool interface_spec::gen_prelude(ofstream &out_stream, bool server) const {
  out_stream << "\nclass " << gen_class_name(server) << ":" 
             << " public RPCBase, public " << Options::ml->rpc_parent() << " {" << endl;
  out_stream << " public:" << endl;

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

bool interface_spec::gen_scope(ofstream &out_h, ofstream &out_c) const {
  out_h << "class " << Options::type_class() << "  {\npublic:\n";

  bool first = true;
  out_h << "typedef enum {\n ";
  for (unsigned u=0; u<Options::vec_types.size(); u++) {
    type_defn *td = Options::vec_types[u];
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

  if(!Options::shortnames()) {
    // define structs/classes inside main interface class
    // therefore, need to prepend class prefix when using
    for (unsigned u=0; u<Options::vec_types.size(); u++) {
      type_defn *td = Options::vec_types[u];
      td->gen_class(Options::ml->bundler_prefix(), out_h);
    }
  }

  out_h << "enum message_tags {\n";
  out_h << "  verify = " << base() << ",\n";

  for (dictionary_hash_iter<pdstring,remote_func*> rfi=all_functions_.begin(); rfi != all_functions_.end(); rfi++) {
    const remote_func *rf = rfi.currval();
    out_h << "  " << rf->request_tag(true) << ", " << rf->response_tag(true) << ",\n";
  }
  
  out_h << "  error, last }; \n typedef enum message_tags message_tags;\n";

  if (Options::ml->serial()) {
    out_h << "typedef struct buf_struct {\n";
    out_h << "  void *data_ptr;\n";
    out_h << "  message_tags data_type;\n";
    out_h << "} buf_struct;\n";
  }

  out_h << "};\n";

  out_h << endl;

  if (Options::ml->name() == "xdr") {
    out_h << "inline bool P_xdr_send(XDR *xdr, const " << Options::type_class()
	  << "::message_tags &tag) {" << endl;
    out_h << "   return P_xdr_send(xdr, (int32_t)tag);" << endl;
    out_h << "}" << endl;

    out_h << "inline bool P_xdr_recv(XDR *xdr, " << Options::type_class()
	  << "::message_tags &tag) {" << endl;
    out_h << "   int32_t tag_int;" << endl;
    out_h << "   if (!P_xdr_recv(xdr, tag_int)) return false;" << endl;
    out_h << "   tag = (" << Options::type_class() << "::message_tags)tag_int;" << endl;
    out_h << "   return true;" << endl;
    out_h << "}" << endl;
  }
  
  for (unsigned u1=0; u1<Options::vec_types.size(); u1++) {
    type_defn *td = Options::vec_types[u1];
    if (!td->is_in_library() && !td->is_stl()) {
      if(Options::shortnames()) 
        // define structs/classes outside main class
        // therefore, no need to prepend class prefix when using
	td->gen_class(Options::ml->bundler_prefix(), out_h);

      if (Options::ml->address_space() == message_layer::AS_many) {
	const pdstring b_pfx = Options::ml->bundler_prefix(); // e.g. P_xdr
	const pdstring t_pfx = Options::type_prefix(); // e.g. T_visi::

	out_h << endl;

	bool is_abst = td->is_abstract();
	bool printed = false;

	if( !is_abst ) {
	  // don't generate bundlers for abstract classes
	  td->gen_bundler_sig(true,
			      false, // just the prototype
			      true, // the send routine
			      t_pfx, b_pfx, out_h); out_h << std::flush;
	  td->gen_bundler_sig(true,
			      false, // just the prototype
			      false, // the recv routine
			      t_pfx, b_pfx, out_h); out_h << std::flush;
	}
	out_c << endl;
	if ( td->numFields() > 0 ) {
	  // only generate bundler implementations for those structs/classes
          // that were completely declared in the .I file and have non-virtual
          // members
	  if( ! printed )
	    out_c << "// xdr send & recv routines for "
		  << td->bundle_name() << endl;
	  td->gen_bundler_body(true, // the send routine
			       b_pfx, t_pfx, out_c); out_c << std::flush;
	  td->gen_bundler_body(false, // false --> the receive routine
			       b_pfx, t_pfx, out_c); out_c << std::flush;
	  td->gen_bundler_ptr(t_pfx, out_c, out_h);
	}
	if( is_abst ) {
	  // do generate bundlers for pointers to abstract classes
	  if( ! printed )
	    out_c << "// xdr send & recv routines for "
		  << td->bundle_name() << endl;
	  td->gen_bundler_ptr(t_pfx, out_c, out_h);
	}
	out_h << endl;
      }
    }
  }

  return true;
}

bool interface_spec::gen_inlines(ofstream &/*out_stream*/, bool /*server*/) const {
	    
  return true;
}

bool interface_spec::gen_header(ofstream &out_stream, bool server) const {
  gen_prelude(out_stream, server);
  out_stream << std::flush;

  for (dictionary_hash_iter<pdstring, remote_func*> dhi=all_functions_.begin(); dhi != all_functions_.end(); dhi++) {
    dhi.currval()->gen_signature(out_stream, true, server);
    out_stream << std::flush;
  }

  out_stream << "\nprotected:\n virtual void handle_error();\n";
  if (Options::ml->serial()) {
    out_stream << "pdvector<" << Options::type_prefix()
	       << "buf_struct*> async_buffer;\n";
    out_stream << "unsigned head;\n";
  } 
  out_stream << "};\n";

  gen_inlines(out_stream, server);

  return true;
}

bool interface_spec::gen_ctor_helper(ofstream &out_stream, bool server) const {
  if (!server) {
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

bool interface_spec::gen_ctor_4(ofstream &out_stream, bool server,
				bool hdr) const
{
  if (!hdr)
    return true;

  out_stream << (!hdr ? gen_class_prefix(server) : pdstring(""))
	     << gen_class_name(server) 
	     << "(const unsigned tid) :\n ";
  out_stream << " RPCBase(igen_no_err, 0), \n"
	     << Options::ml->rpc_parent() << "(tid)";
  if (Options::ml->serial()) out_stream << ", head(0) ";
  out_stream << " { } \n";
  return true;
}

bool interface_spec::gen_ctor_1(ofstream &out_stream, bool server,
				bool hdr) const {
  out_stream << (!hdr ? gen_class_prefix(server) : pdstring(""))
	     << gen_class_name(server) 
	     << "(PDSOCKET use_sock, int (*r)(void *,caddr_t,int), int (*w)(void *,caddr_t,int), const int nblock)";
  if (hdr) {
    out_stream << ";\n";
    return true;
  }
  
  out_stream << "\n: RPCBase(igen_no_err, 0),\n"
	     << Options::ml->rpc_parent() << "(use_sock, r, w, nblock) ";
  if (Options::ml->serial()) out_stream << " , head(0) ";
  out_stream << "\n";
  return true;
}

bool interface_spec::gen_ctor_2(ofstream &out_stream, bool server,
				bool hdr) const
{
  out_stream << (!hdr ? gen_class_prefix(server) : pdstring("")) << gen_class_name(server)
	     << "(int family, int port, int type, const pdstring host, int (*r)(void*,caddr_t,int), int (*w)(void*,caddr_t,int), int nblock)";

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

bool interface_spec::gen_ctor_3(ofstream &out_stream, bool server,
				bool hdr) const
{
  out_stream << (!hdr ? gen_class_prefix(server) : pdstring("")) << gen_class_name(server)
	     << "(const pdstring machine, const pdstring login, const pdstring program, const pdstring remote_shell, int(*rf)(void*,caddr_t,int), int (*wf)(void*,caddr_t,int), pdvector<pdstring> &args, int nblock, int port_fd)";

  if (hdr) {
    out_stream << ";\n";
    return true;
  }
  out_stream << "\n: RPCBase(igen_no_err, 0),\n"
	     << Options::ml->rpc_parent() << "(machine, login, program, remote_shell, rf, wf, args, nblock, port_fd)";
  if (Options::ml->serial()) out_stream << ", head(0) ";
  out_stream << "\n";
 
  return true;
}

bool interface_spec::gen_ctor_body(ofstream &out_stream, bool server) const {
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

bool interface_spec::gen_dtor_body(ofstream &/*out_stream*/, bool /*server*/) const {
  // gen_dtor_hdr(out_stream, server, false);
  // out_stream << "{ }\n";
  return true;
}

bool interface_spec::gen_server_verify(ofstream &out_stream) const {
  if (!Options::dont_gen_handle_err) {
    out_stream << "void " << gen_class_prefix(true) << "handle_error() {" << endl;
    out_stream << "  cerr << \"Error not handled, exiting\" << endl;" << endl;
    out_stream << "  IGEN_ERR_ASSERT;" << endl;
    out_stream << "  exit(-1);" << endl;
    out_stream << "}" << endl << endl;
  }

  if (Options::ml->address_space() == message_layer::AS_one) 
    return true;

  out_stream << "bool " << gen_class_prefix(true)
	     << "verify_protocol(bool tag_read) {\n";
  out_stream << "  unsigned tag;\n";
  out_stream << "  if (!tag_read) {\n";
  out_stream << "  " << Options::set_dir_decode() << ";\n";

  if (Options::ml->skip()) {
    out_stream << "if (!" << Options::ml->skip_message() << ") ";
    out_stream << Options::error_state(true, 6, "igen_read_err", "false");
  }

  out_stream << "  if (!" << Options::ml->bundler_prefix()
	     << "recv(net_obj(), tag)) ";
  out_stream << Options::error_state(true, 6, "igen_proto_err", "false");
  out_stream << "  if (tag != " << Options::type_prefix() << "verify) ";
  out_stream << Options::error_state(true, 6, "igen_proto_err", "false");
  out_stream << "  }" << endl;

  out_stream << endl;
  out_stream << "  " << Options::set_dir_encode() << ";\n";
  out_stream << "  tag = " << Options::type_prefix() << "verify;\n";
  out_stream << "  unsigned version = " << version() << ";\n";
  out_stream << "  pdstring name = " << "\"" << name() << "\";\n";
  out_stream << "  if (!" << Options::ml->bundler_prefix()
	     << "send(net_obj(), tag) || \n      !"
	     << Options::ml->bundler_prefix() 
	     << "send(net_obj(), name) || \n      !"
	     << Options::ml->bundler_prefix()
	     << "send(net_obj(), version)) ";
  out_stream << Options::error_state(true, 6, "igen_encode_err", "false");

  out_stream << "  if (!" << Options::ml->send_message() << ") ";
  out_stream << Options::error_state(true, 6, "igen_send_err", "false");
  out_stream << "  setVersionVerifyDone();\n";
  out_stream << "  return true;\n}" << endl;

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

    out_stream << "  unsigned tag = " << Options::type_prefix() << "verify;\n";
    out_stream << Options::set_dir_encode() << ";\n";
    out_stream << "  if (!" << Options::ml->bundler_prefix()
               << "send(net_obj(), tag)) ";
    out_stream << Options::error_state(true, 6, "igen_encode_err", "false");

    out_stream << "  if (!" << Options::ml->send_message() << ") ";
    out_stream << Options::error_state(true, 6, "igen_send_err", "false");

    out_stream << "  if (!awaitResponse(" << Options::type_prefix() << "verify)) ";
    out_stream << Options::error_state(true, 6, "igen_proto_err", "false");

    out_stream << "  " << Options::set_dir_decode() << ";\n";
    out_stream << "  pdstring proto;\n";
    out_stream << "  if (!" << Options::ml->bundler_prefix()
               << "recv(net_obj(), proto) ||" << endl
               << "      !" << Options::ml->bundler_prefix() << "recv(net_obj(), tag)) {\n";
    out_stream << "    set_err_state(igen_proto_err);\n";
    out_stream << "    cerr << \"Protocol verify - bad response from server\" << endl;";
    out_stream << "    handle_error(); exit(-1); return false;" << endl;
    out_stream << "  }" << endl;
    
    out_stream << "  if ((tag != " << version() << " || proto != \""
               << name() << "\")) {" << endl;
    out_stream << "     cerr << \" Protocol " << name() << " version " << version()
               << " expected\" << endl;";
    out_stream << "     cerr << \" Found Protocol \" << proto << \" version \" << tag << endl;";
    out_stream << "     set_err_state(igen_proto_err);\n";

    out_stream << "     handle_error(); return false;" << endl;
    out_stream << "  }" << endl;
    // removed the exit(-1) so the caller can look at errorCondition flag
    // and do the polite thing.  Of course this raises a danger; the caller
    // must not forget to check! --ari 03/96

    out_stream << "  return true;\n}" << endl;
  }
  return true;
}

bool interface_spec::gen_interface() const {
  // Generate <file>.xdr.h and <file>.xdr.C:
  gen_scope(Options::dot_h, Options::dot_c);

   // Generate <file>.CLNT.xdr.h:
  gen_header(Options::clnt_dot_h, false); Options::clnt_dot_h << std::flush;

  // Generate <file>.SRVR.xdr.h:
  gen_header(Options::srvr_dot_h, true); Options::srvr_dot_h << std::flush;

  gen_ctor_body(Options::srvr_dot_c, true); Options::srvr_dot_c << std::flush;
  gen_ctor_body(Options::clnt_dot_c, false); Options::clnt_dot_c << std::flush;

  gen_dtor_body(Options::srvr_dot_c, true); Options::srvr_dot_c << std::flush;
  gen_dtor_body(Options::clnt_dot_c, false); Options::clnt_dot_c << std::flush;

  for (dictionary_hash_iter<pdstring, remote_func*> rfi=all_functions_.begin(); rfi != all_functions_.end(); rfi++) {
    rfi.currval()->gen_stub(Options::srvr_dot_c, Options::clnt_dot_c);
    Options::clnt_dot_c << std::flush;
  }

  Options::clnt_dot_c << endl;
  gen_await_response(Options::clnt_dot_c, false); Options::clnt_dot_c << std::flush;

  Options::srvr_dot_c << endl;
  gen_await_response(Options::srvr_dot_c, true); Options::srvr_dot_c << std::flush;

  Options::clnt_dot_c << endl;
  gen_wait_loop(Options::clnt_dot_c, false); Options::clnt_dot_c << std::flush;

  Options::srvr_dot_c << endl;
  gen_wait_loop(Options::srvr_dot_c, true); Options::srvr_dot_c << std::flush;

  Options::srvr_dot_c << endl;
  gen_process_buffered(Options::srvr_dot_c, true); Options::srvr_dot_c << std::flush;

  Options::clnt_dot_c << endl;
  gen_process_buffered(Options::clnt_dot_c, false); Options::clnt_dot_c << std::flush;

  Options::clnt_dot_c << endl;
  gen_client_verify(Options::clnt_dot_c); Options::clnt_dot_c << std::flush;

  Options::srvr_dot_c << endl;
  gen_server_verify(Options::srvr_dot_c); Options::srvr_dot_c << endl;

  if (Options::stl_seen) {
    gen_stl_temps();
  }

  return true;
}

// I am assuming that Vector has already been included
bool interface_spec::gen_stl_temps() const {

  if (Options::ml->address_space() == message_layer::AS_many) 
    Options::temp_dot_c << "template class pdvector<"
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
      // explicitly instantiate bundler recv
      Options::temp_dot_c << endl << "template "
			  << Options::ml->bundler_return_type() << " "
			  << Options::ml->bundler_prefix() << "recv("
			  << Options::ml->marshall_obj() << Options::ml->marshall_obj_ptr() 
			  << ", " << Options::stl_types[stl_index1].name << "<"
			  << Options::stl_types[stl_index1].elements[el_index].name
			  << ">&);\n";
      // explicitly instantiate bundler send
      Options::temp_dot_c << "template "
			  << Options::ml->bundler_return_type() << " "
			  << Options::ml->bundler_prefix() << "send("
			  << Options::ml->marshall_obj() << Options::ml->marshall_obj_ptr() 
			  << ", " << "const " << Options::stl_types[stl_index1].name << "<"
			  << Options::stl_types[stl_index1].elements[el_index].name
			  << ">&);\n";
      // explicitly instantiate bundler send_common
      Options::temp_dot_c << "template "
			  << Options::ml->bundler_return_type() << " "
			  << Options::ml->bundler_prefix() << "send_common("
			  << Options::ml->marshall_obj() << Options::ml->marshall_obj_ptr() 
			  << ", " << Options::stl_types[stl_index1].name << "<"
			  << Options::stl_types[stl_index1].elements[el_index].name
			  << "> const &, bool (*)(" << Options::ml->marshall_obj() 
			  << Options::ml->marshall_obj_ptr() << ", " 
			  << Options::stl_types[stl_index1].elements[el_index].name
			  << " const &));\n";
      // explicitly instantiate writerfn_noMethod
      Options::temp_dot_c << "template "
			  << Options::ml->bundler_return_type() << " "
			  << "writerfn_noMethod("
			  << Options::ml->marshall_obj() << Options::ml->marshall_obj_ptr() 
			  << ", "
			  << Options::stl_types[stl_index1].elements[el_index].name 
			  << " const &);\n";
    }
  }

  for (unsigned stl_index2=0; stl_index2 < Options::stl_types.size(); stl_index2++) {
    for (unsigned el_index=0; el_index < Options::stl_types[stl_index2].elements.size();
	 el_index++) {
      // explicitly instantiate bundler recv for pointer
      Options::temp_dot_c << endl << "template "
			  << Options::ml->bundler_return_type() << " "
			  << Options::ml->bundler_prefix() << "recv("
			  << Options::ml->marshall_obj() << Options::ml->marshall_obj_ptr() 
			  << ", " << Options::stl_types[stl_index2].name << "<"
			  << Options::stl_types[stl_index2].elements[el_index].name
			  << ">*&);\n";
      // explicitly instantiate bundler send for pointer
      Options::temp_dot_c << "template "
			  << Options::ml->bundler_return_type() << " "
			  << Options::ml->bundler_prefix() << "send("
			  << Options::ml->marshall_obj() << Options::ml->marshall_obj_ptr() 
			  << ", " << Options::stl_types[stl_index2].name << "<"
			  << Options::stl_types[stl_index2].elements[el_index].name
			  << ">*);\n";
    }
  }
  
  return true;
}

bool interface_spec::gen_process_buffered(ofstream &out_stream, bool srvr) const {
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

  out_stream << endl << endl;

  
  
  out_stream << Options::type_prefix() << "message_tags "
             << gen_class_prefix(srvr) << "delete_buffer(unsigned index) {\n";
  out_stream << "  unsigned asize = async_buffer.size();" << endl;
  out_stream << "  assert(index < asize);" << endl;
  out_stream << "  " << Options::type_prefix()
             << "buf_struct *item = async_buffer[index];\n";

  out_stream << "  unsigned elems_left = asize - head - 1;" << endl;
  out_stream << "  if (index == head)" << endl;
  out_stream << "    head++;" << endl;
  out_stream << "  else if (index == (asize - 1))" << endl;
  out_stream << "    async_buffer.resize(asize - 1);" << endl;
  out_stream << "  else {" << endl;
  out_stream << "    async_buffer[index] = async_buffer[asize - 1];" << endl;
  out_stream << "    async_buffer.resize(asize - 1);" << endl;
  out_stream << "  }" << endl;

  out_stream << "  if (!elems_left) {" << endl;
  out_stream << "    head = 0; async_buffer.resize(0);" << endl;
  out_stream << "  }" << endl;
  out_stream << "  assert((!async_buffer.size() && !head) || (elems_left == (async_buffer.size()-head)));" << endl;
  out_stream << "  " << Options::type_prefix()
             << "message_tags tag = item->data_type;\n";

  out_stream << endl;
  out_stream << "  switch (tag) {\n";
  
  for (dictionary_hash_iter<pdstring, remote_func*> rfi=all_functions_.begin(); rfi != all_functions_.end(); rfi++) {
    const remote_func *rf = rfi.currval();
    rf->free_async("      ", out_stream, srvr);
  }
  
  out_stream << "   default:\n";
  out_stream << "       IGEN_ERR_ASSERT;\n";
  out_stream << "       set_err_state(igen_request_err);\n";
  out_stream << "       return " << Options::type_prefix() << "error" << ";\n";
  out_stream << "       break;" << endl;
  out_stream << "  }" << endl;

  out_stream << "  delete item;" << endl;
  out_stream << "  return tag;" << endl;
  out_stream << "}" << endl;

  return true;
}

bool interface_spec::gen_await_response(ofstream &out_stream, bool srvr) const {
  if (!Options::ml->serial())
    return true;

  out_stream << "bool " << gen_class_prefix(srvr)
	     << "awaitResponse(const " << Options::type_prefix()
	     << "message_tags &target_tag, bool wait, bool *response) {" << endl;
  out_stream << "  unsigned tag;" << endl;
  out_stream << "  if (!wait) *response = false;" << endl;
  out_stream << "  if (get_err_state() != igen_no_err) return false;" << endl;
  out_stream << "  " << Options::set_dir_decode() << ";" << endl;

  out_stream << "  do {" << endl;

  if (Options::ml->records_used()) {
    out_stream << "    if (!" << Options::ml->skip_message() << ") ";
    out_stream << Options::error_state(true, 9, "igen_read_err", "false");
  }

  out_stream << "    if (!" << Options::ml->recv_tag("net_obj()", "tag") << ") ";
  out_stream << Options::error_state(true, 9, "igen_read_err", "false");

  out_stream << "    if (tag == (unsigned)target_tag) {" << endl;
  out_stream << "      if (!wait) *response = true;" << endl;
  out_stream << "      return true;" << endl;
  out_stream << "    }" << endl;

  out_stream << "    switch (tag) {" << endl;

  out_stream << std::flush;
  for (dictionary_hash_iter<pdstring, remote_func*> rfi=all_functions_.begin(); rfi != all_functions_.end(); rfi++) {
    rfi.currval()->save_async_request("         ", out_stream, srvr);
    out_stream << std::flush;
  }

  out_stream << "     default:\n";
  out_stream << "         set_err_state(igen_request_err);\n";
  out_stream << "         IGEN_ERR_ASSERT;\n";
  out_stream << "         return false;\n";
  out_stream << "    }\n";

  out_stream << "  } while(wait);\n";
  out_stream << "  assert(0);\n";
  out_stream << "  return false;\n";
  out_stream << "}\n";
  out_stream << std::flush;

  return true;
}

bool interface_spec::gen_wait_loop(ofstream &out_stream, bool srvr) const {
  out_stream << Options::type_prefix() << "message_tags "
	     << gen_class_prefix(srvr) << "waitLoop(";
  
  if (Options::ml->address_space() == message_layer::AS_many) {
    out_stream << ") {\n";
  } else {
    out_stream << " bool specific, "
	       << Options::type_prefix() << "message_tags mt, void** buffer) {\n"; 
  }

  out_stream << "  " << Options::type_prefix() << "message_tags tag;\n";

  out_stream << "  if (get_err_state() != igen_no_err) return "
	     << Options::type_prefix() << "error;" << endl;

  out_stream << "  " << Options::set_dir_decode() << ";" << endl;

  if (Options::ml->records_used()) {
    if (Options::ml->skip()) {
      out_stream << "  if (!" << Options::ml->skip_message() << ") ";
      out_stream << Options::error_state(true, 6, "igen_read_err",
					 Options::type_prefix()+"error");
    }
  }

  if (Options::ml->address_space() != message_layer::AS_one) {
    out_stream << "  if (!" << Options::ml->recv_tag("net_obj()", "tag") << ") ";
    out_stream << Options::error_state(true, 6, "igen_read_err",
				       Options::type_prefix()+"error");
    out_stream << "  return switch_on(tag);" << endl;
  } else {
    out_stream << "  if (!specific)" << endl;
    out_stream << "    tag = " << Options::type_prefix() << "message_tags(MSG_TAG_ANY);\n";
    out_stream << "  else \n";
    out_stream << "    tag = mt;\n";

    out_stream << "  void* msgBuf = NULL;\n";
    out_stream << "  thread_t tid = THR_TID_UNSPEC;\n";
    out_stream << "  int err = msg_recv(&tid, (unsigned*)&tag, &msgBuf);\n";
    out_stream << "  setRequestingThread((err == THR_ERR) ? (unsigned)err : tid);\n";
    out_stream << "  if (getRequestingThread() == THR_ERR) ";
    out_stream << Options::error_state(true, 6, "igen_read_err",
				       Options::type_prefix()+"error");
    out_stream << "  return switch_on(tag, msgBuf, buffer);\n"; 
  }

  out_stream << "}" << endl;
  out_stream << endl;

  out_stream << Options::type_prefix() << "message_tags" << endl;
   
  out_stream << gen_class_prefix(srvr)
	     << "switch_on(" << Options::type_prefix() << "message_tags tag";
  if (Options::ml->address_space() == message_layer::AS_one) 
  {
    out_stream << ", void* msgBuf";
  }
  out_stream << ", void** buffer) {" << endl;

  if (Options::ml->address_space() == message_layer::AS_one) {
    out_stream << "  int val = THR_OKAY;\n";
  }

  out_stream << "  switch (tag) {" << endl;

  if (srvr && (Options::ml->address_space() != message_layer::AS_one)) {
    out_stream << "   case " << Options::type_prefix() << "verify:\n";
    out_stream << "       if (!verify_protocol(true)) ";
    out_stream << Options::error_state(true, 12, "igen_proto_err",
				       Options::type_prefix()+"error");
    out_stream << "         return tag;\n";
  }

  for (dictionary_hash_iter<pdstring, remote_func*> rfi=all_functions_.begin(); rfi != all_functions_.end(); rfi++) {
    rfi.currval()->handle_request("   ", out_stream, srvr, true);
  }

  out_stream << "   default:\n";
  out_stream << "       set_err_state(igen_request_err);\n";
  out_stream << "       IGEN_ERR_ASSERT;\n";
  out_stream << "       return " << Options::type_prefix() << "error;\n";
  out_stream << "  }\n";

  if (Options::ml->address_space() == message_layer::AS_one) {
    out_stream << "  if (val == THR_ERR) ";
    out_stream << Options::error_state(true, 6, "igen_read_err", Options::type_prefix()+"error");
  }

  out_stream << "  return tag;" << endl;
  out_stream << "}" << endl;

   
  out_stream << endl;
  if (Options::ml->address_space() != message_layer::AS_one) {
    out_stream << "bool " << gen_class_prefix(srvr)
	       << "wait_for(" << Options::type_prefix() << "message_tags tag) {\n";

    out_stream << "  unsigned size = async_buffer.size();\n";
    out_stream << "  for (unsigned u=head; u<size; u++) \n";
    out_stream << "    if (async_buffer[u]->data_type == tag) {\n";
    out_stream << "      if (process_buffered(u) != tag) \n return false;\n";
    out_stream << "      else \n return true;\n";
    out_stream << "    }\n";

    out_stream << "  if (!awaitResponse(tag)) ";
    out_stream << Options::error_state(true, 6, "igen_proto_err", "false");
    
    out_stream << "  if (!switch_on(tag)) \n return false;\n";
    out_stream << "  else \n return true;\n";
    out_stream << "}\n";

    // New procedure required for async enableData requests
    out_stream << "bool " << gen_class_prefix(srvr)
	       << "wait_for_and_read(" << Options::type_prefix() 
	       << "message_tags tag, void** buffer) {\n";
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

bool interface_spec::new_remote_func(const pdstring *name, pdvector<arg*> *arglist,
				     const remote_func::call_type &callT,
				     bool is_virtual, const arg &return_arg,
				     bool do_free) {
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
  if (!text || (strlen(text) < 17)) return;
  char *buffer = strdup(text);
  char *temp = buffer;
  temp[strlen(temp) - 8] = (char) 0;
  temp += 8;
  if (is_srvr)
    server_ignore += temp;
  else
    client_ignore += temp;
  free(buffer);
}
