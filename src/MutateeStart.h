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

#if !defined(MUTATEE_START_H_)
#define MUTATEE_START_H_

#include <vector>
#include <string>
#include "test_info_new.h"
#include "ParameterDict.h"
#include "dyntypes.h"

TESTLIB_DLL_EXPORT std::string launchMutatee(RunGroup *group, ParameterDict &params);
TESTLIB_DLL_EXPORT std::string launchMutatee(std::string executable, RunGroup *group, ParameterDict &params);
TESTLIB_DLL_EXPORT std::string launchMutatee(std::string executable, std::vector<std::string> &args, 
                          RunGroup *group, ParameterDict &params);

TESTLIB_DLL_EXPORT bool getMutateeParams(RunGroup *group, ParameterDict &params, std::string &exec_name,
                      std::vector<std::string> &args);
TESTLIB_DLL_EXPORT char **getCParams(const std::string &executable, const std::vector<std::string> &args);

//void setMutateeDict(RunGroup *group, ParameterDict &params);

TESTLIB_DLL_EXPORT void registerMutatee(std::string mutatee_string);
TESTLIB_DLL_EXPORT Dyninst::PID getMutateePid(RunGroup *group);

/**
 * setMutateeDict is a macro function because we want to objects created
 * by it to have a lifetime of the caller's scope.  
 **/
#define setMutateeDict(group, paramd) \
   ParamString mutatee_prm(group->mutatee); \
   ParamString platmode_prm(group->platmode); \
   ParamInt startstate_prm((int) group->state); \
   ParamInt createmode_prm((int) group->createmode); \
   ParamInt customexecution_prm((int) group->customExecution); \
   ParamInt selfstart_prm((int) group->selfStart); \
   ParamInt threadmode_prm((int) group->threadmode); \
   ParamInt procmode_prm((int) group->procmode); \
   ParamInt threadmd_prm((int) group->threadmode); \
   ParamInt procmd_prm((int) group->procmode);     \
   paramd["pathname"] = &mutatee_prm; \
   paramd["platmode"] = &platmode_prm; \
   paramd["startState"] = &startstate_prm; \
   paramd["createmode"] = &createmode_prm; \
   paramd["customExecution"] = &customexecution_prm; \
   paramd["selfStart"] = &selfstart_prm; \
   paramd["mt"] = &threadmd_prm; \
   paramd["mp"] = &procmd_prm;


#endif
