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

// $Id: registerSpace.h,v 1.1 2006/10/10 22:04:27 bernat Exp $

#ifndef REGISTER_SPACE_H
#define REGISTER_SPACE_H

#include <stdio.h>
#include "common/h/Vector.h"
#include "common/h/Dictionary.h"
#include "common/h/String.h"
#include "common/h/Types.h"
#if defined(ia64_unknown_linux2_4)
#include "inst-ia64.h"
#endif

class codeGen;
class instPoint;

class registerSlot {
 public:
    Register number;    // what register is it
    int refCount;      	// == 0 if free
    bool needsSaving;	// been used since last restore
    bool mustRestore;   // need to restore it before we are done.		
    bool startsLive;	// starts life as a live register.
    bool beenClobbered; // registers clobbered
};


class registerSpace {
 public:
	registerSpace(const unsigned int dCount, Register *deads,
                 const unsigned int lCount, Register *lives,
                 bool multithreaded = false);
   ~registerSpace();
	Register allocateRegister(codeGen &gen, bool noCost);
    bool allocateSpecificRegister(codeGen &gen, Register r, bool noCost);
	// Free the specified register (decrement its refCount)
	void freeRegister(Register k);
	// Free the register even if its refCount is greater that 1
	void forceFreeRegister(Register k);
	void resetSpace();
	void resetClobbers();
	void setAllLive();
	void saveClobberInfo(const instPoint * location);
	void resetLiveDeadInfo(const int* liveRegs,
			       const int *, const int *, bool);


	// Check to see if the register is free
	bool isFreeRegister(Register k);
	//bool getDisregardLiveness() {return disregardLiveness;}
	bool getDisregardLiveness();
	void setDisregardLiveness(bool dl) {disregardLiveness = dl;}
	
	// Inits the values for the clobbered variables for the floating point registers
	void initFloatingPointRegisters(const unsigned int count, Register *fp);

	
	//Makes register unClobbered
	void unClobberRegister(Register reg);
	void unClobberFPRegister(Register reg);

	// Checks to see if register has been clobbered and clobbers it 
	// if it hasn't been clobbered yet, returns true if we clobber it
	// false if it has already been clobbered
	bool clobberRegister(Register reg);
	bool clobberFPRegister(Register reg);

	// Checks to see if given register has been clobbered, true if it has
	bool beenSaved(Register reg);
	bool beenSavedFP(Register reg);

	// Checks to see if register starts live
	bool isRegStartsLive(Register reg);
	int fillDeadRegs(Register * deadRegs, int num);
	
	// Manually set the reference count of the specified register
	// we need to do so when reusing an already-allocated register
	void fixRefCount(Register k, int iRefCount);
	
	// Bump up the reference count. Occasionally, we underestimate it
	// and call this routine to correct this.
	void incRefCount(Register k);

	u_int getRegisterCount() { return numRegisters; }
	u_int getFPRegisterCount() { return numFPRegisters; }

	registerSlot *getRegSlot(Register k) { return (&registers[k]); }
	registerSlot *getFPRegSlot(Register k) { return (&fpRegisters[k]); }

	int getSPFlag() {return spFlag;}
	
	void copyInfo(registerSpace *rs) const;


	bool readOnlyRegister(Register k);
	// Make sure that no registers remain allocated, except "to_exclude"
	// Used for assertion checking.
	void checkLeaks(Register to_exclude);
   bool for_multithreaded() { return is_multithreaded; }

   registerSpace &operator=(const registerSpace &src);
 private:
	u_int numRegisters;
	u_int numFPRegisters;
	Register highWaterRegister;
	registerSlot *registers;
	registerSlot *fpRegisters;
	int spFlag;
	bool disregardLiveness; // for RPC base tramps
	bool is_multithreaded;
 public:
	bool hasXMM;  // for Intel architectures, XMM registers
 

#if defined(ia64_unknown_linux2_4)

public:
	int originalLocals;
	int originalOutputs;
	int originalRotates;
	int sizeOfStack;

	// storageMap[] needs to be of type 'int' as opposed to
	// 'Register' becuase negative values may be used.
	int storageMap[ BP_R_MAX ];
#endif

};

#endif
