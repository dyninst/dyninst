/* $Log: visi.I,v $
/* Revision 1.2  1994/03/14 20:28:52  newhall
/* changed visi subdirectory structure
/*  */ 
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
  float data;
}dataValue;

$remote visualization {
  $base  7000;
  $version  1;
  $async void Data($array dataValue data);
  $async void Fold(float newBucketWidth);
  $async void InvalidMR(int m,int r);
  $async void AddMetricsResources($array metricType,$array resourceType,float,int);
  $async void NewMetricsResources($array metricType,$array resourceType);
  $async void Phase(float begin,float end,String name);
  $upcall $async void GetMetricResource(String metric,String resource,int type);
  $upcall $async void StopMetricResource(int metricId,int resourceId);
  $upcall $async void PhaseName(float begin,float end,String name);
};
