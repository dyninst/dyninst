/* $Id: VM.I,v 1.25 2004/06/21 19:36:34 pcroth Exp $ */

#include "paradyn/src/DMthread/DMinclude.h"
#include "common/h/Vector.h"
#include "paradyn/src/VMthread/VMinclude.h"

typedef struct VM_activeVisiInfo {
    int visiNum; // tid of active visi used to kill visi (VMDestroyVisi)
    int visiTypeId;  
    pdstring name;
} VM_activeVisiInfo;

$remote VM{

    $base 5000;
    $version 15;

    // returns a list of information about active visualizations
    // the client is responsible for freeing the space associated with
    // this list
    pdvector<VM_activeVisiInfo> *VMActiveVisis(); 


    // returns a list of information about available visualizations
    // the client is responsible for freeing the space associated with
    // this list
    pdvector<VM_visiInfo> *VMAvailableVisis(); 
  
    //  name: string representation of visi (for UIM menuing)
    //  args: command line arguments for the visualiztion, argv[0]
    //        is the executable name
    //  matrix: initial set of metrics and foci to start the visi with
    int VMAddNewVisualization(const char *name,
                            pdvector<pdstring> *argv,	
                            int forceProcessStart,
                            int mi_limit,
                            pdvector<pdstring> *matrix);


    //  converts a string representation of a list of met/focus pairs  
    //  to the internal list of metrespairs representation
    //  returns metrespair_Array values of count = 0, and data = NULL on error
    pdvector<metric_focus_pair> *VMStringToMetResPair(const char *metresString);


    // remenuFlag: if set, remenuing request made by visithread when 
    //             a set of metrics and resource choices can't be enabled
    // forceProcessStart: if set, the visi process is started w/o menuing first
    // visiTypeId: identifier indicating wch visi type to start
    // matrix: list of initial set of metrics/foci to start visi with
    $async void VMCreateVisi(int remenuFlag,  
		   int forceProcessStart,
		   int visiTypeId,
		   phaseType phase_type,
		   pdvector<metric_focus_pair> *matrix);
  
    // visiThreadId: thread identifier associated with the  visi to destroy 
    // can be called by any thread in the paradyn process
    $async void VMDestroyVisi(thread_t visiThreadId);

    // visiThreadId: thread identifier of visithread that has died
    // called by visithread when the visualization process exits
    void VMVisiDied(thread_t visiThreadId);

$signore
  
    static int VM_sequential_init();
    static int VM_post_thread_create_init( VMthreadArgs* targs );
    static VM *vmp;
$signore
};

