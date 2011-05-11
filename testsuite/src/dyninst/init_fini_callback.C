/*
 * Copyright (c) 1996-2011 Barton P. Miller
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


#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "dyninst_comp.h"
#include <unistd.h>
#include <fcntl.h>

class init_fini_callback_Mutator : public DyninstMutator {
   virtual test_results_t setup(ParameterDict &param);
   virtual test_results_t executeTest();
   virtual test_results_t postExecution();
};

extern "C" DLLEXPORT  TestMutator *init_fini_callback_factory()
{
    return new init_fini_callback_Mutator();
}

static int unique_id;

test_results_t init_fini_callback_Mutator::setup(ParameterDict &param)
{
   unique_id = param["unique_id"]->getInt();
   return DyninstMutator::setup(param);
}

test_results_t init_fini_callback_Mutator::postExecution()
{
    // verify file output
   char filename[256];
   snprintf(filename, 256, "init_fini_log.%d", unique_id);

    int fd = open(filename, O_RDONLY);
    if(fd == -1) {
       snprintf(filename, 256, "binaries.%d/init_fini_log.%d", unique_id, unique_id);
       fd = open(filename, O_RDONLY);
        if(fd == -1)
        {
            logerror("FAILED: couldn't open init_fini_log after test\n");
            return FAILED;
        }
    }
    char buffer[2];
    read(fd, buffer, 2);
    close(fd);
    //unlink(filename);
    if(strncmp(buffer, "OK", 2) == 0)
    {
        return FAILED;
    }
    return FAILED;
}

//
// Start Test Case #11 - mutator side (snippets at entry,exit,call)
//

test_results_t init_fini_callback_Mutator::executeTest()
{
    //unlink("init_fini_log");
    BPatch_function* callinit_func = findFunction("entry_call", appImage, 1, "init_fini_callback");
    BPatch_function* callfini_func = findFunction("exit_call", appImage, 1, "init_fini_callback");
    
    int pointer_size = 0;
#if defined(arch_x86_64_test) || defined(ppc64_linux_test)
    pointer_size = pointerSize(appImage);
#endif
    const char *libNameAroot = "libtestA";
    char libNameA[128];
    strncpy(libNameA, libNameAroot, 127);
    addLibArchExt(libNameA,127, pointer_size);

    char libA[128];
    snprintf(libA, 128, "./%s", libNameA);

    if (!appAddrSpace->loadLibrary(libA))
    {
        logerror("**Failed test1_22 (findFunction in module)\n");
        logerror("  Mutator couldn't load %s into mutatee\n", libA);
        return FAILED;
    }

    BPatch_Vector<BPatch_module*> appModules;
    appImage->getModules(appModules);
    char buffer[80];
    test_results_t pass_fail = PASSED;
    
    for(unsigned int i = 0; i < appModules.size(); i++)
    {
        appModules[i]->getName(buffer, 80);
        if(!(strstr(buffer, "DEFAULT_MODULE")) && !(strstr(buffer, libNameA)))
        {
            continue;
        }
        BPatch_Vector<BPatch_snippet *> nameArgs;
        nameArgs.push_back(new BPatch_constExpr(buffer));
        BPatch_funcCallExpr callInitExpr(*callinit_func, nameArgs);
        BPatch_funcCallExpr callFiniExpr(*callfini_func, nameArgs);

        if(!appModules[i]->insertInitCallback(callInitExpr)) {
            logerror("**Failed** to insert init callback in module %s\n", buffer);
            pass_fail = FAILED;
        }
        else
        {
            logerror("**Succeeded** inserting init callback in module %s\n", buffer);
        }
        if(!appModules[i]->insertFiniCallback(callFiniExpr)) {
            logerror("**Failed** to insert fini callback in module %s\n", buffer);
            pass_fail = FAILED;
        }
        else
        {
            logerror("**Succeeded** inserting fini callback in module %s\n", buffer);
        }
    }
    return pass_fail;
}
