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

// $Id: Object-xcoff.C,v 1.17 2002/06/21 14:19:29 chadd Exp $

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
#include <sys/ptrace.h>
#include <procinfo.h> // struct procsinfo
#include <sys/types.h>
#include <sys/param.h> // PAGESIZE

#include <xcoff.h>
#define __AR_BIG__
#define __AR_SMALL__
#include <ar.h> // archive file format.

#include "dyninstAPI/src/showerror.h"
#include "common/h/debugOstream.h"
#include "arch-power.h"

//
// Seek to the desired offset and read the passed length of the file
//   into dest.  If any errors are detected, log a message and return false.
//
bool seekAndRead(int fd, int offset, void **dest, int length, bool allocate)
{
    int cnt;

    if (allocate) {
	*dest = malloc(length);
    }

    if (!*dest) {
	sprintf(errorLine, "Unable to parse executable file: failed allocation, size %d\n", length);
	logLine(errorLine);
	showErrorCallback(42, (const char *) errorLine);
	return false;
    }
#ifdef __alpha
    prmap_t tmp;
    tmp.pr_vaddr = (char*)offset;
    cnt = lseek(proc_fd, (off_t) tmp.pr_vaddr, SEEK_SET);
#else
    cnt = lseek(fd, offset, SEEK_SET);
#endif
    if (cnt != offset) {
        sprintf(errorLine, "Unable to parse executable file: failed seek\n");
	logLine(errorLine);
	showErrorCallback(42, (const char *) errorLine);
	return false;
    }
    cnt = read(fd, *dest, length);
    if (cnt != length) {
        sprintf(errorLine, "Unable to parse executable file: failed read\n");
	logLine(errorLine);
	showErrorCallback(42, (const char *) errorLine);
	return false;
    }
    return true;
}

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
#ifdef __alpha
  prmap_t tmp;
  tmp.pr_vaddr = 0;
  lseek(proc_fd, (off_t) tmp.pr_vaddr, SEEK_SET);
#else 
  lseek(fd, 0, SEEK_SET);
#endif 
  int cnt = read(fd, &filehdr, sizeof(struct fl_hdr));
  if (cnt != sizeof(struct fl_hdr))
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
  char tmpstring[21];
#ifdef __alpha
  prmap_t tmp;
  tmp.pr_vaddr = 0;
  lseek(proc_fd, (off_t) tmp.pr_vaddr, SEEK_SET);
#else
  lseek(fd, 0, SEEK_SET);
#endif
  int cnt = read(fd, &filehdr, sizeof(struct fl_hdr_big));
  if (cnt != sizeof(struct fl_hdr_big))
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
  int cnt;
  char tmpstring[13];

  if (next_offset == 0) return -1;
#ifdef __alpha
  prmap_t tmp;
  tmp.pr_vaddr = next_offset;
  lseek(proc_fd, (off_t) tmp.pr_vaddr, SEEK_SET);
#else
  lseek(fd, next_offset, SEEK_SET);
#endif 
  // Don't read last two bytes (first two bytes of the name)
  cnt = read(fd, &memberhdr, sizeof(struct ar_hdr) - 2);
  if (cnt != (sizeof(struct ar_hdr) - 2)) return -1;
  strncpy(tmpstring, memberhdr.ar_namlen, 4);
  tmpstring[4] = 0; member_len = atol(tmpstring);
  if (member_name) free(member_name);
  member_name = (char *)malloc(member_len+1);
  cnt = read(fd, member_name, member_len);
  if (cnt != member_len) return -1;
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
  int cnt;
  char tmpstring[21];
  
  if (next_offset == 0) return -1;
#ifdef __alpha
  prmap_t tmp;
  tmp.pr_vaddr = next_offset;
  lseek(proc_fd, (off_t) tmp.pr_vaddr, SEEK_SET);
#else
  lseek(fd, next_offset, SEEK_SET);
#endif
  // Don't read last two bytes (first two bytes of the name)
  cnt = read(fd, &memberhdr, sizeof(struct ar_hdr_big) - 2);
  if (cnt != (sizeof(struct ar_hdr_big) - 2)) return -1;
  strncpy(tmpstring, memberhdr.ar_namlen, 4);
  tmpstring[4] = 0; member_len = atol(tmpstring);
  if (member_name) free(member_name);
  member_name = (char *)malloc(member_len+1);
  cnt = read(fd, member_name, member_len);
  if (cnt != member_len) return -1;
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
      sprintf(errorLine, "Error parsing a.out file %s(%s): %s\n", \
              file_.c_str(), member_.c_str(), errType); \
      statusLine(errorLine); \
      showErrorCallback(errCode,(const char *) errorLine); \
      goto cleanup; \
      }

void Object::parse_aout(int fd, int offset, bool is_aout)
{
   // all these vrble declarations need to be up here due to the gotos,
   // which mustn't cross vrble initializations.  Too bad.
   long i,j;
   int cnt;
   string name;
   unsigned value;
   int poolOffset;
   int poolLength;
   union auxent *aux;
   struct filehdr hdr;
   struct syment *sym;
   struct aouthdr aout;
   union auxent *csect;
   char *stringPool=NULL;
   Symbol::SymbolType type; 
   bool foundDebug = false;

   int *lengthPtr = &poolLength;
   struct syment *symbols = NULL;
   struct scnhdr *sectHdr = NULL;
   Symbol::SymbolLinkage linkage = Symbol::SL_UNKNOWN;
   unsigned toc_offset = 0;
   string modName;

   unsigned int nlines=0;
   int linesfdptr=0;
   struct lineno* lines=NULL;

   // Amounts to relocate symbol addresses
   unsigned text_reloc;
   unsigned data_reloc;

   // For reading process data space.
   unsigned ptrace_amount;
   char *in_self;
   char *in_traced;

   // Creating extra inferior heap space
   Address heapAddr;
   Address heapLen;
   Symbol heapSym;
   
   // Get to the right place in the file (not necessarily 0)
#ifdef __alpha
   prmap_t tmp;
   tmp.pr_vaddr = offset;
   lseek(proc_fd, (off_t) tmp.pr_vaddr, SEEK_SET);
#else
   lseek(fd, offset, SEEK_SET);
#endif
   // Load and check the XCOFF file header
   cnt = read(fd, &hdr, sizeof(struct filehdr));
   if (cnt != sizeof(struct filehdr))
     PARSE_AOUT_DIE("Reading file header", 49);

   if (hdr.f_magic == 0x1ef)
     {
       // XCOFF64 file! We don't handle those yet.
       cerr << "Unhandled XCOFF64 file" << endl;
       return;
     }
   
   if (hdr.f_magic != 0x1df)
     {
       fprintf(stderr, "Possible problem, magic number is %x, should be %x\n",
	       hdr.f_magic, 0x1df);
     }

   // Load and check the a.out (auxiliary) header
   cnt = read(fd, &aout, sizeof(struct aouthdr));
   if (cnt != sizeof(struct aouthdr)) 
     PARSE_AOUT_DIE("Reading a.out header", 49);

   // Load the section headers
   sectHdr = (struct scnhdr *) malloc(sizeof(struct scnhdr) * hdr.f_nscns);
   assert(sectHdr);
   cnt = read(fd, sectHdr, sizeof(struct scnhdr) * hdr.f_nscns);
   if ((unsigned) cnt != sizeof(struct scnhdr)* hdr.f_nscns)
     PARSE_AOUT_DIE("Reading section headers", 49);
   if (!seekAndRead(fd, hdr.f_symptr + offset, (void**) &symbols, 
                    hdr.f_nsyms * SYMESZ, true))
     PARSE_AOUT_DIE("Reading symbol table", 49);

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
    * Get the string pool, if there is one
    */
   poolOffset = hdr.f_symptr + hdr.f_nsyms * SYMESZ;
   /* length is stored in the first 4 bytes of the string pool */
   if (!seekAndRead(fd, poolOffset + offset, (void**) &lengthPtr, sizeof(int), false))
     PARSE_AOUT_DIE("Reading string pool size", 49);
   if (poolLength > 0) {
     if (!seekAndRead(fd, poolOffset + offset, (void**) &stringPool, poolLength, true)) 
       PARSE_AOUT_DIE("Reading string pool", 49);
   }
   else stringPool = NULL;

   /* find the text section such that we access the line information */
   for (i=0; i < hdr.f_nscns; i++)
     if (sectHdr[i].s_flags & STYP_TEXT) {
       nlines = sectHdr[i].s_nlnno;
       
       /* Some libraries have shown line numbers of 0 */
       if (nlines == 0)
	 continue;
       /* if there is overflow in the number of lines */
       if (nlines == 65535)
	 for (j=0; j < hdr.f_nscns; j++)
	   if ((sectHdr[j].s_flags & STYP_OVRFLO) &&
	       (sectHdr[j].s_nlnno == (i+1))){
	     nlines = (unsigned int)(sectHdr[j].s_vaddr);
	     break;
	   }
       
       /* read the line information table */
       if (!seekAndRead(fd,sectHdr[i].s_lnnoptr + offset,(void**) &lines,
			nlines*LINESZ,true))
	 PARSE_AOUT_DIE("Reading line information table", 49);

       linesfdptr = sectHdr[i].s_lnnoptr;
       break;
     }

   // Dyninst/Paradyn meanings
   // code_ptr_: location where mutator has the text segment in memory
   // text_reloc: that + value will get you a cup of coffee... the location in
   //               memory where that file's instructions start.
   //           = "text relocation value" = text_org + scnptr - text_start
   if (text_org_ != (unsigned) -1) { // -1 == illegal flag value, assume 0
     text_reloc = text_org_ + sectHdr[aout.o_sntext-1].s_scnptr - aout.text_start;
     // code_off_ is the value in memory such that code_ptr[x] == code_off_ + x
     code_off_ = text_org_ + sectHdr[aout.o_sntext-1].s_scnptr;
   }
   else {
     text_reloc = 0; code_off_ = 0; // set to illegal
   }
   code_len_ = aout.tsize;
   if (!seekAndRead(fd, roundup4(sectHdr[aout.o_sntext-1].s_scnptr) + offset,
		    (void **) &code_ptr_, aout.tsize, true))
     PARSE_AOUT_DIE("Reading text segment", 49);

#ifdef DEBUG
   fprintf(stderr, "text_org_ = %x, scnptr = %x, text_start = %x\n",
	   (unsigned) text_org_, sectHdr[aout.o_sntext-1].s_scnptr, 
	   (unsigned) aout.text_start);
   fprintf(stderr, "Code pointer: %x, reloc: %x, offset: %x, length: %x\n",
	   (unsigned) code_ptr_, (unsigned) text_reloc,
	   (unsigned) code_off_, (unsigned) code_len_);
#endif

   // data_reloc = "relocation value" = data_org_ - aout.data_start
   if (data_org_ != (unsigned) -1) {
     data_reloc = data_org_ - aout.data_start;
     // We're forced to get the data segment through ptrace. While
     // some of the shared libraries are accessible from both the 
     // mutator and mutatee, all of them are not necessarily mapped. 
     // Not to mention, things like the table of contents (TOC) are
     // filled in at load time. Oy.
     data_ptr_ = (Word *)malloc(aout.dsize);
     ptrace_amount = aout.dsize;
     in_self = (char *)data_ptr_;
     // I've seen the data_org_ value start on a halfword-aligned boundary.
     // Since the first two bytes don't matter that I can tell, we round
     // to word alignment
     in_traced = (char *)(roundup4(data_org_));
     // Maximum ptrace block = 1k

     // Here's a fun one. For the a.out file, we normally have the data in
     // segment 2 (0x200...). This is not always the case. Programs compiled
     // with the -bmaxdata flag have the heap in segment 3. In this case, change
     // the lower bound for the allocation constants in aix.C.
     extern Address data_low_addr;
     if (is_aout) {
       if (data_org_ >= 0x30000000)
	 {
	   data_low_addr = 0x30000000;
	 }
       else
	 {
	   data_low_addr = 0x20000000;
	 }
     }

#ifdef DEBUG
     fprintf(stderr, "data_org_ = %x, data_start = %x\n",
	     (unsigned) data_org_, 
	     (unsigned) aout.data_start);
     fprintf(stderr, "Data pointer: %x, reloc: %x\n",
	     (unsigned) data_ptr_, (unsigned) data_reloc);
#endif
     for (ptrace_amount = aout.dsize ; 
	  ptrace_amount > 1024 ; 
	  ptrace_amount -= 1024)
       {
	 if (ptrace(PT_READ_BLOCK, pid_, (int *)in_traced,
		    1024, (int *)in_self) == -1) {
	   //#ifdef DEBUG
	   fprintf(stderr, "PTRACE_READ 1: from %x (in_traced) to %x (in_self)\n",
		   (int) in_traced, (int) in_self);
	   perror("Reading data segment of inferior process");
	   //#endif DEBUG
	   PARSE_AOUT_DIE("Reading data segment", 49);
	 }
	 in_self += 1024;
	 in_traced += 1024;
       }
     if (ptrace_amount)
       if (ptrace(PT_READ_BLOCK, pid_, (int *)in_traced,
		  ptrace_amount, (int *)in_self) == -1) {
#ifdef DEBUG
	 fprintf(stderr, "PTRACE_READ 2: from %x (in_traced) to %x (in_self)\n",
		 (int) in_traced, (int) in_self);
	 perror("Reading data segment of inferior process");
#endif DEBUG
	 PARSE_AOUT_DIE("Reading data segment", 49);
       }
     // data_off_ is the value subtracted from an (absolute) address to
     // give an offset into the mutator's copy of the data
     data_off_ = data_org_;
   }
   else {
     data_reloc = 0;
     data_off_ = 0;
   }
   
#ifdef DEBUG
   fprintf(stderr, "data_org_ = %x, scnptr = %x, data_start = %x\n",
	   (unsigned) data_org_, sectHdr[aout.o_sndata-1].s_scnptr, 
	   (unsigned) aout.data_start);
#endif

   data_len_ = aout.dsize;

#ifdef DEBUG
   fprintf(stderr, "Data pointer: %x, reloc: %x, offset: %x, length: %x\n",
	   (unsigned) data_ptr_, (unsigned) data_reloc, 
	   (unsigned) data_off_, (unsigned) data_len_);
#endif

   foundDebug = false;

   // Find the debug symbol table.
   for (i=0; i < hdr.f_nscns; i++)
     if (sectHdr[i].s_flags & STYP_DEBUG) {
	 foundDebug = true;
	 break;
       }

   if (foundDebug) 
     {
       stabs_ = (long unsigned int) symbols;
       nstabs_ = hdr.f_nsyms;
       stringpool_ = (long unsigned int) stringPool;
       if (!seekAndRead(fd, roundup4(sectHdr[i].s_scnptr + offset),
			(void **) &stabstr_, sectHdr[i].s_size, true))
	 PARSE_AOUT_DIE("Reading initialized debug section", 49);
       linesptr_ = (long unsigned int) lines;
       nlines_ = (int)nlines; 
       linesfdptr_ = linesfdptr;
   }
   else
     {
       // Not all files have debug information. Libraries tend not to.
       stabs_ = 0;
       nstabs_ = 0;
       stringpool_ = 0;
       stabstr_ = 0;
       linesptr_ = 0;
       nlines_ = 0;
       linesfdptr_ = 0;
     }

   // At this point, check to see if our memory (*_org_) values are
   // valid. If not, break -- there's only so much you can do, and
   // reading blind into memory doesn't count.
   if (text_org_ == (unsigned) -1) goto cleanup;

   // Now the symbol table itself:
   for (i=0; i < hdr.f_nsyms; i++) {
     /* do the pointer addition by hand since sizeof(struct syment)
      *   seems to be 20 not 18 as it should be */
     sym = (struct syment *) (((unsigned) symbols) + i * SYMESZ);
     if (sym->n_sclass & DBXMASK)
       continue;

     if ((sym->n_sclass == C_HIDEXT) || 
	 (sym->n_sclass == C_EXT) ||
	 (sym->n_sclass == C_FILE)) {
       if (!sym->n_zeroes) {
	 name = string(&stringPool[sym->n_offset]);
       } else {
	 char tempName[9];
	 memset(tempName, 0, 9);
	 strncpy(tempName, sym->n_name, 8);
	 name = string(tempName);
       }
     }
     
     if ((sym->n_sclass == C_HIDEXT) || (sym->n_sclass == C_EXT)) {
       if (sym->n_sclass == C_HIDEXT) {
	 linkage = Symbol::SL_LOCAL;
       } else {
	 linkage = Symbol::SL_GLOBAL;
       }
       
       if (sym->n_scnum == aout.o_sntext) {
	 type = Symbol::PDST_FUNCTION;
	 value = sym->n_value + text_reloc;
       } else {
	 // bss or data
	 csect = (union auxent *)
	   ((char *) sym + sym->n_numaux * SYMESZ);
	 
	 if (csect->x_csect.x_smclas == XMC_TC0) { 
	   if (toc_offset)
	     logLine("Found more than one XMC_TC0 entry.");
	   toc_offset = sym->n_value + data_reloc;
	   continue;
	 }
	 
	 if ((csect->x_csect.x_smclas == XMC_TC) ||
	     (csect->x_csect.x_smclas == XMC_DS)) {
	   // table of contents related entry not a real symbol.
	   //dump << " toc entry -- ignoring" << endl;
	   continue;
	 }
	 type = Symbol::PDST_OBJECT;
	 value = sym->n_value + data_reloc;
       }
       
       // skip .text entries
       if (name == ".text") continue;
       if (name.prefixed_by(".")) {
	 // XXXX - Hack to make names match assumptions of symtab.C
	 name = string(name.c_str()+1);
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
	 instruction instr;
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
       // Module glink.s is renamed to Global_Linkage below
       if (modName == "Global_Linkage")
	 name += "_linkage";

       // We ran into problems where the io_wait metric wasn't working.
       // It appears as though we only instrument one function with a 
       // given name, and there was a different write we were instrumenting.
       /*
       if ((name == "write") &&
	   !(modName.suffixed_by("libc.a"))) {
	 continue;
       }
       */
       // HACK. This avoids double-loading various tramp spaces
       if (name.prefixed_by("DYNINSTstaticHeap") &&
	   size == 0x18)
	 continue;

       Symbol sym(name, modName, type, linkage, value, false, size);
       
       // If we don't want the function size for some reason, comment out
       // the above and use this:
       // Symbol sym(name, modName, type, linkage, value, false);
#ifdef DEBUG
       fprintf(stderr, "Added symbol %s at addr 0x%x, size 0x%x, module %s\n",
	       name.c_str(), value, size, modName.c_str());
#endif

       symbols_[name] = sym;
       
       if (symbols_.defines(modName)) {
	 // Adjust module's address, if necessary, to ensure that it's <= the
	 // address of this new symbol
	 Symbol &mod_symbol = symbols_[modName];
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
		 string(&stringPool[aux->x_file._x.x_offset]);
	     } else {
	       // x_fname is 14 bytes
	       char tempName[15];
	       memset(tempName, 0, 15);
	       strncpy(tempName, aux->x_file.x_fname, 14);
	       name = string(tempName);
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
	 modName = string("Global_Linkage");
       else {
	 modName = name;
       }
       const Symbol modSym(modName, modName, 
			   Symbol::PDST_MODULE, linkage,
			   UINT_MAX, // dummy address for now!
			   false);
       symbols_[modName] = modSym;
       
       continue;
     }
   }
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
   heapAddr = code_off_ + code_len_;
   // Word-align the heap
   heapAddr += (sizeof(instruction)) - (heapAddr % sizeof(instruction));
   
   // Get the appropriate length
   if (is_aout) // main application binary
     heapLen = 0x1ffffffc - heapAddr;
   else {
     heapLen = PAGESIZE - (heapAddr % PAGESIZE);
   }
   char name_scratch[256];
   sprintf(name_scratch, "%s%i%s%x",
	   "DYNINSTstaticHeap_", (unsigned) heapLen,
	   "_textHeap_", (unsigned) heapAddr);
   name = string(name_scratch);
   modName = string("DYNINSTheap");
   heapSym = Symbol(name, modName, Symbol::PDST_OBJECT, 
		    Symbol::SL_UNKNOWN, heapAddr,
		    false, (int) heapLen);
   symbols_[name] = heapSym;

   // Set the table of contents offset
   toc_offset_ = toc_offset;
   
 cleanup:

   if (sectHdr) free(sectHdr);
   if (stringPool && !foundDebug) free(stringPool);
   if (symbols && !foundDebug) free(symbols);
   if (lines && !foundDebug) free(lines);
   
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

void Object::load_archive(int fd, bool is_aout)
{
  Archive *archive;

  // Determine archive type
#ifdef __alpha
  prmap_t tmp;
  tmp.pr_vaddr = 0;
  lseek(proc_fd, (off_t) tmp.pr_vaddr, SEEK_SET);
#else
  lseek(fd, 0, SEEK_SET);
#endif
  char magic_number[SAIAMAG];
  int cnt = read(fd, magic_number, SAIAMAG);
  if (cnt != SAIAMAG)
    PARSE_AR_DIE("Reading magic number", 49);
  if (!strncmp(magic_number, AIAMAG, SAIAMAG))
    archive = (Archive *) new Archive_32(fd);
  else if (!strncmp(magic_number, AIAMAGBIG, SAIAMAG))
    archive = (Archive *) new Archive_64(fd);
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
      parse_aout(fd, archive->aout_offset, is_aout);
    }
  else
    fprintf(stderr, "Member name %s not found in archive %s!\n",
	    member_.c_str(), file_.c_str());
  return;
}

// This is our all-purpose-parse-anything function. 
// Takes a file and determines from the first two bytes the
// file type (archive or a.out). Assumes that two bytes are
// enough to identify the file format. 

void Object::load_object(bool is_aout)
{
  // Load in an object (archive, object, .so)
  int fd = 0;
  unsigned char magic_number[2];
  int cnt;

  fd = open(file_.c_str(), O_RDONLY, 0);
  if (fd <0) {
    sprintf(errorLine, "Unable to open file %s\n", 
	    file_.c_str());
    statusLine(errorLine);
    showErrorCallback(27,(const char *) errorLine);
    return;
  }
  
  cnt = read(fd, magic_number, 2);
  
  if (cnt != 2) {
    sprintf(errorLine, "Error reading file %s\n", 
	    file_.c_str());
    statusLine(errorLine);
    showErrorCallback(49,(const char *) errorLine);
    close(fd);
    return;
  }
  
  // a.out file: magic number = 0x01df
  // archive file: magic number = 0x3c62 "<b", actually "<bigaf>"
  // or magic number = "<a", actually "<aiaff>"
  if (magic_number[0] == 0x01) {
    if (magic_number[1] == 0xdf)
      parse_aout(fd, 0, is_aout);
    else 
      //parse_aout_64(fd, 0);
      fprintf(stderr, "Don't handle 64 bit files yet");
  }
  else if (magic_number[0] == '<') // archive of some sort
    load_archive(fd, is_aout);
  else // Fallthrough
    { 
      sprintf(errorLine, "Bad magic number in file %s\n",
	      file_.c_str());
      statusLine(errorLine);
      showErrorCallback(49,(const char *) errorLine);
    }
  if (fd) close(fd);
  return;
}

// There are three types of "shared" files:
// archives (made with ar, end with .a)
// objects (ld -bM:SRE)
// new-style shared objects (.so)
// load_shared_object determines from magic number which to use
// since static objects are just a.outs, we can use the same
// function for all

Object::Object(const string file, void (*err_func)(const char *))
  : AObject(file, err_func) {
  cerr << "In illegal constructor Object(string, addr, func)" << endl;
  text_org_ = 0;
  data_org_ = 0;
  member_ = "";
  pid_ = 0;
  load_object(true);
}

Object::Object(const Object& obj)
    : AObject(obj) {
  // Copy over org data
  // You know, this really should never be called, but be careful.
  text_org_ = obj.text_org_;
  data_org_ = obj.data_org_;
  pid_ = obj.pid_;
  load_object(false);
}

// For shared object files
Object::Object(const string file,Address addr,void (*err_func)(const char *))
    : AObject(file, err_func) {
  // Okay, interface limitation problems here. We're passed a 
  // library name (file) and a text relocation address (addr),
  // and we want a member name and data relocation address.
  // Tough.

  cerr << "In illegal constructor Object(string, addr, func)" << endl;

  text_org_ = addr;
  data_org_ = 0;
  member_ = "";
  pid_ = 0;
  load_object(false);
}

// More general object creation mechanism
Object::Object(fileDescriptor *desc, Address baseAddr, void (*err_func)(const char *))
  : AObject(desc->file(), err_func) {
  // We're passed a descriptor object that contains everything needed.
  fileDescriptor_AIX *fda = (fileDescriptor_AIX *)desc;
  text_org_ = fda->addr();
  data_org_ = fda->data();
  member_ = fda->member();
  pid_ = fda->pid();
  load_object(fda->is_aout());
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
