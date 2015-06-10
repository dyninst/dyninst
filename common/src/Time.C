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

#include "common/src/Time.h"
#include <iostream>
#include <ctime>
#include <string.h>
#include <iomanip>
#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include "common/src/int64iostream.h"


timeUnit::ostream_fmt timeUnit::curFmt = timeUnit::sparse;

const timeUnit *timeUnit::_ns = NULL;
const timeUnit *timeUnit::_us = NULL;
const timeUnit *timeUnit::_ms = NULL;
const timeUnit *timeUnit::_sec = NULL;
const timeUnit *timeUnit::_minute = NULL;
const timeUnit *timeUnit::_hour = NULL;
const timeUnit *timeUnit::_day = NULL;
const timeUnit *timeUnit::_year = NULL;
const timeUnit *timeUnit::_leapYear = NULL;

// these helper functions will help reduce code bloat
// The ...Help functions are not inlined (or not included with their inlined
// user functions) because the Help functions call constructors which are
// also inlined and the result is the possibility for a lot of code bloat.
// So the reference is inlined, but the creation step (which only will happen
// once) is not inlined.

const timeUnit *timeUnit::nsHelp() {
  return new timeUnit(fraction(1));
}
const timeUnit *timeUnit::usHelp() {
  return new timeUnit(fraction(1000));
}
const timeUnit *timeUnit::msHelp() {
  return new timeUnit(fraction(1000000));
}
const timeUnit *timeUnit::secHelp() {
  return new timeUnit(fraction(1000000000));
}
const timeUnit *timeUnit::minHelp() {
  int64_t ns_per_sec = sec().get_ns_per_unit().getNumer();
  return new timeUnit(fraction(I64_C(60) * ns_per_sec));
}
const timeUnit *timeUnit::hourHelp() {
  int64_t ns_per_min = minute().get_ns_per_unit().getNumer();
  return new timeUnit(fraction(I64_C(60) * ns_per_min));
}
const timeUnit *timeUnit::dayHelp() {
  int64_t ns_per_hour = hour().get_ns_per_unit().getNumer();
  return new timeUnit(fraction(I64_C(24) * ns_per_hour));
}
const timeUnit *timeUnit::yearHelp() {
  double ns_per_day =static_cast<double>(day().get_ns_per_unit().getNumer());
  fraction ratio(static_cast<int64_t>(365 * ns_per_day));
  return new timeUnit(ratio);
}
const timeUnit *timeUnit::leapYearHelp() {
  double ns_per_day =static_cast<double>(day().get_ns_per_unit().getNumer());
  fraction ratio(static_cast<int64_t>(366 * ns_per_day));
  return new timeUnit(ratio);
}

const timeBase *timeBase::_bStd = NULL;
const timeBase *timeBase::_b1970 = NULL;

const timeBase *timeBase::bStdHelp() {
  return new timeBase(0);
}
// if StdBase is changed, might need to change calculation of numLeapYears
const timeBase *timeBase::b1970Help() {
  int64_t nsPerYear = timeUnit::year().get_ns_per_unit().getNumer();
  int64_t nsPerLeapYear = timeUnit::leapYear().get_ns_per_unit().getNumer();
  int numLeapYears = (StdBaseMark - 1970)/4;
  int numStdYears  = (StdBaseMark - 1970) - numLeapYears;
  return new timeBase(numStdYears * nsPerYear + 
		      numLeapYears * nsPerLeapYear);
}


int64_t timeUnit::cvtTo_ns(double units) const {
  double tns = ns_per_unit * units;
  return static_cast<int64_t>(tns);
}

int64_t timeUnit::cvtTo_ns(int64_t units) const {
  int64_t nsec;
  nsec = ns_per_unit.multReturnInt64(units);
  return nsec;
}

double timeUnit::cvtFrom_nsD(int64_t nsec) const {
  return units_per_ns * (double)nsec;
}

int64_t timeUnit::cvtFrom_nsI(int64_t nsec) const {
  int64_t units;
  units = units_per_ns.multReturnInt64(nsec);
  return units;
}

int64_t timeParent::getRolloverTime(double dt) {
  // a modulus with doubles
  int num = (int) (dt / (double)I64_MAX);
  double rolloverAmt = dt - ((double) num * (double)I64_MAX);
  return (int64_t) rolloverAmt;
}

const timeStamp *timeStamp::_ts1800  = NULL;
const timeStamp *timeStamp::_ts1970  = NULL;
const timeStamp *timeStamp::_tsStd   = NULL;
const timeStamp *timeStamp::_ts2200  = NULL;
// This 88 quintillion number is some 280 years.  For the timeLength class,
// this amount of time should be safely out of reach.  The timeStamp class is
// internally based at the year of timeBase::StdBaseMark, ie. year 2000, the
// uninitialized value represents a timeStamp around year 2280, plenty safe
// out of range for a long time.  We want the same number, eg. 8, for every
// digit of this number so it can be easily recognized from a debugger.
const int64_t    timeParent::uninitializedValue = I64_C(8888888888888888888);

const timeStamp *timeStamp::ts1800Help() {
  timeStamp *p_ts1970 = new timeStamp(0, timeUnit::year(), timeBase::b1970());
  *p_ts1970 -= 53*timeLength::year() + 17*timeLength::leapYear(); //1900 - 1970
  // beginning 2000 (leap year) - beg 2100 (no leap year), 25 4 year spans
  *p_ts1970 -= 76*timeLength::year() + 24*timeLength::leapYear();// 1800 - 1900
  return p_ts1970;
}

const timeStamp *timeStamp::ts1970Help() {
  return new timeStamp(0, timeUnit::sec(), timeBase::b1970());
}

const timeStamp *timeStamp::tsStdHelp() {
  return new timeStamp(0, timeUnit::year(), timeBase::bStd());
}

const timeStamp *timeStamp::ts2200Help() {
  timeStamp *p_ts1970 = new timeStamp(0, timeUnit::year(), timeBase::b1970());
  *p_ts1970 += 23*timeLength::year() + 7*timeLength::leapYear(); // 1970 - 2000
  // beginning 2000 (leap year) - beg 2100 (no leap year), 25 4 year spans
  *p_ts1970 += 75*timeLength::year() + 25*timeLength::leapYear();
  // beg 2100 (no leap year) - beg 2200 (no leap year), 25 4 year spans
  *p_ts1970 += 76*timeLength::year() + 24*timeLength::leapYear();
  return p_ts1970;
}

void timeStamp::initI(int64_t iTime, const timeUnit &u, timeBase b) {
  assign( b.cvtTo_bStd( u.cvtTo_ns(iTime)));
  /*  // if we allow exceptions in the future, we will be able to handle
      // nicely cases of overflows in the fraction class
      } catch(finalMultOverflowExc &) {
      double dRes = static_cast<double>(b.cvtTo_bStd( u.cvtTo_ns(
      static_cast<double>(iTime))));
      assign( getRolloverTime(dRes));
      }
  */
}

void timeLength::initI(int64_t iTime, const timeUnit &u) {
  assign( u.cvtTo_ns(iTime));
  /*  // if we allow exceptions in the future, we will be able to handle
      // nicely cases of overflows in the fraction class  
      } catch(finalMultOverflowExc &) {
      double dRes= static_cast<double>(u.cvtTo_ns(static_cast<double>(iTime)));
      assign( getRolloverTime(dRes));
      }
  */
}

void relTimeStamp::initI(int64_t iTime, const timeUnit &u) {
  assign( u.cvtTo_ns(iTime));
  /*  // if we allow exceptions in the future, we will be able to handle
      // nicely cases of overflows in the fraction class  
      } catch(finalMultOverflowExc &) {
      double dRes= static_cast<double>(u.cvtTo_ns(static_cast<double>(iTime)));
      assign( getRolloverTime(dRes));
      }
  */
}

timeStamp::timeStamp(const timeLength &tl, timeBase b) : timeParent() {
  initI(tl.getI(timeUnit::ns()), timeUnit::ns(), b);
}

timeStamp::timeStamp(double dTime, const timeUnit &u, timeBase b): timeParent()
{
  double dRes = static_cast<double>(b.cvtTo_bStd( u.cvtTo_ns(dTime)));
  if(dRes <= static_cast<double>(I64_MAX)) 
    assign( static_cast<int64_t>(dRes));
  else
    assign( getRolloverTime(dRes));
}

const relTimeStamp *relTimeStamp::_Zero      = NULL;

const relTimeStamp *relTimeStamp::ZeroHelp() {
  return new relTimeStamp(0, timeUnit::sec());
}

const timeLength *timeLength::_zero = NULL;
const timeLength *timeLength::ZeroHelp() {
  return new timeLength(0, timeUnit::sec());
}

relTimeStamp::relTimeStamp(const timeLength &tl) : timeParent() {
  initI(tl.getI(timeUnit::ns()), timeUnit::ns());
}

relTimeStamp::relTimeStamp(double dTime, const timeUnit &u): timeParent()
{
  double dRes = static_cast<double>( u.cvtTo_ns(dTime));
  if(dRes <= static_cast<double>(I64_MAX)) 
    assign( static_cast<int64_t>(dRes));
  else
    assign( getRolloverTime(dRes));
}

const timeLength *timeLength::_ns = NULL;
const timeLength *timeLength::nsHelp() {
  return new timeLength(1, timeUnit::ns());
}
const timeLength *timeLength::_us = NULL;
const timeLength *timeLength::usHelp() {
  return new timeLength(1, timeUnit::us());
}
const timeLength *timeLength::_ms = NULL;
const timeLength *timeLength::msHelp() {
  return new timeLength(1, timeUnit::ms());
}
const timeLength *timeLength::_sec = NULL;
const timeLength *timeLength::secHelp() {
  return new timeLength(1, timeUnit::sec());
}
const timeLength *timeLength::_minute = NULL;
const timeLength *timeLength::minHelp() {
  return new timeLength(1, timeUnit::minute());
}
const timeLength *timeLength::_hour = NULL;
const timeLength *timeLength::hourHelp() {
  return new timeLength(1, timeUnit::hour());
}
const timeLength *timeLength::_day = NULL;
const timeLength *timeLength::dayHelp() {
  return new timeLength(1, timeUnit::day());
}
const timeLength *timeLength::_year = NULL;
const timeLength *timeLength::yearHelp() {
  return new timeLength(1, timeUnit::year());
}
const timeLength *timeLength::_leapYear = NULL;
const timeLength *timeLength::leapYearHelp() {
  return new timeLength(1, timeUnit::leapYear());
}


timeLength::timeLength(double dTime, const timeUnit &u) : timeParent() {
  double dRes = static_cast<double>(u.cvtTo_ns(dTime));
  if(dRes <= static_cast<double>(I64_MAX)) 
    assign( static_cast<int64_t>(dRes));
  else
    assign( getRolloverTime(dRes));
}

ostream& operator<<(ostream&s, const timeUnit::ostream_fmt &u) {
  timeUnit::curFmt = u;
  return s;
}

ostream& operator<<(ostream&s, const timeUnit &u) {
  if(timeUnit::curFmt == timeUnit::sparse) {
    s << "["<< fraction::sparse << u.get_ns_per_unit()<<"]";
  } else { // timeUnit::verbose
    s << fraction::verbose << "[ns_per_unit: " << u.get_ns_per_unit() 
      << ", units_per_ns: " << u.get_units_per_ns() << "]";
  }
  return s;
}

ostream& operator<<(ostream&s, timeBase b) {
  s << timeStamp(-b.get_ns2StdBaseMark(),timeUnit::ns(),timeBase::bStd());
  return s;
}

/*
ostream& operator<<(ostream& s, const timeParent &tp) {
  return tp.put(s);
}
*/

// only handles positives right now
// string buffer should be atleast 16
void insCommas(int num, char strbuf[]) {
    char nsStr[20];
    sprintf(nsStr,"%12d",num);
    int nextIdx=0;
    if(num>=1000000000) {
      strncpy(&strbuf[nextIdx], &nsStr[0], 3);
      nextIdx+=3;
      strbuf[nextIdx++] = ',';  
    }
    if(num>=1000000) {
      strncpy(&strbuf[nextIdx], &nsStr[3], 3);
      nextIdx+=3;
      strbuf[nextIdx++] = ',';  
    }
    if(num>=1000) {
      strncpy(&strbuf[nextIdx], &nsStr[6], 3);
      nextIdx+=3;
      strbuf[nextIdx++] = ',';  
    }
    if(num>=0) {
      strncpy(&strbuf[nextIdx], &nsStr[9], 3);
      nextIdx+=3;
    }
    strbuf[nextIdx]=0;
}

ostream& operator<<(ostream&s, timeLength z) {
  timeLength tUser = z;
  int years = static_cast<int>(tUser.getI(timeUnit::year()));
  timeLength tempT(years,timeUnit::year());
  tUser -= tempT;
  int days  = static_cast<int>(tUser.getI(timeUnit::day()));
  tUser -= timeLength(days,timeUnit::day());
  int hours = static_cast<int>(tUser.getI(timeUnit::hour()));
  tUser -= timeLength(hours,timeUnit::hour());
  int mins  = static_cast<int>(tUser.getI(timeUnit::minute()));
  tUser -= timeLength(mins,timeUnit::minute());
  int secs  = static_cast<int>(tUser.getI(timeUnit::sec()));
  tUser -= timeLength(secs,timeUnit::sec());
  int ms    = static_cast<int>(tUser.getI(timeUnit::ms()));
  tUser -= timeLength(ms,timeUnit::ms());
  int us    = static_cast<int>(tUser.getI(timeUnit::us()));
  tUser -= timeLength(us,timeUnit::us());
  int ns    = static_cast<int>(tUser.getI(timeUnit::ns()));

  bool prev = false, compact = false;
  char cSt[5] = ", ";
  // this is a hack for those that want to print the time compressed
  if(s.flags() & ostream::oct) {  compact=true;  s.flags(ostream::dec);  }
  if(compact == true) strcpy(cSt,",");
  s << "[";
  if(z.get_ns() == 0) {  s << "0 time"; }
  else {
    if(years!=0) {  s << (prev? cSt:"") << years << " years";   prev=true; }
    if(days!=0)  {  s << (prev? cSt:"") << days  << " days";    prev=true; }
    if(hours!=0) {  s << (prev? cSt:"") << hours << " hours";   prev=true; }
    if(mins!=0)  {  s << (prev? cSt:"") << mins  << " mins";    prev=true; }
    if(secs!=0)  {  s << (prev? cSt:"") << secs  << " secs";    prev=true; }
    if(ms!=0)    {  s << (prev? cSt:"") << ms    << " ms";      prev=true; }
    if(us!=0)    {  s << (prev? cSt:"") << us    << " us";      prev=true; }
    if(ns!=0)    {  s << (prev? cSt:"") << ns    << " ns";      prev=true; }
  }
  s << "]";
  return s;
}

ostream& operator<<(ostream&s, timeStamp z) {
  time_t s1970 = static_cast<time_t>(z.getI(timeUnit::sec(), timeBase::b1970()));
  int64_t nsI1970 = static_cast<int64_t>(s1970) * I64_C(1000000000);
  if(! z.isInitialized()) {  
    s << "[uninitialized]";
  } else if(z < timeStamp(0, timeUnit::year(), timeBase::b1970())) {
    timeLength tl(z - timeStamp(0, timeUnit::year(), timeBase::b1970()));
    // setting the oct flag is just a hack to tell the timeLength ostream<<
    // to print the time compressed
#if !defined(os_windows)
    ostream::fmtflags oldflags;
#else
    long oldflags;  // nt environment doesn't define ostream::fmtflags
#endif
    oldflags = s.flags(ostream::oct);
    s << "[1970 + " << tl << "]"; 
    s.flags(oldflags);
  } else {
    int64_t nstot = z.getI(timeUnit::ns(), timeBase::b1970());
    int64_t nsrem = nstot - nsI1970;
    char dateStr[50];
    s1970 = mktime(localtime(&s1970));
    strcpy(dateStr, ctime(&s1970));
    char *p = strstr(dateStr, "\n");  // erase the nl
    if(p != NULL) {  *p++ = 0;   *p = 0; }
    char strFmt[30];
    insCommas(static_cast<int>(nsrem), strFmt);
    s << "[" << dateStr << "  " << strFmt << "ns]";
  }
  return s;
}

ostream& operator<<(ostream&s, relTimeStamp z) {
  if(! z.isInitialized()) {  
    s << "[uninitialized]";
  } else {
    timeLength t(z.getI(timeUnit::ns()), timeUnit::ns());
    s << t;
  }
  return s;
}

// timeLength / timeLength = double
double operator/(const timeLength& a, const timeLength& b) {
  assert(a.isInitialized() && b.isInitialized());
  return static_cast<double>(a.get_ns()) / static_cast<double>(b.get_ns());
}

bool timeParent::isInitialized() const {  
  if(get_ns() == uninitializedValue) return false;
  else return true;
}

timeParent::timeParent() 
  : ns(uninitializedValue) 
{ 
}

timeParent::timeParent(int64_t _ns) 
  : ns(_ns) 
{ 
}

COMMON_EXPORT timeStamp::timeStamp(int64_t iTime, const timeUnit &u, timeBase b) 
  : timeParent()
{
  initI(iTime, u, b);
}

COMMON_EXPORT timeStamp::timeStamp(int iTime, const timeUnit &u, timeBase b) 
  : timeParent() 
{
  initI(iTime, u, b);
}

COMMON_EXPORT relTimeStamp::relTimeStamp(int64_t iTime, const timeUnit &u) : timeParent() {
  initI(iTime, u);
}

COMMON_EXPORT relTimeStamp::relTimeStamp(int iTime, const timeUnit &u) : timeParent() {
  initI(iTime, u);
}

COMMON_EXPORT timeLength::timeLength(int64_t iTime, const timeUnit &u)
  : timeParent() 
{
  initI(iTime, u);
}

COMMON_EXPORT timeLength::timeLength(int iTime, const timeUnit &u)  
  : timeParent() 
{
  initI(static_cast<int64_t>(iTime), u);
}

COMMON_EXPORT timeStamp::timeStamp(int64_t ns_)
  : timeParent(ns_)
{
}

const timeLength &timeLength::Zero() {
  if(_zero == NULL) _zero = ZeroHelp();
  return *_zero;
}
const timeLength &timeLength::ns() {
  if(_ns == NULL) _ns = nsHelp();
  return *_ns;
}
const timeLength &timeLength::us() {
  if(_us == NULL) _us = usHelp();
  return *_us;
}
const timeLength &timeLength::ms() {
  if(_ms == NULL) _ms = msHelp();
  return *_ms;
}
const timeLength &timeLength::sec() {
  if(_sec == NULL) _sec = secHelp();
  return *_sec;
}
const timeLength &timeLength::minute() {
  if(_minute == NULL) _minute = minHelp();
  return *_minute;
}
const timeLength &timeLength::hour() {
  if(_hour == NULL) _hour = hourHelp();
  return *_hour;
}
const timeLength &timeLength::day() {
  if(_day == NULL) _day = dayHelp();
  return *_day;
}
const timeLength &timeLength::year() {
  if(_year == NULL) _year = yearHelp();
  return *_year;
}
const timeLength &timeLength::leapYear() {
  if(_leapYear == NULL) _leapYear = leapYearHelp();
  return *_leapYear;
}

// timeStamp +=/-= timeLength
const timeStamp operator+=(timeStamp &ts, timeLength tl) {
  assert(ts.isInitialized() && tl.isInitialized());
  ts.assign(ts.get_ns() + tl.get_ns());
  return ts;
}
COMMON_EXPORT const timeStamp operator-=(timeStamp &ts, timeLength tl) {
  assert(ts.isInitialized() && tl.isInitialized());
  ts.assign(ts.get_ns() - tl.get_ns());
  return ts;
}

// timeLength +=/-= timeLength
const timeLength operator+=(timeLength &t, timeLength tl) {
  assert(t.isInitialized() && tl.isInitialized());
  t.assign(t.get_ns() + tl.get_ns());
  return t;
}
const timeLength operator-=(timeLength &t, timeLength tl) {
  assert(t.isInitialized() && tl.isInitialized());
  t.assign(t.get_ns() - tl.get_ns());
  return t;
}

// timeLength *=, /= double
const timeLength operator*=(timeLength &t, double d) {
  assert(t.isInitialized());
  t.assign(static_cast<int64_t>(static_cast<double>(t.get_ns()) * d));
  return t;
}
const timeLength operator/=(timeLength &t, double d) {
  assert(t.isInitialized());
  t.assign(static_cast<int64_t>(static_cast<double>(t.get_ns()) / d));
  return t;
}

// - timeLength
const timeLength operator-(const timeLength &t) {
  assert(t.isInitialized());
  return timeLength(-t.get_ns());
}

// timeStamp - timeStamp = timeLength  ;  the length of time between time stamps
const timeLength operator-(const timeStamp& a, const timeStamp& b) {
  assert(a.isInitialized() && b.isInitialized());
  return timeLength(a.get_ns() - b.get_ns());
}

// timeStamp +/- timeLength = timeStamp
const timeStamp operator+(const timeStamp& a, const timeLength& b) {
  assert(a.isInitialized() && b.isInitialized());
  return timeStamp(a.get_ns() + b.get_ns());
}
const timeStamp operator-(const timeStamp& a, const timeLength& b) {
  assert(a.isInitialized() && b.isInitialized());
  return timeStamp(a.get_ns() - b.get_ns());
}

// timeLength + timeStamp = timeStamp
const timeStamp operator+(const timeLength& a, const timeStamp& b) {
  assert(a.isInitialized() && b.isInitialized());
  return timeStamp(a.get_ns() + b.get_ns());
}
// timeLength - timeStamp doesn't make sense, ie. 3 days - Mar 9 = ?

// timeLength +/- timeLength = timeLength
const timeLength operator+(const timeLength& a, const timeLength& b) {  
  assert(a.isInitialized() && b.isInitialized());
  return timeLength(a.get_ns() + b.get_ns());
}
const timeLength operator-(const timeLength& a, const timeLength& b) {
  assert(a.isInitialized() && b.isInitialized());
  return timeLength(a.get_ns() - b.get_ns());
}

// timeLength */ double = timeLength
const timeLength operator*(const timeLength& a, const double b) {
  assert(a.isInitialized());
  return timeLength(static_cast<int64_t>(static_cast<double>(a.get_ns()) * b));
}
const timeLength operator/(const timeLength& a, const double b) {
  assert(a.isInitialized());
  return timeLength(static_cast<int64_t>(static_cast<double>(a.get_ns()) / b));
}

// double */ timeLength = timeLength
const timeLength operator*(const double a, const timeLength& b) {
  assert(b.isInitialized());
  return timeLength(static_cast<int64_t>(a * static_cast<double>(b.get_ns())));
}
const timeLength operator/(const double a, const timeLength& b) {
  assert(b.isInitialized());
  return timeLength(static_cast<int64_t>(a / static_cast<double>(b.get_ns())));
}

// Be careful if writing * operators because Time is based at nanosecond
// level, which can overflow when multiplying times that seem small
// eg. Time(1,timeUnit::day) * Time(2,timeUnit::day) will overflow

// timeStamp @ timeStamp = bool
bool operator==(const timeStamp& a, const timeStamp& b) {
  assert(a.isInitialized() && b.isInitialized());
  return (a.get_ns() == b.get_ns());
}
bool operator!=(const timeStamp& a, const timeStamp& b) {
  assert(a.isInitialized() && b.isInitialized());
  return (a.get_ns() != b.get_ns());
}
bool operator>(const timeStamp& a, const timeStamp& b) {
  assert(a.isInitialized() && b.isInitialized());
  return (a.get_ns() > b.get_ns());
}
bool operator>=(const timeStamp& a, const timeStamp& b) {
  assert(a.isInitialized() && b.isInitialized());
  return (a.get_ns() >= b.get_ns());
}
bool operator<(const timeStamp& a, const timeStamp& b) {
  assert(a.isInitialized() && b.isInitialized());
  return (a.get_ns() < b.get_ns());
}
bool operator<=(const timeStamp& a, const timeStamp& b) {
  assert(a.isInitialized() && b.isInitialized());
  return (a.get_ns() <= b.get_ns());
}

timeStamp earlier(const timeStamp& a, const timeStamp& b) {
  assert(a.isInitialized() && b.isInitialized());
  if(a <= b)  return a;
  else        return b;
}

timeStamp later(const timeStamp& a, const timeStamp& b) {
  assert(a.isInitialized() && b.isInitialized());
  if(a >= b)  return a;
  else        return b;
}

// timeLength @ timeLength = bool
bool operator==(const timeLength& a, const timeLength& b) {
  assert(a.isInitialized() && b.isInitialized());
  return (a.get_ns() == b.get_ns());
}
bool operator!=(const timeLength& a, const timeLength& b) {
  assert(a.isInitialized() && b.isInitialized());
  return (a.get_ns() != b.get_ns());
}
bool operator>(const timeLength& a, const timeLength& b) {
  assert(a.isInitialized() && b.isInitialized());
  return (a.get_ns() > b.get_ns());
}
bool operator>=(const timeLength& a, const timeLength& b) {
  assert(a.isInitialized() && b.isInitialized());
  return (a.get_ns() >= b.get_ns());
}
bool operator<(const timeLength& a, const timeLength& b) {
  assert(a.isInitialized() && b.isInitialized());
  return (a.get_ns() < b.get_ns());
}
bool operator<=(const timeLength& a, const timeLength& b) {
  assert(a.isInitialized() && b.isInitialized());
  return (a.get_ns() <= b.get_ns());
}


timeLength minimum(const timeLength& a, const timeLength& b) {  
  assert(a.isInitialized() && b.isInitialized());
  if(a<=b)  return a;
  else      return b;
}

timeLength maximum(const timeLength& a, const timeLength& b) {  
  assert(a.isInitialized() && b.isInitialized());
  if(a>=b)  return a;
  else      return b;
}

const timeLength abs(const timeLength& a) {  
  assert(a.isInitialized());
  return maximum(a,-a);
}

// need to put this here so can get at timeStamp
timeBase::timeBase(timeStamp mark) {
  ns2StdBaseMark = -mark.get_ns();  // in Std base
  // eg. (2001) 1 year of ns's -> -1 year of ns's to internalTimeBaseMark
}

// relTimeStamp +=/-= timeLength
const relTimeStamp operator+=(relTimeStamp &ts, timeLength tl) {
  assert(ts.isInitialized() && tl.isInitialized());
  ts.assign(ts.get_ns() + tl.get_ns());
  return ts;
}
const relTimeStamp operator-=(relTimeStamp &ts, timeLength tl) {
  assert(ts.isInitialized() && tl.isInitialized());
  ts.assign(ts.get_ns() - tl.get_ns());
  return ts;
}

// relTimeStamp - relTimeStamp = timeLength  ;  the length of time between time stamps
const timeLength operator-(const relTimeStamp& a, const relTimeStamp& b) {
  assert(a.isInitialized() && b.isInitialized());
  return timeLength(a.get_ns() - b.get_ns());
}

// relTimeStamp +/- relTimeLength = relTimeStamp
const relTimeStamp operator+(const relTimeStamp& a, const timeLength& b) {
  assert(a.isInitialized() && b.isInitialized());
  return relTimeStamp(a.get_ns() + b.get_ns());
}
const relTimeStamp operator-(const relTimeStamp& a, const timeLength& b) {
  assert(a.isInitialized() && b.isInitialized());
  return relTimeStamp(a.get_ns() - b.get_ns());
}

// timeLength + relTimeStamp = relTimeStamp
const relTimeStamp operator+(const timeLength& a, const relTimeStamp& b) {
  assert(a.isInitialized() && b.isInitialized());
  return relTimeStamp(a.get_ns() + b.get_ns());
}
// timeLength - timeStamp doesn't make sense, ie. 3 days - Mar 9 = ?


// Be careful if writing * operators because Time is based at nanosecond
// level, which can overflow when multiplying times that seem small
// eg. Time(1,timeUnit::day) * Time(2,timeUnit::day) will overflow

// relTimeStamp @ relTimeStamp = bool
bool operator==(const relTimeStamp& a, const relTimeStamp& b) {
  assert(a.isInitialized() && b.isInitialized());
  return (a.get_ns() == b.get_ns());
}
bool operator!=(const relTimeStamp& a, const relTimeStamp& b) {
  assert(a.isInitialized() && b.isInitialized());
  return (a.get_ns() != b.get_ns());
}
bool operator>(const relTimeStamp& a, const relTimeStamp& b) {
  assert(a.isInitialized() && b.isInitialized());
  return (a.get_ns() > b.get_ns());
}
bool operator>=(const relTimeStamp& a, const relTimeStamp& b) {
  assert(a.isInitialized() && b.isInitialized());
  return (a.get_ns() >= b.get_ns());
}
bool operator<(const relTimeStamp& a, const relTimeStamp& b) {
  assert(a.isInitialized() && b.isInitialized());
  return (a.get_ns() < b.get_ns());
}
bool operator<=(const relTimeStamp& a, const relTimeStamp& b) {
  assert(a.isInitialized() && b.isInitialized());
  return (a.get_ns() <= b.get_ns());
}

relTimeStamp earlier(const relTimeStamp& a, const relTimeStamp& b) {
  assert(a.isInitialized() && b.isInitialized());
  if(a <= b)  return a;
  else        return b;
}

relTimeStamp later(const relTimeStamp& a, const relTimeStamp& b) {
  assert(a.isInitialized() && b.isInitialized());
  if(a >= b)  return a;
  else        return b;
}

timeUnit::timeUnit(fraction _ns_per_unit) : ns_per_unit(_ns_per_unit), 
  units_per_ns(ns_per_unit.reciprocal()) {
  ns_per_unit.reduce();
  units_per_ns.reduce();
}

// Selectors
fraction timeUnit::get_ns_per_unit()  const {  return ns_per_unit;   }
fraction timeUnit::get_units_per_ns() const {  return units_per_ns;  }

// Mutators
void timeUnit::set_ns_per_unit(const fraction &nspu) {
  ns_per_unit = nspu;
  units_per_ns = ns_per_unit.reciprocal();
}

const timeUnit &timeUnit::ns() {
  if(_ns == NULL)  _ns = nsHelp();
  return *_ns;
}
const timeUnit &timeUnit::us() {
  if(_us == NULL)  _us = usHelp();
  return *_us;
}
const timeUnit &timeUnit::ms() {
  if(_ms==NULL)  _ms = msHelp();
  return *_ms;
}
const timeUnit &timeUnit::sec() {
  if(_sec==NULL) _sec = secHelp();
  return *_sec;
}
const timeUnit &timeUnit::minute() {
  if(_minute==NULL)  _minute = minHelp();
  return *_minute;
}
const timeUnit &timeUnit::hour() {
  if(_hour==NULL) _hour = hourHelp();
  return *_hour;
}
const timeUnit &timeUnit::day() {
  if(_day==NULL) _day = dayHelp();
  return *_day;
}
const timeUnit &timeUnit::year() {
  if(_year==NULL) _year = yearHelp();
  return *_year;
}
const timeUnit &timeUnit::leapYear() {
  if(_leapYear==NULL) _leapYear = leapYearHelp();
  return *_leapYear;
}


timeBase::timeBase(int64_t ns2stdMark) {
  ns2StdBaseMark = ns2stdMark;
}

int64_t timeBase::get_ns2StdBaseMark() const {
  return ns2StdBaseMark;
}

int64_t timeBase::cvtTo_bStd(int64_t ns) const {
  // eg. 1994, b1970 -> bStd:  24 yrs - 30 yrs = -6 yrs
  return ns - ns2StdBaseMark;  
}

double  timeBase::cvtFrom_bStd(double ns) const {
  // eg. 1994, bStd -> b1970:  -6 yrs + 30 yrs = 24 yrs
   return ns + static_cast<double>(ns2StdBaseMark);
}

int64_t timeBase::cvtFrom_bStd(int64_t ns) const {
  return ns + ns2StdBaseMark;
}

const timeBase &timeBase::bStd() {
  if(_bStd == NULL) _bStd = bStdHelp();
  return *_bStd;
}
const timeBase &timeBase::b1970() {
  if(_b1970 == NULL) _b1970 = b1970Help();
  return *_b1970;
}
const timeBase &timeBase::bNone() {
  return bStd();
}


int64_t timeParent::get_ns() const {
  return ns;
}

void timeParent::assign(const int64_t v) 
{  
  ns = v;  
}

timeStamp::timeStamp() { }

double timeStamp::getD(const timeUnit &u, timeBase b) const {
  return u.cvtFrom_nsD( b.cvtFrom_bStd(get_ns()));
}

int64_t timeStamp::getI(const timeUnit &u, timeBase b) const {
  return u.cvtFrom_nsI( b.cvtFrom_bStd(get_ns()));
}

const timeStamp &timeStamp::ts1970() {
  if(_ts1970 == NULL)  _ts1970 = ts1970Help();
  return *_ts1970;
}

const timeStamp &timeStamp::tsStd() {
  if(_tsStd == NULL)  _tsStd = tsStdHelp();
  return *_tsStd;
}

const timeStamp &timeStamp::ts1800() {
  if(_ts1800 == NULL)  _ts1800 = ts1800Help();
  return *_ts1800;
}

const timeStamp &timeStamp::ts2200() {
  if(_ts2200 == NULL)  _ts2200 = ts2200Help();
  return *_ts2200;
}

const timeStamp &timeStamp::tsLongAgoTime() {
  return timeStamp::ts1800();
}

const timeStamp &timeStamp::tsFarOffTime() {
  return timeStamp::ts2200();
}

COMMON_EXPORT relTimeStamp::relTimeStamp() { }

double relTimeStamp::getD(const timeUnit &u) const {
  return u.cvtFrom_nsD(get_ns());
}

int64_t relTimeStamp::getI(const timeUnit &u) const {
  return u.cvtFrom_nsI( get_ns());
}

relTimeStamp::relTimeStamp(int64_t ns_) : timeParent(ns_) { }

const relTimeStamp &relTimeStamp::Zero() {
  if(_Zero == NULL)  _Zero = ZeroHelp();
  return *_Zero;
}

COMMON_EXPORT timeLength::timeLength() 
{
}

double timeLength::getD(const timeUnit &u) const 
{
	return u.cvtFrom_nsD( get_ns());
}

int64_t timeLength::getI(const timeUnit &u) const 
{
  return u.cvtFrom_nsI( get_ns());
}

timeLength::timeLength(int64_t ns_) : timeParent(ns_)
{
}
