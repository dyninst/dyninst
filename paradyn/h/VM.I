/* $Log: VM.I,v $
/* Revision 1.3  1994/06/16 18:28:57  newhall
/* *** empty log message ***
/*
# Revision 1.2  1994/04/09  21:26:29  newhall
# change VMDestroyVisi to  async
#
# Revision 1.1  1994/03/30  05:42:34  newhall
# exported services of the Visualization Manager thread
# */

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
  int VMAddNewVisualization(char *name,
			    int argc,
			    char **argv);
  int VMCreateVisi(int visiTypeId);  // return OK on success
  $async void VMDestroyVisi(int visiThreadId);
  void VMVisiDied(int visiThreadId); // from visithread 
};
