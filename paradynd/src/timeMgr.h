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


#ifndef TIMEMGR_HDR
#define TIMEMGR_HDR

#include "common/h/Types.h"
#include "common/h/language.h"
#include "common/h/Time.h"
#include "paradynd/src/timeMechanism.h"

// Responsibilities:
//   - manage the different time mechanism levels
//   - retrieve current wall or cpu times for best time mechanism available
//   - allow conversion into nanoseconds (for construction of Time object)
//         from the primitive time unit TimeMgr chooses (for rtinst library)


class Time;

//--- timeMgrBase class --------------------
template<class dmTimeFuncClass_t = NoClass, class dmTimeQyFuncParam_t = NoArgs>
class timeMgrBase {
  public:
  enum { MaxLevels = 3 };
  // Level one will be used for time retrieval functions that access hardware
  // directly.  Level two will be used for time retrieval functions that
  // require going through the operating system.  The number of levels can be
  // easily increased if a need becomes evident.

  enum timeMechLevel { LEVEL_ONE=0, LEVEL_TWO=1, LEVEL_BEST=2 };

  typedef timeMechanism<dmTimeFuncClass_t,dmTimeQyFuncParam_t> mech_t;
#define MECH_T timeMechanism<dmTimeFuncClass_t,dmTimeQyFuncParam_t>


  timeMgrBase();
  virtual ~timeMgrBase();

  // Selectors
  const timeUnit &getTimeUnit(timeMechLevel l=LEVEL_BEST) const;
  const timeBase &getTimeBase(timeMechLevel l=LEVEL_BEST) const;
  const string get_rtTimeQueryFuncName(timeMechLevel l=LEVEL_BEST) const;
  timeMechLevel getBestLevel() const;
  MECH_T *getMechLevel(timeMechLevel l) { 
    return mechLevels[int(l)]; 
  }
  const MECH_T *getMechLevel(timeMechLevel l) const {
    return mechLevels[int(l)];
  }

  // LEVEL_ONE will be chosen first if available, LEVEL_FOUR is last choice
  //  - can have spaces in installed levels
  //  - don't need timeMechanisms installed
  void installLevel(timeMechLevel l, TYPENAME MECH_T::timeAvailFunc_t taf,
		    const timeUnit u, const timeBase b, 
		    TYPENAME MECH_T::timeQueryFunc_t dmTimerFunc,
		    const char *rtTimerFunc,
		    TYPENAME MECH_T::timeDestroyFunc_t destroyFunc = NULL);
  // LEVEL_ONE will be chosen first if available, LEVEL_FOUR is last choice
  //  - can have spaces in installed levels
  //  - don't need timeMechanisms installed
  void installLevel(timeMechLevel l, TYPENAME MECH_T::timeAvailFunc_t taf,
		    TYPENAME MECH_T::nsCvtFunc_t cvtToNsFunc, const timeBase b,
		    TYPENAME MECH_T::timeQueryFunc_t dmTimerFunc,
		    const char *rtTimerFunc, 
		    TYPENAME MECH_T::timeDestroyFunc_t destroyFunc = NULL);
  // timeMgr works with int64_t instead of rawTime64 because I consider
  // rawTime64 a paradyn concept, and I want to keep the timeMgr decoupled
  // from paradyn as best as possible
  timeLength units2timeLength(int64_t rawunits, timeMechLevel l=LEVEL_BEST);
  timeStamp units2timeStamp(int64_t rawunits, timeMechLevel l=LEVEL_BEST);

  void installMechLevel(timeMechLevel l, MECH_T *mptr) { 
    mechLevels[int(l)] = mptr;
  }
  protected:
  void errLevelNotInstalled() const {
    cerr << "timeMgr: Requested level not installed\n";
  }

  private:
  MECH_T  *mechLevels[MaxLevels];
};

//--- member definitions --------------------
// timeMgrBase()
template<class dmTimeFuncClass_t, class dmTimeQyFuncParam_t>
inline timeMgrBase<dmTimeFuncClass_t, dmTimeQyFuncParam_t>::timeMgrBase() {
  for(timeMechLevel level=LEVEL_ONE; ; level=timeMechLevel(int(level)+1)) {
    installMechLevel(level, NULL);
    if(level == LEVEL_BEST) break;
  }
}

// ~timeMgrBase()
template<class dmTimeFuncClass_t, class dmTimeQyFuncParam_t>
inline timeMgrBase<dmTimeFuncClass_t, dmTimeQyFuncParam_t>::~timeMgrBase() {
  for(timeMechLevel level=LEVEL_ONE; ; level=timeMechLevel(int(level)+1)) {
    // LEVEL_BEST is actually just a reference to another timeMech
    if(level == LEVEL_BEST) break;
    MECH_T *mechToUse = getMechLevel(level);
    if(mechToUse != NULL) {
      delete mechToUse;
    }
  }
}

// getTimeUnit()
template<class dmTimeFuncClass_t, class dmTimeQyFuncParam_t>
inline const timeUnit &timeMgrBase<dmTimeFuncClass_t, dmTimeQyFuncParam_t>::
getTimeUnit(timeMechLevel l) const { 
  const MECH_T *pMech = getMechLevel(l); 
  if(pMech == NULL)  {  errLevelNotInstalled();  return timeUnit::ns(); }
  return pMech->getTimeUnit();
}

// getTimeBase()
template<class dmTimeFuncClass_t, class dmTimeQyFuncParam_t>
inline const timeBase &timeMgrBase<dmTimeFuncClass_t, dmTimeQyFuncParam_t>::
getTimeBase(timeMechLevel l) const { 
  const MECH_T *pMech = getMechLevel(l); 
  if(pMech == NULL) {  errLevelNotInstalled();  return timeBase::bStd(); }
  return pMech->getTimeBase();
}

// get_rtTimeQueryFuncName()
template<class dmTimeFuncClass_t, class dmTimeQyFuncParam_t>
inline const string timeMgrBase<dmTimeFuncClass_t, dmTimeQyFuncParam_t>::
get_rtTimeQueryFuncName(timeMechLevel l) const {
  const MECH_T *pMech = getMechLevel(l);
  if(pMech == NULL) {  errLevelNotInstalled();  return string(""); }
  return pMech->get_rtTimeQueryFuncName();
}

// installLevel() for passing timeUnit
template<class dmTimeFuncClass_t, class dmTimeQyFuncParam_t>
inline void timeMgrBase<dmTimeFuncClass_t, dmTimeQyFuncParam_t>::
installLevel(timeMechLevel l, TYPENAME MECH_T::timeAvailFunc_t taf,
             const timeUnit u, const timeBase b,
             TYPENAME MECH_T::timeQueryFunc_t dmTimerFunc,
	     const char *rtTimerFunc, 
	     TYPENAME MECH_T::timeDestroyFunc_t destroyFunc) {
  MECH_T *curMech = new MECH_T(taf, u, b, dmTimerFunc,rtTimerFunc,destroyFunc);
  installMechLevel(l, curMech);
}

// installLevel() for passing nsCvtFunc
template<class dmTimeFuncClass_t, class dmTimeQyFuncParam_t>
inline void timeMgrBase<dmTimeFuncClass_t, dmTimeQyFuncParam_t>::
installLevel(timeMechLevel l, TYPENAME MECH_T::timeAvailFunc_t taf,
	     TYPENAME MECH_T::nsCvtFunc_t cvtToNsFunc, const timeBase b, 
	     TYPENAME MECH_T::timeQueryFunc_t dmTimerFunc,
             const char *rtTimerFunc,
	     TYPENAME MECH_T::timeDestroyFunc_t destroyFunc) {
  MECH_T *curMech = new MECH_T(taf, cvtToNsFunc, b, dmTimerFunc, rtTimerFunc,
			       destroyFunc);
  installMechLevel(l, curMech);
}

// units2timeLength()
template<class dmTimeFuncClass_t, class dmTimeQyFuncParam_t>
inline timeLength timeMgrBase<dmTimeFuncClass_t, dmTimeQyFuncParam_t>::
units2timeLength(int64_t rawunits, timeMechLevel l) {
  MECH_T *mechToUse = getMechLevel(l);
  if(mechToUse == NULL) {  errLevelNotInstalled(); return timeLength::Zero(); }
  TYPENAME MECH_T::nsCvtFunc_t nsCvtFunc = mechToUse->getCvtFunc();
  if(nsCvtFunc == NULL) {
    return timeLength(rawunits, mechToUse->getTimeUnit());
  } else {
    rawTime64 ns = (*nsCvtFunc)(rawunits);
    return timeLength(ns, timeUnit::ns());
  }
}

// units2timeStamp()
template<class dmTimeFuncClass_t, class dmTimeQyFuncParam_t>
inline timeStamp timeMgrBase<dmTimeFuncClass_t, dmTimeQyFuncParam_t>::
units2timeStamp(int64_t rawunits, timeMechLevel l) {
  MECH_T *mechToUse = getMechLevel(l);
  if(mechToUse == NULL) {  errLevelNotInstalled(); return timeStamp::tsStd(); }
  TYPENAME MECH_T::nsCvtFunc_t nsCvtFunc = mechToUse->getCvtFunc();
  if(nsCvtFunc == NULL) {
    return timeStamp(rawunits, mechToUse->getTimeUnit(),
		     mechToUse->getTimeBase());
  } else {
    rawTime64 ns = (*nsCvtFunc)(rawunits);
    return timeStamp(ns, timeUnit::ns(), mechToUse->getTimeBase());
  }
}

//  timeMechLevel getBestLevel();
template<class dmTimeFuncClass_t, class dmTimeQyFuncParam_t>
inline TYPENAME timeMgrBase<dmTimeFuncClass_t,dmTimeQyFuncParam_t>::timeMechLevel timeMgrBase<dmTimeFuncClass_t,dmTimeQyFuncParam_t>::
getBestLevel() const { 
  const MECH_T *bestMech = getMechLevel(LEVEL_BEST);
  for(timeMechLevel level=LEVEL_ONE; level != LEVEL_BEST ; 
      level=timeMechLevel(int(level)+1)) {
    const MECH_T *tm = getMechLevel(level);
    if(tm != NULL && tm == bestMech) return level;
  }
  cerr << "Best Level unable to be determined\n";
  return LEVEL_TWO;
  // a best level hasn't been chosen yet
}

//--- timeMgr Class  ----------------------------------------------------------
//--- timeMgr: <Class>, <Arg> --------------------
template<class dmTimeFuncClass_t = NoClass, class dmTimeQyFuncParam_t = NoArgs>
class timeMgr : public timeMgrBase<dmTimeFuncClass_t, dmTimeQyFuncParam_t> {
  public:
  typedef TYPENAME timeMgrBase<dmTimeFuncClass_t, dmTimeQyFuncParam_t>::timeMechLevel timeMechLevelC;
  typedef TYPENAME timeMgrBase<dmTimeFuncClass_t, dmTimeQyFuncParam_t>::mech_t mechC_t; 

  timeStamp getTime(TYPENAME mech_t::dtClass_t* pObj,
                    TYPENAME mech_t::dtParam_t timeFuncArg, 
		    timeMechLevelC l) {
    mechC_t *mechToUse = getMechLevel(l);
    if(mechToUse == NULL) { errLevelNotInstalled(); return timeStamp::tsStd();}
    TYPENAME mech_t::timeQueryFunc_t daemonFunc = 
       mechToUse->getDmTimeQueryFunc();
    TYPENAME mech_t::nsCvtFunc_t nsCvtFunc = mechToUse->getCvtFunc();
    int64_t rawunits = (pObj->*daemonFunc)(timeFuncArg);
    if(nsCvtFunc == NULL) {
      return timeStamp(rawunits, mechToUse->getTimeUnit(),
		       mechToUse->getTimeBase());
    } else {
      rawTime64 ns = (*nsCvtFunc)(rawunits);
      return timeStamp(ns, timeUnit::ns(), mechToUse->getTimeBase());
    }
  }
  int64_t getRawTime(TYPENAME mech_t::dtClass_t* pObj, 
                     TYPENAME mech_t::dtParam_t timeFuncArg, 
		     timeMechLevelC l) {
    mechC_t *mechToUse = getMechLevel(l);
    if(mechToUse == NULL) { errLevelNotInstalled(); return 0; }
    TYPENAME mech_t::timeQueryFunc_t daemonFunc = 
                                            mechToUse->getDmTimeQueryFunc();
    int64_t rawunits = (pObj->*daemonFunc)(timeFuncArg);
    return rawunits;
  }
  void destroyMechTimers(TYPENAME mech_t::dtClass_t* pObj) {
    for(timeMechLevelC level=LEVEL_ONE; ; level=timeMechLevel(int(level)+1)) {
      mechC_t *mechToUse = getMechLevel(level);
      if(mechToUse != NULL && level != LEVEL_BEST) 
        mechToUse->destroyMechTimer(pObj);
      installMechLevel(level, NULL);
      if(level == LEVEL_BEST) break;
    }
  }

  // determines the best levels, must be done before accessing current
  //   (ie. best) level
  void determineBestLevels(TYPENAME mech_t::dtClass_t *pObj) {
    for(timeMechLevelC level=LEVEL_ONE; ; level=timeMechLevel(int(level)+1)) {
      mechC_t *tm = getMechLevel(level);
      if(tm!=NULL && tm->tryAvailable(pObj)) {
	installMechLevel(LEVEL_BEST, tm);
	break;
      }
      if(level == LEVEL_BEST) break;
    }
  }

};

/* This code currently won't compile with Visual C++ 6.0 (NT).  See Visual
   C++, BUG: ERROR C2989 and C2988 on Class Template Partial Specialization.
   Fortunately, it's currently not needed.  Currently, the only template
   instantiations are of timeMgr cpuTimeMgr<process, int> and timeMgr
   wallTimeMgr<> (and thus timeMechanism.  Keep around in case of a future
   need. */
/*
//--- timeMgr: <Class>, NoArgs Specialization ----
template<class dmTimeFuncClass_t>
class timeMgr<dmTimeFuncClass_t, NoArgs> : 
             public timeMgrBase<dmTimeFuncClass_t, NoArgs> {
 public:
  
  timeStamp getTime(TYPENAME mech_t::dtClass_t* pObj, timeMechLevel l) {
    mech_t *mechToUse = getMechLevel(l);
    if(mechToUse == NULL) { errLevelNotInstalled(); return timeStamp::tsStd();}
    TYPENAME mech_t::timeQueryFunc_t daemonFunc = 
       mechToUse->getDmTimeQueryFunc();
    int64_t rawunits = (pObj->*daemonFunc)();
    return timeStamp(rawunits,mechToUse->getTimeUnit(),
                     mechToUse->getTimeBase());
  }
  int64_t getRawTime(TYPENAME mech_t::dtClass_t* pObj, timeMechLevel l) {
    mech_t *mechToUse = getMechLevel(l);
    if(mechToUse == NULL) { errLevelNotInstalled(); return 0; }
    TYPENAME mech_t::timeQueryFunc_t daemonFunc =
       mechToUse->getDmTimeQueryFunc();
    int64_t rawunits = (pObj->*daemonFunc)();
    return rawunits;
  }
  // determines the best levels, must be done before accessing current
  //   (ie. best) level
  void determineBestLevels(TYPENAME mech_t::dtClass_t *pObj) {
    for(timeMechLevel level=LEVEL_ONE; ; level=timeMechLevel(int(level)+1)) {
      mech_t *tm = getMechLevel(level);
      if(tm!=NULL && tm->tryAvailable(pObj)) {
	installMechLevel(LEVEL_BEST, tm);
	break;
      }
      if(level == LEVEL_BEST) break;
    }
  }
};

//--- timeMgr: NoClass, <Args> Specialization ----
template<class dmTimeQyFuncParam_t>
class timeMgr<NoClass, dmTimeQyFuncParam_t> : 
             public timeMgrBase<NoClass, dmTimeQyFuncParam_t> {
 public:

  timeStamp getTime(TYPENAME mech_t::dtParam_t timeFuncArg, timeMechLevel l) {
    mech_t *mechToUse = getMechLevel(l);
    if(mechToUse == NULL) { errLevelNotInstalled(); return timeStamp::tsStd();}
    TYPENAME mech_t::timeQueryFunc_t daemonFunc =
       mechToUse->getDmTimeQueryFunc();
    int64_t rawunits = (*daemonFunc)(timeFuncArg);
    return timeStamp(rawunits, mechToUse->getTimeUnit(),
                     mechToUse->getTimeBase());
  }
  int64_t getRawTime(TYPENAME mech_t::dtParam_t timeFuncArg, timeMechLevel l) {
    mech_t *mechToUse = getMechLevel(l);
    if(mechToUse == NULL) { errLevelNotInstalled(); return 0; }
    TYPENAME mech_t::timeQueryFunc_t daemonFunc =
       mechToUse->getDmTimeQueryFunc();
    int64_t rawunits = (*daemonFunc)(timeFuncArg);
    return rawunits;
  }
  // determines the best levels, must be done before accessing current
  //   (ie. best) level
  void determineBestLevels() {
    for(timeMechLevel level=LEVEL_ONE; ; level=timeMechLevel(int(level)+1)) {
      mech_t *tm = getMechLevel(level);
      if(tm!=NULL && tm->tryAvailable()) {
	installMechLevel(LEVEL_BEST, tm);
	break;
      }
      if(level == LEVEL_BEST) break;
    }
  }

};
*/


//--- timeMgr: NoClass, NoArgs Specialization ----
template<>
class timeMgr<NoClass, NoArgs> : public timeMgrBase<NoClass, NoArgs> {
 public:
  typedef timeMechanism<NoClass,NoArgs> nmech_t;

  timeStamp getTime(timeMechLevel l) {
    nmech_t *mechToUse = getMechLevel(l);
    if(mechToUse == NULL) { errLevelNotInstalled(); return timeStamp::tsStd();}
    nmech_t::timeQueryFunc_t daemonFunc = mechToUse->getDmTimeQueryFunc();
    nmech_t::nsCvtFunc_t nsCvtFunc = mechToUse->getCvtFunc();
    int64_t rawunits = (*daemonFunc)();
    if(nsCvtFunc == NULL) {
      return timeStamp(rawunits, mechToUse->getTimeUnit(),
		       mechToUse->getTimeBase());
    } else {
      rawTime64 ns = (*nsCvtFunc)(rawunits);
      return timeStamp(ns, timeUnit::ns(), mechToUse->getTimeBase());
    }
  }
  int64_t getRawTime(timeMechLevel l) {
    nmech_t *mechToUse = getMechLevel(l);
    if(mechToUse == NULL) { errLevelNotInstalled(); return 0; }
    nmech_t::timeQueryFunc_t daemonFunc = mechToUse->getDmTimeQueryFunc();
    int64_t rawunits = (*daemonFunc)();
    return rawunits;
  }
  // determines the best levels, must be done before accessing current
  //   (ie. best) level
  void determineBestLevels() {
    for(timeMechLevel level=LEVEL_ONE; ; level=timeMechLevel(int(level)+1)) {
      nmech_t *tm = getMechLevel(level);
      if(tm!=NULL && tm->tryAvailable()) {
	installMechLevel(LEVEL_BEST, tm);
	break;
      }
      if(level == LEVEL_BEST) break;
    }
  }
};





#endif



