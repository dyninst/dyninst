/*
 * Copyright (c) 1996 Barton P. Miller
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

// $Id: DMinclude.h,v 1.18 2001/11/02 16:05:20 pcroth Exp $

#ifndef dminclude_H
#define dminclude_H

// this should be the only DM header file included by other modules

#include "common/h/String.h"
// trace data streams
#include "pdutil/h/ByteArray.h"

#include "common/h/Vector.h"
// this is an upper limit for data buffering between DM and client threads
// and between VISIthread and visis
#define DM_DATABUF_LIMIT	50	

const unsigned LIB_CONSTRAINT_REGEX_FLAG = 1;
const unsigned LIB_CONSTRAINT_NOCASE_FLAG = 2;

typedef unsigned metricHandle;
typedef unsigned resourceListHandle;
typedef unsigned resourceHandle;
typedef unsigned perfStreamHandle;
typedef unsigned metricInstanceHandle;
typedef unsigned phaseHandle;

typedef enum {OriginalSearch, CallGraphSearch} magnifyType;
typedef enum {GlobalPhase, CurrentPhase} phaseType;
typedef enum {UnNormalized, Normalized, Sampled} dm_MetUnitsType;

struct metfocusType {
    vector<resourceHandle> res;
    metricHandle  met;

    metfocusType(metricHandle iMet,
		 const vector<resourceHandle> &iRes) :
		   res(iRes), met(iMet) {}
    metfocusType() {}
};
typedef struct metfocusType metric_focus_pair;

struct metRLType {
    metricHandle met;
    resourceHandle res;

    metRLType(metricHandle mh,resourceHandle rh): met(mh), res(rh) {} 
    metRLType(){}
};
typedef struct metRLType metricRLType;

struct miInfoType{
    bool successfully_enabled;  // true if this metric/focus pair was enabled
    metricInstanceHandle mi_id;
    metricHandle m_id;
    resourceListHandle r_id;
    string metric_name;
    string metric_units;
    string focus_name;
    dm_MetUnitsType units_type;
    miInfoType(){
        successfully_enabled = 0; mi_id = 0; m_id=0; r_id=0; 
	units_type=Normalized;
    }
};
typedef struct miInfoType metricInstInfo;

struct metNameIdType {
    metricHandle id;
    string       name;
};
typedef struct metNameIdType met_name_id;

struct rlNameIdType {
    resourceListHandle id;
    const char *res_name;	
};
typedef struct rlNameIdType rlNameId;

// client side of termWin igen interface
class termWinUser;
extern termWinUser* twUser;

#endif
