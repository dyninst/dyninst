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

/*
 * Report statistics about dyninst and data collection.
 * $Id: stats.C,v 1.2 2008/07/01 19:26:49 legendre Exp $
 */

#include "common/src/headers.h"
#include "common/src/stats.h"
#include "common/h/util.h"
#include <sstream>
#include <string>
#if 0
#include "dyninstAPI/src/stats.h"

CntStatistic trampBytes;
CntStatistic pointsUsed;
CntStatistic insnGenerated;
CntStatistic totalMiniTramps;
double timeCostLastChanged=0;
// HTable<resourceListRec*> fociUsed;
// HTable<metric*> metricsUsed;
CntStatistic ptraceOtherOps, ptraceOps, ptraceBytes;

void printDyninstStats()
{
    sprintf(errorLine, "    %ld total points used\n", pointsUsed.value());
    logLine(errorLine);
    sprintf(errorLine, "    %ld mini-tramps used\n", totalMiniTramps.value());
    logLine(errorLine);
    sprintf(errorLine, "    %ld tramp bytes\n", trampBytes.value());
    logLine(errorLine);
    sprintf(errorLine, "    %ld ptrace other calls\n", ptraceOtherOps.value());
    logLine(errorLine);
    sprintf(errorLine, "    %ld ptrace write calls\n", 
                ptraceOps.value()-ptraceOtherOps.value());
    logLine(errorLine);
    sprintf(errorLine, "    %ld ptrace bytes written\n", ptraceBytes.value());
    logLine(errorLine);
    sprintf(errorLine, "    %ld instructions generated\n", 
                insnGenerated.value());
    logLine(errorLine);
}
#endif

StatContainer::StatContainer() 
   //stats_(::Dyninst::hash)
{
}

Statistic*
StatContainer::operator[](const std::string &name) 
{
    auto const& it = stats_.find(name);
    return (it != stats_.end()) ? it->second : NULL;
}

void
StatContainer::add(const std::string& name, StatType type)
{
    Statistic *s = NULL;

    switch(type) {
        case CountStat:
            s = (Statistic *)(new CntStatistic(this));
            break;            
        case TimerStat:
            s = (Statistic *)(new TimeStatistic(this));
            break;
        default:
          fprintf(stderr,"Error adding statistic %s: type %d not recognized\n",
                name.c_str(), type);
          return;
    }

    stats_[name] = s;
}

void StatContainer::startTimer(const std::string& name) 
{
    TimeStatistic *timer = dynamic_cast<TimeStatistic *>(stats_[name]);
    if (!timer) return;
    timer->start();
}

void StatContainer::stopTimer(const std::string& name) {
    TimeStatistic *timer = dynamic_cast<TimeStatistic *>(stats_[name]);
    if (!timer) return;
    timer->stop();
}

void StatContainer::incrementCounter(const std::string& name) {
    CntStatistic *counter = dynamic_cast<CntStatistic *>(stats_[name]);
    if (!counter) return;
    (*counter)++;
}

void StatContainer::decrementCounter(const std::string& name) {
    CntStatistic *counter = dynamic_cast<CntStatistic *>(stats_[name]);
    if (!counter) return;
    (*counter)--;
}

void StatContainer::addCounter(const std::string& name, int val) {
    CntStatistic *counter = dynamic_cast<CntStatistic *>(stats_[name]);
    if (!counter) return;
    (*counter) += val;
}

                                  
CntStatistic 
CntStatistic::operator++( int )
{
    CntStatistic ret = *this;
    ++(*this);
    return ret;
}


CntStatistic& 
CntStatistic::operator++()
{
    cnt_++;
    return *this;
}
                               
CntStatistic 
CntStatistic::operator--( int )
{
    CntStatistic ret = *this;
    --(*this);
    return ret;
}

CntStatistic&
CntStatistic::operator--()
{
    cnt_--;
    return *this;
}
                                 
CntStatistic& 
CntStatistic::operator=( long int v )
{
    cnt_ = v;
    return *this;
}

CntStatistic& 
CntStatistic::operator+=( long int v )
{
    cnt_ += v;
    return *this;
}

CntStatistic&
CntStatistic::operator+=( const CntStatistic & v )
{
    cnt_ += v.cnt_;
    return *this;
}

    
CntStatistic& 
CntStatistic::operator-=( long int v )
{
    cnt_ -= v;
    return *this;
}

CntStatistic& 
CntStatistic::operator-=( const CntStatistic & v)
{
    cnt_ -= v.cnt_;
    return *this;
}

inline long int 
CntStatistic::operator*()
{
    return cnt_;
}

long int 
CntStatistic::value()
{
    return cnt_;
}


TimeStatistic& 
TimeStatistic::operator=(TimeStatistic & t)
{
    if( this != &t ) {
        t_ = t.t_;
    }
    return *this;
}

TimeStatistic& 
TimeStatistic::operator+=(TimeStatistic & t)
{
    t_ += t.t_;
    return *this;
}

TimeStatistic& 
TimeStatistic::operator+(TimeStatistic & t) 
{
    TimeStatistic * ret = new TimeStatistic;
    *ret = *this;
    *ret += t;
    return *ret;
}

void 
TimeStatistic::clear()
{
    t_.clear();
}

void 
TimeStatistic::start()
{
    t_.start();
}

void
TimeStatistic::stop()
{
    t_.stop();
}

double 
TimeStatistic::usecs() 
{
    return t_.usecs();
}

double 
TimeStatistic::ssecs() 
{
    return t_.ssecs();
}

double 
TimeStatistic::wsecs() 
{
    return t_.wsecs();
}

bool 
TimeStatistic::is_running() 
{
    return t_.is_running();
}

