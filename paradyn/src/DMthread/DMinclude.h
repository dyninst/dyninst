#ifndef dminclude_H
#define dminclude_H

// this should be the only DM header file included by other modules

#include "util/h/String.h"
#include "util/h/Vector.h"
#include "util/h/sys.h"

typedef unsigned metricHandle;
typedef unsigned resourceListHandle;
typedef unsigned resourceHandle;
typedef unsigned perfStreamHandle;
typedef unsigned metricInstanceHandle;
typedef unsigned phaseHandle;

typedef enum {GlobalPhase, CurrentPhase} phaseType;
typedef enum {UnNormalized, Normalized, Sampled} dm_MetUnitsType;

struct metfocusType{
    vector<resourceHandle> res;
    metricHandle  met;
};
typedef struct metfocusType metric_focus_pair;

struct miInfoType{
    metricInstanceHandle mi_id;
    metricHandle m_id;
    resourceListHandle r_id;
    string metric_name;
    string metric_units;
    string focus_name;
    dm_MetUnitsType units_type;
};
typedef struct miInfoType metricInstInfo;

struct metNameIdType {
    metricHandle id;
    string       name;
};
typedef struct metNameIdType met_name_id;

struct rlNameIdType {
    resourceListHandle id;
    string	res_name;	
};
typedef struct rlNameIdType rlNameId;

#endif
