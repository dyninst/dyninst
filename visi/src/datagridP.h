/*
 * Copyright (c) 1996-1999 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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

#ifndef _datagrid_h
#define _datagrid_h

// $Id: datagridP.h,v 1.18 2003/07/15 22:47:45 schendel Exp $

/////////////////////////////////
//  Data Grid Class definitions
/////////////////////////////////

#include <string.h>
#include <math.h>
#if !defined(i386_unknown_nt4_0)
#include <values.h>
#endif // !defined(i386_unknown_nt4_0)
#include "visi/src/visiTypesP.h"
#include "common/h/Vector.h"
#include "common/h/String.h"

#define SUM	0
#define AVE     1

class Metric{
     pdstring     curr_units;    // how current units are measured 
     pdstring     tot_units;     // how total units are measured
     pdstring     name;          // for y-axis labeling  
     u_int      Id;            // unique metric Id
     int        aggregate;     // either SUM or AVE, for fold operation 
     visi_unitsType unitstype; // specifies units type
     pdstring	label;         // for data values, and ave. aggregate
     pdstring	total_label;   // for sum aggregate 
  public:
     Metric() : curr_units(""), tot_units(""),
                name(""), Id(0), aggregate(SUM),
	        unitstype(Normalized), label(""), total_label("") {}
     Metric(pdstring, pdstring, pdstring, u_int, int, visi_unitsType); 
     ~Metric(); 
     const char *CurrUnits(){return(curr_units.c_str());}
     const char *TotUnits(){return(tot_units.c_str());}
     const char *Name(){return(name.c_str());}
     u_int Identifier(){return(Id);}
     int Aggregate(){return(aggregate);}
     visi_unitsType UnitsType(){return(unitstype);}
     const char *Label(){return(label.c_str());}
     const char *AveLabel(){return(label.c_str());}
     const char *SumLabel(){return(total_label.c_str());}
};


class Resource{
     pdstring   name;     // obj. name for graph labeling
     u_int    Id;       // unique resource id
   public:
     Resource(){name = ""; Id = 0;}
     Resource(pdstring, u_int);
     ~Resource();
     const char *Name(){return(name.c_str());}
     u_int    Identifier(){return(Id);}
};

class PhaseInfo{
  private:
    u_int phaseHandle;
    visi_timeType startTime;
    visi_timeType endTime;
    visi_timeType bucketWidth;
    pdstring phaseName;
  public:
    PhaseInfo(){
	    phaseHandle = 0; startTime = -1.0; 
	    endTime = -1.0; bucketWidth = -1.0; phaseName = "";
    }
    PhaseInfo(u_int h, visi_timeType s, visi_timeType e, visi_timeType w,
              pdstring n) {
       phaseHandle = h;
       startTime = s;
       endTime = e;
       bucketWidth = w;
       phaseName = n;
    }
    PhaseInfo(const PhaseInfo &src) : phaseName(src.phaseName) {
       phaseHandle = src.phaseHandle;
       startTime = src.startTime;
       endTime = src.endTime;
       bucketWidth = src.bucketWidth;
    }
    PhaseInfo &operator=(const PhaseInfo &src) {
       if (this == &src)
	  return *this;

       phaseHandle = src.phaseHandle;
       startTime = src.startTime;
       endTime = src.endTime;
       bucketWidth = src.bucketWidth;
       phaseName = src.phaseName;
       return *this;
    }
       
    ~PhaseInfo(){
    }
    void setStartTime(visi_timeType s){ startTime = s;}
    void setEndTime(visi_timeType e){ endTime = e;}
    void setBucketWidth(visi_timeType w){ bucketWidth = w;}
    u_int getPhaseHandle() const { return(phaseHandle);}
    const char *getName() const{ return(phaseName.c_str());}
    visi_timeType getStartTime() const{ return(startTime);}
    visi_timeType getEndTime() const{ return(endTime);}
    visi_timeType getBucketWidth() const{ return(bucketWidth);}
};

extern unsigned enable_pd_samplevalue_debug;

#if ENABLE_DEBUG_CERR == 1
#define sampleVal_cerr if (enable_pd_samplevalue_debug) cerr
#else
#define sampleVal_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

///////////////////////////////////////////////////////////////////
// visi_GridCellHisto: 
// size: number of buckets
// lastBucketFilled: number of full buckets 
// valid:   indicates that the cell contains histogram data  
// enabled: indicates that the cell can accept data values 
// if enabled == 0 then no values can be added to the data grid
// if enabled == 1 and valid == 1 then data values are present
// if enabled == 1 and valid == 0 then values are not present, 
//                                but instrumentation  has been
//                                enabled for this cell
///////////////////////////////////////////////////////////////////

class visi_GridCellHisto {
  private:
     int   valid;    // set when data values are present for this cell
     int   firstValidBucket;  // first index into "value" that is not NaN
     int   size;              // size of array "value"
     int   lastBucketFilled;  // bucket number of last data value added
     int   enabled;   // set when data values can be added to this cell
     visi_sampleType *value;   // array of data values
     visi_sampleType initActVal;   // array of data values
  public: 
     void *userdata;  // to allow visi writer to add info to grid cells
     visi_GridCellHisto(){value = NULL; valid = 0; size = 0; 
			  userdata = NULL; lastBucketFilled = -1; 
			  firstValidBucket = -1; enabled = 0;
			  initActVal = -1;}
     visi_GridCellHisto(int);
     ~visi_GridCellHisto();
     int    LastBucketFilled(){return(lastBucketFilled);}
     int    Size(){return(size);}
     int    Valid(){return(valid);}
     int    Enabled(){return(enabled);}
     void   SetEnabled(){enabled = 1;}
     void   ClearEnabled(){enabled = 0;}
     int    FirstValidBucket() { return(firstValidBucket); }
     void   Invalidate(){delete[] value; value = NULL; size = 0; 
			 valid = 0; lastBucketFilled = -1;}
     visi_sampleType  *GetValueRawData(){ return(value);}

     void SetInitialActualValue(visi_sampleType v) { initActVal = v; }
     visi_sampleType GetInitialActualValue() { return initActVal; }
     int AddNewValues(visi_sampleType *temp, int arraySize,
		      int lbf, void *ud, int v, int e, 
		      visi_sampleType initActVal);
     int AddValue(visi_sampleType x, int i, int numElements);
     void Fold(visi_unitsType unitstype);

     // returns true when there are NaN spans of between valid data buckets
     bool   InvalidSpans(){
        if(value != NULL) {
            for(int i = firstValidBucket; i < lastBucketFilled; i++){
                if(isnan(value[i])) return true;
	    }
	}
	return false;
     }

     visi_sampleType Value(int i, visi_unitsType unitstype);
     void Value(visi_sampleType *samples, int firstBucket, int lastBucket,
		visi_unitsType unitstype);
     visi_sampleType SumValue(visi_timeType width, visi_unitsType unitstype);
     visi_sampleType AggregateValue(visi_unitsType unitstype);
};


////////////////////////////////////////
// visi_GridHistoArray: 
////////////////////////////////////////
class  visi_GridHistoArray {
   private:
      visi_GridCellHisto *values;
      int size;

      visi_GridHistoArray(const visi_GridHistoArray &);

   public:
      visi_GridHistoArray(){values = NULL; size = 0;}
      visi_GridHistoArray(int);
      ~visi_GridHistoArray();

      int LastBucketFilled(int resource){
         if((resource < 0) || (resource >= size))
	   return(VISI_ERROR_INT);
         else
	   return(values[resource].LastBucketFilled());
      }

      visi_GridHistoArray& operator= (visi_GridHistoArray &new_elm){
	  values = new_elm.values; // BUG -- make a real copy
	  size = new_elm.size;
	  new_elm.values = 0;
	  new_elm.size = 0;
	  return *this;
      }

      int SetInitialActualValue(int resource, visi_sampleType x) {
	 if((resource < 0) || (resource >= size))
	   return VISI_ERROR_INT;
	 values[resource].SetInitialActualValue(x);
	 return VISI_OK;
      }

      int AddValue(visi_sampleType x,
	           int resource,
	           int bucketNum,
	           int numBuckets){

	 if((resource < 0) || (resource >= size))
	   return(VISI_ERROR_INT);
	 return(values[resource].AddValue(x,bucketNum,numBuckets));
      }
      int   Size(){ return(size);}
      visi_GridCellHisto *Value(){return(values);}
      int    Valid(int);
      int    Invalidate(int);
      int    AddNewResources(int);
      int    AddNewValues(visi_GridCellHisto *,int);

      void Fold(visi_unitsType unitstype) {
        int i;
	for(i=0; i< size; i++)
	  values[i].Fold(unitstype);
      } 

      visi_sampleType  AggregateValue(int i, visi_unitsType unitstype) {
	if((i>=0)&&(i<size))
	  return(values[i].AggregateValue(unitstype));
        else
	  return(VISI_ERROR);
      }
      visi_sampleType  SumValue(int i,visi_timeType width, 
				visi_unitsType unitstype){
	if((i>=0)&&(i<size))
	  return(values[i].SumValue(width, unitstype));
        else
	  return(VISI_ERROR);
      }

      // returns true if histogram in element "which" contains invalid
      // spans of bucket values
      bool InvalidSpans(int which){
	if((which>=0)&&(which<size))
	  return(values[which].InvalidSpans());
        else
	  return false;
      }

      visi_GridCellHisto&   operator[](int i){
        if ((i>= 0) && (i < size)){
	  return(values[i]);
	}
	else{
	  return(values[0]);
	}
      }
};


///////////////////////////////////////////////////////////////
// visi_DataGrid: 
// metrics:  list of metric info. for metrics in data grid 
// resources: list of resource info. for resources in data grid 
// numBins: number of bins in the histogram of each datagrid cell 
// binWidth: size of each bin in seconds
// data_values: array of size numMetrics each containing an array
//              of size numResources
///////////////////////////////////////////////////////////////
class visi_DataGrid {
 private:
     Metric     *metrics;
     Resource   *resources;
     int         numMetrics;
     int         numResources;
     int         numBins;
     visi_timeType    binWidth;
     visi_GridHistoArray  *data_values;
     pdvector<PhaseInfo *> phases;
     visi_timeType   start_time;
     int 	phase_handle; // -1: global -2: not yet defined
  public:
     visi_DataGrid(){
        metrics=NULL; 
        resources=NULL; 
        numMetrics=numResources=0;
        data_values=NULL; 
        numBins= 0; 
        binWidth=0.0;
        start_time = 0.0;
        phase_handle = -2;
     }
     visi_DataGrid& operator= (const visi_DataGrid& v) { // copy assignment
        if (this != &v) {       // beware of self-assignment!
           u_int p;

		  delete[] metrics;

          numMetrics = v.numMetrics;
          metrics = new Metric[numMetrics];
             // calls default ctor for class Metric
          for (int m=0; m<numMetrics; m++)
              metrics[m] = v.metrics[m]; // operator=() for class Metric

          delete[] resources;
          numResources = v.numResources;
          resources = new Resource[numResources]; // calls default ctor
          for (int r=0; r<numResources; r++)
              resources[r] = v.resources[r]; // calls operator=()

          delete[] data_values;
          data_values = new visi_GridHistoArray[v.numMetrics]; // default ctor
          for (int l=0; l<numMetrics; l++)
              data_values[l] = v.data_values[l]; // operator=()

          numBins = v.numBins;
          binWidth = v.binWidth;
          start_time = v.start_time;
          phase_handle = v.phase_handle;

          for (p=0; p < phases.size(); p++)
             delete phases[p];

          phases.resize(v.phases.size());
          for (p=0; p < phases.size(); p++) {
             const PhaseInfo *src_phase = v.phases[p];
             phases[p] = new PhaseInfo(*src_phase); // copy-ctor for PhaseInfo
          }
        }
        return *this;
     }

     visi_DataGrid(int, int, Metric*, Resource*,
                   int, visi_timeType, visi_timeType, int);
     visi_DataGrid(int, int, visi_metricType*, visi_resourceType*,
		   int, visi_timeType, visi_timeType, int);
     ~visi_DataGrid();
     const char *MetricName(int i);
     const char *MetricUnits(int i);
     const visi_unitsType MetricUnitsType(int i);
     const char *MetricLabel(int i);
     const char *MetricAveLabel(int i);
     const char *MetricSumLabel(int i);
     const char *ResourceName(int j);
     int        NumMetrics(){return(numMetrics);}
     int        FoldMethod(int);
     int        NumResources(){return(numResources);}
     u_int      MetricId(int,bool&); // returns metric Id
     u_int      ResourceId(int,bool&); // returns Resource Id
     int        NumBins(){return(numBins);}
     visi_timeType   BinWidth(){return(binWidth);}
     int        Valid(int,int);  
     int        Invalidate(int,int);
     int        AddNewMetrics(int,visi_metricType *);
     int        AddNewResource(int,visi_resourceType *);
     int        ResourceInGrid(u_int);
     int        MetricInGrid(u_int);
     u_int	NumPhases(){ return(phases.size());}
     void       AddNewPhase(int, visi_timeType, visi_timeType, visi_timeType,
                            pdstring);
     visi_timeType   GetStartTime(){ return(start_time);}
     int 	GetPhaseHandle(){return(phase_handle);}
     const char *GetMyPhaseName();

     const PhaseInfo	*GetPhaseInfo(u_int i){
        u_int j = phases.size();
        if((j == 0) || (i >= j)){
           return(NULL);
        }
        return(phases[i]);
     }

     int AddEndTime(visi_timeType end,u_int handle){
        PhaseInfo *p;

        for(u_int i = 0; i < phases.size(); i++){
           p = phases[i]; 
           if(p->getPhaseHandle() == handle){
              p->setEndTime(end);
              return(1);
           }
        }
        return(0);
     }

     int	ResourceIndex(u_int resId){
        for(int i = 0; i < numResources; i++){
           if(resources[i].Identifier() == resId)
              return(i);
	     }
	     return(-1);
     }

     int	MetricIndex(u_int metId){
        for(int i = 0; i < numMetrics; i++){
           if(metrics[i].Identifier() == metId)
              return(i);
	     }
	     return(-1);
     }

     visi_sampleType AggregateValue(int i,int j){
        sampleVal_cerr << "AggregateValue (avg)-  " << MetricName(i) << " "
                       << ResourceName(j) << "\n";
        
        if((i>=0)&&(i<numMetrics))
           return(data_values[i].AggregateValue(j, metrics[i].UnitsType())); 
        else
           return(VISI_ERROR);
     }

     visi_sampleType  SumValue(int i,int j){
        sampleVal_cerr << "datagrid::SumValue()-  " << MetricName(i) << " "
                       << ResourceName(j) << "\n";
        
        if((i>=0)&&(i<numMetrics))
           return(data_values[i].SumValue(j,binWidth, metrics[i].UnitsType())); 
        else
           return(VISI_ERROR);
     }

     void  Fold(visi_timeType width) {
        int i;
        for(i=0; i < numMetrics; i++) {
           visi_unitsType type = MetricUnitsType(i);
           data_values[i].Fold(type);
        }
        binWidth = width;
     }

     int SetInitialActualValue(int metric, int resource, 
                               visi_sampleType value) {
        if((metric < 0) || (metric >= numMetrics))
           return(VISI_ERROR_INT);
        return (data_values[metric].SetInitialActualValue(resource, value));
     }

     int AddValue(int metric, 
                  int resource, 
                  int bucket,
                  visi_sampleType value){
        if((metric < 0) || (metric >= numMetrics))
           return(VISI_ERROR_INT);
        return(data_values[metric].AddValue(value,resource,bucket,numBins));
     }
     
     visi_GridHistoArray&  operator[](int i){
        if((i < 0) || (i >= numMetrics)){
           return(data_values[0]);
        }
        return(data_values[i]);
     }

     int LastBucketFilled(int metric,int resource){
        if((metric < 0) || (metric >= numMetrics))
           return(VISI_ERROR_INT);
        return(data_values[metric].LastBucketFilled(resource));
     }

     bool InvalidSpans(int metric, int resource){
        if((metric < 0) || (metric >= numMetrics))
           return false;
        else 
           return (data_values[metric].InvalidSpans(resource));
     }

};
#endif
