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

#include <assert.h> 
#include <stdlib.h> 
#include <fstream> 
#include <string.h> 

#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <math.h>

#include "tcl.h"
#include "common/h/std_namesp.h"
#include "common/h/headers.h"
#include "pdutil/h/pdsocket.h"
#include "visi/h/visualization.h"

//#define DEBUG

#if defined(__cplusplus)
extern "C" {
#endif

static int visi_ExportHistos(char * dirname, int *metric_ids, int *resource_ids);

static const char * get_resource_nickname(const char * resource_name){
  const char *temp;

  temp = resource_name + (strlen(resource_name)-1);

  while(temp != resource_name && *(temp - 1) != '/')
    temp--;

  return temp;
}

// This function takes no arguments and initializes 2 TCL global
// variables:
//   mrIDbyindex[]: list containing formatted strings: "metricid:resourceid"
//   mrlabelbyindex[]: list containing formatted strings: "metriclabel:resourcelabel"

int get_subscribed_mrpairs(ClientData ,
			   Tcl_Interp *interp,
			   int , Tcl_Obj *CONST []){

  int nb_metrics = visi_NumMetrics();
  int nb_resources = visi_NumResources();
  int num_mrpairs=0;
  
  (void)Tcl_UnsetVar(interp, "mrIdByIndex", 0);
  (void)Tcl_UnsetVar(interp, "mrLabelByIndex", 0);

#ifdef DEBUG
  cout << "NumMetrics:" << visi_NumMetrics()
       << " NumResources:" << nb_resources << endl;
#endif

  // Gather subscribed m/r pairs
  for(int i=0; i<nb_metrics; i++){
    for(int j=0; j<nb_resources; j++){
#ifdef DEBUG
      cout << "checking mid: " <<i <<", rid:" << j << "... ";
#endif
      if( visi_Valid(i, j) ){
	char mrID[16];
        char mrLabel[256];

#ifdef DEBUG
        cout << "valid and subscribed!" << endl;
#endif
        sprintf(mrID, "%d:%d", i, j);
        sprintf(mrLabel, "%s:%s", visi_MetricName(i),
                get_resource_nickname(visi_ResourceName(j)));

        bool aflag;
        aflag = (Tcl_SetVar(interp, "mrIdByIndex", 
		 const_cast<char*>(mrID),
		 TCL_GLOBAL_ONLY | TCL_APPEND_VALUE | TCL_LIST_ELEMENT) != NULL);
        assert(aflag);

        aflag = (Tcl_SetVar(interp, "mrLabelByIndex", 
		 const_cast<char*>(mrLabel),
		 TCL_GLOBAL_ONLY | TCL_APPEND_VALUE | TCL_LIST_ELEMENT) != NULL);
        assert(aflag);
        num_mrpairs++;
      }
    }
  }

  Tcl_SetIntObj(Tcl_GetObjResult(interp), num_mrpairs);

  if(num_mrpairs <= 0){
    visi_showErrorVisiCallback("No valid/subscribed Metric:Focus pairs found");
    return TCL_ERROR;
  }

  return TCL_OK;
}          

// This function takes 2 arguments
// 1) Dir to export to
// 2) list of formatted strings representing "metricids:resourceids" for
//    histograms to export

int DoExport(ClientData,
			   Tcl_Interp *interp,
			   int objc, Tcl_Obj *CONST objv[]){
  Tcl_Obj **listItems;
  int error, num_listItems;
  int *metric_ids, *resource_ids;
  int length;
  char *dir;

  if(objc != 3){
    //error
    cerr << "Internal Error: DoExport called w/ wrong number of args" <<endl;
    return TCL_ERROR;
  }

  dir = Tcl_GetStringFromObj(objv[1], &length);
#ifdef DEBUG
  cout << "Export Dir: " << dir << endl;
#endif

  error = Tcl_ListObjGetElements(interp, objv[2], &num_listItems, &listItems);
  if(error != TCL_OK){
    //error
    cerr << "Internal Error: Tcl_ListObjGetElements failed" <<endl;
    return TCL_ERROR;
  }

#ifdef DEBUG
  cerr << "Number of List Items:" <<num_listItems <<endl;
#endif
  metric_ids = new int [num_listItems+1];
  resource_ids = new int [num_listItems+1];

  for(int i=0; i<num_listItems; i++){
    char *temp, *temp2;

    temp2 = Tcl_GetStringFromObj(listItems[i], &length);
    temp = new char [length+1] ;
#ifdef DEBUG
    cerr << "length of temp2:" << strlen(temp2) << endl;
    cerr << "length of temp:" << length+1 << endl;
#endif
    strcpy(temp, temp2);
    temp2 = strpbrk(temp, ":");
    *temp2 = '\0';
    temp2++;

    metric_ids[i] = atoi(temp);
    resource_ids[i] = atoi(temp2);

#ifdef DEBUG
    cerr << "Extracted: " << visi_MetricName(metric_ids[i]) <<":" << visi_ResourceName(resource_ids[i]) << endl;
#endif
    delete[] temp;
  }

  metric_ids[num_listItems] = -1;
  resource_ids[num_listItems] = -1;

#ifdef DEBUG
  cerr << "Done" <<endl;
#endif
  visi_ExportHistos(dir, metric_ids, resource_ids);

  delete[] metric_ids;
  delete[] resource_ids;

  Tcl_SetIntObj(Tcl_GetObjResult(interp), 0);
  return TCL_OK;
}

int visi_ExportMetricTable(std::ofstream& fptr, int metric_id,
                           int * resource_ids)
{
  int i, j;
  for(i=0; resource_ids[i] != -1; i++);
  int num_resources = i;
  int * r_strlen = new int [num_resources];

  int firstbucket = visi_FirstValidBucket(metric_id, resource_ids[0]);
  int lastbucket = visi_LastBucketFilled(metric_id, resource_ids[0]);
  int numbuckets = lastbucket-firstbucket+1;
  visi_sampleType ** data = new visi_sampleType *[num_resources];

#ifdef DEBUG
  cout << "Writing Header ..." ;
#endif

  fptr <<"##############################################################" << endl
       <<"#\tResource Histogram Table for Metric: "   << visi_MetricName(metric_id)
       <<"(\""        << visi_MetricLabel(metric_id) << "\")" << endl
       <<"#\t===========================================================" <<endl
       <<"#" <<endl
       <<"#\tNumEntries: "  << numbuckets << endl
       <<"#\tGranularity: " << visi_BucketWidth() << endl
       <<"#\tStartTime: "   << visi_GetStartTime() << endl
       <<"##############################################################" << endl;

  fptr <<endl << "Resources: ";
  for(i=0; i<num_resources; i++){
    fptr << visi_ResourceName(resource_ids[i]) << " ";
    r_strlen[i] = strlen(visi_ResourceName(resource_ids[i]));
  }
  fptr << endl;
 
#ifdef DEBUG
  cout << "Done"  <<endl ;
#endif

#ifdef DEBUG
  cout << "Getting *" <<numbuckets <<"* Data Values (" << firstbucket
       << "--" << lastbucket << ") ..." ;
  cout << "visi_NumBuckets returns: " << visi_NumBuckets() <<endl;
#endif

  for(i=0; i<num_resources; i++){
    data[i] = new visi_sampleType[visi_NumBuckets()];
    if(!data[i]){
      cerr << "new() failed" << endl;
      visi_showErrorVisiCallback("new() failed: Save Aborted");
      return -1;
    }
  }

  for(i=0; i<num_resources; i++){
    if( visi_DataValues(metric_id, resource_ids[i], data[i], firstbucket, lastbucket)
      <= 0){
      cerr << "visi_DataValues failed:" <<metric_id <<resource_ids[i] <<endl;
      visi_showErrorVisiCallback("visi_DataValues() failed: Save Aborted");
      return -1;
    }
  }

#ifdef DEBUG
  cout << "Success"  <<endl ;
#endif

#ifdef DEBUG
  cout << "Writing data[" << firstbucket <<"] -- data["
       << lastbucket << "]"  <<endl ;
#endif
  for (i=firstbucket; i<= lastbucket ; i++){
#ifdef DEBUG
    cout << "Writing bucket[" <<i <<"]: " <<endl;
#endif
    fptr << "           ";
    for(j=0; j<num_resources; j++){
      fptr.width(r_strlen[j]);

      if(!isnan(data[j][i]))
        fptr << data[j][i] ;
      else
        fptr << "nan" ;
    }
    fptr <<endl;
  }
  fptr <<endl;
#ifdef DEBUG
  cout << "Done" << endl;
#endif

  for(i=0; i<num_resources; i++)
    delete[] data[i];
  delete[] data;
  return 0;
}

// Code to export the histo data:
// dirname: string name of dir to dump files in
// metric_ids/resource_ids: list of m/r pairs to save
//                          Last element of each list must be -1 to denote ENDLIST
#define SIZE 20  //warning: static size used for temp arrays.
int visi_ExportHistos(char * filename, int *metric_ids, int *resource_ids){
  char curfile[256];
  sprintf(curfile, "%s", filename);
  std::ofstream fptr (curfile, std::ios::out);
  bool saved[SIZE];
  int tmp_resource_list[SIZE];
  int index, i, j;
  
  for(i=0; i<SIZE; i++)
    saved[i] = false;

  for(i=0; metric_ids[i] != -1; ){
    index=0;
    for(j=i; metric_ids[j] != -1; j++){
      if( !saved[j] && (metric_ids[i] == metric_ids[j]) ){
        tmp_resource_list[index++] = resource_ids[j];
        saved[j] = true;
      }
    }
    tmp_resource_list[index] = -1;

#ifdef DEBUG
  cout << "Exporting metric: " << visi_MetricName(metric_ids[i])
       << ", resources: ";
  for(i=0; tmp_resource_list[i]!=-1; i++)
    cout << visi_ResourceName(tmp_resource_list[i]) << " ..." << endl;
#endif
    
    if( visi_ExportMetricTable(fptr, metric_ids[i], tmp_resource_list) < 0 ){
      cerr << "ExportMetricTable() failed" <<endl;
      return -1;
    }

    while( saved[i] )
      i++;
  }
  fptr.close();

#ifdef DEBUG
    cout << "Success" << endl;
#endif

  return 0;
}

#undef DEBUG

#if defined(__cplusplus)
};
#endif /* defined(__cplusplus) */
