#ifndef DMphase_H
#define DMphase_H

#include "util/h/String.h"
#include "util/h/sys.h"
#include "util/h/Vector.h"

class performanceStream; 

typedef int phaseHandle;

class phaseInfo {

private:
	timeStamp    startTime;
	timeStamp    endTime;
	float	     bucketWidth;
	phaseHandle  handle;
	string       *name;
	static vector<phaseInfo *> dm_phases;
	static int  numPhases;
public:

	phaseInfo(timeStamp s,timeStamp e,timeStamp b,phaseHandle h,string *n){
    		startTime = s;
    		endTime = e;
    		bucketWidth = b;
    		handle = h;
    		name = new string( n->string_of() );
		numPhases++;
        }
	~phaseInfo(){
    		delete name;
    		numPhases--;
        }
	timeStamp GetStartTime(){ return(startTime); }
	phaseHandle GetPhaseHandle(){ return(handle);}
	timeStamp GetEndTime(){ return(endTime); }
	float GetBucketWidth(){ return(bucketWidth);}
	void SetEndTime(timeStamp time){ endTime = time;}
	void ChangeBucketWidth(float newWidth){ bucketWidth = newWidth; }
        const char *PhaseName(){return(name->string_of());}
	static int NumPhases(){return(numPhases);}
	static void AddNewPhase(phaseInfo *p){ dm_phases+=p; }
};

#endif
