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


#ifndef TIMEMECHANISM_H
#define TIMEMECHANISM_H

#include "common/h/Time.h"
#include "common/h/language.h"
#include "common/h/String.h"
#include "rtinst/h/rtinst.h"


// These classes can be used as template arguments to the timeMechanism
// class.  When used they indicate that the time retrieval and time available
// functions that can be stored (via a function ptr) by the timeMechanism are
// not members of a class and don't take arguments (always the case for time
// available function).
class NoArgs { };
class NoClass { };

class timeMechanismBase {
 public:
  typedef rawTime64 (*nsCvtFunc_t)(rawTime64);

  timeMechanismBase(const timeUnit u, const timeBase b, 
		    const char *rtTimeQyFunc) :
    timeunit(u), timebase(b), rtTimeQueryFuncName(rtTimeQyFunc), 
    nsCvtFunc(NULL) {
  }
  timeMechanismBase(nsCvtFunc_t cvtFunc, const timeBase b, 
		    const char *rtTimeQyFunc) :
    timeunit(timeUnit::sec()), // this is just a holder value, it's not used
    timebase(b), rtTimeQueryFuncName(rtTimeQyFunc), nsCvtFunc(cvtFunc) {
  }

  // Selectors
  bool isAvailable() const { return available; }
  const timeUnit &getTimeUnit() const { return timeunit; }
  const timeBase &getTimeBase() const { return timebase; }
  const string get_rtTimeQueryFuncName() const { return rtTimeQueryFuncName; }
  nsCvtFunc_t getCvtFunc() const { return nsCvtFunc; }

 protected:
  timeUnit timeunit;
  timeBase timebase;
  mutable bool available;
  string rtTimeQueryFuncName;
  // unitConversionFunc can be used instead of the timeunit, if timeunit
  // is used instead, then func == NULL
  nsCvtFunc_t nsCvtFunc;
};


// A time Mechanism represents a "timer level" and all the things associated
// with a timer level.  For example the unit and base of the (primitive) time
// that the time querying function (also referred to as time retrieval
// function) returns.  A function that indicates whether the level is
// available, referred to as the time available function.  Also a string that
// indicates the function name of the time querying function located in the
// rtinst library.  Also a "destroy" function called when the timeMechanism
// is no longer used.  This is called by default by the mechanism destructor
// for the NoClass case, but needs to be called directly in the case that
// there is a real class template argument.

// This timeMechanism specialization occurs when a class type and argument
// type (other than NoClass, NoArgs) are passed as template arguments in
// setting up the timeMechanism.  That is, this timeMechanism would store a
// time querying and time available function that are members of a class and
// a time querying function that takes arguments.
template<class dmTimeFuncClass_t = NoClass, class dmTimeQyFuncParam_t = NoArgs>
class timeMechanism : public timeMechanismBase {
  public:
  typedef dmTimeFuncClass_t dtClass_t;
  typedef dmTimeQyFuncParam_t dtParam_t;

  typedef int64_t (dmTimeFuncClass_t:: *timeQueryFunc_t)(dtParam_t);
  typedef bool (dmTimeFuncClass_t:: *timeAvailFunc_t)();
  typedef void (dmTimeFuncClass_t:: *timeDestroyFunc_t)();

  private:
  timeAvailFunc_t  timeAvailFunc;
  timeQueryFunc_t  dmQueryFunc;
  timeDestroyFunc_t timeDestroyFunc;
  
  public:
  timeMechanism(timeAvailFunc_t taf, const timeUnit u, const timeBase b, 
		timeQueryFunc_t dqf, const char *rtTimeQyFunc, 
		timeDestroyFunc_t df = NULL) : 
  timeMechanismBase(u, b,rtTimeQyFunc), timeAvailFunc(taf), 
  dmQueryFunc(dqf), timeDestroyFunc(df) {
  }
  timeMechanism(timeAvailFunc_t taf, nsCvtFunc_t cvtFunc, const timeBase b, 
		timeQueryFunc_t dqf, const char *rtTimeQyFunc, 
		timeDestroyFunc_t df = NULL) : 
  timeMechanismBase(cvtFunc, b,rtTimeQyFunc), timeAvailFunc(taf), 
  dmQueryFunc(dqf), timeDestroyFunc(df) {
  }
  ~timeMechanism() {
    // can't call destroyTimer method from here because unaware of 
    // containing object
  }

  timeQueryFunc_t getDmTimeQueryFunc() const { return dmQueryFunc; }
  void destroyMechTimer(dtClass_t *obj) {
    if(timeDestroyFunc != static_cast<timeDestroyFunc_t>(NULL)) {
      (obj->*timeDestroyFunc)();
    }
  }

  // Mutators
  bool tryAvailable(dtClass_t *obj) const {
    available = (obj->*timeAvailFunc)(); 
    return available;
  }
};

template<class dmTimeFuncClass_t, class dmTimeQyFuncParam_t>
inline ostream& operator<<(ostream&s, const timeMechanism<dmTimeFuncClass_t, dmTimeQyFuncParam_t> &l) {
  return s << "[level- rtFuncName: " << l.get_rtTimeQueryFuncName() 
	   << "  timeunit: " << l.getTimeUnit() << "  timebase: "
	   << l.getTimeBase() << " ]";
}

/* This code currently won't compile with Visual C++ 6.0 (NT).  See Visual
   C++, BUG: ERROR C2989 and C2988 on Class Template Partial Specialization.
   Fortunately, it's currently not needed.  Currently, the only template
   instantiations are of timeMgr cpuTimeMgr<process, int> and timeMgr
   wallTimeMgr<> (and thus timeMechanism<process, int> and timeMechanism<>).
   We'll keep around in case of a future need.  */
/*
template<class dmTimeFuncClass_t>
class timeMechanism<dmTimeFuncClass_t, NoArgs> : public timeMechanismBase {
  public:
  typedef dmTimeFuncClass_t dtClass_t;
  typedef NoArgs dtParam_t;

  typedef int64_t (dtClass_t:: *timeQueryFunc_t)();
  typedef bool (dtClass_t:: *timeAvailFunc_t)();

  private:
  timeAvailFunc_t timeAvailFunc;
  timeQueryFunc_t dmQueryFunc;
  
  public:
  timeMechanism(timeAvailFunc_t taf, const timeUnit u, const timeBase b, 
		timeQueryFunc_t dqf, const char *rtTimeQyFunc) : 
     timeMechanismBase(u, b,rtTimeQyFunc), timeAvailFunc(taf), dmQueryFunc(dqf) {
  }

  timeQueryFunc_t getDmTimeQueryFunc() const { return dmQueryFunc; }

  // Mutators
  bool tryAvailable(dtClass_t *obj) const {
    available = (obj->*timeAvailFunc)(); 
    return available;
  }
};

template<class dmTimeQyFuncParam_t> 
class timeMechanism<NoClass, dmTimeQyFuncParam_t> : public timeMechanismBase {
  public:
  typedef NoClass dtClass_t;
  typedef dmTimeQyFuncParam_t dtParam_t;

  typedef int64_t (*timeQueryFunc_t)(dmTimeQyFuncParam_t);
  typedef bool (*timeAvailFunc_t)();

  private:
  timeAvailFunc_t timeAvailFunc;
  timeQueryFunc_t dmQueryFunc;
  
  public:
  timeMechanism(timeAvailFunc_t taf, const timeUnit u, const timeBase b, 
		timeQueryFunc_t dqf, const char *rtTimeQyFunc) : 
    timeMechanismBase(u, b, rtTimeQyFunc), timeAvailFunc(taf), dmQueryFunc(dqf) {
  }

  timeQueryFunc_t getDmTimeQueryFunc() const { return dmQueryFunc; }

  // Mutators
  bool tryAvailable() const {
    available = (*timeAvailFunc)(); 
    return available;
  }
};
*/

// This timeMechanism specialization occurs when no class type or argument
// type is passed as template arguments and thus NoClass and NoArgs classes
// get passed as default template arguments.  This results in a timeMechanism
// that stores a time querying function and time available function that are
// not members of a class and take no arguments.
template<>
class timeMechanism<NoClass, NoArgs> : public timeMechanismBase {
  public:
  typedef NoClass dtClass_t;
  typedef NoArgs dtParam_t;

  typedef int64_t (*timeQueryFunc_t)();
  typedef bool (*timeAvailFunc_t)();
  typedef void (*timeDestroyFunc_t)();

  private:
  timeAvailFunc_t   timeAvailFunc;
  timeQueryFunc_t   dmQueryFunc;
  timeDestroyFunc_t timeDestroyFunc;

  public:
  timeMechanism(timeAvailFunc_t taf, const timeUnit u, const timeBase b, 
		timeQueryFunc_t dqf, const char *rtTimeQyFunc, 
		timeDestroyFunc_t df = NULL) : 
    timeMechanismBase(u, b,rtTimeQyFunc), timeAvailFunc(taf), 
    dmQueryFunc(dqf), timeDestroyFunc(df) {
  }
  timeMechanism(timeAvailFunc_t taf, nsCvtFunc_t cvtFunc, const timeBase b, 
		timeQueryFunc_t dqf, const char *rtTimeQyFunc, 
		timeDestroyFunc_t df = NULL) : 
    timeMechanismBase(cvtFunc, b,rtTimeQyFunc), timeAvailFunc(taf), 
    dmQueryFunc(dqf), timeDestroyFunc(df) {
  }
  ~timeMechanism() {
    if(timeDestroyFunc != static_cast<timeDestroyFunc_t>(NULL)) {
      (*timeDestroyFunc)();
    }
  }

  timeQueryFunc_t getDmTimeQueryFunc() const { return dmQueryFunc; }

  // Mutators
  bool tryAvailable() const {
    available = (*timeAvailFunc)(); 
    return available;
  }
};







#endif
