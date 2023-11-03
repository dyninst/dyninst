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

/************************************************************************
 * Timer.h: timer functions (for posix systems).
************************************************************************/


#if !defined(_Timer_h_)
#define _Timer_h_


/************************************************************************
 * header files.
************************************************************************/
#include "common/src/headers.h"
#include "common/h/util.h"

/************************************************************************
 * class timer
************************************************************************/

class COMMON_EXPORT timer {
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

    double      usecs_, ssecs_, wsecs_;
    double      cu_, cs_, cw_;
    unsigned activation_count_;

     const double CYCLES_PER_SEC_;
     const double MICROSECS_PER_SEC_;
     const double NANOSECS_PER_SEC_;
};

#endif /* !defined(_Timer_h_) */
