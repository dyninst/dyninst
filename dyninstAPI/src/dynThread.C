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

#include "dynProcess.h"
#include "dynThread.h"
#include "debug.h"
#include "function.h"
#include "mapped_module.h"
#include "mapped_object.h"

using namespace Dyninst::ProcControlAPI;

PCThread::PCThread(PCProcess *parent, int ind,
        Thread::ptr thr) :
    proc_(parent),
    pcThr_(thr),
    index_(ind),
    stackAddr_(thr->getStackBase()),
    startFuncAddr_(thr->getStartFunction()),
    startFunc_(NULL),
    savedLWP_(thr->getLWP()),
    savedTid_(thr->getTID()),
    manuallySetTid_(0)
{
    findStartFunc();
    findStackTop();
}

PCThread *PCThread::createPCThread(PCProcess *parent, Thread::ptr thr)
{
    PCThread *ret = new PCThread(parent, parent->incrementThreadIndex(), thr);
    assert(ret);

    return ret;
}

dynthread_t PCThread::getTid() const {
	if( pcThr_ == Thread::ptr() ) {
		return savedTid_;
	}
    THR_ID pcTid = pcThr_->getTID();
    if( pcTid == NULL_THR_ID || pcTid == 0 ) {
        if( manuallySetTid_ == DYNINST_SINGLETHREADED ) return 0;
        return manuallySetTid_;
    }
    return pcTid;
}

void PCThread::setTid(dynthread_t tid) {
    manuallySetTid_ = tid;
}

int PCThread::getIndex() const {
    return index_;
}

Dyninst::LWP PCThread::getLWP() const {
    if( pcThr_ == Thread::ptr() ) return savedLWP_;
    return pcThr_->getLWP();
}

void PCThread::findSingleThreadInfo() {
    assert( startFuncAddr_ == 0 || stackAddr_ == 0 );

    std::vector<Frame> stackWalk;
    if( !walkStack(stackWalk) ) return;

    proccontrol_printf("%s[%d]: searching stackwalk for initial func and stack top\n",
            FILE__, __LINE__);

    int pos = stackWalk.size() - 1;
    while( (stackAddr_ == 0 || startFuncAddr_ == 0) && pos >= 0 ) {
        if( dyn_debug_proccontrol ) {
            cerr << stackWalk[pos] << endl;
        }

        if( stackAddr_ == 0 ) stackAddr_ = (Address) stackWalk[pos].getSP();
        if( startFuncAddr_ == 0 ) {
            func_instance *tmpFunc = stackWalk[pos].getFunc();
            if( tmpFunc != NULL ) {
                mapped_module *mod = tmpFunc->mod();
                if( mod && !mod->obj()->isSystemLib(mod->obj()->fullName()) ) {
                    startFuncAddr_ = tmpFunc->ifunc()->getOffset();
                }
            }
        }
        pos--;
    }
}

void PCThread::findStartFunc() {
    // If this thread has exited, it is too late to determine this info
    if( pcThr_ == Thread::ptr() ) return;
        
    startFuncAddr_ = pcThr_->getStartFunction();
    if( startFuncAddr_ == 0 ) findSingleThreadInfo();

    // If still haven't found the starting address, don't try to find the function
    if( startFuncAddr_ == 0 ) return;

    startFunc_ = proc_->findOneFuncByAddr(startFuncAddr_);
}

func_instance *PCThread::getStartFunc() {
    if( startFunc_ == NULL ) findStartFunc();
    return startFunc_;
}

void PCThread::findStackTop() {
    // If this thread has exited, it is too late to determine this info
    if( pcThr_ == Thread::ptr() ) return;

    stackAddr_ = pcThr_->getStackBase();
    if( stackAddr_ == 0 ) findSingleThreadInfo();
}

Address PCThread::getStackAddr() {
    if( stackAddr_ == 0 ) findStackTop();
    return stackAddr_;
}

PCProcess *PCThread::getProc() const {
    return proc_;
}

bool PCThread::walkStack(std::vector<Frame> &stackWalk) {
    if (cached_stackwalk_.isValid()) {
        stackWalk = cached_stackwalk_.getStackwalk();
        for (unsigned i=0; i<stackWalk.size(); i++) {
            stackWalk[i].setThread(this);
        }
        return true;
    }

    if (!proc_->walkStack(stackWalk, this))
    {
        return false;
    }

    cached_stackwalk_.setStackwalk(stackWalk);

    return true;
}

Frame PCThread::getActiveFrame() {
    Frame frame;

    if (!proc_->getActiveFrame(frame, this))
    {
      return Frame();
    }

    return frame;
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

bool PCThread::isLive() const {
    if( pcThr_ == Thread::ptr() ) return false;
    return pcThr_->isLive();
}

void PCThread::markExited() { 
    pcThr_ = Thread::ptr();
}

bool PCThread::isRunning() {
    if( pcThr_ == Thread::ptr() ) return false;
    return pcThr_->isRunning();
}

bool PCThread::continueThread() {
    if( pcThr_ == Thread::ptr() ) return false;

    clearStackwalk();

    proccontrol_printf("%s[%d]: continuing thread %d/%d\n", 
            FILE__, __LINE__, proc_->getPid(), getLWP());

    if( !pcThr_->continueThread() ) {
        proccontrol_printf("%s[%d]: failed to continue thread %d/%d\n",
                FILE__, __LINE__, proc_->getPid(), getLWP());
        return false;
    }

    return true;
}

bool PCThread::postIRPC(inferiorRPCinProgress *newRPC) {
    if( pcThr_ == Thread::ptr() ) return false;
    return pcThr_->postIRPC(newRPC->rpc);
}
