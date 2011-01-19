/*
 * Copyright (c) 1996-2010 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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

#include "pcProcess.h"
#include "pcThread.h"
#include "debug.h"

using namespace Dyninst::ProcControlAPI;

PCThread::PCThread(PCProcess *parent, int ind,
        Thread::ptr thr) :
    proc_(parent),
    pcThr_(thr),
    index_(ind),
    stackAddr_(0),
    startFunc_(NULL),
    savedLWP_(thr->getLWP()),
    savedTid_(thr->getLWP()) // TODO use actual TID provided by ProcControlAPI
{
}

PCThread *PCThread::createPCThread(PCProcess *parent, Thread::ptr thr)
{
    PCThread *ret = new PCThread(parent, parent->incrementThreadIndex(), thr);
    assert(ret);

    return ret;
}

dynthread_t PCThread::getTid() const {
    if( pcThr_ == Thread::ptr() ) return savedTid_;
    return (dynthread_t)pcThr_->getLWP();
}

int PCThread::getIndex() const {
    return index_;
}

int PCThread::getLWP() const {
    if( pcThr_ == Thread::ptr() ) return savedLWP_;
    return pcThr_->getLWP();
}

int_function *PCThread::getStartFunc() const {
    return startFunc_;
}

Address PCThread::getStackAddr() const {
    return stackAddr_;
}

void PCThread::updateStartFunc(int_function *ifunc) {
    startFunc_ = ifunc;
}

void PCThread::updateStackAddr(Address stackStart) {
    stackAddr_ = stackStart;
}

PCProcess *PCThread::getProc() const {
    return proc_;
}

bool PCThread::walkStack(pdvector<Frame> &stackWalk) {
    if( pcThr_ == Thread::ptr() ) return false;

    bool continueWhenDone = false;
    if( pcThr_->isRunning() ) {
        continueWhenDone = true;
        if( !pcThr_->stopThread() ) {
            stackwalk_printf("%s[%d]: Failed to stop thread %ld to perform stackwalk\n",
                    __FILE__, __LINE__, getTid());
            return false;
        }
    }

    if (cached_stackwalk_.isValid()) {
        stackWalk = cached_stackwalk_.getStackwalk();
        for (unsigned i=0; i<stackWalk.size(); i++) {
            stackWalk[i].setThread(this);
        }
        return true;
    }

    stackwalk_printf("%s[%d]: beginning stack walk on thread %ld\n",
                     FILE__, __LINE__, getTid());

    Frame active = getActiveFrame();
    active.setThread(this);
    bool retval = proc_->walkStackFromFrame(active, stackWalk);

    // if the stackwalk was successful, cache it
    if (retval) {
        cached_stackwalk_.setStackwalk(stackWalk);
    }

    stackwalk_printf("%s[%d]: ending stack walk on thread %ld\n",
                     FILE__, __LINE__, getTid());

    if (continueWhenDone) {
        proc_->invalidateActiveMultis();
        if( !pcThr_->continueThread() ) {
            stackwalk_printf("%s[%d]: failed to continue thread %ld after stackwalk\n",
                    FILE__, __LINE__, getTid());
        }
    }

    return retval;
}

void PCThread::clearStackwalk() {
    cached_stackwalk_.clear();
}

bool PCThread::getRegisters(RegisterPool &regs, bool /* includeFP */) {
    if( pcThr_ == Thread::ptr() ) return false;

    // XXX ProcControlAPI doesn't yet get floating point registers
    return pcThr_->getAllRegisters(regs);
}

bool PCThread::changePC(Address newPC) {
    if( pcThr_ == Thread::ptr() ) return false;

    return pcThr_->setRegister(MachRegister::getPC(proc_->getArch()), newPC);
}

Frame PCThread::getActiveFrame() {
    if( pcThr_ == Thread::ptr() ) return Frame();

    Address pc = 0, fp = 0, sp = 0;

    assert( pcThr_->isStopped() && "Thread not stopped before trying to get the active frame" );

    if( !pcThr_->getRegister(MachRegister::getPC(proc_->getArch()), pc) ) {
        stackwalk_printf("%s[%d]: Failed to obtain the current PC\n", __FILE__, __LINE__);
        return Frame();
    }

    if( !pcThr_->getRegister(MachRegister::getFramePointer(proc_->getArch()), fp) ) {
        stackwalk_printf("%s[%d]: Failed to obtain the current frame pointer\n", __FILE__, __LINE__);
        return Frame();
    }

    if( !pcThr_->getRegister(MachRegister::getStackPointer(proc_->getArch()), sp) ) {
        stackwalk_printf("%s[%d]: Failed to obtain the current stack pointer\n", __FILE__, __LINE__);
        return Frame();
    }

    return Frame(pc, fp, sp, proc_->getPid(), proc_, this, true);
}

bool PCThread::isLive() const {
    if( pcThr_ == Thread::ptr() ) return false;
    return pcThr_->isLive();
}

void PCThread::markExited() { 
    pcThr_ = Thread::ptr();
}

bool PCThread::isRunning() {
    return pcThr_->isRunning();
}

bool PCThread::continueThread() {
    clearStackwalk();
    proc_->invalidateActiveMultis();

    proccontrol_printf("%s[%d]: continuing thread %d/%d\n", 
            FILE__, __LINE__, proc_->getPid(), getLWP());

    if( !pcThr_->continueThread() ) {
        proccontrol_printf("%s[%d]: failed to continue thread %d/%d\n",
                proc_->getPid(), getLWP());
        return false;
    }

    return true;
}

bool PCThread::postIRPC(inferiorRPCinProgress *newRPC) {
    return pcThr_->postIRPC(newRPC->rpc);
}
