/* $Log: VM.I,v $
/* Revision 1.7  1994/09/25 01:49:03  newhall
/* UI and VM interface changes to support new representation
/* of metric/focus lists as a list of metric/focus pairs (either
/* as a string or a list of metric* resourceList* pairs).
/*
# Revision 1.6  1994/09/22  00:42:31  markc
# Changed "String" to "char*"
# Declared $array types explicitly
# Incremented version number
#
# Revision 1.5  1994/08/17  17:55:25  markc
# Incremented version numbers because a new version of igen has been
# committed.
#
# Revision 1.4  1994/08/13  20:49:57  newhall
# changed parameters to VMCreateVisi
#
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
  char *name;
}VM_activeVisiInfo;

$remote typedef struct {
  int visiTypeId;  // id of visi used to start visi (VMCreateVisi)
  char *name;
}VM_visiInfo;

$remote array VM_activeVisiInfo VM_activeVisiInfo_Array;
$remote array VM_visiInfo VM_visiInfo_Array;

$remote VM{

  $base 5000;
  $version 5;

  VM_activeVisiInfo_Array VMActiveVisis(); 
  VM_visiInfo_Array VMAvailableVisis(); 
  
  //  name: string representation of visi, for UIM menuing
  //  args: command line arguments for the visualiztion, argv[0]
  //        is the executable name
  //  matrix: initial set of metrics and foci to start the visi with
  int VMAddNewVisualization(char *name,
			    int argc,
			    char **argv,
			    int forceProcessStart,
			    char *matrix,
			    int numMatrices);

  // remenuFlag: if set, remenuing request made by visithread when 
  //             a set of metrics and resource choices can't be enabled
  // forceProcessStart: if set, the visi process is started w/o menuing first
  // visiTypeId: identifier indicating wch visi type to start
  // matrix: list of initial set of metrics/foci to start visi with
  int VMCreateVisi(int remenuFlag,  
		   int forceProcessStart,
		   int visiTypeId,
		   char *matrix,
		   int numMatrices);
  
  // visiThreadId: thread identifier associated with the  visi to destroy 
  $async void VMDestroyVisi(int visiThreadId);

  // visiThreadId: thread identifier of visithread that has died
  void VMVisiDied(int visiThreadId); // from visithread 
};
