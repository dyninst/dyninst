/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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
 * $Id: templates-nt.C,v 1.4 2002/07/03 23:13:46 schendel Exp $
 */

/* The VC++ v5.0 compiler (probably correctly) generates warning C4660's 
 * "template class specialization XXXX is already instantiated"
 * however when linking the executable, the linker is unfortunately not
 * able to resolve these external symbols and produces error LNK2001's,
 * therefore the compiler warning is being disabled for this template file.
 */
#pragma warning (disable: 4660)

#include "common/h/Dictionary.h"
#include "common/src/Dictionary.C"

#include "arg.h"
#include "type_defn.h"
#include "message_layer.h"
#include "Options.h"

template class vector<arg *>;
template class vector<unsigned>;
template class vector<type_defn *>;
template class vector<message_layer *>;
template class vector<Options::stl_data>;
template class vector<Options::el_data>;
template class dictionary_hash<string, type_defn *>;
template class dictionary_hash<string, remote_func *>;
template class vector<dictionary_hash<string, type_defn *>::entry>;
template class vector<dictionary_hash<string, remote_func *>::entry>;
