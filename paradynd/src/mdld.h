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

// $Id: mdld.h,v 1.22 2004/03/23 01:12:35 eli Exp $

#if !defined(mdl_daemon_hdr)
#define mdl_daemon_hdr

#include "pdutil/h/metricStyle.h"
#include "paradynd/src/comm.h"

typedef struct {
  int aggregate;
  metricStyle style;
} mdl_inst_data;

extern void mdl_get_info(pdvector<T_dyninstRPC::metricInfo>&);
extern bool mdl_metric_data(const pdstring&, mdl_inst_data&);

extern machineMetFocusNode *makeMachineMetFocusNode(int mid, 
                                    const Focus& focus, const pdstring &met_name,
				    pdvector<pd_process *> procs,
				    bool replace_components_if_present,
				    bool enable);
extern processMetFocusNode *makeProcessMetFocusNode(const Focus& focus, 
				    const pdstring &met_name, pd_process *procs,
				    bool replace_components_if_present,
				    bool enable);

extern bool mdl_can_do(const pdstring &metric_name);

//extern metricFocusNode *mdl_observed_cost(pdvector< pdvector<pdstring> >& canon_focus,
//					       pdstring& met_name,
//					       pdstring& flat_name,
//					       pdvector<process *> procs);

// Get the initial mdl info (metrics, constraints, statements)
extern bool mdl_get_initial(pdstring flavor, pdRPC*);

extern bool mdl_get_lib_constraints(pdvector<pdstring> &);

bool mdl_init_be( pdstring& flavor );
bool mdl_apply( void );
bool mdl_check_node_constraints( void );
extern bool saw_mdl;

#endif
