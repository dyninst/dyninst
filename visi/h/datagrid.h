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
/* Revision 1.9  1994/06/16 18:21:46  newhall
/* bug fix to AddNewValues
/*
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

#define SUM	0
#define AVE     1

class Metric{
     char       *units;    // how units are measured  i.e. "ms" 
     char       *name;     // for y-axis labeling  
     int         Id;       // unique metric Id
     int         aggregate; //either SUM or AVE, for fold operation 
  public:
     Metric(){units = NULL; name = NULL; Id = NOVALUE; aggregate=SUM;}
     Metric(char* ,char*,int,int); 
     ~Metric(){delete[] units;delete[] name;} 
     char       *Units(){return(units);}
     char       *Name(){return(name);}
     int         Identifier(){return(Id);}
     int         Aggregate(){return(aggregate);}
};


class Resource{
     char    *name;     // obj. name for graph labeling
     int      Id;       // unique resource id
   public:
     Resource(){name = NULL; Id = -1;}
     Resource(char*,int);
     ~Resource(){ delete[] name;} 
     char     *Name(){return(name);}
     int       Identifier(){return(Id);}
};


///////////////////////////////////////////////
// visi_GridCellHisto:  A grid cell element 
// (histogram) is an instance of this class
// valid: indicates that the cell contains a histogram
// deleted: indicates that the cell cannot accept new data values
// size: number of buckets
// lastBucketFilled: number of full buckets 
////////////////////////////////////////////

class visi_GridCellHisto {
  private:
     int   valid;
     int   size;
     int   lastBucketFilled;  // bucket number of last data value added   
     int   deleted;  // set on delete cell element, cleared on add new element
     sampleType *value;
  public: 
     void *userdata;  // to allow visi writer to add info to grid cells
     visi_GridCellHisto(){value = NULL; valid = 0; size = 0; 
			  userdata = NULL; lastBucketFilled = -1; deleted = 0;}
     visi_GridCellHisto(int);
     ~visi_GridCellHisto(){delete[] value;}
     int    LastBucketFilled(){return(lastBucketFilled);}
     sampleType  *Value(){ return(value);}

     sampleType  Value(int i) { 
	    if((i < 0) || (i > size)){
	     return(ERROR);
            }
	    return(value[i]);
     }

     int    Size(){return(size);}
     int    Valid(){return(valid);}
     int    Deleted(){return(deleted);}
     void   SetDeleted(){deleted = 1;}
     void   ClearDeleted(){deleted = 0;}
     void   Invalidate(){delete[] value; value = NULL; size = 0; 
			 valid = 0; lastBucketFilled = -1;}

     int    AddNewValues(sampleType *temp,
			 int arraySize,
			 int lbf,
			 void *ud,
			 int v, 
			 int d){
        
	if(temp == NULL){
          value = NULL;
	  size = 0;
	}
	else{
	  // initialize cell to temp values
	  value = new sampleType[arraySize];
	  size = arraySize;
	  for(int i=0;i<size;i++){
	   value[i] = temp[i]; 
	  }
	}
	lastBucketFilled = lbf;
	userdata = ud;
	valid = v;
	deleted = d;
	return(OK);
     }

     void   Fold(int method){
       int i,j;
       if(valid){
         for(i=0,j=0;(i< (lastBucketFilled+1)/2) // new bucket counter
	     && (j< (lastBucketFilled+1)); // old bucket counter
	     i++,j+=2){
	   if((value[j] != ERROR) && (value[j+1] != ERROR))
             value[i] = value[j] + value[j+1];
           else
	     value[i] = ERROR;
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

     sampleType  AggregateValue(int method){
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
	  visi_ErrorHandler(ERROR_AGGREGATE,"values == NULL");
	  return(ERROR_AGGREGATE);
	}
     }

     int    AddValue(sampleType x,
		     int i,
		     int numElements){
        
       int j;

       if (deleted){  // if deleted is set, don't add values
         return(OK);
       }
       if (!valid){ // if this is the first value create a histo cell array 
	 if(value == NULL)
	   value = new sampleType[numElements];
	 size = numElements;
	 valid = 1;
	 deleted = 0;
	 for(j=0;j<size;j++){
	   value[j] = ERROR;
         }
       }
       if((i < 0) || (i >= size))
	 return(ERROR_SUBSCRIPT);
       value[i] = x;
       if(i > lastBucketFilled)
        lastBucketFilled = i;
       return(OK);
     }

     sampleType  operator[](int i){
       if((i >= size) || (i < 0)){
	 visi_ErrorHandler(ERROR_SUBSCRIPT,
			   "error in [] operator in histogridcell");
	 return(ERROR);
       }
       return(value[i]);
     }
};


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
      void   ClearDeleted(){
        for(int i = 0; i < size; i++){
	  values[i].ClearDeleted();
	}
      }

      void   Fold(int method){
        int i;
	for(i=0; i< size; i++)
	  values[i].Fold(method);
      } 

      sampleType  AggregateValue(int i,
			     int method){
	if((i>=0)&&(i<size))
	  return(values[i].AggregateValue(method));
        else
	  return(ERROR);
      }
      sampleType  SumValue(int i,timeType width){
	if((i>=0)&&(i<size))
	  return(values[i].SumValue(width));
        else
	  return(ERROR);
      }

      visi_GridCellHisto&   operator[](int i){
        if ((i>= 0) && (i < size)){
	  return(values[i]);
	}
	else{
	  visi_ErrorHandler(ERROR_SUBSCRIPT,
			    "error in [] operator GridHistoArray");
	  return(values[0]);
	}
      }
};


class visi_DataGrid {
 protected:
     Metric     *metrics;
     Resource   *resources;
     int         numMetrics;
     int         numResources;
     int         numBins;
     timeType    binWidth;
     visi_GridHistoArray  *data_values;
  public:
     visi_DataGrid(){
	 metrics=NULL; 
	 resources=NULL; 
	 numMetrics=numResources=0;
	 data_values=NULL; 
	 numBins= 0; 
	 binWidth=0.0;
     }

     visi_DataGrid(int,int,Metric *,Resource *,int,timeType);
     visi_DataGrid(int,int,visi_metricType *,visi_resourceType *,int,timeType);
     virtual ~visi_DataGrid();
     char      *MetricName(int i);
     char      *MetricUnits(int i);
     char      *ResourceName(int j);
     int        NumMetrics(){return(numMetrics);}
     int        FoldMethod(int);
     int        NumResources(){return(numResources);}
     int        MetricId(int); // returns metric Id
     int        ResourceId(int); // returns Resource Id
     int        NumBins(){return(numBins);}
     timeType   BinWidth(){return(binWidth);}
     int        Valid(int,int);  
     int        Invalidate(int,int);
     int        AddNewMetrics(int,visi_metricType *);
     int        AddNewResource(int,visi_resourceType *);
     int        ResourceInGrid(int);
     int        MetricInGrid(int);
     void       ClearDeleted(){
           for(int i = 0; i < numMetrics; i++){
               data_values[i].ClearDeleted();
	   }
     }

     sampleType AggregateValue(int i,int j){
       if((i>=0)&&(i<numMetrics))
	 return(data_values[i].AggregateValue(j,metrics[i].Aggregate())); 
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
	 data_values[i].Fold(metrics[i].Aggregate());
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
 	   visi_ErrorHandler(ERROR_SUBSCRIPT,
			   "error in [] operator DATAGRID");
 	   return(data_values[0]);
        }
        return(data_values[i]);
     }

     int LastBucketFilled(int metric,int resource){
        if((metric < 0) || (metric >= numMetrics))
	  return(ERROR_SUBSCRIPT);
        return(data_values[metric].LastBucketFilled(resource));
     }

};
#endif
