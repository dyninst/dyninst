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

// signature.h
// Put in by Ariel Tamches; moved from parse.h into its own .h/.C file combo
// for clarity.

#ifndef _SIGNATURE_H_
#define _SIGNATURE_H_

#include "common/h/Vector.h"
#include "common/h/String.h"
#include <fstream>
#include "arg.h"

using std::ofstream;

class signature {
public:
  signature(pdvector<arg*> *alist, const pdstring rf_name);
  ~signature() { }

  pdstring type(const bool use_bool=false) const;
  pdstring base_type() const { return type_;}
  void type(const pdstring t, const unsigned star);
  bool gen_sig(ofstream &out_stream) const;
  bool tag_bundle_send(ofstream &out_stream, const pdstring &return_value,
		       const pdstring &req_tag) const;
  bool tag_bundle_send_many(ofstream &out_stream,
                            const pdstring &return_value,
                            const pdstring &req_tag) const;
  bool tag_bundle_send_one(ofstream &out_stream,
                           const pdstring return_value,
                           const pdstring req_tag) const;
  bool arg_struct(ofstream &out_stream) const;
  pdstring dump_args(const pdstring message, const pdstring sep) const;
  pdstring gen_bundler_call(bool send_routine, const pdstring &obj_name,
                            const pdstring &data_name) const;
  unsigned int num_args( void ) const   { return args.size(); } 

private:
  pdvector<arg*> args;
  pdstring type_;
  bool is_const_;
  unsigned stars_;
};

#endif
