
/*
 * $Log: metParse.h,v $
 * Revision 1.8  1995/11/21 21:06:53  naim
 * Fixing unitsType definition for MDL grammar - naim
 *
 * Revision 1.7  1995/11/21  15:15:36  naim
 * Changing the MDL grammar to allow more flexible metric definitions (i.e. we
 * can specify all elements in any order). Additionally, the option "fold"
 * has been removed - naim
 *
 * Revision 1.6  1995/11/08  06:23:16  tamches
 * removed some warnings
 *
 * Revision 1.5  1995/05/18 10:58:32  markc
 * mdl
 *
 * Revision 1.4  1995/02/07  21:59:53  newhall
 * added a force option to the visualization definition, this specifies
 * if the visi should be started before metric/focus menuing
 * removed compiler warnings
 *
 * Revision 1.3  1994/08/22  15:53:25  markc
 * Config language version 2.
 *
 * Revision 1.1  1994/07/07  03:25:26  markc
 * Configuration language parser.
 *
 */

#ifndef _MET_PARSE_H
#define _MET_PARSE_H

#include "util/h/machineType.h"
#include "util/h/Vector.h"
#include <stdio.h>
#include "dyninstRPC.xdr.h"
#include "paradyn/src/met/globals.h"

#define SET_NAME 1
#define SET_COMMAND 2
#define SET_HOST 3
#define SET_FLAVOR 4
#define SET_USER 5
#define SET_DIR 6
#define SET_DAEMON 7
#define SET_FORCE 8

typedef enum {SET_MNAME, SET_UNITS, SET_AGG, SET_STYLE,
	      SET_MFLAVOR, SET_MODE, SET_UNITTYPE, SET_CONSTRAINT, SET_TEMPS,
	      SET_BASE} setType;

typedef struct metricFld {
  setType spec;
  string name;
  string units;
  unsigned agg;
  unsigned style;
  vector<string> *flavor;
  bool mode;
  bool unittype;
  T_dyninstRPC::mdl_constraint *constraint;
  string temps;
  unsigned base_type;
  vector<T_dyninstRPC::mdl_stmt*> *base_m_stmt_v;
} metricFld;

class metricDef {
public:
  void setName(string name) {name_ = name; name_flag = true;}
  void setUnits(string units) {units_ = units; units_flag = true;}
  void setAgg(unsigned agg) {agg_ = agg; agg_flag = true;}
  void setStyle(unsigned style) {style_ = style; style_flag = true;}
  void setFlavor(vector<string> *flavor) {flavor_ = flavor; 
					  flavor_flag = true;}
  void setMode(bool mode) {mode_ = mode; mode_flag = true;}
  void setUnittype(bool unittype) {unittype_ = unittype; unittype_flag = true;}
  void setConstraintList(T_dyninstRPC::mdl_constraint *constraint)  
  {
    (*constraint_list_) += constraint;
    constraint_list_flag = true;
  }
  void setTemps(string temps) 
  {
    (*temps_) += temps; 
    temps_flag = true;
  }
  void setBaseType(unsigned base_type) {base_type_ = base_type; 
					base_type_flag = true;}
  void setBaseMstmtV(vector<T_dyninstRPC::mdl_stmt*> *base_m_stmt_v) 
		     {base_m_stmt_v_ = base_m_stmt_v;
		      base_m_stmt_v_flag = true;}
  void setField(metricFld &f) {
    switch(f.spec) {
      case SET_MNAME:
	this->setName(f.name);
	break;
      case SET_UNITS:
	this->setUnits(f.units);
	break;
      case SET_AGG:
	this->setAgg(f.agg);
	break;
      case SET_STYLE:
	this->setStyle(f.style);
	break;
      case SET_MFLAVOR:
	this->setFlavor(f.flavor);
	break;
      case SET_MODE:
	this->setMode(f.mode);
	break;
      case SET_UNITTYPE:
	this->setUnittype(f.unittype);
	break;
      case SET_CONSTRAINT:
	this->setConstraintList(f.constraint);
	break;
      case SET_TEMPS:
	this->setTemps(f.temps);
	break;
      case SET_BASE:
	this->setBaseType(f.base_type);
	this->setBaseMstmtV(f.base_m_stmt_v);
	break;
    }
  }
  string missingFields() {
    string msg("MDL error - missing fields from your metric declaration: ");
    if (!base_type_flag || !base_m_stmt_v_flag) msg += string("base ");
    if (!units_flag) msg += string("units ");
    if (!name_flag) msg += string("name ");
    if (!style_flag) msg += string("style ");
    if (!agg_flag) msg += string("agg ");
    if (!flavor_flag) msg += string("flavor ");
    return(msg);
  }
  int addNewMetricDef(string ident) {
    bool ok=true;
    if (!base_m_stmt_v_flag ||
	!base_type_flag ||
        !units_flag ||
	!name_flag ||
	!style_flag ||
	!agg_flag ||
	!flavor_flag) 
    {
      ok=false;
    }
    if (ok)
      return(mdl_data::new_metric(ident,
	     			  name_,
				  units_,
				  agg_,
				  style_,
				  base_type_,
				  base_m_stmt_v_,
				  flavor_,
				  constraint_list_,
				  temps_,
				  mode_,
				  unittype_));
    else
      return(false);
  }
  metricDef() : mode_(false),
		unittype_(true),
		base_m_stmt_v_flag(false),
		base_type_flag(false), 
		temps_flag(false), 
		constraint_list_flag(false), 
		mode_flag(false), 
		unittype_flag(false),
		flavor_flag(false), 
		style_flag(false), 
		agg_flag(false),
		units_flag(false), 
		name_flag(false) 
  {
    constraint_list_ = new vector<T_dyninstRPC::mdl_constraint*>;
    temps_ = new vector<string>;
  }
  ~metricDef() {}
private:
  string name_;
  string units_;
  unsigned agg_;
  unsigned style_;
  vector<string> *flavor_;
  bool mode_;
  bool unittype_;
  vector<T_dyninstRPC::mdl_constraint*> *constraint_list_;
  vector<string> *temps_;
  unsigned base_type_; 
  vector<T_dyninstRPC::mdl_stmt*> *base_m_stmt_v_;
  bool base_m_stmt_v_flag;
  bool base_type_flag;
  bool temps_flag;
  bool constraint_list_flag;
  bool mode_flag;
  bool unittype_flag;
  bool flavor_flag;
  bool style_flag;
  bool agg_flag;
  bool units_flag;
  bool name_flag;
};

typedef struct mdl_base {
  vector<T_dyninstRPC::mdl_stmt*> *m_stmt_v;
  unsigned type;
} mdl_base;

typedef enum { ERR_NO_ERROR,
	       ERR_NAME_IN_USE_TYPE,
	       ERR_NAME_IN_USE_SAME,
	       ERR_NAME_IN_USE,
	       ERR_BAD_ARGS,
	       ERR_MALLOC
	       } metError;

class processMet;
class visiMet;
class parseMet;
class daemonMet;
class tunableMet;

extern metError metParseError;
extern const char *metParseError_list[6];

typedef struct iop_struct {
  unsigned type;
  unsigned arg;
  string *str;
  string *str2;
} iop_struct;

typedef struct ie_struct {
  iop_struct rand1;
  iop_struct rand2;
  unsigned bin_op;
} ie_struct;

typedef struct field {
  string *val;
  int spec;
  string *flav;
  int force;
} field;

typedef struct string_list {
  string name;
  vector<string> elements;
} string_list;

extern void add_string_list(string &name, vector<string> &elem);

struct parseStack {
  int i;
  float f;
  unsigned u;
  int flav;
  field fld;
  bool b;
  string *sp;
  char *charp;
  string_list *slist;
  processMet *pm;
  tunableMet *tm;
  daemonMet *dm;
  visiMet *vm;
  vector<string> *vs;
  ie_struct expr;
  iop_struct rand;
  T_dyninstRPC::mdl_instr_req *instr_req;
  vector <T_dyninstRPC::mdl_icode*> *icode_v;
  T_dyninstRPC::mdl_stmt *m_stmt;
  vector<T_dyninstRPC::mdl_stmt*> *m_stmt_v;
  T_dyninstRPC::mdl_expr *m_expr;
  vector<T_dyninstRPC::mdl_expr*> *m_expr_v;
  mdl_base base;
  T_dyninstRPC::mdl_constraint *constraint;
  T_dyninstRPC::mdl_icode *i_code;
  metricFld mfld;
  metricDef *mde;
};

extern FILE *yyin;

typedef enum {TYPE_BASE,
	      TYPE_STRING,
	      TYPE_VISI,
	      TYPE_PROCESS,
	      TYPE_DAEMON,
	      TYPE_TUNABLE} itemType;

class tunableMet {
public:
  tunableMet() { }
  tunableMet(float f, string &c) : Fvalue_(f), name_(c), useBvalue_(false) {}
  tunableMet(int b, string &c) : Bvalue_(b), name_(c), useBvalue_(true) {}
  ~tunableMet() { }

  static bool addTunable (string &c, float f);
  static bool addTunable (string &c, int b);

  // print the instance
  void dump() const;
  static void dumpAll();

  static vector<tunableMet*> allTunables;
  string name() const { return name_; }
  int Bvalue() const { return Bvalue_; }
  float Fvalue() const { return Fvalue_; }
  bool useBvalue() const { return useBvalue_; }
  
private:
  float Fvalue_;
  int Bvalue_;
  string name_;
  bool useBvalue_;
  static void addHelper(tunableMet *addIt);
};

class daemonMet {
 public:
  daemonMet() { }
  daemonMet(string& nm, string& cmd, string& exec, string& u, string& h, string& flav);
  ~daemonMet() { }

  bool set_field (field &f);
  static bool addDaemon(daemonMet *ds);

  // print the instance
  void dump() const;
  static void dumpAll();

  string name() const { return name_; }
  string command() const { return command_; }
  string execDir() const { return execDir_; }
  string user() const { return user_; }
  string host() const { return host_; }
  string flavor() const { return flavor_; }

  static vector<daemonMet*> allDaemons;
  
private:
  string name_;
  string command_;
  string execDir_;
  string user_;
  string host_;
  string flavor_;
};

class processMet {
public:
  processMet() { }
  processMet(string& nm, string& cmd, string& d, string& h, string& u, string& exec);
  ~processMet() { }

  bool set_field (field &f);
  static bool addProcess(processMet *pm);

  // print the instance
  void dump() const;
  static void dumpAll();

  string name() const { return name_; }
  string command() const { return command_;}
  string daemon() const { return daemon_; }
  string host() const { return host_; }
  string user() const { return user_; }
  string execDir() const { return execDir_; }

  static vector<processMet*> allProcs;

private:
  string name_;
  string command_;
  string daemon_;
  string host_;
  string user_;
  string execDir_;
};

class visiMet {
public:
  visiMet() : force_(0) { }
  visiMet(string& nm, string& u, string& h, string& e, string& c, int& f);
  ~visiMet() { }

  bool set_field (field &f);
  static bool addVisi(visiMet *vs);

  // print the instance
  void dump() const;
  static void dumpAll();

  string name() const { return name_; }
  string command() const { return command_; }
  int force() const { return force_; }

  static vector<visiMet*> allVisis;

private:
  string name_;
  string user_;
  string host_;
  string execDir_;
  string command_;
  int force_;

};

#endif

