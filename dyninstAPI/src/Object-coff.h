/*
 * Copyright (c) 1996-2004 Barton P. Miller
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

/************************************************************************
 * $Id: Object-coff.h,v 1.13 2004/03/23 01:11:59 eli Exp $
 * COFF object files.
 * Note - this is DEC OSF/1 coff which probably isn't the real thing
 *
 * Note - this probably isn't perfect
 *      - the symbol dictionary will only map one symbol table entry to each name
 *      - I am not determining if symbols are external
 *      - I may be ignoring some useful symbols from the symbol table
 *      - I am checking the dictionary to make sure no existing entry get clobbered
 *      - I am using mmap
 *      - All of these objects need some type of "destroy" method to munmap and
 *          free memory.  This method will be invoked explictly, not implicitly.
************************************************************************/





#if !defined(_Object_coff_h_)
#define _Object_coff_h_



#define LOG_WORD 2

/************************************************************************
 * header files.
************************************************************************/

#include <common/h/Dictionary.h>
//#include <common/h/Line.h>
#include <common/h/String.h>
#include <common/h/Symbol.h>
#include <common/h/Types.h>
#include <common/h/Vector.h>

extern "C" {
#include <a.out.h>
};

#include <filehdr.h>
#include <syms.h>
#include <ldfcn.h>
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
             Object (const pdstring, void (*)(const char *) = log_msg);
             Object (const Object &);
	     Object (const pdstring, const Address baseAddr,
                void (*)(const char *) = log_msg);
	     // "Filedescriptor" ctor
	     Object(fileDescriptor *desc, Address baseAddr = 0 , void (*)(const char *) = log_msg);

    virtual ~Object ();
    Object&   operator= (const Object &);
    bool isDynamic()	{ return dynamicallyLinked; }
    pdstring& GetFile() { return file_; }
    
    bool isEEL() const { return false; }

private:
    void    load_object (bool sharedLib);
    void	get_relocation_entries(LDFILE *ldptr, int num);
    bool	dynamicallyLinked;
};

inline
Object::~Object() {
}

inline
Object&
Object::operator=(const Object& obj) {
    (void) AObject::operator=(obj);
    return *this;
}



#endif /* !defined(_Object_bsd_h_) */
