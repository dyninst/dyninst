
/*
 * $Log: metParse.h,v $
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
extern char *metParseError_list[6];

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
  vector<T_dyninstRPC::mdl_constraint*> *v_cons;
  T_dyninstRPC::mdl_icode *i_code;
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

