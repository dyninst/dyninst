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

// $Id: mdl.C,v 1.55 2002/11/25 23:52:42 schendel Exp $

#include "dyninstRPC.xdr.CLNT.h"
#include "paradyn/src/met/globals.h"
#include "paradyn/src/met/metricExt.h"

#include <iostream.h>

inline unsigned ui_hash(const unsigned &u) { return u; }

vector<unsigned> mdl_env::frames;
vector<mdl_var> mdl_env::all_vars;

vector<T_dyninstRPC::mdl_stmt*> mdl_data::stmts;
vector<T_dyninstRPC::mdl_metric*> mdl_data::all_metrics;
dictionary_hash<unsigned, vector<mdl_type_desc> > mdl_data::fields(ui_hash);
vector<mdl_focus_element> mdl_data::foci;
vector<T_dyninstRPC::mdl_constraint*> mdl_data::all_constraints;
vector<string> mdl_data::lib_constraints;
vector<unsigned> mdl_data::lib_constraint_flags;

static bool do_operation(mdl_var& ret, mdl_var& left, mdl_var& right, unsigned bin_op);
static bool do_operation(mdl_var& ret, mdl_var& lval, unsigned u_op, bool is_preop);


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


T_dyninstRPC::mdl_metric::mdl_metric(string id, string name, string units, 
				    u_int agg, u_int sty, u_int type, string hwcntr, 
				    vector<T_dyninstRPC::mdl_stmt*> *mv, 
				    vector<string> *flav,
				    vector<T_dyninstRPC::mdl_constraint*> *cons,
				    vector<string> *temp_counters,
				    bool developerMode,
				    int unitstype)
: id_(id), name_(name), units_(units), agg_op_(agg), style_(sty),
  type_(type), hwcntr_(hwcntr), stmts_(mv), flavors_(flav), constraints_(cons),
  temp_ctr_(temp_counters), developerMode_(developerMode),
  unitstype_(unitstype) { }

T_dyninstRPC::mdl_metric::mdl_metric()
: id_(""), name_(""), units_(""), agg_op_(0), style_(0), type_(0), 
  stmts_(NULL), flavors_(NULL), constraints_(NULL), temp_ctr_(NULL),
  developerMode_(false), unitstype_(0) { }

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
			  u_int agg, u_int sty, u_int type, string hwcntr, 
			  vector<T_dyninstRPC::mdl_stmt*> *mv,
			  vector<string> *flavs,
			  vector<T_dyninstRPC::mdl_constraint*> *cons,
			  vector<string> *temp_counters,
			  bool developerMode,
			  int normalized) {

  T_dyninstRPC::mdl_metric *m = new T_dyninstRPC::mdl_metric(id, name, units, 
							     agg,
							     sty, type, hwcntr, mv, 
							     flavs, cons,
							     temp_counters,
							     developerMode,
							     normalized);
  if (!m)
    return false;
  else {
    mdl_data::unique_name(id);
    all_metrics += m;
    return true;
  }
}

bool T_dyninstRPC::mdl_metric::apply(vector<processMetFocusNode *> *,
				     const Focus &, vector<pd_process *>,
				     bool, bool) {
  mdl_env::push();
  if (!mdl_env::add(id_, true, type_)) return false;
  assert(temp_ctr_);
  unsigned tc_size = temp_ctr_->size();
  for (unsigned tc=0; tc<tc_size; tc++)
    if (!mdl_env::add((*temp_ctr_)[tc], true, MDL_T_COUNTER)) return false;

  // apply 'base' statements
  assert(stmts_);
  unsigned size = stmts_->size();
  vector<const instrDataNode*> flags;
  for (unsigned u=0; u<size; u++) {
    if (!(*stmts_)[u]->apply(NULL, flags)) {
      cerr << "In metric " << name_ << ": apply of " << u 
        << "th base statement failed." << endl;
      return false;
    }
  }

  // apply constraints
  size = constraints_->size();
  for (unsigned u1=0; u1<size; u1++) {
    if ((*constraints_)[u1]->match_path_) {
      // inlined constraint def
      instrDataNode *drn = NULL;
      Hierarchy res;
      if (!(*constraints_)[u1]->apply(NULL, &drn, res, NULL, false))
      {
        cerr << "In metric " << name_ << ": apply of " << u1
          << "th constraint failed." << endl;
        return false;
      }
    } else {
      // name of global constraint
      unsigned gl_size = mdl_data::all_constraints.size();
      bool found=false;
      for (unsigned in=0; in<gl_size; in++)
        if (mdl_data::all_constraints[in]->id_ == (*constraints_)[u1]->id_) {
          found = true; break;
        }
      if (!found) {
        cerr << "In metric " << name_ << ": global constraint " 
          << (*constraints_)[u1]->id_ << " undefined." << endl;
        return false;
      }
    }
  }

  // cout << "apply of " << name_ << " ok\n";
  mdl_env::pop();
  return true;
}

T_dyninstRPC::mdl_constraint::mdl_constraint()
: id_(""), match_path_(NULL), stmts_(NULL), replace_(false),
  data_type_(0), hierarchy_(0), type_(0) { }

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

  if (match_path && size) 
  {
    if ((*match_path)[0] == "Code") 
    {
      hierarchy_ = MDL_RES_CODE;
      type_ = (size == 1) ? MDL_T_MODULE : MDL_T_PROCEDURE;
    }
    else if ((*match_path)[0] == "Process") 
      hierarchy_ = MDL_RES_PROCESS;
    else if ((*match_path)[0] == "Machine")
      hierarchy_ = MDL_RES_MACHINE;
    else if((*match_path)[0] == "Memory") 
    {
      hierarchy_ = MDL_RES_MEMORY;
      if (size == 1)
        type_ = MDL_T_VARIABLE;
      else
        type_ = MDL_T_INT;
    }
    else if ((*match_path)[0] == "SyncObject") 
    {
      hierarchy_ = MDL_RES_SYNCOBJECT;
      if (size == 1)
	type_ = MDL_T_STRING;
      else if ((*match_path)[1] == "SpinLock")
	type_ = MDL_T_INT;
      else if ((*match_path)[1] == "Barrier")
	type_ = MDL_T_INT;
      else if ((*match_path)[1] == "Message")
	type_ = MDL_T_INT;
      else if ((*match_path)[1] == "Semaphore")
	type_ = MDL_T_INT;
      else if ((*match_path)[1] == "Mutex")
	type_ = MDL_T_INT;
      else if ((*match_path)[1] == "CondVar")
	type_ = MDL_T_INT;
      else if ((*match_path)[1] == "RwLock")
	type_ = MDL_T_INT;
      else 
      {
        cout << "Error in constraint '" << id.c_str()
          << "': unknown resource '" << (*match_path)[1].c_str()
          << "'" << endl;
	error = true;
      }
    }
    else 
    {
      cout << "Error in constraint '" << id.c_str()
        << "': unknown resource '" << (*match_path)[0].c_str()
        << "'" << endl;
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

bool T_dyninstRPC::mdl_constraint::apply(instrCodeNode *,
					 instrDataNode **,
					 const Hierarchy &,
					 pd_process *, 
					 bool)
{
  mdl_env::push();

  switch (data_type_) {
  case MDL_T_COUNTER:
  case MDL_T_WALL_TIMER:
  case MDL_T_PROC_TIMER:
  case MDL_T_HW_TIMER:
  case MDL_T_HW_COUNTER:
    break;
  case MDL_T_NONE:
    return true;
  default:
    return false;
  }

  if (!mdl_env::add(id_, true, data_type_)) return false;
  
  // find the type for "$constraint*"
  if (!stmts_ || !match_path_) return false;
  unsigned size = match_path_->size();
  if (!size) return false;
  for(unsigned dx = 0; dx < size; dx++) {
    string s = string("$constraint") + string(dx);
    if (!mdl_env::add(s, false, type_)) {
      mdl_env::pop(); return false;
    }
  }

  unsigned stmts_size = stmts_->size();
  vector<const instrDataNode*> flags;
  for (unsigned q=0; q<stmts_size; q++)
    if (!(*stmts_)[q]->apply(NULL, flags)) {
      mdl_env::pop();
      return false;
    }
  mdl_env::pop();
  return true;
}

bool T_dyninstRPC::mdl_constraint::replace() {
  return replace_ ;
}

string T_dyninstRPC::mdl_constraint::id() {
  return id_ ;
}

T_dyninstRPC::mdl_constraint *mdl_data::new_constraint(string id, 
  vector<string> *path, vector<T_dyninstRPC::mdl_stmt*> *stmts, 
  bool replace, u_int d_type) 
{
  bool error;
  T_dyninstRPC::mdl_constraint *cons = new T_dyninstRPC::mdl_constraint(
    id, path, stmts, replace, d_type, error);
  if (error) {
    delete cons;
    return NULL;
  } else
    return cons;
}

T_dyninstRPC::mdl_stmt::mdl_stmt() { }

T_dyninstRPC::mdl_for_stmt::mdl_for_stmt(string index_name, 
					 T_dyninstRPC::mdl_expr *list_exp, 
					 T_dyninstRPC::mdl_stmt *body) 
: for_body_(body), index_name_(index_name), list_expr_(list_exp) { }

T_dyninstRPC::mdl_for_stmt::mdl_for_stmt()
: for_body_(NULL), index_name_(""), list_expr_(NULL) { }

T_dyninstRPC::mdl_for_stmt::~mdl_for_stmt() {
  delete for_body_;
  delete list_expr_;
}


bool T_dyninstRPC::mdl_for_stmt::apply(instrCodeNode *mn,
				       vector<const instrDataNode*>& flags) {
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
					   bool is_lib, vector<string> *flavor) 
: type_(type), id_(ident), elements_(elems), is_lib_(is_lib), flavor_(flavor) 
{ }

T_dyninstRPC::mdl_list_stmt::mdl_list_stmt()
: type_(MDL_T_NONE), id_(""), elements_(NULL), is_lib_(false), flavor_(NULL)
{ }

T_dyninstRPC::mdl_list_stmt::~mdl_list_stmt() { delete elements_; }

bool T_dyninstRPC::mdl_list_stmt::apply(instrCodeNode * ,
					vector<const instrDataNode*>& ) {
  if (!elements_)
    return false;
  unsigned list_type = MDL_T_NONE;
  switch (type_) {
  case MDL_T_INT: list_type = MDL_T_LIST_INT; break;
  case MDL_T_FLOAT: list_type = MDL_T_LIST_FLOAT; break;
  case MDL_T_STRING: list_type = MDL_T_LIST_STRING; break;
  case MDL_T_PROCEDURE_NAME: list_type = MDL_T_LIST_PROCEDURE_NAME; break;
  case MDL_T_MODULE: list_type = MDL_T_LIST_MODULE; break;
  default: 
      // this should only happen if there is a parser error.
      abort();
      return false;
  }
  return (mdl_env::add(id_, false, list_type));
}

T_dyninstRPC::mdl_icode::mdl_icode()
  : if_expr_(NULL), expr_(NULL) { }

T_dyninstRPC::mdl_icode::mdl_icode(T_dyninstRPC::mdl_expr *expr1, 
				   T_dyninstRPC::mdl_expr *expr2)
  : if_expr_(expr1), expr_(expr2) { }

T_dyninstRPC::mdl_icode::~mdl_icode() { delete if_expr_; delete expr_; }

bool T_dyninstRPC::mdl_icode::apply(AstNode *&, bool, pd_process *)
{
  if (!expr_) return false;
  AstNode* ast = NULL;
  if (!expr_->apply(ast)) return false;
  if (if_expr_)
  {
    AstNode* pred = NULL;
    if (!if_expr_->apply(pred)) return false;
  }
  return true;
}

T_dyninstRPC::mdl_expr::mdl_expr() { }

T_dyninstRPC::mdl_expr::~mdl_expr() { }

T_dyninstRPC::mdl_v_expr::mdl_v_expr() 
: type_(MDL_T_NONE), int_literal_(0), bin_op_(0), u_op_(0),
  left_(NULL), right_(NULL), args_(NULL), ok_(false) 
{ assert(0); }

T_dyninstRPC::mdl_v_expr::mdl_v_expr(int int_lit) 
: type_(MDL_EXPR_INT), int_literal_(int_lit), bin_op_(0), u_op_(0),
  left_(NULL), right_(NULL), args_(NULL), do_type_walk_(false), ok_(false) 
{ }

T_dyninstRPC::mdl_v_expr::mdl_v_expr(string a_str, bool is_literal) 
: int_literal_(0), bin_op_(0), u_op_(0), left_(NULL), 
  right_(NULL), args_(NULL), do_type_walk_(false), ok_(false) 
{
  if (is_literal)
  {
    type_ = MDL_EXPR_STRING;
    str_literal_ = a_str;
  }
  else
  {
    type_ = MDL_EXPR_VAR;
    var_ = a_str;
  }
}

T_dyninstRPC::mdl_v_expr::mdl_v_expr(T_dyninstRPC::mdl_expr* expr, 
				     vector<string> fields) 
: type_(MDL_EXPR_DOT), int_literal_(0), bin_op_(0), 
  u_op_(0), left_(expr), right_(NULL), args_(NULL),
  fields_(fields), do_type_walk_(false), ok_(false) 
{ }

T_dyninstRPC::mdl_v_expr::mdl_v_expr(string func_name,
				     vector<T_dyninstRPC::mdl_expr *> *a) 
: type_(MDL_EXPR_FUNC), int_literal_(0), var_(func_name), bin_op_(100000),
  u_op_(0), left_(NULL), right_(NULL), args_(a),
  do_type_walk_(false), ok_(false)
{ }

T_dyninstRPC::mdl_v_expr::mdl_v_expr(u_int bin_op, T_dyninstRPC::mdl_expr *left,
				     T_dyninstRPC::mdl_expr *right) 
: type_(MDL_EXPR_BINOP), int_literal_(0), bin_op_(bin_op), u_op_(0),
  left_(left), right_(right), args_(NULL), do_type_walk_(false), ok_(false) 
{ }

T_dyninstRPC::mdl_v_expr::mdl_v_expr(string var, u_int assign_op, 
				     T_dyninstRPC::mdl_expr *expr)
: type_(MDL_EXPR_ASSIGN), int_literal_(0), var_(var), bin_op_(assign_op), 
  u_op_(0), left_(expr), right_(NULL), args_(NULL), 
  do_type_walk_(false), ok_(false) 
{ }

T_dyninstRPC::mdl_v_expr::mdl_v_expr(u_int u_op, T_dyninstRPC::mdl_expr *expr,
				     bool is_preop)
: int_literal_(0), bin_op_(0), u_op_(u_op), left_(expr), right_(NULL), 
  args_(NULL), do_type_walk_(false), ok_(false) 
{ 
  if (is_preop)
    type_ = MDL_EXPR_PREUOP;
  else
    type_ = MDL_EXPR_POSTUOP;
}

T_dyninstRPC::mdl_v_expr::mdl_v_expr(string var, T_dyninstRPC::mdl_expr* index_expr) 
: type_(MDL_EXPR_INDEX), int_literal_(0), var_(var), bin_op_(0),
  u_op_(0), left_(index_expr), right_(NULL), args_(NULL),
  do_type_walk_(false), ok_(false)
{ }

T_dyninstRPC::mdl_v_expr::~mdl_v_expr() 
{
  if (args_) 
  {
    unsigned size = args_->size();
    for (unsigned u=0; u<size; u++)
      delete (*args_)[u];
    delete args_;
  }
  delete left_; delete right_;
}

bool T_dyninstRPC::mdl_v_expr::apply(AstNode*&)
{
  switch (type_) 
  {
    case MDL_EXPR_INT: 
    case MDL_EXPR_STRING:
      ok_ = true;
      return ok_;
    case MDL_EXPR_INDEX:
    {
      if (var_ == string("$arg"))
      {
        mdl_var temp(false);
        if (!left_->apply(temp))
          return false;
        int x;
        if (!temp.get(x)) return false;
        ok_ = true;
        return ok_;
      }
      else if (var_ == string("$constraint"))
      {
        mdl_var temp(false);
        if (!left_->apply(temp))
          return false;
        int x;
        if (!temp.get(x))
          return false;
        ok_ = true;
        return ok_;
      }
      mdl_var array(false);
      if (!mdl_env::get(array, var_)) return false;
      if (!array.is_list()) return false;  
      mdl_var temp (false);
      if (!left_->apply (temp)) return false;
      int x;
      if (!temp.get(x)) return false;
      ok_ = true;
      return ok_;
    }
    case MDL_EXPR_BINOP:
    {
      if (!left_ || !right_) return false;
      AstNode *lnode, *rnode;
      if (!left_->apply (lnode)) return false;
      if (!right_->apply (rnode)) return false;
      ok_ =  true;
      return ok_;
    }
    case MDL_EXPR_FUNC:
    {
      if (var_ == string("startWallTimer") || var_ == string("stopWallTimer")
      || var_ == string("startProcessTimer") || var_ == string("stopProcessTimer")
      || var_ == string("startProcessTimer_lwp"))
      {
        if (!args_) return false;
        unsigned size = args_->size();
        if (size != 1) return false;

        mdl_var timer(false);
        if (!(*args_)[0]->apply(timer)) return false;
        if (timer.type() != MDL_T_WALL_TIMER 
          && timer.type() != MDL_T_PROC_TIMER)
        {
          cerr << "operand of timer operation is not a timer." << endl;
          return false;
        }
        ok_ = true;
      }
      else if (var_ == string("readSymbol") || var_ == string("readAddress"))
        ok_ = true;
      else
      {
        AstNode *tmparg=NULL;
        for (unsigned u = 0; u < args_->size(); u++) 
        {
          if (!(*args_)[u]->apply(tmparg)) 
          {
            ok_ = false; break;
          }
        }
        ok_ = true;
      }
      return ok_;
    }
    case MDL_EXPR_DOT:
    {
      mdl_var dot_var;
      if (!left_->apply(dot_var))
      {
        cerr << "Invalid expression on the left of DOT." << endl;
        return false;
      }
      if (dot_var.get_type() != MDL_T_VARIABLE)
      {
        cerr << "Invalid expression type on the left of DOT." << endl;
        return false;
      }
      ok_ = true;
      return ok_;
    }
    case MDL_EXPR_ASSIGN:
    {
      mdl_var lvar;
      if (!mdl_env::get(lvar, var_))
      {
        cerr << "variable '" << var_ 
          << "' undeclared on the left hand side of the assignment."
          << endl;
        return false;
      }
      if (lvar.type() != MDL_T_COUNTER)
      {
        cerr << "variable '" << var_ << "' must be a counter." << endl;
        return false;
      }

      AstNode* ast_arg;
      if (!left_->apply(ast_arg))
      {
        cerr << "Invalid right hand side of assignment." << endl;
        return false;
      }
      ok_ = true;
      return ok_;
    }
    case MDL_EXPR_VAR:
    {
      if (var_==string("$cmin")||var_==string("$cmax")||var_==string("$return"))
      {
        ok_ = true; return true;
      }
      mdl_var get_drn;
      assert (mdl_env::get(get_drn, var_));
      switch (get_drn.type())
      {
        case MDL_T_INT:
        case MDL_T_COUNTER:
        case MDL_T_PROC_TIMER:
        case MDL_T_WALL_TIMER:
        case MDL_T_HW_TIMER:
        case MDL_T_HW_COUNTER:
        case MDL_T_DATANODE:
          ok_ = true;
          return true;
        default:
          cerr << "type of variable " << var_.c_str()
            << " is not known" << endl;
          return false;
      }
    }
    case MDL_EXPR_PREUOP:
    {
      switch (u_op_)
      {
        case MDL_ADDRESS:
        {
          mdl_var get_drn;
          if (!left_->apply(get_drn)) return false;
          instrDataNode *drn;
          if (!get_drn.get(drn)) return false;
          break;
        }
        case MDL_MINUS:
        {
          mdl_var tmp;
          if (!left_->apply (tmp)) return false;
          int value;
          if (!tmp.get(value)) return false;
          break;
        }
        default:
          return false;
      }
      ok_ = true;
      return true;
    }
    case MDL_EXPR_POSTUOP:
    {
      switch (u_op_)
      {
        case MDL_PLUSPLUS:
        {
          mdl_var tmp;
          if (!left_->apply (tmp)) return false;
          if (tmp.type() != MDL_T_COUNTER) return false;
          break;
        }
        default: return false;
      }
      ok_ = true;
      return true;
    }
    default:
      return false;
  }
  return true;
}

bool T_dyninstRPC::mdl_v_expr::apply(mdl_var& ret) 
{
  switch (type_) 
  {
    case MDL_EXPR_INT: 
      ok_ = ret.set(int_literal_);
      return ok_;
    case MDL_EXPR_STRING:
      ok_ = ret.set(str_literal_);
      return ok_;
    case MDL_EXPR_INDEX:
    {
      if (var_ == string("$arg"))
      {
        // we only allow $arg to appear inside the icode. is this
        // correct? -chun.
        assert(0);
      }
      else if (var_ == string("$constraint"))
      {
        mdl_var temp(false);
        if (!left_->apply(temp))
          return false;
        int x;
        if (!temp.get(x))
          return false;
        ok_ = mdl_env::get(ret, var_+string(x));
        return ok_;
      }
      mdl_var array(false);
      if (!mdl_env::get(array, var_)) return false;
      if (!array.is_list()) return false;  
      mdl_var temp (false);
      if (!left_->apply (temp)) return false;
      int x;
      if (!temp.get(x)) return false;
      ok_ = array.get_ith_element(ret, x); //set_type(array.element_type());
      return ok_;
    }
    case MDL_EXPR_BINOP:
    {
      mdl_var left_val(false), right_val(false);
      if (!left_ || !right_) return false;
      if (!left_->apply(left_val) || !right_->apply(right_val))
        return false;
      ok_ =  do_operation(ret, left_val, right_val, bin_op_);
      return ok_;
    }
    case MDL_EXPR_ASSIGN:
    {
      // we only support the assignment of an expression to a variable
      mdl_var lval(false), rval(false);
      if (!mdl_env::get(lval, var_)) return false;
      if (!left_ || !left_->apply(rval))
        return false;
      if (rval.type() == MDL_T_NONE)
      {
        ok_ = true;
        return ok_;
      }
      switch (bin_op_)
      {
        case MDL_ASSIGN:
          ok_ = true; break;
        case MDL_PLUSASSIGN:
          ok_ = do_operation(ret, lval, rval, MDL_PLUS); break;
        case MDL_MINUSASSIGN:
          ok_ = do_operation(ret, lval, rval, MDL_MINUS); break;
        default:
          ok_ = false; break;
      }
      return ok_;
    }
    case MDL_EXPR_FUNC:
    {
      unsigned arg_size = 0;
      if (args_)
        arg_size = args_->size();

      if (var_ == string("startWallTimer") || var_ == string("stopWallTimer"))
      {
        assert (0);
        ok_ = false;
      }
      else if (var_ == string("startProcessTimer") || var_ == string("stopProcessTimer") ||
               var_ == string("startProcessTimer_lwp"))
      {
        assert (0);
        ok_ = false;
      }
      else
      {
        mdl_var temp(false);
        for (unsigned u = 0; u < arg_size; u++)
          if (!(*args_)[u]->apply(temp))
            return false;
        ok_ = true;
      }
      return ok_;
    }
    case MDL_EXPR_DOT:
    {
      if (!fields_.size()) 
      {
        cout << "Error: empty fields followed by '.'" << endl;
        return false;
      }
      else 
      {
        // build type_walk here
        do_type_walk_ = true;
        mdl_var temp(false);
        if (!left_->apply(temp)) return false;

        //
        // check if all the fields are valid.  for example, the only 
        // fields MDL_T_VARIABLEs can have are "upper" or "lower".
        // these fields are registered in mdl_init().  -chun.
        //

        vector<mdl_type_desc> acceptable_fdlst;
        unsigned v_index = 0, v_max = fields_.size();
        unsigned current_type = temp.type();

        while (v_index < v_max) 
        {
          if (!mdl_data::fields.defines(current_type))
          {
            cerr << "Invalid field type." << endl;
            return false;
          }
          acceptable_fdlst = mdl_data::fields[current_type];
          string next_field = fields_[v_index];
          bool type_found=false;
          unsigned size = acceptable_fdlst.size();

          for (unsigned u=0; u<size; u++) {
            if (acceptable_fdlst[u].name == next_field) 
            {
              type_found = true;
              type_walk += current_type;
              type_walk += u;
              current_type = acceptable_fdlst[u].type;
              break;
            }
	  }
          if (!type_found)
          {
            cerr << "Invalid field type." << endl;
            return false; 
          }
          v_index++;
        }
        ret.set_type(current_type);
        ok_ = true;
        return ok_;
      }
    }
    case MDL_EXPR_VAR:
    {
      if (var_==string("$cmin")||var_==string("$cmax")||var_==string("$return"))
        ok_ = true;
      else
        ok_ = mdl_env::get (ret, var_);
      return ok_;
    }
    case MDL_EXPR_PREUOP:
    {
      mdl_var lval(false);
      if (!left_ || !left_->apply (lval)) return false;
      ok_ = do_operation(ret, lval, u_op_, true);
      return ok_;
    }
    case MDL_EXPR_POSTUOP:
    {
      mdl_var lval(false);
      if (!left_ || !left_->apply (lval)) return false;
      ok_ = do_operation(ret, lval, u_op_, false);
      return ok_;
    }
    default:
      return false;
  }
  return false;
}

T_dyninstRPC::mdl_if_stmt::mdl_if_stmt(T_dyninstRPC::mdl_expr *expr, 
				       T_dyninstRPC::mdl_stmt *body) 
: expr_(expr), body_(body) { }

T_dyninstRPC::mdl_if_stmt::mdl_if_stmt()
: expr_(NULL), body_(NULL) { }

T_dyninstRPC::mdl_if_stmt::~mdl_if_stmt() {
  delete expr_; delete body_;
}

bool T_dyninstRPC::mdl_if_stmt::apply(instrCodeNode * ,
				      vector<const instrDataNode*>& flags) {
  mdl_var res(false); int iv;
  if (!expr_->apply(res))
    return false;
  switch (res.get_type()) {
  case MDL_T_INT:
    if (!res.get(iv))
      return false;
    break;
  default:
    return false;
  }
  bool ret = body_->apply(NULL, flags);
  return ret;
}

T_dyninstRPC::mdl_seq_stmt::mdl_seq_stmt(vector<T_dyninstRPC::mdl_stmt*> *stmts) 
: stmts_(stmts) { }

T_dyninstRPC::mdl_seq_stmt::mdl_seq_stmt()
: stmts_(NULL) { }

T_dyninstRPC::mdl_seq_stmt::~mdl_seq_stmt() {
  if (stmts_) {
    unsigned size = stmts_->size();
    for (unsigned u=0; u<size; u++)
      delete (*stmts_)[u];
    delete stmts_;
  }
}
 
bool T_dyninstRPC::mdl_seq_stmt::apply(instrCodeNode * ,
				       vector<const instrDataNode*>& flags) {
  if (!stmts_)
    return true;
  unsigned size = stmts_->size();
  for (unsigned index=0; index<size; index++)
    if (!(*stmts_)[index]->apply(NULL, flags))
      return false;
  return true;
}

T_dyninstRPC::mdl_instr_stmt::mdl_instr_stmt(unsigned pos, 
					     T_dyninstRPC::mdl_expr *expr,
					     vector<T_dyninstRPC::mdl_icode*> *reqs,
					     unsigned where, bool constrained) 
: position_(pos), point_expr_(expr), icode_reqs_(reqs),
  where_instr_(where), constrained_(constrained) { }

T_dyninstRPC::mdl_instr_stmt::mdl_instr_stmt()
: position_(0), point_expr_(NULL), icode_reqs_(NULL),
  where_instr_(0), constrained_(false) { }

T_dyninstRPC::mdl_instr_stmt::~mdl_instr_stmt() {
  delete point_expr_;
  if (icode_reqs_) {
    unsigned size = icode_reqs_->size();
    for (unsigned u=0; u<size; u++)
      delete (*icode_reqs_)[u];
    delete icode_reqs_;
  }
}

bool T_dyninstRPC::mdl_instr_stmt::apply(instrCodeNode * ,
					 vector<const instrDataNode*>& ) {
  mdl_var temp(false);
  if (!icode_reqs_)
    return false;
  if (!point_expr_->apply(temp))
    return false;

  // It is illegal to add postInsn to a function exit point
  if (temp.get_type() == MDL_T_POINT_RETURN) {
    if (where_instr_ == MDL_POST_INSN) {
      cout << "MDL error: you can't insert postInsn at a procedure exit point." << endl;
      return false;
    }
    temp.set_type(MDL_T_POINT);
  }

  if (temp.get_type() != MDL_T_POINT)
    return false;
  instPoint *p;
  if (!temp.get(p))
    return false;
  unsigned size = icode_reqs_->size();

  switch (position_) {
  case MDL_PREPEND:
  case MDL_APPEND:
    break;
  default:
    return false;
  }

  switch (where_instr_) {
  case MDL_PRE_INSN:
  case MDL_POST_INSN:
    break;
  default:
    return false;
  }

  AstNode *an=NULL;
  for (unsigned u=0; u<size; u++)
    if (!(*icode_reqs_)[u]->apply(an, u, NULL))
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
  kid.name = "Message"; kid.type = MDL_T_INT; kid.end_allowed = true; kids += kid;
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

  self.name = "Memory"; self.type = MDL_T_STRING; self.end_allowed = true;
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
  desc.name = "return"; desc.type = MDL_T_POINT_RETURN; field_list += desc;
  //desc.name = "tag"; desc.type = MDL_T_INT; field_list += desc;
  mdl_data::fields[MDL_T_PROCEDURE] = field_list;
  mdl_data::fields[MDL_T_PROCEDURE_NAME] = field_list;
  field_list.resize(0);

  desc.name = "name"; desc.type = MDL_T_STRING; field_list += desc;
  desc.name = "funcs"; desc.type = MDL_T_LIST_PROCEDURE; field_list += desc;
  mdl_data::fields[MDL_T_MODULE] = field_list;
  field_list.resize(0);

  desc.name = "upper"; desc.type = MDL_T_INT; field_list += desc;
  desc.name = "lower"; desc.type = MDL_T_INT; field_list += desc;
  mdl_data::fields[MDL_T_VARIABLE] = field_list;
  field_list.resize(0);

  return true;
}

// 970930 - changed mdl ehanged code exclusion to Code/module or 
//  Code/module/func instead of  module/func or module.
// check that exclude_node strings have form "module" or "module/func" only
bool mdl_check_node_constraints_OLD() {

    for(u_int i=0; i < mdl_data::lib_constraints.size(); i++) {
        char *temp = P_strdup(mdl_data::lib_constraints[i].c_str()); 
	char *where = 0;
	bool bad_string = false;
	if(temp && (where = P_strchr(temp,'/'))) {
	    char *where2=0;
	    if(where == temp) { 
		bad_string = true; // first character is '/' 
	    }
	    else if((where2=P_strrchr(temp,'/')) && where2 != where ){ 
		bad_string = true; // there is more than one '/' in the string
	    }
	    else if (temp[mdl_data::lib_constraints[i].length()-1] == '/'){
		bad_string = true; // last character is '/' 
	    }
	}
	if(temp) delete temp;
	if(bad_string) {
	    cout << "parse error: exclude_node " <<
		mdl_data::lib_constraints[i].c_str() << endl;
	    cout << "should be: `exclude_node \"module/function\";'  or `exclude_node \"module\";'" << endl;
            return false;
	}
    }
    return true;
}

// 970930 - changed mdl ehanged code exclusion to /Code/module or 
//  /Code/module/func instead of  module/func or module.
//  In addition, modified mdl_data::lib_constraints to by
//  stripping off the leading /Code/ from each entry.  Thus,
//  after returning (with val == true) mdl_data::lib_constraints
//  should have entries of the old form: "module", or "module/func"....
// check that exclude_node strings have form "module" or "module/func" only
bool mdl_check_node_constraints() {
    unsigned int i;
    // locations of first, second, and 3rd "/" characters....
    char *temp, *first_slash, *second_slash, *third_slash, *fourth_slash;
    string modified_constraint;
    bool bad_string;

    first_slash = second_slash = third_slash = fourth_slash = NULL;

    // temp vector to hold lib constraints with leading 
    //  "Code/" stripped off....
    vector<string> modified_lib_constraints;

    for(i=0;i<mdl_data::lib_constraints.size(); i++) {
        first_slash = second_slash = third_slash = NULL;

        bad_string = 0;
        // get copy of string char *data for strchr....
        temp = P_strdup(mdl_data::lib_constraints[i].c_str());
	// Doh!!!!  Changed exclude directive to have form /Code,
	//  instead of Code/, so strip off leading '/'
	first_slash = P_strchr(temp, RH_SEPARATOR);
	if (first_slash != temp) {
	    bad_string = 1;
	    first_slash = NULL;
	} 
        // Now that the leading "/" is stripped off, 
	// stript everything before new first slash off of constraint....
	// This should place constraint in form which system previously
	// expected....
	if (first_slash != NULL) {
	    second_slash = P_strchr(&first_slash[1], RH_SEPARATOR);
	}
	if (second_slash != NULL) {
	    modified_constraint = string(&second_slash[1]);
	    third_slash = P_strchr(&second_slash[1], RH_SEPARATOR);
	}
	if (third_slash != NULL) {
	    fourth_slash = P_strchr(&third_slash[1], RH_SEPARATOR);
	}
	// excluded item should have at least 2 "/", e.g.
	//  "/Code/module", or "/Code/module/func"....
	if (first_slash == NULL || second_slash == NULL) {
	    bad_string = 1; 
	    cerr << "exclude syntax : could not find 2 separators"
                    " in resource hierarchy path" << endl;
	}
	// and at most most 2....
	if (fourth_slash != NULL) {
	    bad_string = 1;
	    cerr << "exclude syntax : found too many separators"
                    " in resource hierarchy path" << endl;
	}
	if (bad_string != 1) {
	    // the substring between the first and second slash 
	    //  should be "Code"
	    if (strncmp(CODE_RH_NAME, &first_slash[1], second_slash - \
		    first_slash - 1) != 0) {
	      cerr << "exclude syntax : top level resource hierarchy path must be " << CODE_RH_NAME << endl;;
	        bad_string = 1;
	    }
	}
	// dont forget to free up temp....
	if (temp != NULL) delete []temp;
	
	if (bad_string) {
	    cout << "exclude syntax error : " << mdl_data::lib_constraints[i].c_str() << " is not of expected form : /Code/module/function, or /Code/module" << endl;
	    return false;
	} 
	
	modified_lib_constraints += modified_constraint;
    }

    mdl_data::lib_constraints = modified_lib_constraints;
    return true;
}

bool mdl_apply() {
  //
  // apply resource list statements
  //
  unsigned size = mdl_data::stmts.size();
  vector<const instrDataNode*> flags;
  for (unsigned u=0; u<size; u++)
    if (!mdl_data::stmts[u]->apply(NULL, flags))
      return false;

  //
  // apply constraints
  //
  vector<T_dyninstRPC::mdl_constraint*> ok_cons;
  size = mdl_data::all_constraints.size();
  for (unsigned u1=0; u1<size; u1++) {
    instrDataNode *drn = NULL;
    Hierarchy res;
    if (mdl_data::all_constraints[u1]->apply(NULL, &drn, res, NULL, false)) {
      ok_cons += mdl_data::all_constraints[u1];
      // cout << "constraint defined: " << mdl_data::all_constraints[u1]->id_ << endl;
    } else {
      cerr << "Error in constraint '" << mdl_data::all_constraints[u1]->id_ << 
	   "'. Constraint deleted." << endl;
      delete mdl_data::all_constraints[u1];
    }
  }
  mdl_data::all_constraints = ok_cons;

  //
  // apply metrics
  //
  Focus focus;
  string empty;
  vector<pd_process*> emptyP;

  vector<T_dyninstRPC::mdl_metric*> ok_mets;
  size = mdl_data::all_metrics.size();
  for (unsigned u2=0; u2<size; u2++) {
    vector<processMetFocusNode *> nodes;
    if (mdl_data::all_metrics[u2]->apply(&nodes, focus, emptyP, false,false)
	== true)
    {
      ok_mets += mdl_data::all_metrics[u2];
      // cout << "metric defined: " << mdl_data::all_metrics[u2]->id_ << endl;
    } else {
      cerr << "Error in metric '" << mdl_data::all_metrics[u2]->id_ << 
	   "'. Metric deleted." << endl;
      delete mdl_data::all_metrics[u2];
    }
  }
  mdl_data::all_metrics = ok_mets;
  
  return true;
}

bool mdl_send(dynRPCUser *remote) {
  remote->send_stmts(&mdl_data::stmts);
  remote->send_constraints(&mdl_data::all_constraints);
  remote->send_metrics(&mdl_data::all_metrics);
  if(mdl_data::lib_constraints.size()){
      remote->send_libs(&mdl_data::lib_constraints);
  }
  else {
      remote->send_no_libs();
  }

  return true;
}


bool mdl_get_lib_constraints(vector<string> &lc, vector<unsigned> &lcf){
    for(u_int i=0; i < mdl_data::lib_constraints.size(); i++){
        lc += mdl_data::lib_constraints[i];
		lcf += mdl_data::lib_constraint_flags[i];
    }
    return (lc.size()>0);
}

void mdl_destroy() {
  unsigned size = mdl_data::all_constraints.size();
  for (unsigned u=0; u<size; u++)
    delete (mdl_data::all_constraints[u]);
  mdl_data::all_constraints.resize(0);
  
  size = mdl_data::all_metrics.size();
  for (unsigned u1=0; u1<size; u1++)
    delete (mdl_data::all_metrics[u1]);
  mdl_data::all_metrics.resize(0);

  size = mdl_data::stmts.size();
  for (unsigned u2=0; u2<size; u2++)
    delete mdl_data::stmts[u2];
}

static bool do_operation(mdl_var& ret, mdl_var& left_val, mdl_var& right_val, 
  unsigned bin_op) 
{
  int ltype = left_val.get_type();
  int rtype = right_val.get_type();

  if (ltype == MDL_T_NONE || rtype == MDL_T_NONE)
    return true;

  switch (bin_op) 
  {
  case MDL_PLUS:
  case MDL_MINUS:
  case MDL_DIV:
  case MDL_MULT:
  {
    if ((ltype == MDL_T_INT) && (rtype == MDL_T_INT)) 
    {
      int v1, v2;
      if (!left_val.get(v1)) return false;
      if (!right_val.get(v2)) return false;
      switch (bin_op) 
      {
      case MDL_PLUS: return (ret.set(v1+v2));
      case MDL_MINUS: return (ret.set(v1-v2));
      case MDL_DIV: return (ret.set(v1/v2));
      case MDL_MULT: return (ret.set(v1*v2));
      }
    } 
    else if (((ltype == MDL_T_INT)||(ltype == MDL_T_FLOAT)) 
      && ((rtype == MDL_T_INT) || (rtype == MDL_T_FLOAT)))
    {
      float v1, v2;
      if (ltype == MDL_T_INT) 
      {
        int i1;
        if (!left_val.get(i1)) return false; 
        v1 = i1;
      }
      else 
      {
        if (!left_val.get(v1)) return false;
      }

      if (rtype == MDL_T_INT) 
      {
        int i1; if (!right_val.get(i1)) return false; v2 = i1;
      }
      else 
      {
        if (!right_val.get(v2)) return false;
      }
      switch (bin_op) 
      {
      case MDL_PLUS: return (ret.set(v1+v2));
      case MDL_MINUS: return (ret.set(v1-v2));
      case MDL_DIV: return (ret.set(v1/v2));
      case MDL_MULT: return (ret.set(v1*v2));
      }
    }
    else
      return false;
  }
  case MDL_LT:
  case MDL_GT:
  case MDL_LE:
  case MDL_GE:
  case MDL_EQ:
  case MDL_NE:
  {
    if ((ltype == MDL_T_STRING) && (rtype == MDL_T_STRING))
    {
      return ret.set(1); 
    }
    if ((ltype == MDL_T_INT) && (rtype == MDL_T_INT)) 
    {
      int v1, v2;
      if (!left_val.get(v1)) return false;
      if (!right_val.get(v2)) return false;
      switch (bin_op) 
      {
      case MDL_LT: return (ret.set(v1 < v2));
      case MDL_GT: return (ret.set(v1 > v2));
      case MDL_LE: return (ret.set(v1 <= v2));
      case MDL_GE: return (ret.set(v1 >= v2));
      case MDL_EQ: return (ret.set(v1 == v2));
      case MDL_NE: return (ret.set(v1 != v2));
      }
    }
    else if (((ltype == MDL_T_INT)||(ltype == MDL_T_FLOAT))
      && ((rtype == MDL_T_INT) || (rtype == MDL_T_FLOAT)))    {
      float v1, v2;
      if (ltype == MDL_T_INT) 
      {
        int i1; if (!left_val.get(i1)) return false; v1 = i1;
      }
      else 
      {
        if (!left_val.get(v1)) return false;
      }
      if (rtype == MDL_T_INT) 
      {
        int i1; if (!right_val.get(i1)) return false; v2 = i1;
      }
      else 
      {
        if (!right_val.get(v2)) return false;
      }
      switch (bin_op) 
      {
      case MDL_LT: return (ret.set(v1 < v2));
      case MDL_GT: return (ret.set(v1 > v2));
      case MDL_LE: return (ret.set(v1 <= v2));
      case MDL_GE: return (ret.set(v1 >= v2));
      case MDL_EQ: return (ret.set(v1 == v2));
      case MDL_NE: return (ret.set(v1 != v2));
      }
    }
    else
      return false;
  }
  case MDL_AND:
  case MDL_OR:
  {
    if ((ltype == MDL_T_INT) && (rtype == MDL_T_INT)) 
    {
      int v1, v2;
      if (!left_val.get(v1)) return false;
      if (!right_val.get(v2)) return false;
      switch (bin_op) 
      {
      case MDL_AND: return (ret.set(v1 && v2));
      case MDL_OR: return (ret.set(v1 || v2));
      }
    }
    else
      return false;
  }
  default:
      return false;
  }
  return false;
}

static bool do_operation(mdl_var& ret, mdl_var& lval, unsigned u_op, bool/*is_preop*/) 
{
  switch (u_op) 
  {
    case MDL_PLUSPLUS:
    {
      if (lval.get_type() == MDL_T_INT)
      {
        int x;
        if (!lval.get(x)) return false;
        return (ret.set(x++));
      }
      else
      {
        cerr << "Invalid type for operator ++" << endl;
        return false;
      }
    }
    case MDL_MINUS:
    {
      if (lval.get_type() == MDL_T_INT)
      {
        int x;
        if (!lval.get(x)) return false;
        return ret.set(-x);
      }
      else
      {
        cerr << "Invalid type for operator -" << endl;
        return false;
      }
    }
    case MDL_ADDRESS:
      return true;
    case MDL_NOT:
    default:
      return false;
  }
  return false;
}

bool T_dyninstRPC::mdl_list_stmt::mk_list(vector<string> &) {return true;}
bool T_dyninstRPC::mdl_for_stmt::mk_list(vector<string> &) {return true;}
bool T_dyninstRPC::mdl_if_stmt::mk_list(vector<string> &) {return true;}
bool T_dyninstRPC::mdl_seq_stmt::mk_list(vector<string> &) {return true;}
bool T_dyninstRPC::mdl_instr_stmt::mk_list(vector<string> &) {return true;}
bool T_dyninstRPC::mdl_constraint::mk_list(vector<string> &) {return true;}
bool T_dyninstRPC::mdl_v_expr::mk_list(vector<string> &) {return true;}

