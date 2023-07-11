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

#ifndef DYNTHREAD_H
#define DYNTHREAD_H

#include <vector>
#include "frame.h"

#include "PCProcess.h"

/*
 * pcThread.h
 *
 * A class that encapsulates a ProcControlAPI thread. This class is meant
 * to be a replacement for the old dyn_thread class.
 */

class PCProcess;
typedef Dyninst::THR_ID dynthread_t;
class inferiorRPCinProgress; 

class PCThread {
    friend class PCProcess;
    friend class PCEventHandler;

public:
    static PCThread *createPCThread(PCProcess *parent, ProcControlAPI::Thread::ptr thr);

    // Stackwalking interface
    bool walkStack(std::vector<Frame> &stackWalk);
    Frame getActiveFrame();
    bool getRegisters(ProcControlAPI::RegisterPool &regs, bool includeFP = false);
    bool changePC(Address newPC);

    // Field accessors
    int getIndex() const;
    Dyninst::LWP getLWP() const;
    PCProcess *getProc() const;
    bool isLive() const;
	ProcControlAPI::Thread::ptr pcThr() const { return pcThr_; }
    // Thread info
    dynthread_t getTid() const;
    func_instance *getStartFunc();
    Address getStackAddr();

    void clearStackwalk();

protected:
    PCThread(PCProcess *parent, int ind,
            ProcControlAPI::Thread::ptr thr);

    void markExited();
    void setTid(dynthread_t tid);
    bool continueThread();
    bool isRunning();
    bool postIRPC(inferiorRPCinProgress *newRPC);
    void findStartFunc();
    void findStackTop();
    void findSingleThreadInfo();

    PCProcess *proc_;
    ProcControlAPI::Thread::ptr pcThr_;

    int index_;
    Address stackAddr_;
    Address startFuncAddr_;
    func_instance *startFunc_;

    // When we run an inferior RPC we cache the stackwalk of the
    // process and return that if anyone asks for a stack walk
    int_stackwalk cached_stackwalk_;
	Dyninst::LWP savedLWP_;
    dynthread_t savedTid_;
    dynthread_t manuallySetTid_; // retrieved from the mutatee
};

#endif
