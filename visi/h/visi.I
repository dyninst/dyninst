/* $Log: visi.I,v $
/* Revision 1.2  1994/04/13 21:23:09  newhall
/* *** empty log message ***
/*
# Revision 1.1  1994/03/29  02:42:13  newhall
# exported services of the visualization process
#
# Revision 1.3  1994/03/17  05:20:35  newhall
# changed bucket width and time value's type to double
#
# Revision 1.2  1994/03/14  20:28:52  newhall
# changed visi subdirectory structure
#  */ 

$remote typedef struct {
     String  units;    // how units are measured  i.e. "ms" 
     String  name;     // for y-axis labeling  
     int     Id;       // unique metric Id
     int     aggregate;  //either SUM or AVE
}metricType;


$remote typedef struct{
     String name;     // obj. name for graph labeling
     int    Id;       // unique resource id
}resourceType;

$remote typedef struct{
  int 	metricId;
  int 	resourceId;
  int 	bucketNum;
  double data;
}dataValue;


$remote visualization {
  $base  7000;
  $version  1;
  $async void Data($array dataValue data);
  $async void Fold(double newBucketWidth);
  $async void InvalidMR(int m,int r);
  $async void AddMetricsResources($array metricType,$array resourceType,double,int);
  $async void NewMetricsResources($array metricType,$array resourceType);
  $async void Phase(double begin,double end,String name);
  $upcall $async void GetMetricResource(String metric,String resource,int type);
  $upcall $async void StopMetricResource(int metricId,int resourceId);
  $upcall $async void PhaseName(double begin,double end,String name);
};
