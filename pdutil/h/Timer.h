/************************************************************************
 * Timer.h: timer functions (for posix systems).
************************************************************************/





#if !defined(_Timer_h_)
#define _Timer_h_





/************************************************************************
 * header files.
************************************************************************/

#include <iostream.h>
#include "util/h/headers.h"





/************************************************************************
 * class timer
************************************************************************/

class timer {
public:
     timer ();
     timer (const timer &);
    ~timer ();

    timer&     operator= (const timer &);
    timer&    operator+= (const timer &);
    timer      operator+ (const timer &)                const;

    void           clear ();
    void           start ();
    void            stop ();

    double         usecs ()                             const;
    double         ssecs ()                             const;
    double         wsecs ()                             const;

    bool      is_running ()                             const;

    double CYCLES_PER_SEC() const { return CYCLES_PER_SEC_;}
    double NANOSECS_PER_SEC() const { return NANOSECS_PER_SEC_;}
    double MICROSECS_PER_SEC() const { return MICROSECS_PER_SEC_;}

     // friend
     // ostream&  operator<< (ostream &os, const timer &t) {
     // t.print(os); return os;}

private:
     // void           print (ostream &);

    static
    void     get_current (double &, double &, double &);

    enum timer_state { STOPPED, RUNNING };

    double      usecs_, ssecs_, wsecs_;
    double      cu_, cs_, cw_;
    timer_state state_;

     const double CYCLES_PER_SEC_;
     const double MICROSECS_PER_SEC_;
     const double NANOSECS_PER_SEC_;
};

inline
timer::timer()
: usecs_(0), ssecs_(0), wsecs_(0), cu_(0), cs_(0), cw_(0),
  state_(STOPPED),
  CYCLES_PER_SEC_(sysconf(_SC_CLK_TCK)), MICROSECS_PER_SEC_(1.0e6),
  NANOSECS_PER_SEC_(1.0e9)
{
}

inline
timer::timer(const timer& t)
    : usecs_(t.usecs_), ssecs_(t.ssecs_), wsecs_(t.wsecs_),
    cu_(t.cu_), cs_(t.cs_), cw_(t.cw_), state_(t.state_),
    CYCLES_PER_SEC_(t.CYCLES_PER_SEC_), MICROSECS_PER_SEC_(t.MICROSECS_PER_SEC_),
    NANOSECS_PER_SEC_(t.NANOSECS_PER_SEC_)
{
}

inline
timer::~timer() {
}

inline
timer&
timer::operator=(const timer& t) {
    if (this != &t) {
        usecs_ = t.usecs_; ssecs_ = t.ssecs_; wsecs_ = t.wsecs_;
        cu_    = t.cu_;    cs_    = t.cs_;    cw_    = t.cw_;
        state_ = t.state_;
    }
    return *this;
}

inline
timer&
timer::operator+=(const timer& t) {
    timer st = t; st.stop();
    usecs_ += st.usecs_;
    ssecs_ += st.ssecs_;
    wsecs_ += st.wsecs_;
    return *this;
}

inline
timer
timer::operator+(const timer& t) const {
    timer ret = *this;
    return ret += t;
}

inline
void
timer::clear() {
    usecs_ = ssecs_ = wsecs_ = 0;
    cu_    = cs_    = cw_    = 0;
    state_ = STOPPED;
}

inline
void
timer::start() {
    get_current(cu_, cs_, cw_);
    state_ = RUNNING;
}

inline
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

inline
double
timer::usecs() const {
    return usecs_;
}

inline
double
timer::ssecs() const {
    return ssecs_;
}

inline
double
timer::wsecs() const {
    return wsecs_;
}

inline
bool
timer::is_running() const {
    return (state_ == RUNNING);
}

#if defined(notdef)
inline
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





#if defined(sparc_sun_solaris2_3)
#if !defined(HAVE_GET_CURRENT_DEFINITION)
#define HAVE_GET_CURRENT_DEFINITION

#include <sys/time.h>
#include <sys/times.h>

inline
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
#endif /* defined(sparc_sun_solaris2_3) */





#if !defined(HAVE_GET_CURRENT_DEFINITION)
#define HAVE_GET_CURRENT_DEFINITION

#include <sys/types.h>
#include <sys/time.h>
#include <sys/times.h>

extern "C" gettimeofday(struct timeval *tp, struct timezone *tzp);

inline
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





#endif /* !defined(_Timer_h_) */
