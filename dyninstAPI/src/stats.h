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

// $Id: stats.h,v 1.12 2006/12/06 21:17:50 bernat Exp $

#ifndef STATS_H
#define STATS_H

#include "common/h/Dictionary.h"
#include "common/h/String.h"
#include "common/h/Timer.h"

extern void printDyninstStats();

class CntStatistic;

extern CntStatistic insnGenerated;
extern CntStatistic totalMiniTramps;
extern CntStatistic trampBytes;
extern CntStatistic ptraceOps;
extern CntStatistic ptraceOtherOps;
extern CntStatistic ptraceBytes;
extern CntStatistic pointsUsed;

class StatContainer;  // All your class declarations are forward. 

class Statistic  {
 public:
    virtual bool is_count() const { return false; }
    virtual bool is_timer() const { return false; }
    /* ... etc. */

    // dynamic_casts are a pain in the neck
    virtual long int value() { return 0; }
    virtual double usecs() const { return 0; }
    virtual double ssecs() const { return 0; }
    virtual double wsecs() const { return 0; }


 protected:
    Statistic(StatContainer *c) : 
        container_(c)
    { }
    virtual ~Statistic() {}; // Avoid warnings

    StatContainer *container_;

    // 
    bool valid;
};

class CntStatistic : public Statistic {
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
    bool is_count() const { return true; }

    /** overloaded operators **/
    CntStatistic operator++( int );
    CntStatistic& operator++();

    CntStatistic operator--( int );
    CntStatistic& operator--();  

    CntStatistic& operator=( long int );
    CntStatistic& operator=( const CntStatistic &);

    CntStatistic& operator+=( long int );
    CntStatistic& operator+=( const CntStatistic &);
    
    CntStatistic& operator-=( long int );
    CntStatistic& operator-=( const CntStatistic &);

    // Return the value of this statistic
    long int operator*();
    long int value();

 private:
    long int cnt_;
};

/* Wraps the timer class */
class TimeStatistic : public Statistic {
 friend class StatContainer;

 protected:
    TimeStatistic(StatContainer *c) :
        Statistic(c)
    {}

 public:
    TimeStatistic() :
        Statistic(NULL)
    {}

    bool is_timer() const { return true; }

    TimeStatistic& operator=(const TimeStatistic &);
    TimeStatistic& operator+=(const TimeStatistic &);
    TimeStatistic& operator+(const TimeStatistic &) const;

    void clear();
    void start();
    void stop();

    double usecs() const;
    double ssecs() const;
    double wsecs() const;

    bool is_running() const;

 private:

    timer t_;
};

typedef enum {
    CountStat,
    TimerStat
} StatType;


/* A container for a group of (one expects) mutually related statistics. */
class StatContainer {
 public:
    StatContainer() :
        stats_(pdstring::hash)
    { }

    /* Access or create a statistic indexed by the provided name.
     *
     * This operator may return null if the named statistic does
     * not exist.
     */
    Statistic * operator[](pdstring) const;

    // Create a new statistic of the given type indexed by name.
    // **This will replace any existing stat with the same index
    //   within this container**
    void add(pdstring name, StatType type);

    // Access all of the existing statistics
    const dictionary_hash< pdstring, Statistic * > &
    allStats() const { return stats_; }

    // And some pass-through methods, encapsulated for
    // ease of use
    void startTimer(pdstring);
    void stopTimer(pdstring);
    void incrementCounter(pdstring);
    void decrementCounter(pdstring);
    void addCounter(pdstring, int);

 private:
    dictionary_hash< pdstring, Statistic * > stats_;

};
#endif
