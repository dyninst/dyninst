#if !defined(_dv_bufferpool_H)
#define _dv_bufferpool_H

// This is a buffer pool for data values
// it is used between the DM and its client threads
#include "paradyn/src/DMthread/DMinclude.h"
#include "paradyn/src/DMthread/BufferPool.h"

struct dataValueT{
    metricInstanceHandle mi;
    int bucketNum;
    sampleValue value;
    phaseType type;
}; 
typedef struct dataValueT dataValueType; 

extern BufferPool<dataValueType> datavalues_bufferpool; 

#endif
