/*
 * Put all the templates in one file
 *
 */

/*
 * $Log: templates.C,v $
 * Revision 1.8  1995/02/27 18:40:05  tamches
 * Minor changes to reflect new TCthread (tunable constant header
 * files have moved)
 *
 * Revision 1.7  1995/02/26  02:27:51  newhall
 * added source file DMphase.C
 *
 * Revision 1.6  1995/02/16  08:05:27  markc
 * Added missing template instantiation requests.
 * Changed template instantiation requests to the correct form.
 *
 * Revision 1.5  1995/01/26  17:57:00  jcargill
 * Changed igen-generated include files to new naming convention; fixed
 * some bugs compiling with gcc-2.6.3.
 *
 * Revision 1.4  1994/11/01  05:45:17  karavan
 * UIthread changes to implement multiple focus selection on a single display
 *
 * Revision 1.3  1994/10/25  17:56:11  karavan
 * added resource Display Objects for multiple abstractions to UIthread code
 *
 * Revision 1.2  1994/10/09  01:29:13  karavan
 * added UIM templates connected with change to new UIM/visiThread metric-res
 * selection interface.
 *
 * Revision 1.1  1994/09/22  00:49:12  markc
 * Paradyn now uses one template code generating file.  All threads should use
 * this file to generate template code.
 *
 * Revision 1.2  1994/08/22  15:59:31  markc
 * Add List<daemonEntry*> which is the daemon definition dictionary.
 *
 * Revision 1.1  1994/05/19  00:02:18  hollings
 * added templates.
 *
 *
 */
#pragma implementation "list.h"
#include "util/h/list.h"

#include "util/h/String.h"
//#include "util/h/tunableConst.h"

// Igen includes
#pragma implementation "Vector.h"
#include "util/h/Vector.h"
#pragma implementation "Queue.h"
#include "util/h/Queue.h"

#pragma implementation "Dictionary.h"
#include "util/h/Dictionary.h"

#pragma implementation "dyninstRPC.xdr.h"
#include "dyninstRPC.xdr.h"

#pragma implementation "visi.xdr.h"
#include "visi.xdr.h"


/* *********************************   
 * DMthread stuff
 */

#include "dataManager.thread.h"
#include "dyninstRPC.xdr.CLNT.h"
#include "paradyn/src/DMthread/DMinternals.h"
class uniqueName;

//template class List<tunableConstant*>; --AT 2/95

template class List<uniqueName*>;
template class List<executable *>;
template class List<paradynDaemon *>;
template class List<performanceStream *>;
template class List<daemonEntry*>;
template class List<component*>;
template class HTable<metric*>;
template class List<sampleInfo*>;
template class HTable<resource*>;
template class HTable<abstraction*>;
template class dictionary_hash<unsigned, metricInstance*>;
template class vector<unsigned>;
template class vector<phaseInfo *>;
template class vector<performanceStream *>;


/* ********************************
 * PCthread stuff
 */
#include "paradyn/src/PCthread/PCshg.h"
#include "paradyn/src/PCthread/PCevalTest.h"
#include "paradyn/src/PCthread/PCglobals.h"
#include "paradyn/src/PCthread/PCauto.h"
#include "paradyn/src/PCthread/PCwhen.h"
#include "paradyn/src/PCthread/PCwhere.h"
#include "paradyn/src/PCthread/PCwhy.h"

template class List<focus *>;
template class List<focusList *>;
template class List<metricInstance *>;

template class HTable<PCmetric *>;
template class HTable<datum *>;
template class HTable<focus *>;

template class List<datum *>;
template class List<hint *>;
template class List<hypothesis *>;
template class List<searchHistoryNode *>;
template class List<test *>;
template class List<testResult *>;
template class List<timeInterval *>;

/* *************************************
 * UIthread stuff
 */
#include "paradyn/src/VMthread/metrespair.h"
#include "../src/UIthread/UIglobals.h"
class resourceList;
class pRec;

template class HTable<pRec *>;
template class List<resourceDisplayObj *>;
template class List<resourceList *>;
template class List<metrespair *>;
template class List<tokenRec *>;
template class List<stringHandle>;
template class List<dag *>;
template class List<resource **>;

/* ************************************
 * VMthread stuff
 */
#include "paradyn/src/VMthread/VMtypes.h"

template class List<VMactiveStruct *>;
template class List<VMvisisStruct *>;

/* ***********************************
 * met stuff
 */
#include "paradyn/src/met/metParse.h" 

class stringList;
class daemonMet;
class processMet;
class visiMet;
class tunableMet;

template class List<stringList*>;
template class List<daemonMet*>;
template class List<processMet*>;
template class List<visiMet*>;
template class List<tunableMet*>;
template class List<char*>;

// Igen - dyninstRPC stuff

template class queue<T_dyninstRPC::buf_struct*>;
template class vector<string>;
template class vector<T_dyninstRPC::metricInfo>;
template bool_t T_dyninstRPC_P_xdr_stl(XDR*, vector<string>*, bool_t (*)(XDR*, string*), string*);
template bool_t T_dyninstRPC_P_xdr_stl(XDR*, vector<T_dyninstRPC::metricInfo>*, bool_t (*)(XDR*, T_dyninstRPC::metricInfo*), T_dyninstRPC::metricInfo*);
template bool_t T_dyninstRPC_P_xdr_stl_PTR(XDR*, vector<string>**, bool_t (*)(XDR*, string*), string*);
template bool_t T_dyninstRPC_P_xdr_stl_PTR(XDR*, vector<T_dyninstRPC::metricInfo>**, bool_t (*)(XDR*, T_dyninstRPC::metricInfo*), T_dyninstRPC::metricInfo*);

// Igen - visi stuff

template class queue<T_visi::buf_struct*>;
template class vector<T_visi::dataValue>;
template class vector<T_visi::visi_matrix>;
template class vector<T_visi::phase_info>;
template class vector<float>;
template bool_t T_visi_P_xdr_stl(XDR*, vector<string>*, bool_t (*)(XDR*, string*), string*);
template bool_t T_visi_P_xdr_stl(XDR*, vector<T_visi::dataValue>*, bool_t (*)(XDR*, T_visi::dataValue*), T_visi::dataValue*);
template bool_t T_visi_P_xdr_stl(XDR*, vector<T_visi::visi_matrix>*, bool_t (*)(XDR*, T_visi::visi_matrix*), T_visi::visi_matrix*);
template bool_t T_visi_P_xdr_stl(XDR*, vector<float>*, bool_t (*)(XDR*, float*), float*);
template bool_t T_visi_P_xdr_stl(XDR*, vector<T_visi::phase_info>*, bool_t (*)(XDR*, T_visi::phase_info*), T_visi::phase_info*);
template bool_t T_visi_P_xdr_stl_PTR(XDR*, vector<string>**, bool_t (*)(XDR*, string*), string*);
template bool_t T_visi_P_xdr_stl_PTR(XDR*, vector<T_visi::dataValue>**, bool_t (*)(XDR*, T_visi::dataValue*), T_visi::dataValue*);
template bool_t T_visi_P_xdr_stl_PTR(XDR*, vector<T_visi::visi_matrix>**, bool_t (*)(XDR*, T_visi::visi_matrix*), T_visi::visi_matrix*);
template bool_t T_visi_P_xdr_stl_PTR(XDR*, vector<float>**, bool_t (*)(XDR*, float*), float*);
template bool_t T_visi_P_xdr_stl_PTR(XDR*, vector<T_visi::phase_info>**, bool_t (*)(XDR*, T_visi::phase_info*), T_visi::phase_info*);
