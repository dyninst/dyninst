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

#include "common/h/fraction.h"
#include "common/h/int64iostream.h"

fraction::ostream_fmt fraction::curFmt = sparse;

/*
const fraction operator*(const fraction &a, const fraction &b) {
  if(b.getNumer() < a.getMultOverflowPt()) {
    return fraction(a.getNumer() * b.getNumer(), a.getDenom() * b.getDenom());
  } else {
    fraction c = a * b.getNumer();
    c.setDenom(c.getDenom()*b.getDenom());
    return c;
  }
};
*/

int64_t lcd(int64_t a, int64_t b) {
  int64_t _gcd = gcd(a,b);
  if(a > b) { a = a / _gcd; }
  else      { b = b / _gcd; }
  int64_t overflowPt = I64_MAX / b;
  if(a > overflowPt) {  cerr << "lcd: overflow\n";  return -1; }
  return a * b;
}

int64_t gcd(int64_t a, int64_t b) {
  if (b == 0) return a;
  return gcd(b, a % b);
}

// fraction + fraction
/*const fraction operator+(const fraction &a, const fraction &b) {
  if(a.getDenom() == b.getDenom()) {
    return fraction(a.getNumer() + b.getNumer(), a.getDenom());
  } else {
    int64_t ilcd = lcd(a.getDenom(), b.getDenom());
    return fraction(a.getNumer()*ilcd + b.getNumer()*ilcd, a.getDenom()*ilcd);
  }
}

// fraction - fraction
const fraction operator-(const fraction &a, const fraction &b) {
  if(a.getDenom() == b.getDenom()) {
    return fraction(a.getNumer() - b.getNumer(), a.getDenom());
  } else {
    int64_t ilcd = lcd(a.getDenom(), b.getDenom());
    return fraction(a.getNumer()*ilcd - b.getNumer()*ilcd, a.getDenom()*ilcd);
  }
}
*/

const fraction operator*(const fraction &a, int64_t b) {
  if(b < a.getInterimMultOverflowPt()) {
    //   b < interimMultOverflowPt
    return fraction(b * a.getNumer(), a.getDenom());
  } else if(b <= a.getFinalMultOverflowPt()) {
    //   interimMultOverflowPt <= b <= finalMultInterimPt
    cerr << "fraction::operator*- an interim overflow has occurred\n";
    return fraction(I64_MAX);
  } else {  //  finalMultInterimPt < b
    cerr << "fraction::operator*- a final overflow has occurred\n";
    return fraction(I64_MAX);
  }
}

int64_t fraction::multReturnInt64(int64_t b) const {
  int64_t ret = 0;
  if(b < getInterimMultOverflowPt()) {
    //   b < interimMultOverflowPt
    fraction fres(b * getNumer(), getDenom());
    ret = fres.getI();
  } else if(b <= getFinalMultOverflowPt()) {
    //   interimMultOverflowPt <= b <= finalMultInterimPt
    ret = multNoInterimOverflow(b);
  } else {  //  finalMultInterimPt < b
    cerr << "fraction::multReturnInt64- a final overflow has occurred\n";    
    return I64_MAX;
  }
  return ret;
}

void fraction::calcOverflowPts() const {
  if(getNumer() == 0) 
    interimMultOverflowPt = I64_MAX;
  else 
    interimMultOverflowPt = I64_MAX / getNumer();
  fraction recip;
  recip.setRaw(getDenom(), getNumer());
  if(recip.getDenom()==0 || recip.getD() > 1.0) 
    finalMultOverflowPt = I64_MAX;
  else
    finalMultOverflowPt = recip.multNoInterimOverflow(I64_MAX);
}

// getFrSpec is a helper routine for the gt, lt operators
// n : numerator,   d: denominator
// ra: returns the numerator high 32 bits / denom (always positive)
// rb: returns the numerator low  32 bits / denom (always positive)
// rsign: -1 for negative, 1 for positive result
void getFrSpec(int64_t n, int64_t d, double *ra, double *rb, int *rsign) {
  int sign = 1;
  if(n < 0) { n = -n;  sign = -sign; }
  if(d < 0) { d = -d;  sign = -sign; }
  *rsign = sign;
  int64_t upperI = n & I64_C(0xFFFFFFFF00000000);
  *ra = static_cast<double>(upperI) / static_cast<double>(d);
  int64_t lowerI = n & 0xFFFFFFFF;
  *rb = static_cast<double>(lowerI) / static_cast<double>(d);
}

// the gcd of the denominators could easily grow beyond the int64 max
// so implement using doubles
bool operator>(const fraction &a, const fraction &b) {
  double ax, ay;
  int as;
  getFrSpec(a.getNumer(), a.getDenom(), &ax, &ay, &as);
  if(as == -1) { ax =- ax; ay =-ay; }

  double bx, by;
  int bs;
  getFrSpec(b.getNumer(), b.getDenom(), &bx, &by, &bs);
  if(bs == -1) { bx =- bx; by =-by; }
  return (ax > bx || (ax == bx && ay > by));
}

bool operator<(const fraction &a, const fraction &b) {
  double ax, ay;
  int as;
  getFrSpec(a.getNumer(), a.getDenom(), &ax, &ay, &as);
  if(as == -1) { ax =- ax; ay =-ay; }

  double bx, by;
  int bs;
  getFrSpec(b.getNumer(), b.getDenom(), &bx, &by, &bs);
  if(bs == -1) { bx =- bx; by =-by; }
  return (ax < bx || (ax == bx && ay < by));
}

bool operator>=(const fraction &a, const fraction &b) {
  return ((a > b) || (a == b));
}

bool operator<=(const fraction &a, const fraction &b) {
  return ((a < b) || (a == b));
}

ostream& operator<<(ostream&s, const fraction::ostream_fmt u) {
  fraction::curFmt = u;
  return s;
}

ostream& operator<<(ostream&s, const fraction &z) {
  if(fraction::curFmt == fraction::sparse) {
    s << "(" << z.getNumer() << "/" << z.getDenom() << ")";
  } else { // fraction::verbose
    s << "(" << z.getNumer() << "/" << z.getDenom() << " - interimOvflw:";
    s << z.getInterimMultOverflowPt() << ", finalOvflw: " 
      << z.getFinalMultOverflowPt();
  }
  return s;
}


