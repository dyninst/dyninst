/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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

/************************************************************************
 * Timer.h: timer functions (for posix systems).
************************************************************************/


#if !defined(_Timer_h_)
#define _Timer_h_


/************************************************************************
 * header files.
************************************************************************/

#include <iostream.h>
#include "common/h/headers.h"


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
