/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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

#ifndef DMphase_H
#define DMphase_H

#include "common/h/String.h"
#include "common/h/Vector.h"
#include "common/h/Time.h"
#include "visi.xdr.h"
#include "DMinclude.h"

class phaseInfo {
private:
        static const relTimeStamp histCurTime;  // a sentinal value
	relTimeStamp    startTime;
	relTimeStamp    endTime;
	timeLength   bucketWidth;
	phaseHandle  handle;
	string       name;
	static vector<phaseInfo *> dm_phases;
	phaseInfo(relTimeStamp s, relTimeStamp e,timeLength b,const string &n);
public:
	~phaseInfo();
	relTimeStamp GetStartTime(){ return(startTime); }
	phaseHandle GetPhaseHandle(){ return(handle);}
	relTimeStamp GetEndTime(){ return(endTime); }
	timeLength GetBucketWidth(){ return(bucketWidth);}
	void SetEndTime(relTimeStamp time){ endTime = time;}
	void ChangeBucketWidth(timeLength newWidth){ bucketWidth = newWidth; }
        const char *PhaseName(){return(name.string_of());}
	static vector<T_visi::phase_info> *GetAllPhaseInfo();
	static int NumPhases(){return(dm_phases.size());}
	// If no startTime is passed, than use as a start time for the phase
	// the current time of the histogram.
	static void startPhase(const string &name, bool with_new_pc, 
			       bool with_visis, 
			       const relTimeStamp startTime = histCurTime);
	static void setLastEndTime(relTimeStamp);
	// returns start time of current phase 
	static relTimeStamp GetLastPhaseStart();  
	static phaseHandle CurrentPhaseHandle();
	static timeLength GetLastBucketWidth();
	static void setCurrentBucketWidth(timeLength new_width);
};

#endif
