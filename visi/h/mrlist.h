/* $Log: mrlist.h,v $
/* Revision 1.1  1994/03/14 20:27:30  newhall
/* changed visi subdirectory structure
/*  */ 
#ifndef _mrlist_h
#define _mrlist_h
#include "error.h"
#include "visi.h"


typedef struct{
  int size;
  char *list;
}visi_mrListType;

 
class visi_MRList{
  private:
    int numElements;
    visi_mrListType *list;
    int wildCard;
    int listSize; //removeing elements doesn't realloc space, so this
		  //value can be greater than numElements
  public:
    visi_MRList(){listSize = numElements = 0; list = NULL;}
    visi_MRList(int size,int wCard,char *members);
    visi_MRList(int size,class metricType *metrics);
    visi_MRList(int size,class resourceType *resources);
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
