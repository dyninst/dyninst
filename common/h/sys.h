#ifndef sys_H
#define sys_H

typedef double timeStamp;
typedef float sampleValue;

typedef struct {
   timeStamp start;
   timeStamp end;
   sampleValue value;
} Interval;

#endif
