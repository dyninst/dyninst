#if !defined(i386_unknown_nt4_0)
#include <stream.h> 
#endif // !defined(i386_unknown_nt4_0)

#include <assert.h> 
#include <stdlib.h> 
#include <fstream.h> 
#include <string.h> 

#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <math.h>

#include "tcl.h"
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

int visi_ExportOneHisto(ofstream& fptr, int metric_id, int resource_id)
{
  int firstbucket = visi_FirstValidBucket(metric_id, resource_id);
  int lastbucket = visi_LastBucketFilled(metric_id, resource_id);
  int numbuckets = lastbucket-firstbucket+1;
  visi_sampleType * data;
  int i;

#ifdef DEBUG
  cout << "Writing Header ..." ;
#endif

  fptr << "Histogram:" << endl
       << "\tMetric: "   << visi_MetricName(metric_id)
       << "(\""        << visi_MetricLabel(metric_id) << "\")" << endl 
       << "\tFocus: "    << visi_ResourceName(resource_id) << endl;
 
  fptr << "NumEntries: "  << numbuckets << endl
       << "Granularity: " << visi_BucketWidth() << endl
       << "StartTime: "   << visi_GetStartTime() << endl << endl;

#ifdef DEBUG
  cout << "Done"  <<endl ;
#endif

#ifdef DEBUG
  cout << "Getting *" <<numbuckets <<"* Data Values (" << firstbucket
       << "--" << lastbucket << ") ..." ;
  cout << "visi_NumBuckets returns: " << visi_NumBuckets() <<endl;
#endif

  data = new visi_sampleType[visi_NumBuckets()];
  if(!data){
    cerr << "new() failed" << endl;
    visi_showErrorVisiCallback("new() failed: Save Aborted");
    return -1;
  }

  if( visi_DataValues(metric_id, resource_id, data, firstbucket, lastbucket)
      <= 0){
    //error handler
    cerr << "visi_DataValues failed:" <<metric_id <<resource_id <<endl;
    visi_showErrorVisiCallback("visi_DataValues() failed: Save Aborted");
    return -1;
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
    cout << "Writing bucket[" <<i <<"]: " << data[i] << " ..." ;
#endif
    if(!isnan(data[i]))
      fptr << data[i] << endl;
    else
      fptr << "nan" << endl;
#ifdef DEBUG
    cout << "Done" << endl;
#endif
  }

  delete[] data;
  return 0;
}

// Code to export the histo data:
// dirname: string name of dir to dump files in
// metric_ids/resource_ids: list of m/r pairs to save
//                          Last element of each list must be -1 to denote ENDLIST
int visi_ExportHistos(char * dirname, int *metric_ids, int *resource_ids){
  int findex=0;

  char curfile[256];
  sprintf(curfile, "%s/INDEX", dirname);
  ofstream index (curfile, ios::out);
  ofstream fptr;
  
  for(int i=0; metric_ids[i] != -1; i++){
    sprintf(curfile, "%s/hist_%d", dirname, findex++);
    index << curfile << ":"
          << "\n\tMetric = " << visi_MetricName(metric_ids[i])
          << "\n\tResource = " << visi_ResourceName(resource_ids[i])
          << endl;

    fptr.open(curfile, ios::out);
    if ( fptr.fail() ) {
      cerr << "failed to create fptr (ofstream)" <<endl;
      visi_showErrorVisiCallback("Internal Error (ofstream) -- Save Aborted!");
      index.close();
      return -1;
    }

#ifdef DEBUG
    cout << "Exporting metricid: " << visi_MetricName(metric_ids[i])
         << ", resourceid: " << visi_ResourceName(resource_ids[i]) << " ..." ;
#endif
    if( visi_ExportOneHisto(fptr, metric_ids[i], resource_ids[i]) < 0 ){
      cerr << "ExportOneHisto() failed" <<endl;
      index.close();
      fptr.close();
      return -1;
    }
    fptr.close();
#ifdef DEBUG
    cout << "Success" << endl;
#endif
  }
  index.close();

  //delete dir;
  return 0;
}

#undef DEBUG

#if defined(__cplusplus)
};
#endif /* defined(__cplusplus) */
