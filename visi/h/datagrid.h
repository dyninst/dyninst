/* $Log: datagrid.h,v $
/* Revision 1.2  1994/03/15 02:03:19  newhall
/* added public member "userdata" to class visi_GridCellHisto
/* to allow visi writer to add info to grid cells
/*
 * Revision 1.1  1994/03/14  20:27:26  newhall
 * changed visi subdirectory structure
 *  */ 
#ifndef _datagrid_h
#define _datagrid_h
#include <string.h>
#include  <math.h>
#include <values.h>
#include "error.h" 
#include "visi.h" 

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


class visi_GridCellHisto {
  private:
     float *value;
     int   valid;
     int   size;
  public: 
     void *userdata;  // to allow visi writer to add info to grid cells
     visi_GridCellHisto(){value = NULL; valid = 0; size = 0; userdata = NULL;}
     visi_GridCellHisto(int);
     ~visi_GridCellHisto(){delete[] value;}
     float  *Value(){ return(value);}
     float  Value(int i) { 
	    if((i < 0) || (i > size)){
	     return(ERROR);
            }
	    return(value[i]);
     }
     int    Size(){return(size);}
     int    Valid(){return(valid);}
     void   Invalidate(){delete[] value; value = NULL; size = 0; valid = 0;}
     void   Fold(int method){
       int i,j;
       if(valid){
         for(i=0,j=0;(i< (size/2)) && (j< (size-1)); i++,j+=2){
	   if((value[j] != ERROR) && (value[j+1] != ERROR))
             value[i] = value[j] + value[j+1];
           else
	     value[i] = ERROR;
	   if((method == AVE) && (value[i] != ERROR))
	     value[i] /= 2; 
	 }
	 for(i=(size/2); i < size; i++){
           value[i] = ERROR;
	 }
       }
     }
     float  AggregateValue(int method){
	int i,num;
	float sum;
        if(value != NULL){
           for(sum=0.0,num=i=0; i< size; i++){
	     if(value[i] != ERROR){
	       sum += value[i]; 
	       num++;
	     }
	   }
	   if(method==SUM)
	     return(sum);
           else if(num != 0){
	     return(sum/(1.0*num));
	   }
	   else{
	     visi_ErrorHandler(ERROR_AGGREGATE,"divide by zero");
	     return(ERROR_AGGREGATE);
           }
	}
	else{
	  visi_ErrorHandler(ERROR_AGGREGATE,"values == NULL");
	  return(ERROR_AGGREGATE);
	}
     }
     int    AddValue(float x,int i,int numElements){
       /* if this is the first value create a histo cell array */
       int j;
       if (!valid){
	 if(value == NULL)
	   value = new float[numElements];
	 size = numElements;
	 valid = 1;
	 for(j=0;j<size;j++){
	   value[j] = ERROR;
         }
       }
       if((i < 0) || (i >= size))
	 return(ERROR_SUBSCRIPT);
       value[i] = x;
       return(OK);
     }

     float  operator[](int i){
       if((i >= size) || (i < 0)){
	 visi_ErrorHandler(ERROR_SUBSCRIPT,"error in [] operator in histogridcell");
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
      int    AddValue(float x,int resource,int bucketNum,int numBuckets){
	if((resource < 0) || (resource >= size))
	   return(ERROR_SUBSCRIPT);
	return(values[resource].AddValue(x,bucketNum,numBuckets));
      }
      visi_GridCellHisto *Value(){return(values);}
      int    Valid(int);
      int    Invalidate(int);
      int    AddNewResources(int,int);
      void   Fold(int method){
        int i;
	for(i=0; i< size; i++)
	  values[i].Fold(method);
      } 
      float  AggregateValue(int i,int method){
	if((i>=0)&&(i<size))
	  return(values[i].AggregateValue(method));
        else
	  return(ERROR);
      }
      visi_GridCellHisto&   operator[](int i){
        if ((i>= 0) && (i < size)){
	  return(values[i]);
	}
	else{
	  visi_ErrorHandler(ERROR_SUBSCRIPT,"error in [] operator GridHistoArray");
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
     double      binWidth;
     visi_GridHistoArray  *data_values;
  public:
     visi_DataGrid(){metrics=NULL; resources=NULL; numMetrics=numResources=0;
		     data_values=NULL; numBins= 0; binWidth=0.0;}
     visi_DataGrid(int,int,Metric *,Resource *,int,double);
     visi_DataGrid(int,int,metricType *,resourceType *,int,double);
     virtual ~visi_DataGrid();
     char      *MetricName(int i);
     char      *MetricUnits(int i);
     char      *ResourceName(int j);
     char      *MetricList();
     char      *ObjectList();
     int        NumMetrics(){return(numMetrics);}
     int        FoldMethod(int);
     int        NumResources(){return(numResources);}
     int        MetricId(int); // returns metric Id
     int        ResourceId(int); // returns Resource Id
     int        NumBins(){return(numBins);}
     double     BinWidth(){return(binWidth);}
     int        Valid(int,int);
     int        Invalidate(int,int);
     float      AggregateValue(int i,int j){
       if((i>=0)&&(i<numMetrics))
	 return(data_values[i].AggregateValue(j,metrics[i].Aggregate())); 
       else
	 return(ERROR);
     }
     void  Fold(double width){
       int i;
       for(i=0; i < numMetrics; i++)
	 data_values[i].Fold(metrics[i].Aggregate());
       binWidth = width;
     }
     int  AddValue(int metric, int resource, int bucket,float value){
	if((metric < 0) || (metric >= numMetrics))
	  return(ERROR_SUBSCRIPT);
	return(data_values[metric].AddValue(value,resource,bucket,numBins));
     }
     visi_GridHistoArray&  operator[](int i){
       if((i < 0) || (i >= numMetrics)){
	 visi_ErrorHandler(ERROR_SUBSCRIPT,"error in [] operator DATAGRID");
	 return(data_values[0]);
       }
       return(data_values[i]);
     }

};
#endif
