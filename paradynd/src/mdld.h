
/*
 * $Log: mdld.h,v $
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
extern bool mdl_metric_data(string&, mdl_inst_data&);

extern metricDefinitionNode *mdl_do(vector< vector<string> >& canon_focus,
				    string& metric_name, string& flat_name,
				    vector<process *> procs);
extern bool mdl_can_do(string& metric_name);

extern metricDefinitionNode *mdl_observed_cost(vector< vector<string> >& canon_focus,
					       string& met_name,
					       string& flat_name,
					       vector<process *> procs);

// Get the initial mdl info (metrics, constraints, statements)
extern bool mdl_get_initial(string flavor, pdRPC*);

#endif
