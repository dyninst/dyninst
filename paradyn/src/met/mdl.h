
#ifndef MDL_EXTRA_H
#define MDL_EXTRA_H

/*
 * Parse.h - define the classes that are used in parsing an interface.
 */

#include "util/h/String.h"
#include "util/h/Vector.h"
#include "util/h/Dictionary.h"
#include <iostream.h>
#include <fstream.h>
#include "util/h/aggregation.h"

#define MDL_FOLD_SUM 0
#define MDL_FOLD_AVG 1

#define MDL_AGG_SUM aggSum
#define MDL_AGG_AVG aggMin
#define MDL_AGG_MIN aggMax
#define MDL_AGG_MAX aggAvg

#define MDL_T_SCALAR_BASE 30
#define MDL_T_INT 30
#define MDL_T_FLOAT 31
#define MDL_T_STRING 32
#define MDL_T_BOOL 33
#define MDL_T_POINT 34
#define MDL_T_PROCEDURE 35
#define MDL_T_MODULE 36
#define MDL_T_CALLSITE 37
#define MDL_T_COUNTER 38
#define MDL_T_PROC_TIMER 39
#define MDL_T_WALL_TIMER 40
#define MDL_T_PROCESS 41
#define MDL_T_DRN     42
#define MDL_T_SCALAR_END 42

#define MDL_T_LIST_BASE 42
#define MDL_T_LIST_INT 42
#define MDL_T_LIST_FLOAT 43
#define MDL_T_LIST_STRING 44
#define MDL_T_LIST_PROCEDURE 45
#define MDL_T_LIST_MODULE 46
#define MDL_T_LIST_POINT 47
#define MDL_T_LIST_END 47

#define MDL_T_NONE 50

#define MDL_PREPEND 60
#define MDL_APPEND  61

#define MDL_RETURN 110
#define MDL_ENTER 111
#define MDL_SELF 112
#define MDL_CALLSITE 113
#define MDL_ARG 114

#define MDL_PLUS 120
#define MDL_MINUS 121
#define MDL_DIV 122
#define MDL_MULT 123
#define MDL_LT 124
#define MDL_GT 125
#define MDL_LE 126
#define MDL_GE 127
#define MDL_EQ 128
#define MDL_NE 129
#define MDL_AND 130
#define MDL_OR 131
#define MDL_NOT 132

#define MDL_SET_COUNTER 200
#define MDL_ADD_COUNTER 201
#define MDL_SUB_COUNTER 202
#define MDL_START_WALL_TIMER 203
#define MDL_STOP_WALL_TIMER 204
#define MDL_START_PROC_TIMER 205
#define MDL_STOP_PROC_TIMER 206
#define MDL_READ_ADDRESS 207
#define MDL_READ_SYMBOL 208
#define MDL_CALL_FUNC 209
#define MDL_CALL_FUNC_COUNTER 210

#define MDL_PRE_INSN   300
#define MDL_POST_INSN  301

#define MDL_RVAL_INT 400
#define MDL_RVAL_STRING 401
#define MDL_RVAL_ARRAY 402
#define MDL_RVAL_EXPR 403
#define MDL_RVAL_FUNC 404
#define MDL_RVAL_DEREF 405

#define MDL_RES_SYNCOBJECT 500
#define MDL_RES_CODE       501
#define MDL_RES_PROCESS    502
#define MDL_RES_MACHINE    503

#define MDL_T_CONSTRAINT   600

typedef struct {
  unsigned type;
  string name;
  bool end_allowed;
} mdl_type_desc;

typedef struct {
  mdl_type_desc self;
  vector<mdl_type_desc> kids;
} mdl_focus_element;

inline string instr_req_to_string(unsigned type, string obj) {

  switch(type) {
  case MDL_SET_COUNTER:
    return (string("setCounter(") + obj + ", "); 
  case MDL_ADD_COUNTER:
    return (string("addCounter(") + obj + ", ");
  case MDL_SUB_COUNTER:
    return (string("subCounter(") + obj + ", ");
  case MDL_START_WALL_TIMER:
    return (string("startWallTimer(") + obj + "); ");
  case MDL_STOP_WALL_TIMER:
    return (string("stopWallTimer(") + obj + "); ");
  case MDL_START_PROC_TIMER:
    return (string("startProcTimer(") + obj + "); ");
  case MDL_STOP_PROC_TIMER:
    return (string("stopProcTimer(") + obj + "); ");
  }
}

inline string op_to_string(unsigned op) {
  switch(op) {
  case MDL_PLUS:
    return " + ";
  case MDL_MINUS:
    return " - ";
  case MDL_DIV:
    return " / ";
  case MDL_MULT:
    return " * ";
  case MDL_LT:
    return " < n";
  case MDL_GT:
    return " > ";
  case MDL_LE:
    return " <= ";
  case MDL_GE:
    return " >= ";
  case MDL_EQ:
    return " == ";
  case MDL_NE:
    return " != ";
  case MDL_AND:
    return " && ";
  case MDL_OR:
    return " || ";
  case MDL_NOT:
    return " ! ";
  }
}

inline string fold_to_string(unsigned f) {
  switch (f) {
  case MDL_FOLD_AVG: return ("avg");
  case MDL_FOLD_SUM: return ("sum");
  default: return ("error");
  }
}

inline string agg_to_string(unsigned f) {
  switch (f) {
  case MDL_AGG_AVG: return ("avg");
  case MDL_AGG_SUM: return ("sum");
  case MDL_AGG_MAX: return ("max");
  case MDL_AGG_MIN: return ("min");
  default: return ("error");
  }
}

#if defined(PARADYN)
class process { };
class pdFunction { };
class module { };
class instPoint { };
class dataReqNode { };
class metricDefinitionNode { };
class AstNode { };
#else
// #include "paradynd/src/symtab.h"
// #include "paradynd/src/ast.h"
class process;
class pdFunction;
class module;
class instPoint;
class dataReqNode;
class metricDefinitionNode;
class AstNode;
#endif

class mdl_var;

class mdl_var {
public:

  inline mdl_var(bool is_remote=false);
  inline mdl_var(string &, bool is_remote);
  inline ~mdl_var();
  inline void destroy();

  inline void dump();

  inline mdl_var(string& nm, int i, bool is_remote);
  inline mdl_var(string& nm, float f, bool is_remote);
  inline mdl_var(string& nm, instPoint *p, bool is_remote);
  inline mdl_var(string& nm, pdFunction *pr, bool is_remote);
  inline mdl_var(string& nm, module *mod, bool is_remote);
  inline mdl_var(string& nm, string& s, bool is_remote);
  inline mdl_var(string& nm, process *pr, bool is_remote);
  inline mdl_var(string& nm, vector<pdFunction*> *vp, bool is_remote);
  inline mdl_var(string& nm, vector<module*> *vm, bool is_remote);
  inline mdl_var(string& nm, vector<int> *vi, bool is_remote);
  inline mdl_var(string& nm, vector<float> *vf, bool is_remote);
  inline mdl_var(string& nm, vector<string> *vs, bool is_remote);
  inline mdl_var(string& nm, dataReqNode *drn, bool is_remote);
  inline mdl_var(string& nm, vector<instPoint*> *ipl, bool is_remote);

  inline bool get(int &i);
  inline bool get(float &f);
  inline bool get(instPoint *&p);
  inline bool get(pdFunction *&pr);
  inline bool get(module *&mod);
  inline bool get(string& s);
  inline bool get(process *&pr);
  inline bool get(vector<pdFunction*> *&vp);
  inline bool get(vector<module*> *&vm);
  inline bool get(vector<int> *&vi);
  inline bool get(vector<float> *&vf);
  inline bool get(vector<string> *&vs);
  inline bool get(dataReqNode *&drn);
  inline bool get(vector<instPoint*> *&vip);

  inline bool set(int i);
  inline bool set(float f);
  inline bool set(instPoint *p);
  inline bool set(pdFunction *pr);
  inline bool set(module *mod);
  inline bool set(string& s);
  inline bool set(process *pr);
  inline bool set(vector<pdFunction*> *vp);
  inline bool set(vector<module*> *vm);
  inline bool set(vector<int> *vi);
  inline bool set(vector<float> *vf);
  inline bool set(vector<string> *vs);
  inline bool set(vector<bool> *vb);
  inline bool set(dataReqNode *drn);
  inline bool set(vector<instPoint*> *ipl);

  inline void set_type(unsigned type);
  inline bool is_list() const;
  inline unsigned element_type() const;
  inline unsigned list_size() const;
  inline bool get_ith_element(mdl_var &ret, unsigned ith) const;
  inline unsigned as_list();
  inline bool make_list(unsigned type);
  inline bool add_to_list(mdl_var& add_me);
  
  inline string name() const;
  inline unsigned type() const;
  inline bool remote() const;

private:
  union mdl_types {
    int i;
    float f;
    instPoint *point_;
    pdFunction *pr;
    process *the_process;
    module *mod;
    vector<pdFunction*>  *list_pr;
    vector<int>          *list_int;
    vector<float>        *list_float;
    vector<string>       *list_string;
    vector<module*>      *list_module;
    vector<instPoint*>   *list_pts;
    dataReqNode          *drn;
    void *ptr;
  };

  string name_;
  unsigned type_;
  mdl_types vals;
  string string_val;
  bool remote_;
};

class mdl_env {
public:
  static inline void dump();
  static inline void push();
  static inline bool pop();
  static inline bool add(mdl_var& value);
  static inline bool add(string& var_name, bool is_remote, unsigned type=MDL_T_NONE);
  static inline bool get(mdl_var& ret, string& var_name);
  static inline bool set(mdl_var& value, string& var_name);

  static inline bool set(int i, string& var_name);
  static inline bool set(float f, string& var_name);
  static inline bool set(instPoint *p, string& var_name);
  static inline bool set(pdFunction *pr, string& var_name);
  static inline bool set(module *mod, string& var_name);
  static inline bool set(string& s, string& var_name);
  static inline bool set(process *pr, string& var_name);
  static inline bool set(vector<pdFunction*> *vp, string& var_name);
  static inline bool set(vector<module*> *vm, string& var_name);
  static inline bool set(vector<int> *vi, string& var_name);
  static inline bool set(vector<float> *vf, string& var_name);
  static inline bool set(vector<string> *vs, string& var_name);
  static inline bool set(vector<instPoint*> *vip, string& var_name);
  static inline bool set(dataReqNode *drn, string& var_name);

  static inline bool set_type(unsigned type, string& var_name);
  static inline bool type(unsigned &ret, string& var_name);
  static inline bool is_remote(bool &is_rem, string& var_name);

private:
  static inline bool find(unsigned &index, string& var_name);
  static vector<unsigned> frames;
  static vector<mdl_var> all_vars;
};

void mdl_var::dump() {
  cout << name_ << " ";
  switch (type_) {
  case MDL_T_NONE:
    cout << " MDL_T_NONE\n";
    break;
  case MDL_T_INT:
    cout << " MDL_T_INT\n";
    break;
  case MDL_T_STRING:
    cout << "MDL_T_STRING\n";
    break;
  case MDL_T_FLOAT:
    cout << "MDL_T_FLOAT\n";
    break;
    // case MDL_T_BOOL:
    // cout << "MDL_T_BOOL\n";
    // break;
  case MDL_T_POINT:
    cout << "MDL_T_POINT\n";
    break;
  case MDL_T_PROCEDURE:
    cout << "MDL_T_PROCEDURE\n";
    break;
  case MDL_T_MODULE:
    cout << "MDL_T_MODULE\n";
    break;
  case MDL_T_CALLSITE:
    cout << "MDL_T_CALLSITE\n";
    break;
  case MDL_T_COUNTER:
    cout << "MDL_T_COUNTER\n";
    break;
  case MDL_T_WALL_TIMER:
    cout << "MDL_T_WALL_TIMER\n";
    break;
  case MDL_T_PROC_TIMER:
    cout << "MDL_T_PROC_TIMER\n";
    break;
  case MDL_T_DRN:
    cout << "MDL_T_DRN\n";
    break;
  case MDL_T_LIST_PROCEDURE:
    cout << "MDL_T_LIST_PROCEDURE\n";
    break;
  case MDL_T_LIST_MODULE:
    cout << "MDL_T_LIST_MODULE\n";
    break;
  case MDL_T_LIST_POINT:
    cout << "MDL_T_LIST_POINT\n";
    break;
  case MDL_T_PROCESS:
    cout << "MDL_T_PROCESS\n";
    break;
  default:
    cout << " No type\n";
  }
}

void mdl_var::destroy() { type_ = MDL_T_NONE; vals.ptr = NULL; }

mdl_var::~mdl_var() {
  destroy();
}

mdl_var::mdl_var(bool is_remote) : type_(MDL_T_NONE), remote_(is_remote) {
  vals.ptr = NULL;
}

mdl_var::mdl_var(string& nm, bool is_rem)
: name_(nm), type_(MDL_T_NONE), remote_(is_rem) { vals.ptr = NULL; }

mdl_var::mdl_var(string& nm, int i, bool is_rem)
: name_(nm), type_(MDL_T_INT), remote_(is_rem) { vals.i = i;}

mdl_var::mdl_var(string& nm, float f, bool is_rem)
: name_(nm), type_(MDL_T_FLOAT), remote_(is_rem) { vals.f = f;}

mdl_var::mdl_var(string& nm, instPoint *p, bool is_rem)
: name_(nm), type_(MDL_T_POINT), remote_(is_rem) { vals.point_ = p;}

mdl_var::mdl_var(string& nm, pdFunction *pr, bool is_rem)
: name_(nm), type_(MDL_T_PROCEDURE), remote_(is_rem) { vals.pr = pr;}

mdl_var::mdl_var(string& nm, module *md, bool is_rem)
: name_(nm), type_(MDL_T_MODULE), remote_(is_rem) { vals.mod = md;}

mdl_var::mdl_var(string& nm, string& s, bool is_rem)
: name_(nm), type_(MDL_T_STRING), remote_(is_rem) { string_val = s;}

mdl_var::mdl_var(string& nm, process *p, bool is_rem) 
: name_(nm), type_(MDL_T_PROCESS), remote_(is_rem) { vals.the_process = p; }

mdl_var::mdl_var(string& nm, vector<pdFunction*> *vp, bool is_remote) 
: name_(nm), type_(MDL_T_LIST_PROCEDURE), remote_(is_remote) { vals.list_pr = vp; }
     
mdl_var::mdl_var(string& nm, vector<module*> *vm, bool is_remote) 
: name_(nm), type_(MDL_T_LIST_MODULE), remote_(is_remote) { vals.list_module = vm; }

mdl_var::mdl_var(string& nm, vector<int> *vi, bool is_remote)
: name_(nm), type_(MDL_T_LIST_INT), remote_(is_remote) {
  vals.list_int = vi;
}

mdl_var::mdl_var(string& nm, vector<float> *vf, bool is_remote)
: name_(nm), type_(MDL_T_LIST_FLOAT), remote_(is_remote) {
  vals.list_float = vf;
}

mdl_var::mdl_var(string& nm, vector<string> *vs, bool is_remote)
: name_(nm), type_(MDL_T_LIST_STRING), remote_(is_remote) {
  vals.list_string = vs;
}

mdl_var::mdl_var(string& nm, vector<instPoint*> *vip, bool is_remote)
: name_(nm), type_(MDL_T_LIST_POINT), remote_(is_remote) { vals.list_pts = vip; }

mdl_var::mdl_var(string& nm, dataReqNode *drn, bool is_remote) 
: name_(nm), type_(MDL_T_DRN), remote_(is_remote) { vals.drn = drn; }

bool mdl_var::get(int &i) {
  if (type_ != MDL_T_INT) return false;
  i = vals.i;
  return true;
}
bool mdl_var::get(float &f) {
  if (type_ != MDL_T_FLOAT) return false;
  f = vals.f;
  return true;
}

bool mdl_var::get(instPoint *&pt) {
  if (type_ != MDL_T_POINT) return false;
  pt = vals.point_;
  return true;
}
bool mdl_var::get(pdFunction *&pr) {
  if (type_ != MDL_T_PROCEDURE) return false;
  pr = vals.pr;
  return true;
}
bool mdl_var::get(module *&md) {
  if (type_ != MDL_T_MODULE) return false;
  md = vals.mod;
  return true;
}
bool mdl_var::get(string& s) {
  if (type_ != MDL_T_STRING) return false;
  s = string_val;
  return true;
}
bool mdl_var::get(process *&pr) {
  if (type_ != MDL_T_PROCESS) return false;
  pr = vals.the_process;
  return true;
}
bool mdl_var::get(dataReqNode *&drn) {
  if (type_ != MDL_T_DRN) return false;
  drn = vals.drn; 
  return true;
}

bool mdl_var::get(vector<pdFunction*> *&vp) {
  if (type_ != MDL_T_LIST_PROCEDURE) return false;
  vp = vals.list_pr;
  return true;
}
bool mdl_var::get(vector<module*> *&vm) {
  if (type_ != MDL_T_LIST_MODULE) return false;
  vm = vals.list_module;
  return true;
}
bool mdl_var::get(vector<int> *&vi) {
  if (type_ != MDL_T_LIST_INT) return false;
  vi = vals.list_int;
  return true;
}
bool mdl_var::get(vector<float> *&vf) {
  if (type_ != MDL_T_LIST_FLOAT) return false;
  vf = vals.list_float;
  return true;
}
bool mdl_var::get(vector<string> *&vs) {
  if (type_ != MDL_T_LIST_STRING) return false;
  vs = vals.list_string;
  return true;
}
bool mdl_var::get(vector<instPoint*> *&vip) {
  if (type_ != MDL_T_LIST_POINT) return false;
  vip = vals.list_pts;
  return true;
}

bool mdl_var::set(int i) {
  destroy();
  type_ = MDL_T_INT;
  vals.i = i;
  return true;
}
bool mdl_var::set(float f) {
  destroy();
  type_ = MDL_T_FLOAT;
  vals.f = f;
  return true;
}

bool mdl_var::set(instPoint *pt) {
  destroy();
  type_ = MDL_T_POINT;
   vals.point_ = pt;
  return true;
}
bool mdl_var::set(pdFunction *pr) {
  destroy();
  type_ = MDL_T_PROCEDURE;
  vals.pr = pr;
  return true;
}
bool mdl_var::set(module *md) {
  destroy();
  type_ = MDL_T_MODULE;
  vals.mod = md;
  return true;
}
bool mdl_var::set(string& s) {
  destroy();
  type_ = MDL_T_STRING;
  string_val = s;
  return true;
}
bool mdl_var::set(process *pr) {
  destroy();
  type_ = MDL_T_PROCESS;
  vals.the_process = pr;
  return true;
}
bool mdl_var::set(dataReqNode *drn) {
  destroy();
  type_ = MDL_T_DRN;
  vals.drn = drn;
  return true;
}

bool mdl_var::set(vector<pdFunction*> *vp) {
  destroy();
  type_ = MDL_T_LIST_PROCEDURE;
  vals.list_pr = vp;
  return true;
}
bool mdl_var::set(vector<module*> *vm) {
  destroy();
  type_ = MDL_T_LIST_MODULE;
  vals.list_module = vm;
  return true;
}
bool mdl_var::set(vector<int> *vi) {
  destroy();
  type_ = MDL_T_LIST_INT;
  vals.list_int = vi;
  return true;
}
bool mdl_var::set(vector<float> *vf) {
  destroy();
  type_ = MDL_T_LIST_FLOAT;
  vals.list_float = vf;
  return true;
}
bool mdl_var::set(vector<string> *vs) {
  destroy();
  type_ = MDL_T_LIST_STRING;
  vals.list_string = vs;
  return true;
}
bool mdl_var::set(vector<instPoint*> *vip) {
  destroy();
  type_ = MDL_T_LIST_POINT;
  vals.list_pts = vip;
  return true;
}

bool mdl_var::is_list() const {
  if ((type_ >= MDL_T_LIST_BASE) && (type_ <= MDL_T_LIST_END))
    return true;
  else
    return false;
}

unsigned mdl_var::list_size() const {
  switch (type_) {
  case MDL_T_LIST_INT:
    return (vals.list_int->size());    
  case MDL_T_LIST_FLOAT:
    return (vals.list_float->size());
  case MDL_T_LIST_STRING:
    return (vals.list_string->size());
  case MDL_T_LIST_PROCEDURE:
    return (vals.list_pr->size());
  case MDL_T_LIST_MODULE:
    return (vals.list_module->size());
  default:
    return 0;
  }
}

bool mdl_var::get_ith_element(mdl_var &ret, unsigned ith) const {
  if (ith >= list_size()) return false;

  switch (type_) {
  case MDL_T_LIST_INT:
    return (ret.set((*vals.list_int)[ith]));
  case MDL_T_LIST_FLOAT:
    return (ret.set((*vals.list_float)[ith]));
  case MDL_T_LIST_STRING:
    return (ret.set((*vals.list_string)[ith]));
  case MDL_T_LIST_PROCEDURE:
    assert(0); return false;
  case MDL_T_LIST_MODULE:
    assert(0); return false;
  default:
    return false;
  }
  return true;
}

bool mdl_var::add_to_list(mdl_var& add_me) {
  int i; float f; string s;
  if (!vals.ptr) return false;
  switch (type_) {
  case MDL_T_LIST_INT:
    if (!add_me.get(i)) return false;
    (*vals.list_int) += i;
    break;
  case MDL_T_LIST_FLOAT:
    if (!add_me.get(f)) return false;
    (*vals.list_float) += f;
    break;
  case MDL_T_LIST_STRING: 
    if (!add_me.get(s)) return false;
    (*vals.list_string) += s;
    break;
  default:
    return false;
  }
  return true;
}

bool mdl_var::make_list(unsigned element_type) {
  vector<string> *vs;
  vector<int> *vi;
  vector<float> *vf;
  vector<module*> *vm;
  vector<pdFunction*> *vp;

  switch (element_type) {
  case MDL_T_INT:
    vi = new vector<int>;
    if (!set(vi)) return false;
    break;
  case MDL_T_FLOAT:
    vf = new vector<float>;
    if (!set(vf)) return false;
    break;
  case MDL_T_STRING:
    vs = new vector<string>;
    if (!set(vs)) return false;
    break;
  case MDL_T_PROCEDURE:
    vp = new vector<pdFunction*>;
    if (!set(vp)) return false;
    break;
  case MDL_T_MODULE:
    vm = new vector<module*>;
    if (!set(vm)) return false;
    break;
  default:
    return false;
  }
  return true;
}

unsigned mdl_var::as_list() {
  switch (type_) {
  case MDL_T_INT:
    return MDL_T_LIST_INT;
  case MDL_T_FLOAT:
    return MDL_T_LIST_FLOAT;
  case MDL_T_STRING:
    return MDL_T_LIST_STRING;
  case MDL_T_PROCEDURE:
    return MDL_T_LIST_PROCEDURE;
  case MDL_T_MODULE:
    return MDL_T_LIST_MODULE;
  default:
    return MDL_T_NONE;
  }
}

void mdl_var::set_type(unsigned type) {
  destroy();
  type_ = type;
  vals.i = 0;
}

string mdl_var::name() const { return name_;}
unsigned mdl_var::type() const { return type_; }

unsigned mdl_var::element_type() const {
  switch (type_) {
  case MDL_T_LIST_INT:        return MDL_T_INT;
  case MDL_T_LIST_FLOAT:      return MDL_T_FLOAT;
  case MDL_T_LIST_STRING:     return MDL_T_STRING;
  case MDL_T_LIST_PROCEDURE:  return MDL_T_PROCEDURE;
  case MDL_T_LIST_MODULE:     return MDL_T_MODULE;
  case MDL_T_LIST_POINT:      return MDL_T_POINT;
  default:                    return MDL_T_NONE;
  }
}

void mdl_env::dump() {
  for (unsigned u=0; u<mdl_env::all_vars.size(); u++)
    mdl_env::all_vars[u].dump();
}

void mdl_env::push() {
  mdl_env::frames += mdl_env::all_vars.size();
}

bool mdl_env::pop() {
  unsigned frame_size = mdl_env::frames.size();
  unsigned index = mdl_env::frames[frame_size - 1];
  mdl_env::frames.resize(frame_size-1);
  mdl_env::all_vars.resize(index);
  return true;
}

bool mdl_env::add(mdl_var& value) {
  mdl_env::all_vars += value;
  return true;
}

bool mdl_env::add(string& var_name, bool is_remote, unsigned type) {
  mdl_var temp(var_name, is_remote);
  if (((type >= MDL_T_SCALAR_BASE) && (type <= MDL_T_SCALAR_END)) ||
      ((type >= MDL_T_LIST_BASE) && (type <= MDL_T_LIST_END)))
    temp.set_type(type);
  mdl_env::all_vars += temp;
  return true;
}

bool mdl_env::find(unsigned &index, string& var_name) {
  unsigned size = mdl_env::all_vars.size();
  if (!size)
    return false;

  do {
    size--;
    if (mdl_env::all_vars[size].name() == var_name) {
      index = size;
      return true;
    }
  } while (size);
  return false;
}

bool mdl_env::type(unsigned &ret, string& var_name) {
  unsigned index;
  if (!mdl_env::find(index, var_name))
    return false;
  ret = mdl_env::all_vars[index].type();
  return true;
}

bool mdl_env::is_remote(bool &is_rem, string& var_name) {
  unsigned index;
  if (!mdl_env::find(index, var_name))
    return false;
  is_rem = mdl_env::all_vars[index].remote();
  return true;
}

bool mdl_env::set_type(unsigned type, string& var_name) {
  unsigned index;
  if (!mdl_env::find(index, var_name))
    return false;
  mdl_env::all_vars[index].set_type(type);
  return true;
}

bool mdl_env::get(mdl_var &ret, string& var_name) {
  unsigned index;
  if (!mdl_env::find(index, var_name))
    return false;
  
  ret = mdl_env::all_vars[index];
  return true;
}

bool mdl_env::set(int i, string& var_name) {
  unsigned index;
  if (!mdl_env::find(index, var_name))
    return false;
  mdl_env::all_vars[index].set(i);
  return true;
}

bool mdl_env::set(float f, string& var_name) {
  unsigned index;
  if (!mdl_env::find(index, var_name))
    return false;
  mdl_env::all_vars[index].set(f);
  return true;
}

bool mdl_env::set(instPoint *p, string& var_name) {
  unsigned index;
  if (!mdl_env::find(index, var_name))
    return false;
  mdl_env::all_vars[index].set(p);
  return true;
}

bool mdl_env::set(pdFunction *pr, string& var_name) {
  unsigned index;
  if (!mdl_env::find(index, var_name))
    return false;
  mdl_env::all_vars[index].set(pr);
  return true;
}

bool mdl_env::set(module *mod, string& var_name) {
  unsigned index;
  if (!mdl_env::find(index, var_name))
    return false;
  mdl_env::all_vars[index].set(mod);
  return true;
}

bool mdl_env::set(string& s, string& var_name) {
  unsigned index;
  if (!mdl_env::find(index, var_name))
    return false;
  mdl_env::all_vars[index].set(s);
  return true;
}

bool mdl_env::set(process *pr, string& var_name) {
  unsigned index;
  if (!mdl_env::find(index, var_name))
    return false;
  mdl_env::all_vars[index].set(pr);
  return true;
}

bool mdl_env::set(dataReqNode *drn, string& var_name) {
  unsigned index;
  if (!mdl_env::find(index, var_name))
    return false;
  mdl_env::all_vars[index].set(drn);
  return true;
}

bool mdl_env::set(vector<pdFunction*> *vp, string& var_name) {
  unsigned index;
  if (!mdl_env::find(index, var_name))
    return false;
  mdl_env::all_vars[index].set(vp);
  return true;
}
bool mdl_env::set(vector<module*> *vm, string& var_name) {
  unsigned index;
  if (!mdl_env::find(index, var_name))
    return false;
  mdl_env::all_vars[index].set(vm);
  return true;
}
bool mdl_env::set(vector<int> *vi, string& var_name) {
  unsigned index;
  if (!mdl_env::find(index, var_name))
    return false;
  mdl_env::all_vars[index].set(vi);
  return true;
}
bool mdl_env::set(vector<float> *vf, string& var_name) {
  unsigned index;
  if (!mdl_env::find(index, var_name))
    return false;
  mdl_env::all_vars[index].set(vf);
  return true;
}
bool mdl_env::set(vector<string> *vs, string& var_name) {
  unsigned index;
  if (!mdl_env::find(index, var_name))
    return false;
  mdl_env::all_vars[index].set(vs);
  return true;
}
bool mdl_env::set(vector<instPoint*> *vip, string& var_name) {
  unsigned index;
  if (!mdl_env::find(index, var_name))
    return false;
  mdl_env::all_vars[index].set(vip);
  return true;
}

#endif
