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

// interface_spec.h
// Put in by Ariel Tamches; moved from parse.h into its own .h/.C file combo
// for clarity.

#ifndef _INTERFACE_SPEC_H_
#define _INTERFACE_SPEC_H_

#include "common/h/String.h"
#include "common/h/Dictionary.h"
#include "remote_func.h"
#include "arg.h"

using std::ofstream;

class interface_spec {
public:
  interface_spec(const pdstring *name, const unsigned &b, const unsigned &v);
  ~interface_spec();

  bool gen_interface() const;

  // TODO reverse arg list ?
  bool new_remote_func(const pdstring *name, pdvector<arg*> *arglist,
		       const remote_func::call_type &callT,
		       bool is_virtual, const arg &return_arg,
		       bool do_free);
  
  void ignore(bool is_srvr, char *text);
  bool are_bundlers_generated() const;
  pdstring name() const { return name_;}
  unsigned base() const { return base_;}
  unsigned version() const { return version_;}

  pdstring gen_class_name(bool server) const { 
    return (server ? server_name_ : client_name_); }
  
  pdstring gen_class_prefix(bool server) const {
    return (server ? server_prefix_ : client_prefix_);}
  
  bool gen_process_buffered(ofstream &out_stream, bool srvr) const;
  bool gen_await_response(ofstream &out_stream, bool srvr) const;
  bool gen_wait_loop(ofstream &out_stream, bool srvr) const;
  bool gen_scope(ofstream &out_h, ofstream &out_c) const;
  bool gen_client_verify(ofstream &out_stream) const;
  bool gen_server_verify(ofstream &out_stream) const;

private:
  pdstring name_;
  unsigned base_;
  unsigned version_;
  pdstring client_prefix_;
  pdstring server_prefix_;
  pdstring client_name_;
  pdstring server_name_;

  pdvector<pdstring> client_ignore;
  pdvector<pdstring> server_ignore;

  bool gen_stl_temps() const;
  bool gen_stl_bundler(ofstream &out_h, ofstream &out_c) const;
  bool gen_stl_bundler_ptr(ofstream &out_h, ofstream &out_c) const;
  bool gen_header(ofstream &out_stream, bool server) const;
  bool gen_inlines(ofstream &out_stream, bool server) const;
  bool gen_prelude(ofstream &out_stream, bool server) const;
  bool gen_dtor_hdr(ofstream &out_stream, bool server, bool hdr) const;
  bool gen_ctor_hdr(ofstream &out_stream, bool server) const;
  bool gen_dtor_body(ofstream &out_stream, bool server) const;
  bool gen_ctor_body(ofstream &out_stream, bool server) const;
  bool gen_ctor_helper(ofstream &out_stream, bool server) const;
  bool gen_ctor_1(ofstream &out_stream, bool server,
		  bool hdr) const;
  bool gen_ctor_2(ofstream &out_stream, bool server,
		  bool hdr) const;
  bool gen_ctor_3(ofstream &out_stream, bool server,
		  bool hdr) const;
  bool gen_ctor_4(ofstream &out_stream, bool server,
		  bool hdr) const;

  dictionary_hash<pdstring, remote_func*> all_functions_;
};

#endif
