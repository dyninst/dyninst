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

/*
 * PCdataQ.C
 *
 * dataQ class: used to store and align data values
 *
 * $Log: PCdataQ.C,v $
 * Revision 1.1  1996/02/02 02:06:30  karavan
 * A baby Performance Consultant is born!
 *
 */

#include <stream.h>
#include "PCdataQ.h"

#define PCInitDataQSize 20
 
dataQ::dataQ () 
{
  qsize = PCInitDataQSize;
  qdata = new Interval [qsize];
  head = 0; // empty
  tail = 0;
}

dataQ::~dataQ ()
{
  delete qdata;
}

void 
dataQ::grow()
{
  assert (head == tail);

  int newsize = qsize * 2;
  Interval *newqdata = new Interval [newsize];
  // copy elements in order from head to end of array,
  // a total of (qsize - head elements) will be copied, 
  // starting at position 0
  for (int i = head, j = 0; i < qsize; i++, j++) { 
    newqdata[j] = qdata[i];
  }
  // append elements in order from start of array to head - 1,
  // a total of head elements will be copied, starting at 
  // position (qsize - head)
  for (int k = 0, l = qsize - head; k < head; k++, l++) {
    newqdata[l] = qdata[k];
  }
  tail = qsize;
  head = 0;
  qsize = newsize;
  delete qdata;
  qdata = newqdata;
}

void
dataQ::print()
{
  if (head == tail) {
    cout << "\tData Queue is empty" << endl;
  } else {
    int counter = head;
    while (counter != tail) {
      cout << "  QValue=" << qdata[counter].value << " start=" 
	<< qdata[counter].start << " end=" << qdata[counter].end << endl;
      if (++counter == qsize)
	counter = 0;
    }
  }
}

bool
dataQ::enqueue (timeStamp startTime, timeStamp endTime, sampleValue newval)
{
  qdata[tail].start = startTime;
  qdata[tail].end = endTime;
  qdata[tail].value = newval;
  if (tail == (qsize - 1))
    tail = 0;
  else
    tail += 1;
  if (tail == head) {
    // overflow
    this->grow();
    return true;
  }
  return false;
}

bool
dataQ::dequeue (timeStamp *startTime, timeStamp *endTime, sampleValue *newval)
{
  if (this->isEmpty())
    return false;
  
  *startTime = qdata[head].start;
  *endTime = qdata[head].end;
  *newval = qdata[head].value;
  if (head == (qsize - 1))
    head = 0;
  else
    head++;
  return true;
}

bool
dataQ::seeNextItem (timeStamp *startTime, timeStamp *endTime, 
		    sampleValue *newval)
{
  if (this->isEmpty())
    return false;
  
  *startTime = qdata[head].start;
  *endTime = qdata[head].end;
  *newval = qdata[head].value;
  return true;
}

bool
dataQ::isEmpty ()
{
  return (head == tail);
}
