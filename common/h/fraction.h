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

#ifndef __FRACTION_H
#define __FRACTION_H

#include "common/h/std_namesp.h"
#include "common/h/Types.h"

#if defined(i386_unknown_nt4_0)  || defined(mips_unknown_ce2_11) //ccw 30 mar 2001
        				// exception SPECIFICATIONS aren't
#pragma warning( disable : 4290 )   	// implemented yet on NT
#endif


class fraction {
  mutable int64_t numer;
  mutable int64_t denom;
  // largest multiplicand until an interim overflow will occur, and need to 
  // handle by different means
  mutable int64_t interimMultOverflowPt;  
  // largest multipland until a final overflow will occur, no way to handle
  // because result value is just to large to handle with an int64_t
  mutable int64_t   finalMultOverflowPt;
 public:
  typedef enum { sparse, verbose } ostream_fmt; 
  static ostream_fmt curFmt;

  fraction() : numer(0), denom(0), 
    interimMultOverflowPt(0), finalMultOverflowPt(0)  { }
  explicit fraction(int64_t n) : numer(n), denom(I64_C(1))  { 
    calcOverflowPts();
  }
  explicit fraction(int64_t n, int64_t d) : numer(n), denom(d)  { 
    calcOverflowPts();
  }
  // default copy constructor

  // Selectors
  int64_t getNumer() const {  return numer; }
  int64_t getDenom() const {  return denom; }
  // returns an integer representation of the fraction
  int64_t getI() const {
    return numer / denom;
  }
  // returns a double representation of the fraction
  double  getD() const { return (double) numer / (double) denom; }
  int64_t getInterimMultOverflowPt() const {  
    return interimMultOverflowPt; 
  }
  int64_t   getFinalMultOverflowPt() const {  
    return   finalMultOverflowPt; 
  }

/* A fast specialized multiplication of a fraction and an int64
   uses a special method that can keep from an interim overflow from occurring
   precise in regards to the result (doesn't round, ie. rounds result down).
   Returns int int64_t instead of a fraction, like the operator* does.
   If a final overflow occurs, prints an error message and returns the
   largest integer.
*/
  int64_t multReturnInt64(int64_t b) const;

  fraction reciprocal() const {  
    return fraction(denom, numer); 
  }
 private:
  int64_t multNoInterimOverflow(int64_t b) const {
    int64_t intdiv = b / getDenom();
    int64_t decrem = b % getDenom();
    return intdiv*getNumer() + decrem*getNumer()/getDenom();
  }

 public:
  // Mutators
  void setNumer(int64_t n) {
    numer = n;
    calcOverflowPts();
  }
  void setDenom(int64_t d) { 
    denom = d; 
    calcOverflowPts();
  }
 private:
  void setRaw(int64_t n, int64_t d) {
    numer = n;
    denom = d;
  }
 public:
  void set(int64_t n, int64_t d) { 
    setRaw(n,d);
    calcOverflowPts();
  }
  // This is const, because only modifying internal variables
  void calcOverflowPts() const;

  // A fraction is still logically unchanged when you reduce it, so consider
  // it a constant operation  eg. this will work:
  //   const fraction a(2,4);  a.reduce();
  void reduce() const;
};


ostream& operator<<(ostream&s, const fraction::ostream_fmt u);
ostream& operator<<(ostream&s, const fraction &z);

/* If you want these, feel free to write them
const fraction &operator+(const fraction &a, const fraction &b); {
const fraction &operator-(const fraction &a, const fraction &b);
*/

int64_t gcd(int64_t a, int64_t b);
int64_t lcd(int64_t a, int64_t b);

inline void fraction::reduce() const {
  int64_t igcd = gcd(numer, denom);
  numer = numer / igcd;
  denom = denom / igcd;
  calcOverflowPts();
}

// fraction * double
inline double operator*(const fraction &a, const double d) {
  return d * a.getD();
}
// double   * fraction
inline double operator*(const double d, const fraction &a) {
  return a * d;
}

// Upon an interim or final overflow, prints an error message and returns
// the largest fraction representable.
// fraction * int64_t
const fraction operator*(const fraction &a, int64_t b);
// int64_t  * fraction
inline const fraction operator*(int64_t a, const fraction &b) {
  return b * a;
}

// fraction + fraction
//const fraction operator+(const fraction &a, const fraction &b);
// fraction - fraction
//const fraction operator-(const fraction &a, const fraction &b);

// fraction * fraction
/* Feel free to implement if needed:
const fraction operator*(const fraction &a, const fraction &b); 
*/


inline const fraction operator/(const fraction &a, const fraction &b) {
  return fraction(a.getNumer() * b.getDenom(), a.getDenom() * b.getNumer());
}

inline bool operator==(const fraction &a, const fraction &b) {
  fraction ar = a;
  ar.reduce();
  fraction br = b;
  br.reduce();
  return ((ar.getNumer() == br.getNumer()) && (ar.getDenom() == br.getDenom()));
}
inline bool operator!=(const fraction &a, const fraction &b) {
  fraction ar = a;
  ar.reduce();
  fraction br = b;
  br.reduce();
  return ((ar.getNumer() != br.getNumer()) || (ar.getDenom() != br.getDenom()));
}

// getFrSpec is a helper routine for the gt, lt operators
// n : numerator,   d: denominator
// ra: returns the numerator high 32 bits / denom (always positive)
// rb: returns the numerator low  32 bits / denom (always positive)
// rsign: -1 for negative, 1 for positive result
void getFrSpec(int64_t n, int64_t d, double *ra, double *rb, int *rsign);
bool operator>(const fraction &a, const fraction &b);
bool operator<(const fraction &a, const fraction &b);
bool operator>=(const fraction &a, const fraction &b);
bool operator<=(const fraction &a, const fraction &b);




#endif
