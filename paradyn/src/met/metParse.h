
/*
 * $Log: metParse.h,v $
 * Revision 1.3  1994/08/22 15:53:25  markc
 * Config language version 2.
 *
 * Revision 1.1  1994/07/07  03:25:26  markc
 * Configuration language parser.
 *
 */

#ifndef _MET_PARSE_H
#define _MET_PARSE_H

#include "util/h/machineType.h"
#include "util/h/list.h"
#include <stdio.h>

#define SET_NAME 1
#define SET_COMMAND 2
#define SET_HOST 3
#define SET_FLAVOR 4
#define SET_USER 5
#define SET_DIR 6
#define SET_DAEMON 7

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
class stringList;
class tunableMet;


extern metError metParseError;
extern char *metParseError_list[6];

typedef struct field {
  char *val;
  int spec;
  int flav;
} field;

struct parseStack {
  char *cp;
  int i;
  float f;
  int flav;
  field fld;
  processMet *pm;
  tunableMet *tm;
  daemonMet *dm;
  visiMet *vm;
  List<char*> *sl;
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
  tunableMet(float f, char *c) : Fvalue(f), name(c), useBvalue(0) {;}
  tunableMet(int b, char *c) : Bvalue(b), name(c), useBvalue(1) {;}
  ~tunableMet() { destroy(); }

  void destroy() {if (name) delete name; name = 0;}
  static int addTunable (char *c, float f);
  static int addTunable (char *c, int b);

  // print the instance
  void dump() const;
  static void dumpAll();
  static List<tunableMet*> allTunables;

  char *name;
  int useBvalue;
  float Fvalue;
  int Bvalue;

 private:
  static int addHelper(char *c);
};

class stringList {
public:
  stringList(char *n, List<char*> it)
    {name = n; items = it;}
  ~stringList() { destroy(); }
  void destroy() { if (name) delete name; items.removeAll();}

  // create a new list, or update the string list on an existing list
  static int addStringList (char *name, List<char*> it);

  // print the instance
  void dump() const ;
  static void dumpAll();
  static List<stringList*> allLists;

  List<char*> items;
  char *name;
};

class daemonMet {
 public:
  daemonMet() : name(0), command(0), execDir(0), user(0), host(0)
    { ; }
  ~daemonMet() { destroy(); }
  void destroy();

  int set_field (field &f);
  static int addDaemon(daemonMet *ds);

  // print the instance
  void dump() const;
  static void dumpAll();
  static List<daemonMet*> allDaemons;

  char *name;
  char *command;
  char *execDir;
  char *user;
  char *host;
  int flavor;
};

class processMet {
public:
  processMet() : name(0), command(0), daemon(0), host(0), user(0), execDir(0)
    { ; }
  ~processMet() { destroy(); }
  void destroy();

  int set_field (field &f);
  static int addProcess(processMet *pm);

  // print the instance
  void dump() const;
  static void dumpAll();
  static List<processMet*> allProcs;

  char *name;
  char * command;
  char * daemon;
  char * host;
  char * execDir;
  char * user;
};

class visiMet {
public:
  visiMet() : name(0), user(0), host(0), execDir(0), command(0)
    { ; }
  ~visiMet() { destroy(); }
  void destroy();

  int set_field (field &f);
  static int addVisi(visiMet *vs);

  // print the instance
  void dump() const;
  static void dumpAll();
  static List<visiMet*> allVisis;

  char *name;
  char * command;
  char * execDir;
  char * host;
  char * user;
};

#endif

