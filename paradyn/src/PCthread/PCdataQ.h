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
 * PCdataQ.h
 *
 * dataQ class
 *
 * $Log: PCdataQ.h,v $
 * Revision 1.1  1996/02/02 02:07:23  karavan
 * A baby Performance Consultant is born!
 *
 */


#include "PCintern.h"

class dataQ {
public:
  dataQ ();
  ~dataQ();
  bool enqueue (timeStamp start, timeStamp end, sampleValue value);
  bool dequeue (timeStamp *start, timeStamp *end, sampleValue *value);
  bool seeNextItem (timeStamp *start, timeStamp *end, sampleValue *value);
  int getQsize () {return qsize;}
  void print();
  bool isEmpty();
private:
  void grow();
  Interval *qdata;
  int qsize;
  int head;
  int tail;
};
