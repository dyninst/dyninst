/* $Log: VM.I,v $
/* Revision 1.15  1996/01/05 20:00:27  newhall
/* removed warnings
/*
# Revision 1.14  1995/08/12  22:27:12  newhall
# added VM_sequential_init(), VM_post_thread_create_init(), & vmp members
#
# Revision 1.13  1995/08/01  02:15:27  newhall
# added phase type parameter to VMCreateVisi
#
# Revision 1.12  1995/06/02  20:58:44  newhall
# made code compatable with new datamanager interface
# replaced List and HTable container classes with STL containers
#
# Revision 1.10  1995/02/26  02:11:52  newhall
# added some phase interface routines
#
# Revision 1.9  1995/02/16  08:01:39  markc
# Modified interfaces:
#   1) XDR interfaces use strings and vectors instead of igen arrays and char*'s
#   2) No sync upcalls are allowed by the server in xdr interfaces
#   3) Redefined igen arrays that were used in thread code to use manually defined
#      structures rather than igen-defined, since igen did not need to know about
#      these structures.
#   4) Upped all of the version numbers since all new interface code is generated
#   5) Async calls are no longer handled while waiting for sync responses, these
#      requests are buffered.
#
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

#include "paradyn/src/DMthread/DMinclude.h"
#include "util/h/Vector.h"

typedef struct VM_activeVisiInfo {
  int visiNum; // tid of active visi used to kill visi (VMDestroyVisi)
  int visiTypeId;  
  string name;
} VM_activeVisiInfo;

typedef struct VM_visiInfo {
  int visiTypeId;  // id of visi used to start visi (VMCreateVisi)
  string name;
} VM_visiInfo;

$remote VM{

  $base 5000;
  $version 11;

  // returns a list of information about active visualizations
  // the client is responsible for freeing the space associated with
  // this list
  vector<VM_activeVisiInfo> *VMActiveVisis(); 


  // returns a list of information about available visualizations
  // the client is responsible for freeing the space associated with
  // this list
  vector<VM_visiInfo> *VMAvailableVisis(); 
  
  //  name: string representation of visi (for UIM menuing)
  //  args: command line arguments for the visualiztion, argv[0]
  //        is the executable name
  //  matrix: initial set of metrics and foci to start the visi with
  int VMAddNewVisualization(const char *name,
		            vector<string> *argv,	
			    int forceProcessStart,
			    char *matrix,
			    int numMatrices);


  //  converts a string representation of a list of met/focus pairs  
  //  to the internal list of metrespairs representation
  //  returns metrespair_Array values of count = 0, and data = NULL on error
  vector<metric_focus_pair> *VMStringToMetResPair(const char *metresString);


  // remenuFlag: if set, remenuing request made by visithread when 
  //             a set of metrics and resource choices can't be enabled
  // forceProcessStart: if set, the visi process is started w/o menuing first
  // visiTypeId: identifier indicating wch visi type to start
  // matrix: list of initial set of metrics/foci to start visi with
  int VMCreateVisi(int remenuFlag,  
		   int forceProcessStart,
		   int visiTypeId,
		   phaseType phase_type,
		   vector<metric_focus_pair> *matrix);
  
  // visiThreadId: thread identifier associated with the  visi to destroy 
  // can be called by any thread in the paradyn process
  $async void VMDestroyVisi(thread_t visiThreadId);

  // visiThreadId: thread identifier of visithread that has died
  // called by visithread when the visualization process exits
  void VMVisiDied(thread_t visiThreadId);

  $signore
  
  static int VM_sequential_init();
  static int VM_post_thread_create_init();
  static VM *vmp;
  $signore
};

