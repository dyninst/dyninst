/* $Log $  */ 
#include "visi.CLNT.h" 



// upcall routines for visualization

void visualizationUser::GetMetricResource(String metric,String resource,int type){

  fprintf(stderr,"## in visualizationUser::GetMetricResource \n");
  fprintf(stderr,"## metric = %s resource = %s type = %d\n",metric,resource,type);
}


void visualizationUser::StopMetricResource(int metricId,int resourceId){

  fprintf(stderr,"## in visualizationUser::StopMetricResource \n");
  fprintf(stderr,"## metricId = %d resourceId =%d",metricId,resourceId);

}

void visualizationUser::PhaseName(double begin,double end,String name){

  fprintf(stderr,"## in visualizationUser::PhaseName \n");
  fprintf(stderr,"## begin = %f end =%f name = %s",begin,end,name);
}





