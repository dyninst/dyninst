/*
 * Copyright (c) 1996-2007 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */


/************************************************************************
 * $Id: Object-coff.h,v 1.3 2007/05/30 19:20:46 legendre Exp $
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

#include "symtabAPI/h/Dyn_Symbol.h"
#include <common/h/Types.h>
#include <string>
#include <vector>
#include <map>

using namespace std;

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
#include <map>
#include <string>
#include <vector>

using namespace std;





/************************************************************************
 * class Object
************************************************************************/

class Object : public AObject {
public:
             //Object (const string, void (*)(const char *) = log_msg);
             Object (const Object &);
	     Object (const string, const Address baseAddr,
                void (*)(const char *) = log_msg);
	     // "Filedescriptor" ctor
	     //Object(const fileDescriptor &desc, void (*)(const char *) = log_msg);
	     Object(const fDescriptor &desc, void (*)(const char *) = log_msg);

    virtual ~Object ();
    Object&   operator= (const Object &);
    bool isDynamic()	{ return dynamicallyLinked; }
    string& GetFile() { return file_; }
    
    bool isEEL() const { return false; }
    const char *interpreter_name() const { return NULL; }

private:
    void    load_object (bool sharedLib);
    void    get_relocation_entries(LDFILE *ldptr, int num);
    bool    dynamicallyLinked;
    LDFILE  *ldptr;
    filehdr fhdr;
    bool    did_open;
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
