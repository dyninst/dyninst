/* $Log: mrlist.C,v $
/* Revision 1.2  1994/03/14 20:28:49  newhall
/* changed visi subdirectory structure
/*  */ 
#include "visi/h/mrlist.h" 
/////////////////// method functions for class visi_MRList

visi_MRList::visi_MRList(int size,int wCard,char *members){

int i,j,k;
int wordsize;
int wch;

  if((size > 0) && (members != NULL)){
    numElements = size; 
    if((list=(visi_mrListType *)malloc(sizeof(visi_mrListType)*numElements)) 
	== NULL){
      perror("Error in visi_MRList constructor");
    }
    i=j=wch=0;
    while((members[i] != '\0') && (wch < numElements)){
      while((members[i] != ',') && (members[i] != '\0')){
         i++;
      }
      wordsize = i-j+1;
      if((list[wch].list = (char *)malloc(sizeof(char)*wordsize))==NULL){
	perror("Error in visi_MRList constructor");
      }
      for(k=0;k<wordsize;k++){
	list[wch].list[k] = '\0';
      }
      if((strncpy(list[wch].list,&members[j],wordsize-1)) == NULL){
	perror("Error in visi_MRList constructor");
      }
      list[wch].size = wordsize -1;
      j = i+1;
      if(members[i] != '\0') i++;
      wch++;
    }
  }
  listSize = numElements;
  if(wCard)
    wildCard = 1;
  else
    wildCard = 0;
}


visi_MRList::visi_MRList(int size,class metricType *metrics){

int i,k,nameLength;

  if((size > 0) && (metrics != NULL)){
    numElements = size;
    if((list=(visi_mrListType *)malloc(sizeof(visi_mrListType)*numElements)) 
	== NULL){
      visi_ErrorHandler(ERROR_MALLOC,"Error in visi_MRList constructor");
    }
    for(i=0;i<numElements;i++){
      nameLength = strlen(metrics[i].name);
      if((list[i].list = (char *)malloc(sizeof(char)*(nameLength+1)))==NULL){
        visi_ErrorHandler(ERROR_MALLOC,"Error in visi_MRList constructor");
      }
      for(k=0;k<nameLength+1;k++){
	list[i].list[k] = '\0';
      }
      if((strncpy(list[i].list,metrics[i].name,nameLength)) == NULL){
        visi_ErrorHandler(ERROR_STRNCPY,"Error in visi_MRList constructor");
      }
      list[i].list[nameLength] = '\0';
    }
    listSize = numElements;
  }
}


visi_MRList::visi_MRList(int size,class resourceType *resources){

int i,k,nameLength;

  if((size > 0) && (resources != NULL)){
    numElements = size;
    if((list=(visi_mrListType *)malloc(sizeof(visi_mrListType)*numElements)) 
	== NULL){
      visi_ErrorHandler(ERROR_MALLOC,"Error in visi_MRList constructor");
    }
    for(i=0;i<numElements;i++){
      nameLength = strlen(resources[i].name);
      if((list[i].list = (char *)malloc(sizeof(char)*(nameLength+1)))==NULL){
        visi_ErrorHandler(ERROR_MALLOC,"Error in visi_MRList constructor");
      }
      for(k=0;k<nameLength+1;k++){
	list[i].list[k] = '\0';
      }
      if((strncpy(list[i].list,resources[i].name,nameLength)) == NULL){
        visi_ErrorHandler(ERROR_STRNCPY,"Error in visi_MRList constructor");
      }
      list[i].list[nameLength] = '\0';
    }
    listSize = numElements;
  }

}


visi_MRList::~visi_MRList(){
  int i;

  for(i = 0; i < listSize; i++)
    if(list[i].list != NULL)
      free(list[i].list);
  free(list);

}

int visi_MRList::AddElements(int num,char *elements){

int i,j,k;
int start,end;
int size;
int wordsize;
int wch;

if(!numElements){
  if((list=(visi_mrListType *)malloc(sizeof(visi_mrListType)*num))==NULL){
    visi_ErrorHandler(ERROR_MALLOC,"visi_MRList::AddElements");
    perror("Error in AddElements");
    return(ERROR_MALLOC);
  }
  start = 0;
  end   = num;
}
else{
 size = numElements + num;
 if((list=(visi_mrListType *)realloc(list,sizeof(visi_mrListType)*size))==NULL){
    perror("Error in AddElements");
    visi_ErrorHandler(ERROR_MALLOC,"visi_MRList::AddElements");
    return(ERROR_MALLOC);
 }
 start = numElements;
 end   = numElements+num;
}

if((num > 0) &&(elements != NULL)){
  i=j=wch=0;
  while((elements[i] != '\0') && (wch < num)){
    while((elements[i] != ',') && (elements[i] != '\0')){
       i++;
    }
    wordsize = i-j+1;
    if((list[start].list = (char *)malloc(sizeof(char)*wordsize)) == NULL){
      perror("Error in AddElements");
      visi_ErrorHandler(ERROR_MALLOC,"visi_MRList::AddElements");
      return(ERROR_MALLOC);
    }
    for(k=0;k<wordsize;k++){
      list[start].list[k] = '\0';
    }
    if((strncpy(list[start].list,&elements[j],wordsize-1)) == NULL){
      perror("Error in AddElements");
      visi_ErrorHandler(ERROR_STRNCPY,"visi_MRList::AddElements");
      return(ERROR_STRNCPY);
    }
    list[start].size = wordsize -1;
    j = i+1;
    if(elements[i] != '\0') i++;
    wch++;
    start++;
  }
  numElements += num;
}
listSize = numElements;
return(OK);
}

void visi_MRList::Print(){

int i;

  fprintf(stderr,"number of elements in list: %d\n",numElements);
  for(i=0; i < numElements; i++){
    fprintf(stderr,"list element %d: %s\n",i,list[i].list); 
  }
  if(wildCard)
    fprintf(stderr,"list element %d: wildCard\n",i);
}


int visi_MRList::RemoveElement(int elmNum){

int i;

  if((elmNum < 0) || (elmNum >= numElements)){
     visi_ErrorHandler(ERROR_NOELM,"visi_MRList::RemoveElement"); 
     return(ERROR_NOELM);
  }

  free(list[elmNum].list);
  for(i = elmNum; i < (numElements-1); i++)
    list[i] = list[i+1];
  numElements--;
  return(OK);
}

int visi_MRList::CreateMRList(char **elements){

int  totalSize = 0;
int  i,j;
  
  for(i=0; i < numElements; i++){
    totalSize += list[i].size;
  }
  totalSize += numElements; //add extra space for commas
  if(wildCard)
    totalSize += 2;
  if(((*elements) = (char *)malloc(sizeof(char)*totalSize)) == NULL){
    perror("error in CreateMRList\n");
    visi_ErrorHandler(ERROR_MALLOC,"visi_MRList::CreateMRList"); 
    return(ERROR_MALLOC);
  }
  for(i=0;i<totalSize;i++){
    (*elements)[i] = '\0';
  }
  j=0;
  for(i=0; i < numElements; i++){
    if(j < totalSize)
      if((strncpy(&((*elements)[j]),list[i].list,list[i].size))==NULL){
        perror("error in CreateMRList\n");
        visi_ErrorHandler(ERROR_STRNCPY,"visi_MRList::CreateMRList"); 
        return(ERROR_STRNCPY);
      }
    j += list[i].size;
    if(i < (numElements-1)){
      if((strncpy(&((*elements)[j]),",",1))==NULL){
        perror("error in CreateMRList\n");
        visi_ErrorHandler(ERROR_STRNCPY,"visi_MRList::CreateMRList"); 
        return(ERROR_STRNCPY);
      }
      j++;
    }
  }
  if((wildCard) && (j<totalSize)){
    if((strncpy(&((*elements)[j]),",*",2))==NULL){
        perror("error in CreateMRList\n");
        visi_ErrorHandler(ERROR_STRNCPY,"visi_MRList::CreateMRList"); 
        return(ERROR_STRNCPY);
    }
    j +=2;
  }
  if(j<totalSize){
   if((strncpy(&((*elements)[j]),"\0",1))==NULL){
        perror("error in CreateMRList\n");
        visi_ErrorHandler(ERROR_STRNCPY,"visi_MRList::CreateMRList"); 
        return(ERROR_STRNCPY);
   }
  }

  return(OK);
}
