/* $Log: VM.I,v $
/* Revision 1.4  1994/08/13 20:49:57  newhall
/* changed parameters to VMCreateVisi
/*
# Revision 1.3  1994/06/16  18:28:57  newhall
# *** empty log message ***
#
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
  $version 2;

  // for comm. with PC and UIM 
  $array VM_activeVisiInfo VMActiveVisis(); 
  $array VM_visiInfo VMAvailableVisis(); 
  
  //  name: string representation of visi, for UIM menuing
  //  args: command line arguments for the visualiztion, argv[0]
  //        is the executable name
  int VMAddNewVisualization(char *name,
			    int argc,
			    char **argv);

  // remenuFlag: if set, remenuing request made by visithread when 
  //             a set of metrics and resource choices can't be enabled
  // visiTypeId: identifier indicating wch visi type to start
  // metList, resList:   a string representation of an initail set of metrics
  //                     and/or resources to start the visi with
  int VMCreateVisi(int remenuFlag,  
		   int visiTypeId,
		   char *metList,
		   char *resList);  // return OK on success
  
  // visiThreadId: thread identifier associated with the  visi to destroy 
  $async void VMDestroyVisi(int visiThreadId);

  // visiThreadId: thread identifier of visithread that has died
  void VMVisiDied(int visiThreadId); // from visithread 
};
