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

#ifndef _MDL_DATA_H
#define _MDL_DATA_H

// The daemon needs to include this file at times.


#include "dyninstRPC.xdr.h"

class mdl_data {
public:
  static dictionary_hash<unsigned, pdvector<mdl_type_desc> > fields;
  static pdvector<mdl_focus_element> foci;
  static pdvector<T_dyninstRPC::mdl_stmt*> stmts;
  static pdvector<T_dyninstRPC::mdl_constraint*> all_constraints;
  static pdvector<string> lib_constraints;
  static pdvector<unsigned> lib_constraint_flags;

  static T_dyninstRPC::mdl_constraint *new_constraint(string id, pdvector<string> *path,
					       pdvector<T_dyninstRPC::mdl_stmt*> *stmts,
					       bool replace, u_int data_type);

  static pdvector<T_dyninstRPC::mdl_metric*> all_metrics;
  static bool new_metric(string id, string name, string units,
			 u_int agg, u_int style, u_int type, string hwcntr, 
			 pdvector<T_dyninstRPC::mdl_stmt*> *stmts, 
			 pdvector<string> *flavs,
			 pdvector<T_dyninstRPC::mdl_constraint*> *cons,
		 pdvector<string> *temp_counters,
			 bool developerMode,
			 int normalized);


  // unique_name: prepares for the declaration of a new mdl object,
  // by deleting all objects previously declared that have the same
  // name.
  static void unique_name(string name);


};


#endif
