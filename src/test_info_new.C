/*
 * Copyright (c) 1996-2008 Barton P. Miller
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

#include "test_info_new.h"
#include <assert.h>

// The constructor for TestInfo
TestInfo::TestInfo(unsigned int i, const char *iname, const char *imrname,
                   const char *isoname, bool _serialize_enable, const char *ilabel) :
	index(i), name(iname), mutator_name(imrname), soname(isoname),
	serialize_enable(_serialize_enable),
	label(ilabel), mutator(NULL), disabled(false), enabled(false),
	result_reported(false)
{
   for (unsigned i=0; i<NUM_RUNSTATES; i++)
   {
      results[i] = UNKNOWN;
   }
}

// Constructor for RunGroup, with an initial test specified
RunGroup::RunGroup(const char *mutatee_name, start_state_t state_init,
                   create_mode_t attach_init, bool ex, TestInfo *test_init,
                   const char *modname_, const char *compiler_, const char *optlevel_, 
                   const char *abi_)
  : mutatee(mutatee_name), state(state_init), useAttach(attach_init),
    customExecution(ex), disabled(false), mod(NULL), 
    compiler(compiler_), optlevel(optlevel_), abi(abi_)
{
  Module::registerGroupInModule(std::string(modname_), this);
  tests.push_back(test_init);
}

// Constructor for RunGroup with no initial test specified
RunGroup::RunGroup(const char *mutatee_name, start_state_t state_init,
                   create_mode_t attach_init, bool ex, const char *modname_,
                   const char *compiler_, const char *optlevel_, 
                   const char *abi_)
  : mutatee(mutatee_name), state(state_init), useAttach(attach_init),
    customExecution(ex), disabled(false), mod(NULL),
    compiler(compiler_), optlevel(optlevel_), abi(abi_)
{
   Module::registerGroupInModule(std::string(modname_), this);
}

// RunGroup's destructor clears its vector of tests
RunGroup::~RunGroup() {
   assert(0);
}

TestInfo::~TestInfo() {
   assert(0);
}

Module::Module(std::string name_)
{
   name = name_;
   tester = loadModuleLibrary();
   creation_error = (tester == NULL);
   if (creation_error) {
      allmods[name] = NULL;
      return;
   }
      allmods[name] = this;
   initialized = true;
   setup_run = false;
}

bool Module::registerGroupInModule(std::string modname, RunGroup *group)
{
   assert(group);
   Module *mod = NULL;
   if (allmods.count(modname)) {
      mod = allmods[modname];
   }
   else {
      mod = new Module(modname);
      if (mod->creation_error) {
         delete mod;
         mod = NULL;
      }
   }

   group->mod = mod;
   if (!mod)
      return false;

   mod->groups.push_back(group);
   return true;
}


void Module::getAllModules(std::vector<Module *> &mods)
{
   mods.clear();
   std::map<std::string, Module *>::iterator i;
   for (i=allmods.begin(); i!=allmods.end(); i++)
   {
      if ((*i).second)
      {
         mods.push_back((*i).second);
      }
   }
}

bool Module::setupRun()
{
   return setup_run;
}

void Module::setSetupRun(bool result)
{
   setup_run = result;
}

bool Module::isInitialized()
{
  return initialized;
}

void Module::setInitialized(bool result)
{
  initialized = result;
}

std::map<std::string, Module *> Module::allmods;
