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

// $Id: baseTramp.h,v 1.24 2008/09/04 21:06:08 bill Exp $

// baseTramp class definition

#ifndef BASE_TRAMP_H
#define BASE_TRAMP_H

#include "dyntypes.h"
#include "inst.h" // callWhen
#include "dyninstAPI/src/codeRange.h"
//#include "arch.h"
#include "ast.h"

#include <list>

class baseTramp;
class AddressSpace;


class baseTramp { 
    baseTramp();

 public:
    static baseTramp *create(instPoint *p);
    static baseTramp *createForIRPC(AddressSpace *as);
    static baseTramp *fork(baseTramp *parBT, AddressSpace *child);

    func_instance *func() const;
    instPoint *point() const { return point_; }
    instPoint *instP() const { return point(); }
    AddressSpace *proc() const;
    
    void initializeFlags();

    bool generateCode(codeGen &gen,
                      Dyninst::Address baseInMutatee);

    bool generateCodeInlined(codeGen &gen,
                             Dyninst::Address baseInMutatee);

    bool checkForFuncCalls();

    ~baseTramp();

    int numDefinedRegs();

    void setIRPCAST(AstNodePtr ast) { ast_ = ast; }

  private:
    instPoint *point_;
    AddressSpace *as_;

    AstNodePtr ast_;
    
    bool shouldRegenBaseTramp(registerSpace *rs); 

 private:
    // We keep two sets of flags. The first controls which features
    // we enable in the base tramp, including:
    // multithread support, which classes of registers are saved, etc.

    // The second records (during code gen) what has been done so we
    // can undo it later. 
    
    cfjRet_t funcJumpState_;
    bool needsStackFrame_;
    bool threaded_;
    bool optimizationInfo_;

  public:    
    // Tracking the results of code generation.
    bool savedFPRs;
    bool createdFrame;
    bool savedOrigAddr;
    bool createdLocalSpace;
    bool alignedStack;
    bool savedFlags;
    bool optimizedSavedRegs;
    bool suppressGuards;
    bool suppressThreads;
    bool spilledRegisters;
    int  stackHeight;
    bool skippedRedZone;
    bool wasFullFPRSave;
    
    
    bool validOptimizationInfo() { return optimizationInfo_; }

 public:
    // Code generation methods
    bool generateSaves(codeGen &gen, registerSpace *);
    bool generateRestores(codeGen &gen, registerSpace *);
    
    // Generated state methods
    bitArray definedRegs;

    int funcJumpSlotSize();
    bool guarded() const;
    bool threaded() const;
    bool doOptimizations();
    bool makesCall();
    bool needsFrame() { return needsStackFrame_; }
    bool madeFrame() { return createdFrame; }

    bool saveFPRs();
    void setNeedsFrame(bool);
};

#define X86_REGS_SAVE_LIMIT 3

#endif



