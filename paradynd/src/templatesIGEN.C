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
 * $Id: templatesIGEN.C,v 1.10 2002/04/09 18:06:28 mjbrim Exp $
 * Generate all the templates in one file.
 */

//template instantiations for igen classes now done in .temp.C files
//automatically created by igen, except for the following needed
//for the nt build

#if defined(i386_unknown_nt4_0)
#include "pdutil/h/xdr_send_recv.h"
#include "dyninstRPC.xdr.h"
template bool writerfn_noMethod<int>(struct XDR *, const int);
template bool writerfn_noMethod<unsigned>(struct XDR *, const unsigned);
template bool writerfn_noMethod<string>(struct XDR *, const string);
template bool writerfn_noMethod<T_dyninstRPC::mdl_expr *>(struct XDR *, T_dyninstRPC::mdl_expr * const);
template bool writerfn_noMethod<T_dyninstRPC::mdl_stmt *>(struct XDR *, T_dyninstRPC::mdl_stmt * const);
template bool writerfn_noMethod<T_dyninstRPC::mdl_icode *>(struct XDR *, T_dyninstRPC::mdl_icode * const);
template bool writerfn_noMethod<T_dyninstRPC::mdl_constraint *>(struct XDR *, T_dyninstRPC::mdl_constraint * const);
template bool writerfn_noMethod<T_dyninstRPC::mdl_metric *>(struct XDR *, T_dyninstRPC::mdl_metric * const);
template bool writerfn_noMethod<T_dyninstRPC::batch_buffer_entry>(struct XDR *, const T_dyninstRPC::batch_buffer_entry);
template bool writerfn_noMethod<T_dyninstRPC::trace_batch_buffer_entry>(struct XDR *, const T_dyninstRPC::trace_batch_buffer_entry);
template bool writerfn_noMethod<T_dyninstRPC::resourceInfoCallbackStruct>(struct XDR *, const T_dyninstRPC::resourceInfoCallbackStruct);
template bool writerfn_noMethod<T_dyninstRPC::focusStruct>(struct XDR *, const T_dyninstRPC::focusStruct);
template bool writerfn_noMethod<T_dyninstRPC::metricInfo>(struct XDR *, const T_dyninstRPC::metricInfo);
#endif
