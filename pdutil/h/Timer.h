/************************************************************************
 * Timer.h: timer functions (for posix systems).
************************************************************************/





#if !defined(_Timer_h_)
#define _Timer_h_





/************************************************************************
 * header files.
************************************************************************/

#include <iostream.h>
#include <unistd.h>





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

    friend
    ostream&  operator<< (ostream &, const timer &);

private:
    void           print (ostream &)                    const;

    static
    void     get_current (double &, double &, double &);

    enum timer_state { STOPPED, RUNNING };

    double      usecs_, ssecs_, wsecs_;
    double      cu_, cs_, cw_;
    timer_state state_;

    static const double CYCLES_PER_SEC;
    static const double MICROSECS_PER_SEC;
    static const double NANOSECS_PER_SEC;
};

inline
timer::timer()
    : usecs_(0), ssecs_(0), wsecs_(0), cu_(0), cs_(0), cw_(0),
    state_(STOPPED) {
}

inline
timer::timer(const timer& t)
    : usecs_(t.usecs_), ssecs_(t.ssecs_), wsecs_(t.wsecs_),
    cu_(t.cu_), cs_(t.cs_), cw_(t.cw_), state_(t.state_) {
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

inline
ostream&
operator<<(ostream& os, const timer& t) {
    t.print(os); return os;
}

inline
void
timer::print(ostream& os) const {
    timer_state ostate = state_;
    if (ostate == RUNNING) {
        ((timer *) this)->stop();
    }

    os << "{"
       << " usecs=" << usecs_
       << " ssecs=" << ssecs_
       << " wsecs=" << wsecs_
       << " }";

    if (ostate == RUNNING) {
        ((timer *) this)->start();
    }
}





/************************************************************************
 * architecture/operating system specific timer functions.
************************************************************************/





#undef HAVE_GET_CURRENT_DEFINITION

const double timer::CYCLES_PER_SEC    = sysconf(_SC_CLK_TCK);
const double timer::MICROSECS_PER_SEC = 1.0e6;
const double timer::NANOSECS_PER_SEC  = 1.0e9;





#if defined(sparc_sun_solaris2_3)
#if !defined(HAVE_GET_CURRENT_DEFINITION)
#define HAVE_GET_CURRENT_DEFINITION

#include <sys/time.h>
#include <sys/times.h>

inline
void
timer::get_current(double& u, double& s, double& w) {
    u = gethrvtime() / NANOSECS_PER_SEC;

    struct tms tb;
    if (times(&tb) == -1) {
        perror("times");
        abort();
    }
    s = tb.tms_stime / CYCLES_PER_SEC;

    w = gethrtime() / NANOSECS_PER_SEC;
}

#endif /* !defined(HAVE_GET_CURRENT_DEFINITION) */
#endif /* defined(sparc_sun_solaris2_3) */





#if !defined(HAVE_GET_CURRENT_DEFINITION)
#define HAVE_GET_CURRENT_DEFINITION

#include <sys/types.h>
#include <sys/time.h>
#include <sys/times.h>

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
        perror("gettimeofday");
        abort();
    }

    u = tb.tms_utime / CYCLES_PER_SEC;
    s = tb.tms_stime / CYCLES_PER_SEC;
    w = (tv.tv_sec + tv.tv_usec/MICROSECS_PER_SEC);
}
#endif /* !defined(HAVE_GET_CURRENT_DEFINITION) */





#endif /* !defined(_Timer_h_) */
