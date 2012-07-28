/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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

#if !defined(MODULE_H_)
#define MODULE_H_

#include <map>
#include <vector>
#include <string>
#include <test_lib.h>

class RunGroup;
class ComponentTester;

class Module {
   bool creation_error;
   bool initialized;
   bool setup_run;
   bool remote;
   static std::map<std::string, Module *> localmods;
   static std::map<std::string, Module *> remotemods;
   static std::map<std::string, Module *> &mods(bool remote) { return remote ? remotemods : localmods; }

   Module(std::string name_, bool remote_);
   ComponentTester *loadModuleLibrary();
   void *libhandle;
public:
   std::string name;
   ComponentTester *tester;
   std::vector<RunGroup *> groups;

   TESTLIB_DLL_EXPORT bool isInitialized();
   TESTLIB_DLL_EXPORT void setInitialized(bool result);

   TESTLIB_DLL_EXPORT bool setupRun();
   TESTLIB_DLL_EXPORT void setSetupRun(bool result);

   TESTLIB_DLL_EXPORT static bool registerGroupInModule(std::string modname, RunGroup *group, bool remote);
   TESTLIB_DLL_EXPORT static void getAllModules(std::vector<Module *> &mods);
};

#endif
