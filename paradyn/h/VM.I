/* $Log: VM.I,v $
/* Revision 1.9  1995/02/16 08:01:39  markc
/* Modified interfaces:
/*   1) XDR interfaces use strings and vectors instead of igen arrays and char*'s
/*   2) No sync upcalls are allowed by the server in xdr interfaces
/*   3) Redefined igen arrays that were used in thread code to use manually defined
/*      structures rather than igen-defined, since igen did not need to know about
/*      these structures.
/*   4) Upped all of the version numbers since all new interface code is generated
/*   5) Async calls are no longer handled while waiting for sync responses, these
/*      requests are buffered.
/*
# Revision 1.8  1994/09/30  21:19:32  newhall
# added interface function VMStringToMetResPair
# changed parameters to VMCreateVisi to take list of metrespair
#
# Revision 1.7  1994/09/25  01:49:03  newhall
# UI and VM interface changes to support new representation
# of metric/focus lists as a list of metric/focus pairs (either
# as a string or a list of metric* resourceList* pairs).
#
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

#include "../src/VMthread/metrespair.h"

typedef struct VM_activeVisiInfo {
  int visiNum; // tid of active visi used to kill visi (VMDestroyVisi)
  int visiTypeId;  
  char *name;
} VM_activeVisiInfo;

typedef struct VM_activeVisiInfo_Array {
  int count;
  VM_activeVisiInfo *data;
} VM_activeVisiInfo_Array;

typedef struct VM_visiInfo {
  int visiTypeId;  // id of visi used to start visi (VMCreateVisi)
  char *name;
} VM_visiInfo;

typedef struct VM_visiInfo_Array {
  int count;
  VM_visiInfo *data;
} VM_visiInfo_Array;

typedef struct metrespair_Array {
  int count;
  metrespair *data;
} metrespair_Array;

$remote VM{

  $base 5000;
  $version 6;

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


//  converts a string representation of a list of met/focus pairs  
//  to the internal list of metrespairs representation
//  returns metrespair_Array values of count = 0, and data = NULL on error
metrespair_Array  VMStringToMetResPair(char *metresString);


  // remenuFlag: if set, remenuing request made by visithread when 
  //             a set of metrics and resource choices can't be enabled
  // forceProcessStart: if set, the visi process is started w/o menuing first
  // visiTypeId: identifier indicating wch visi type to start
  // matrix: list of initial set of metrics/foci to start visi with
  int VMCreateVisi(int remenuFlag,  
		   int forceProcessStart,
		   int visiTypeId,
		   metrespair *matrix,
		   int numMatrices);
  
  // visiThreadId: thread identifier associated with the  visi to destroy 
  $async void VMDestroyVisi(int visiThreadId);

  // visiThreadId: thread identifier of visithread that has died
  void VMVisiDied(int visiThreadId); // from visithread 
};

