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

// templates.C
// for table visi

/*
 * $Log: templates.C,v $
 * Revision 1.5  1996/08/16 21:37:02  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.4  1995/12/22 22:37:43  tamches
 * 2 new instantiations
 *
 * Revision 1.3  1995/11/29 00:43:56  tamches
 * added lots of templates needed for new pdLogo stuff
 *
 * Revision 1.2  1995/11/20 20:20:44  tamches
 * a new template to support changes to tableVisi.C
 *
 * Revision 1.1  1995/11/04 00:47:44  tamches
 * First version of new table visi
 *
 */

#include "Vector.h"

template class vector<unsigned>;

#include "tvFocus.h"
template class vector<tvFocus>;

#include "tvMetric.h"
template class vector<tvMetric>;

#include "tvCell.h"
template class vector<tvCell>;
template class vector< vector<tvCell> >;

#include "../../../paradyn/src/UIthread/minmax.C"
template unsigned max(unsigned, unsigned);
template float max(float, float);
template bool ipmax(unsigned, unsigned);
template int ipmin(int, int);
template int min(int, int);
template int max(int, int);

#include "util/src/DictionaryLite.C"
#include "paradyn/src/UIthread/pdLogo.h"
template class vector<pdLogo *>;
template class dictionary_lite<string, pdLogo *>;
template class pair<string, pdLogo *>;
template class vector<dictionary_lite<string, pdLogo *>::hash_pair>;
template class vector< vector<dictionary_lite<string,pdLogo*>::hash_pair> >;

template class dictionary_lite<string, pdLogo::logoStruct>;
template class vector<pdLogo::logoStruct>;
template class vector<dictionary_lite<string, pdLogo::logoStruct>::hash_pair>;
template class vector< vector<dictionary_lite<string, pdLogo::logoStruct>::hash_pair> >;
template class pair<string, pdLogo::logoStruct>;

template class vector<string>;
