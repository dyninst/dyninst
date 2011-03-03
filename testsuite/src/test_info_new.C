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

#include "test_info_new.h"
#include <assert.h>

int TestInfo::global_max_test_name_length = 10;
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
                   create_mode_t attach_init, 
                   test_threadstate_t threads_, test_procstate_t procs_, 
                   test_linktype_t linktype_,
                   bool ex,
                   test_pictype_t pic_,
                   TestInfo *test_init,
                   const char *modname_, const char *compiler_, const char *optlevel_, 
                   const char *abi_)
  : mutatee(mutatee_name), state(state_init), useAttach(attach_init),
    customExecution(ex), disabled(false), mod(NULL),
    threadmode(threads_), procmode(procs_),
    linktype(linktype_),
    pic(pic_),
    compiler(compiler_), optlevel(optlevel_), abi(abi_)
{
  Module::registerGroupInModule(std::string(modname_), this);
  tests.push_back(test_init);
}

// Constructor for RunGroup with no initial test specified
RunGroup::RunGroup(const char *mutatee_name, start_state_t state_init,
                   create_mode_t attach_init, 
                   test_threadstate_t threads_, test_procstate_t procs_,
                   test_linktype_t linktype_,
                   bool ex,
                   test_pictype_t pic_,
                   const char *modname_,
                   const char *compiler_, const char *optlevel_, 
                   const char *abi_)
  : mutatee(mutatee_name), state(state_init), useAttach(attach_init),
    customExecution(ex), disabled(false), mod(NULL),
    threadmode(threads_), procmode(procs_),
    linktype(linktype_),
    pic(pic_),
    compiler(compiler_), optlevel(optlevel_), abi(abi_)
{
   Module::registerGroupInModule(std::string(modname_), this);
}

// Constructor for RunGroup with no initial test specified
RunGroup::RunGroup(const char *mutatee_name, start_state_t state_init,
                   create_mode_t attach_init, 
                   bool ex,
                   test_pictype_t pic_,
                   const char *modname_,
                   const char *compiler_, const char *optlevel_, 
                   const char *abi_)
  : mutatee(mutatee_name), state(state_init), useAttach(attach_init),
    customExecution(ex), disabled(false), mod(NULL),
    threadmode(TNone), procmode(PNone),
    linktype(DynamicLink),
    pic(pic_),
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

int TestInfo::getMaxTestNameLength()
{
	return global_max_test_name_length;
}

void TestInfo::setMaxTestNameLength(int newlen)
{
	global_max_test_name_length = newlen;
}


std::map<std::string, Module *> Module::allmods;

#if !defined(os_windows_test)

#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>

#ifndef timersub
#define timersub(b, a, r) \
do { \
    (r)->tv_sec = (b)->tv_sec - (a)->tv_sec;\
    (r)->tv_usec = (b)->tv_usec - (a)->tv_usec;\
    if((r)->tv_usec < 0) {\
        (r)->tv_sec--;\
        (r)->tv_usec += 1000000;\
    } \
} while(0)
#endif

#ifndef timeradd
#define timeradd(b, a, r) \
do { \
    (r)->tv_sec = (b)->tv_sec + (a)->tv_sec;\
    (r)->tv_usec = (b)->tv_usec + (a)->tv_usec;\
    if((r)->tv_usec > 1000000) {\
        (r)->tv_sec += (r)->tv_usec / 1000000;\
        (r)->tv_usec = (r)->tv_usec % 1000000;\
    } \
} while(0)
#endif

UsageMonitor::useProcState UsageMonitor::use_proc = PS_UNKNOWN;

UsageMonitor::UsageMonitor()
{
    if (use_proc == PS_UNKNOWN) {
        struct stat s;
        if (stat("/proc/self/status", &s) == 0)
            use_proc = PS_USE;
        else
            use_proc = PS_SKIP;
    }
    clear();
}

void UsageMonitor::clear()
{
    total_mem = 0;
    total_cpu.tv_sec = 0;
    total_cpu.tv_usec = 0;
    state = UM_CLEAR;
}

void UsageMonitor::start()
{
    if (state == UM_COMPLETE) {
        fprintf(stderr, "*** Refusing to start completed UsageMonitor\n");
        return;
    }
    mark(&start_usage);
}

void UsageMonitor::end()
{
    if (state == UM_COMPLETE) return;

    struct rusage end_usage;
    mark(&end_usage);

    timersub(&end_usage.ru_utime, &start_usage.ru_utime, &end_usage.ru_utime);
    timeradd(&end_usage.ru_utime, &total_cpu, &total_cpu);
    timersub(&end_usage.ru_stime, &start_usage.ru_stime, &end_usage.ru_stime);
    timeradd(&end_usage.ru_stime, &total_cpu, &total_cpu);
    total_mem += (end_usage.ru_maxrss - start_usage.ru_maxrss);
    state = UM_HASDATA;
}

void UsageMonitor::set(timeval &cpu)
{
    total_cpu = cpu;
    state = UM_HASDATA;
}

void UsageMonitor::set(unsigned long mem)
{
    total_mem = mem;
    state = UM_HASDATA;
}

void UsageMonitor::complete()
{
    state = UM_COMPLETE;
}

bool UsageMonitor::has_data() const
{
    return state != UM_CLEAR;
}

const timeval &UsageMonitor::cpuUsage() const
{
    return total_cpu;
}

const unsigned long UsageMonitor::memUsage() const
{
    return total_mem;
}

UsageMonitor &UsageMonitor::operator=(const UsageMonitor &rhs)
{
    if (this != &rhs) {
        total_cpu = rhs.total_cpu;
        total_mem = rhs.total_mem;
        state = rhs.state;
    }
    return *this;
}

UsageMonitor &UsageMonitor::operator+=(const UsageMonitor &rhs)
{
    if (state != UM_CLEAR && rhs.state != UM_CLEAR) {
        total_mem += rhs.total_mem;
        if (state == UM_COMPLETE || rhs.state == UM_COMPLETE)
            state = UM_COMPLETE;
        else
            state = UM_HASDATA;
    }

    return *this;
}

const UsageMonitor UsageMonitor::operator+(const UsageMonitor &rhs) const
{
    return UsageMonitor(*this) += rhs;
}

void UsageMonitor::mark(struct rusage *ru)
{
    getrusage(RUSAGE_SELF, ru);
    if (!ru->ru_maxrss && use_proc != PS_SKIP) {
        unsigned long vmRSS  = 0;
        unsigned long vmSize = 0;

        FILE *fp = fopen("/proc/self/status", "r");
        if (!fp) return;

        char buf[1024] = {0};
        char *ptr = buf, *end = buf + sizeof(buf) - 1;
        while (!feof(fp) && !ferror(fp)) {
            int i = fread(ptr, sizeof(char), end - ptr, fp);
            ptr[i+1] = '\0';

            ptr = strstr(buf, "VmRSS:");
            if (ptr) sscanf(ptr, "VmRSS: %lu", &vmRSS);
            ptr = strstr(buf, "VmSize:");
            if (ptr) sscanf(ptr, "VmSize: %lu", &vmSize);

            if (!feof(fp) && !ferror(fp)) {
                ptr = strrchr(buf, '\n');
                if (!ptr++) break;

                for (i = 0; ptr + i < end; ++i) buf[i] = ptr[i];
                ptr = buf + i;
            }
        }
        fclose(fp);

        if (vmRSS)  ru->ru_maxrss = vmRSS;
        if (vmSize) ru->ru_ixrss  = vmSize;

        if (!vmRSS && !vmSize)
            use_proc = PS_SKIP;
    }
}
#endif
