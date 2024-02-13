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

// $Id: Timer.C,v 1.19 2007/05/30 19:20:16 legendre Exp $

#include "common/src/Timer.h"
#if defined(_MSC_VER)
#include <time.h>
#endif
timer::timer()
: usecs_(0), ssecs_(0), wsecs_(0), cu_(0), cs_(0), cw_(0),
  activation_count_(0),
#if defined(os_windows)
  CYCLES_PER_SEC_(CLOCKS_PER_SEC), // TODO: is this right?
#else
  CYCLES_PER_SEC_(sysconf(_SC_CLK_TCK)), 
#endif
  MICROSECS_PER_SEC_(1.0e6),
  NANOSECS_PER_SEC_(1.0e9)
{
}

timer::timer(const timer& t)
    : usecs_(t.usecs_), ssecs_(t.ssecs_), wsecs_(t.wsecs_),
    cu_(t.cu_), cs_(t.cs_), cw_(t.cw_), activation_count_(t.activation_count_),
    CYCLES_PER_SEC_(t.CYCLES_PER_SEC_), MICROSECS_PER_SEC_(t.MICROSECS_PER_SEC_),
    NANOSECS_PER_SEC_(t.NANOSECS_PER_SEC_)
{
}

timer::~timer() {}

timer&
timer::operator=(const timer& t) {
    if (this != &t) {
        usecs_ = t.usecs_; ssecs_ = t.ssecs_; wsecs_ = t.wsecs_;
        cu_    = t.cu_;    cs_    = t.cs_;    cw_    = t.cw_;
        activation_count_ = t.activation_count_;
    }
    return *this;
}

timer&
timer::operator+=(const timer& t) {
    timer st = t; st.stop();
    usecs_ += st.usecs_;
    ssecs_ += st.ssecs_;
    wsecs_ += st.wsecs_;
    return *this;
}

timer
timer::operator+(const timer& t) const {
    timer ret = *this;
    return ret += t;
}

void
timer::clear() {
    usecs_ = ssecs_ = wsecs_ = 0;
    cu_    = cs_    = cw_    = 0;
    activation_count_ = 0;
}

void
timer::start() {
    if (activation_count_ == 0)
        get_current(cu_, cs_, cw_);
    activation_count_++;
}

void
timer::stop() {
    if (activation_count_ == 0) return;

    activation_count_--;
    if (activation_count_ == 0) {
        double cu, cs, cw;
        get_current(cu, cs, cw);
        
        usecs_ += (cu - cu_);
        ssecs_ += (cs - cs_);
        wsecs_ += (cw - cw_);
    }
}

double
timer::usecs() const {
    return usecs_;
}

double
timer::ssecs() const {
    return ssecs_;
}

double
timer::wsecs() const {
    return wsecs_;
}

bool
timer::is_running() const {
    return (activation_count_ > 0);
}



/************************************************************************
 * architecture/operating system specific timer functions.
************************************************************************/




#undef HAVE_GET_CURRENT_DEFINITION



#if defined(os_windows)
#if !defined(HAVE_GET_CURRENT_DEFINITION)
#define HAVE_GET_CURRENT_DEFINITION

#include <sys/timeb.h>
#include <time.h>
#include <winbase.h>
#include <limits.h>

void
timer::get_current(double& u, double& s, double& w) {
  /*
    u = user time
    s = system time
    w = wall time
  */

  struct _timeb tb;
  _ftime(&tb);
  w = (double)tb.time + (double)tb.millitm/1000.0;

  FILETIME kernelT, userT, creatT, exitT;
  if (GetProcessTimes(GetCurrentProcess(), &creatT, &exitT, &kernelT, &userT)) {
    timer t;
    s = ((double)kernelT.dwHighDateTime * ((double)_UI32_MAX + 1.0)
        + (double)kernelT.dwLowDateTime)*100.0 / t.NANOSECS_PER_SEC();
    u = ((double)userT.dwHighDateTime * ((double)_UI32_MAX + 1.0)
        + (double)userT.dwLowDateTime)*100.0 / t.NANOSECS_PER_SEC();
  } else {
    u = 0;
    s = 0;
  }
}

#endif /* !defined(HAVE_GET_CURRENT_DEFINITION) */
#endif /* defined(os_windows) */




#if !defined(HAVE_GET_CURRENT_DEFINITION)
#define HAVE_GET_CURRENT_DEFINITION

#include <sys/types.h>
#include <sys/time.h>
#include <sys/times.h>

#if !defined(os_linux)
extern "C" int gettimeofday(struct timeval *tp, struct timezone *tzp);
#endif

void
timer::get_current(double& u, double& s, double& w) {
    struct tms     tb;
    struct timeval tv;
    if (times(&tb) == -1) {
        perror("times");
        abort();
    }
    if (gettimeofday(&tv, 0) == -1) {
      perror("gettimeofday");
      abort();
    }

    timer t;
    u = tb.tms_utime / t.CYCLES_PER_SEC();
    s = tb.tms_stime / t.CYCLES_PER_SEC();
    w = (tv.tv_sec + tv.tv_usec/t.MICROSECS_PER_SEC());
}
#endif /* !defined(HAVE_GET_CURRENT_DEFINITION) */
