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
 * excluded
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

// $Id: Object-xcoff.C,v 1.44 2005/10/11 07:14:25 jodom Exp $

#include "common/h/headers.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/stats.h"
#include "common/h/Types.h"
#include "common/h/Dictionary.h"
#include "dyninstAPI/src/Object.h"
#include "dyninstAPI/src/instP.h" // class instInstance
#include "common/h/pathName.h"

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

#include <xcoff.h>
#define __AR_BIG__
#define __AR_SMALL__
#include <ar.h> // archive file format.

#include "dyninstAPI/src/showerror.h"
#include "common/h/debugOstream.h"
#include "arch.h"

#if defined(AIX_PROC)
#include <sys/procfs.h>
#else
#include <sys/ptrace.h>
#endif

/* For some reason this symbol type isn't global */
#if !defined(C_WEAKEXT)
#define C_WEAKEXT 0
#endif

unsigned long roundup4(unsigned long val) {
   while (val % 4 != 0)
      val++;
   return val;
}

// Methods to read file and ar header for both small (32-bit) and
// large (64-bit) archive files. This lets us have a single archive
// parsing method.
int Archive_32::read_arhdr()
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

int Archive_64::read_arhdr()
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

int Archive_32::read_mbrhdr()
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
int Archive_64::read_mbrhdr()
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

pdvector<fileOpener *> fileOpener::openedFiles;

fileOpener *fileOpener::openFile(const pdstring &f) {
    for (unsigned i = 0; i < openedFiles.size(); i++) {
        if (openedFiles[i]->file() == f) {
            openedFiles[i]->refcount_++;
            return openedFiles[i];
        }
    }

    // New file. Neeefty.
    fileOpener *newFO = new fileOpener(f);
    assert(newFO);
    
    if (!newFO->open()) {
        fprintf(stderr, "File %s\n", f.c_str());
        perror("Opening file");
        return NULL;
    }
    if (!newFO->mmap()) {
        fprintf(stderr, "File %s\n", f.c_str());
        perror("mmaping file");
        return NULL;
    }
    openedFiles.push_back(newFO);

    return newFO;
}

bool fileOpener::open() {
    if (fd_ != 0)
        return true;
    // Open ze file....
    fd_ = ::open(fileName_.c_str(), O_RDONLY, 0); 
    if (fd_ < 0) {
        sprintf(errorLine, "Unable to open %s: %s\n",
                fileName_.c_str(), strerror(errno));
        statusLine(errorLine);
        showErrorCallback(27, errorLine);
        fd_ = 0;
        return false;
    }

    // Set size
    struct stat statBuf;
    int ret;
    ret = fstat(fd_, &statBuf);
    if (ret == -1) {
        sprintf(errorLine, "Unable to stat %s: %s\n",
                fileName_.c_str(), strerror(errno));
        statusLine(errorLine);
        showErrorCallback(27, errorLine);
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
                fileName_.c_str(), strerror(errno));
        statusLine(errorLine);
        showErrorCallback(27, errorLine);
        mmapStart_ = NULL;
        return false;
    }

    return true;
}
    
bool fileOpener::set(unsigned addr) {
    assert(fd_);
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
    assert(fd_);
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
      sprintf(errorLine, "Error parsing a.out file %s(%s): %s \n", \
              file_.c_str(), member_.c_str(), errType); \
      statusLine(errorLine); \
      showErrorCallback(errCode,(const char *) errorLine); \
      return; \
      }

void Object::parse_aout(int offset, bool is_aout)
{
   // all these vrble declarations need to be up here due to the gotos,
   // which mustn't cross vrble initializations.  Too bad.
   long i,j;
   pdstring name;
   bool foundMain = false;
   bool foundStart = false;
   unsigned value;
   union auxent *aux;
   struct filehdr hdr;
   struct syment *sym;
   struct aouthdr aout;
   union auxent *csect;
   char *stringPool=NULL;
   Symbol::SymbolType type; 
   bool foundDebug = false;

   struct syment *symbols = NULL;
   struct scnhdr *sectHdr = NULL;
   Symbol::SymbolLinkage linkage = Symbol::SL_UNKNOWN;
   unsigned toc_offset = 0;
   pdstring modName;

   int linesfdptr=0;
   struct lineno* lines=NULL;

   // Get to the right place in the file (not necessarily 0)
   if (!fo_->set(offset))
       PARSE_AOUT_DIE("Seeking to correct offset", 49);

   if (!fo_->read(&hdr, sizeof(struct filehdr)))
       PARSE_AOUT_DIE("Reading file header", 49);

   if (hdr.f_magic == 0x1ef) {
       // XCOFF64 file! We don't handle those yet.
         bperr("Unhandled XCOFF64 header found!\n");
         return;
   }
   
   if (hdr.f_magic != 0x1df) {
       bperr( "Possible problem, magic number is %x, should be %x\n",
              hdr.f_magic, 0x1df);
   }
   
   if (!fo_->read(&aout, sizeof(struct aouthdr)))
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
   if ((unsigned) aout.text_start != sectHdr[aout.o_sntext-1].s_paddr)
       PARSE_AOUT_DIE("Checking text address", 49);
   if ((unsigned) aout.tsize != sectHdr[aout.o_sntext-1].s_size) 
       PARSE_AOUT_DIE("Checking text size", 49);
   if ((unsigned) aout.data_start != sectHdr[aout.o_sndata-1].s_paddr) 
       PARSE_AOUT_DIE("Checking data address", 49);
   if ((unsigned long) aout.dsize != sectHdr[aout.o_sndata-1].s_size)
       PARSE_AOUT_DIE("Checking data size", 49);

    /*
    * Get the pdstring pool, if there is one
    */
   if( hdr.f_nsyms ) 
   {
       // We want to go after the symbol table...
       if (!fo_->set(offset + hdr.f_symptr + (hdr.f_nsyms*SYMESZ)))
           PARSE_AOUT_DIE("Could not seek to string pool", 49);
       Address stringPoolSize;
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
   for (i=0; i < hdr.f_nscns; i++)
   {
       if (sectHdr[i].s_flags & STYP_TEXT) {
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
           break;
       }
   }

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
   Address fileTextOffset = roundup4(sectHdr[aout.o_sntext-1].s_scnptr);
   Address fileDataOffset = roundup4(sectHdr[aout.o_sndata-1].s_scnptr);

   if (!fo_->set(fileTextOffset + offset))
       PARSE_AOUT_DIE("Seeking to start of text segment", 49);
   code_ptr_ = (Word *)fo_->ptr();
   if (!code_ptr_)
       PARSE_AOUT_DIE("Reading text segment", 49);

   if (!fo_->set(fileDataOffset + offset))
       PARSE_AOUT_DIE("Seeking to start of data segment", 49);
   data_ptr_ = (Word *)fo_->ptr();
   if (!code_ptr_)
       PARSE_AOUT_DIE("Reading data segment", 49);

   // Offsets; symbols will be offset from the virtual address. Grab
   // that now.  These are defined (and asserted, above) to be equal
   // to the s_paddr (and s_vaddr) fields in their respective
   // sections.
   code_off_ = roundup4(aout.text_start);
   data_off_ = roundup4(aout.data_start);

   // As above, these are equal to the s_size fields in the respective fields.
   code_len_ = aout.tsize;
   data_len_ = aout.dsize;
   
   //FIND LOADER INFO!

   loader_off_ = sectHdr[aout.o_snloader-1].s_scnptr;
   loader_len_ = sectHdr[aout.o_snloader-1].s_size; 

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
   
   // Now the symbol table itself:
   for (i=0; i < hdr.f_nsyms; i++) 
   {
     /* do the pointer addition by hand since sizeof(struct syment)
      *   seems to be 20 not 18 as it should be. Mmm alignment. */
     sym = (struct syment *) (((unsigned) symbols) + i * SYMESZ);


     if (sym->n_sclass & DBXMASK) {
         continue;
     }
     
     if ((C_WEAKEXT && (sym->n_sclass == C_WEAKEXT)) ||
         (sym->n_sclass == C_HIDEXT) || 
         (sym->n_sclass == C_EXT) ||
         (sym->n_sclass == C_FILE)) {
         if (!sym->n_zeroes) {
             name = pdstring(&stringPool[sym->n_offset]);
         } else {
             char tempName[9];
             memset(tempName, 0, 9);
             strncpy(tempName, sym->n_name, 8);
             name = pdstring(tempName);
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
             type = Symbol::PDST_FUNCTION;
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
                 if (toc_offset)
                     logLine("Found more than one XMC_TC0 entry.");
                 toc_offset = sym->n_value;
                 continue;
             }
             
           if ((csect->x_csect.x_smclas == XMC_TC) ||
               (csect->x_csect.x_smclas == XMC_DS)) {
               // table of contents related entry not a real symbol.
               //dump << " toc entry -- ignoring" << endl;
               continue;
           }
           type = Symbol::PDST_OBJECT;

           if (sym->n_value < aout.data_start) {
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
       
       if (name.prefixed_by(".")) {
           // XXXX - Hack to make names match assumptions of symtab.C
           name = pdstring(name.c_str()+1);
       }
       else if (type == Symbol::PDST_FUNCTION) {
           // text segment without a leading . is a toc item
           //dump << " (no leading . so assuming toc item & ignoring)" << endl;
           continue;
       }
       
       unsigned int size = 0;
       if (type == Symbol::PDST_FUNCTION) {
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
       // Template for linkage functions:
       // l      r12,<offset>(r2) // address of call into R12
       // st     r2,20(r1)        // Store old TOC on the stack
       // l      r0,0(r12)        // Address of callee func
       // l      r2,4(r12)        // callee TOC
       // mtctr  0                // We keep the LR static, use the CTR
       // bctr                    // non-saving branch to CTR
       
       if (size == 0x18) {
           // See if this is linkage code
           Word *inst = (Word *)((char *)code_ptr_ + value - code_off_);
           instructUnion lr12, lr0, bctr;
           lr12.raw = inst[0];
           lr0.raw = inst[2];
           bctr.raw = inst[5];

           if ((lr12.dform.op == Lop) && (lr12.dform.rt == 12) && (lr12.dform.ra == 2) &&
                (lr0.dform.op == Lop) && (lr0.dform.rt == 0) &&
                (lr0.dform.ra == 1 || lr0.dform.ra == 12) &&
               (bctr.xlform.op == BCLRop) && (bctr.xlform.xo == BCCTRxop) 
	        && !( (name == "execve") && (modName == "/usr/lib/libc.a") ) )
               name += "_linkage";
       }

       // HACK. This avoids double-loading various tramp spaces
       if (name.prefixed_by("DYNINSTstaticHeap") &&
           size == 0x18) {
           continue;
       }

       // We ignore the symbols for the floating-point and general purpose
       // register save/restore macros
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
       
       
       if( name == "main" )
           foundMain = true;
       if( name == "_start" )
           foundStart = true;

       parsing_printf("Symbol %s, addr 0x%lx, mod %s, size %d\n",
                      name.c_str(), value, modName.c_str(), size);
       Symbol sym(name, modName, type, linkage, value, false, size);
       
       // If we don't want the function size for some reason, comment out
       // the above and use this:
       // Symbol sym(name, modName, type, linkage, value, false);
       // fprintf( stderr, "Added symbol %s at addr 0x%x, size 0x%x, module %s\n", name.c_str(), value, size, modName.c_str());
       
       symbols_[name].push_back( sym );
       if (symbols_.defines(modName)) {
           // Adjust module's address, if necessary, to ensure that it's <= the
           // address of this new symbol
           
           pdvector< Symbol > & mod_symbols = symbols_[modName];
           
#if defined( DEBUG )
           if( mod_symbols.size() != 1 ) {
               fprintf( stderr, "%s[%d]: module name has more than one symbol:\n", __FILE__, __LINE__  );
               for( unsigned int i = 0; i < mod_symbols.size(); i++ ) {
               	   cerr << mod_symbols[i] << endl;
                   }
               fprintf( stderr, "\n" );
               }
#endif /* defined( DEBUG ) */               

           Symbol & mod_symbol = mod_symbols[ 0 ];
           
           if (value < mod_symbol.addr()) {
               //cerr << "adjusting addr of module " << modName
               //     << " to " << value << endl;
               mod_symbol.setAddr(value);
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
                         name = 
                         pdstring(&stringPool[aux->x_file._x.x_offset]);
                     } else {
                         // x_fname is 14 bytes
                         char tempName[15];
                         memset(tempName, 0, 15);
                         strncpy(tempName, aux->x_file.x_fname, 14);
                         name = pdstring(tempName);
                     }
                 }
             }
         }

         //dump << "found module \"" << name << "\"" << endl;
         
         // Hack time. Break it down
         // Problem: libc and others show up as file names. So if the
         // file being loaded is a .a (it's a hack, remember?) use the
         // .a as the modName instead of the symbol we just found.
         if (file_.suffixed_by(".a") ||
             file_.suffixed_by(".so") ||
             file_.suffixed_by(".so.1"))
             modName = file_;
         else if (name == "glink.s")
             modName = pdstring("Global_Linkage");
         else {
             modName = name;
         }
         const Symbol modSym(modName, modName, 
                             Symbol::PDST_MODULE, linkage,
                             UINT_MAX, // dummy address for now!
                             false);
                             
         /* The old code always had the last module win. */
         if( symbols_[modName].size() == 0 ) {
         	symbols_[modName].push_back( modSym );
         	} else {
	        symbols_[modName][0] = modSym;
	        }
         
         continue;
     }
   }
   
   if( !foundMain && is_aout )
   {
       //we havent found a symbol for main therefore we have to parse _start
       //to find the address of main

       //last two calls in _start are to main and exit
       //find the end of _start then back up to find the target addresses
       //for exit and main
      
       int c;
       instructUnion i;
       int calls = 0;
       
       for( c = 0; code_ptr_[ c ] != 0; c++ );

       while( c > 0 )
       {
           i.raw = code_ptr_[ c ];

           if(i.iform.lk && 
              ((i.iform.op == Bop) || (i.bform.op == BCop) ||
               ((i.xlform.op == BCLRop) && 
                ((i.xlform.xo == 16) || (i.xlform.xo == 528)))))
           {
               calls++;
               if( calls == 2 )
                   break;
           }
           c--;
       }
       
       Address currAddr = aout.text_start + c * instruction::size();
       Address mainAddr = 0;
       
       if( ( i.iform.op == Bop ) || ( i.bform.op == BCop ) )
       {
           int  disp = 0;
           if(i.iform.op == Bop)
           {
               disp = i.iform.li;
           }
           else if(i.bform.op == BCop)
           {
               disp = i.bform.bd;
           }

           disp <<= 2;

           if(i.iform.aa)
           {
               mainAddr = (Address)disp;
           }
           else
               mainAddr = (Address)( currAddr + disp );      
       }  

       Symbol sym( "main", "DEFAULT_MODULE", Symbol::PDST_FUNCTION,
                   Symbol::SL_GLOBAL, mainAddr, 0, (unsigned) -1 );
      
       symbols_[ "main" ].push_back( sym );
   
       //since we are here make up a sym for _start as well

       Symbol sym1( "__start", "DEFAULT_MODULE", Symbol::PDST_FUNCTION,
                   Symbol::SL_GLOBAL, aout.text_start, 0, (unsigned) -1 );
       symbols_[ "__start" ].push_back( sym1 );       
   }

#if 0
   // This got moved to Dyninst-land. Well, it will....

   // We grab the space at the end of the various objects to use as
   // trampoline space. This allows us to instrument those objects 
   // (reasonably) safely.
   // For the application, we actually have from the end of the text
   // to the beginning of the data segment as free space -- this 
   // is readonly to the application, but writeable by debuggers.
   // For shared objects, we can only safely assume that we have from
   // the end of the object to the next page boundary.
   // Since most of this code is the same, I've unified it.

   // Start of the heap

   //IF we are in aout (the executable) and we have .loader info
   //then make the heap start at the end of the loader info.
   //(ASSUMPTION:) On AIX, the loader info is between the text and data sections. 
   //The XCOFF header info does not explicitly state that the loader info
   //is loaded into memory BUT the loader loads it and it is needed upon exit().
   //if we overwrite it the mutatee will seg fault in __modfini 
   //(b/c of bad data in find_rtinit)

   //use the end of the .loader section as the start point ONLY IF:
   //the .loader info is present AND 
   //this is the executable and not a shared lib AND
   //the .loader section is loaded at a larger memory address than the end of .text AND
   //the .loader section is loaded at a smaller memory address than 0x1ffffffc

   //there are no hard and fast rules as to where the .loader section needs
   //to be. the loader could put it before .text, after .data, before .data
   //but beyond 0x1ffffffc, somewhere in ohio.

   // So if loader is near the rest of the text segment, avoid it.

   heapAddr = 0;
       // Pick bigger of loader and code, as long as loader is in the same segment
   
   if (is_aout &&
       (loader_off_+loader_len_ < 0x10000000) &&
       ((loader_off_ + loader_len_) > code_off_ + code_len_))
       heapAddr = loader_off_ + loader_len_;
   else
       heapAddr = code_off_ + code_len_;
   
   // Word-align the heap
   heapAddr += (instruction::size()) - (heapAddr % instruction::size());
   
   // Get the appropriate length
   if (is_aout) // main application binary
       heapLen = (0x1ffffffc - text_org_) - heapAddr;
   else {
       heapLen = PAGESIZE - (heapAddr % PAGESIZE);
   }
   
   char name_scratch[256];
   sprintf(name_scratch, "DYNINSTstaticHeap_%i_uncopiedHeap_%x_scratchpage",
           (unsigned) heapLen,
           (unsigned) heapAddr);
   name = pdstring(name_scratch);
   modName = pdstring("DYNINSTheap");
   heapSym = Symbol(name, modName, Symbol::PDST_FUNCTION, 
                    Symbol::SL_UNKNOWN, heapAddr,
                    false, (int) heapLen);
   
   symbols_[name].push_back( heapSym );
#endif
   
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
      statusLine(errorLine); \
      showErrorCallback(errCode,(const char *) errorLine); \
      return; \
      }
   
void Object::load_archive(bool is_aout) {
    Archive *archive;
    
    // Determine archive type
    // Start at the beginning...
    if (!fo_->set(0))
        PARSE_AR_DIE("Seeking to file start", 49);

    char magic_number[SAIAMAG];
    if (!fo_->read(magic_number, SAIAMAG))
        PARSE_AR_DIE("Reading magic number", 49);

    if (!strncmp(magic_number, AIAMAG, SAIAMAG))
        archive = (Archive *) new Archive_32(fo_);
    else if (!strncmp(magic_number, AIAMAGBIG, SAIAMAG))
        archive = (Archive *) new Archive_64(fo_);
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
                         archive->member_len - 1)) {
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
        bperr( "Member name %s not found in archive %s!\n",
               member_.c_str(), file_.c_str());
    delete archive;
    return;
}

// This is our all-purpose-parse-anything function. 
// Takes a file and determines from the first two bytes the
// file type (archive or a.out). Assumes that two bytes are
// enough to identify the file format. 

void Object::load_object(bool is_aout)
{
    // Load in an object (archive, object, .so)

    assert(fo_);

    unsigned char magic_number[2];
    if (!fo_->set(0)) {
        sprintf(errorLine, "Error reading file %s\n", 
                file_.c_str());
        statusLine(errorLine);
        showErrorCallback(49,(const char *) errorLine);
        return;
    }

    if (!fo_->read((void *)magic_number, 2)) {
        sprintf(errorLine, "Error reading file %s\n", 
                file_.c_str());
        statusLine(errorLine);
        showErrorCallback(49,(const char *) errorLine);
        return;
    }

    // a.out file: magic number = 0x01df
    // archive file: magic number = 0x3c62 "<b", actually "<bigaf>"
    // or magic number = "<a", actually "<aiaff>"
    if (magic_number[0] == 0x01) {
        if (magic_number[1] == 0xdf)
            parse_aout(0, is_aout);
        else 
            //parse_aout_64(fd, 0);
            bperr( "Don't handle 64 bit files yet");
    }
    else if (magic_number[0] == '<') // archive of some sort
        load_archive(is_aout);
    else {// Fallthrough
        sprintf(errorLine, "Bad magic number in file %s\n",
                file_.c_str());
        statusLine(errorLine);
        showErrorCallback(49,(const char *) errorLine);
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
    assert(0);
}

// More general object creation mechanism
Object::Object(const fileDescriptor &desc, void (*err_func)(const char *))
    : AObject(desc.file(), err_func) {
    member_ = desc.member();
    
    fo_ = fileOpener::openFile(desc.file());
    assert(fo_);
    // We want this to be "if a.out", so... 
    
    load_object(!desc.isSharedObject());
}

Object::~Object() 
{
  // Cleanup memory, otherwise we'll have mega-leaks.
  if (code_ptr_) free(code_ptr_);
  if (data_ptr_) free(data_ptr_);

}

Object& Object::operator=(const Object& obj) {
    (void) AObject::operator=(obj);
    return *this;
}


//
// parseCompilerType - parse for compiler that was used to generate object
//	return true for "native" compiler
//
//      XXX - This really should be done on a per module basis
//
bool parseCompilerType(Object *objPtr) 
{

    SYMENT *syms;
    int stab_nsyms;
    char *stringPool;
    union auxent *aux;
    char *stabstr_nextoffset;
    const char *stabstrs = 0;
    char *stabstr=NULL;

    objPtr->get_stab_info(stabstr, stab_nsyms, syms, stringPool);

    for (int i=0;i<stab_nsyms;i++) {
        SYMENT *sym = (SYMENT *) (((unsigned) syms) + i * SYMESZ);
        char tempName[15];
        char *compilerName;
        pdstring name;
        if (sym->n_sclass == C_FILE) {
            if (!sym->n_zeroes) {
                name = pdstring(&stringPool[sym->n_offset]);
            } else {
                char tempName[9];
                memset(tempName, 0, 9);
                strncpy(tempName, sym->n_name, 8);
                name = pdstring(tempName);
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
                //		if (!strncmp("IBM VisualAge C++", compilerName, strlen("IBM VisualAge C++"))) {
                if (strstr(compilerName, "IBM") != NULL 
                    && strstr(compilerName, "VisualAge") != NULL
                    && strstr(compilerName, "C++") != NULL) {
		    // bperr( "compiler is IBM C++\n");
		    return true;
		}
   	   }
       }
    }
    // bperr("compiler is GNU\n");
    return false;
}
