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

#ifndef PCTHREAD_H
#define PCTHREAD_H

#include "frame.h"

#include "proccontrol/h/Process.h"

/*
 * pcThread.h
 *
 * A class that encapsulates a ProcControlAPI thread. This class is meant
 * to be a replacement for the old dyn_thread class.
 */

class PCProcess;
typedef long dynthread_t;

class PCThread {
public:
    static PCThread *createPCThread(PCProcess *parent, int index,
                                    int lwpId, dynthread_t async_tid);

    // Stackwalking interface
    bool walkStack(pdvector<Frame> &stackWalk);
    bool getRegisters(struct dyn_saved_regs *regs, bool includeFP = false);
    bool changePC(Address, struct dyn_saved_regs *) { return false; }

    // Field accessors
    dynthread_t getTid() const;
    int getIndex() const;
    int getLWPId() const;
    int_function *getStartFunc() const;
    Address getIndirectStartAddr() const;
    Address getStackAddr() const;
    int getFD() const;
    PCProcess *getProc() const;

    // Field mutators
    void updateStartFunc(int_function *ifunc);
    void updateStackAddr(Address stackStart);

    int_function *mapInitialFunc(int_function *ifunc);

    // Architecture-specific
    Frame getActiveFrame();

protected:
    PCProcess *proc_;
    ProcControlAPI::Thread::ptr pcThr_;
};

#endif
