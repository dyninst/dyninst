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

#ifndef SAMPLE_H
#define SAMPLE_H

#include <iostream.h>
#include "common/h/Types.h"
#include "common/h/Time.h"


// -----------------------------------------------------------------------
// pdSample class definition ----------------------------------------
class pdSample {
 private:
  int64_t data;
  // This is only temporary, just needed for while paradyn in differnt states
  // as far as time type enhancement.  Future information like this should
  // probably be communicated through some type of sample subtypes.

  static const pdSample *_zero;
  static const int64_t MaxValue;
 public:
  static const pdSample &Zero();

  // Constructors
  pdSample()  {  }
  explicit pdSample(const int64_t v) : data(v) { }
  //  explicit pdSample(const pdCount v) : data(v.getValue()) {  }
  explicit pdSample(const timeLength &o) : data(o.get_ns()) {  }
  
  // General Assignment from like type
  /*
  pdSample& operator=(const pdSample &o) {
    if(this != &o) {
      this->assign(o);     }
    return *this; 
  }
  */
  
  // user-defined operator casts are very bad, More Effective C++ ch.5
  // so use non-operator cast function
  int64_t getValue() const { return data; }
  
  void assign(const int64_t v)  {  data = v;  }
  void assign(const pdSample &o) {  assign(o.getValue());  }
  void assign(const timeLength &o) {  assign(o.get_ns());  }
  
  pdSample& operator+=(const pdSample &a) {
    assign(data + a.getValue());
    return *this;
  }
  pdSample& operator-=(const pdSample &a) {
    assign(data - a.getValue());
    return *this;
  }
};

inline const pdSample &pdSample::Zero() {
  if(_zero == NULL) _zero = new pdSample(0);
  return *_zero;
}


// pdSample @ pdSample operators ----------------------------------
inline const pdSample operator+(const pdSample a, const pdSample b) {
  return pdSample(a.getValue() + b.getValue());
}
inline const pdSample operator-(const pdSample a, const pdSample b) {
  return pdSample(a.getValue() - b.getValue());
}
inline const pdSample operator*(const pdSample a, const pdSample b) {
  return pdSample(a.getValue() * b.getValue());
}
inline const pdSample operator/(const pdSample a, const pdSample b) {
  return pdSample(a.getValue() / b.getValue());
}

// pdSample * double 
inline const pdSample operator*(const pdSample a, const double b) {
  return pdSample(static_cast<int64_t>(static_cast<double>(a.getValue()) * b));
}
// double * pdSample
inline const pdSample operator*(const double a, const pdSample b) {
  return pdSample(static_cast<int64_t>(a * static_cast<double>(b.getValue())));
}
// pdSample / double 
inline const pdSample operator/(const pdSample a, const double b) {
  return pdSample(static_cast<int64_t>(static_cast<double>(a.getValue()) / b));
}
// double / pdSample
inline const pdSample operator/(const double a, const pdSample b) {
  return pdSample(static_cast<int64_t>(a / static_cast<double>(b.getValue())));
}


inline bool operator==(const pdSample a, const pdSample b) {
  return (a.getValue() == b.getValue());
}
inline bool operator!=(const pdSample a, const pdSample b) {
  return (a.getValue() != b.getValue());
}
inline bool operator>(const pdSample a, const pdSample b) {
  return (a.getValue() > b.getValue());
}
inline bool operator>=(const pdSample a, const pdSample b) {
  return (a.getValue() >= b.getValue());
}
inline bool operator<(const pdSample a, const pdSample b) {
  return (a.getValue() < b.getValue());
}
inline bool operator<=(const pdSample a, const pdSample b) {
  return (a.getValue() <= b.getValue());
}

ostream& operator<<(ostream&s, const pdSample &sm);

#endif











