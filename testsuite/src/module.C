/*
 * Copyright (c) 1996-2009 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "module.h"
#include "test_info_new.h"

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
