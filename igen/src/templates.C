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

/*
 * $Log: templates.C,v $
 * Revision 1.7  1997/04/29 22:58:06  mjrg
 * Changed generated code so that it compiles with Visual C++
 *
 * Revision 1.6  1996/08/16 19:10:32  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.5  1996/01/18 16:35:54  hollings
 * Added extra  items for the AIX linker.
 *
 *
 */
#include "util/h/headers.h"

#pragma implementation "Vector.h"
#include "util/h/Vector.h"

#pragma implementation "Dictionary.h"
#include "util/h/Dictionary.h"

#include "parse.h"

template class vector<arg*>;
template class vector<message_layer*>;
template class vector<string>;
template class vector<Options::el_data>;
template class vector<Options::stl_data>;
template class vector<type_defn *>;

template class dictionary_iter<string, remote_func *>;
template class dictionary_iter<string, type_defn *>;
template class dictionary<string, type_defn *>;
template class dictionary<string, remote_func *>;

template class dictionary_hash<string, remote_func*>;
template class dictionary_hash<string, type_defn*>;
template class dictionary_hash_iter<string, type_defn*>;
template class dictionary_hash_iter<string, remote_func*>;

template class refCounter<string_ll>;
