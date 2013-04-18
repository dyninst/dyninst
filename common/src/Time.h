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

#ifndef __TIME_H
#define __TIME_H

#include <assert.h>
#include "common/src/Types.h"
#include "common/src/fraction.h"

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
   COMMON_EXPORT static const timeUnit *_leapYear;
   COMMON_EXPORT static const timeUnit *_year;
   COMMON_EXPORT static const timeUnit *_day;
   COMMON_EXPORT static const timeUnit *_hour;
   COMMON_EXPORT static const timeUnit *_minute;
   COMMON_EXPORT static const timeUnit *_sec;
   COMMON_EXPORT static const timeUnit *_ms;
   COMMON_EXPORT static const timeUnit *_us;
   COMMON_EXPORT static const timeUnit *_ns;
   COMMON_EXPORT static const timeUnit *nsHelp();
   COMMON_EXPORT static const timeUnit *usHelp();
   COMMON_EXPORT static const timeUnit *msHelp();
   COMMON_EXPORT static const timeUnit *secHelp();
   COMMON_EXPORT static const timeUnit *minHelp();
   COMMON_EXPORT static const timeUnit *hourHelp();
   COMMON_EXPORT static const timeUnit *dayHelp();
   COMMON_EXPORT static const timeUnit *yearHelp();
   COMMON_EXPORT static const timeUnit *leapYearHelp();
 public:
   COMMON_EXPORT static const timeUnit &ns();
   COMMON_EXPORT static const timeUnit &us();
   COMMON_EXPORT static const timeUnit &ms();
   COMMON_EXPORT static const timeUnit &sec();
   COMMON_EXPORT static const timeUnit &minute();
   COMMON_EXPORT static const timeUnit &hour();
   COMMON_EXPORT static const timeUnit &day();
   COMMON_EXPORT static const timeUnit &year();
   COMMON_EXPORT static const timeUnit &leapYear();

 public:
  typedef enum { sparse, verbose } ostream_fmt; 
   static ostream_fmt curFmt;

 private:
  fraction ns_per_unit;
  fraction units_per_ns;

 public:
   COMMON_EXPORT timeUnit(fraction _ns_per_unit);
  // default copy constructor

  // Selectors
   COMMON_EXPORT fraction get_ns_per_unit()  const;
   COMMON_EXPORT fraction get_units_per_ns() const;

  // Mutators
   COMMON_EXPORT void set_ns_per_unit(const fraction &nspu);
   COMMON_EXPORT int64_t cvtTo_ns(double  units) const;
   COMMON_EXPORT int64_t cvtTo_ns(int64_t units) const;
   COMMON_EXPORT double  cvtFrom_nsD(int64_t ns) const;
   COMMON_EXPORT int64_t cvtFrom_nsI(int64_t ns) const;
};

COMMON_EXPORT ostream& operator<<(ostream&s, const timeUnit::ostream_fmt &u);
COMMON_EXPORT ostream& operator<<(ostream&s, const timeUnit &u);

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
  COMMON_EXPORT static const timeBase *_b1970;  // logically const
  COMMON_EXPORT static const timeBase *_bStd;
  COMMON_EXPORT static const timeBase *bStdHelp();
  COMMON_EXPORT static const timeBase *b1970Help();
 public:
  // ie. the year for the internal time base, if changed check to make
  // sure timeBase::b1970Help() is still accurate
  enum { StdBaseMark = 2000 };  
  COMMON_EXPORT static const timeBase &bStd();
  COMMON_EXPORT static const timeBase &b1970();
  // bNone is for times like process time, time is counted since the
  // start of the process representing the process time in regards to
  // a real time base (not unit) as in b1970 doesn't make sense
  COMMON_EXPORT static const timeBase &bNone();

 private:
  int64_t ns2StdBaseMark;  

 public:
  //Paradyn default base: from nearest century turnover

  COMMON_EXPORT timeBase(int64_t ns2stdMark);
  COMMON_EXPORT timeBase(timeStamp mark);  // defined inline below
  COMMON_EXPORT int64_t get_ns2StdBaseMark() const;
  COMMON_EXPORT int64_t cvtTo_bStd(int64_t ns) const;
  COMMON_EXPORT double  cvtFrom_bStd(double ns) const;
  COMMON_EXPORT int64_t cvtFrom_bStd(int64_t ns) const;
};

COMMON_EXPORT ostream& operator<<(ostream&s, timeBase b);

// Responsibilities:
//   - store time in a standard unit and time base, currently this is:
//        * standard unit:      nanoseconds
//        * standard time base: Jan 1, 12:00:00am 2000

// an abstract class
class timeParent {
 private:
  COMMON_EXPORT static const int64_t uninitializedValue;
  int64_t ns;

  // Constructors
 public:
  // Selectors
  COMMON_EXPORT int64_t get_ns() const;

  // returns true if the timeStamp is uninitialized
  COMMON_EXPORT bool isInitialized() const;

  // the disadvantage of having a timeParent operator<< via the help
  // of this virtual put function is that it causes a pointer to the vtable
  // to be carried around with all of our timeLengths and timeStamps
  // and they become less lightweight
  //  virtual ostream& put(ostream& s) const = 0;  // write *this to s

 protected:
  COMMON_EXPORT timeParent();
  COMMON_EXPORT timeParent(int64_t _ns);
  // Mutators
  COMMON_EXPORT void assign(const int64_t v);
  COMMON_EXPORT int64_t getRolloverTime(double t);
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
  COMMON_EXPORT static const timeStamp *_ts1800;
  COMMON_EXPORT static const timeStamp *_ts1970;
  COMMON_EXPORT static const timeStamp *_tsStd;  // ie. timeBase::InternalTimeBase (2000)
  COMMON_EXPORT static const timeStamp *_ts2200;
  COMMON_EXPORT static const timeStamp *ts1800Help();
  COMMON_EXPORT static const timeStamp *ts1970Help();
  COMMON_EXPORT static const timeStamp *tsStdHelp();
  COMMON_EXPORT static const timeStamp *ts2200Help();
 public:
  COMMON_EXPORT static const timeStamp &ts1800();
  COMMON_EXPORT static const timeStamp &ts1970();
  COMMON_EXPORT static const timeStamp &tsStd();
  COMMON_EXPORT static const timeStamp &ts2200();
  COMMON_EXPORT static const timeStamp &tsLongAgoTime();
  COMMON_EXPORT static const timeStamp &tsFarOffTime();

  // need this constructor to use in vector container class
  // value set to a "weird" value to represent uninitialized
  COMMON_EXPORT timeStamp();
  // eg. to create the time Jan 1, 1995 12:00am you could do this:
  //    timeStamp myTime(25, timeUnit::year(), timeBase::b1970());
  // one way to create July 20, 1976 8:35am:
  //    timeStamp myBirthDay = timeStamp::b1970() + 4*timeLength::year() +
  //                           2*timeStamp::leapYear() +
  //                           202*timeLength::day() + 8*timeLength::hour() +
  //                           35*timeLength::min();  
  COMMON_EXPORT timeStamp(int64_t iTime, const timeUnit &u, timeBase b);
  COMMON_EXPORT timeStamp(int iTime, const timeUnit &u, timeBase b);
  COMMON_EXPORT timeStamp(const timeLength &tl, timeBase b);
  COMMON_EXPORT timeStamp(double dTime, const timeUnit &u, timeBase b);

  // Selectors
  COMMON_EXPORT double getD(const timeUnit &u, timeBase b) const;
  // eg. to get the number of seconds since 1970 do this: 
  //        ts.getI(timeUnit::sec(), timeBase::b1970()) 
  COMMON_EXPORT int64_t getI(const timeUnit &u, timeBase b) const;

  // ostream& put(ostream& s) const { return s << *this; }

  friend COMMON_EXPORT const timeStamp operator+=(timeStamp &ts, timeLength tl);
  friend COMMON_EXPORT const timeStamp operator-=(timeStamp &ts, timeLength tl);
  friend COMMON_EXPORT const timeLength operator-(const timeStamp& a, const timeStamp& b);
  friend COMMON_EXPORT const timeStamp operator+(const timeStamp& a, const timeLength& b);
  friend COMMON_EXPORT const timeStamp operator-(const timeStamp& a, const timeLength& b);
  friend COMMON_EXPORT const timeStamp operator+(const timeLength& a, const timeStamp& b);
  // non-member ==, !=, >, <, >=, <=  operators also defined for timeStamp

 private:
  COMMON_EXPORT void initI(int64_t iTime, const timeUnit &u, timeBase b);
  COMMON_EXPORT timeStamp(int64_t ns_);
};

COMMON_EXPORT ostream& operator<<(ostream&s, timeStamp z);

// relTimeStamp ---------------------------------------------------
// A timeStamp that proceeds on a timeline based at "zero"
// It has no relation to time on the "date" timeline.  There is no
// map from the "zero" based timeline for the relTimeStamp to the timeline
// based on "human time" for the timeStamp class.

class relTimeStamp : public timeParent {
 private:
  COMMON_EXPORT static const relTimeStamp *_Zero;
  COMMON_EXPORT static const relTimeStamp *ZeroHelp();
 public:
  COMMON_EXPORT static const relTimeStamp &Zero();

  // need this constructor to use in vector container class
  COMMON_EXPORT relTimeStamp();
  COMMON_EXPORT relTimeStamp(int64_t iTime, const timeUnit &u);
  COMMON_EXPORT relTimeStamp(int iTime, const timeUnit &u);
  COMMON_EXPORT relTimeStamp(const timeLength &tl);
  COMMON_EXPORT relTimeStamp(double dTime, const timeUnit &u);

  // Selectors
  COMMON_EXPORT double getD(const timeUnit &u) const;
  COMMON_EXPORT int64_t getI(const timeUnit &u) const;
  // ostream& put(ostream& s) const { return s << *this; }

  friend COMMON_EXPORT const relTimeStamp operator+=(relTimeStamp &ts, timeLength tl);
  friend COMMON_EXPORT const relTimeStamp operator-=(relTimeStamp &ts, timeLength tl);
  friend COMMON_EXPORT const timeLength operator-(const relTimeStamp& a,const relTimeStamp& b);
  friend COMMON_EXPORT const relTimeStamp operator+(const relTimeStamp& a, const timeLength& b);
  friend COMMON_EXPORT const relTimeStamp operator-(const relTimeStamp& a, const timeLength& b);
  friend COMMON_EXPORT const relTimeStamp operator+(const timeLength& a, const relTimeStamp& b);
  // non-member ==, !=, >, <, >=, <=  operators also defined for relTimeStamp

 private:
  COMMON_EXPORT void initI(int64_t iTime, const timeUnit &u);
  COMMON_EXPORT relTimeStamp(int64_t ns_);
};

COMMON_EXPORT ostream& operator<<(ostream&s, relTimeStamp z);

// timeLength ---------------------------------------------------

// A timeLength represents a length of time and has no notion of a start or
// base time.  It has a range of signed 64bit integer or about 292 years
// since we're counting nanoseconds.

class timeLength : public timeParent {
 private:
  // logically const
  COMMON_EXPORT static const timeLength *_zero;
  COMMON_EXPORT static const timeLength *_ns;
  COMMON_EXPORT static const timeLength *_us;
  COMMON_EXPORT static const timeLength *_ms;
  COMMON_EXPORT static const timeLength *_sec;
  COMMON_EXPORT static const timeLength *_minute;
  COMMON_EXPORT static const timeLength *_hour;
  COMMON_EXPORT static const timeLength *_day;
  COMMON_EXPORT static const timeLength *_year;
  COMMON_EXPORT static const timeLength *_leapYear;
  COMMON_EXPORT static const timeLength *ZeroHelp();
  COMMON_EXPORT static const timeLength *nsHelp();
  COMMON_EXPORT static const timeLength *usHelp();
  COMMON_EXPORT static const timeLength *msHelp();
  COMMON_EXPORT static const timeLength *secHelp();
  COMMON_EXPORT static const timeLength *minHelp();
  COMMON_EXPORT static const timeLength *hourHelp();
  COMMON_EXPORT static const timeLength *dayHelp();
  COMMON_EXPORT static const timeLength *yearHelp();
  COMMON_EXPORT static const timeLength *leapYearHelp();
 public:
  COMMON_EXPORT static const timeLength &Zero();
  COMMON_EXPORT static const timeLength &ns();
  COMMON_EXPORT static const timeLength &us();
  COMMON_EXPORT static const timeLength &ms();
  COMMON_EXPORT static const timeLength &sec();
  COMMON_EXPORT static const timeLength &minute();
  COMMON_EXPORT static const timeLength &hour();
  COMMON_EXPORT static const timeLength &day();
  COMMON_EXPORT static const timeLength &year();
  COMMON_EXPORT static const timeLength &leapYear();

  // need this constructor to use in vector container class
  COMMON_EXPORT timeLength();
  COMMON_EXPORT timeLength(int64_t iTime, const timeUnit &u);
  COMMON_EXPORT timeLength(int iTime, const timeUnit &u);
  COMMON_EXPORT timeLength(double dTime, const timeUnit &u);

  // Selectors
  COMMON_EXPORT double getD(const timeUnit &u) const;
  COMMON_EXPORT int64_t getI(const timeUnit &u) const;

  //ostream& put(ostream& s) const { return s << *this; }

  friend COMMON_EXPORT const timeLength operator+=(timeLength &t, timeLength tl);
  friend COMMON_EXPORT const timeLength operator-=(timeLength &t, timeLength tl);
  friend COMMON_EXPORT const timeLength operator*=(timeLength &t, double d);
  friend COMMON_EXPORT const timeLength operator/=(timeLength &t, double d);
  friend COMMON_EXPORT const timeLength operator-(const timeLength &t);
  friend COMMON_EXPORT const timeLength operator-(const timeStamp& a, const timeStamp& b);
  friend COMMON_EXPORT const timeLength operator-(const relTimeStamp& a,const relTimeStamp& b);
  friend COMMON_EXPORT const timeStamp operator+(const timeStamp& a, const timeLength& b);
  friend COMMON_EXPORT const timeStamp operator-(const timeStamp& a, const timeLength& b);
  friend COMMON_EXPORT const timeStamp operator+(const timeLength& a, const timeStamp& b);
  friend COMMON_EXPORT const timeLength operator+(const timeLength& a, const timeLength& b);
  friend COMMON_EXPORT const timeLength operator-(const timeLength& a, const timeLength& b);
  friend COMMON_EXPORT const timeLength operator*(const timeLength& a, double b);
  friend COMMON_EXPORT const timeLength operator/(const timeLength& a, double b);
  friend COMMON_EXPORT const timeLength operator*(double a, const timeLength& b);
  friend COMMON_EXPORT const timeLength operator/(double a, const timeLength& b);
  friend COMMON_EXPORT double operator/(const timeLength& a, const timeLength& b);
  // non-member ==, !=, >, <, >=, <=  operators also defined for timeLength

 private:
  COMMON_EXPORT void initI(int64_t iTime, const timeUnit &u);
  // a fast constructor just for timeLength operators
  COMMON_EXPORT timeLength(int64_t ns_);
};

COMMON_EXPORT ostream& operator<<(ostream&s, timeLength z);

// timeStamp +=/-= timeLength
COMMON_EXPORT const timeStamp operator+=(timeStamp &ts, timeLength tl);
COMMON_EXPORT const timeStamp operator-=(timeStamp &ts, timeLength tl);

// timeLength +=/-= timeLength
COMMON_EXPORT const timeLength operator+=(timeLength &t, timeLength tl);
COMMON_EXPORT const timeLength operator-=(timeLength &t, timeLength tl);

// timeLength *=, /= double
COMMON_EXPORT const timeLength operator*=(timeLength &t, double d);
COMMON_EXPORT const timeLength operator/=(timeLength &t, double d);

// - timeLength
COMMON_EXPORT const timeLength operator-(const timeLength &t);

// timeStamp - timeStamp = timeLength  ;  the length of time between time stamps
COMMON_EXPORT const timeLength operator-(const timeStamp& a, const timeStamp& b);

// timeStamp +/- timeLength = timeStamp
COMMON_EXPORT const timeStamp operator+(const timeStamp& a, const timeLength& b);
COMMON_EXPORT const timeStamp operator-(const timeStamp& a, const timeLength& b);

// timeLength + timeStamp = timeStamp
COMMON_EXPORT const timeStamp operator+(const timeLength& a, const timeStamp& b);
// timeLength - timeStamp doesn't make sense, ie. 3 days - Mar 9 = ?

// timeLength +/- timeLength = timeLength
COMMON_EXPORT const timeLength operator+(const timeLength& a, const timeLength& b);
COMMON_EXPORT const timeLength operator-(const timeLength& a, const timeLength& b);

// timeLength */ double = timeLength
COMMON_EXPORT const timeLength operator*(const timeLength& a, double b);
COMMON_EXPORT const timeLength operator/(const timeLength& a, double b);

// double */ timeLength = timeLength
COMMON_EXPORT const timeLength operator*(double a, const timeLength& b);
COMMON_EXPORT const timeLength operator/(double a, const timeLength& b);

// Be careful if writing * operators because Time is based at nanosecond
// level, which can overflow when multiplying times that seem small
// eg. Time(1,timeUnit::day) * Time(2,timeUnit::day) will overflow

// timeStamp @ timeStamp = bool
COMMON_EXPORT bool operator==(const timeStamp& a, const timeStamp& b);
COMMON_EXPORT bool operator!=(const timeStamp& a, const timeStamp& b);
COMMON_EXPORT bool operator>(const timeStamp& a, const timeStamp& b);
COMMON_EXPORT bool operator>=(const timeStamp& a, const timeStamp& b);
COMMON_EXPORT bool operator<(const timeStamp& a, const timeStamp& b);
COMMON_EXPORT bool operator<=(const timeStamp& a, const timeStamp& b);

COMMON_EXPORT timeStamp earlier(const timeStamp& a, const timeStamp& b);
COMMON_EXPORT timeStamp later(const timeStamp& a, const timeStamp& b);
// timeLength @ timeLength = bool
COMMON_EXPORT bool operator==(const timeLength& a, const timeLength& b);
COMMON_EXPORT bool operator!=(const timeLength& a, const timeLength& b);
COMMON_EXPORT bool operator>(const timeLength& a, const timeLength& b);
COMMON_EXPORT bool operator>=(const timeLength& a, const timeLength& b);
COMMON_EXPORT bool operator<(const timeLength& a, const timeLength& b);
COMMON_EXPORT bool operator<=(const timeLength& a, const timeLength& b);


COMMON_EXPORT timeLength minimum(const timeLength& a, const timeLength& b);

COMMON_EXPORT timeLength maximum(const timeLength& a, const timeLength& b);
COMMON_EXPORT const timeLength abs(const timeLength& a);

// relTimeStamp +=/-= timeLength
COMMON_EXPORT const relTimeStamp operator+=(relTimeStamp &ts, timeLength tl);
COMMON_EXPORT const relTimeStamp operator-=(relTimeStamp &ts, timeLength tl);

// relTimeStamp - relTimeStamp = timeLength  ;  the length of time between time stamps
COMMON_EXPORT const timeLength operator-(const relTimeStamp& a, const relTimeStamp& b);

// relTimeStamp +/- relTimeLength = relTimeStamp
COMMON_EXPORT const relTimeStamp operator+(const relTimeStamp& a, const timeLength& b);
COMMON_EXPORT const relTimeStamp operator-(const relTimeStamp& a, const timeLength& b);

// timeLength + relTimeStamp = relTimeStamp
COMMON_EXPORT const relTimeStamp operator+(const timeLength& a, const relTimeStamp& b);
// timeLength - timeStamp doesn't make sense, ie. 3 days - Mar 9 = ?


// Be careful if writing * operators because Time is based at nanosecond
// level, which can overflow when multiplying times that seem small
// eg. Time(1,timeUnit::day) * Time(2,timeUnit::day) will overflow

// relTimeStamp @ relTimeStamp = bool
COMMON_EXPORT bool operator==(const relTimeStamp& a, const relTimeStamp& b);
COMMON_EXPORT bool operator!=(const relTimeStamp& a, const relTimeStamp& b);
COMMON_EXPORT bool operator>(const relTimeStamp& a, const relTimeStamp& b);
COMMON_EXPORT bool operator>=(const relTimeStamp& a, const relTimeStamp& b);
COMMON_EXPORT bool operator<(const relTimeStamp& a, const relTimeStamp& b);
COMMON_EXPORT bool operator<=(const relTimeStamp& a, const relTimeStamp& b);

COMMON_EXPORT relTimeStamp earlier(const relTimeStamp& a, const relTimeStamp& b);

COMMON_EXPORT relTimeStamp later(const relTimeStamp& a, const relTimeStamp& b);



#endif

