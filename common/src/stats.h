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

// $Id: stats.h,v 1.3 2008/09/03 06:08:43 jaw Exp $

#ifndef STATS_H
#define STATS_H

#include <string>
//#include "Dictionary.h"
#include "Timer.h"
#include "common/h/util.h"

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

class COMMON_EXPORT Statistic  {
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
    virtual ~Statistic() = default; // Avoid warnings
    Statistic(const Statistic&) = default;

    StatContainer *container_;
};

class COMMON_EXPORT CntStatistic : public Statistic {
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

    CntStatistic& operator+=( long int );
    CntStatistic& operator+=( const CntStatistic &);
    
    CntStatistic& operator-=( long int );
    CntStatistic& operator-=(  const CntStatistic &);

    // Return the value of this statistic
    long int operator*();
    long int value();

 private:
    long int cnt_;
};

/* Wraps the timer class */
class COMMON_EXPORT TimeStatistic : public Statistic {
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
class StatContainer {
 public:
    COMMON_EXPORT StatContainer(); 

    /* Access or create a statistic indexed by the provided name.
     *
     * This operator may return null if the named statistic does
     * not exist.
     */
    COMMON_EXPORT Statistic * operator[](const std::string &);
    COMMON_EXPORT Statistic * operator[](const char *s) {
       std::string namestr(s);
       return (*this)[namestr];
    }

    // Create a new statistic of the given type indexed by name.
    // **This will replace any existing stat with the same index
    //   within this container**
    COMMON_EXPORT void add(const std::string& name, StatType type);

    // Access all of the existing statistics
    COMMON_EXPORT dyn_hash_map< std::string, Statistic * > &
       allStats() { return stats_; }

    // And some pass-through methods, encapsulated for
    // ease of use
    COMMON_EXPORT void startTimer(const std::string&);
    COMMON_EXPORT void stopTimer(const std::string&);
    COMMON_EXPORT void incrementCounter(const std::string&);
    COMMON_EXPORT void decrementCounter(const std::string&);
    COMMON_EXPORT void addCounter(const std::string&, int);

 private:
    dyn_hash_map< std::string, Statistic * > stats_;

};
#endif
