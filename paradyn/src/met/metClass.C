
/*
 * Implements classes used for metric description language
 *
 * $Log: metClass.C,v $
 * Revision 1.3  1995/02/07 21:59:50  newhall
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

List<stringList*> stringList::allLists;
List<daemonMet*> daemonMet::allDaemons;
List<processMet*> processMet::allProcs;
List<visiMet*> visiMet::allVisis;
List<tunableMet*> tunableMet::allTunables;

metError metParseError = ERR_NO_ERROR;
char *metParseError_list[6] = {"No error",
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

void daemonMet::destroy()
{
  if (command)
    delete command;
  if (execDir)
    delete execDir;
  if (user)
    delete user;
  if (name)
    delete name;
  if (host)
    delete host;
  host = 0;
  command = 0;
  execDir = 0;
  user = 0;
  name = 0;
}

void visiMet::destroy ()
{
  if (command)
    delete command;
  if (host)
    delete host;
  if (user)
    delete user;
  if (execDir)
    delete execDir;
  if (name)
    delete name;
  name = 0;
  command = 0;
  host = 0;
  user = 0;
  execDir = 0;
}

void processMet::destroy ()
{
  if (command)
    delete command;
  if (daemon)
    delete daemon;
  if (host)
    delete host;
  if (execDir)
    delete execDir;
  if (user)
    delete user;
  if (name)
    delete name;
  command = 0;
  daemon = 0;
  host = 0;
  execDir = 0;
  user = 0;
  name = 0;
}

void tunableMet::dumpAll()
{
  List<tunableMet*> walk;
  for (walk=allTunables; *walk; walk++) {
    (*walk)->dump();
  }
}

void tunableMet::dump() const
{
  cout << "TUNABLE:  name " <<  (name ? name : "<EMPTY>");
  if (useBvalue) {
    cout << "   value: " << (Bvalue ? "TRUE" : "FALSE") << endl;
  } else {
    cout << "   value: " << Fvalue << endl;
  }
}

int tunableMet::addHelper(char *c)
{
  List<tunableMet*> walk;
  if (!c) {
    metParseError = ERR_BAD_ARGS;
    return 0;
  }
  for (walk=allTunables; *walk; walk++)
    if (!strcmp((*walk)->name, c))
      allTunables.remove(*walk);
  return 1;
}

int tunableMet::addTunable(char *c, float f)
{
  if (!addHelper(c))
    return 0;

  tunableMet *tm = new tunableMet(f, c);
  if (!tm) {
    metParseError = ERR_MALLOC;
    delete c;
    return 0;
  }
  allTunables.add(tm);
  return 1;
}

int tunableMet::addTunable(char *c, int b)
{
  if (!addHelper(c))
    return 0;

  tunableMet *tm = new tunableMet(b, c);
  if (!tm) {
    metParseError = ERR_MALLOC;
    delete c;
    return 0;
  }
  allTunables.add(tm);
  return 1;
}

int processMet::set_field (field &f)
{
  switch (f.spec) {
  case SET_COMMAND:
    if (command) delete command;
    command = f.val;
    break;
  case SET_DAEMON:
    if (daemon) delete daemon;
    daemon = f.val;
    break;
  case SET_HOST:
    if (host) delete host;
    host = f.val;
    break;
  case SET_USER:
    if (user) delete user;
    user = f.val;
    break;
  case SET_DIR:
    if (execDir) delete execDir;
    execDir = f.val;
    break;
  case SET_NAME:
    if (name) delete name;
    name = f.val;
    break;
  default:
    return 0;
  }
  return 0;
}

int daemonMet::set_field (field &f)
{
  switch (f.spec) {
  case SET_COMMAND:
    if (command) delete command;
    command = f.val;
    break;
  case SET_USER:
    if (user) delete user;
    user = f.val;
    break;
  case SET_DIR:
    if (execDir) delete execDir;
    execDir = f.val;
    break;
  case SET_FLAVOR:
    flavor = f.flav;
    break;
  case SET_NAME:
    if (name) delete name;
    name = f.val;
    break;
  case SET_HOST:
    if (host) delete host;
    host = f.val;
    break;
  default:
    return 0;
  }
  return 1;
}

int visiMet::set_field(field &f)
{
  switch (f.spec) {
  case SET_COMMAND:
    if (command) delete command;
    command = f.val;
    break;
  case SET_HOST:
    if (host) delete host;
    host = f.val;
    break;
  case SET_USER:
    if (user) delete (user);
    user = f.val;
    break;
  case SET_DIR:
    if (execDir) delete execDir;
    execDir = f.val;
    break;
  case SET_NAME:
    if (name) delete (name);
    name = f.val;
    break;
  case SET_FORCE:
    force = f.flav;
    if((force < 0) || (force > 1)){
      force = 0;
    }
    break;
  default:
    return 0;
  }
  return 1;
}

int stringList::addStringList(char *nm, List<char*> it)
{
  List<stringList*> walk;
  if (!nm) {
    metParseError = ERR_BAD_ARGS;
    it.removeAll();
    return 0;
  }
  for (walk=allLists; *walk; walk++)
    if (!strcmp((*walk)->name, nm))
      allLists.remove(*walk);

  stringList *sl = new stringList(nm, it);
  if (!sl) {
    metParseError = ERR_MALLOC;
    delete nm;
    it.removeAll();
  }
  allLists.add(sl);
  return 1;
}

void stringList::dump() const
{
  List<char*> walk;
  cout << "STRINGLIST: " << name << endl;
  for (walk=items; *walk; walk++) {
    cout << *walk << " : ";
  }
  cout << "}\n";
}

void stringList::dumpAll()
{
  List<stringList*> walk;
  for (walk=allLists; *walk; walk++)
    (*walk)->dump();
}

void daemonMet::dumpAll()
{
  List<daemonMet*> walk;
  for (walk=allDaemons; *walk; walk++)
    (*walk)->dump();
}

void daemonMet::dump() const
{
  cout << "DAEMONMET: " << (name ? name : "<EMPTY>") << endl;
  cout << "     command: " << (command ? command : "<EMPTY>") << endl;
  cout << "     execDir: " << (execDir ? execDir : "<EMPTY>") << endl;
  cout << "     host: " << (host ? host : "<EMPTY>") << endl;
  cout << "     flavor: " << flavor << endl;
  cout << "     user: " << (user ? user : "<EMPTY>") << endl;
}

int daemonMet::addDaemon(daemonMet *dm)
{
  List<daemonMet*> walk;

  if (!dm->command || !dm->name) {
    metParseError = ERR_BAD_ARGS;
    dm->destroy();
    return 0;
  }
  
  for (walk=allDaemons; *walk; walk++)
    if (!strcmp(dm->name, (*walk)->name))
      allDaemons.remove(*walk);
  allDaemons.add(dm);
  return 1;
}

void processMet::dumpAll()
{
  List<processMet*> walk;
  for (walk=allProcs; *walk; walk++)
    (*walk)->dump();
}

void processMet::dump() const
{
  cout << "PROCESSMET: " << (name ? name : "<EMPTY>") << endl;
  cout << "    command: " << (command ? command : "<EMPTY>") << endl;
  cout << "    daemon: " << (daemon ? daemon : "<EMPTY>") << endl;
  cout << "    host: " << (host ? host : "<EMPTY>")  << endl;
  cout << "    execDir: " << (execDir ? execDir : "<EMPTY>") << endl;
  cout << "    user: " << (user ? user : "<EMPTY>") << endl;
}

int processMet::addProcess(processMet *pm)
{
  List<processMet*> walk;

  if (!pm->name || !pm->command) {
    metParseError = ERR_BAD_ARGS;
    pm->destroy();
    return 0;
  }
  for (walk=allProcs; *walk; walk++)
    if (!strcmp((*walk)->name, pm->name))
      allProcs.remove(*walk);
  allProcs.add(pm);
  return 1;
}

void visiMet::dumpAll()
{
  List<visiMet*> walk;
  for (walk=allVisis; *walk; walk++)
    (*walk)->dump();
}

void visiMet::dump() const
{
  cout << "VISIMET: " << (name ? name : "<EMPTY>") << endl;
  cout << "    command: " << (command ? command : "<EMPTY>") << endl; 
  cout << "    host: " << (host ? host : "<EMPTY>") << endl;
  cout << "    execDir: " << (execDir ? execDir : "<EMPTY>") << endl;
  cout << "    user: " << (user ? user : "<EMPTY>") << endl;
}

int visiMet::addVisi(visiMet *vm)
{
  List<visiMet*> walk;
  if (!vm->name || !vm->command) {
    metParseError = ERR_BAD_ARGS;
    vm->destroy();
    return 0;
  }
  for (walk=allVisis; *walk; walk++)
    if (!strcmp((*walk)->name, vm->name))
      allVisis.remove(*walk);
  allVisis.add(vm);
  return 1;
}

