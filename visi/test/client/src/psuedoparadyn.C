/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

/* $Id: psuedoparadyn.C,v 1.12 2004/04/16 20:41:16 legendre Exp $
 * */ 
#include <stdio.h>
#include <iostream>
#include "common/h/String.h"
#include "common/h/Vector.h"
#include "pdutil/h/makenan.h"
#include "visi.xdr.CLNT.h" 


#define DEFAULT_BUCKET_WIDTH 0.2    // initial width of each histogram bucket
#define DEFAULT_NUM_BUCKETS  1000   // number of histogram buckets

// global variables
visualizationUser *visip;
int currPhaseId = 0;
int lastBucketFilled = 0;
double bucketWidth = DEFAULT_BUCKET_WIDTH;
double minbucketWidth = DEFAULT_BUCKET_WIDTH;
int numBuckets = DEFAULT_NUM_BUCKETS;
u_int nextMetId = 0;
u_int nextResId = 0;
pdvector<T_visi::visi_matrix> mrlist;
void SendData(u_int,u_int,int &,double &);
void Fold(double &bucketWidth,int &lastBucketCollected);
void NewMR();
void NewPhase(int &currId,double binSize,int lastBucket,double minWidth);

// upcall routines for visualization
void visualizationUser::GetMetricResource(pdstring metric_res,int num, int type){

  cerr << "## in visualizationUser::GetMetricResource " << endl;
  NewMR();
}


void visualizationUser::StopMetricResource(u_int metricId,u_int resourceId){

  cerr << "## in visualizationUser::StopMetricResource " << endl;
  cerr << "## metId = " << metricId << " resId = " << resourceId << endl;
  u_int size = mrlist.size();
  for(u_int i=0; i < size; i++){
      if((metricId == mrlist[i].met.Id) && (resourceId == mrlist[i].res.Id)){
          mrlist[i] = mrlist[(size - 1)];
	  mrlist.resize(size - 1);
	  return;
  }}
}

void visualizationUser::StartPhase(double begin, pdstring name, bool wperf, bool wvisi){

  cerr << "## in visualizationUser::StartPhase " << endl;
  cerr << "## begin = " << begin << " name = " << name << endl;
  NewPhase(currPhaseId,bucketWidth,lastBucketFilled,minbucketWidth);
}

void visualizationUser::GetPhaseInfo(){
    cerr << "## in visualizationUser::GetPhaseInfo upcall" << endl;
    cerr << "##     this routine is not supported" << endl;
}

void visualizationUser::showError(int, pdstring){

}


main(int argc, char *argv[]){


    if(argc < 2){
        cerr << "Error: command must be in following form:" << endl; 
	cerr << " (1)  psuedoparadyn server_executable <server_args>" << endl;
	exit(-1);
    }

    int pid;
    pdvector<pdstring> arg_list;
    
    if(argc > 2){
        u_int index = 2;
	while(argv[index]){
	    arg_list += argv[index];
	    index++;
        } 
    }

    //    int fd = RPCprocessCreate(pid, "localhost", "", argv[1], arg_list);
    int fd = RPCprocessCreate("localhost", "", argv[1], "", arg_list, 0);
    if(fd < 0) exit(-1);
    visip = new visualizationUser(fd,NULL,NULL,false);
   
    // send initial set of metrics and foci to visi


    // enter loop
    bool done = false;
    while(!done){

	while (visip->buffered_requests()) {
	      if (visip->process_buffered() == T_visi::error) {
	          cerr << "error on visi\n";
	          assert(0);
	    }
	}
        if(RPC_readReady(fd)){
	    if(visip->waitLoop() == T_visi::error){
		cerr << "error on visi\n";
	        assert(0);
        }}
	else {
            int wch = -1;
 	    while((wch < 0) || (wch > 9)){
		cerr << endl << "----------------------------------" << endl;
		cerr << "Enter number of operation to perform:" << endl;
		cerr << "    0: add random data values (values are between 0 and 100)" << endl;
		cerr << "    1: add NULL data values" << endl;
		cerr << "    2: send ZERO data values   " << endl;
		cerr << "    3: send user specified data Values"  << endl;    
		cerr << "    4: fold" << endl;
		cerr << "    5: start a new phase" << endl;
		cerr << "    6: add new Met/Res" << endl;
		cerr << "    7: print all active metrics and resources" << endl;
		cerr << "    8: continue (do nothing--useful for receiving upcalls from visi process)"   << endl;
		cerr << "    9: quit" << endl;
		cerr << endl << "----------------------------------" << endl;
		scanf("%d",&wch);
	    }
	    switch(wch){
                case 0:
		    cerr << "enter number of buckets to send" << endl;
		    scanf("%d",&wch);
		    SendData(wch,0,lastBucketFilled,bucketWidth);
		    break;
                case 1:
		    cerr << "enter number of NULL buckets to send" << endl;
		    scanf("%d",&wch);
		    SendData(wch,1,lastBucketFilled,bucketWidth);
		    break;
                case 2:
		    cerr << "enter number of ZERO buckets to send" << endl;
		    scanf("%d",&wch);
		    SendData(wch,2,lastBucketFilled,bucketWidth);
		    break;
                case 3:
		    cerr << "enter number of buckets to send" << endl;
		    scanf("%d",&wch);
		    SendData(wch,3,lastBucketFilled,bucketWidth);
		    break;
                case 4:
		    Fold(bucketWidth,lastBucketFilled);
		    break;
                case 5:
                    NewPhase(currPhaseId,bucketWidth,
			     lastBucketFilled,minbucketWidth);
		    break;
                case 6:
		    NewMR();
		    break;
                case 7:
		    {
			cerr << "LIST OF ENABLED METRIC/FOCUS PAIRS" << endl;
                        for(u_int i=0; i < mrlist.size(); i++){
			    cerr << "    metric[" << i << "] = " << 
				  mrlist[i].met.name << " res[" <<
				  i << "] = " << mrlist[i].res.name
				  << endl;
			}
		    }
		    break;
                case 8:
		    break;
                case 9:
		    done = true;
		    break;
                default:
		    break;
	    }
	}
    }

}

void SendData(u_int howmany, 
	      u_int type_values,
	      int &lastBucketCollected,
	      double &bucketWidth){

    if(howmany == 0) return;

    float user_selected_value = 0.0;
    if(type_values == 3){
        cerr << "enter data value as a float" << endl;
        scanf("%f",&user_selected_value);
    }
    pdvector <T_visi::dataValue> tempData;
    for(u_int i=0; i < howmany; i++){
        for(u_int j=0; j < mrlist.size(); j++){
	    T_visi::dataValue newValue;
	    newValue.metricId = mrlist[j].met.Id;
	    newValue.resourceId = mrlist[j].res.Id;
	    newValue.bucketNum = lastBucketCollected;
	    newValue.data = 0.0;
	    if(type_values == 1){
		newValue.data = PARADYN_NaN; 
            }
	    else if(type_values == 0){
	        newValue.data = 0.1*(rand() % 1000);
            }
	    else if(type_values == 3){
                newValue.data = user_selected_value;
	    }
	    tempData += newValue;
	}
	lastBucketCollected++;
	if(lastBucketCollected > numBuckets){
	    // send new data values, then do a fold
	    visip->Data(tempData);
	    tempData.resize(0);
	    Fold(bucketWidth,lastBucketCollected);
	}
    }
    visip->Data(tempData);
}

void Fold(double &bucketWidth,int &lastBucketCollected){
    bucketWidth *=2;
    lastBucketCollected /=2;
    visip->Fold(bucketWidth);
}

void NewPhase(int &currId,double binSize,int lastBucket,double minWidth){

    double currTime = binSize*lastBucket;
    if(currId > 0)
        visip->PhaseEnd(currTime,currId);
    currId++;
    char temp[32];
    sprintf(temp,"%s%d","phase_",currId);
    pdstring name = temp;
    visip->PhaseStart(currTime,-1.0,minWidth,name,currId);
}

void NewMR(){

    cerr << "Add new metrics and resources to visualization:" << endl;
    cerr << "Enter number of metrics and number of resources" << endl;
    u_int num_mets = 0, num_res = 0;
    scanf("%d%d",&num_mets,&num_res);
    pdvector<T_visi::visi_matrix> newlist;
    for(u_int i=0; i < num_mets; i++){
        for(u_int j=0; j < num_res; j++){
	    T_visi::visi_matrix newElement;
	    char temp[32];
	    sprintf(temp, "%s%d","metric_",nextMetId);
	    newElement.met.name = temp;
	    sprintf(temp, "%s%d","units_",nextMetId);
	    newElement.met.curr_units = temp;
	    newElement.met.tot_units = temp;       
	    newElement.met.Id = nextMetId;
	    newElement.met.aggregate = 1;
	    sprintf(temp, "%s%d","resource_",nextResId+j);
	    newElement.res.name = temp;
	    newElement.res.Id = nextResId+j;
	    newlist += newElement;
        }
        nextMetId++;
    }
    nextResId += num_res;
    visip->AddMetricsResources(newlist,bucketWidth,numBuckets,0.0,-1);
    mrlist += newlist;
}

