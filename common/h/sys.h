#ifndef sys_H
#define sys_H

/* These must represent the same value */
#define BASESAMPLEINTERVAL 200000  /* 200 msecs */
#define BASEBUCKETWIDTH    0.2   /* .2 seconds  */

typedef double timeStamp;
typedef float sampleValue;

typedef struct {
   timeStamp start;
   timeStamp end;
   sampleValue value;
} Interval;

#endif
