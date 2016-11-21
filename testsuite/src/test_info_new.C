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

#include "test_info_new.h"
#include "module.h"
#include <assert.h>
#include <stdio.h>

static const char *extract_name(const char *tag, const char *label)
{
   const char *start = strstr(label, tag);
   assert(start);
   start += strlen(tag);

   unsigned int size = 0;
   while (start[size] != ',' && start[size] != '\0') size++;
   assert(size);
   
   char *result = (char *) malloc(size+1);
   strncpy(result, start, size);
   result[size] = '\0';

   return result;
}


// The constructor for TestInfo
TestInfo::TestInfo(unsigned int i, const char *iname, const char *imrname,
                   const char *isoname, bool _serialize_enable, const char *ilabel) :
	index(i), name(iname), mutator_name(imrname), soname(isoname),
	serialize_enable(_serialize_enable),
	label(ilabel), mutator(NULL), disabled(false), limit_disabled(false),
	enabled(false), result_reported(false)
{
   assert(name);
   assert(mutator_name);
   assert(label);
   assert(soname);

	for (unsigned i=0; i<NUM_RUNSTATES; i++)
   {
      results[i] = UNKNOWN;
   }
}

TestInfo::TestInfo(unsigned int i, const char *libsuffix, const char *ilabel) :
   index(i),
   serialize_enable(false),
   label(ilabel),
   mutator(NULL),
   disabled(false),
   limit_disabled(false),
   enabled(false),
   result_reported(false)
{
   name = extract_name("test: ", label);
   mutator_name = extract_name("mutator: ", label);

   char *temp_soname = (char *) malloc(strlen(mutator_name) + strlen(libsuffix) + 1);
   strcpy(temp_soname, mutator_name);
   strcat(temp_soname, libsuffix);
   soname = temp_soname;
   assert(name);
   assert(mutator_name);
   assert(label);
   assert(soname);

   for (unsigned i=0; i<NUM_RUNSTATES; i++)
   {
      results[i] = UNKNOWN;
   }
}
           

// Constructor for RunGroup, with an initial test specified
RunGroup::RunGroup(const char *mutatee_name, start_state_t state_init,
                   create_mode_t attach_init, 
                   test_threadstate_t threads_, test_procstate_t procs_, 
                   run_location_t mutator_location_, run_location_t mutatee_location_, 
                   mutatee_runtime_t mutatee_runtime_,
                   test_linktype_t linktype_,
                   bool ex,
                   test_pictype_t pic_,
                   TestInfo *test_init,
                   const char *modname_, const char *compiler_, const char *optlevel_, 
                   const char *abi_, const char *platmode_)
  : mutatee(mutatee_name), state(state_init), createmode(attach_init),
    customExecution(ex), selfStart(false), index(0), disabled(false), connection(false),
    mutator_location(mutator_location_), mutatee_location(mutatee_location_),
    mutatee_runtime(mutatee_runtime_),
    mod(NULL), modname(modname_),
    threadmode(threads_), procmode(procs_),
    linktype(linktype_),
    pic(pic_),
    compiler(compiler_), optlevel(optlevel_), abi(abi_), platmode(platmode_)
{
  tests.push_back(test_init);
}

// Constructor for RunGroup with no initial test specified
RunGroup::RunGroup(const char *mutatee_name, start_state_t state_init,
                   create_mode_t attach_init, 
                   test_threadstate_t threads_, test_procstate_t procs_,
                   run_location_t mutator_location_, run_location_t mutatee_location_, 
                   mutatee_runtime_t mutatee_runtime_,
                   test_linktype_t linktype_,
                   bool ex,
                   test_pictype_t pic_,
                   const char *modname_,
                   const char *compiler_, const char *optlevel_, 
                   const char *abi_, const char *platmode_)
  : mutatee(mutatee_name), state(state_init), createmode(attach_init),
    customExecution(ex), selfStart(false), index(0), disabled(false), connection(false),
    mutator_location(mutator_location_), mutatee_location(mutatee_location_),
    mutatee_runtime(mutatee_runtime_),
    mod(NULL), modname(modname_),
    threadmode(threads_), procmode(procs_),
    linktype(linktype_),
    pic(pic_),
    compiler(compiler_), optlevel(optlevel_), abi(abi_), platmode(platmode_)
{
}

// Constructor for RunGroup with no initial test specified
RunGroup::RunGroup(const char *mutatee_name, 
                   start_state_t state_init,
                   create_mode_t attach_init, 
                   bool ex,
                   const char *modname_, 
                   test_pictype_t pic_,
                   const char *compiler_, 
                   const char *optlevel_, 
                   const char *abi_,
                   const char *platmode_)
  : mutatee(mutatee_name), state(state_init), createmode(attach_init),
    customExecution(ex), selfStart(false), index(0), disabled(false), connection(false),
    mutator_location(local), mutatee_location(local), mutatee_runtime(pre),
    mod(NULL), modname(modname_),
    threadmode(TNone), procmode(PNone),
    linktype(DynamicLink),
    pic(pic_),
    compiler(compiler_), optlevel(optlevel_), abi(abi_), platmode(platmode_)
{
}

// RunGroup's destructor clears its vector of tests
RunGroup::~RunGroup() {
   assert(0);
}

TestInfo::~TestInfo() {
   assert(0);
}

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

unsigned long UsageMonitor::memUsage() const
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
