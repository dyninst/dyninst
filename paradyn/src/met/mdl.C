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

// $Id: mdl.C,v 1.61 2003/07/18 15:44:49 schendel Exp $

#include "dyninstRPC.xdr.CLNT.h"
#include "paradyn/src/met/globals.h"
#include "paradyn/src/met/metricExt.h"
#include "pdutil/h/mdl.h"

#include <iostream>


bool mdl_get_lib_constraints(pdvector<pdstring> &lc, pdvector<unsigned> &lcf){
    for(u_int i=0; i < mdl_data::cur_mdl_data->lib_constraints.size(); i++){
        lc += mdl_data::cur_mdl_data->lib_constraints[i];
		lcf += mdl_data::cur_mdl_data->lib_constraint_flags[i];
    }
    return (lc.size()>0);
}

void mdl_destroy() {
  unsigned size = mdl_data::cur_mdl_data->all_constraints.size();
  for (unsigned u=0; u<size; u++)
    delete (mdl_data::cur_mdl_data->all_constraints[u]);
  mdl_data::cur_mdl_data->all_constraints.resize(0);
  
  size = mdl_data::cur_mdl_data->all_metrics.size();
  for (unsigned u1=0; u1<size; u1++)
    delete (mdl_data::cur_mdl_data->all_metrics[u1]);
  mdl_data::cur_mdl_data->all_metrics.resize(0);

  size = mdl_data::cur_mdl_data->stmts.size();
  for (unsigned u2=0; u2<size; u2++)
    delete mdl_data::cur_mdl_data->stmts[u2];
  mdl_data::cur_mdl_data->stmts.resize( 0 );
}

