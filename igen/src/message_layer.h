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

// message_layer.h
// Put in by Ariel Tamches; moved from parse.h into its own .h/.C file combo
// for clarity.

#ifndef _MESSAGE_LAYER_H_
#define _MESSAGE_LAYER_H_

#include "common/h/String.h"

class message_layer {
public:
  typedef enum { AS_one, AS_many, AS_none } AS;
  typedef enum { Med_xdr, Med_thread, Med_other, Med_none, Med_rpc, Med_mrnet} medium;

  message_layer() { med_ = Med_none; }
  message_layer(const pdstring) { }
  message_layer(const pdstring nm, const medium md, const pdstring bp, const pdstring brt,
		const pdstring mdp, const pdstring mo, const pdstring mop, const AS as,
		const pdstring bfail, const pdstring bok, const pdstring dir_f,
		const pdstring pack_f, const pdstring unpack_f, const pdstring free_c,
		const pdstring rpc_par, const pdstring tag_type,
		const pdstring send_msg, const bool r_used,
		const pdstring skip_msg, const pdstring r_msg, const pdstring incs,
		const bool do_serial, const pdstring enc, const pdstring dec,
		const bool do_skip);
  ~message_layer() { }
  bool operator== (const message_layer &other) const { return (name_ == other.name());}

  const pdstring &name() const { return name_;}
  medium med() const { return med_;}
  const pdstring &bundler_prefix() const { return bundler_prefix_;}
  const pdstring &bundler_return_type() const { return bundler_return_type_;}
  const pdstring &marshall_data_ptr() const { return marshall_data_ptr_;}
  const pdstring &marshall_obj() const { return marshall_obj_;}  
  const pdstring &marshall_obj_ptr() const { return marshall_obj_ptr_;}
  AS address_space() const { return address_space_;}
  const pdstring &bundle_fail() const { return bundle_fail_;}
  const pdstring &bundle_ok() const { return bundle_ok_;}
  const pdstring &dir_field() const { return dir_field_;}
  const pdstring &pack_const() const { return pack_const_;}
  const pdstring &unpack_const() const { return unpack_const_;}
  const pdstring &free_const() const { return free_const_;}
  const pdstring &rpc_parent() const { return rpc_parent_;}
  const pdstring &tag_type() const { return tag_type_;}
  const pdstring &send_message() const { return send_message_;}
  pdstring send_tag(const pdstring &obj_name, const pdstring &tag) const;
  pdstring recv_tag(const pdstring &obj_name, const pdstring &tag) const;
  bool records_used() const { return records_used_;}
  const pdstring &skip_message() const { return skip_message_;}
  const pdstring &recv_message() const { return recv_message_;}
  const pdstring &includes() const { return incs_;}
  bool serial() const { return serial_;}
  const pdstring &set_dir_decode() const { return decode_;}
  const pdstring &set_dir_encode() const { return encode_;}
  bool skip() const { return skip_;}

private:
  pdstring name_;
  medium med_;
  pdstring bundler_prefix_;
  pdstring bundler_return_type_;
  pdstring dir_is_free_;
  pdstring marshall_data_ptr_;
  pdstring marshall_obj_;
  pdstring marshall_obj_ptr_;
  AS address_space_;
  pdstring bundle_fail_;
  pdstring bundle_ok_;
  pdstring dir_field_;
  pdstring pack_const_;
  pdstring unpack_const_;
  pdstring free_const_;
  pdstring rpc_parent_;
  pdstring tag_type_;
  pdstring send_message_;
  bool records_used_;
  pdstring skip_message_;
  pdstring recv_message_;
  pdstring incs_;
  bool serial_;
  pdstring encode_;
  pdstring decode_;
  bool skip_;
};

#endif
