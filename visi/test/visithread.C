/* $Log: visithread.C,v $
/* Revision 1.4  1994/06/08 17:34:48  newhall
/* *** empty log message ***
/*
 * Revision 1.3  1994/04/13  21:21:46  newhall
 * *** empty log message ***
 *
 * Revision 1.2  1994/03/29  02:55:58  newhall
 * *** empty log message ***
 *  */ 
#include <stdio.h>
#include "../sparc-sun-sunos4.1.3/visi.CLNT.h" 



// upcall routines for visualization

void visualizationUser::GetMetricResource(String metric,String resource,int type){

  fprintf(stderr,"## in visualizationUser::GetMetricResource \n");
  if(metric == NULL)
    fprintf(stderr,"metric NULL\n");
  else
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





