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

#ifndef __TIME_H
#define __TIME_H


#include "common/h/Types.h"
#include "common/h/fraction.h"

/* These user classes that are defined:
   timeUnit  - eg. microsecond, minute, year
               size: 32 bytes
   timeBase  - represents the time that a timeStamp is based from  (eg. 1970)
               size: 8 bytes
   timeStamp - represents a point in time, ie. a length of time from a timeBase
               size: 8 bytes
   timeLength- represents a length of time
               size: 8 bytes
*/

// timeUnit ---------------------------------------------------

// A timeUnit represents a unit of time and thus consists of a ratio of how
// many nanoseconds (the standard unit decided upon) per unit of the
// represented time unit.  The timeUnit class is used by the timeStamp and
// timeLength classes for converting the represented base to (via
// ns_per_unit) and from (via units_per_ns) nanoseconds.

class timeUnit {
 private:
  // logically const
  static const timeUnit *_leapYear, *_year, *_day, *_hour, *_min;
  static const timeUnit *_sec, *_ms, *_us, *_ns;
  static const timeUnit *nsHelp(),  *usHelp(),   *msHelp(),  *secHelp();
  static const timeUnit *minHelp(), *hourHelp(), *dayHelp(), *yearHelp();
  static const timeUnit *leapYearHelp();
 public:
  static const timeUnit &ns(), &us(), &ms(), &sec();
  static const timeUnit &min(), &hour(), &day(), &year(), &leapYear();

 public:
  typedef enum { sparse, verbose } ostream_fmt; 
  static ostream_fmt curFmt;

 private:
  fraction ns_per_unit;
  fraction units_per_ns;

 public:
  timeUnit(fraction _ns_per_unit) : ns_per_unit(_ns_per_unit), 
    units_per_ns(ns_per_unit.reciprocal()) {
    ns_per_unit.reduce();
    units_per_ns.reduce();
  }
  // default copy constructor

  // Selectors
  fraction get_ns_per_unit()  const {  return ns_per_unit;   }
  fraction get_units_per_ns() const {  return units_per_ns;  }

  // Mutators
  void set_ns_per_unit(const fraction &nspu) {
    ns_per_unit = nspu;
    units_per_ns = ns_per_unit.reciprocal();
  }
  int64_t cvtTo_ns(double  units) const;
  int64_t cvtTo_ns(int64_t units) const;
  double  cvtFrom_nsD(int64_t ns) const;
  int64_t cvtFrom_nsI(int64_t ns) const;
};

ostream& operator<<(ostream&s, const timeUnit::ostream_fmt &u);
ostream& operator<<(ostream&s, const timeUnit &u);

inline const timeUnit &timeUnit::ns() {
  if(_ns == NULL)  _ns = nsHelp();
  return *_ns;
}
inline const timeUnit &timeUnit::us() {
  if(_us == NULL)  _us = usHelp();
  return *_us;
}
inline const timeUnit &timeUnit::ms() {
  if(_ms==NULL)  _ms = msHelp();
  return *_ms;
}
inline const timeUnit &timeUnit::sec() {
  if(_sec==NULL) _sec = secHelp();
  return *_sec;
}
inline const timeUnit &timeUnit::min() {
  if(_min==NULL)  _min = minHelp();
  return *_min;
}
inline const timeUnit &timeUnit::hour() {
  if(_hour==NULL) _hour = hourHelp();
  return *_hour;
}
inline const timeUnit &timeUnit::day() {
  if(_day==NULL) _day = dayHelp();
  return *_day;
}
inline const timeUnit &timeUnit::year() {
  if(_year==NULL) _year = yearHelp();
  return *_year;
}
inline const timeUnit &timeUnit::leapYear() {
  if(_leapYear==NULL) _leapYear = leapYearHelp();
  return *_leapYear;
}

class timeStamp;

// timeBase ---------------------------------------------------

// Represents the starting time that a timeStamp time is from.  For example
// time from gettimeofday has a time base of 1970.  I have set up these
// constant time bases:
// b1970: represents Jan 1, 12:00:00am 1970
// bStd:  represents Jan 1, 12:00:00am 2000
// bNone: no base, sometimes want timeStamp that has no time base as in
//        process times.  eg. for process time we care only about the length
//        of the time not so much the start time (ie. base)

// A timeBase is stored as number of nanoseconds from the represented (start)
// time to bStd (ie. Jan 1, 12:00:00am 2000).  This class also provides
// functions that convert nanoseconds to and from the represented time base
// and the standard time base (bStd).  No this is not the base class of any
// of timeParent, timeStamp, timeLength.  Sorry for any confusion.

class timeBase {
 private:
  // change internal time base to 2100 in the year 2050
  static const timeBase *_bStd, *_b1970;  // logically const
  static const timeBase *bStdHelp(), *b1970Help();
 public:
  // ie. the year for the internal time base, if changed check to make
  // sure timeBase::b1970Help() is still accurate
  enum { StdBaseMark = 2000 };  
  static const timeBase &bStd();
  static const timeBase &b1970();
  // bNone is for times like process time, time is counted since the
  // start of the process representing the process time in regards to
  // a real time base (not unit) as in b1970 doesn't make sense
  static const timeBase &bNone();

 private:
  int64_t ns2StdBaseMark;  

 public:
  //Paradyn default base: from nearest century turnover

  timeBase(int64_t ns2stdMark) {
    ns2StdBaseMark = ns2stdMark;
  }
  timeBase(timeStamp mark);  // defined inline below
  int64_t get_ns2StdBaseMark() const {
    return ns2StdBaseMark;
  }
  int64_t cvtTo_bStd(int64_t ns) const {
    // eg. 1994, b1970 -> bStd:  24 yrs - 30 yrs = -6 yrs
    return ns - ns2StdBaseMark;  
  }
  double  cvtFrom_bStd(double ns) const {
    // eg. 1994, bStd -> b1970:  -6 yrs + 30 yrs = 24 yrs
    return ns + ns2StdBaseMark;
  }
  int64_t cvtFrom_bStd(int64_t ns) const {
    return ns + ns2StdBaseMark;
  }

};

ostream& operator<<(ostream&s, timeBase b);

inline const timeBase &timeBase::bStd() {
  if(_bStd == NULL) _bStd = bStdHelp();
  return *_bStd;
}
inline const timeBase &timeBase::b1970() {
  if(_b1970 == NULL) _b1970 = b1970Help();
  return *_b1970;
}
inline const timeBase &timeBase::bNone() {
  return bStd();
}

// Responsibilities:
//   - store time in a standard unit and time base, currently this is:
//        * standard unit:      nanoseconds
//        * standard time base: Jan 1, 12:00:00am 2000

// an abstract class
class timeParent {
 private:
  int64_t ns;

  // Constructors
 public:

  // Selectors
  int64_t get_ns() const {
    return ns;
  }

  // the disadvantage of having a timeParent operator<< via the help
  // of this virtual put function is that it causes a pointer to the vtable
  // to be carried around with all of our timeLengths and timeStamps
  // and they become less lightweight
  //  virtual ostream& put(ostream& s) const = 0;  // write *this to s

 protected:
  timeParent() { }
  timeParent(int64_t _ns) : ns(_ns) { }
  // Mutators
  void assign(const int64_t v)  {  ns = v;  }
  int64_t getRolloverTime(double t);
};

//ostream& operator<<(ostream& s, const timeParent &tp);

class timeLength;

// timeStamp ---------------------------------------------------

// A timeStamp represents a point in time.  To do this it stores the number
// of nanoseconds since the standard time base (Jan 1, 2000).  It provides a
// generic method of storing time.  It is based on a 64bit signed integer so
// it can represent any time at nanosecond granularity between year 2000 +-
// 292 years (ie. 1708-2292).

class timeStamp : public timeParent {
 private:
  static const timeStamp *_ts1970;
  static const timeStamp *_tsStd;  // ie. timeBase::InternalTimeBase (2000)
  static const timeStamp *_ts2200;
  static const timeStamp *ts1970Help();
  static const timeStamp *tsStdHelp();
  static const timeStamp *ts2200Help();
 public:
  static const timeStamp &ts1970();
  static const timeStamp &tsStd();
  static const timeStamp &ts2200();

  // need this constructor to use in vector container class
  timeStamp();
  // eg. to create the time Jan 1, 1995 12:00am you could do this:
  //    timeStamp myTime(25, timeUnit::year(), timeBase::b1970());
  // one way to create July 20, 1976 8:35am:
  //    timeStamp myBirthDay = timeStamp::b1970() + 4*timeLength::year() +
  //                           2*timeStamp::leapYear() +
  //                           202*timeLength::day() + 8*timeLength::hour() +
  //                           35*timeLength::min();
  timeStamp(int64_t iTime, const timeUnit &u, timeBase b) : timeParent() {
    initI(iTime, u, b);
  }
  timeStamp(int iTime, const timeUnit &u, timeBase b) : timeParent() {
    initI(iTime, u, b);
  }
  timeStamp(double dTime, const timeUnit &u, timeBase b);

  // Selectors
  double getD(const timeUnit &u, timeBase b) const {
    return u.cvtFrom_nsD( b.cvtFrom_bStd(get_ns()));
  }
  // eg. to get the number of seconds since 1970 do this: 
  //        ts.getI(timeUnit::sec(), timeBase::b1970()) 
  int64_t getI(const timeUnit &u, timeBase b) const {
    return u.cvtFrom_nsI( b.cvtFrom_bStd(get_ns()));
  }

  // ostream& put(ostream& s) const { return s << *this; }

  friend const timeStamp operator+=(timeStamp &ts, timeLength tl);
  friend const timeStamp operator-=(timeStamp &ts, timeLength tl);
  friend const timeLength operator+(const timeStamp a, const timeStamp b);
  friend const timeStamp operator+(const timeStamp a, const timeLength b);
  friend const timeStamp operator-(const timeStamp a, const timeLength b);
  friend const timeStamp operator+(const timeLength a, const timeStamp b);
  // non-member ==, !=, >, <, >=, <=  operators also defined for timeStamp

 private:
  void initI(int64_t iTime, const timeUnit &u, timeBase b);
  timeStamp(int64_t ns_) : timeParent(ns_) { }
};

ostream& operator<<(ostream&s, timeStamp z);

inline const timeStamp &timeStamp::ts1970() {
  if(_ts1970 == NULL)  _ts1970 = ts1970Help();
  return *_ts1970;
}

inline const timeStamp &timeStamp::tsStd() {
  if(_tsStd == NULL)  _tsStd = tsStdHelp();
  return *_tsStd;
}

inline const timeStamp &timeStamp::ts2200() {
  if(_ts2200 == NULL)  _ts2200 = ts2200Help();
  return *_ts2200;
}

// need to define this here since calls inlined ts1970
inline timeStamp::timeStamp() : timeParent() {
  *this = timeStamp::ts1970();
}

// timeLength ---------------------------------------------------

// A timeLength represents a length of time and has no notion of a start or
// base time.  It has a range of signed 64bit integer or about 292 years
// since we're counting nanoseconds.

class timeLength : public timeParent {
 private:
  // logically const
  static const timeLength *_zero, *_ns, *_us, *_ms, *_sec, *_min;
  static const timeLength *_hour, *_day, *_year, *_leapYear;
  static const timeLength *ZeroHelp(), *nsHelp(), *usHelp(), *msHelp();
  static const timeLength *secHelp(), *minHelp(), *hourHelp(), *dayHelp();
  static const timeLength *yearHelp(), *leapYearHelp();
 public:
  static const timeLength &Zero(), &ns(), &us(), &ms(), &sec();
  static const timeLength &min(), &hour(), &day(), &year(), &leapYear();

  timeLength(int64_t iTime, const timeUnit &u) : timeParent() {
    initI(iTime, u);
  }
  timeLength(int iTime, const timeUnit &u) : timeParent() {
    initI(static_cast<int64_t>(iTime), u);
  }
  timeLength(double dTime, const timeUnit &u);

  // Selectors
  double getD(const timeUnit &u) const {   return u.cvtFrom_nsD( get_ns());   }
  int64_t getI(const timeUnit &u) const {  return u.cvtFrom_nsI( get_ns());   }

  //ostream& put(ostream& s) const { return s << *this; }

  friend const timeLength operator+=(timeLength &t, timeLength tl);
  friend const timeLength operator-=(timeLength &t, timeLength tl);
  friend const timeLength operator-(const timeStamp a, const timeStamp b);
  friend const timeStamp operator+(const timeStamp a, const timeLength b);
  friend const timeStamp operator-(const timeStamp a, const timeLength b);
  friend const timeStamp operator+(const timeLength a, const timeStamp b);
  friend const timeLength operator+(const timeLength a, const timeLength b);
  friend const timeLength operator-(const timeLength a, const timeLength b);
  friend const timeLength operator*(const timeLength a, const double b);
  friend const timeLength operator/(const timeLength a, const double b);
  friend const timeLength operator*(const double a, const timeLength b);
  friend const timeLength operator/(const double a, const timeLength b);
  friend const double operator/(const timeLength a, const timeLength b);
  // non-member ==, !=, >, <, >=, <=  operators also defined for timeLength

 private:
  void initI(int64_t iTime, const timeUnit &u);
  // a fast constructor just for timeLength operators
  timeLength(int64_t ns_) : timeParent(ns_) { }
};

inline const timeLength &timeLength::Zero() {
  if(_zero == NULL) _zero = ZeroHelp();
  return *_zero;
}
inline const timeLength &timeLength::ns() {
  if(_ns == NULL) _ns = nsHelp();
  return *_ns;
}
inline const timeLength &timeLength::us() {
  if(_us == NULL) _us = usHelp();
  return *_us;
}
inline const timeLength &timeLength::ms() {
  if(_ms == NULL) _ms = msHelp();
  return *_ms;
}
inline const timeLength &timeLength::sec() {
  if(_sec == NULL) _sec = secHelp();
  return *_sec;
}
inline const timeLength &timeLength::min() {
  if(_min == NULL) _min = minHelp();
  return *_min;
}
inline const timeLength &timeLength::hour() {
  if(_hour == NULL) _hour = hourHelp();
  return *_hour;
}
inline const timeLength &timeLength::day() {
  if(_day == NULL) _day = dayHelp();
  return *_day;
}
inline const timeLength &timeLength::year() {
  if(_year == NULL) _year = yearHelp();
  return *_year;
}
inline const timeLength &timeLength::leapYear() {
  if(_leapYear == NULL) _leapYear = leapYearHelp();
  return *_leapYear;
}


ostream& operator<<(ostream&s, timeLength z);

// timeStamp +=/-= timeLength
inline const timeStamp operator+=(timeStamp &ts, timeLength tl) {
  ts.assign(ts.get_ns() + tl.get_ns());
  return ts;
}
inline const timeStamp operator-=(timeStamp &ts, timeLength tl) {
  ts.assign(ts.get_ns() - tl.get_ns());
  return ts;
}

// timeLength +=/-= timeLength
inline const timeLength operator+=(timeLength &t, timeLength tl) {
  t.assign(t.get_ns() + tl.get_ns());
  return t;
}
inline const timeLength operator-=(timeLength &t, timeLength tl) {
  t.assign(t.get_ns() - tl.get_ns());
  return t;
}

// timeStamp - timeStamp = timeLength  ;  the length of time between time stamps
inline const timeLength operator-(const timeStamp a, const timeStamp b) {
  return timeLength(a.get_ns() - b.get_ns());
}

// timeStamp +/- timeLength = timeStamp
inline const timeStamp operator+(const timeStamp a, const timeLength b) {
  return timeStamp(a.get_ns() + b.get_ns());
}
inline const timeStamp operator-(const timeStamp a, const timeLength b) {
  return timeStamp(a.get_ns() - b.get_ns());
}

// timeLength + timeStamp = timeStamp
inline const timeStamp operator+(const timeLength a, const timeStamp b) {
  return timeStamp(a.get_ns() + b.get_ns());
}
// timeLength - timeStamp doesn't make sense, ie. 3 days - Mar 9 = ?

// timeLength +/- timeLength = timeLength
inline const timeLength operator+(const timeLength a, const timeLength b) {  
  return timeLength(a.get_ns() + b.get_ns());
}
inline const timeLength operator-(const timeLength a, const timeLength b) {
  return timeLength(a.get_ns() - b.get_ns());
}

// timeLength */ double = timeLength
inline const timeLength operator*(const timeLength a, const double b) {
  return timeLength(static_cast<int64_t>(a.get_ns() * b));
}
inline const timeLength operator/(const timeLength a, const double b) {
  return timeLength(static_cast<int64_t>(a.get_ns() / b));
}

// double */ timeLength = timeLength
inline const timeLength operator*(const double a, const timeLength b) {
  return timeLength(static_cast<int64_t>(a * b.get_ns()));
}
inline const timeLength operator/(const double a, const timeLength b) {
  return timeLength(static_cast<int64_t>(a / b.get_ns()));
}

// timeLength / timeLength = double
inline const double operator/(const timeLength a, const timeLength b) {
  return static_cast<double>(a.get_ns()) / static_cast<double>(b.get_ns());
}


// Be careful if writing * operators because Time is based at nanosecond
// level, which can overflow when multiplying times that seem small
// eg. Time(1,timeUnit::day) * Time(2,timeUnit::day) will overflow

// timeStamp @ timeStamp = bool
inline bool operator==(const timeStamp a, const timeStamp b) {
  return (a.get_ns() == b.get_ns());
}
inline bool operator!=(const timeStamp a, const timeStamp b) {
  return (a.get_ns() != b.get_ns());
}
inline bool operator>(const timeStamp a, const timeStamp b) {
  return (a.get_ns() > b.get_ns());
}
inline bool operator>=(const timeStamp a, const timeStamp b) {
  return (a.get_ns() >= b.get_ns());
}
inline bool operator<(const timeStamp a, const timeStamp b) {
  return (a.get_ns() < b.get_ns());
}
inline bool operator<=(const timeStamp a, const timeStamp b) {
  return (a.get_ns() <= b.get_ns());
}

// timeLength @ timeLength = bool
inline bool operator==(const timeLength a, const timeLength b) {
  return (a.get_ns() == b.get_ns());
}
inline bool operator!=(const timeLength a, const timeLength b) {
  return (a.get_ns() != b.get_ns());
}
inline bool operator>(const timeLength a, const timeLength b) {
  return (a.get_ns() > b.get_ns());
}
inline bool operator>=(const timeLength a, const timeLength b) {
  return (a.get_ns() >= b.get_ns());
}
inline bool operator<(const timeLength a, const timeLength b) {
  return (a.get_ns() < b.get_ns());
}
inline bool operator<=(const timeLength a, const timeLength b) {
  return (a.get_ns() <= b.get_ns());
}


// need to put this here so can get at timeStamp
inline timeBase::timeBase(timeStamp mark) {
  ns2StdBaseMark = -mark.get_ns();  // in Std base
  // eg. (2001) 1 year of ns's -> -1 year of ns's to internalTimeBaseMark
}


#endif

