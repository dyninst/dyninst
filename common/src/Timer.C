// Timer.C

#include "util/h/Timer.h"

timer::timer()
: usecs_(0), ssecs_(0), wsecs_(0), cu_(0), cs_(0), cw_(0),
  state_(STOPPED),
  CYCLES_PER_SEC_(sysconf(_SC_CLK_TCK)), MICROSECS_PER_SEC_(1.0e6),
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





#if defined(sparc_sun_solaris2_4)
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

extern "C" gettimeofday(struct timeval *tp, struct timezone *tzp);

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
