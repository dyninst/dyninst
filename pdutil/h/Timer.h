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

#endif /* !defined(_Timer_h_) */
