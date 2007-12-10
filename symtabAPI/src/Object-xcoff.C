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

// $Id: Object-xcoff.C,v 1.14 2007/12/10 22:33:36 giri Exp $

#include <regex.h>

#include "symtabAPI/src/Object.h"
#include "symtabAPI/src/Collections.h"
#include "common/h/pathName.h"
#include "symtabAPI/h/Symtab.h"

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;


#include <procinfo.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <assert.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <scnhdr.h>
#include <sys/time.h>
#include <sys/reg.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <procinfo.h> // struct procsinfo
#include <sys/types.h>
#include <sys/param.h> // PAGESIZE
#include <set>
using namespace std;
#include <iomanip>

#include <xcoff.h>
#define __AR_BIG__
#define __AR_SMALL__
#include <ar.h> // archive file format.

#include <dlfcn.h>
#include "common/h/debugOstream.h"

/* For some reason this symbol type isn't global */
#if !defined(C_WEAKEXT)
#define C_WEAKEXT 0
#endif

char errorLine[100];
std::string current_func_name;
std::string current_mangled_func_name;
Symbol *current_func = NULL;

// 07/11/2006 giri: definitions below are borrowed from dyninstAPI/src/arch-power.h
// to remove the dependency on dyninst. Only the ones required have been borrowed

#define Bop             18      /* (unconditional) branch */
#define Lop             32      /* load (word) (aka lwz op in PowerPC) */
#define BCLRop          19      /* branch conditional link register */
#define BCCTRxop        528     /* branch conditional count register */

struct iform {            // unconditional branch +
  unsigned op : 6;
  signed   li : 24;
  unsigned aa : 1;
  unsigned lk : 1;

};

struct dform {
    unsigned op : 6;
    unsigned rt : 5;        // rt, rs, frt, frs, to, bf_l
    unsigned ra : 5;
    signed   d_or_si : 16;  // d, si, ui
};

struct xlform {
  unsigned op : 6;
  unsigned bt : 5;   // rt, bo, bf_
  unsigned ba : 5;   // ba, bi, bfa_
  unsigned bb : 5;
  unsigned xo : 10;  // xo, eo
  unsigned lk : 1;
};

typedef union {
  struct iform  iform;  // branch;
  struct dform  dform;
  struct xlform xlform;
  unsigned int  raw;
} instructUnion;

unsigned long roundup4(unsigned long val) {
   while (val % 4 != 0)
      val++;
   return val;
}

// giri: Brought in here from dyninst/src/aix.C
typedef void *Name;
typedef enum { VirtualName, MemberVar, Function, MemberFunction, Class,
               Special, Long } NameKind;
typedef enum { RegularNames = 0x1, ClassNames = 0x2, SpecialNames = 0x4,
               ParameterText = 0x8, QualifierText = 0x10 } DemanglingOptions;

Name *(*P_native_demangle)(char *, char **, unsigned long) = NULL;
char *(*P_functionName)(Name *) = NULL;
char *(*P_varName)(Name *) = NULL;
char *(*P_text)(Name *) = NULL;
NameKind (*P_kind)(Name *) = NULL;

void loadNativeDemangler() 
{
   P_native_demangle = NULL;
   
   void *hDemangler = dlopen("libdemangle.so.1", RTLD_LAZY|RTLD_MEMBER);
   if (hDemangler != NULL) {
      P_native_demangle = (Name*(*)(char*, char**, long unsigned int)) dlsym(hDemangler, "demangle");
      //if (!P_native_demangle) 
      //   BPatch_reportError(BPatchSerious,122,
      //             "unable to locate function demangle in libdemangle.so.1\n");

      P_functionName = (char*(*)(Name*)) dlsym(hDemangler, "functionName");
      //if (!P_functionName) 
      //   BPatch_reportError(BPatchSerious,122,
      //         "unable to locate function functionName in libdemangle.so.1\n");
      
      P_varName = (char*(*)(Name*)) dlsym(hDemangler, "varName");
      //if (!P_varName) 
      //   BPatch_reportError(BPatchSerious,122,
      //              "unable to locate function varName in libdemangle.so.1\n");

      P_kind = (NameKind(*)(Name*)) dlsym(hDemangler, "kind");
      //if (!P_kind) 
      //   BPatch_reportError(BPatchSerious,122,
      //                      "unable to locate function kind in libdemangle.so.1\n");
      
      P_text = (char*(*)(Name*)) dlsym(hDemangler, "text");
      //if (!P_text) 
      //   BPatch_reportError(BPatchSerious,122,
      //                 "unable to locate function text in libdemangle.so.1\n");
   } 
}


extern "C" char *cplus_demangle(char *, int);
extern void dedemangle( const char * demangled, char * dedemangled );

#define DMGL_PARAMS      (1 << 0)       /* Include function args */
#define DMGL_ANSI        (1 << 1)       /* Include const, volatile, etc */

char *P_cplus_demangle( const char * symbol, bool nativeCompiler, bool includeTypes ) {
 /* If the symbol isn't from the native compiler, or the native demangler
    isn't available, use the built-in. */
 bool nativeDemanglerAvailable = P_native_demangle != NULL &&
         P_text != NULL &&
         P_varName != NULL &&
         P_functionName != NULL;
 if( !nativeCompiler || ! nativeDemanglerAvailable ) {
  char * demangled = cplus_demangle( const_cast<char *>(symbol),
     includeTypes ? DMGL_PARAMS | DMGL_ANSI : 0 );
  if( demangled == NULL ) { return NULL; }

  if( ! includeTypes ) {
   /* De-demangling never makes a string longer. */
   char * dedemangled = strdup( demangled );
   assert( dedemangled != NULL );

   dedemangle( demangled, dedemangled );
   assert( dedemangled != NULL );

   free( demangled );
   return dedemangled;
   }

  return demangled;
   } /* end if not using native demangler. */
   else if( nativeDemanglerAvailable ) {
  /* Use the native demangler, which apparently behaves funny. */
  Name * name;
  char * rest;
  
  /* P_native_demangle() won't actually demangled 'symbol'.
     Find out what P_kind() of symbol it is and demangle from there. */
  name = (P_native_demangle)( const_cast<char*>(symbol), (char **) & rest,
   RegularNames | ClassNames | SpecialNames | ParameterText | QualifierText );
  if( name == NULL ) { return NULL; }

  char * demangled = NULL;
  switch( P_kind( name ) ) {
   case Function:
                           if (includeTypes)
                                demangled = (P_text)( name );
                           else
    demangled = (P_functionName)( name );   
    break;
   
   case MemberFunction:
    /* Doing it this way preserves the leading classnames. */
    demangled = (P_text)( name );
    break;

   case MemberVar:
    demangled = (P_varName)( name );
    break;

   case VirtualName:
   case Class:
   case Special:
   case Long:
    demangled = (P_text)( name );
    break;
   default: assert( 0 );
   } /* end P_kind() switch */

  /* Potential memory leak: no P_erase( name ) call.  Also, the
     char *'s returned from a particular Name will be freed
     when that name is erase()d or destroyed, so strdup if we're
     fond of them. */
   
  if( ! includeTypes ) {
   /* De-demangling never makes a string longer. */
   char * dedemangled = strdup( demangled );
   assert( dedemangled != NULL );

   dedemangle( demangled, dedemangled );
   assert( dedemangled != NULL );

   return dedemangled;
   }

  return demangled;
  } /* end if using native demangler. */
 else {
  /* We're trying to demangle a native binary but the native demangler isn't available.  Punt. */ 
  return NULL;
  }
 } /* end P_cplus_demangle() */


// Methods to read file and ar header for both small (32-bit) and
// large (64-bit) archive files. This lets us have a single archive
// parsing method.
int xcoffArchive_32::read_arhdr()
{
  char tmpstring[13];

  if (!fo_->set(0))
      return -1;
  if (!fo_->read(&filehdr, sizeof(struct fl_hdr)))
      return -1;
  if (strncmp(filehdr.fl_magic, AIAMAG, SAIAMAG))
    return -1;
  strncpy(tmpstring, filehdr.fl_fstmoff, 12);
  tmpstring[12] = 0; first_offset = atol(tmpstring);
  if (first_offset % 2) first_offset++;
  strncpy(tmpstring, filehdr.fl_lstmoff, 12);
  tmpstring[12] = 0; last_offset = atol(tmpstring);
  if (last_offset % 2) last_offset++;
  next_offset = first_offset;
  // Offsets are always even
  return 0;
}

int xcoffArchive_64::read_arhdr()
{
  char tmpstring[22];

  if (!fo_->set(0))
      return -1;

  if (!fo_->read(&filehdr, sizeof(struct fl_hdr_big)))
      return -1;

  if (strncmp(filehdr.fl_magic, AIAMAGBIG, SAIAMAG))
    return -1;
  strncpy(tmpstring, filehdr.fl_fstmoff, 21);
  tmpstring[21] = 0; first_offset = atol(tmpstring);
  if (first_offset % 2) first_offset++;
  strncpy(tmpstring, filehdr.fl_lstmoff, 21);
  tmpstring[21] = 0; last_offset = atol(tmpstring);
  if (last_offset % 2) last_offset++;
  next_offset = first_offset;

  return 0;
}

// Archive member header parsing function for 32/64-bit archives
// Pre:  takes an offset into the ar file which is the start point
//       of a member header
// Post: member_name contains the name of the file corresponding to
//       the member header
//       next_offset contains the offset to the next member header
//       or 0 if there are no members
//       aout_offset contains the offset to the file corresponding 
//       to the member name

int xcoffArchive_32::read_mbrhdr()
{
    char tmpstring[13];
    
    if (next_offset == 0) return -1;

    if (!fo_->set(next_offset))
        return -1;

    if (!fo_->read(&memberhdr, sizeof(struct ar_hdr) - 2))
        return -1;

    strncpy(tmpstring, memberhdr.ar_namlen, 4);
    tmpstring[4] = 0; member_len = atol(tmpstring);
    if (member_name) free(member_name);
    member_name = (char *)malloc(member_len+1);
    
    if (!fo_->read(member_name, member_len))
        return -1;
    // Terminating null
    member_name[member_len] = 0;
    
    // Set the file offset for this member
    aout_offset = next_offset + sizeof(struct ar_hdr) + member_len;
    if (aout_offset % 2) aout_offset += 1;
    
    // Fix up next_offset
    if (next_offset == last_offset)
        next_offset = 0; // termination condition
    else {
        strncpy(tmpstring, memberhdr.ar_nxtmem, 12);
        tmpstring[12] = 0; next_offset = atol(tmpstring);
        if (next_offset % 2) next_offset++;
    }
    
    return 0;
}
// 64-bit function. Differences: structure size
// A lot of shared code. 
int xcoffArchive_64::read_mbrhdr()
{
  char tmpstring[21];
  
  if (next_offset == 0) return -1;

    if (!fo_->set(next_offset))
        return -1;

    if (!fo_->read(&memberhdr, sizeof(struct ar_hdr_big) - 2))
        return -1;

    strncpy(tmpstring, memberhdr.ar_namlen, 4);
    tmpstring[4] = 0; member_len = atol(tmpstring);
    if (member_name) free(member_name);
    member_name = (char *)malloc(member_len+1);

    if (!fo_->read(member_name, member_len))
        return -1;
    // Terminating null
    member_name[member_len] = 0;
    
    // Set the file offset for this member
    aout_offset = next_offset + sizeof(struct ar_hdr_big) + member_len;
    if (aout_offset % 2) aout_offset += 1;
    
    // Fix up next_offset
    if (next_offset == last_offset)
        next_offset = 0; // termination condition
    else {
        strncpy(tmpstring, memberhdr.ar_nxtmem, 20);
        tmpstring[20] = 0; next_offset = atol(tmpstring);
        if (next_offset % 2) next_offset++;
    }

    return 0;
}

// Wrapper to handle the 1:many mapping of archives and members

std::vector<fileOpener *> fileOpener::openedFiles;

fileOpener *fileOpener::openFile(const std::string &filename) {
    // Logic: if we're opening a library, match by name. If
    // we're opening an a.out, then we have to uniquely
    // open each time (as we open in /proc, and exec has the
    // same name).

    if(filename.substr(0,5) != "/proc"){
     for (unsigned i = 0; i < openedFiles.size(); i++) {
            if (openedFiles[i]->file() == filename) {
                openedFiles[i]->refcount_++;
                return openedFiles[i];
            }
        }
    }
    
    // Originally we were checking if its a shared Object. Now we 
    // check if the filename does not start with a /proc
    /*if (desc.isSharedObject()) {
        for (unsigned i = 0; i < openedFiles.size(); i++) {
            if (openedFiles[i]->file() == desc.file()) {
                openedFiles[i]->refcount_++;
                return openedFiles[i];
            }
        }
    }*/
    
    // New file. Neeefty.
    fileOpener *newFO = new fileOpener(filename);
    assert(newFO);
   
    if (!newFO->open()) {
        fprintf(stderr, "File %s\n", filename.c_str());
        //perror("Opening file");
        return NULL;
    }
    if (!newFO->mmap()) {
        fprintf(stderr, "File %s\n", filename.c_str());
        //perror("mmaping file");
        return NULL;
    }
    openedFiles.push_back(newFO);

    return newFO;
}

fileOpener *fileOpener::openFile(void *ptr, unsigned size) {
    // Logic: if we're opening a library, match by name. If
    // we're opening an a.out, then we have to uniquely
    // open each time (as we open in /proc, and exec has the
    // same name).

   for (unsigned i = 0; i < openedFiles.size(); i++) 
   {
       if ((openedFiles[i]->mem_image() == ptr) && (openedFiles[i]->size() == size)){
           openedFiles[i]->refcount_++;
           return openedFiles[i];
       }
    }
    
    // New file. Neeefty.
    fileOpener *newFO = new fileOpener(ptr,size);
    assert(newFO);
    
    openedFiles.push_back(newFO);

    return newFO;
}

void fileOpener::closeFile() {
    refcount_--;

    if (refcount_ > 0) return;

    // Remove us from the big list...
    std::vector<fileOpener *>::iterator iter = openedFiles.begin();
    for(; iter!=openedFiles.end(); iter++)
    {
     if(*iter == this)
  openedFiles.erase(iter);
    }
    
    /*for (unsigned i = 0; i < openedFiles.size(); i++) {
        if (openedFiles[i] == this)
            openedFiles.erase(i));
    }*/

    if(file()!="") //only if its not a mem image
    {
     ::munmap(mmapStart_, size_);

     ::close(fd_);
    } 
    mmapStart_ = 0;
    fd_ = 0;
    size_ = 0;

    delete this;
}

fileOpener::~fileOpener() {
    // Assert that we're already closed?
    assert(fd_ == 0);
    assert(mmapStart_ == 0);
    assert(size_ == 0);
}

bool fileOpener::open() {
    if (fd_ != 0)
        return true;
    fd_ = ::open(file_.c_str(), O_RDONLY, 0); 
    if (fd_ < 0) {
        sprintf(errorLine, "Unable to open %s: %s\n",
                file_.c_str(), strerror(errno));
        //statusLine(errorLine);
        //showErrorCallback(27, errorLine);
        fd_ = 0;
        return false;
    }

    // Set size
    struct stat statBuf;
    int ret;
    ret = fstat(fd_, &statBuf);
    if (ret == -1) {
        sprintf(errorLine, "Unable to stat %s: %s\n",
                file_.c_str(), strerror(errno));
        //statusLine(errorLine);
        //showErrorCallback(27, errorLine);
        return false;
    }
    assert(ret == 0);

    size_ = statBuf.st_size;
    assert(size_);
    return true;
}

bool fileOpener::mmap() {
    assert(fd_);
    assert(size_);

    if (mmapStart_ != NULL)
        return true;

    mmapStart_ = ::mmap(NULL, size_, PROT_READ, MAP_SHARED, fd_, 0);

    if (mmapStart_ == MAP_FAILED) {
        sprintf(errorLine, "Unable to mmap %s: %s\n",
                file().c_str(), strerror(errno));
        //statusLine(errorLine);
        //showErrorCallback(27, errorLine);
        mmapStart_ = NULL;
        return false;
    }

    return true;
}
    
bool fileOpener::set(unsigned addr) {
    //assert(fd_);     may not be present if its a mem image
    assert(size_);
    assert(mmapStart_);

    if (addr >= size_) {
        fprintf(stderr, "Warning: attempting to set offset to address %d (0x%x) greater than size %d (0x%x)\n",
                addr, addr,
                size_, size_);
        return false;
    }
    offset_ = addr;
    return true;
}

bool fileOpener::read(void *buf, unsigned size) {
    //assert(fd_); may not be present if its a mem image
    assert(size_);
    assert(mmapStart_);

    if ((offset_ + size) >= size_)
        return false;

    memcpy(buf, ptr(), size);

    offset_ += size;
    return true;
}

bool fileOpener::seek(int offset) {
    if ((((int) offset_ + offset) < 0) ||
        ((offset_ + offset) >= size_))
        return false;
    offset_ += offset;
    return true;
}

void *fileOpener::ptr() const {
    // Return a pointer to the current offset
    char *tmpPtr = (char *)mmapStart_ + offset_;
    return (void *)tmpPtr;
}

void *fileOpener::getPtrAtOffset(unsigned offset) const
{
    char *tmpPtr = (char *)mmapStart_ + offset;
    return (void *)tmpPtr;
}
    
// This function parses a 32-bit XCOFF file, either as part of an
// archive (library) or from an a.out file. It takes an open file
// descriptor from which it reads its data. In addition, it loads the
// data segment from the pid given in AIX_PID_HACK, the pid of the
// running process. We have to do this because the data segment
// in memory is more correct than the one on disk. Specifically, the
// TOC for inter-module function calls is incorrect on disk, because
// offsets are calculated at runtime instead of link time.
//
// int fd: file descriptor for a.out object, not closed
// int offset: offset to begin reading at (for archive libraries)

// File parsing error macro, assumes errorLine defined
#define PARSE_AOUT_DIE(errType, errCode) { \
      fprintf(stderr,"warning:parsing a.out file %s(%s): %s \n", \
              file_.c_str(), member_.c_str(), errType); \
      err_func_(errType); \
      return; \
      }

void Object::parse_aout(int offset, bool /*is_aout*/)
{
   // all these vrble declarations need to be up here due to the gotos,
   // which mustn't cross vrble initializations.  Too bad.
   long i,j;
   std::string name;
   unsigned value;
   unsigned secno;
   unsigned textSecNo, dataSecNo , loaderSecNo;
   union auxent *aux = NULL;
   struct filehdr hdr;
   struct syment *sym = NULL;
   struct aouthdr aout;
   union auxent *csect = NULL;
   char *stringPool=NULL;
   Symbol::SymbolType type; 
   bool foundDebug = false;
   bool foundLoader = false;
   bool foundData = false;
       
   stabs_ = NULL;
   nstabs_ = 0;
   stringpool_ = NULL;
   stabstr_ = NULL;
   linesptr_ = NULL;
   nlines_ = 0;
   linesfdptr_ = 0;

   struct syment *symbols = NULL;
   struct scnhdr *sectHdr = NULL;
   Symbol::SymbolLinkage linkage = Symbol::SL_UNKNOWN;
   unsigned toc_offset = 0;
   std::string modName;
   baseAddress_ = (Offset)fo_->getPtrAtOffset(offset);

   int linesfdptr=0;
   struct lineno* lines=NULL;

   // Get to the right place in the file (not necessarily 0)
   if (!fo_->set(offset))
       PARSE_AOUT_DIE("Seeking to correct offset", 49);

   if (!fo_->read(&hdr, sizeof(struct filehdr)))
       PARSE_AOUT_DIE("Reading file header", 49);

   if (hdr.f_magic == 0x1ef || hdr.f_magic == 0x01F7) {
       // XCOFF64 file! We don't handle those yet.
         //PARSE_AOUT_DIE("unhandled XCOFF64 header found", 49);
         //bperr("Unhandled XCOFF64 header found!\n");
         return;
   }
   
   if (hdr.f_magic != 0x1df) {
        PARSE_AOUT_DIE("possible problem, magic number is not 0x1df", 49);
       //bperr( "Possible problem, magic number is %x, should be %x\n",
              //hdr.f_magic, 0x1df);
   }
#if 0   
   if(hdr.f_opthdr == 0)
    cout << "no aout header" << endl;
   if(hdr.f_opthdr == _AOUTHSZ_SHORT)
    cout << "short header" << endl;
#endif   
  
   if (!fo_->read(&aout, hdr.f_opthdr))
       PARSE_AOUT_DIE("Reading a.out header", 49);


   sectHdr = (struct scnhdr *) fo_->ptr();
   fo_->seek(sizeof(struct scnhdr) * hdr.f_nscns);

   if (!sectHdr)
       PARSE_AOUT_DIE("Reading section headers", 49);
   
   //if binary is not stripped 
   if( hdr.f_symptr ) {
       fo_->set(offset + hdr.f_symptr);
       symbols = (struct syment *) fo_->ptr();
       fo_->seek(hdr.f_nsyms * SYMESZ);
       
       if (!symbols)
           PARSE_AOUT_DIE("Reading symbol table", 49);
   }
   // Consistency check
   if(hdr.f_opthdr == _AOUTHSZ_EXEC) //complete aout header present
   {
    if ((unsigned) aout.text_start != sectHdr[aout.o_sntext-1].s_paddr)
        PARSE_AOUT_DIE("Checking text address", 49);
    if ((unsigned) aout.data_start != sectHdr[aout.o_sndata-1].s_paddr) 
        PARSE_AOUT_DIE("Checking data address", 49);
    if ((unsigned) aout.tsize != sectHdr[aout.o_sntext-1].s_size) 
        PARSE_AOUT_DIE("Checking text size", 49);
    if ((unsigned long) aout.dsize != sectHdr[aout.o_sndata-1].s_size)
        PARSE_AOUT_DIE("Checking data size", 49);
   }
   /*else if(hdr.f_opthdr == _AOUTHSZ_SHORT)
   {
    if ((unsigned) aout.text_start != sectHdr[aout.o_sntext-1].s_paddr)
        PARSE_AOUT_DIE("Checking text address", 49);
    if ((unsigned) aout.data_start != sectHdr[aout.o_sndata-1].s_paddr) 
        PARSE_AOUT_DIE("Checking data address", 49);
    if ((unsigned) aout.tsize != sectHdr[aout.o_sntext-1].s_size) 
        PARSE_AOUT_DIE("Checking text size", 49);
    if ((unsigned long) aout.dsize != sectHdr[aout.o_sndata-1].s_size)
        PARSE_AOUT_DIE("Checking data size", 49);
   }*/
   if(hdr.f_opthdr !=  0)
	 entryAddress_ = aout.entry;

    /*
    * Get the string pool, if there is one
    */
   if( hdr.f_nsyms ) 
   {
       // We want to go after the symbol table...
       if (!fo_->set(offset + hdr.f_symptr + (hdr.f_nsyms*SYMESZ)))
           PARSE_AOUT_DIE("Could not seek to string pool", 49);
       Offset stringPoolSize;
       fo_->read(&stringPoolSize, 4);
       
       if (!fo_->set(offset + hdr.f_symptr + (hdr.f_nsyms*SYMESZ)))
           PARSE_AOUT_DIE("Could not seek to string pool", 49);

       // First 4 bytes is the length; this is included in the string pool pointer
       stringPool = (char *)fo_->ptr();

       if (!stringPool)
           PARSE_AOUT_DIE("Getting pointer to string pool", 49);
   }
   
   /* find the text section such that we access the line information */
   nlines_ = 0;
   no_of_sections_ = hdr.f_nscns;
   foundLoader = false;
   foundData = false;
   
   for (i=0; i < hdr.f_nscns; i++)
   {
       if(sectHdr[i].s_flags & STYP_DATA)
       {
           dataSecNo = i;
    foundData = true;
       }   
       else if(sectHdr[i].s_flags & STYP_LOADER)
       {
           loaderSecNo = i;
    foundLoader = true;
       }    
       if (sectHdr[i].s_flags & STYP_TEXT) {
           textSecNo = i;
           nlines_ = sectHdr[i].s_nlnno;
           
           /* if there is overflow in the number of lines */
           if (nlines_ == 65535)
               for (j=0; j < hdr.f_nscns; j++)
                   if ((sectHdr[j].s_flags & STYP_OVRFLO) &&
                       (sectHdr[j].s_nlnno == (i+1))){
                       nlines_ = (unsigned int)(sectHdr[j].s_vaddr);
                       break;
                   }
           
           /* There may not be any line information. */
           if (nlines_ == 0)
               continue;

           /* read the line information table */
           if (!fo_->set(offset + sectHdr[i].s_lnnoptr))
               PARSE_AOUT_DIE("Seeking to line information table", 49);
           lines = (struct lineno *)fo_->ptr();
           if (!lines)
               PARSE_AOUT_DIE("Reading line information table", 49);
           fo_->seek(nlines_ * LINESZ);
           
           linesfdptr = sectHdr[i].s_lnnoptr;
           //break;
       }
       if (sectHdr[i].s_flags & STYP_BSS) {
          bss_size_ = sectHdr[i].s_size;
       }
   }

   for (i=0; i < hdr.f_nscns; i++)
       sections_.push_back(new Section(i, sectHdr[i].s_name, sectHdr[i].s_paddr, fo_->getPtrAtOffset(offset+sectHdr[i].s_scnptr), sectHdr[i].s_size));

   // Time to set up a lot of variables.
   // code_ptr_: a pointer to the text segment
   // code_off_: Subtracted from an offset before lookup occurs
   // code_len_: Size of text in bytes
   // text_reloc_: the amount to add to the "base address" given us by the system.

   // data_ptr_: a pointer to the data segment
   // data_off_: subtracted from a data offset before lookup occurs
   // data_len_: size of data in bytes.
   // data_reloc_: the amount to add to the "base address" given us by the system.

   // Temporaries: the file is loaded read-only and thus we can't modify it.
   //Offset fileTextOffset = roundup4(sectHdr[aout.o_sntext-1].s_scnptr);
   //Offset fileDataOffset = roundup4(sectHdr[aout.o_sndata-1].s_scnptr);
   Offset fileTextOffset = roundup4(sectHdr[textSecNo].s_scnptr);
   Offset fileDataOffset;
   if(foundData)
    fileDataOffset = roundup4(sectHdr[dataSecNo].s_scnptr);
   else
    fileDataOffset = (Offset) -1;
   

   if (!fo_->set(fileTextOffset + offset))
       PARSE_AOUT_DIE("Seeking to start of text segment", 49);
   code_ptr_ = (char *)fo_->ptr();
   if (!code_ptr_)
       PARSE_AOUT_DIE("Reading text segment", 49);

   if(foundData)
   {
    if (!fo_->set(fileDataOffset + offset))
        PARSE_AOUT_DIE("Seeking to start of data segment", 49);
    data_ptr_ = (char *)fo_->ptr();
    if (!data_ptr_)
        PARSE_AOUT_DIE("Reading data segment", 49);
   }
   else
    data_ptr_ = NULL;

   // Offsets; symbols will be offset from the virtual address. Grab
   // that now.  These are defined (and asserted, above) to be equal
   // to the s_paddr (and s_vaddr) fields in their respective
   // sections.
   //code_off_ = roundup4(aout.text_start);
   //data_off_ = roundup4(aout.data_start);
   code_off_ = roundup4(sectHdr[textSecNo].s_paddr);
   if(foundData)
 data_off_ = roundup4(sectHdr[dataSecNo].s_paddr);
   else
    data_off_ = (Offset) -1;

   // As above, these are equal to the s_size fields in the respective fields.
   //code_len_ = aout.tsize;
   //data_len_ = aout.dsize;
   code_len_ = sectHdr[textSecNo].s_size;
   if(foundData)
 data_len_ = sectHdr[dataSecNo].s_size;
   else
    data_len_ = 0;

   if(foundLoader)
   {
    //FIND LOADER INFO!
    loader_off_ = sectHdr[loaderSecNo].s_scnptr;
    //loader_off_ = sectHdr[aout.o_snloader-1].s_scnptr;
    loadAddress_ = loader_off_;
    loader_len_ = sectHdr[loaderSecNo].s_size; 
    //loader_len_ = sectHdr[aout.o_snloader-1].s_size; 
   } 

#if 0
   fprintf(stderr, "Loader offset: 0x%x, len 0x%x\n", loader_off_, loader_len_);
   fprintf(stderr, "Loader vaddr: 0x%x\n", sectHdr[aout.o_snloader-1].s_vaddr);
#endif

   text_reloc_ = fileTextOffset;
   data_reloc_ = fileDataOffset;

   // And some debug output
#if defined(DEBUG)
   fprintf(stderr, "Data dump from %s/%s\n",
           file_.c_str(), member_.c_str());
   fprintf(stderr, "Text offset in file: 0x%x (0x%x + 0x%x), virtual address 0x%x, size %d (0x%x) bytes\n",
           fileTextOffset + offset, fileTextOffset, offset, code_off_, code_len_, code_len_);
   fprintf(stderr, "Data offset in file: 0x%x (0x%x + 0x%x), virtual address 0x%x, size %d (0x%x) bytes\n",
           fileDataOffset + offset, fileDataOffset, offset, data_off_, data_len_, data_len_);
#endif
   
   foundDebug = false;

   // Find the debug symbol table.
   for (i=0; i < hdr.f_nscns; i++)
   {
       if (sectHdr[i].s_flags & STYP_DEBUG) 
       {
           foundDebug = true;
           break;
       }
   }
   if( foundDebug ) 
   {
       stabs_ = (void *) symbols;
       nstabs_ = hdr.f_nsyms;
       stringpool_ = (void *) stringPool;
       if( hdr.f_nsyms ) {
           if (!fo_->set(sectHdr[i].s_scnptr + offset))
               PARSE_AOUT_DIE("Seeking to initialized debug section", 49);

           stabstr_ = fo_->ptr();
           if (!stabstr_) 
               PARSE_AOUT_DIE("Reading initialized debug section", 49);
       }
       
       linesptr_ = (void *) lines;
       linesfdptr_ = linesfdptr + offset;
   }
   else
   {
       // Not all files have debug information. Libraries tend not to.
       stabs_ = NULL;
       nstabs_ = 0;
       stringpool_ = NULL;
       stabstr_ = NULL;
       linesptr_ = NULL;
       nlines_ = 0;
       linesfdptr_ = 0;
   }
   
   no_of_symbols_ = hdr.f_nsyms;
   // Now the symbol table itself:
   for (i=0; i < hdr.f_nsyms; i++) 
   {
     /* do the pointer addition by hand since sizeof(struct syment)
      *   seems to be 20 not 18 as it should be. Mmm alignment. */
     sym = (struct syment *) (((unsigned) symbols) + i * SYMESZ);


     if (sym->n_sclass & DBXMASK) {
         continue;
     }
     
     secno = sym->n_scnum;
     if ((C_WEAKEXT && (sym->n_sclass == C_WEAKEXT)) ||
         (sym->n_sclass == C_HIDEXT) || 
         (sym->n_sclass == C_EXT) ||
         (sym->n_sclass == C_FILE)) {
         if (!sym->n_zeroes) {
             name = std::string(&stringPool[sym->n_offset]);
         } else {
             char tempName[9];
             memset(tempName, 0, 9);
             strncpy(tempName, sym->n_name, 8);
             name = std::string(tempName);
         }
     }

     if ((C_WEAKEXT && (sym->n_sclass == C_WEAKEXT)) ||
         (sym->n_sclass == C_HIDEXT) || 
         (sym->n_sclass == C_EXT)) {
         if (sym->n_sclass == C_HIDEXT) {
             linkage = Symbol::SL_LOCAL;
         } else {
             linkage = Symbol::SL_GLOBAL;
         }
         
         if (sym->n_scnum == aout.o_sntext) {
             type = Symbol::ST_FUNCTION;
             value = sym->n_value;
             /*
             fprintf(stderr, "Text symbol %s, at 0x%x\n",
                     name.c_str(), value);
             */
         } else {
             // bss or data
             csect = (union auxent *)
             ((char *) sym + sym->n_numaux * SYMESZ);
             
             if (csect->x_csect.x_smclas == XMC_TC0) { 
                 if (toc_offset);
                     //logLine("Found more than one XMC_TC0 entry.");
                 toc_offset = sym->n_value;
                 continue;
             }
             
           if ((csect->x_csect.x_smclas == XMC_TC) ||
               (csect->x_csect.x_smclas == XMC_DS)) {
               // table of contents related entry not a real symbol.
               //dump << " toc entry -- ignoring" << endl;
               continue;
           }
           type = Symbol::ST_OBJECT;

           if (foundData && sym->n_value < sectHdr[dataSecNo].s_paddr) {
               // Very strange; skip
               continue;
           }

           value = sym->n_value;


           // Shift it back down; data_org will be added back in later.

           //fprintf(stderr, "Sym %s, at 0x%x\n",
           //      name.c_str(), value);
       } 
       
       // skip .text entries
       if (name == ".text")  {
           continue;
       }
       
       if (name[0] == '.' ) {
           // XXXX - Hack to make names match assumptions of symtab.C
           name = std::string(name.c_str()+1);
       }
       else if (type == Symbol::ST_FUNCTION) {
           // text segment without a leading . is a toc item
           //dump << " (no leading . so assuming toc item & ignoring)" << endl;
           continue;
       }
       
       unsigned int size = 0;
       if (type == Symbol::ST_FUNCTION) {
           // Find address of inst relative to code_ptr_, instead of code_off_
           
           Word *inst = (Word *)((char *)code_ptr_ + value - code_off_);
           // If the instruction we got is a unconditional branch, flag it.
           // I've seen that in some MPI functions as a poor man's aliasing
           instructUnion instr;
           instr.raw = inst[0];
           if ((instr.iform.op == Bop) &&
               (instr.iform.lk == 0))
               size = 4; // Unconditional branch at the start of the func, no link
           else {
               while (inst[size] != 0) size++;
               size *= sizeof(Word);
           }
       }

       // AIX linkage code appears as a function. Since we don't remove it from
       // the whereaxis yet, I append a _linkage tag to each so that they don't
       // appear as duplicate functions

       // 2/07 rutar - There are some good usages for having the _linkage functions
       // appear (such as distinguishing program information by the names of 
       // functions being callsed so I put these functions back in the mix ...
       // the name "_linkage" is appended so that the difference is noted
       
       // Template for linkage functions:
       // l      r12,<offset>(r2) // address of call into R12
       // st     r2,20(r1)        // Store old TOC on the stack
       // l      r0,0(r12)        // Address of callee func
       // l      r2,4(r12)        // callee TOC
       // mtctr  0                // We keep the LR static, use the CTR
       // bctr                    // non-saving branch to CTR
       
       if (size == 0x18) {
           // See if this is linkage code, and if so skip (so it doesn't go in the
           // list of functions). 
           Word *inst = (Word *)((char *)code_ptr_ + value - code_off_);
           instructUnion lr12, lr0, bctr;
           lr12.raw = inst[0];
           lr0.raw = inst[2];
           bctr.raw = inst[5];

           if ((lr12.dform.op == Lop) && (lr12.dform.rt == 12) && (lr12.dform.ra == 2) &&
                (lr0.dform.op == Lop) && (lr0.dform.rt == 0) &&
                (lr0.dform.ra == 1 || lr0.dform.ra == 12) &&
               (bctr.xlform.op == BCLRop) && (bctr.xlform.xo == BCCTRxop))
	     {
	       int sourceSize = strlen(name.c_str());
	       char tempLinkName[sourceSize + 9];
	       memset(tempLinkName, 0, sourceSize+8);
	       strncpy(tempLinkName, name.c_str(),sourceSize);
	       strcat(tempLinkName,"_linkage"); 
	       name = std::string(tempLinkName);
	     }
       }

       /*giri: Dyninst related. Moved there.
       // HACK. This avoids double-loading various tramp spaces
       if (name.prefixed_by("DYNINSTstaticHeap") &&
           size == 0x18) {
           continue;
       }*/
       

#if 0
       // We ignore the symbols for the floating-point and general purpose
       // register save/restore macros
       // Not any more - 20APR06, bernat
       // With nate's "hunt it all down" parsing we find these anyway; might
       // as well give 'em a name.
       
       if (name.prefixed_by("_savef") ||
           name.prefixed_by("_restf") ||
           name.prefixed_by("_savegpr") ||
           name.prefixed_by("_restgpr") ||
           name.prefixed_by("$SAVEF") ||
           name.prefixed_by("$RESTF") ||
           name.prefixed_by("Ssavef") ||
           name.prefixed_by("Srestf") ||
           name == "fres" ||
           name == "fsav")
           continue;
#endif       
       
//       if( name == "main" )
//           foundMain = true;
//       if( name == "_start" )
//           foundStart = true;

       //parsing_printf("Symbol %s, addr 0x%lx, mod %s, size %d\n",
       //              name.c_str(), value, modName.c_str(), size);
       Section *sec;
       if(secno >= 1 && secno <= sections_.size())
    sec = sections_[secno-1];
       else
           sec = NULL;
       Symbol *sym = new Symbol(name, modName, type, linkage, value, sec, size);
       
       // If we don't want the function size for some reason, comment out
       // the above and use this:
       // Symbol sym(name, modName, type, linkage, value, false);
       // fprintf( stderr, "Added symbol %s at addr 0x%x, size 0x%x, module %s\n", name.c_str(), value, size, modName.c_str());
       
       symbols_[name].push_back( sym );
       if (symbols_.find(modName)!=symbols_.end()) {
           // Adjust module's address, if necessary, to ensure that it's <= the
           // address of this new symbol
           
           std::vector< Symbol *> & mod_symbols = symbols_[modName];
           
#if defined( DEBUG )
           if( mod_symbols.size() != 1 ) {
               fprintf( stderr, "%s[%d]: module name has more than one symbol:\n", __FILE__, __LINE__  );
               for( unsigned int i = 0; i < mod_symbols.size(); i++ ) {
                   cerr << *(mod_symbols[i]) << endl;
                   }
               fprintf( stderr, "\n" );
               }
#endif /* defined( DEBUG ) */               

           Symbol *mod_symbol = mod_symbols[ 0 ];
           
           if (value < mod_symbol->getAddr()) {
               //cerr << "adjusting addr of module " << modName
               //     << " to " << value << endl;
               mod_symbol->setAddr(value);
           }
 }  
     } else if (sym->n_sclass == C_FILE) {
         if (!strcmp(name.c_str(), ".file")) {
             int j;
             /* has aux record with additional information. */
             for (j=1; j <= sym->n_numaux; j++) {
                 aux = (union auxent *) ((char *) sym + j * SYMESZ);
                 if (aux->x_file._x.x_ftype == XFT_FN) {
                     // this aux record contains the file name.
                     if (!aux->x_file._x.x_zeroes) {
                         name = std::string(&stringPool[aux->x_file._x.x_offset]);
                     } else {
                         // x_fname is 14 bytes
                         char tempName[15];
                         memset(tempName, 0, 15);
                         strncpy(tempName, aux->x_file.x_fname, 14);
                         name = std::string(tempName);
                     }
                 }
             }
         }

         //dump << "found module \"" << name << "\"" << endl;
         
         // Hack time. Break it down
         // Problem: libc and others show up as file names. So if the
         // file being loaded is a .a (it's a hack, remember?) use the
         // .a as the modName instead of the symbol we just found.
  std::string::size_type len = file_.length(); 
         if (((len>2)&&(file_.substr(len-2,2)==".a")) ||
             ((len>3)&&(file_.substr(len-3,3)==".so")) ||
             ((len>5)&&(file_.substr(len-5,5)==".so.1")))
             modName = file_;
         else if (name == "glink.s")
             modName = std::string("Global_Linkage");
         else {
             modName = name;
         }
         Symbol *modSym = new Symbol(modName, modName,
                             Symbol::ST_MODULE, linkage,
                             UINT_MAX // dummy address for now!
                             );
                             
         /* The old code always had the last module win. */
         if( symbols_[modName].size() == 0 ) {
          symbols_[modName].push_back( modSym );
          } else {
         symbols_[modName][0] = modSym;
         }
         
         continue;
     }
   }
   
   toc_offset_ = toc_offset;
      
   code_vldS_ = code_off_;
   code_vldE_ = code_off_ + code_len_;
   data_vldS_ = data_off_;
   data_vldE_ = data_off_ + data_len_;

   return;
}

// Archive parsing
// Libraries on AIX can be archive files. These files are distinguished
// by their magic number, and come in two types: 32-bit (small) and 
// 64-bit (big). The structure of an either archive file is similar:
// <Archive header> (magic number, offsets)
// <Member header>  (member file name)
//   <member file>
// <Member header>
//   <member file> 
// and so on. Given a member name, we scan the archive until we find
// that name, at which point parse_aout is called.
// The only difference between small and big archive is the size of
// the archive/member header variables (12 byte vs. 20 byte).
// Both archives are handled in one parsing function, which keys off
// the magic number of the archive.
// Note: all data in the headers is in ASCII.

// More macros
#define PARSE_AR_DIE(errType, errCode) { \
      sprintf(errorLine, "Error parsing archive file %s: %s\n", \
              file_.c_str(), errType); \
      err_func_(errType); \
      return; \
      }
   
void Object::load_archive(bool is_aout) {
    xcoffArchive *archive;
    
    // Determine archive type
    // Start at the beginning...
    if (!fo_->set(0))
        PARSE_AR_DIE("Seeking to file start", 49);

    char magic_number[SAIAMAG];
    if (!fo_->read(magic_number, SAIAMAG))
        PARSE_AR_DIE("Reading magic number", 49);

    if (!strncmp(magic_number, AIAMAG, SAIAMAG))
        archive = (xcoffArchive *) new xcoffArchive_32(fo_);
    else if (!strncmp(magic_number, AIAMAGBIG, SAIAMAG))
        archive = (xcoffArchive *) new xcoffArchive_64(fo_);
    else
        PARSE_AR_DIE("Unknown magic number", 49);
    
    if (archive->read_arhdr())
        PARSE_AR_DIE("Reading file header", 49);
    
    bool found = false;
    
    while (archive->next_offset != 0)
    {
        if (archive->read_mbrhdr())
            PARSE_AR_DIE("Reading member header", 49);

        if (!strncmp(archive->member_name,
                     member_.c_str(),
                     archive->member_len - 1)) 
 {
            found = true;
            break; // Got the right one
        }
    }
    if (found) // found the right member
    {
            // At this point, we should be able to read the a.out
            // file header.
            parse_aout(archive->aout_offset, is_aout);
    }
    else
    {
     //Return error for not an ordinary file. Should have had a membername/offset if it was one.
 //probably reached here because openFile() was called instead of openArchive.
    }
}

// This is our all-purpose-parse-anything function. 
// Takes a file and determines from the first two bytes the
// file type (archive or a.out). Assumes that two bytes are
// enough to identify the file format. 

void Object::load_object()
{
    // Load in an object (archive, object, .so)

    assert(fo_);

    unsigned char magic_number[2];
    unsigned short f_flags;
    
    if (!fo_->set(0)) {
        sprintf(errorLine, "Error reading file %s\n", 
                file_.c_str());
 err_func_(errorLine); 
        //statusLine(errorLine);
        //showErrorCallback(49,(const char *) errorLine);
        return;
    }

    if (!fo_->read((void *)magic_number, 2)) {
        sprintf(errorLine, "Error reading file %s\n", 
                file_.c_str());
 err_func_(errorLine); 
        //statusLine(errorLine);
        //showErrorCallback(49,(const char *) errorLine);
        return;
    }

    if (!fo_->set(18)){
        sprintf(errorLine, "Error moving to the offset 18 file %s\n", 
                file_.c_str());
 err_func_(errorLine); 
        //statusLine(errorLine);
        //showErrorCallback(49,(const char *) errorLine);
    }   
       
    if (!fo_->read((void *)(&f_flags), 2)) {
        sprintf(errorLine, "Error reading file %s\n", 
                file_.c_str());
 err_func_(errorLine); 
        //statusLine(errorLine);
        //showErrorCallback(49,(const char *) errorLine);
        return;
    }
    
    // a.out file: magic number = 0x01df
    // archive file: magic number = 0x3c62 "<b", actually "<bigaf>"
    // or magic number = "<a", actually "<aiaff>"
    if (magic_number[0] == 0x01) {
        if (magic_number[1] == 0xdf)
 {
     if(f_flags & F_SHROBJ)
     {
  is_aout_ = false;
             parse_aout(0, false);
     }
     else
     {
  is_aout_ = true;
      parse_aout(0,true);
     } 
 }    
        else; 
            //parse_aout_64(fd, 0);
            //bperr( "Don't handle 64 bit files yet");
    }
    else if (magic_number[0] == '<') // archive of some sort
    {
     if(f_flags & F_SHROBJ)
 {
     //load_archive(false);
     is_aout_ = false;
     parse_aout(offset_, false);
 }    
 else
 {
     //load_archive(true);
     is_aout_ = true;
     parse_aout(offset_, true);
 }    
    } 
    else {// Fallthrough
        sprintf(errorLine, "Bad magic number in file %s\n",
                file_.c_str());
 err_func_(errorLine); 
        //statusLine(errorLine);
        //showErrorCallback(49,(const char *) errorLine);
    }
    return;
}

// There are three types of "shared" files:
// archives (made with ar, end with .a)
// objects (ld -bM:SRE)
// new-style shared objects (.so)
// load_shared_object determines from magic number which to use
// since static objects are just a.outs, we can use the same
// function for all

Object::Object(const Object& obj)
    : AObject(obj) {
    // You know, this really should never be called, but be careful.
    if(obj.fo_->file() != "") 
     fo_ = fileOpener::openFile(obj.fo_->file());
    else
     fo_ = fileOpener::openFile((void *)obj.fo_->mem_image(), obj.fo_->size());
    assert(0);
}


Object::Object(std::string &filename, void (*err_func)(const char *))
    : AObject(filename, err_func)
{    
    loadNativeDemangler();
    fo_ = fileOpener::openFile(filename);
    assert(fo_);
    load_object(); 
}

Object::Object(char *mem_image, size_t image_size, void (*err_func)(const char *))
    : AObject(NULL, err_func)
{
    loadNativeDemangler();
    fo_ = fileOpener::openFile((void *)mem_image, image_size);
    assert(fo_);
    load_object();
}

Object::Object(std::string &filename, std::string &member_name, Offset offset, void (*err_func)(const char *))
    : AObject(filename, err_func), member_(member_name), offset_(offset)
{    
    loadNativeDemangler();
    fo_ = fileOpener::openFile(filename);
    assert(fo_);
    load_object(); 
}

Object::Object(char *mem_image, size_t image_size, std::string &member_name, Offset offset, void (*err_func)(const char *))
    : AObject(NULL, err_func), member_(member_name), offset_(offset)
{
    loadNativeDemangler();
    fo_ = fileOpener::openFile((void *)mem_image, image_size);
    assert(fo_);
    load_object();
}

Object& Object::operator=(const Object& obj) {
    (void) AObject::operator=(obj);
    if(obj.fo_->file() != "") 
     fo_ = fileOpener::openFile(obj.fo_->file());
    else
     fo_ = fileOpener::openFile((void *)obj.fo_->mem_image(), obj.fo_->size());
    return *this;
}


void Object::get_stab_info(char *&stabstr, int &nstabs, void *&stabs, char *&stringpool) const {
  stabstr = (char *) stabstr_;
  nstabs = nstabs_;
  stabs = stabs_;
  stringpool = (char *) stringpool_;
}

//
// parseCompilerType - parse for compiler that was used to generate object
// return true for "native" compiler
//
//      XXX - This really should be done on a per module basis
//
bool parseCompilerType(Object *objPtr) 
{

    SYMENT *syms;
    int stab_nsyms;
    char *stringPool;
    union auxent *aux;
    char *stabstr=NULL;

    void *syms_void = NULL;
    objPtr->get_stab_info(stabstr, stab_nsyms, syms_void, stringPool);
    syms = static_cast<SYMENT *>(syms_void);
    for (int i=0;i<stab_nsyms;i++) {
        SYMENT *sym = (SYMENT *) (((unsigned) syms) + i * SYMESZ);
        char tempName[15];
        char *compilerName;
        std::string name;
        if (sym->n_sclass == C_FILE) {
            if (!sym->n_zeroes) {
                name = std::string(&stringPool[sym->n_offset]);
            } else {
                char tempName[9];
                memset(tempName, 0, 9);
                strncpy(tempName, sym->n_name, 8);
                name = std::string(tempName);
            }
     if (!strcmp(name.c_str(), ".file")) {
                int j;
                /* has aux record with additional information. */
                for (j=1; j <= sym->n_numaux; j++) {
      aux = (union auxent *) ((char *) sym + j * SYMESZ);
      if (aux->x_file._x.x_ftype == XFT_CV) {
                        // this aux record contains the file name.
                        if (!aux->x_file._x.x_zeroes) {
                            compilerName = &stringPool[aux->x_file._x.x_offset];
                        } else {
                            // x_fname is 14 bytes
                            memset(tempName, 0, 15);
                            strncpy(tempName, aux->x_file.x_fname, 14);
                            compilerName = tempName;
                        }
                    }
         }
  //
  // Use presence of string "IBM VisualAge C++" to confirm
  //   it's the IBM compiler
  //
                char *compiler_strings[] = {
                   "IBM.*VisualAge.*C\\+\\+",
                   "IBM.* XL .*C\\+\\+",
                   NULL};
                for (char **cpp = compiler_strings; *cpp != NULL; cpp++) {
                   regex_t reg;
                   if (regcomp(&reg, *cpp, REG_NOSUB))
                      break;
                   if (!regexec(&reg, compilerName, 0, NULL, 0)) {
                      regfree(&reg);
                      return true;
                   }
                   regfree(&reg);
                }
       }
       }
    }
    // bperr("compiler is GNU\n");
    return false;
}

// Moved to here from Dyn_Symtab.C
void Object::getModuleLanguageInfo(hash_map<std::string, supportedLanguages> *)
{
}

ObjectType Object::objType() const {
   return is_aout() ? obj_Executable : obj_SharedLib;
}

bool Object::emitDriver(Symtab *obj, string fName, std::vector<Symbol *>&functions, std::vector<Symbol *>&variables, std::vector<Symbol *>&mods, std::vector<Symbol *>&notypes, unsigned flag) {
   return true;
}

bool AObject::getSegments(vector<Segment> &/*segs*/) const
{
   return true;
}

bool AObject::getMappedRegions(std::vector<Region> &regs) const
{
   Region reg;
   reg.addr = code_vldS_;
   reg.size = code_len_;
   reg.offset = code_off_;
   regs.push_back(reg);

   reg.addr = data_vldS_;
   reg.size = data_len_;
   reg.offset = data_off_;

   const Object *obj = dynamic_cast<const Object *>(this);
   if (obj) {
      reg.size += obj->bss_size();
   }

   regs.push_back(reg);
   
   return true;
}


/* FIXME: hack. */
Offset trueBaseAddress = 0;

void Object::parseFileLineInfo() {
    static set<std::string> haveParsedFileMap;

    cerr << "parsing line infor for file :" << file_ << endl;

    if( haveParsedFileMap.count(file_) != 0 ) { return; }
    // /* DEBUG */ fprintf( stderr, "%s[%d]: Considering image at 0x%lx\n", __FILE__, __LINE__, fileOnDisk );

    /* FIXME: hack.  Should be argument to parseLineInformation(), which should in turn be merged
       back into here so it can tell how far to extend the range of the last line information point. */
 
    //Offset baseAddress = obj()->codeBase();
    if(is_aout())
    	trueBaseAddress = 0;
    else
        trueBaseAddress = baseAddress_;

    /* We haven't parsed this file already, so iterate over its stab entries. */
    char * stabstr = NULL;
    int nstabs = 0;
    SYMENT * syms = 0;
    char * stringpool = NULL;
    void *syms_void = NULL;
    get_stab_info( stabstr, nstabs, syms_void, stringpool );
    syms = (SYMENT *) syms_void;
    int nlines = 0;
    char * lines = NULL;
    unsigned long linesfdptr;
    get_line_info( nlines, lines, linesfdptr );

    /* I'm not sure why the original code thought it should copy (short) names (through here). */
    char temporaryName[256];
    char * funcName = NULL;
    char * currentSourceFile = NULL;
    char * moduleName = NULL;

    /* Iterate over STAB entries. */
    for( int i = 0; i < nstabs; i++ ) {
     	/* sizeof( SYMENT ) is 20, not 18, as it should be. */
        SYMENT * sym = (SYMENT *)( (unsigned)syms + (i * SYMESZ) );

        /* Get the name (period) */
	if (!sym->n_zeroes) {
     	    moduleName = &stringpool[sym->n_offset];
	} else {
     	    memset(temporaryName, 0, 9);
     	    strncpy(temporaryName, sym->n_name, 8);
            moduleName = temporaryName;
 	}

        /* Extract the current source file from the C_FILE entries. */
        if( sym->n_sclass == C_FILE ) {
            if (!strcmp(moduleName, ".file")) {
  		// The actual name is in an aux record.
        	int j;
        	/* has aux record with additional information. */
        	for (j=1; j <= sym->n_numaux; j++) {
          	    union auxent *aux = (union auxent *) ((char *) sym + j * SYMESZ);
          	    if (aux->x_file._x.x_ftype == XFT_FN) {
            		// this aux record contains the file name.
            		if (!aux->x_file._x.x_zeroes) {
              		    moduleName = &stringpool[aux->x_file._x.x_offset];
            		} else {
              		    // x_fname is 14 bytes
              		    memset(temporaryName, 0, 15);
              		    strncpy(temporaryName, aux->x_file.x_fname, 14);
              		    moduleName = temporaryName;
            		}
          	    }
                }
            }

            currentSourceFile = strrchr( moduleName, '/' );
            if( currentSourceFile == NULL ) { currentSourceFile = moduleName; }
            else { ++currentSourceFile; }

            /* We're done with this entry. */
            continue;
        }/* end if C_FILE */

        /* This apparently compensates for a bug in the naming of certain entries. */
        char * nmPtr = NULL;
        if(! sym->n_zeroes && (
                                    ( sym->n_sclass & DBXMASK ) ||
                                    ( sym->n_sclass == C_BINCL ) ||
                                    ( sym->n_sclass == C_EINCL )
                                    ) ) {
            if( sym->n_offset < 3 ) {
                if( sym->n_offset == 2 && stabstr[ 0 ] ) {
          	    nmPtr = & stabstr[ 0 ];
            	} else {
          	    nmPtr = & stabstr[ sym->n_offset ];
            	}
            } else if( ! stabstr[ sym->n_offset - 3 ] ) {
            	nmPtr = & stabstr[ sym->n_offset ];
            } else {
            	/* has off by two error */
            	nmPtr = & stabstr[ sym->n_offset - 2 ];
            }
        } else {
      	    // names 8 or less chars on inline, not in stabstr
      	    memset( temporaryName, 0, 9 );
      	    strncpy( temporaryName, sym->n_name, 8 );
      	    nmPtr = temporaryName;
    	} /* end bug compensation */

    	/* Now that we've compensated for buggy naming, actually
       	parse the line information. */
    	if( ( sym->n_sclass == C_BINCL )
        	|| ( sym->n_sclass == C_EINCL )
        	|| ( sym->n_sclass == C_FUN ) ) {
      	    if( funcName ) {
        	free( funcName );
        	funcName = NULL;
      	    }
      	    funcName = strdup( nmPtr );

      	    std::string pdCSF( currentSourceFile );
      	    parseLineInformation(& pdCSF, funcName, (SYMENT *)sym, linesfdptr, lines, nlines );
    	} /* end if we're actually parsing line information */
    } /* end iteration over STAB entries. */

    if( funcName != NULL ) {
        free( funcName );
    }
    haveParsedFileMap.insert(file_);
} /* end parseFileLineInfo() */

void Object::parseLineInformation(std::string * currentSourceFile,
                                         char * symbolName,
                                         SYMENT * sym,
                                         Offset linesfdptr,
                                         char * lines,
                                         int nlines ) {
  union auxent * aux;
  std::vector<IncludeFileInfo> includeFiles;

  /* if it is beginning of include files then update the data structure
     that keeps the beginning of the include files. If the include files contain
     information about the functions and lines we have to keep it */
  if( sym->n_sclass == C_BINCL ) {
    includeFiles.push_back( IncludeFileInfo( (sym->n_value - linesfdptr)/LINESZ, symbolName ) );
  }
  /* similiarly if the include file contains function codes and line information
     we have to keep the last line information entry for this include file */
  else if( sym->n_sclass == C_EINCL ) {
    if( includeFiles.size() > 0 ) {
      includeFiles[includeFiles.size()-1].end = (sym->n_value-linesfdptr)/LINESZ;
    }
  }
  /* if the enrty is for a function than we have to collect all info
     about lines of the function */
  else if( sym->n_sclass == C_FUN ) {
    /* I have no idea what the old code did, except not work very well.
       Somebody who understands XCOFF should look at this. */
    int initialLine = 0;
    int initialLineIndex = 0;
    Offset funcStartAddress = 0;
    Offset funcEndAddress = 0;

    for( int j = -1; ; --j ) {
      SYMENT * extSym = (SYMENT *)( ((Offset)sym) + (j * SYMESZ) );
      if( extSym->n_sclass == C_EXT || extSym->n_sclass == C_HIDEXT ) {
        aux = (union auxent *)( ((Offset)extSym) + SYMESZ );
#ifndef __64BIT__
        initialLineIndex = ( aux->x_sym.x_fcnary.x_fcn.x_lnnoptr - linesfdptr )/LINESZ;
#endif
        funcStartAddress = extSym->n_value;
        break;
      } /* end if C_EXT found */
    } /* end search for C_EXT */

    /* access the line information now using the C_FCN entry*/
    SYMENT * bfSym = (SYMENT *)( ((Offset)sym) + SYMESZ );
    if( bfSym->n_sclass != C_FCN ) {
      //bperr("unable to process line info for %s\n", symbolName);
      return;
    }
    SYMENT * efSym = (SYMENT *)( ((Offset)bfSym) + (2 * SYMESZ) );
    while (efSym->n_sclass != C_FCN)
      efSym = (SYMENT *) ( ((Offset)efSym) + SYMESZ );
    funcEndAddress = efSym->n_value;

    aux = (union auxent *)( ((Offset)bfSym) + SYMESZ );
    initialLine = aux->x_sym.x_misc.x_lnsz.x_lnno;

    std::string whichFile = *currentSourceFile;
    for( unsigned int j = 0; j < includeFiles.size(); j++ ) {
      if(( includeFiles[j].begin <= (unsigned)initialLineIndex )
                && ( includeFiles[j].end >= (unsigned)initialLineIndex ) ) {
        whichFile = includeFiles[j].name;
        break;
      }
    } /* end iteration of include files */

#if 0    
    int_function * currentFunction = obj()->findFuncByAddr( funcStartAddress + trueBaseAddress );
    if( currentFunction == NULL ) {
      /* Some addresses point to gdb-inaccessible memory; others have symbols (gdb will disassemble them)
         but the contents look like garbage, and may be data with symbol names.  (Who knows why.) */
      // fprintf( stderr, "%s[%d]: failed to find function containing address 0x%lx; line number information will be lost.\n", __FILE__, __LINE__, funcStartAddress + trueBaseAddress );
      return;
    }
    mapped_module * currentModule = currentFunction->mod();
    assert( currentModule != NULL );
    LineInformation & currentLineInformation = currentModule->lineInfo_; 
#endif

    unsigned int previousLineNo = 0;
    Offset previousLineAddr = 0;
    bool isPreviousValid = false;

    /* Iterate over this entry's lines. */
    for( int j = initialLineIndex + 1; j < nlines; j++ ) {
      LINENO * lptr = (LINENO *)( lines + (j * LINESZ) );
      if( ! lptr->l_lnno ) { break; }
      unsigned int lineNo = lptr->l_lnno + initialLine - 1;
      Offset lineAddr = lptr->l_addr.l_paddr + trueBaseAddress;

      if( isPreviousValid ) {
        // /* DEBUG */ fprintf( stderr, "%s[%d]: adding %s:%d [0x%lx, 0x%lx).\n", __FILE__, __LINE__, whichFile.c_str(), previousLineNo, previousLineAddr, lineAddr );
        unsigned current_col = 0;
	if(previousLineNo == 596 || previousLineNo == 597)
	{
		cerr << "FuncEndAddress: " <<setbase(16) << funcEndAddress << setbase(10) << ",totallines:" << nlines << ":" << endl;
		cerr << __FILE__ <<"[" << __LINE__ << "]:inserted address range [" << setbase(16) << previousLineAddr << "," << lineAddr << ") for source " << whichFile << ":" << setbase(10) << previousLineNo << endl;
	}	
	lineInfo_[whichFile].addLine(whichFile.c_str(), previousLineNo, current_col, previousLineAddr, lineAddr );
        //currentLineInformation.addLine( whichFile.c_str(), previousLineNo, current_col, previousLineAddr, lineAddr );
      }

      previousLineNo = lineNo;
      previousLineAddr = lineAddr;
      isPreviousValid = true;
    } /* end iteration over line information */

    if( isPreviousValid ) {
      /* Add the instruction (always 4 bytes on power) pointed at by the last entry.  We'd like to add a
         bigger range, but it's not clear how.  (If the function has inlined code, we won't know about
         it until we see the next section, so claiming "until the end of the function" will give bogus results.) */
      // /* DEBUG */ fprintf( stderr, "%s[%d]: adding %s:%d [0x%lx, 0x%lx).\n", __FILE__, __LINE__, whichFile.c_str(), previousLineNo, previousLineAddr, previousLineAddr + 4 );
      while (previousLineAddr < funcEndAddress) {
        unsigned current_col = 0;
	if(previousLineNo == 596 || previousLineNo == 597)
		cerr << __FILE__ <<"[" << __LINE__ << "]:inserted address range [" << setbase(16) << previousLineAddr << "," <<  previousLineAddr + 4 << ") for source " << whichFile << ":" << setbase(10) << previousLineNo << endl;
	lineInfo_[whichFile].addLine(whichFile.c_str(), previousLineNo, current_col, previousLineAddr,  previousLineAddr + 4);
        //currentLineInformation.addLine( whichFile.c_str(), previousLineNo, current_col, previousLineAddr, previousLineAddr + 4 );
        previousLineAddr += 4;
      }
    }
  } /* end if we found a C_FUN symbol */
} /* end parseLineInformation() */


// Gets the stab and stabstring section and parses it for types
// and variables
void Object::parseTypeInfo(Symtab *obj)
{
    int i, j;
    int nstabs;
    SYMENT *syms;
    SYMENT *tsym;
    char *stringPool;
    char tempName[9];
    char *stabstr=NULL;
    union auxent *aux;
    std::string funcName;
    Offset staticBlockBaseAddr = 0;
    typeCommon *commonBlock = NULL;
    Symbol *commonBlockVar = NULL;
    std::string currentSourceFile;
    bool inCommonBlock = false;
    Module *mod = NULL;
    
    void *syms_void = NULL;
    get_stab_info(stabstr, nstabs, syms_void, stringPool);
    syms = (SYMENT *) syms_void;

    bool parseActive = true;
    //fprintf(stderr, "%s[%d]:  parseTypes for module %s: nstabs = %d\n", FILE__, __LINE__,mod->fileName().c_str(),nstabs);
    //int num_active = 0;

     for (i=0; i < nstabs; i++) {
         /* do the pointer addition by hand since sizeof(struct syment)
	  *   seems to be 20 not 18 as it should be */
	 SYMENT *sym = (SYMENT *) (((unsigned) syms) + i * SYMESZ);
	 if (sym->n_sclass == C_FILE) {
	     char *moduleName;
	     if (!sym->n_zeroes) {
	          moduleName = &stringPool[sym->n_offset];
	     } else {
	          memset(tempName, 0, 9);
	          strncpy(tempName, sym->n_name, 8);
	          moduleName = tempName;
	     }
	     /* look in aux records */
             for (j=1; j <= sym->n_numaux; j++) {
                 aux = (union auxent *) ((char *) sym + j * SYMESZ);
                 if (aux->x_file._x.x_ftype == XFT_FN) {
                     if (!aux->x_file._x.x_zeroes) {
                         moduleName = &stringPool[aux->x_file._x.x_offset];
                     } else {
			 memset(moduleName, 0, 15);
			 strncpy(moduleName, aux->x_file.x_fname, 14);
                   }
               }
	    }
	    currentSourceFile = std::string(moduleName);
	    if(!obj->findModule(mod, currentSourceFile))
	    {
	    	if(!obj->findModule(mod,extract_pathname_tail(currentSourceFile)))
		    continue;
	    }

	    //TODO? check for process directories??
            //currentSourceFile = mod->processDirectories(currentSourceFile);

            if (strrchr(moduleName, '/')) {
	        moduleName = strrchr(moduleName, '/');
                moduleName++;
            }

            if (!strcmp(moduleName, mod->fileName().c_str())) {
                parseActive = true;
                // Clear out old types
                mod->getModuleTypes()->clearNumberedTypes();
	   }	
	   else { 
	       parseActive = false;
           }
     }
     if (!parseActive) continue;

     //num_active++;
     char *nmPtr;
     if (!sym->n_zeroes && ((sym->n_sclass & DBXMASK) ||
                            (sym->n_sclass == C_BINCL) ||
                            (sym->n_sclass == C_EINCL))) {
         if(sym->n_offset < 3) {
	     if (sym->n_offset == 2 && stabstr[0]) {
                 nmPtr = &stabstr[0];
             } else
                 nmPtr = &stabstr[sym->n_offset];
         } else if (!stabstr[sym->n_offset-3]) {
	     nmPtr = &stabstr[sym->n_offset];
	 } else {
             /* has off by two error */
             nmPtr = &stabstr[sym->n_offset-2];
	 }    
      #if 0	 
      #ifdef notdef
         bperr("using nmPtr = %s\n", nmPtr);
         bperr("got n_offset = (%d) %s\n", sym->n_offset, &stabstr[sym->n_offset]);
         if (sym->n_offset>=2)
             bperr("got n_offset-2 = %s\n", &stabstr[sym->n_offset-2]);
         if (sym->n_offset>=3)
             bperr("got n_offset-3 = %x\n", stabstr[sym->n_offset-3]);
         if (sym->n_offset>=4)
             bperr("got n_offset-4 = %x\n", stabstr[sym->n_offset-4]);
      #endif
      #endif
     } else {
         // names 8 or less chars on inline, not in stabstr
	 memset(tempName, 0, 9);
	 strncpy(tempName, sym->n_name, 8);
	 nmPtr = tempName;
     }
     if ((sym->n_sclass == C_BINCL) ||
          (sym->n_sclass == C_EINCL) ||
          (sym->n_sclass == C_FUN)) 
     {
         funcName = nmPtr;
         /* The call to parseLineInformation(), below, used to modify the symbols passed to it. */
         if (funcName.find(":") < funcName.length())
             funcName = funcName.substr(0,funcName.find(":"));
      }
      if (sym->n_sclass & DBXMASK) {
	  if (sym->n_sclass == C_BCOMM) {
	      char *commonBlockName;

              inCommonBlock = true;
	      commonBlockName = nmPtr;
		
	      std::vector<Symbol *>vars;
	      if(!obj->findSymbolByType(vars, commonBlockName, Symbol::ST_OBJECT))
	      {
	      	  if(!obj->findSymbolByType(vars, commonBlockName, Symbol::ST_OBJECT, true))
		      commonBlockVar = NULL;
	          else
	      	      commonBlockVar = vars[0];
	      }
	      else
	      	  commonBlockVar = vars[0];
	      
	      std::string cbName = commonBlockName;
	      if (!commonBlockVar) {
		  //bperr("unable to find variable %s\n", commonBlockName);
	      } else {
		  commonBlock = dynamic_cast<typeCommon *>(mod->getModuleTypes()->findVariableType(cbName));
		  if (commonBlock == NULL) {
		      // its still the null type, create a new one for it
		      //TODO? ? ID for this typeCommon ?
		      commonBlock = new typeCommon(cbName);
		      mod->getModuleTypes()->addGlobalVariable(cbName, commonBlock);
	          }
		  // reset field list
		  commonBlock->beginCommonBlock();
	      }
	  } else if (sym->n_sclass == C_ECOMM) {
             inCommonBlock = false;
             if (commonBlock == NULL)
                continue;

	    // copy this set of fields
	    std::vector<Symbol *> bpmv;
   	    if (obj->findSymbolByType(bpmv, funcName, Symbol::ST_FUNCTION) || !bpmv.size()) {
	      //bperr("unable to locate current function %s\n", funcName.c_str());
	      } else {
		Symbol *func = bpmv[0];
		commonBlock->endCommonBlock(func, (void *)obj->getBaseOffset());
	      }

 //TODO?? size for local variables??
//	      // update size if needed
//	      if (commonBlockVar)
//		  commonBlockVar->setSize(commonBlock->getSize());
	      commonBlockVar = NULL;
	      commonBlock = NULL;
	  } else if (sym->n_sclass == C_BSTAT) {
	      // begin static block
	      // find the variable for the common block
	      tsym = (SYMENT *) (((unsigned) syms) + sym->n_value * SYMESZ);

	      // We can't lookup the value by name, because the name might have been
	      // redefined later on (our lookup would then pick the last one)

	      // Since this whole function is AIX only, we're ok to get this info

	      staticBlockBaseAddr = tsym->n_value;

	      /*
	      char *staticName, tempName[9];
	      if (!tsym->n_zeroes) {
		  staticName = &stringPool[tsym->n_offset];
	      } else {
		  memset(tempName, 0, 9);
		  strncpy(tempName, tsym->n_name, 8);
		  staticName = tempName;
	      }

	      BPatch_variableExpr *staticBlockVar = progam->findVariable(staticName);
	      if (!staticBlockVar) {
		  bperr("unable to find static block %s\n", staticName);
		  staticBlockBaseAddr = 0;
	      } else {
		  staticBlockBaseAddr = (Offset) staticBlockVar->getBaseAddr();
	      }
	      */

	  } else if (sym->n_sclass == C_ESTAT) {
	      staticBlockBaseAddr = 0;
	  }

          // There's a possibility that we were parsing a common block that
          // was never instantiated (meaning there's type info, but no
          // variable info

          if (inCommonBlock && commonBlock == NULL)
             continue;
    
      if(!mod){
        std::string modName = currentSourceFile;
        std::string fName = extract_pathname_tail(modName);
        if(!obj->findModule(mod, fName))
        {
            modName = obj->file();
            if(!obj->findModule(mod, modName))
                continue;
        }
      }

	  if (staticBlockBaseAddr && (sym->n_sclass == C_STSYM)) {
              parseStabString(mod, 0, nmPtr, 
		  sym->n_value+staticBlockBaseAddr, commonBlock);
	  } else {
              parseStabString(mod, 0, nmPtr, sym->n_value, commonBlock);
	  }
      }
    }
#if defined(TIMED_PARSE)
  struct timeval endtime;
  gettimeofday(&endtime, NULL);
  unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
  unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
  unsigned long difftime = lendtime - lstarttime;
  double dursecs = difftime/(1000 );
  cout << __FILE__ << ":" << __LINE__ <<": parseTypes("<< obj->exec()->file()
       <<") took "<<dursecs <<" msecs" << endl;
#endif

//  fprintf(stderr, "%s[%d]:  parseTypes for %s, num_active = %d\n", FILE__, __LINE__, mod->fileName().c_str(), num_active);
}
