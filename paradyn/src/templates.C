/*
 * Put all the templates in one file
 *
 */

/*
 * $Log: templates.C,v $
 * Revision 1.3  1994/10/25 17:56:11  karavan
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


/* *********************************   
 * DMthread stuff
 */

#include "dataManager.h"
#include "dyninstRPC.CLNT.h"
#include "paradyn/src/DMthread/DMinternals.h"
#include "util/h/tunableConst.h"
class uniqueName;

typedef List<uniqueName*> t0;
typedef List<executable *> t1;
typedef List<paradynDaemon *> t2;
typedef List<performanceStream *> t3;
typedef List<daemonEntry*> t4;

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

// typedef HTable<metricInstance *> p3;
// typedef List<focus *> p5;
// typedef List<focusList *> p6;
// typedef List<metricInstance *> p9;

typedef HTable<PCmetric *> p0;
typedef HTable<datum *> p1;
typedef HTable<focus *> p2;

typedef List<datum *> p4;
typedef List<hint *> p7;
typedef List<hypothesis *> p8;
typedef List<searchHistoryNode *> p10;
typedef List<test *> p11;
typedef List<testResult *> p12;
typedef List<timeInterval *> p13;

/* *************************************
 * UIthread stuff
 */
#include "paradyn/src/VMthread/metrespair.h"
#include "../src/UIthread/UIglobals.h"
class resourceList;
class pRec;

typedef HTable<pRec *> h1; 
typedef List<resourceDisplayObj *> h2;
typedef List<resourceList *> h3;
typedef List<metrespair *> h4;
typedef List<tokenRec *> h5;
typedef List<stringHandle> h6;
typedef List<dag *> h7;

/* ************************************
 * VMthread stuff
 */
#include "paradyn/src/VMthread/VMtypes.h"

typedef List<VMactiveStruct *> v1;
typedef List<VMvisisStruct *> v2;

/* ***********************************
 * met stuff
 */
#include "paradyn/src/met/metParse.h" 

class stringList;
class daemonMet;
class processMet;
class visiMet;
class tunableMet;

typedef List<stringList*> m1;
typedef List<daemonMet*> m2;
typedef List<processMet*> m3;
typedef List<visiMet*> m4;
typedef List<tunableMet*> m5; 
