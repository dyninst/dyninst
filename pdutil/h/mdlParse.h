/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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

// $Id: mdlParse.h,v 1.4 2004/03/23 01:12:40 eli Exp $

#ifndef _MDL_PARSE_H
#define _MDL_PARSE_H

#include <stdio.h>
#include "common/h/machineType.h"
#include "common/h/String.h"
#include "common/h/Vector.h"
#include "dyninstRPC.xdr.h"
#include "pdutil/h/mdl_data.h"


#define SET_NAME 1
#define SET_COMMAND 2
#define SET_HOST 3
#define SET_FLAVOR 4
#define SET_USER 5
#define SET_DIR 6
#define SET_DAEMON 7
#define SET_FORCE 8
#define SET_LIMIT 9
#define SET_METFOCUS 10
#define SET_REMSH 11
#define SET_AUTO_START 12

class T_dyninstRPC;
class T_dyninstRPC::mdl_constraint;
class T_dyninstRPC::mdl_stmt;
class T_dyninstRPC::mdl_icode;
class T_dyninstRPC::mdl_expr;


typedef enum
{
    LIB_CONSTRAINT_REGEX_FLAG = 1,
    LIB_CONSTRAINT_NOCASE_FLAG = 2
} LibConstraintFlagType;

typedef enum {SET_MNAME, SET_UNITS, SET_AGG, SET_STYLE,
	      SET_MFLAVOR, SET_MODE, SET_UNITTYPE, SET_CONSTRAINT, SET_TEMPS,
	      SET_BASE} setType;

typedef struct metricFld {
  setType spec;
  pdstring name;
  pdstring units;
  unsigned agg;
  unsigned style;
  pdvector<pdstring> *flavor;
  bool mode;
  int unittype;
  T_dyninstRPC::mdl_constraint *constraint;
  pdstring temps;
  unsigned base_type;
  pdvector<T_dyninstRPC::mdl_stmt*> *base_m_stmt_v;
  pdstring base_hwcntr_str;
} metricFld;

class metricDef {
public:
  void setName(pdstring name) {name_ = name; name_flag = true;}
  void setUnits(pdstring units) {units_ = units; units_flag = true;}
  void setAgg(unsigned agg) {agg_ = agg; agg_flag = true;}
  void setStyle(unsigned style) {style_ = style; style_flag = true;}
  void setFlavor(pdvector<pdstring> *flavor) {flavor_ = flavor; 
					  flavor_flag = true;}
  void setMode(bool mode) {mode_ = mode; mode_flag = true;}
  void setUnittype(int unittype) {unittype_ = unittype; unittype_flag = true;}
  void setConstraintList(T_dyninstRPC::mdl_constraint *constraint)  
  {
    (*constraint_list_) += constraint;
    constraint_list_flag = true;
  }
  void setTemps(pdstring temps) 
  {
    (*temps_) += temps; 
    temps_flag = true;
  }
  void setBaseType(unsigned base_type) {base_type_ = base_type; 
					base_type_flag = true;}
  void setBaseMstmtV(pdvector<T_dyninstRPC::mdl_stmt*> *base_m_stmt_v) 
		     {base_m_stmt_v_ = base_m_stmt_v;
		      base_m_stmt_v_flag = true;}

  void setBaseHwStr(pdstring hw_str) { base_hwcntr_str = hw_str; }

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
	this->setBaseHwStr(f.base_hwcntr_str);
	break;
    }
  }
  pdstring missingFields() {
    pdstring msg("MDL error - missing fields from your metric declaration: ");
    if (!base_type_flag || !base_m_stmt_v_flag) msg += pdstring("base ");
    if (!units_flag) msg += pdstring("units ");
    if (!name_flag) msg += pdstring("name ");
    if (!style_flag) msg += pdstring("style ");
    if (!agg_flag) msg += pdstring("agg ");
    if (!flavor_flag) msg += pdstring("flavor ");
    return(msg);
  }
  int addNewMetricDef(pdstring ident) {
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
      return(mdl_data::cur_mdl_data->new_metric(ident,
	     			  name_,
				  units_,
				  agg_,
				  style_,
				  base_type_,
                                  base_hwcntr_str,
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
		unittype_(0),
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
    constraint_list_ = new pdvector<T_dyninstRPC::mdl_constraint*>;
    temps_ = new pdvector<pdstring>;
  }
  ~metricDef() {}
private:
  pdstring name_;
  pdstring units_;
  unsigned agg_;
  unsigned style_;
  pdvector<pdstring> *flavor_;
  bool mode_;
  int unittype_;
  pdvector<T_dyninstRPC::mdl_constraint*> *constraint_list_;
  pdvector<pdstring> *temps_;
  unsigned base_type_; 
  pdvector<T_dyninstRPC::mdl_stmt*> *base_m_stmt_v_;
  pdstring base_hwcntr_str;
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
  pdvector<T_dyninstRPC::mdl_stmt*> *m_stmt_v;
  unsigned type;
  pdstring* hwcntr_str;
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

// struct for instr_expr, since I unified it with metric_expr,
// don't need it anymore  --chun
/*
typedef struct ie_struct {
  T_dyninstRPC::mdl_instr_rand *rand1;
  T_dyninstRPC::mdl_instr_rand *rand2;
  unsigned bin_op;
} ie_struct;
*/

typedef struct field {
  pdstring *val;
  bool bval;
  int spec;
  pdstring *flav;
  int force;
  int limit;
} field;

typedef struct string_list {
  pdstring name;
  pdvector<pdstring> elements;
} string_list;

extern void add_string_list(pdstring &name, pdvector<pdstring> &elem);

// Warning: bison will allocate a parseStack struct with malloc, so the constructors
// for C++ classes will not be called!
// This struct should not have any C++ objects. Use a pointer instead, and allocate
// the object with new.
struct parseStack {
  int i;
  float f;
  unsigned u;
  int flav;
  field fld;
  bool b;
  pdstring *sp;
  char *charp;
  string_list *slist;
  processMet *pm;
  tunableMet *tm;
  daemonMet *dm;
  visiMet *vm;
  pdvector<pdstring> *vs;
  //ie_struct expr;
  //T_dyninstRPC::mdl_instr_rand *rand;
  //pdvector<T_dyninstRPC::mdl_instr_rand *> *pars;
  //T_dyninstRPC::mdl_instr_req *instr_req;
  pdvector <T_dyninstRPC::mdl_icode*> *icode_v;
  T_dyninstRPC::mdl_stmt *m_stmt;
  pdvector<T_dyninstRPC::mdl_stmt*> *m_stmt_v;
  T_dyninstRPC::mdl_expr *m_expr;
  pdvector<T_dyninstRPC::mdl_expr*> *m_expr_v;
  mdl_base base;
  T_dyninstRPC::mdl_constraint *constraint;
  T_dyninstRPC::mdl_icode *i_code;
  metricFld *mfld;
  metricDef *mde;
  pdvector<pdstring> *vsf;
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
  tunableMet(float f, pdstring &c) : Fvalue_(f), name_(c), useBvalue_(false) {}
  tunableMet(bool b, pdstring &c) : Bvalue_(b), name_(c), useBvalue_(true) {}
  ~tunableMet() { }

  static bool addTunable (pdstring &c, float f);
  static bool addTunable (pdstring &c, bool b);

  // print the instance
  void dump() const;
  static void dumpAll();

  static pdvector<tunableMet*> allTunables;
  pdstring name() const { return name_; }
  bool Bvalue() const { return Bvalue_; }
  float Fvalue() const { return Fvalue_; }
  bool useBvalue() const { return useBvalue_; }
  
private:
  float Fvalue_;
  bool Bvalue_;
  pdstring name_;
  bool useBvalue_;
  static void addHelper(tunableMet *addIt);
};

class daemonMet {
 public:
  daemonMet() { }
  daemonMet(pdstring& nm, pdstring& cmd, pdstring& remsh, pdstring& exec, pdstring& u, pdstring& h, pdstring& flav);
  ~daemonMet() { }

  bool set_field (field &f);
  static bool addDaemon(daemonMet *ds);

  // print the instance
  void dump() const;
  static void dumpAll();

  pdstring name() const { return name_; }
  pdstring command() const { return command_; }
  pdstring remoteShell() const { return remoteShell_; }
  pdstring execDir() const { return execDir_; }
  pdstring user() const { return user_; }
  pdstring host() const { return host_; }
  pdstring flavor() const { return flavor_; }

  static pdvector<daemonMet*> allDaemons;
  
private:
  pdstring name_;
  pdstring command_;
  pdstring remoteShell_;
  pdstring execDir_;
  pdstring user_;
  pdstring host_;
  pdstring flavor_;
};

class processMet {
public:
  processMet() { autoStart_ = true; }
  processMet(pdstring& nm, pdstring& cmd, pdstring& d, pdstring& h, pdstring& u, pdstring& exec, bool auto_start);
  ~processMet() { }

  bool set_field (field &f);
  static bool addProcess(processMet *pm);

  // print the instance
  void dump() const;
  static void dumpAll();

  pdstring name() const { return name_; }
  pdstring command() const { return command_;}
  pdstring daemon() const { return daemon_; }
  pdstring host() const { return host_; }
  pdstring user() const { return user_; }
  pdstring execDir() const { return execDir_; }
  bool autoStart() const { return autoStart_; }

  // These two methods are implemented in metMain.C rather than metClass.C
  static bool doInitProcess();
  static void checkDaemonProcess( const pdstring &host );

private:
  pdstring name_;
  pdstring command_;
  pdstring daemon_;
  pdstring host_;
  pdstring user_;
  pdstring execDir_;
  bool autoStart_;

  static pdvector<processMet*> allProcs;
};

class visiMet {
public:
  visiMet() : force_(0), limit_(0) {metfocus_ = NULL; }
  visiMet(pdstring& nm,pdstring& u,pdstring& h,pdstring& e,pdstring& c,int& f,int& l);
  ~visiMet() {delete metfocus_; }

  bool set_field (field &f);
  static bool addVisi(visiMet *vs);

  // print the instance
  void dump() const;
  static void dumpAll();

  pdstring name() const { return name_; }
  pdstring command() const { return command_; }
  int force() const { return force_; }
  int limit() const { return limit_; }
  pdvector<pdstring> *metfocus() const { return metfocus_;}

  static pdvector<visiMet*> allVisis;

private:
  pdstring name_;
  pdstring user_;
  pdstring host_;
  pdstring execDir_;
  pdstring command_;
  int force_;
  int limit_;
  pdvector<pdstring> *metfocus_;
};


// functions and variables implementing the MDL parser
int mdlparse( void );
void mdlrestart( FILE* );
extern FILE* mdlin;
extern const char* mdlBufPtr;
extern unsigned int mdlBufRemaining;



#endif // _MDL_PARSE_H

