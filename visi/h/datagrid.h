#ifndef _datagrid_h
#define _datagrid_h
/*
 * Copyright (c) 1993, 1994 Barton P. Miller, Jeff Hollingsworth,
 *     Bruce Irvin, Jon Cargille, Krishna Kunchithapadam, Karen
 *     Karavanic, Tia Newhall, Mark Callaghan.  All rights reserved.
 * 
 * This software is furnished under the condition that it may not be
 * provided or otherwise made available to, or used by, any other
 * person, except as provided for by the terms of applicable license
 * agreements.  No title to or ownership of the software is hereby
 * transferred.  The name of the principals may not be used in any
 * advertising or publicity related to this software without specific,
 * written prior authorization.  Any use of this software must include
 * the above copyright notice.
 *
 */

/* $Log: datagrid.h,v $
/* Revision 1.21  1995/11/17 17:27:54  newhall
/* added normalized member to Metric class which specifies units type
/* added MetricLabel, MetricAveLabel, and MetricSumLabel DG method functions
/*
 * Revision 1.20  1995/11/12  23:29:24  newhall
 * removed warnings, removed error.C
 *
 * Revision 1.19  1995/11/12  00:45:05  newhall
 * added PARADYNEXITED event, added "InvalidSpans" dataGrid method
 *
 * Revision 1.18  1995/08/01  01:58:43  newhall
 * changes relating to phase interface stuff
 *
 * Revision 1.17  1995/06/02  21:01:54  newhall
 * changed type of metric and focus handles to u_int
 *
 * Revision 1.16  1995/02/26  01:59:27  newhall
 * added phase interface functions
 *
 * Revision 1.15  1994/11/08  01:51:04  newhall
 * array bounds error fix to visi_GridCellHisto::Value
 *
 * Revision 1.14  1994/11/02  04:14:35  newhall
 * memory leak fixes
 *
 * Revision 1.13  1994/09/25  01:57:45  newhall
 * added visi_DataGrid method functions ResourceIndex, MetricIndex
 *
 * Revision 1.12  1994/08/11  02:49:35  newhall
 * removed deleted member to gridcell
 *
 * Revision 1.11  1994/07/30  03:25:25  newhall
 * added enabled member to gridcell to indicate that the metric associated
 * w/ this cell has been enabled and data will arrive for it eventually
 * updated member functions affected by this addition
 *
 * Revision 1.10  1994/07/20  22:17:20  newhall
 * added FirstValidBucket method function to visi_GridCellHisto class
 *
 * Revision 1.9  1994/06/16  18:21:46  newhall
 * bug fix to AddNewValues
 *
 * Revision 1.8  1994/06/07  17:47:16  newhall
 * added method functions to support adding metrics
 * and resources to an existing data grid
 *
 * Revision 1.7  1994/05/23  20:55:16  newhall
 * To visi_GridCellHisto class: added deleted flag, SumValue
 * method function, and fixed AggregateValue method function
 *
 * Revision 1.6  1994/05/11  17:11:03  newhall
 * changed data values from double to float
 *
 * Revision 1.5  1994/04/13  21:22:51  newhall
 * *** empty log message ***
 *
 * Revision 1.4  1994/03/26  04:17:44  newhall
 * change all floats to double
 *
 * Revision 1.3  1994/03/17  05:17:25  newhall
 * added lastBucketFilled data member to class visi_GridCellHisto:  value of
 * the largest bucket number for which new data values have been added
 *
 * Revision 1.2  1994/03/15  02:03:19  newhall
 * added public member "userdata" to class visi_GridCellHisto
 * to allow visi writer to add info to grid cells
 *
 * Revision 1.1  1994/03/14  20:27:26  newhall
 * changed visi subdirectory structure
 *  */ 
/////////////////////////////////
//  Data Grid Class definitions
/////////////////////////////////

#include <string.h>
#include  <math.h>
#include <values.h>
#include "visiTypes.h"
#include "util/h/Vector.h"
#include "util/h/String.h"

#define SUM	0
#define AVE     1

class Metric{
     string     units;    // how units are measured  i.e. "ms" 
     string     name;     // for y-axis labeling  
     u_int      Id;       // unique metric Id
     int        aggregate; //either SUM or AVE, for fold operation 
     bool	normalized; // specifies units type
     string	label;      // for data values, and ave. aggregate
     string	total_label; // for sum aggregate 
  public:
     Metric(){name = 0; units = 0; Id = 0; aggregate=SUM;
	      normalized = true; label = 0; total_label = 0;}
     Metric(string,string,u_int,int,bool); 
     ~Metric(); 
     const char *Units(){return(units.string_of());}
     const char *Name(){return(name.string_of());}
     u_int       Identifier(){return(Id);}
     int         Aggregate(){return(aggregate);}
     bool        Normalized(){return(normalized);}
     const char *Label(){return(label.string_of());}
     const char *AveLabel(){return(label.string_of());}
     const char *SumLabel(){return(total_label.string_of());}
};


class Resource{
     string   name;     // obj. name for graph labeling
     u_int    Id;       // unique resource id
   public:
     Resource(){name = NULL; Id = 0;}
     Resource(string,u_int);
     ~Resource();
     const char *Name(){return(name.string_of());}
     u_int    Identifier(){return(Id);}
};

class PhaseInfo{
  private:
    u_int phaseHandle;
    timeType startTime;
    timeType endTime;
    timeType bucketWidth;
    string phaseName;
  public:
    PhaseInfo(){
	    phaseHandle = 0; startTime = -1.0; 
	    endTime = -1.0; phaseName = NULL;
    }
    PhaseInfo(u_int h,timeType s,timeType e,timeType w, string n){
	   phaseHandle = h;
	   startTime = s;
	   endTime = e;
	   bucketWidth = w;
	   phaseName = n;
    }
    ~PhaseInfo(){
    }
    void setStartTime(timeType s){ startTime = s;}
    void setEndTime(timeType e){ endTime = e;}
    void setBucketWidth(timeType w){ bucketWidth = w;}
    u_int  getPhaseHandle() const { return(phaseHandle);}
    const char *getName() { return(phaseName.string_of());}
    timeType getStartTime() { return(startTime);}
    timeType getEndTime() { return(endTime);}
    timeType getBucketWidth() { return(bucketWidth);}
};


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
     sampleType *value;   // array of data values
  public: 
     void *userdata;  // to allow visi writer to add info to grid cells
     visi_GridCellHisto(){value = NULL; valid = 0; size = 0; 
			  userdata = NULL; lastBucketFilled = -1; 
			  firstValidBucket = -1; enabled = 0;}
     visi_GridCellHisto(int);
     ~visi_GridCellHisto();
     int    LastBucketFilled(){return(lastBucketFilled);}
     sampleType  *Value(){ return(value);}

     sampleType  Value(int i) { 
	    if((i < 0) || (i >= size)){
	     return(ERROR);
            }
	    return(value[i]);
     }

     int    Size(){return(size);}
     int    Valid(){return(valid);}
     int    Enabled(){return(enabled);}
     void   SetEnabled(){enabled = 1;}
     void   ClearEnabled(){enabled = 0;}
     int    FirstValidBucket() { return(firstValidBucket); }
     void   Invalidate(){delete[] value; value = NULL; size = 0; 
			 valid = 0; lastBucketFilled = -1;}

     int    AddNewValues(sampleType *temp,
			 int arraySize,
			 int lbf,
			 void *ud,
			 int v, 
			 int e){
        
	if(temp == NULL){
          value = NULL;
	  size = 0;
	}
	else{
	  // initialize cell to temp values
	  value = new sampleType[arraySize];
	  size = arraySize;
	  for(int i=0;i<size;i++){
	    if(!isnan(temp[i])){
	      value[i] = temp[i]; 
	      if(firstValidBucket == -1)
	         firstValidBucket = i;
            }
	    else
	      value[i] = ERROR; 
	  }
	}
	lastBucketFilled = lbf;
	userdata = ud;
	valid = v;
	enabled = e;
	return(OK);
     }

     void   Fold(){
       int i,j;
       if(valid){
	 firstValidBucket = -1;
         for(i=0,j=0;(i< (lastBucketFilled+1)/2) // new bucket counter
	     && (j< (lastBucketFilled+1)); // old bucket counter
	     i++,j+=2){
	   if((!isnan(value[j])) && (!isnan(value[j+1]))){
             value[i] = value[j] + value[j+1];
	     if(firstValidBucket == -1){
	       firstValidBucket = i;
             }
	   }
           else{
	     value[i] = ERROR;
           }
	   if((value[i] != ERROR))
	     value[i] = value[i]/2; 
	 }
	 for(i=(lastBucketFilled+1)/2; i < size; i++){
           value[i] = ERROR;
	 }
	 lastBucketFilled = ((lastBucketFilled+1)/2)-1;
       }
     }

     sampleType  SumValue(timeType width){
       int i;
       sampleType sum;

        if(value != NULL){
           for(sum=0.0,i=0; i< size; i++){
	     if(!isnan(value[i])){
	       sum += value[i]; 
	     }
	   }
	   return(sum*width);
	}
	else{
	  return(ERROR);
	}
     }

     sampleType  AggregateValue(){
	int i,num;
	sampleType sum;

        if(value != NULL){
           for(sum=0.0,num=i=0; i< size; i++){
	     if(!isnan(value[i])){
	       sum += value[i]; 
	       num++;
	     }
	   }

           if(num != 0){
	     return(sum/(1.0*num));
	   }
	   else{
	     return(ERROR);
           }
	}
	else{
	  return(ERROR);
	}
     }

     // returns true when there are NaN spans of between valid data buckets
     bool   InvalidSpans(){
        if(value != NULL) {
            for(int i = firstValidBucket; i < lastBucketFilled; i++){
                if(isnan(value[i])) return true;
	    }
	}
	return false;
     }

     int    AddValue(sampleType x,
		     int i,
		     int numElements){
        
       int j;

       if (!enabled){ // if this cell has not been enabled don't add values
         return(OK);
       }
       if (!valid){ // if this is the first value create a histo cell array 
	 if(value == NULL)
	   value = new sampleType[numElements];
	 size = numElements;
	 valid = 1;
	 enabled = 1;
	 for(j=0;j<size;j++){
	   value[j] = ERROR;
         }
       }
       if((i < 0) || (i >= size))
	 return(ERROR_SUBSCRIPT);
       value[i] = x;
       if(i > lastBucketFilled)
        lastBucketFilled = i;
       if(firstValidBucket == -1)
	 firstValidBucket = i;
       return(OK);
     }

     sampleType  operator[](int i){
       if((i >= size) || (i < 0)){
	 return(ERROR);
       }
       return(value[i]);
     }
};


////////////////////////////////////////
// visi_GridHistoArray: 
////////////////////////////////////////
class  visi_GridHistoArray {
   private:
      visi_GridCellHisto *values;
      int size;
   public:
      visi_GridHistoArray(){values = NULL; size = 0;}
      visi_GridHistoArray(int);
      ~visi_GridHistoArray();

      int LastBucketFilled(int resource){
         if((resource < 0) || (resource >= size))
	   return(ERROR_SUBSCRIPT);
         else
	   return(values[resource].LastBucketFilled());
      }

      int AddValue(sampleType x,
	           int resource,
	           int bucketNum,
	           int numBuckets){

	 if((resource < 0) || (resource >= size))
	   return(ERROR_SUBSCRIPT);
	 return(values[resource].AddValue(x,bucketNum,numBuckets));
      }
      int   Size(){ return(size);}
      visi_GridCellHisto *Value(){return(values);}
      int    Valid(int);
      int    Invalidate(int);
      int    AddNewResources(int);
      int    AddNewValues(visi_GridCellHisto *,int);

      void   Fold(){
        int i;
	for(i=0; i< size; i++)
	  values[i].Fold();
      } 

      sampleType  AggregateValue(int i){
	if((i>=0)&&(i<size))
	  return(values[i].AggregateValue());
        else
	  return(ERROR);
      }
      sampleType  SumValue(int i,timeType width){
	if((i>=0)&&(i<size))
	  return(values[i].SumValue(width));
        else
	  return(ERROR);
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
     timeType    binWidth;
     visi_GridHistoArray  *data_values;
     vector<PhaseInfo *> phases;
     timeType   start_time;
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

     visi_DataGrid(int, int, Metric*, Resource*, int, timeType, timeType, int);
     visi_DataGrid(int, int, visi_metricType*, visi_resourceType*,
		   int, timeType, timeType, int);
     ~visi_DataGrid();
     const char *MetricName(int i);
     const char *MetricUnits(int i);
     const char *MetricLabel(int i);
     const char *MetricAveLabel(int i);
     const char *MetricSumLabel(int i);
     const char *ResourceName(int j);
     int        NumMetrics(){return(numMetrics);}
     int        FoldMethod(int);
     int        NumResources(){return(numResources);}
     u_int      *MetricId(int); // returns metric Id
     u_int      *ResourceId(int); // returns Resource Id
     int        NumBins(){return(numBins);}
     timeType   BinWidth(){return(binWidth);}
     int        Valid(int,int);  
     int        Invalidate(int,int);
     int        AddNewMetrics(int,visi_metricType *);
     int        AddNewResource(int,visi_resourceType *);
     int        ResourceInGrid(u_int);
     int        MetricInGrid(u_int);
     int	NumPhases(){ return(phases.size());}
     void       AddNewPhase(int,timeType,timeType,timeType,string);
     timeType   GetStartTime(){ return(start_time);}
     int 	GetPhaseHandle(){return(phase_handle);}
     const char *GetMyPhaseName();

     PhaseInfo	*GetPhaseInfo(unsigned i){
	    unsigned j = phases.size();
            if((j == 0) || (i >= j)){
		return(NULL);
            }
	    return(phases[i]);
     }

     int AddEndTime(timeType end,unsigned handle){
	   PhaseInfo *p;

           for(unsigned i = 0; i < phases.size(); i++){
	       p = phases[i]; 
	       if(p->getPhaseHandle() == handle){
	          p->setEndTime(end);
		  return(1);
	       }
	   }
	   return(0);
     }

     int	ResourceIndex(unsigned resId){
             for(int i = 0; i < numResources; i++){
		 if(resources[i].Identifier() == resId)
		    return(i);
	     }
	     return(-1);
     }

     int	MetricIndex(unsigned metId){
             for(int i = 0; i < numMetrics; i++){
		 if(metrics[i].Identifier() == metId)
		    return(i);
	     }
	     return(-1);
     }

     sampleType AggregateValue(int i,int j){
       if((i>=0)&&(i<numMetrics))
	 return(data_values[i].AggregateValue(j)); 
       else
	 return(ERROR);
     }

     sampleType  SumValue(int i,int j){
       if((i>=0)&&(i<numMetrics))
	 return(data_values[i].SumValue(j,binWidth)); 
       else
	 return(ERROR);
     }

     void  Fold(timeType width){
       int i;
       for(i=0; i < numMetrics; i++)
	 data_values[i].Fold();
       binWidth = width;
     }

     int AddValue(int metric, 
	          int resource, 
		  int bucket,
		  sampleType value){
	if((metric < 0) || (metric >= numMetrics))
	   return(ERROR_SUBSCRIPT);
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
	  return(ERROR_SUBSCRIPT);
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
