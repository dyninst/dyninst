/*
 * Copyright (c) 1996-2004 Barton P. Miller
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

// $Id: Timer.C,v 1.15 2005/02/24 10:14:44 rchen Exp $

#include "common/h/Timer.h"

timer::timer()
: usecs_(0), ssecs_(0), wsecs_(0), cu_(0), cs_(0), cw_(0),
  state_(STOPPED),
#if defined(i386_unknown_nt4_0) \
 || defined(mips_unknown_ce2_11) //ccw 20 july 2000 : 29 mar 2001
  CYCLES_PER_SEC_(CLK_TCK), // TODO: is this right?
#else
  CYCLES_PER_SEC_(sysconf(_SC_CLK_TCK)), 
#endif
  MICROSECS_PER_SEC_(1.0e6),
  NANOSECS_PER_SEC_(1.0e9)
{
}

timer::timer(const timer& t)
    : usecs_(t.usecs_), ssecs_(t.ssecs_), wsecs_(t.wsecs_),
    cu_(t.cu_), cs_(t.cs_), cw_(t.cw_), state_(t.state_),
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
        state_ = t.state_;
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
    state_ = STOPPED;
}

void
timer::start() {
    get_current(cu_, cs_, cw_);
    state_ = RUNNING;
}

void
timer::stop() {
    if (state_ == RUNNING) {
        double cu, cs, cw;
        get_current(cu, cs, cw);

        usecs_ += (cu - cu_);
        ssecs_ += (cs - cs_);
        wsecs_ += (cw - cw_);
        state_ = STOPPED;
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
    return (state_ == RUNNING);
}

#if defined(notdef)
void
timer::print(ostream& os) {
    timer_state ostate = state_;
    if (ostate == RUNNING) {
        stop();
    }

    os << "{"
       << " usecs=" << usecs_
       << " ssecs=" << ssecs_
       << " wsecs=" << wsecs_
       << " }";

    if (ostate == RUNNING) {
        start();
    }
}
#endif




/************************************************************************
 * architecture/operating system specific timer functions.
************************************************************************/





#undef HAVE_GET_CURRENT_DEFINITION




#if defined(i386_unknown_nt4_0) \
 || defined(mips_unknown_ce2_11) //ccw 20 july 2000 : 29 mar 2001
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
#endif /* defined(i386_unknown_nt4_0) */





#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_solaris2_5)
#if !defined(HAVE_GET_CURRENT_DEFINITION)
#define HAVE_GET_CURRENT_DEFINITION

#include <sys/time.h>
#include <sys/times.h>

void
timer::get_current(double& u, double& s, double& w) {
    timer t;
    u = gethrvtime() / t.NANOSECS_PER_SEC();

    struct tms tb;
    if (times(&tb) == -1) {
      P_perror("times");
      P_abort();
    }
    s = tb.tms_stime / t.CYCLES_PER_SEC();

    w = gethrtime() / t.NANOSECS_PER_SEC();
}

#endif /* !defined(HAVE_GET_CURRENT_DEFINITION) */
#endif /* defined(sparc_sun_solaris2_4) */





#if !defined(HAVE_GET_CURRENT_DEFINITION)
#define HAVE_GET_CURRENT_DEFINITION

#include <sys/types.h>
#include <sys/time.h>
#include <sys/times.h>

#if !defined(rs6000_ibm_aix4_1) \
 && !defined(i386_unknown_linux2_0) \
 && !defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 && !defined(alpha_dec_osf4_0) \
 && !defined(mips_sgi_irix6_4) \
 && !defined(ia64_unknown_linux2_4) \
 && !defined(sparc_sun_solaris2_7)
   // aix 4.1 and linux don't need or agree with the following declaration:
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
      P_perror("gettimeofday");
      P_abort();
    }

    timer t;
    u = tb.tms_utime / t.CYCLES_PER_SEC();
    s = tb.tms_stime / t.CYCLES_PER_SEC();
    w = (tv.tv_sec + tv.tv_usec/t.MICROSECS_PER_SEC());
}
#endif /* !defined(HAVE_GET_CURRENT_DEFINITION) */
