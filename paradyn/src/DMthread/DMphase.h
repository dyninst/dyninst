/*
 * Copyright (c) 1993, 1994 Barton P. Miller, Jeff Hollingsworth,
 *     Bruce Irvin, Jon Cargille, Krishna Kunchithapadam, Karen
 *     Karavanic, Tia Newhall, Mark Callaghan.  All rights reserved.
 *
 * This software is furnished under the condition that it may not be
 * provided or otherwise made available to, or used by, any other
 * person, except as provided for by the terms of applicable license
 * agreements.  No title to or ownership of the software is hereby
 * transferred.  The name of the principals may not be used in any
 * advertising or publicity related to this software without specific,
 * written prior authorization.  Any use of this software must include
 * the above copyright notice.
 *
 */

#ifndef DMphase_H
#define DMphase_H

#include "util/h/String.h"
#include "util/h/sys.h"
#include "util/h/Vector.h"
#include "DMinclude.h"

class phaseInfo {
private:
	timeStamp    startTime;
	timeStamp    endTime;
	float	     bucketWidth;
	phaseHandle  handle;
	string       name;
	static vector<phaseInfo *> dm_phases;
	phaseInfo(timeStamp s,timeStamp e,timeStamp b,const string &n);
public:
	~phaseInfo(){ }
	timeStamp GetStartTime(){ return(startTime); }
	phaseHandle GetPhaseHandle(){ return(handle);}
	timeStamp GetEndTime(){ return(endTime); }
	float GetBucketWidth(){ return(bucketWidth);}
	void SetEndTime(timeStamp time){ endTime = time;}
	void ChangeBucketWidth(float newWidth){ bucketWidth = newWidth; }
        const char *PhaseName(){return(name.string_of());}
	static int NumPhases(){return(dm_phases.size());}
	static void startPhase(timeStamp start_Time, const string &name);
};

#endif
