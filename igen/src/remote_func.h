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

// remote_func.h
// Put in by Ariel Tamches; moved from parse.h into its own .h/.C file combo
// for clarity.

#ifndef _REMOTE_FUNC_H_
#define _REMOTE_FUNC_H_

#include "common/h/Vector.h"
#include "arg.h" // class arg
#include "signature.h"

using std::ofstream;

class remote_func { 
public:
  typedef enum { async_upcall,      // from server to client, async
		 sync_call,         // from client to server, sync
		 async_call         // from client to server, async
		 } call_type; 

  remote_func(const pdstring name, pdvector<arg*> *arglist,
              const call_type &ct, const bool &is_v, const arg &return_arg,
              const bool do_free);
  ~remote_func() { }
  bool operator== (const remote_func &other) const {
     return (other.name() == name_);
  }

  bool gen_stub(ofstream &out_srvr, ofstream &out_clnt) const;
  bool gen_signature(ofstream &out_stream, const bool &hdr,
                     const bool srvr) const;
  bool gen_async_struct(ofstream &out_stream) const;
  bool save_async_request(const pdstring &spaces, ofstream &out_stream,
                          const bool srvr) const;
  bool free_async(const pdstring &spaces, ofstream &out_stream,
                  const bool srvr) const;
  bool handle_request(const pdstring &spaces, ofstream &out_stream,
                      const bool srvr, bool special=false) const;

  pdstring request_tag(bool unqual=false) const;
  pdstring response_tag(bool unqual=false) const;

  bool is_virtual() const { return is_virtual_;}
  bool is_void() const { return (return_arg_.is_void());}
  bool is_srvr_call() const { return (function_type_ == async_upcall);}
  bool is_async_call() const {
    return ((function_type_ == async_upcall) ||
            (function_type_ == async_call));
  }

  pdstring name() const { return name_; }
  call_type function_type() const { return function_type_;}
  pdstring return_value() const {
     return ((is_async_call()||is_void()) ? 
             pdstring("") : pdstring("ret_arg"));
  }
  bool do_free() const { return do_free_;}
  pdstring sig_type(const bool use_const=false) const {
     return call_sig_.type(use_const);
  }
  pdstring ret_type(const bool use_const=false) const {
     return return_arg_.type(use_const);
  }

private:
  pdstring name_;
  call_type function_type_;
  bool is_virtual_;
  signature call_sig_;
  arg return_arg_;
  bool do_free_;

  bool gen_stub_helper(ofstream &out_srvr, ofstream &out_clnt,
                       const bool server) const;
  bool gen_stub_helper_many(ofstream &out_srvr, ofstream &out_clnt,
                            const bool server) const;
  bool gen_stub_helper_one(ofstream &out_srvr, ofstream &out_clnt,
                           const bool server) const;
};

#endif
