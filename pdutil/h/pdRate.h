/*
 * Copyright (c) 1996-1999 Barton P. Miller
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

#ifndef RATE_H
#define RATE_H

#include <iostream>
#include <assert.h>
#include <math.h>
#include "common/h/Time.h"
#include "common/h/headers.h"   // for isnan on NT
#include "pdutil/h/pdSample.h"

// -----------------------------------------------------------------------
// pdRate class definition ----------------------------------------
class pdRate {
 private:
  double data;

  static const pdRate *_zero;
  static const pdRate *_nan;
  static const pdRate *nanHelp();
 public:
  static const pdRate &Zero();
  static const pdRate &NaN();    // returns the not-a-number element of pdRate

  // Constructors
  pdRate()  { setNaN(); }
  explicit pdRate(const double v) : data(v) { }
  explicit pdRate(const pdSample s, const timeLength t, 
		  const timeUnit tu = timeUnit::ns()) {    
    if(s.isNaN())  {  setNaN(); }
    else {
      data = static_cast<double>(s.getValue()) / t.getD(tu); 
    }
  }

  // General Assignment from like type
  /*
  pdRate& operator=(const pdRate &o) {
    if(this != &o) {
      this->assign(o);     }
    return *this; 
  }
  */

  bool isNaN() const {  return (isnan(getValue()) != 0); }
  void setNaN() {  *this = NaN(); }
  
  // user-defined operator casts are very bad, More Effective C++ ch.5
  // so use non-operator cast function
  double getValue() const { return data; }
  
  void assign(const double v)  {  data = v;  }
  
  pdRate& operator+=(const pdRate &a) {
    assert(!a.isNaN());
    assign(data + a.getValue());
    return *this;
  }
  pdRate& operator-=(const pdRate &a) {
    assert(!a.isNaN());
    assign(data - a.getValue());
    return *this;
  }
};

inline const pdRate &pdRate::Zero() {
  if(_zero == NULL) _zero = new pdRate(0);
  return *_zero;
}

inline const pdRate &pdRate::NaN() {
  if(_nan == NULL) {  _nan = nanHelp(); }
  return *_nan;
}

// pdRate @ pdRate operators ----------------------------------
inline const pdRate operator+(const pdRate a, const pdRate b) {
  assert(!a.isNaN() && !b.isNaN());
  return pdRate(a.getValue() + b.getValue());
}
inline const pdRate operator-(const pdRate a, const pdRate b) {
  assert(!a.isNaN() && !b.isNaN());
  return pdRate(a.getValue() - b.getValue());
}
inline const pdRate operator*(const pdRate a, const pdRate b) {
  assert(!a.isNaN() && !b.isNaN());
  return pdRate(a.getValue() * b.getValue());
}
inline const pdRate operator/(const pdRate a, const pdRate b) {
  assert(!a.isNaN() && !b.isNaN());
  return pdRate(a.getValue() / b.getValue());
}

// assumes that the pdRate was formed with pdSample / timeLength.getD(ns())
// don't use if this isn't true
inline const pdSample operator*(const pdRate a, const timeLength b) {
  assert(!a.isNaN() && b.isInitialized());
  return pdSample(static_cast<int64_t>(a.getValue() * b.getD(timeUnit::ns())));
}

// pdRate * double 
inline const pdRate operator*(const pdRate a, const double b) {
  assert(!a.isNaN());
  return pdRate(a.getValue() * b);
}
// double * pdRate
inline const pdRate operator*(const double a, const pdRate b) {
  assert(!b.isNaN());
  return pdRate(a * b.getValue());
}
// pdRate / double 
inline const pdRate operator/(const pdRate a, const double b) {
  assert(!a.isNaN());
  return pdRate(a.getValue() / b);
}
// double / pdRate
inline const pdRate operator/(const double a, const pdRate b) {
  assert(!b.isNaN());
  return pdRate(a / b.getValue());
}


inline bool operator==(const pdRate a, const pdRate b) {
  assert(!a.isNaN() && !b.isNaN());
  return (a.getValue() == b.getValue());
}
inline bool operator!=(const pdRate a, const pdRate b) {
  assert(!a.isNaN() && !b.isNaN());
  return (a.getValue() != b.getValue());
}
inline bool operator>(const pdRate a, const pdRate b) {
  assert(!a.isNaN() && !b.isNaN());
  return (a.getValue() > b.getValue());
}
inline bool operator>=(const pdRate a, const pdRate b) {
  assert(!a.isNaN() && !b.isNaN());
  return (a.getValue() >= b.getValue());
}
inline bool operator<(const pdRate a, const pdRate b) {
  assert(!a.isNaN() && !b.isNaN());
  return (a.getValue() < b.getValue());
}
inline bool operator<=(const pdRate a, const pdRate b) {
  assert(!a.isNaN() && !b.isNaN());
  return (a.getValue() <= b.getValue());
}

ostream& operator<<(ostream&s, const pdRate &sm);

#endif
