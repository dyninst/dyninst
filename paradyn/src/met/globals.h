
#ifndef _MDL_GLOBALS_H
#define _MDL_GLOBALS_H

#include "dyninstRPC.xdr.h"

class mdl_data {
public:
  static dictionary_hash<unsigned, vector<mdl_type_desc> > fields;
  static vector<mdl_focus_element> foci;
  static vector<T_dyninstRPC::mdl_stmt*> stmts;
  static vector<T_dyninstRPC::mdl_constraint*> all_constraints;

  static T_dyninstRPC::mdl_constraint *new_constraint(string id, vector<string> *path,
					       vector<T_dyninstRPC::mdl_stmt*> *stmts,
					       bool replace, u_int data_type);

  static vector<T_dyninstRPC::mdl_metric*> all_metrics;
  static bool new_metric(string id, string name, string units,
			 u_int agg, u_int style, u_int type,
			 vector<T_dyninstRPC::mdl_stmt*> *stmts, 
			 vector<string> *flavs,
			 vector<T_dyninstRPC::mdl_constraint*> *cons,
			 vector<string> *temp_counters,
			 bool developerMode,
			 int normalized);


  // unique_name: prepares for the declaration of a new mdl object,
  // by deleting all objects previously declared that have the same
  // name.
  static void unique_name(string name);


};


#endif
