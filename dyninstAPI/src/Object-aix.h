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

/************************************************************************
 * AIX object files.
 * $Id: Object-aix.h,v 1.13 2000/07/28 17:20:41 pcroth Exp $
************************************************************************/





#if !defined(_Object_aix_h_)
#define _Object_aix_h_





/************************************************************************
 * header files.
************************************************************************/

#include "common/h/Dictionary.h"
#include "common/h/String.h"
#include "common/h/Symbol.h"
#include "common/h/Types.h"
#include "common/h/Vector.h"

extern "C" {
#include <a.out.h>
};

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>





/************************************************************************
 * class Object
************************************************************************/

class Object : public AObject {
public:
             Object (const string, void (*)(const char *) = log_msg);
             Object (const Object &);
	     Object (const string, const Address baseAddr,
                void (*)(const char *) = log_msg);
    ~Object ();

    Object&   operator= (const Object &);
    int getTOCoffset() const { return toc_offset_; }
    void get_stab_info(char *&stabstr, int &nstabs, Address &stabs, char *&stringpool) {
	stabstr = (char *) stabstr_;
	nstabs = nstabs_;
	stabs = stabs_;
	stringpool = (char *) stringpool_;
    }
    void get_line_info(int& nlines, char*& lines,unsigned long& fdptr){
	nlines = nlines_;
	lines = (char*) linesptr_; 
	fdptr = linesfdptr_;
    }

private:
    void load_object ();
    int  toc_offset_;
    int  nstabs_;
    int  nlines_;
    Address stabstr_;
    Address stabs_;
    Address stringpool_;
    Address linesptr_;
    Address linesfdptr_;
};



#endif /* !defined(_Object_aix_h_) */
