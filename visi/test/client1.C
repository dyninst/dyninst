/* $Log: client1.C,v $
/* Revision 1.4  1994/03/26 04:37:01  newhall
/* change all floats to double
/* */
#include "visi.CLNT.h"

#define TESTMETRIC	"blah,blah blah,metric 3,metric 4,last metric"
#define TESTRESOURCE	"resource1/blah1/foo1,resource2/foo2,resource3"


main(int argc,char *argv[]){

int i=0;
int fd,pid;
visualizationUser *vup;
char **arg_list;
char temp[128], temp2[128];
int wch;

int done;
metricType *metrics;
metricType_Array mets;
resourceType *resources;
resourceType_Array res;
dataValue   *data;
dataValue_Array blah;
int mId,rId,bNum,nval;
double num,num2;
int nMets,nRes;
double bWidth;


  if(argc < 2){
    fprintf(stdout,"incorrect num arguments: client1 server_executable\n");
    exit(-1);
  }
  fprintf(stderr,"in client\n");
  arg_list = new char*[argc+1];
  arg_list[i++] = strdup(argv[1]);
  if(argc > 2){ 
    sprintf(temp,"%s",TESTMETRIC);
    arg_list[i++] = strdup(temp);
    sprintf(temp,"%s",TESTRESOURCE);
    arg_list[i++] = strdup(temp);
  }
  arg_list[i++] = 0;


  fprintf(stderr,"in client before RPCprocessCreate\n");
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
      while((wch < 0) || (wch > 7)){
        fprintf(stdout,"\n---------------------------------------------\n");
        fprintf(stdout,"enter number of operation to perform:\n");
        fprintf(stdout,"---------------------------------------------\n");
        fprintf(stdout,"0:data values  1:fold    2:invalid m/r  3:add m/r\n");
        fprintf(stdout,"4:new m/r      5:phase       6:print    7:quit\n");
        fprintf(stdout,"---------------------------------------------\n\n");
        scanf("%d",&wch);
      }
      switch(wch){
	case 0:
	    fprintf(stdout,"\nSend Data values to server process:\n");   
	    fprintf(stdout,"enter number of dataValues\n");
            scanf("%d",&nval);
	    data = (dataValue *)malloc(sizeof(dataValue)*nval);
	    for(i=0;i<nval;i++){
	      fprintf(stdout,"enter metricId (int)\n");   
              scanf("%d",&data[i].metricId);
	      fprintf(stdout,"enter resourceId (int)\n");   
              scanf("%d",&data[i].resourceId);
	      fprintf(stdout,"enter bucketNum (int)\n");   
              scanf("%d",&data[i].bucketNum);
	      fprintf(stdout,"enter data value (double)\n");   
              scanf("%lf",&num);
	      data[i].data = num;
	    }

            blah.data = data;
	    blah.count = nval;
	    vup->Data(blah);
	    free(data);
	    break;
	case 1:
	    fprintf(stdout,"\nFold:\n");   
	    fprintf(stdout,"enter new bucket width value (double)\n");
            scanf("%lf",&num);
	    vup->Fold(num);
	    break;
	case 2:
	    fprintf(stdout,"\nInvalidate Metric and Resources:\n");   
	    fprintf(stdout,"enter metricId (int)\n");   
            scanf("%d",&mId);
	    fprintf(stdout,"enter resourceId (int)\n");   
            scanf("%d",&rId);
	    vup->InvalidMR(mId,rId);
	    break;
	case 3:
	    fprintf(stdout,"\nAdd Metrics and Resorces:\n");   
	    fprintf(stdout,"enter number of metrics (int)\n");
            scanf("%d",&nMets);
	    metrics = (metricType *)malloc(sizeof(metricType)*nMets);
	    for(i=0;i<nMets;i++){
	     fprintf(stdout,"enter metric name (string), Id (int), units (string), and aggregate(0 or 1)\n");
	     scanf("%s%d%s%d",temp,&metrics[i].Id,temp2,&metrics[i].aggregate);
	     metrics[i].name = strdup(temp);
	     metrics[i].units = strdup(temp2);
	    }
	    mets.count = nMets;
	    mets.data = metrics;
	    fprintf(stdout,"enter number of resources  (int)\n");
            scanf("%d",&nRes);
	    resources = (resourceType *)malloc(sizeof(resourceType)*nRes);
	    for(i=0;i<nRes;i++){
	     fprintf(stdout,"enter resources name (string), Id (int)\n");
	     scanf("%s%d",temp,&resources[i].Id);
	     resources[i].name = strdup(temp);
	    }
	    res.count = nRes;
	    res.data = resources;
	    fprintf(stdout,"enter numBuckets (int)\n");
            scanf("%d",&bNum);
	    fprintf(stdout,"enter bucket width (double)\n");
            scanf("%lf",&bWidth);
	    vup->AddMetricsResources(mets,res,bWidth,bNum);
	    for(i=0;i<nMets;i++){
	      free(metrics[i].name);
	      free(metrics[i].units);
	    }
	    for(i=0;i<nRes;i++){
	      free(resources[i].name);
	    }
	    free(metrics);
	    free(resources);
	    break;
	case 4:
	    fprintf(stdout,"\nNew Metrics and Resorces: not currently supported\n");   
	    fprintf(stdout,"enter number of metrics (int)\n");
            scanf("%d",&nMets);
	    metrics = (metricType *)malloc(sizeof(metricType)*nMets);
	    for(i=0;i<nMets;i++){
	     fprintf(stdout,"enter metric name(string), Id (int), units(string), and aggregate (int 0 or 1)\n");
	     scanf("%s%d%s%d",temp,&metrics[i].Id,temp2,&metrics[i].aggregate);
	     metrics[i].name = strdup(temp);
	     metrics[i].units = strdup(temp2);
	    }
	    mets.count = nMets;
	    mets.data = metrics;
	    fprintf(stdout,"enter number of resources (int)\n");
            scanf("%d",&nRes);
	    resources = (resourceType *)malloc(sizeof(resourceType)*nRes);
	    for(i=0;i<nRes;i++){
	     fprintf(stdout,"enter resources name (string), Id (int)\n");
	     scanf("%s%d",temp,&resources[i].Id);
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
	    break;
	case 5:
	    fprintf(stdout,"\nPhase:\n");   
	    fprintf(stdout,"enter phase name(string), begin(double), end(doulbe)\n"); 
	    scanf("%s%lf%lf",temp,&num,&num2);
	    vup->Phase(num,num2,temp);
	    break;
	case 6:
	    vup->Phase(3.4,4.5,"blah");
	    break;
	case 7:
	    done = 1;
	    break;

      }

  }
  for(i=0;i<argc;i++)
    free(arg_list[i]);
  delete [] arg_list;
  delete(vup);

}
