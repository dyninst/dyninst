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

// $Id: mdlClass.C,v 1.1 2003/06/07 12:39:44 pcroth Exp $

// Implements classes used for metric description language

#include "pdutil/h/mdlParse.h"
#include <assert.h>


pdvector<daemonMet*> daemonMet::allDaemons;
pdvector<processMet*> processMet::allProcs;
pdvector<visiMet*> visiMet::allVisis;
pdvector<tunableMet*> tunableMet::allTunables;

static pdvector<string_list*> all_strings;

void add_string_list(string& name, pdvector<string>& elem) {
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
  if (useBvalue()) {
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

bool tunableMet::addTunable(string &c, bool b)
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
  case SET_AUTO_START:
	autoStart_ = f.bval;
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
  case SET_REMSH:
	remoteShell_ = *f.val;
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
  case SET_METFOCUS:
    if (metfocus_ == NULL)
    	metfocus_ = new pdvector<string>;
    (*metfocus_) += *f.val;
    break;
  default:
    return false;
  }
  return true;
}

daemonMet::daemonMet(string &nm, string &cmd, string &remsh, string &exec, 
					 string &u, string &h, string& flav)
: name_(nm), command_(cmd), remoteShell_(remsh), execDir_(exec), user_(u), 
  host_(h), flavor_(flav) { }

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
		       string &u, string &exec, bool auto_start)
: name_(nm), command_(cmd), daemon_(d), host_(h), user_(u), execDir_(exec),
  autoStart_(auto_start) { }

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
  cout << "    autoStart: ";
  if( autoStart_ )
	  cout << "true" << endl;
  else
	  cout << "false" << endl;
}

bool processMet::addProcess(processMet *pm)
{
  if ((pm == NULL) || (pm->name() == NULL) || (pm->command() == NULL)) {
    metParseError = ERR_BAD_ARGS;
    delete pm;
	pm = NULL;
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

visiMet::visiMet(string &nm, string &u, string &h, string &e, string &, // c
                 int &,int &) // f, l
  : name_(nm), user_(u), host_(h), execDir_(e) { metfocus_ = NULL; }

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
  if ((vm == NULL) || (vm->name() == NULL) || (vm->command() == NULL)) {
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

