/* $Log: client2.C,v $
/* Revision 1.3  1994/04/13 21:21:43  newhall
/* *** empty log message ***
/*
 * Revision 1.2  1994/03/26  04:37:03  newhall
 * change all floats to double
 * */
#include <stdio.h>
#include <stdlib.h>
#include "visi.CLNT.h"

#define TESTMETRIC	"blah,blah blah,metric 3,metric 4,last metric"
#define TESTRESOURCE	"resource1/blah1/foo1,resource2/foo2,resource3"

int numMets,numRes,numBuckets,lastBucketcollected = 0;
double bucketwidth;
visualizationUser *vup;

void Phase(){
char temp[32];
  fprintf(stdout,"\nPhase:\n");   
  sprintf(temp,"%s","psuedo_phase");
  vup->Phase(0.0,4.5,temp);
}

void AddMR(int nMets, int nRes){

metricType *metrics;
resourceType *resources;
metricType_Array mets;
resourceType_Array res;
char temp[32],temp2[32];
int i;



  fprintf(stdout,"\nAdd Metrics and Resorces:\n");   
  metrics = (metricType *)malloc(sizeof(metricType)*nMets);
  resources = (resourceType *)malloc(sizeof(resourceType)*nRes);
  for(i=0;i<nMets;i++){
     sprintf(temp,"%s%d","metric",i);
     metrics[i].name = strdup(temp);
     sprintf(temp2,"%s%d","units",i);
     metrics[i].units = strdup(temp2);
     metrics[i].Id = i;
     metrics[i].aggregate = (i % 2);
   }
   mets.count = nMets;
   mets.data = metrics;
   for(i=0;i<nRes;i++){
     resources[i].Id = i;
     sprintf(temp,"%s%d","resource",i);
     resources[i].name = strdup(temp);
   }
   res.count = nRes;
   res.data = resources;
   vup->AddMetricsResources(mets,res,bucketwidth,numBuckets);
   for(i=0;i<nMets;i++){
      free(metrics[i].name);
      free(metrics[i].units);
   }
   for(i=0;i<nRes;i++){
      free(resources[i].name);
   }
   free(metrics);
   free(resources);
}

void NewMR(int nMets, int nRes){

metricType *metrics;
resourceType *resources;
metricType_Array mets;
resourceType_Array res;
char temp[32],temp2[32];
int i;



  fprintf(stdout,"\nNew Metrics and Resorces not currently supported\n");   
  metrics = (metricType *)malloc(sizeof(metricType)*nMets);
  resources = (resourceType *)malloc(sizeof(resourceType)*nRes);
  for(i=0;i<nMets;i++){
     sprintf(temp,"%s%d","metric",i);
     metrics[i].name = strdup(temp);
     sprintf(temp2,"%s%d","units",i);
     metrics[i].units = strdup(temp2);
     metrics[i].Id = i;
     metrics[i].aggregate = (i % 2);
   }
   mets.count = nMets;
   mets.data = metrics;
   for(i=0;i<nRes;i++){
     resources[i].Id = i;
     sprintf(temp,"%s%d","resource",i);
     resources[i].name = strdup(temp);
   }
   res.count = nRes;
   res.data = resources;
   vup->NewMetricsResources(mets,res);
   for(i=0;i<nMets;i++){
      free(metrics[i].name);
      free(metrics[i].units);
   }
   for(i=0;i<nRes;i++){
      free(resources[i].name);
   }
   free(metrics);
   free(resources);
}

void Fold(){
   fprintf(stdout,"\nFold:\n");   
   bucketwidth *= 2;
   lastBucketcollected /= 2;
   vup->Fold(bucketwidth);
}

void InvalidMR(){

int mId,rId;

   fprintf(stdout,"\nInvalidate Metric and Resources:\n");   
   mId = rand() % numMets;
   rId = rand() % numRes;
   vup->InvalidMR(mId,rId);
}

void SendData(int numTimes){

dataValue_Array blah;
dataValue   *data;
int i,j,k,m;

  fprintf(stdout,"\nSend Data values to server process: num Buckets = %d\n",numTimes);   
  for(m=0; m<numTimes; m++){
    if(lastBucketcollected == numBuckets){
       Fold();
       fprintf(stdout,"Fold in SendData due to full histogram\n");
    }
    data = (dataValue *)malloc(sizeof(dataValue)*(numMets*numRes));
    k = 0;
    for(i=0;i<numMets;i++)
      for(j=0;j<numRes;j++){
          data[k].metricId = i;
          data[k].resourceId = j;
          data[k].bucketNum = lastBucketcollected; 
          data[k].data = 1.0*(rand() % 1000);
          k++;
     }
     blah.data = data;
     blah.count = numMets*numRes;
     lastBucketcollected++;
     vup->Data(blah);
  }
  free(data);
}


main(int argc,char *argv[]){

int i=0;
int fd,pid;
char **arg_list;
char temp[128];
int wch;
int done;
int numtimes;


  if(argc < 2){
    fprintf(stdout,"incorrect num arguments: client2 server_executable\n");
    fprintf(stdout,"or: client2 server_executable numMetrics numResources numBuckets bucketWidth\n");
    exit(-1);
  }
  fprintf(stderr,"in client\n");
  arg_list = new char*[2];
  arg_list[i++] = strdup(argv[1]);
  arg_list[i] = 0;

  if(argc == 6){
    numMets = atoi(argv[2]);
    numRes  = atoi(argv[3]);
    numBuckets  = atoi(argv[4]);
    bucketwidth  = atof(argv[5]);
    fprintf(stdout," numMets = %d numRes = %d numBuckets = %d bucketwidth = %f\n",numMets,numRes,numBuckets,bucketwidth);
  }
  else{
    numMets = 3;
    numRes  = 4;
    numBuckets  = 10;
    bucketwidth  = 0.5;
  }


  fd = RPCprocessCreate(&pid, "localhost", "", argv[1],arg_list);
  if (fd < 0) {
    perror("process Create");
    exit(-1);
  }

  
  fprintf(stderr,"in client before new visualizationUser\n");
  vup = new visualizationUser(fd,NULL,NULL);

  done = 0;
  while(!done){

      // check for upcall
      // these don't do anything yet 
      if(RPC_readReady(fd))
        vup->awaitResponce(-1);

      wch = -1;
      while((wch < 0) || (wch > 6)){
        fprintf(stdout,"\n---------------------------------------------\n");
        fprintf(stdout,"enter number of operation to perform:\n");
        fprintf(stdout,"---------------------------------------------\n");
        fprintf(stdout,"0:data values  1:fold    2:invalid m/r  3:add m/r\n");
        fprintf(stdout,"4:new m/r      5:phase   6:quit\n");
        fprintf(stdout,"---------------------------------------------\n\n");
        scanf("%d",&wch);
      }
      switch(wch){
	case 0:
	    fprintf(stdout,"enter number of buckets to send\n"); 
	    scanf("%d",&numtimes);
	    SendData(numtimes);

	    break;
	case 1:
	    Fold();
	    break;
	case 2:
	    InvalidMR();
	    break;
	case 3:
	    AddMR(numMets,numRes);
	    break;
	case 4:
	    NewMR(1,1);
	    break;
	case 5:
	    Phase();
	    break;
	case 6:
	    done = 1;
	    break;
      }

  }
  for(i=0;i<2;i++)
    free(arg_list[i]);
  delete [] arg_list;
  delete(vup);

}
