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

// $Id: templates-X.C,v 1.4 2002/12/20 07:50:09 jaw Exp $

#if !defined(rs6000_ibm_aix4_1)
ERROR: "templates-X is only for AIX"
#endif

#include "common/h/String.h"
#include "common/h/Symbol.h"
#pragma implementation "Dictionary.h"
#include "common/src/Dictionary.C"

template class dictionary_hash<string, Symbol>;

#ifndef external_templates
template class pdvector<string>;
template class pdvector<u_int>;
template class pdvector<Symbol>;
template class pdvector<dictionary_hash<string, Symbol>::entry>;
#endif

// visualizationUser stuff
// dummy client-side routines, since never actually used

#include "visi.xdr.CLNT.h" // auto-created by igen

void visualizationUser::GetPhaseInfo() {
   // visi asking us to list the currently defined phases
   cerr << "visualizationUser::GetPhaseInfo()" << endl;
   ////pdvector<T_visi::phase_info> phases;
   ////this->PhaseData(phases);
   assert(0);
}

void visualizationUser::GetMetricResource(string, int, int) {
   // visi asking us to instrument selected metric/focus pairs, and to
   // start sending that data to it.
   cerr << "visualizationUser::GetMetricResource(...)" << endl;
   assert(0);
}

void visualizationUser::StopMetricResource(u_int /*metricid*/, u_int /*focusid*/) {
   // visi asking us to disable data collection for m/f pair
   cerr << "visualizationUser::StopMetricResource(...)" << endl;
   assert(0);
}

void visualizationUser::StartPhase(double, string, bool, bool) {
   // visi asking us to start a new phase
   cerr << "visualizationUser::StartPhase(...)" << endl;
   assert(0);
}

void visualizationUser::showError(int code, string msg) {
   // visi asking us to show an error
   cerr << "visualizationUser::showError(code=" << code
        << ",msg=" << msg << ")" << endl;
}

