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


#define MetStyleEventCounter	0
#define MetStyleSampledFunction	1

#include "util/h/Types.h"
#include "util/h/aggregation.h"
#include "paradyn/src/met/mdl.h"

$remote
struct batch_buffer_entry {
   int mid;
   double startTimeStamp;
   double endTimeStamp;
   double value;
   u_int weight;
   bool  internal_met; 

$ignore
   batch_buffer_entry &operator=(const batch_buffer_entry &src) {
      mid = src.mid;
      startTimeStamp = src.startTimeStamp;
      endTimeStamp = src.endTimeStamp;
      value = src.value;
      weight = src.weight;
      internal_met = src.internal_met;
      return *this;
   }
$ignore
};


/* used by paradynd to inform a change in the status of an application. */
// typedef enum { procPaused, procExited } processStatus_t;
#define procPaused 0
#define procExited 1
typedef u_int processStatus_t;

$remote
abstract class mdl_expr {
$ignore
public:
  virtual ~mdl_expr();
  virtual bool apply(mdl_var &ret) = 0;
#ifdef NO_MDL_PRINT
  virtual void print(ostream& os) = 0;
#endif
$ignore
};

$remote
class mdl_v_expr : mdl_expr {
  string var_;
  vector<string> fields_;
  vector<mdl_expr*> *args_;
  int literal_;
  u_int arg_;
  mdl_expr *left_;
  mdl_expr *right_;
  u_int type_;
  vector<u_int> type_walk;
  bool do_type_walk_;
  bool ok_;
  u_int res_type_;

$ignore
public:
  mdl_v_expr(string var, vector<string> fields);
  mdl_v_expr(string func_name, vector<T_dyninstRPC::mdl_expr*> *args);
  mdl_v_expr(int int_literal);
  mdl_v_expr(string string_lit);
  mdl_v_expr(u_int bin_op, T_dyninstRPC::mdl_expr *left, T_dyninstRPC::mdl_expr *right);
  mdl_v_expr(string var, u_int array_index);
  ~mdl_v_expr();
  bool apply(mdl_var &ret);
#ifdef NO_MDL_PRINT
  virtual void print(ostream& os) {
    os << "MDL_V_EXPR: var=" << var_ << " res_type=" << res_type_ << " do_type_walk="
      << do_type_walk_;
    unsigned size = fields_.size(), index;
    if (size) {
      os << "Fields: ";
      for (index=0; index<size; index++) {
	os << fields_[index] << " ";
      }
      os << endl;
    }
  }
#endif

$ignore
};

$remote
abstract class mdl_rand {
$ignore
public:
  virtual ~mdl_rand() {}
  virtual bool apply(AstNode *&) = 0;
$ignore
};

$remote
class mdl_instr_rand: mdl_rand {
  u_int type_;
  u_int val_;
  string name_;
  vector<T_dyninstRPC::mdl_rand *> args_;
$ignore
public:
  mdl_instr_rand(u_int type);
  mdl_instr_rand(u_int type, u_int val);
  mdl_instr_rand(u_int type, string name);
  mdl_instr_rand(u_int type, string name, vector<T_dyninstRPC::mdl_instr_rand *>args);
  ~mdl_instr_rand();
  virtual bool apply(AstNode *&);
$ignore
};

$remote
class mdl_instr_req {
  u_int type_;
  T_dyninstRPC::mdl_instr_rand *rand_;
  string timer_counter_name_;

$ignore
public:
  // start and stop timers, read symbol
  mdl_instr_req(u_int type, string name);

  // for counters
  mdl_instr_req(T_dyninstRPC::mdl_instr_rand *arg, u_int type, string obj_name);

  // for calls
  mdl_instr_req(u_int type, T_dyninstRPC::mdl_instr_rand *rand);

 ~mdl_instr_req();

   bool apply(AstNode *&mn, AstNode *pred, bool mn_initialized);
$ignore
};

$remote
class mdl_icode {
  T_dyninstRPC::mdl_instr_rand *if_op1_;
  T_dyninstRPC::mdl_instr_rand *if_op2_;
  u_int bin_op_;
  bool use_if_;
  mdl_instr_req *req_;

$ignore
public:
  mdl_icode(T_dyninstRPC::mdl_instr_rand *iop1, T_dyninstRPC::mdl_instr_rand *iop2,
	    u_int bop, bool use, T_dyninstRPC::mdl_instr_req *r);
  ~mdl_icode();
   bool apply(AstNode *&mn, bool mn_initialized);
$ignore
};

$remote
abstract class mdl_stmt {
$ignore
public:
  virtual ~mdl_stmt() { }
  virtual bool apply(metricDefinitionNode *mn, vector<dataReqNode*>& flags) = 0;
#ifdef NO_MDL_PRINT
  virtual void print(ostream& os) = 0;
#endif
$ignore
};

$remote
class mdl_list_stmt : mdl_stmt {
  u_int type_;
  string id_;
  vector<string> *elements_;
  bool is_lib_;
  vector<string> *flavor_;

$ignore
public:
  mdl_list_stmt(u_int type, string ident, vector<string> *elems,
		bool is_lib, vector<string> *flavor);
  virtual ~mdl_list_stmt();
  virtual bool apply(metricDefinitionNode *mn, vector<dataReqNode*>& flags);
#ifdef NO_MDL_PRINT
  virtual void print(ostream&) { }
#endif

$ignore
};

$remote
class mdl_for_stmt : mdl_stmt {
  mdl_stmt *for_body_;
  string index_name_;
  mdl_expr *list_expr_;

$ignore
public:
  mdl_for_stmt(string index_name, T_dyninstRPC::mdl_expr *list_exp, mdl_stmt *body);
  virtual ~mdl_for_stmt();
  virtual bool apply(metricDefinitionNode *mn, vector<dataReqNode*>& flags);
#ifdef NO_MDL_PRINT
  virtual void print(ostream&) { }
#endif

$ignore
};

$remote
class mdl_if_stmt : mdl_stmt {
  mdl_expr *expr_;
  mdl_stmt *body_;

$ignore
public:
  mdl_if_stmt(mdl_expr *expr, mdl_stmt *body);
  virtual ~mdl_if_stmt();
  virtual bool apply(metricDefinitionNode *mn, vector<dataReqNode*>& flags);
#ifdef NO_MDL_PRINT
  virtual void print(ostream&) { }
#endif

$ignore
};

$remote
class mdl_seq_stmt : mdl_stmt {
  vector<mdl_stmt*> *stmts_;

$ignore
public:
  mdl_seq_stmt(vector<mdl_stmt*> *stmts);
  virtual ~mdl_seq_stmt();
  virtual bool apply(metricDefinitionNode *mn, vector<dataReqNode*>& flags);
#ifdef NO_MDL_PRINT
  virtual void print(ostream&) { }
#endif

$ignore
};

$remote
class mdl_instr_stmt : mdl_stmt {
  u_int position_; // append or prepend
  mdl_expr *point_expr_; // e.g. $start.entry.  Can be a fn entry, exit, or call site.
  vector<mdl_icode*> *icode_reqs_; // the instrumentation code itself
  u_int where_instr_; // preInsn or postInsn
  bool constrained_; // if not true, no constraints will be applied

$ignore
public:
  mdl_instr_stmt(u_int pos, mdl_expr *point_expr, vector<mdl_icode*> *i_reqs,
		 unsigned where_instr, bool constrained);
  virtual ~mdl_instr_stmt();
  virtual bool apply(metricDefinitionNode *mn, vector<dataReqNode*>& flags);
#ifdef NO_MDL_PRINT
  virtual void print(ostream& os) {
    os << "MDL_INSTR_STMT: constrained=" << constrained_ << " ";
    point_expr_->print(os);
  }
#endif

$ignore
};

$remote 
class mdl_constraint {
  string id_;
  vector<string> *match_path_;
  vector<mdl_stmt*> *stmts_;
  bool replace_;
  u_int data_type_;
  u_int hierarchy_;
  u_int type_;

$ignore
public:
  mdl_constraint(string id, vector<string> *match_path,
		 vector<T_dyninstRPC::mdl_stmt*> *stmts, bool replace, u_int d_type,
		 bool& error);
  ~mdl_constraint();
  bool apply(metricDefinitionNode *mn, dataReqNode *&flag,
	     vector<string>& resource, process *proc);

#ifdef NO_MDL_PRINT
   friend ostream& operator<< (ostream& os, const T_dyninstRPC::mdl_constraint& cons) {
     os << "\nConstraint: " << cons.id_ << " ";
     unsigned size, index;
     if (cons.match_path_) {
       size = cons.match_path_->size();
       for (index=0; index<size; index++) os << (*cons.match_path_)[index] << "/";
       os << endl;
     }
     os << "Replace=" << cons.replace_ << " Data_type=" << cons.data_type_ << " Hierarchy=" 
       << cons.hierarchy_ << " Type=" << cons.type_ << endl;
    if (cons.stmts_) {
      size = cons.stmts_->size();
      os << "Statements\n";
      for (index=0; index<size; index++) {
	(*cons.stmts_)[index]->print(os);
	os << " ";
      }
    }
    return os;
   }
#endif

$ignore
};

$remote
class mdl_metric {
  string id_;
  string name_;
  string units_;
  u_int agg_op_; // avg, sum, min, or max
  u_int style_;  // currently, always EventCounter
  u_int type_;   // counter, processTimer, or wallTimer
  vector<mdl_stmt*> *stmts_; // for "base is <type_> { ... } (is this correct?)
  vector<string> *flavors_; // which flavors this metric is valid for
  vector<mdl_constraint*> *constraints_;
  vector<string> *temp_ctr_; // temporary counters
  bool developerMode_;
  int unitstype_; // normalized, unnormalized, or sampled

$ignore
public:
  mdl_metric(string id, string name, string units,
	     u_int agg, u_int style, u_int type,
	     vector<mdl_stmt*> *mv, vector<string> *flavs, 
	     vector<T_dyninstRPC::mdl_constraint*> *cons,
	     vector<string> *temp_c, bool developerMode,int unitstype);
  ~mdl_metric();
#ifdef NO_MDL_PRINT
  friend ostream& operator<< (ostream &os, const T_dyninstRPC::mdl_metric& met) {
    os << "\nMetric: " << met.id_ << " " << met.name_ << " agg_op=" << met.agg_op_ << " style=" 
      << met.style_ << " type=" << met.type_ << endl;
    unsigned size, index;
    if (met.stmts_) {
      size = met.stmts_->size();
      os << "Statements\n";
      for (index=0; index<size; index++) {
	(*met.stmts_)[index]->print(os);
	 os << " ";
      }
    }
    if (met.constraints_) {
      size = met.constraints_->size();
      os << "Constraints\n";
      for (index=0; index<size; index++) {
	os << *((*met.constraints_)[index]) << " ";
      }
    }
    return os;
  }
#endif
  metricDefinitionNode *apply(vector< vector<string> >&focus, string& flat_name,
	                      vector<process *> procs,
	                      bool replace_components_if_present);
$ignore
};

/* descriptive information about a metric */
$remote struct metricInfo { 
    string name;
    int style;
    int aggregate;
    string units;
    bool developerMode;
    int unitstype;
    u_int handle;  // this is currently ignored by the daemons 
};

$remote struct focusStruct {
  vector<u_int> focus;
};

/*
 * Create a paradynd process.
 *
 */
int createDaemon(string machine,
		 string login,
		 string name,
		 vector<string> argv);


$remote dynRPC {
    $base 4000;
    $version 48;

    //
    // Define a program to run. 
    // argv: the command and arguments
    // dir: working directory where program will run.
    int addExecutable(vector<string> argv, string dir);

    //
    // Connect up with a running program (not possible on all platforms).
    //
    bool attach(int pid);

    //
    // Start an application running (This starts the actual execution).
    //
    bool startProgram(int program);

    //
    //   Stop a process
    //
    bool pauseProgram(int program);

    //
    //   Stop all processes associted with the application.
    //
    bool pauseApplication();

    //
    // Continue a paused process
    //
    void continueProgram(int program);

    //
    // Continue all of the processes
    //
    void continueApplication();

    //
    // Disconnect the tool from the process.
    //    pause - leave the process in a stopped state.
    //
    //
    bool detachProgram(int program, bool pause);

    //
    // Gets called when a new application is started that is not visible to
    //   paradyn (e.g. pvm_spawn, or when a process forks, or when a process execs.
    //   In the latter case, pass calledFromExec=true to let paradyn know that
    //   in starting this new process, another one was stomped on).
    //
    $upcall $async void newProgramCallbackFunc( int pid, 
						vector<string> argv, 
						string machine,
	                                        bool calledFromExec);
    //
    // Message display with "error" visual characteristics
    // and lookup in error database for explanation. This function
    // calls showError function in UIM class
    //
    $upcall $async void showErrorCallback (int errCode,
					   string errString,
				           string hostName);

    //
    // Handler that gets called when new sample data is delivered.
    //
    //   p - a program from addProgram
    //   mi - a metricInstance returned by enableDataCollection
    //   startTimeStamp - starting time of the interval covered by this sample.
    //   endTimeStamp - ending time of the interval covered by this sample.
    //   value - the value of this sample
    //
//   $virtual $upcall $async void sampleDataCallbackFunc(int program, 
//						       int mi,
//						       double startTimeStamp, 
//						       double endTimeStamp, 
//						       double value);

   // batch the samples (paradynd-->paradyn) to get better performance
   // the functionality is essentially the same as the 
   // sampleDataCallbackFunc, which is now obsolete

   $virtual $upcall $async
   void batchSampleDataCallbackFunc(int program,
	                            vector<batch_buffer_entry> theBatchBuffer);

   //
   // XXXX - Hack this should be generalized into a vector.
   //
   $virtual $upcall $async void cpDataCallbackFunc(int program,
                                                      double timeStamp,
                                                      int context,
                                                      double total,
                                                      double share);

   // 
   // Handler that reports the initial time for which a daemon reports a sample
   // The earliest (absolute) initial time will become the base time (0)
   //
   $virtual $upcall $async void firstSampleCallback(int program,
                                                    double time);

    //
    // Handler that gets called when a new resource is defined.
    //
    // resource_name    - name of resource, divided into parts
    // abstraction      - abstraction to which it belongs     
    // type		- the MDL type for this resource (MDL_T_STRING, etc.)
    $virtual $upcall $async void resourceInfoCallback(int program, 
					     vector<string> resource_name,
					     string abstraction, u_int type);

    // The id that will be used to represent this resource
    $async void resourceInfoResponse(vector<string> resource_name, u_int resource_id);

    $virtual $upcall $async void resourceBatchMode(bool);

    //
    // Handler that gets called when new mapping data is collected.
    //
    $upcall $async void mappingInfoCallback(int program, 
					    string abstraction, 
					    string type, 
					    string key,
					    string value);

    // Upcall for a new metric being defined.
    //
    $upcall $async void newMetricCallback(metricInfo info); 

    void setSampleRate(double sampleInterval);

    //
    // tell dyninst to ignore this resource for instrumentation.
    //    Currently this only applies to programs, modules, and procedures.
    //    False - suppress
    //    True - enable
    //
    bool setTracking(u_int resource_id, bool);

    //
    // Routines to control data collection on a performanceStream.
    //
    // resourceList	- a list of resources
    // metric	- what metric to collect data for
    //
    $async void enableDataCollection(vector<focusStruct> focus, 
                                      vector<string> metric,
				      vector<u_int> mi_ids, 
				      u_int daemon_id,
				      u_int request_id);
    //
    // synchronous enable call, for propogating metrics
    //
    int enableDataCollection2(vector<u_int> focus, string metric, int id);

    //
    // Upcall to tell paradyn that all daemons are ready after a metric 
    // enable request
    //
    $upcall $async void enableDataCallback(u_int daemon_id, 
                                           vector<int> return_id,
                                           vector<u_int> mi_ids,
					   u_int request_id);
    //
    // Upcall to tell paradyn that all daemons are ready after computing 
    // the value for predicted data cost
    //    id: an identifier for the request
    //  client_id: an id passed by calling thread
    //
    $upcall $async void getPredictedDataCostCallback(u_int id,
						     u_int req_id,
						     float val,
						     u_int clientID);

    //
    // stop collecting data for the named mi.
    //     mi		- a metricInstance returned by enableDataCollection.
    //
    $async void disableDataCollection(int mi);

    // 
    // called by a daemon when there is no more data to be sent for a metric
    // instance, because all processes have exited.
    $virtual $upcall $async void endOfDataCollection(int mi);

    //
    // Return the expected cost of collecting performance data for a single
    //    metric at a given focus.  The value returned is the fraction of
    //    perturbation expected (i.e. 0.10 == 10% slow down expected).
    //    id: an identifier for the request
    //  client_id: an id passed by calling thread
    //
    $async void getPredictedDataCost(u_int id,
				     u_int req_id,
				     vector<u_int> focus, 
				     string metric,
				     u_int clientID);

    //
    // Get the static configuration information.
    //
    vector<metricInfo> getAvailableMetrics();

    //
    // Create a new resource.
    //
    void addResource(u_int parent, u_int id, string name, u_int type);

    //
    // stdio or stderr from an application.
    //
    $upcall $async void applicationIO(int pid, int len, string line);

    //
    // stdio or stderr from an application.
    //
    $virtual $upcall $async void reportStatus(string line);

    //
    // Debugger style calls.
    //
    //
    string getStatus(int pid);
    void coreProcess(int pid);

    //
    // For paradynd's that may get started, but not by paradyn
    // This allows them to report system information
    // flavor = programming environment {"pvm", "cm5", ... }
    $virtual $upcall $async void reportSelf(string machine, 
					    string program, 
					    int pid,
					    string flavor);

    //
    // this call is used by paradynd to inform paradyn of a change in
    // the status of a process -- the process stops (due to a signal), 
    // or the process exits
    //
    $virtual $upcall $async void processStatus(int pid, u_int procstat);

    //
    // For timing and other measurements of paradynd
    //
    void printStats();

    // MDL methods
    $async void send_metrics(vector<mdl_metric*> *);
    $async void send_constraints(vector<mdl_constraint*>*);
    $async void send_stmts(vector<mdl_stmt*> *);
    $async void send_libs(vector<string>*);
    $async void send_no_libs();

    // get the current wall clock time of the daemon to adjust for clock differences
    double getTime();

};
