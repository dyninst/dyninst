#include <iostream>
#include <time.h>
#include <string.h>
#include <iomanip>
#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include "common/h/Time.h"
#include "common/h/int64iostream.h"


timeUnit::ostream_fmt timeUnit::curFmt = timeUnit::sparse;

const timeUnit *timeUnit::_ns = NULL;
const timeUnit *timeUnit::_us = NULL;
const timeUnit *timeUnit::_ms = NULL;
const timeUnit *timeUnit::_sec = NULL;
const timeUnit *timeUnit::_min = NULL;
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
  int64_t ns_per_min = min().get_ns_per_unit().getNumer();
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
  return (fraction &)units_per_ns * (double)nsec;
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
const timeLength *timeLength::_min = NULL;
const timeLength *timeLength::minHelp() {
  return new timeLength(1, timeUnit::min());
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
  int mins  = static_cast<int>(tUser.getI(timeUnit::min()));
  tUser -= timeLength(mins,timeUnit::min());
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
  int64_t max_time_t = (I64_C(1)<<((sizeof(time_t)*8)-1))-1;  // eg. (2^(32-1)) -1
  if(! z.isInitialized()) {  
    s << "[uninitialized]";
  } else if(z < timeStamp(0, timeUnit::year(), timeBase::b1970()) ||
	    z > timeStamp(max_time_t, timeUnit::sec(), timeBase::b1970())) {
    timeLength tl(z - timeStamp(0, timeUnit::year(), timeBase::b1970()));
    // setting the oct flag is just a hack to tell the timeLength ostream<<
    // to print the time compressed
#if !defined(mips_sgi_irix6_4) && !defined(i386_unknown_nt4_0)  &&!defined(mips_unknown_ce2_11) //ccw 10 apr 2001
    ostream::fmtflags oldflags;
#else
    long oldflags;  // irix,nt environment doesn't define ostream::fmtflags
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
const double operator/(const timeLength a, const timeLength b) {
  assert(a.isInitialized() && b.isInitialized());
  return static_cast<double>(a.get_ns()) / static_cast<double>(b.get_ns());
}
