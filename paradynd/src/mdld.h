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

/*
 * $Log: mdld.h,v $
 * Revision 1.9  1997/06/05 04:27:07  newhall
 * changed exclude_lib to mean shared object is not instrumentable
 * added exclude_func mdl option to exclude shared object functions
 *
 * Revision 1.8  1997/02/21 20:15:54  naim
 * Moving files from paradynd to dyninstAPI + eliminating references to
 * dataReqNode from the ast class. This is the first pre-dyninstAPI commit! - naim
 *
 * Revision 1.7  1997/01/15 00:28:31  tamches
 * extra bool param to mdl_do()
 *
 * Revision 1.6  1996/09/26 18:58:48  newhall
 * added support for instrumenting dynamic executables on sparc-solaris
 * platform
 *
 * Revision 1.5  1996/08/16 21:19:18  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.4  1996/03/12 20:48:24  mjrg
 * Improved handling of process termination
 * New version of aggregateSample to support adding and removing components
 * dynamically
 * Added error messages
 *
 * Revision 1.3  1996/01/29 22:09:28  mjrg
 * Added metric propagation when new processes start
 * Adjust time to account for clock differences between machines
 * Daemons don't enable internal metrics when they are not running any processes
 * Changed CM5 start (paradynd doesn't stop application at first breakpoint;
 * the application stops only after it starts the CM5 daemon)
 *
 * Revision 1.2  1995/08/24  15:04:16  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 *
 */
#if !defined(mdl_daemon_hdr)
#define mdl_daemon_hdr

#include "paradynd/src/metric.h"
#include "paradynd/src/comm.h"

typedef struct {
  int aggregate;
  metricStyle style;
} mdl_inst_data;

extern void mdl_get_info(vector<T_dyninstRPC::metricInfo>&);
extern bool mdl_metric_data(const string&, mdl_inst_data&);

extern metricDefinitionNode *mdl_do(vector< vector<string> >& canon_focus,
				    string& metric_name, string& flat_name,
				    vector<process *> procs,
				    bool, bool);
extern bool mdl_can_do(string& metric_name);

//extern metricDefinitionNode *mdl_observed_cost(vector< vector<string> >& canon_focus,
//					       string& met_name,
//					       string& flat_name,
//					       vector<process *> procs);

// Get the initial mdl info (metrics, constraints, statements)
extern bool mdl_get_initial(string flavor, pdRPC*);

extern bool mdl_get_lib_constraints(vector<string> &);
extern bool mdl_get_func_constraints(vector<string> &);

#endif
