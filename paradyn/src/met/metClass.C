
/*
 * Implements classes used for metric description language
 *
 * $Log: metClass.C,v $
 * Revision 1.6  1996/04/04 21:55:23  newhall
 * added limit option to visi definition
 *
 * Revision 1.5  1995/11/08  06:23:33  tamches
 * removed some warnings
 *
 * Revision 1.4  1995/05/18 10:58:15  markc
 * Added mdl hooks
 *
 * Revision 1.3  1995/02/07  21:59:50  newhall
 * added a force option to the visualization definition, this specifies
 * if the visi should be started before metric/focus menuing
 * removed compiler warnings
 *
 * Revision 1.2  1994/08/31  22:21:01  markc
 * Added log entries to metClass
 *
 */

#include "metParse.h"
#include <assert.h>


vector<daemonMet*> daemonMet::allDaemons;
vector<processMet*> processMet::allProcs;
vector<visiMet*> visiMet::allVisis;
vector<tunableMet*> tunableMet::allTunables;

static vector<string_list*> all_strings;

void add_string_list(string& name, vector<string>& elem) {
  unsigned size = all_strings.size();
  for (unsigned u=0; u<size; u++)
    if (all_strings[u]->name == name) {
      all_strings[u]->name = name; all_strings[u]->elements = elem; return;
    }
  string_list *sl = new string_list;
  sl->name = name; sl->elements = elem;
  all_strings += sl;
}

metError metParseError = ERR_NO_ERROR;
const char *metParseError_list[6] = {"No error",
				     "Name in use, by different type",
				     "Name in use, by same type",
				     "Name in use",
				     "Bad arguments",
				     "Malloc",
				   };

void handle_error() 
{
  metParseError = ERR_NO_ERROR;
}

void tunableMet::dumpAll()
{
  unsigned size = allTunables.size();
  for (unsigned u=0; u<size; u++)
    allTunables[u]->dump();
}

void tunableMet::dump() const
{
  cout << "TUNABLE:  name " << name_;
  if (useBvalue) {
    cout << "   value: " << (Bvalue_ ? "TRUE" : "FALSE") << endl;
  } else {
    cout << "   value: " << Fvalue_ << endl;
  }
}

void tunableMet::addHelper(tunableMet *add_me) {
  unsigned size = allTunables.size();

  for (unsigned u=0; u<size; u++) {
    if (allTunables[u]->name() == add_me->name()) {
      delete allTunables[u];
      allTunables[u] = add_me;
      return;
    }
  }
  allTunables += add_me;
}

bool tunableMet::addTunable(string &c, float f)
{
  tunableMet *tm = new tunableMet(f, c);
  if (!tm) {
    metParseError = ERR_MALLOC;
    return false;
  }
  addHelper(tm);
  return true;
}

bool tunableMet::addTunable(string &c, int b)
{
  tunableMet *tm = new tunableMet(b, c);
  if (!tm) {
    metParseError = ERR_MALLOC;
    return false;
  }
  addHelper(tm);
  return true;
}

bool processMet::set_field (field &f)
{
  switch (f.spec) {
  case SET_COMMAND:
    command_ = *f.val;
    break;
  case SET_DAEMON:
    daemon_ = *f.val;
    break;
  case SET_HOST:
    host_ = *f.val;
    break;
  case SET_USER:
    user_ = *f.val;
    break;
  case SET_DIR:
    execDir_ = *f.val;
    break;
  case SET_NAME:
    name_ = *f.val;
    break;
  default:
    return false;
  }
  return true;
}

bool daemonMet::set_field (field &f)
{
  switch (f.spec) {
  case SET_COMMAND:
    command_ = *f.val;
    break;
  case SET_USER:
    user_ = *f.val;
    break;
  case SET_DIR:
    execDir_ = *f.val;
    break;
  case SET_FLAVOR:
    flavor_ = *f.flav;
    break;
  case SET_NAME:
    name_ = *f.val;
    break;
  case SET_HOST:
    host_ = *f.val;
    break;
  default:
    return false;
  }
  return true;
}

bool visiMet::set_field(field &f)
{
  switch (f.spec) {
  case SET_COMMAND:
    command_ = *f.val;
    break;
  case SET_HOST:
    host_ = *f.val;
    break;
  case SET_USER:
    user_ = *f.val;
    break;
  case SET_DIR:
    execDir_ = *f.val;
    break;
  case SET_NAME:
    name_ = *f.val;
    break;
  case SET_FORCE:
    force_ = f.force;
    if((force_ < 0) || (force_ > 1)){
      force_ = 0;
    }
    break;
  case SET_LIMIT:
    limit_ = f.limit;
    break;
  default:
    return false;
  }
  return true;
}

daemonMet::daemonMet(string &nm, string &cmd, string &exec, string &u,
		     string &h, string& flav)
: name_(nm), command_(cmd), execDir_(exec), user_(u), host_(h), flavor_(flav) { }

void daemonMet::dumpAll() {
  unsigned size = allDaemons.size();
  for (unsigned u=0; u<size; u++)
    allDaemons[u]->dump();
}

void daemonMet::dump() const
{
  cout << "DAEMONMET: " << name_ << endl;
  cout << "     command: " << command_ << endl;
  cout << "     execDir: " << execDir_ << endl;
  cout << "     host: " << host_ << endl;
  cout << "     flavor: " << flavor_ << endl;
  cout << "     user: " << user_ << endl;
}

bool daemonMet::addDaemon(daemonMet *dm)
{
  if (!dm || !dm->command_.length() || !dm->name_.length()) {
    metParseError = ERR_BAD_ARGS;
    if (dm) delete dm; dm = NULL;
    return false;
  }

  unsigned size = allDaemons.size();
  for (unsigned u=0; u<size; u++) {
    if (dm->name() == allDaemons[u]->name()) {
      delete allDaemons[u];
      allDaemons[u] = dm;
      return true;
    }
  }
  allDaemons += dm;
  return true;
}

processMet::processMet(string &nm, string &cmd, string &d, string &h,
		       string &u, string &exec)
: name_(nm), command_(cmd), daemon_(d), host_(h), user_(u), execDir_(exec) { }

void processMet::dumpAll()
{
  unsigned size = allProcs.size();
  for (unsigned u=0; u<size; u++)
    allProcs[u]->dump();
}

void processMet::dump() const
{
  cout << "PROCESSMET: " << name_ << endl;
  cout << "    command: " << command_ << endl;
  cout << "    daemon: " << daemon_ << endl;
  cout << "    host: " << host_ << endl;
  cout << "    execDir: " << execDir_ << endl;
  cout << "    user: " << user_ << endl;
}

bool processMet::addProcess(processMet *pm)
{
  if (!pm || !pm->name || !pm->command) {
    metParseError = ERR_BAD_ARGS;
    if (pm) delete pm; pm = NULL;
    return false;
  }

  unsigned size = allProcs.size();
  for (unsigned u=0; u<size; u++) {
    if (allProcs[u]->name() == pm->name()) {
      delete allProcs[u]; 
      allProcs[u] = pm;
      return true;
    }
  }
  allProcs += pm;
  return true;
}

visiMet::visiMet(string &nm, string &u, string &h, string &e, string &c, 
		int &f,int &l) 
  : name_(nm), user_(u), host_(h), execDir_(e) { }

void visiMet::dumpAll()
{
  unsigned size = allVisis.size();
  for (unsigned u=0; u<size; u++)
    allVisis[u]->dump();
}

void visiMet::dump() const
{
  cout << "VISIMET: " << name_ << endl;
  cout << "    command: " << command_ << endl;
  cout << "    host: " << host_ << endl;
  cout << "    execDir: " << execDir_ << endl;
  cout << "    user: " << user_ << endl;
}

bool visiMet::addVisi(visiMet *vm)
{
  if (!vm || !vm->name || !vm->command) {
    metParseError = ERR_BAD_ARGS;
    delete vm; vm = 0;
    return false;
  }
  unsigned size = allVisis.size();
  for (unsigned u=0; u<size; u++) {
    if (allVisis[u]->name() == vm->name()) {
      delete allVisis[u];
      allVisis[u] = vm;
      return true;
    }
  }
  allVisis += vm;
  return true;
}

