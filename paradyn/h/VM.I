/* $Log: VM.I,v $
/* Revision 1.1  1994/03/30 05:42:34  newhall
/* exported services of the Visualization Manager thread
/* */
$remote typedef struct {
  int visiNum; // tid of active visi used to kill visi (VMDestroyVisi)
  int visiTypeId;  
  String name;
}VM_activeVisiInfo;

$remote typedef struct {
  int visiTypeId;  // id of visi used to start visi (VMCreateVisi)
  String name;
}VM_visiInfo;


$remote VM{

  $base 5000;
  $version 1;

// for comm. with PC and UIM 
  $array VM_activeVisiInfo VMActiveVisis(); 
  $array VM_visiInfo VMAvailableVisis(); 
  int VMAddNewVisualization(char *name,int argc,char **argv);
  int VMCreateVisi(int visiTypeId);  // return OK on success
  int VMDestroyVisi(int visiThreadId);  // return OK on success
  void VMVisiDied(int visiThreadId); // from visithread 
};
