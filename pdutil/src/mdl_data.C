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
//
// $Id: mdl_data.C,v 1.1 2003/06/07 12:39:45 pcroth Exp $
//
#include "pdutil/h/mdl_data.h"

inline unsigned ui_hash(const unsigned &u) { return u; }

// static members of mdl_env
pdvector<mdl_env::Frame> mdl_env::frames;
pdvector<mdl_var> mdl_env::all_vars;
string mdl_env::savedMsg;

// static members of mdl_data
pdvector<T_dyninstRPC::mdl_stmt*> mdl_data::stmts;
pdvector<T_dyninstRPC::mdl_metric*> mdl_data::all_metrics;
dictionary_hash<unsigned, pdvector<mdl_type_desc> > mdl_data::fields(ui_hash);
pdvector<mdl_focus_element> mdl_data::foci;
pdvector<T_dyninstRPC::mdl_constraint*> mdl_data::all_constraints;
pdvector<string> mdl_data::lib_constraints;
pdvector<unsigned int> mdl_data::lib_constraint_flags;


void mdl_data::unique_name(string name) {
  unsigned u, v;
    
  unsigned sz = mdl_data::stmts.size();
  for (u = 0; u < sz; u++) {
    T_dyninstRPC::mdl_list_stmt *lstmt = (T_dyninstRPC::mdl_list_stmt *) mdl_data::stmts[u];
    if (lstmt->id_ == name) {
      delete mdl_data::stmts[u];
      for (v = u; v < sz-1; v++) {
    mdl_data::stmts[v] = mdl_data::stmts[v+1];
      }
      mdl_data::stmts.resize(sz-1);
      break;
    }
  }

  sz = mdl_data::all_constraints.size();
  for (u = 0; u < sz; u++) {
    if (mdl_data::all_constraints[u]->id_ == name) {
      delete mdl_data::all_constraints[u];
      for (v = u; v < sz-1; v++) {
    mdl_data::all_constraints[v] = mdl_data::all_constraints[v+1];
      }
      mdl_data::all_constraints.resize(sz-1);
      break;
    }
  }

  sz = mdl_data::all_metrics.size();
  for (u = 0; u < sz; u++) {
    if (mdl_data::all_metrics[u]->id_ == name) {
      delete mdl_data::all_metrics[u];
      for (v = u; v < sz-1; v++) {
        mdl_data::all_metrics[v] = mdl_data::all_metrics[v+1];
      }
      mdl_data::all_metrics.resize(sz-1);
      break;
    }
  }
}

