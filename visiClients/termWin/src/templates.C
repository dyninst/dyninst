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
 * $Log: templates.C,v $
 * Revision 1.1  2001/04/25 20:11:57  wxd
 * this commit enables redirect application's output to a new tcl/tk terminal window. Therefore the output of paradyn front-end and debug information of paradyd will not mix up with the output of application.
 *
 * current commit can only redirect application's output when paradyn starts application. no modification is made when paradyn attach an already running application. also current commit can only work in unix-like platform. NT version met a cupple problems.
 *
 * this part of modification to paradyn is adding a new tcl/tk terminal window to show output of application which paradyn is working on. In that window, save menu can give paradyn user the chance to save the application's output to disk. option menu let paradyn user choose the way of terminal window's termination. Currently two modes are provide: on paradyn exit or persistant, in which paradyn exit is the default way.
 *
 * Revision 1.9  2000/07/27 17:42:43  pcroth
 * Updated #includes to reflect new locations of util headers
 *
 * Revision 1.8  1999/03/13 15:23:59  pcroth
 * Added support for building under Windows NT
 *
 * Revision 1.7  1997/10/29 03:38:21  tamches
 * fix to previous commit
 *
 * Revision 1.6  1997/10/28 20:45:22  tamches
 * update for new dictionary class
 *
 * Revision 1.5  1997/09/05 22:20:33  naim
 * Changes to template files to be able to compile paradyn with -O3 - naim
 *
 * Revision 1.4  1997/04/30 15:37:16  mjrg
 * added template needed for new implementation of string class
 *
 * Revision 1.3  1996/08/16 21:35:49  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.2  1996/01/17 18:32:00  newhall
 * changes due to new visiLib
 *
 * Revision 1.1  1995/12/15 22:01:55  tamches
 * first version of phaseTable
 *
 */

#pragma implementation "Vector.h"
#include "common/h/Vector.h"
//#include "visi/h/visiTypes.h"

class clientConn;

template class vector<clientConn *>;

// logo stuff:
#include "paradyn/src/UIthread/minmax.C"
template float max(const float, const float);

#include "common/h/String.h"
#include "common/src/Dictionary.C"
#include "pdLogo.h"
template class dictionary_hash<string, pdLogo *>;
template class vector<dictionary_hash<string, pdLogo *>::entry>;
template class vector<pdLogo*>;

template class dictionary_hash<string, pdLogo::logoStruct>;
template class vector<dictionary_hash<string, pdLogo::logoStruct>::entry>;
template class vector<pdLogo::logoStruct>;

template class vector<unsigned int>;
template class refCounter<string_ll>;
