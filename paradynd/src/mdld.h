
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
				    string& metric_name, string& flat_name);
extern bool mdl_can_do(string& metric_name);

extern metricDefinitionNode *mdl_observed_cost(vector< vector<string> >& canon_focus,
					       string& met_name,
					       string& flat_name);

// Get the initial mdl info (metrics, constraints, statements)
extern bool mdl_get_initial(string flavor, pdRPC*);

#endif
