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

// message_layer.C

#include "message_layer.h"
#include "Options.h"

message_layer::message_layer(const pdstring nm, const medium md, const pdstring bp,
			     const pdstring brt, const pdstring mdp, const pdstring mo,
			     const pdstring mop, const AS as, const pdstring bfail,
			     const pdstring bok, const pdstring dir_f, const pdstring pack_f,
			     const pdstring unpack_f, const pdstring free_f,
			     const pdstring rpc_par, const pdstring tag_type,
				 const pdstring send_msg,
			     const bool r_used, const pdstring skip_msg,
			     const pdstring recv_msg, const pdstring incs,
			     const bool do_serial, 
			     const pdstring enc, const pdstring dec,
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

pdstring message_layer::send_tag(const pdstring &obj_name, const pdstring &tag_s) const {
   return Options::ml->bundler_prefix() + // e.g. "P_xdr"
          "send(" +
          obj_name +
          ", " + tag_s + ")";
}

pdstring message_layer::recv_tag(const pdstring &obj_name, const pdstring &tag_s) const {
   return Options::ml->bundler_prefix() + // e.g. "P_xdr"
          "recv(" +
          obj_name +
          ", " + tag_s + ")";
}
