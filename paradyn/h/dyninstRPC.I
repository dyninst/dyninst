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

/* $Id: dyninstRPC.I,v 1.110 2003/07/15 22:45:18 schendel Exp $
 */

#define MetStyleEventCounter	0
#define MetStyleSampledFunction	1

#include "common/h/Types.h"
#include "pdutil/h/mdl.h"

$remote
struct batch_buffer_entry {
   int mid;
   double startTimeStamp;
   double endTimeStamp;
   double value;
$ignore
   batch_buffer_entry &operator=(const batch_buffer_entry &src) {
      mid = src.mid;
      startTimeStamp = src.startTimeStamp;
      endTimeStamp = src.endTimeStamp;
      value = src.value;
      return *this;
   }
$ignore
};

// trace data streams
$remote
struct trace_batch_buffer_entry {
   int mid;
   u_int length;
   byteArray traceRecord;

$ignore
   trace_batch_buffer_entry &operator=(const trace_batch_buffer_entry &src) {
      mid = src.mid;
      length = src.length;
      traceRecord = src.traceRecord;
      return *this;
   }
$ignore
};

/* used by paradynd to inform a change in the status of an application. */
#define procPaused 0
#define procExited 1
typedef u_int processStatus_t;

$remote abstract class mdl_expr {
$ignore
public:
  virtual ~mdl_expr();
  virtual bool apply(mdl_var &ret) = 0;
  virtual bool apply() = 0;
#ifdef NO_MDL_PRINT
  virtual void print(ostream& os) = 0;
#endif
$ignore
};

$remote class mdl_v_expr : $virtual mdl_expr
{
  u_int type_;
  int int_literal_;
  pdstring str_literal_;
  pdstring var_;
  u_int bin_op_;
  u_int u_op_;
  // these two are misnomer in cases other than MDL_RVAL_BINOP. when
  // it's a unary expression or when there's only one arg expression,
  // left_ is used and right_ is set to NULL, but in those cases,
  // there's really no "left" or "right" expressions. --chun
  mdl_expr *left_;
  mdl_expr *right_;
  pdvector<mdl_expr*> *args_;
  pdvector<pdstring> fields_;

  // for fields. set in paradyn's mdl.C and used in paradynd's mdl.C
  // --chun.
  pdvector<u_int> type_walk;
  bool do_type_walk_;

  bool ok_;

$ignore
public:
  mdl_v_expr(int int_literal);
  mdl_v_expr(pdstring a_str, bool is_literal);
  mdl_v_expr(mdl_expr* expr, pdvector<pdstring> fields);
  mdl_v_expr(pdstring func_name, pdvector<mdl_expr*> *args);
  mdl_v_expr(u_int bin_op, mdl_expr *left, mdl_expr *right);
  mdl_v_expr(pdstring var, u_int assign_op, mdl_expr* expr);
  mdl_v_expr(u_int u_op, mdl_expr *expr, bool is_preop);
  mdl_v_expr(pdstring var, mdl_expr *index_expr);
  mdl_v_expr(u_int type,
                int intLiteral,
                pdstring strLiteral,
                pdstring var,
                u_int binOp,
                u_int unOp,
                mdl_expr* leftExpr,
                mdl_expr* rightExpr,
                pdvector<mdl_expr*>* args,
                const pdvector<pdstring>& fields,
                const pdvector<u_int>& type_walk,
                bool useTypeWalk,
                bool isOK );
  ~mdl_v_expr();
  bool apply(mdl_var &ret); // non-code-gen version
  bool apply();                 // code-gen version
#ifdef NO_MDL_PRINT
  virtual void print(ostream& os) 
  {
    os << "MDL_V_EXPR: var=" << var_ << " do_type_walk=" << do_type_walk_;
    unsigned size = fields_.size(), index;
    if (size) 
    {
      os << "Fields: ";
      for (index=0; index<size; index++) 
        os << fields_[index] << " ";
      os << endl;
    }
  }
#endif
  unsigned int GetType( void ) const                { return type_; }
  int GetIntLiteral( void ) const                   { return int_literal_; }
  pdstring GetStringLiteral( void ) const             { return str_literal_; }
  pdstring GetVar( void ) const                       { return var_; }
  unsigned int GetBinaryOp( void ) const            { return bin_op_; }
  unsigned int GetUnaryOp( void ) const             { return u_op_; }
  const mdl_expr* GetLeftExpression( void ) const   { return left_; }
  const mdl_expr* GetRightExpression( void ) const  { return right_; }
  const pdvector<mdl_expr*>* GetArguments( void ) const  { return args_; }
  const pdvector<pdstring>& GetFields( void ) const   { return fields_; }
  const pdvector<unsigned int>& GetTypeWalks( void ) const { return type_walk;
  }
  bool UseTypeWalk( void ) const                    { return do_type_walk_; } 
   
  bool IsOK( void ) const                           { return ok_; }

$ignore
};

$remote class mdl_icode {
  mdl_expr *if_expr_;
  mdl_expr *expr_;

$ignore
public:
  mdl_icode(mdl_expr *if_expr, mdl_expr *expr);
  ~mdl_icode();
  bool apply();

  const mdl_expr* GetIfExpression( void ) const             { return if_expr_; }
  const mdl_expr* GetExpression( void ) const               { return expr_; }
$ignore
};

$remote abstract class mdl_stmt {
$ignore
public:
  virtual ~mdl_stmt() { }
  virtual bool apply(pdvector<const instrDataNode*>& flags) = 0;
#ifdef NO_MDL_PRINT
  virtual void print(ostream& os) = 0;
#endif
$ignore
};

$remote class mdl_list_stmt : $virtual mdl_stmt {
  u_int type_;
  pdstring id_;
  pdvector<pdstring> *elements_;
  bool is_lib_;
  pdvector<pdstring> *flavor_;

$ignore
public:
  mdl_list_stmt(u_int type, pdstring ident, pdvector<pdstring> *elems,
		bool is_lib, pdvector<pdstring> *flavor);
  virtual ~mdl_list_stmt();
  virtual bool apply(pdvector<const instrDataNode*>& flags);
#ifdef NO_MDL_PRINT
  virtual void print(ostream&) { }
#endif

  u_int GetType( void ) const                           { return type_; }
  pdstring GetId( void ) const                            { return id_; }
  const pdvector<pdstring>* GetElements( void ) const     { return elements_; }
  bool IsLib( void ) const                              { return is_lib_; }
  const pdvector<pdstring>* GetFlavors( void ) const      { return flavor_; }

$ignore
};

$remote class mdl_for_stmt : $virtual mdl_stmt {
  mdl_stmt *for_body_;
  pdstring index_name_;
  mdl_expr *list_expr_;

$ignore
public:
  mdl_for_stmt(pdstring index_name, mdl_expr *list_exp, mdl_stmt *body);
  virtual ~mdl_for_stmt();
  virtual bool apply(pdvector<const instrDataNode*>& flags);
#ifdef NO_MDL_PRINT
  virtual void print(ostream&) { }
#endif

  const mdl_stmt* GetBodyStatement( void ) const    { return for_body_; }
  pdstring GetIndexName( void ) const                 { return index_name_; }
  const mdl_expr* GetExpression( void ) const       { return list_expr_; }

$ignore
};

$remote class mdl_if_stmt : $virtual mdl_stmt {
  mdl_expr *expr_;
  mdl_stmt *body_;

$ignore
public:
  mdl_if_stmt(mdl_expr *expr, mdl_stmt *body);
  virtual ~mdl_if_stmt();
  virtual bool apply(pdvector<const instrDataNode*>& flags);
#ifdef NO_MDL_PRINT
  virtual void print(ostream&) { }
#endif

  const mdl_expr* GetExpression( void ) const       { return expr_; }
  const mdl_stmt* GetBodyStatement( void ) const    { return body_; }

$ignore
};

$remote class mdl_seq_stmt : $virtual mdl_stmt {
  pdvector<mdl_stmt*> *stmts_;

$ignore
public:
  mdl_seq_stmt(pdvector<mdl_stmt*> *stmts);
  virtual ~mdl_seq_stmt();
  virtual bool apply(pdvector<const instrDataNode*>& flags);
#ifdef NO_MDL_PRINT
  virtual void print(ostream&) { }
#endif

  const pdvector<mdl_stmt*>* GetStatements( void ) const  { return stmts_; }

$ignore
};

$remote class mdl_instr_stmt : $virtual mdl_stmt {
  u_int position_; // append or prepend
  mdl_expr *point_expr_; // e.g. $start.entry.  Can be a fn entry, exit, or call site.
  pdvector<mdl_icode*> *icode_reqs_; // the instrumentation code itself
  u_int where_instr_; // preInsn or postInsn
  bool constrained_; // if not true, no constraints will be applied

$ignore
public:
  mdl_instr_stmt(u_int pos, mdl_expr *point_expr, pdvector<mdl_icode*> *i_reqs,
		 unsigned where_instr, bool constrained);
  virtual ~mdl_instr_stmt();
  virtual bool apply(pdvector<const instrDataNode*>& flags);
#ifdef NO_MDL_PRINT
  virtual void print(ostream& os) {
    os << "MDL_INSTR_STMT: constrained=" << constrained_ << " ";
    point_expr_->print(os);
  }
#endif

  unsigned int GetPosition( void ) const                { return position_; }
  const mdl_expr* GetExpression( void ) const           { return point_expr_; }

  const pdvector<mdl_icode*>* GetInstrumentation( void ) const  
                                                        { return icode_reqs_; }
  unsigned int GetInstrLoc( void ) const                { return where_instr_; }
  bool IsConstrained( void ) const                      { return constrained_; }

$ignore
};

$remote class mdl_constraint {
  pdstring id_;
  pdvector<pdstring> *match_path_;
  pdvector<mdl_stmt*> *stmts_;
  bool replace_;
  u_int data_type_; // type of this constraint: counter, timer, etc.
  u_int hierarchy_;
  u_int type_; // type of things constrained: module, procedure, int, etc.

$ignore
public:
  mdl_constraint(pdstring id, pdvector<pdstring> *match_path,
		 pdvector<mdl_stmt*> *stmts, bool replace, u_int d_type,
		 bool& error);
  mdl_constraint(pdstring id, pdvector<pdstring> *match_path,
		 pdvector<mdl_stmt*> *stmts, bool replace, u_int d_type,
		 u_int hierarchy, u_int type);
  ~mdl_constraint();
  bool apply();
  bool replace();
  pdstring id();

#ifdef NO_MDL_PRINT
   friend ostream& operator<< (ostream& os, const mdl_constraint& cons) {
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

  pdstring GetId( void ) const                            { return id_; }
  const pdvector<pdstring>* GetMatchPath( void ) const    { return match_path_; }
  const pdvector<mdl_stmt*>* GetStatements( void ) const    { return stmts_; }
  bool IsReplacing( void ) const                            { return replace_; }
  u_int GetDataType( void ) const                       { return data_type_; }
  u_int GetHierarchy( void ) const                      { return hierarchy_; }
  u_int GetType( void ) const                           { return type_; }

$ignore
};

$remote class mdl_metric {
  pdstring id_;
  pdstring name_;
  pdstring units_;
  u_int agg_op_; // avg, sum, min, or max
  u_int style_;  // currently, always EventCounter
  u_int type_;   // counter, processTimer, or wallTimer
  pdstring hwcntr_;  // string denoting hw counter event
  pdvector<mdl_stmt*> *stmts_; // for "base is <type_> { ... }
  pdvector<pdstring> *flavors_; // which flavors this metric is valid for
  pdvector<mdl_constraint*> *constraints_;
  pdvector<pdstring> *temp_ctr_; // temporary counters
  bool developerMode_;
  int unitstype_; // normalized, unnormalized, or sampled

$ignore
public:
  mdl_metric(pdstring id, pdstring name, pdstring units,
	     u_int agg, u_int style, u_int type, pdstring hwcntr, 
	     pdvector<mdl_stmt*> *mv, pdvector<pdstring> *flavs, 
	     pdvector<mdl_constraint*> *cons,
	     pdvector<pdstring> *temp_c, bool developerMode,int unitstype);
  ~mdl_metric();
#ifdef NO_MDL_PRINT
  friend ostream& operator<< (ostream &os, const mdl_metric& met) {
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

  bool apply();

  pdstring GetId( void ) const                    { return id_; }
  pdstring GetName( void ) const                  { return name_; }
  pdstring GetUnits( void ) const                 { return units_; }
  u_int GetAggOp( void ) const                  { return agg_op_; }
  u_int GetStyle( void ) const                  { return style_; }
  u_int GetType( void ) const                   { return type_; }
  pdstring GetHWCounterEvent( void ) const        { return hwcntr_; }
  const pdvector<mdl_stmt*>* GetStatements( void ) const    { return stmts_; }
  const pdvector<pdstring>* GetFlavors( void ) const          { return flavors_; }
  const pdvector<mdl_constraint*>* GetConstraints( void ) const 
                                                { return constraints_; }
  const pdvector<pdstring>* GetTempCounters( void ) const     { return temp_ctr_;}
  bool IsDeveloperMode( void ) const            { return developerMode_; }
  int GetUnitsType( void ) const                { return unitstype_; }

$ignore
};

/* descriptive information about a metric */

$remote struct metricInfo { 
    pdstring name;
    int style;
    int aggregate;
    pdstring units;
    bool developerMode;
    int unitstype;
    u_int handle;  // this is currently ignored by the daemons 
};

$remote struct focusStruct {
  pdvector<u_int> focus;
};

$remote struct resourceInfoCallbackStruct {
   u_int temporaryId;
   pdvector<pdstring> resource_name;
   pdstring abstraction;
   u_int type;
};


typedef
enum
{
    inst_insert_unknown,
    inst_insert_success,
    inst_insert_failure,
    inst_insert_deferred
} inst_insert_result_t;


$remote struct indivInstResponse
{
    u_int mi_id;
    u_int status;       // one of the inst_insert_result_t values
    pdstring emsg;

$ignore
    indivInstResponse( u_int mid = 0,
                        inst_insert_result_t _status = inst_insert_failure,
                        pdstring msg = "" )
      : mi_id( mid ),
        status( _status ),
        emsg( msg )
    { }
$ignore
};


$remote struct instResponse
{
    u_int request_id;
    u_int daemon_id;
    pdvector<indivInstResponse> rinfo;

$ignore
    instResponse( u_int reqId = 0, u_int did = 0 )
      : request_id( reqId ),
      daemon_id( did )
    { }
$ignore
};


$remote struct rawMDL
{
    pdstring buf;

$ignore
    rawMDL( pdstring _buf = "" )
      : buf( _buf )
    { }
$ignore
};


/*
 * Create a paradynd process.
 *
 */
int createDaemon(pdstring machine,
		 pdstring login,
		 pdstring name,
		 pdvector<pdstring> argv);


$remote dynRPC {
    $base 4000;
    $version 62;

    //
    // Define a program to run. 
    // argv: the command and arguments
    // dir: working directory where program will run.
    int addExecutable(pdvector<pdstring> argv, pdstring dir);

    //
    // Connect up with a running program (not possible on all platforms).
    // 'path' gives the full path name to the executable, used only to read
    // the symbol table.
    // values for 'afterAttach': 1 --> pause, 2 --> run, 0 --> leave as is
    //
    bool attach(pdstring path, int pid, int afterAttach);

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
    // Monitor the dynamic call sites contained in function <function_name>
    //		
    void MonitorDynamicCallSites(pdstring function_name);

    //
    // Called by paradynd when a new process starts up.  Specifically, paradynd
    // calls it when it detects that the new program has finished running
    // its DYNINST().
    //
    // If the state of the appl as a whole is 'running' then we (paradyn) will
    // issue an igen call to continue this new process; otherwise, we'll leave it
    // paused.
    //
    $upcall $async void newProgramCallbackFunc( int pid, 
						pdvector<pdstring> argv, 
						pdstring machine,
	                                        bool calledFromExec,
						bool wantToInitiallyRun);
    //
    // Message display with "error" visual characteristics
    // and lookup in error database for explanation. This function
    // calls showError function in UIM class
    //
    $upcall $async void showErrorCallback (int errCode,
					   pdstring errString,
				           pdstring hostName);

   // batch the samples (paradynd-->paradyn) to get better performance
   // the functionality is essentially the same as the 
   // sampleDataCallbackFunc, which is now obsolete

   $virtual $upcall $async
   void batchSampleDataCallbackFunc(int program,
	                            pdvector<batch_buffer_entry> theBatchBuffer);

   // batch the traces (paradynd-->paradyn) to get better performance
   // the functionality is essentially the same as the 
   // batchSampleDataCallbackFunc, which is now obsolete

   $virtual $upcall $async
   void batchTraceDataCallbackFunc(int program,
                                    pdvector<trace_batch_buffer_entry> theTraceBatchBuffer);

   //
   // XXXX - Hack this should be generalized into a vector.
   //
   $virtual $upcall $async void cpDataCallbackFunc(int program,
                                                      double timeStamp,
                                                      int context,
                                                      double total,
                                                      double share);

   // 
   // Handler that reports the initial time that the daemon started the
   // the first process.  Called after DYNINSTinit completed.
   // The earliest (absolute) initial time will become the base time (0)
   //
   $virtual $upcall $async void setDaemonStartTime(int program,
                                                   double startTime);

   // 
   // Handler that reports the initial actual value the metric with id mid.
   // Used to reconstruct the actual value of mid at any point.  Done
   // by integrating across the change in sample values and then adding
   // the initial actual value.
   //
   $virtual $upcall $async void setInitialActualValueFE(int mid,
                                                        double initActualVal);


   //
   // Handler that gets called when a new resource is defined.
   //
   // temporaryId      - daemon generated resource id (it will become the 
   //				permanent id, unless there are conflicts
   //				with id's generated by another daemon)
   // resource_name    - name of resource, divided into parts
   // abstraction      - abstraction to which it belongs     
   // type		- the MDL type for this resource (MDL_T_STRING, etc.)
   // 
   $virtual $upcall $async void resourceInfoCallback(u_int temporaryId, 
					     pdvector<pdstring> resource_name,
					     pdstring abstraction, u_int type);

    $virtual $upcall $async void severalResourceInfoCallback(pdvector<resourceInfoCallbackStruct>);
	
   //
   // Handlers that gets called in paradynd when a new call graph relationship
   //  is defined....
   // Names post-fixed w/ Callback to preserve convention that functions
   //  names *Callback represent callbacks from paradynd TO paradyn....
   //

   //Notify the DM that a new executable file is being parsed, so it
   //will be ready for the call graph relationships for this graph.
   $virtual $upcall $async void CallGraphAddProgramCallback(pdstring exe_name);

   //Add resource to the call graph specified by "exe_name", 
   $virtual $upcall $async void AddCallGraphNodeCallback(pdstring exe_name, 
                                                          pdstring resource);

   //Specify the children of a resource for CallGraph "exe_name"
   $virtual $upcall $async void AddCallGraphStaticChildrenCallback(
                                                      pdstring exe_name, 
                                                      pdstring resource, 
	                                              pdvector<pdstring> children);
   //Specify the root node if CallGraph "exe_name"
   $virtual $upcall $async void CallGraphSetEntryFuncCallback(pdstring exe_name,
                                                            pdstring entry_func,
							      int tid);
    
   //Notify the DM that all static info for a given call graph has been
   //reported.
   $virtual $upcall $async void CallGraphFillDone(pdstring exe_name);	

   //Notify the call graph that function <parent> contains one or more
   //dynamic call sites.
   $virtual $upcall $async void CallGraphAddDynamicCallSiteCallback(pdstring exe_name, 
                                                             pdstring parent);

   //Notify the call graoh that function <parent> calls function <child>
   //through a dynamic call site.
   $virtual $upcall $async void AddCallGraphDynamicChildCallback(
                                  pdstring exe_name,pdstring parent, pdstring child);


   // In case there are conflicts between the resource Id's generated by
   // the daemons, paradyn generates a new Id and report it to the daemon.
   $async void resourceInfoResponse(pdvector<u_int> temporaryIds,
				    pdvector<u_int>resourceIds);


   $virtual $upcall $async void resourceBatchMode(bool);

   // Inform front-end about retired resources.
   $virtual $upcall $async void retiredResource(pdstring res);

   //
   // Handler that gets called when new mapping data is collected.
   //
   $upcall $async void mappingInfoCallback(int program, 
					   pdstring abstraction, 
					   pdstring type, 
					   pdstring key,
					   pdstring value);

   // Upcall for a new metric being defined.
   //
   $upcall $async void newMetricCallback(metricInfo info); 

   $async void setSampleRate(double sampleInterval);

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
   $async void enableDataCollection(pdvector<focusStruct> focus, 
                                    pdvector<pdstring> metric,
				    pdvector<u_int> mi_ids, 
				    u_int daemon_id,
				    u_int request_id);
   //
   // synchronous enable call, for propogating metrics
   //
   // Note: we'd like to just send back a single instResponse, but
   // currently igen does not support sending back dynamically allocated
   // structs as such.
   instResponse enableDataCollection2(pdvector<u_int> focus,
                                        pdstring metric,
                                        int id,
                                        u_int daemon_id );

   //
   // Upcall to tell paradyn that all daemons are ready after a metric 
   // enable request
   //
   $upcall $async void enableDataCallback( instResponse resp );
					                      

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
				    pdvector<u_int> focus, 
				    pdstring metric,
				    u_int clientID);

   //
   // Get the static configuration information.
   //
   pdvector<metricInfo> getAvailableMetrics();

   //
   // Create a new resource.
   //
   void addResource(u_int parent, u_int id, pdstring name, u_int type);

   // should be removed for output redirection
   // left untouched for paradynd log info use
   //
   // stdio or stderr from an application.
   //
   $upcall $async void applicationIO(int pid, int len, pdstring line);

   //
   // stdio or stderr from an application.
   //
   $virtual $upcall $async void reportStatus(pdstring line);

   //
   // Debugger style calls.
   //
   //
   pdstring getStatus(int pid);
   void coreProcess(int pid);

   //
   // For paradynd's that may get started, but not by paradyn
   // This allows them to report system information
   //
   $virtual $upcall $async void reportSelf(pdstring machine, 
					   pdstring program, 
					   int pid,
					   pdstring flavor);

    //
    // The front end is done handling this daemon's reportSelf call.
    // A daemon should put off doing anything that will cause it
    // to ignore its event loop for an extended time until it sees
    // this message from the front end, so that the front-end
    // doesn't get delayed waiting for responses to its synchronous calls
    // in its reportSelf handler.
    //
    // Note: only used when reportSelf is used (i.e., only when 
    // some external job launcher like mpirun starts our daemons for us)
    //
    $virtual $async void reportSelfDone();


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
   $virtual $async void send_mdl( pdvector<rawMDL> );

   // get the current wall clock time of the daemon to adjust for clock differences
   double getTime();
};
