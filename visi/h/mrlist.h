#ifndef _mrlist_h
#define _mrlist_h
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

/* $Log: mrlist.h,v $
/* Revision 1.2  1994/05/11 17:11:06  newhall
/* changed data values from double to float
/*
 * Revision 1.1  1994/03/14  20:27:30  newhall
 * changed visi subdirectory structure
 *  */ 
 /////////////////////////////////////////////
 // Class visi_MRList
 //   this class allows visualization writer
 //   to access and manipulate the list of
 //   metrics and resources associated with
 //   the data grid
 //   changes to these lists do not effect
 //   the contents of the data grid
 /////////////////////////////////////////////

#include "visiTypes.h"


typedef struct{
  int size;
  char *list;
}visi_mrListType;

 
class visi_MRList{
  private:
    int numElements;  // current number of list elements
    visi_mrListType *list;  // list elements
    int wildCard;     // 1-if wild card character is appended to list
    int listSize; //removeing elements doesn't realloc space, so this
		  //value can be greater than numElements
  public:
    visi_MRList(){listSize = numElements = 0; list = NULL;}
    visi_MRList(int size,int wCard,char *members);
    visi_MRList(int size,visi_metricType *metrics);
    visi_MRList(int size,visi_resourceType *resources);
    ~visi_MRList();
    int   AddElements(int num,char *elements);
    void  Print();
    void  AddWildCard(){wildCard = 1;}
    int   NumElements(){return(numElements);}
    void  RemoveWildCard(){wildCard = 0;}
    int   RemoveElement(int elmNum);
    int   CreateMRList(char **elements);
    int   ListSize(){return(listSize);}
};
#endif
