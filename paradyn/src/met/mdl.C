
#include "dyninstRPC.xdr.CLNT.h"
#include "paradyn/src/met/globals.h"
#include "paradyn/src/met/metricExt.h"

#include <iostream.h>
#include <stdio.h>

extern FILE *yyin;
extern int yyparse();

// Determine the type of "$constraint"
bool hack_in_cons=false;
void hack_cons_type(vector<string>*);
unsigned hacked_cons_type = MDL_T_NONE;

inline unsigned ui_hash(const unsigned &u) { return u; }

vector<unsigned> mdl_env::frames;
vector<mdl_var> mdl_env::all_vars;

vector<T_dyninstRPC::mdl_stmt*> mdl_data::stmts;
vector<T_dyninstRPC::mdl_metric*> mdl_data::all_metrics;
dictionary_hash<unsigned, vector<mdl_type_desc> > mdl_data::fields(ui_hash);
vector<mdl_focus_element> mdl_data::foci;
vector<T_dyninstRPC::mdl_constraint*> mdl_data::all_constraints;

static bool do_instr_rand(u_int arg_type, u_int arg_val, string& arg_name, string& a2);
static bool do_operation(mdl_var& ret, mdl_var& left, mdl_var& right, unsigned bin_op);

T_dyninstRPC::mdl_metric::mdl_metric(string id, string name, string units, unsigned fold,
				     unsigned agg, unsigned sty, u_int type,
				     vector<T_dyninstRPC::mdl_stmt*> *mv, vector<string> *flav,
				     vector<T_dyninstRPC::mdl_constraint*> *cons,
				     vector<string> *temp_counters)
: id_(id), name_(name), units_(units), fold_op_(fold), agg_op_(agg), style_(sty),
  type_(type), stmts_(mv), flavors_(flav), constraints_(cons), temp_ctr_(temp_counters) { }

T_dyninstRPC::mdl_metric::mdl_metric() { }

T_dyninstRPC::mdl_metric::~mdl_metric() {
  if (stmts_) {
    unsigned size = stmts_->size();
    for (unsigned u=0; u<size; u++)
      delete (*stmts_)[u];
    delete stmts_;
  }
  delete flavors_;
  if (constraints_) {
    unsigned size = constraints_->size();
    for (unsigned u=0; u<size; u++)
      delete (*constraints_)[u];
    delete constraints_;
  }
}

bool mdl_data::new_metric(string id, string name, string units,
			  unsigned fold, unsigned agg, unsigned sty, u_int type,
			  vector<T_dyninstRPC::mdl_stmt*> *mv,
			  vector<string> *flavs,
			  vector<T_dyninstRPC::mdl_constraint*> *cons,
			  vector<string> *temp_counters) {
  T_dyninstRPC::mdl_metric *m = new T_dyninstRPC::mdl_metric(id, name, units, fold, agg,
							     sty, type, mv, flavs, cons,
							     temp_counters);
  if (!m)
    return false;
  else {
    unsigned am_size = all_metrics.size();
    for (unsigned am=0; am<am_size; am++)
      if (all_metrics[am]->id_ == name) {
	delete all_metrics[am];
	all_metrics[am] = m;
	return true;
      }
    all_metrics += m;
    return true;
  }
}

metricDefinitionNode *T_dyninstRPC::mdl_metric::apply(vector< vector<string> >&focus,
						      string& flat_name) {
  mdl_env::push();
  if (!mdl_env::add(id_, true, type_)) return NULL;
  assert(temp_ctr_);
  unsigned tc_size = temp_ctr_->size();
  for (unsigned tc=0; tc<tc_size; tc++)
    if (!mdl_env::add((*temp_ctr_)[tc], true, MDL_T_COUNTER)) return NULL;
  assert(stmts_);
  unsigned size = stmts_->size();

  vector<dataReqNode*> flags;
  for (unsigned u=0; u<size; u++) {
    if (!(*stmts_)[u]->apply(NULL, flags)) {
      // cout << "apply of " << name_ << " failed\n";
      return NULL;
    }
  }
  size = constraints_->size();
  for (u=0; u<size; u++) {
    if ((*constraints_)[u]->match_path_) {
      // inlined constraint def
      dataReqNode *drn = NULL; vector<string> res;
      if (!(*constraints_)[u]->apply(NULL, drn, res, NULL)) {
	return NULL;
      }
    } else {
      // name of global constraint
      unsigned gl_size = mdl_data::all_constraints.size(); bool found=false;
      for (unsigned in=0; in<gl_size; in++)
	if (mdl_data::all_constraints[in]->id_ == (*constraints_)[u]->id_) {
	  found = true; break;
	}
      if (!found) return NULL;            // The global constraint does not exist
    }
  }

  // cout << "apply of " << name_ << " ok\n";
  mdl_env::pop();
  return ((metricDefinitionNode*)1);
}

T_dyninstRPC::mdl_constraint::mdl_constraint() { }
T_dyninstRPC::mdl_constraint::mdl_constraint(string id, vector<string> *match_path,
				      vector<T_dyninstRPC::mdl_stmt*> *stmts,
				      bool replace, u_int d_type, bool& error)
: id_(id), match_path_(match_path), stmts_(stmts), replace_(replace),
  data_type_(d_type), hierarchy_(0), type_(0)
{
  error = false;

  if (!match_path) {
    error = false;
    return;
  }

  unsigned size = match_path->size();

  if (match_path && size) {
//    if ((*match_path)[0] == "Procedure") {
    if ((*match_path)[0] == "Code") {
      hierarchy_ = MDL_RES_CODE;
      type_ = (size == 1) ? MDL_T_MODULE : MDL_T_PROCEDURE;
    } else if ((*match_path)[0] == "Process") {
      hierarchy_ = MDL_RES_PROCESS;
    } else if ((*match_path)[0] == "Machine") {
      hierarchy_ = MDL_RES_MACHINE;
    } else if ((*match_path)[0] == "SyncObject") {
      hierarchy_ = MDL_RES_SYNCOBJECT;
      if (size != 2) {
	type_ = MDL_T_NONE;
      } else if ((*match_path)[1] == "SpinLock") {
	type_ = MDL_T_INT;
      } else if ((*match_path)[1] == "Barrier") {
	type_ = MDL_T_INT;
      } else if ((*match_path)[1] == "MsgTag") {
	type_ = MDL_T_INT;
      } else if ((*match_path)[1] == "Semaphore") {
	type_ = MDL_T_INT;
      } else {
	error = true;
      }
    } else {
      error = true;
    }
  }
}

T_dyninstRPC::mdl_constraint::~mdl_constraint() {
  delete match_path_;
  if (stmts_) {
    for (unsigned u=0; u<stmts_->size(); u++)
      delete (*stmts_)[u];
    delete stmts_;
  }
}
  
bool T_dyninstRPC::mdl_constraint::apply(metricDefinitionNode *mn, dataReqNode *&pred,
					 vector<string>& resources, process *proc) {
  mdl_env::push();

  switch (data_type_) {
  case MDL_T_COUNTER:
  case MDL_T_WALL_TIMER:
  case MDL_T_PROC_TIMER:
    break;
  case MDL_T_NONE:
    return true;
  default:
    return false;
  }

  if (!mdl_env::add(id_, true, data_type_)) return false;
  
  // find the type for "$constraint"
  if (!stmts_ || !match_path_) return false;
  unsigned size = match_path_->size();
  if (!size) return false;
  if (!mdl_env::add(string("$constraint"), false, type_)) {
    mdl_env::pop(); return false;
  }

  unsigned stmts_size = stmts_->size();
  vector<dataReqNode*> flags;
  for (unsigned q=0; q<stmts_size; q++)
    if (!(*stmts_)[q]->apply(NULL, flags)) {
      mdl_env::pop();
      return false;
    }
  mdl_env::pop();
  return true;
}

T_dyninstRPC::mdl_constraint *mdl_data::new_constraint(string id, vector<string> *path,
						vector<T_dyninstRPC::mdl_stmt*> *stmts,
						bool replace, u_int d_type) {
  bool error;
  T_dyninstRPC::mdl_constraint *cons = new T_dyninstRPC::mdl_constraint(id, path, stmts,
									replace, d_type,
									error);
  if (error) {
    delete cons;
    return NULL;
  } else
    return cons;
}

T_dyninstRPC::mdl_instr_req::mdl_instr_req(u_int a_type, u_int a_val, string a_name,
					   string a_name_2, u_int type, string obj_name)
: arg_type_(a_type), arg_val_(a_val), arg_name_(a_name), arg_name_2_(a_name_2),
  type_(type), timer_counter_name_(obj_name) { }

T_dyninstRPC::mdl_instr_req::mdl_instr_req(u_int type, string obj_name)
: type_(type), timer_counter_name_(obj_name) { }

T_dyninstRPC::mdl_instr_req::mdl_instr_req() : type_(0) { }
T_dyninstRPC::mdl_instr_req::~mdl_instr_req() { }

bool T_dyninstRPC::mdl_instr_req::apply(unsigned where, instPoint *p, unsigned where_instr,
					metricDefinitionNode *mn, AstNode *anode,
					vector<dataReqNode*>& flags) {
  switch (where) {
  case MDL_PREPEND:
  case MDL_APPEND:
    break;
  default:
    return false;
  }

  switch (where_instr) {
  case MDL_PRE_INSN:
  case MDL_POST_INSN:
    break;
  default:
    return false;
  }

  switch (type_) {
  case MDL_SET_COUNTER:
  case MDL_ADD_COUNTER:
  case MDL_SUB_COUNTER:
  case MDL_CALL_FUNC:
  case MDL_CALL_FUNC_COUNTER:
    // handle instr_rand
    if (!do_instr_rand(arg_type_, arg_val_, arg_name_, arg_name_2_)) return false;
  }
  mdl_var timer;
  if (!mdl_env::get(timer, timer_counter_name_)) return false;

  switch (type_) {
  case MDL_SET_COUNTER:
  case MDL_ADD_COUNTER:
  case MDL_SUB_COUNTER:
  case MDL_CALL_FUNC:
  case MDL_CALL_FUNC_COUNTER:
    if (timer.type() != MDL_T_COUNTER) return false;
    break;
  case MDL_START_WALL_TIMER:
  case MDL_STOP_WALL_TIMER:
    if (timer.type() != MDL_T_WALL_TIMER) return false;
    break;
  case MDL_START_PROC_TIMER:
  case MDL_STOP_PROC_TIMER:
    if (timer.type() != MDL_T_PROC_TIMER) return false;
    break;
  default:
    return false;
  }
  return true;
}

T_dyninstRPC::mdl_stmt::mdl_stmt() { }

T_dyninstRPC::mdl_for_stmt::mdl_for_stmt(string index_name, T_dyninstRPC::mdl_expr *list_exp, T_dyninstRPC::mdl_stmt *body) 
: for_body_(body), index_name_(index_name), list_expr_(list_exp) { }
T_dyninstRPC::mdl_for_stmt::mdl_for_stmt() { }
T_dyninstRPC::mdl_for_stmt::~mdl_for_stmt() {
  delete for_body_;
  delete list_expr_;
}

bool T_dyninstRPC::mdl_for_stmt::apply(metricDefinitionNode *mn,
					 vector<dataReqNode*>& flags) {
  mdl_env::push();
  if (!mdl_env::add(index_name_, false)) return false;
  mdl_var list_var(false);
  if (!list_expr_->apply(list_var))
    return false;
  if (!list_var.is_list())
    return false;
  mdl_env::set_type(list_var.element_type(), index_name_);
  bool res = for_body_->apply(mn, flags);
  mdl_env::pop();
  return res;
}

T_dyninstRPC::mdl_list_stmt::mdl_list_stmt(u_int type, string ident,
					   vector<string> *elems,
					   bool is_lib, string flavor) 
: type_(type), id_(ident), elements_(elems), is_lib_(is_lib), flavor_(flavor) { }
T_dyninstRPC::mdl_list_stmt::mdl_list_stmt() { }
T_dyninstRPC::mdl_list_stmt::~mdl_list_stmt() { delete elements_; }

bool T_dyninstRPC::mdl_list_stmt::apply(metricDefinitionNode *mn,
					 vector<dataReqNode*>& flags) {
  if (!elements_)
    return false;
  unsigned list_type = MDL_T_NONE;
  switch (type_) {
  case MDL_T_INT: list_type = MDL_T_LIST_INT; break;
  case MDL_T_FLOAT: list_type = MDL_T_LIST_FLOAT; break;
  case MDL_T_STRING: list_type = MDL_T_LIST_STRING; break;
  case MDL_T_PROCEDURE: list_type = MDL_T_LIST_PROCEDURE; break;
  case MDL_T_MODULE: list_type = MDL_T_LIST_MODULE; break;
  default: return false;
  }
  return (mdl_env::add(id_, false, list_type));
}

T_dyninstRPC::mdl_icode::mdl_icode() { }
T_dyninstRPC::mdl_icode::mdl_icode(u_int iop1, u_int ival1, string str1,
				   u_int iop2, u_int ival2, string str2,
				   u_int bin_op, bool use_if,
				   T_dyninstRPC::mdl_instr_req *ireq)
: if_op1_(iop1), if_val1_(ival1), if_str1_(str1),
  if_op2_(iop2), if_val2_(ival2), if_str2_(str2),
  bin_op_(bin_op), use_if_(use_if), req_(ireq) { }
T_dyninstRPC::mdl_icode::~mdl_icode() { delete req_; }

bool T_dyninstRPC::mdl_icode::apply(u_int position, instPoint *p, u_int where_instr,
				    metricDefinitionNode *mn,
				    vector<dataReqNode*>& flags) {
  if (!req_) return false;
  if (use_if_) {
    string empty;
    if (!do_instr_rand(if_op1_, if_val1_, if_str1_, empty)) return false;

    switch (bin_op_) {
    case MDL_LT:  case MDL_GT:  case MDL_LE:  case MDL_GE:  case MDL_EQ:  case MDL_NE:
      if (!do_instr_rand(if_op2_, if_val2_, if_str2_, empty)) return false;
      break;
    case MDL_T_NONE:
      break;
    default:
      return false;
    }
  }
  return (req_->apply(position, p, where_instr, mn, NULL, flags));
}

T_dyninstRPC::mdl_expr::mdl_expr() { }
T_dyninstRPC::mdl_expr::~mdl_expr() { }

T_dyninstRPC::mdl_v_expr::mdl_v_expr() 
: args_(NULL), literal_(0), arg_(0), left_(NULL), right_(NULL), type_(MDL_T_NONE),
  ok_(false) { }

T_dyninstRPC::mdl_v_expr::mdl_v_expr(string var, vector<string> fields) 
: var_(var), fields_(fields),
  args_(NULL), literal_(0), arg_(0), left_(NULL), right_(NULL),
  type_(MDL_RVAL_DEREF), do_type_walk_(false), ok_(false) { }

T_dyninstRPC::mdl_v_expr::mdl_v_expr(string func_name,
				     vector<T_dyninstRPC::mdl_expr *> *a) 
: var_(func_name), args_(a),
  literal_(0), arg_(100000), left_(NULL), right_(NULL),
  type_(MDL_RVAL_FUNC), do_type_walk_(false), ok_(false) { }

T_dyninstRPC::mdl_v_expr::mdl_v_expr(int int_lit) 
: args_(NULL), literal_(int_lit), arg_(0), left_(NULL), right_(NULL),
  type_(MDL_RVAL_INT), do_type_walk_(false), ok_(false) { }

T_dyninstRPC::mdl_v_expr::mdl_v_expr(string string_lit) 
: var_(string_lit),
  args_(NULL), literal_(0), arg_(0), left_(NULL), right_(NULL),
  type_(MDL_RVAL_STRING), do_type_walk_(false), ok_(false) { }

T_dyninstRPC::mdl_v_expr::mdl_v_expr(u_int bin_op, T_dyninstRPC::mdl_expr *left,
				 T_dyninstRPC::mdl_expr *right) 
: args_(NULL), literal_(0),
  arg_(bin_op), left_(left), right_(right),
  type_(MDL_RVAL_EXPR), do_type_walk_(false), ok_(false) { }

T_dyninstRPC::mdl_v_expr::mdl_v_expr(string var, u_int array_index) 
: var_(var), args_(NULL), literal_(0), arg_(array_index), left_(NULL), right_(NULL),
  type_(MDL_RVAL_ARRAY), do_type_walk_(false), ok_(false) { }

T_dyninstRPC::mdl_v_expr::~mdl_v_expr() {
  delete args_; delete left_; delete right_;
  if (args_) {
    unsigned size = args_->size();
    for (unsigned u=0; u<size; u++)
      delete (*args_)[u];
    delete args_;
  }
}

bool T_dyninstRPC::mdl_v_expr::apply(mdl_var& ret) {
  switch (type_) {
  case MDL_RVAL_INT: 
    ok_ = true;
    return (ret.set(literal_));
  case MDL_RVAL_STRING:
    ok_ = true;
    return (ret.set(var_));
  case MDL_RVAL_ARRAY:
    {
      mdl_var array(false);
      if (!mdl_env::get(array, var_)) return false;
      if (!array.is_list()) return false;  
      ok_ = true;
      ret.set_type(array.element_type());
      return true;
    }
  case MDL_RVAL_EXPR:
    {
      mdl_var left_val(false), right_val(false);
      if (!left_ || !right_) return false;
      if (!left_->apply(left_val)) return false;
      if (!right_->apply(right_val)) return false;
      ok_ =  do_operation(ret, left_val, right_val, arg_);
      return ok_;
    }
  case MDL_RVAL_FUNC:
    if (!args_) return false;
    else {  // DO NOT DEFINE VARIABLES ACROSS LABEL JUMPS
    unsigned size = args_->size();

    if (var_ == "lookupFunction") {
      if (size != 1) return false;
      mdl_var arg1(false);
      if (!(*args_)[0]->apply(arg1)) return false;
      if (arg1.type() != MDL_T_STRING) return false;
      arg_ = 0;
      ret.set_type(MDL_T_PROCEDURE);
      ok_ = true;
    } else if (var_ == "lookupModule") {
      if (size != 1) return false;
      mdl_var arg1(false);
      if (!(*args_)[0]->apply(arg1)) return false;
      if (arg1.type() != MDL_T_STRING) return false;
      arg_ = 1;
      ret.set_type(MDL_T_MODULE);
      ok_ = true;
    } else if (var_ == "libraryTag") {
      if (size != 1) return false;
      mdl_var arg1(false);
      if (!(*args_)[0]->apply(arg1)) return false;
      if (arg1.type() != MDL_T_INT) return false;
      arg_ = 2;
      int res = 1;
      ok_ = ret.set(res);
    } else
      return false;
    return ok_;
    }
  case MDL_RVAL_DEREF:
    if (!fields_.size()) {
      do_type_walk_ = false;
      ok_ = mdl_env::get(ret, var_);
      return ok_;
    } else {
      // TODO -- build type_walk here
      do_type_walk_ = true;
      mdl_var temp(false);
      if (!mdl_env::get(temp, var_)) return false;

      vector<mdl_type_desc> found;
      unsigned v_index = 0, v_max = fields_.size();
      unsigned current_type = temp.type();

      while (v_index < v_max) {
	if (!mdl_data::fields.defines(current_type)) return false;
	found = mdl_data::fields[current_type];
	string next_field = fields_[v_index];
	bool type_found=false;
	unsigned size = found.size();
	for (unsigned u=0; u<size; u++) 
	  if (found[u].name == next_field) {
	    type_found = true;
	    type_walk += current_type;
	    type_walk += u;
	    current_type = found[u].type;
	    break;
	  }
	if (!type_found) return false; 
	v_index++;
      }
      ok_ = true;
      ret.set_type(current_type);
      return ok_;
    }
  default:
    return false;
  }
  return false;
}

T_dyninstRPC::mdl_if_stmt::mdl_if_stmt(T_dyninstRPC::mdl_expr *expr, T_dyninstRPC::mdl_stmt *body) : expr_(expr), body_(body) { }
T_dyninstRPC::mdl_if_stmt::mdl_if_stmt() { }
T_dyninstRPC::mdl_if_stmt::~mdl_if_stmt() {
  delete expr_; delete body_;
}

bool T_dyninstRPC::mdl_if_stmt::apply(metricDefinitionNode *mn,
				      vector<dataReqNode*>& flags) {
  mdl_var res(false); int iv;
  if (!expr_->apply(res))
    return false;
  switch (res.type()) {
  case MDL_T_INT:
    if (!res.get(iv))
      return false;
    iv = 1;
    if (!iv)
      return true;
    break;
  default:
    return false;
  }
  bool ret = body_->apply(NULL, flags);
  return ret;
}

T_dyninstRPC::mdl_seq_stmt::mdl_seq_stmt(vector<T_dyninstRPC::mdl_stmt*> *stmts) : stmts_(stmts) { }
T_dyninstRPC::mdl_seq_stmt::mdl_seq_stmt() { }
T_dyninstRPC::mdl_seq_stmt::~mdl_seq_stmt() {
  if (stmts_) {
    unsigned size = stmts_->size();
    for (unsigned u=0; u<size; u++)
      delete (*stmts_)[u];
    delete stmts_;
  }
}
 
bool T_dyninstRPC::mdl_seq_stmt::apply(metricDefinitionNode *mn,
				       vector<dataReqNode*>& flags) {
  if (!stmts_)
    return true;
  unsigned size = stmts_->size();
  for (unsigned index=0; index<size; index++)
    if (!(*stmts_)[index]->apply(NULL, flags))
      return false;
  return true;
}

T_dyninstRPC::mdl_instr_stmt::mdl_instr_stmt(unsigned pos, T_dyninstRPC::mdl_expr *expr,
				      vector<T_dyninstRPC::mdl_icode*> *reqs,
				      unsigned where, bool constrained) 
: position_(pos), point_expr_(expr), icode_reqs_(reqs),
  where_instr_(where), constrained_(constrained) { }
T_dyninstRPC::mdl_instr_stmt::mdl_instr_stmt() { }
T_dyninstRPC::mdl_instr_stmt::~mdl_instr_stmt() {
  delete point_expr_;
  if (icode_reqs_) {
    unsigned size = icode_reqs_->size();
    for (unsigned u=0; u<size; u++)
      delete (*icode_reqs_)[u];
    delete icode_reqs_;
  }
}

bool T_dyninstRPC::mdl_instr_stmt::apply(metricDefinitionNode *mn,
					 vector<dataReqNode*>& flags) {
  mdl_var temp(false);
  if (!icode_reqs_)
    return false;
  if (!point_expr_->apply(temp))
    return false;
  if (temp.type() != MDL_T_POINT)
    return false;
  instPoint *p;
  if (!temp.get(p))
    return false;
  unsigned size = icode_reqs_->size();
  for (unsigned u=0; u<size; u++)
    if (!(*icode_reqs_)[u]->apply(position_, p, where_instr_, NULL, flags))
      return false;
  return true;
}

bool mdl_init() {
  static bool init_done = false;
  if (init_done) return true;

  vector<mdl_type_desc> kids;
  mdl_type_desc self, kid;
  mdl_focus_element fe;

  self.name = "SyncObject"; self.type = 0; self.end_allowed = false; 
  kid.name = "MsgTag"; kid.type = MDL_T_INT; kid.end_allowed = true; kids += kid;
  kid.name = "Barrier"; kid.type = MDL_T_INT; kid.end_allowed = true; kids += kid;
  kid.name = "Semaphore"; kid.type = MDL_T_INT; kid.end_allowed = true; kids += kid;
  kid.name = "SpinLock"; kid.type = MDL_T_INT; kid.end_allowed = true; kids += kid;
  fe.self = self; fe.kids = kids;
  mdl_data::foci += fe;
  kids.resize(0);

//  self.name = "Procedure"; self.type = MDL_T_STRING; self.end_allowed = true;
  self.name = "Code"; self.type = MDL_T_STRING; self.end_allowed = true;
  kid.name = "Module"; kid.type = MDL_T_STRING; self.end_allowed = true; kids += kids;
  fe.self = self; fe.kids = kids;
  mdl_data::foci += fe;
  kids.resize(0);

  self.name = "Process"; self.type = MDL_T_STRING; self.end_allowed = true;
  fe.self = self; fe.kids.resize(0);
  mdl_data::foci += fe;

  self.name = "Machine"; self.type = MDL_T_STRING; self.end_allowed = true;
  fe.self = self; fe.kids.resize(0);
  mdl_data::foci += fe;

  mdl_env::push();
  mdl_env::add(string("$procedures"), false, MDL_T_LIST_PROCEDURE);
  mdl_env::add(string("$modules"), false, MDL_T_LIST_MODULE);
  mdl_env::add(string("$start"), false, MDL_T_PROCEDURE);
  mdl_env::add(string("$exit"), false, MDL_T_PROCEDURE);
  mdl_env::add(string("$machine"), false, MDL_T_STRING);

  /* Are these entered by hand at the new scope ? */
  /* $constraint, $arg, $return */

  vector<mdl_type_desc> field_list;
  mdl_type_desc desc;
  desc.name = "name"; desc.type = MDL_T_STRING; field_list += desc;
  desc.name = "calls"; desc.type = MDL_T_LIST_POINT; field_list += desc;
  desc.name = "entry"; desc.type = MDL_T_POINT; field_list += desc;
  desc.name = "return"; desc.type = MDL_T_POINT; field_list += desc;
  desc.name = "tag"; desc.type = MDL_T_INT; field_list += desc;
  mdl_data::fields[MDL_T_PROCEDURE] = field_list;
  field_list.resize(0);

  desc.name = "name"; desc.type = MDL_T_STRING; field_list += desc;
  desc.name = "funcs"; desc.type = MDL_T_LIST_PROCEDURE; field_list += desc;
  mdl_data::fields[MDL_T_MODULE] = field_list;
  field_list.resize(0);

  field_list.resize(0);

  return true;
}

// TODO -- if this fails, all metrics are unusable ?
bool mdl_apply() {
  unsigned size = mdl_data::stmts.size();

  vector<dataReqNode*> flags;
  for (unsigned u=0; u<size; u++)
    if (!mdl_data::stmts[u]->apply(NULL, flags))
      return false;

  // TODO -- use a proper vector
  vector< vector<string> >vs;
  string empty;

  vector<T_dyninstRPC::mdl_constraint*> ok_cons;
  size = mdl_data::all_constraints.size();
  for (u=0; u<size; u++) {
    dataReqNode *drn = NULL;
    vector<string> res;
    if (mdl_data::all_constraints[u]->apply(NULL, drn, res, NULL)) {
      ok_cons += mdl_data::all_constraints[u];
      // cout << "constraint defined: " << mdl_data::all_constraints[u]->id_ << endl;
    } else
      delete mdl_data::all_constraints[u];
  }
  mdl_data::all_constraints = ok_cons;

  vector<T_dyninstRPC::mdl_metric*> ok_mets;
  size = mdl_data::all_metrics.size();
  for (u=0; u<size; u++) {
    if (mdl_data::all_metrics[u]->apply(vs, empty) == (metricDefinitionNode*)1) {
      ok_mets += mdl_data::all_metrics[u];
      // cout << "metric defined: " << mdl_data::all_metrics[u]->id_ << endl;
    } else
      delete mdl_data::all_metrics[u];
  }
  mdl_data::all_metrics = ok_mets;
  
  return true;
}

bool mdl_send(dynRPCUser *remote) {
  remote->send_stmts(&mdl_data::stmts);
  remote->send_constraints(&mdl_data::all_constraints);
  remote->send_metrics(&mdl_data::all_metrics);

#ifdef notdef
  unsigned size = mdl_data::all_metrics.size(), index;
  for (index=0; index<size; index++) {
    cout << *(mdl_data::all_metrics[index]) << endl;
  }
  size = mdl_data::all_constraints.size();
  for (index=0; index<size; index++) {
    cout << *(mdl_data::all_constraints[index]) << endl;
  }
#endif

  return true;
}

void mdl_destroy() {
  unsigned size = mdl_data::all_constraints.size();
  for (unsigned u=0; u<size; u++)
    delete (mdl_data::all_constraints[u]);
  mdl_data::all_constraints.resize(0);
  
  size = mdl_data::all_metrics.size();
  for (u=0; u<size; u++)
    delete (mdl_data::all_metrics[u]);
  mdl_data::all_metrics.resize(0);

  size = mdl_data::stmts.size();
  for (u=0; u<size; u++)
    delete mdl_data::stmts[u];
}

static bool do_operation(mdl_var& ret, mdl_var& left_val, mdl_var& right_val, unsigned bin_op) {

  switch (bin_op) {
  case MDL_PLUS:
  case MDL_MINUS:
  case MDL_DIV:
  case MDL_MULT:
    if ((left_val.type() == MDL_T_INT) && (right_val.type() == MDL_T_INT)) {
      int v1, v2;
      if (!left_val.get(v1)) return false;
      if (!right_val.get(v2)) return false;
      switch (bin_op) {
      case MDL_PLUS: return (ret.set(v1+v2));
      case MDL_MINUS: return (ret.set(v1-v2));
      case MDL_DIV: return (ret.set(v1/v2));
      case MDL_MULT: return (ret.set(v1*v2));
      }
    } else if (((left_val.type() == MDL_T_INT) || (left_val.type() == MDL_T_FLOAT)) &&
	       ((right_val.type() == MDL_T_INT) || (right_val.type() == MDL_T_FLOAT))) {
      float v1, v2;
      if (left_val.type() == MDL_T_INT) {
	int i1; if (!left_val.get(i1)) return false; v1 = i1;
      } else {
	if (!left_val.get(v1)) return false;
      }
      if (right_val.type() == MDL_T_INT) {
	int i1; if (!right_val.get(i1)) return false; v2 = i1;
      } else {
	if (!right_val.get(v2)) return false;
      }
      switch (bin_op) {
      case MDL_PLUS: return (ret.set(v1+v2));
      case MDL_MINUS: return (ret.set(v1-v2));
      case MDL_DIV: return (ret.set(v1/v2));
      case MDL_MULT: return (ret.set(v1*v2));
      }
    } else
      return false;
  case MDL_LT:
  case MDL_GT:
  case MDL_LE:
  case MDL_GE:
  case MDL_EQ:
  case MDL_NE:
    if ((left_val.type() == MDL_T_INT) && (right_val.type() == MDL_T_INT)) {
      int v1, v2;
      if (!left_val.get(v1)) return false;
      if (!right_val.get(v2)) return false;
      switch (bin_op) {
      case MDL_LT: return (ret.set(v1 < v2));
      case MDL_GT: return (ret.set(v1 > v2));
      case MDL_LE: return (ret.set(v1 <= v2));
      case MDL_GE: return (ret.set(v1 >= v2));
      case MDL_EQ: return (ret.set(v1 == v2));
      case MDL_NE: return (ret.set(v1 != v2));
      }
    } else if (((left_val.type() == MDL_T_INT) || (left_val.type() == MDL_T_FLOAT)) &&
	       ((right_val.type() == MDL_T_INT) || (right_val.type() == MDL_T_FLOAT))) {
      float v1, v2;
      if (left_val.type() == MDL_T_INT) {
	int i1; if (!left_val.get(i1)) return false; v1 = i1;
      } else {
	if (!left_val.get(v1)) return false;
      }
      if (right_val.type() == MDL_T_INT) {
	int i1; if (!right_val.get(i1)) return false; v2 = i1;
      } else {
	if (!right_val.get(v2)) return false;
      }
      switch (bin_op) {
      case MDL_LT: return (ret.set(v1 < v2));
      case MDL_GT: return (ret.set(v1 > v2));
      case MDL_LE: return (ret.set(v1 <= v2));
      case MDL_GE: return (ret.set(v1 >= v2));
      case MDL_EQ: return (ret.set(v1 == v2));
      case MDL_NE: return (ret.set(v1 != v2));
      }
    } else
      return false;
  case MDL_AND:
  case MDL_OR:
    if ((left_val.type() == MDL_T_INT) && (right_val.type() == MDL_T_INT)) {
      int v1, v2;
      if (!left_val.get(v1)) return false;
      if (!right_val.get(v2)) return false;
      switch (bin_op) {
      case MDL_AND: return (ret.set(v1 && v2));
      case MDL_OR: return (ret.set(v1 || v2));
      }
    } else
      return false;
  default:
      return false;
  }
  return false;
}

bool do_instr_rand(u_int arg_type, u_int arg_val, string& arg_name, string& arg_name2) {
  switch (arg_type) {
  case MDL_T_INT:
    break;
  case MDL_ARG:
    // TODO -- check arg_ that is used as register index
    // Check the legality of this -- or allow to be used anywhere ?
    break;
  case MDL_RETURN:
    break;
  case MDL_READ_SYMBOL:
    break;
  case MDL_READ_ADDRESS:
    break;
  case MDL_CALL_FUNC:
    break;
  case MDL_CALL_FUNC_COUNTER:
    break;
  case MDL_T_COUNTER:
    break;
  default:
    return false;
  }
  return true;
}

void hack_cons_type(vector<string> *str_vec) {
  hack_in_cons = true;
  hacked_cons_type = MDL_T_NONE;
  unsigned size = str_vec->size();
  if (!size || (size==1)) {
    hack_in_cons = false; return;
  }
//  if ((*str_vec)[0] == "Procedure") {
  if ((*str_vec)[0] == "Code") {
    switch (size) {
    case 2:
      hacked_cons_type = MDL_T_MODULE;
      break;
    case 3:
      hacked_cons_type = MDL_T_PROCEDURE;
      break;
    default:
      hack_in_cons = false;
    }
  } else if ((*str_vec)[0] == "SyncObject") {
    switch (size) {
    case 2:
      if ((*str_vec)[1] == "MsgTag") {
	hacked_cons_type = MDL_T_INT;
      }
      break;
    default:
      hack_in_cons = false;
    }
  } else 
    hack_in_cons = false;
}
