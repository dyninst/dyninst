/*
 * Copyright (c) 1996-1999 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

/*
 * PCmetric.h
 * 
 * The PCmetric class and the PCmetricInst class.
 * 
 * $Log: PCmetric.h,v $
 * Revision 1.24  1999/08/09 05:40:36  csserra
 * - added support for (mips-sgi-irix6.4) native compiler build
 * - eliminated misc. compiler warnings
 *
 * Revision 1.23  1999/03/03 18:15:15  pcroth
 * Updated to support Windows NT as a front-end platform
 * Changes made to X code, to use Tcl analogues when appropriate
 * Also changed in response to modifications in thread library and igen output.
 *
 * Revision 1.22  1996/08/16 21:03:33  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.21  1996/04/30 06:26:59  karavan
 * change PC pause function so cost-related metric instances aren't disabled
 * if another phase is running.
 *
 * fixed bug in search node activation code.
 *
 * added change to treat activeProcesses metric differently in all PCmetrics
 * in which it is used; checks for refinement along process hierarchy and
 * if there is one, uses value "1" instead of enabling activeProcesses metric.
 *
 * changed costTracker:  we now use min of active Processes and number of
 * cpus, instead of just number of cpus; also now we average only across
 * time intervals rather than cumulative average.
 *
 * Revision 1.20  1996/02/02 02:07:30  karavan
 * A baby Performance Consultant is born!
 *
 */

#ifndef pc_metric_h
#define pc_metric_h

#include "PCintern.h"
#include "PCfilter.h"
//sys.h defines the following:
//  typedef double timeStamp;
//  typedef float sampleValue;
//  struct Interval {
//     timeStamp start;
//     timeStamp end;
//      sampleValue value;
//  };

typedef enum {derived, simple} PCmetricType;
typedef metricInstanceHandle PCmetDataID;
typedef bool (*initPCmetricInstFunc)(focus foo);
typedef sampleValue (*evalPCmetricInstFunc)(focus foo, sampleValue *data,
					    int dataSize); 
typedef enum {cf, tlf} focusType;
struct metNameFocusStruct {
  const char *mname;
  focusType whichFocus;
  filterType ft;
};
typedef struct metNameFocusStruct metNameFocus;

struct PCMetInfoStruct {
  metricHandle mh;
  focusType fType;
  filterType ft;
};
typedef struct PCMetInfoStruct PCMetInfo;

class PCmetric;

class PCmetric {
  friend class PCmetricInst;
public:
  static unsigned pauseTimeMetricHandle;
  // (future) user-defined derived metric
  PCmetric (const char *thisName, metNameFocus *dataSpecs, int dataSpecsSize, 
	    initPCmetricInstFunc setupFun, evalPCmetricInstFunc calcFun,
	    bool withPause);
  // wrapper for dm metrics
  PCmetric (char *DMmetName, focusType ftype, bool *success); 
  const char *getName() {return metName.string_of();}
  static dictionary_hash<string, PCmetric*> AllPCmetrics;
  metricHandle getMh () {return mh;}
private:
  string metName;
  bool InstWithPause;
  vector <PCMetInfo*> *DMmetrics; 
  metricHandle mh;
  focusType ft;
  initPCmetricInstFunc setup;
  evalPCmetricInstFunc calc;
};


#endif


