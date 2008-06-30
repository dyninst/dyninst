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

// $Id: stats.h,v 1.2 2008/06/30 17:33:25 legendre Exp $

#ifndef STATS_H
#define STATS_H

#include <string>
//#include "Dictionary.h"
#include "Timer.h"
#include "dynutil/h/util.h"

#if 0
extern void printDyninstStats();

class CntStatistic;

extern CntStatistic insnGenerated;
extern CntStatistic totalMiniTramps;
extern CntStatistic trampBytes;
extern CntStatistic ptraceOps;
extern CntStatistic ptraceOtherOps;
extern CntStatistic ptraceBytes;
extern CntStatistic pointsUsed;

#endif
class StatContainer;  // All your class declarations are forward. 

class DLLEXPORT Statistic  {
 public:
    virtual bool is_count()  { return false; }
    virtual bool is_timer()  { return false; }
    /* ... etc. */

    // dynamic_casts are a pain in the neck
    virtual long int value() { return 0; }
    virtual double usecs()  { return 0; }
    virtual double ssecs() { return 0; }
    virtual double wsecs() { return 0; }


 protected:
    Statistic(StatContainer *c) : 
        container_(c)
    { }
    virtual ~Statistic() {}; // Avoid warnings

    StatContainer *container_;

    // 
    bool valid;
};

class DLLEXPORT CntStatistic : public Statistic {
 friend class StatContainer;

 protected:
    // Constructor with which a StatContainer provides
    // this statistic with an up-pointer to itself
    CntStatistic(StatContainer *c) :
        Statistic(c),
        cnt_(0)
    { }

 public:
    CntStatistic() : 
        Statistic(NULL),
        cnt_(0)
    { }

    // respond appropriately to type-of-stat requests
    bool is_count() { return true; }

    /** overloaded operators **/
    CntStatistic operator++( int );
    CntStatistic& operator++();

    CntStatistic operator--( int );
    CntStatistic& operator--();  

    CntStatistic& operator=( long int );
    CntStatistic& operator=( CntStatistic &);

    CntStatistic& operator+=( long int );
    CntStatistic& operator+=( CntStatistic &);
    
    CntStatistic& operator-=( long int );
    CntStatistic& operator-=(  CntStatistic &);

    // Return the value of this statistic
    long int operator*();
    long int value();

 private:
    long int cnt_;
};

/* Wraps the timer class */
class DLLEXPORT TimeStatistic : public Statistic {
 friend class StatContainer;

 protected:
    TimeStatistic(StatContainer *c) :
        Statistic(c)
    {}

 public:
    TimeStatistic() :
        Statistic(NULL)
    {}

    bool is_timer()  { return true; }

    TimeStatistic& operator=( TimeStatistic &);
    TimeStatistic& operator+=( TimeStatistic &);
    TimeStatistic& operator+( TimeStatistic &) ;

    void clear();
    void start();
    void stop();

    double usecs() ;
    double ssecs() ;
    double wsecs() ;

    bool is_running() ;

 private:

    timer t_;
};

typedef enum {
    CountStat,
    TimerStat
} StatType;


/* A container for a group of (one expects) mutually related statistics. */
class DLLEXPORT StatContainer {
 public:
    StatContainer(); 

    /* Access or create a statistic indexed by the provided name.
     *
     * This operator may return null if the named statistic does
     * not exist.
     */
    Statistic * operator[](std::string &);
    Statistic * operator[](const char *s) {
       std::string namestr(s);
       return (*this)[namestr];
    }

    // Create a new statistic of the given type indexed by name.
    // **This will replace any existing stat with the same index
    //   within this container**
    void add(std::string name, StatType type);

    // Access all of the existing statistics
    dyn_hash_map< std::string, Statistic * > &
    allStats() { return stats_; }

    // And some pass-through methods, encapsulated for
    // ease of use
    void startTimer(std::string);
    void stopTimer(std::string);
    void incrementCounter(std::string);
    void decrementCounter(std::string);
    void addCounter(std::string, int);

 private:
    dyn_hash_map< std::string, Statistic * > stats_;

};
#endif
