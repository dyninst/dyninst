#ifndef dminclude_H
#define dminclude_H

// this should be the only DM header file included by other modules

#include "util/h/String.h"
#include "util/h/Vector.h"
#include "util/h/sys.h"
// this is an upper limit for data buffering between DM and client threads
// and between VISIthread and visis
#define DM_DATABUF_LIMIT	50	

typedef unsigned metricHandle;
typedef unsigned resourceListHandle;
typedef unsigned resourceHandle;
typedef unsigned perfStreamHandle;
typedef unsigned metricInstanceHandle;
typedef unsigned phaseHandle;

typedef enum {GlobalPhase, CurrentPhase} phaseType;
typedef enum {UnNormalized, Normalized, Sampled} dm_MetUnitsType;

struct metfocusType {
    vector<resourceHandle> res;
    metricHandle  met;

    metfocusType(metricHandle iMet,
		 const vector<resourceHandle> &iRes) :
		   res(iRes), met(iMet) {}
    metfocusType() {}
};
typedef struct metfocusType metric_focus_pair;

struct metRLType {
    metricHandle met;
    resourceHandle res;

    metRLType(metricHandle mh,resourceHandle rh): met(mh), res(rh) {} 
    metRLType(){}
};
typedef struct metRLType metricRLType;

struct miInfoType{
    bool successfully_enabled;  // true if this metric/focus pair was enabled
    metricInstanceHandle mi_id;
    metricHandle m_id;
    resourceListHandle r_id;
    string metric_name;
    string metric_units;
    string focus_name;
    dm_MetUnitsType units_type;
    miInfoType(){
        successfully_enabled = 0; mi_id = 0; m_id=0; r_id=0; 
	units_type=Normalized;
    }
};
typedef struct miInfoType metricInstInfo;

struct metNameIdType {
    metricHandle id;
    string       name;
};
typedef struct metNameIdType met_name_id;

struct rlNameIdType {
    resourceListHandle id;
    const char *res_name;	
};
typedef struct rlNameIdType rlNameId;

#endif
