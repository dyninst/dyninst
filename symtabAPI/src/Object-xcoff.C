/*
 * Copyright (c) 1996-2011 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

// $Id: Object-xcoff.C,v 1.32 2008/09/20 03:56:10 jaw Exp $

// Define this before all others to insure xcoff.h is included
// with __XCOFF_HYBRID__ defined.
#define __XCOFF_HYBRID__

#include <regex.h>

#include "symtabAPI/h/Symtab.h"
#include "symtabAPI/src/Object.h"
#include "common/h/pathName.h"

#include "Collections.h"
#include "Symtab.h"
#include "Module.h"

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

#include "debug.h"

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
#include <exceptab.h>
#define __AR_BIG__
#define __AR_SMALL__
#include <ar.h> // archive file format.
#include <scnhdr.h>

#include <dlfcn.h>
#include "common/h/debugOstream.h"

/* For some reason this symbol type isn't global */
#if !defined(C_WEAKEXT)
#define C_WEAKEXT 0
#endif

char errorLine[100];
std::string symt_current_func_name;
std::string symt_current_mangled_func_name;
Symbol *symt_current_func = NULL;

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
    val += 3;
    return val - (val % 4);
}

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

fileOpener *fileOpener::openFile(void *ptr, unsigned size) 
{
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

void fileOpener::closeFile() 
{
   refcount_--;

   if (refcount_ > 0) return;

   // Remove us from the big list...
   std::vector<fileOpener *>::iterator iter = openedFiles.begin();
   for (; iter!=openedFiles.end(); iter++)
   {
      if (*iter == this)
         openedFiles.erase(iter);
   }

   /*for (unsigned i = 0; i < openedFiles.size(); i++) {
     if (openedFiles[i] == this)
     openedFiles.erase(i));
     }*/

   if (file()!="") //only if its not a mem image
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

bool fileOpener::set(unsigned addr) 
{
    //assert(fd_);     may not be present if its a mem image
    assert(size_);
    assert(mmapStart_);

    if (addr >= size_) {
        fprintf(stderr, "%s[%d]: Warning: attempting to set offset to address %d (0x%x) greater than size %d (0x%x)\n", FILE__, __LINE__, 
                addr, addr,
                size_, size_);
        return false;
    }
    offset_ = addr;
    return true;
}

bool fileOpener::read(void *buf, unsigned size) 
{
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

Region::perm_t getRegionPerms(unsigned flags){
    if(flags & STYP_TEXT)
        return Region::RP_RX;
    else if(flags & STYP_DATA)
        return Region::RP_RW;
    else if(flags & STYP_DATA)
        return Region::RP_RW;
    else if(flags & STYP_DATA)
        return Region::RP_RW;
    else
        return Region::RP_R;
}

Region::RegionType getRegionType(unsigned flags){
    if(flags & STYP_TEXT)
        return Region::RT_TEXT;
    else if(flags & STYP_DATA)
        return Region::RT_DATA;
    else if(flags & STYP_DATA)
        return Region::RT_BSS;
    else if(flags & STYP_DATA)
        return Region::RT_DATA;
    else if(flags & STYP_DEBUG)
        return Region::RT_SYMTAB;
    else
        return Region::RT_OTHER;
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
              mf->filename().c_str(), member_.c_str(), errType); \
      err_func_(errType); \
      return; \
      }

// ----------------------------------------------------------------------------
// 32/64-bit section header member access macros.
// These macros depend on the definition of variables scnh_base and is64,
// and are only meant to be used from within Object::parse_aout().
// 
#define SCNH_NAME(i)           (char *)(scnh_base + scn_size * (i) + s_name_off)
#define SCNH_PADDR(i)   ((*(uint64_t *)(scnh_base + scn_size * (i) + s_paddr_off)) >> word_shift)
#define SCNH_VADDR(i)   ((*(uint64_t *)(scnh_base + scn_size * (i) + s_vaddr_off)) >> word_shift)
#define SCNH_SIZE(i)    ((*(uint64_t *)(scnh_base + scn_size * (i) + s_size_off)) >> word_shift)
#define SCNH_SCNPTR(i)  ((*(uint64_t *)(scnh_base + scn_size * (i) + s_scnptr_off)) >> word_shift)
#define SCNH_RELPTR(i)  ((*(uint64_t *)(scnh_base + scn_size * (i) + s_relptr_off)) >> word_shift)
#define SCNH_LNNOPTR(i) ((*(uint64_t *)(scnh_base + scn_size * (i) + s_lnnoptr_off)) >> word_shift)
#define SCNH_EXCEPTPTR(i) ((*(uint64_t *)(scnh_base + scn_size * (i) + s_exceptptr_off)) >> word_shift)
#define SCNH_NRELOC(i)  ((*(uint32_t *)(scnh_base + scn_size * (i) + s_nreloc_off)) >> half_shift)
#define SCNH_NLNNO(i)   ((*(uint32_t *)(scnh_base + scn_size * (i) + s_nlnno_off)) >> half_shift)
#define SCNH_FLAGS(i)    (*(uint32_t *)(scnh_base + scn_size * (i) + s_flags_off))

// ----------------------------------------------------------------------------
// 32/64-bit symbol table entry member access macros.
// These macros depend on the definition of variables scnh_base and is64,
// and are only meant to be used from within Object::parse_aout().
// 

bool Object::fillExceptionTable(struct exceptab *etab, unsigned int etab_size, bool is64,
		struct syment *symbols)

{
	//  Unfortunately the xcoff exception table appears to hold locations for 
	//  trap instructions, but nothing to do with c++ exceptions??
	//  This routine is not what we want
	if (!etab)
	{
		fprintf(stderr, "%s[%d]:  FIXME!\n", FILE__, __LINE__);
		return false;
	}

	for (unsigned int i = 0; i < etab_size; ++i)
	{
		struct exceptab *tabentry = ((struct exceptab *)etab) + i;
		if (!tabentry)
		{
			fprintf(stderr, "%s[%d]:  bad table entry!\n", FILE__, __LINE__);
			continue;
		}

		//  Warn:  header files expressly warn against using sizeof(EXCEPTTAB)
		if (is64)
		{
			//char lang = tabentry->_u._s64._lang64;
			char reason = tabentry->_u._s64._reason64;
			if (reason == 0)
			{
				//  This case happens for the first exception entry
				//  for a given function.  If this is zero, the address
				//  field contains the symbol table entry of the 
				//  relevant function
				//int symindex = tabentry->_u._s64._addr64.e_symndx;
				//struct syment *sym = (struct syment *) (((char *)symbols) + symindex * SYMESZ);
				//const char *name =  &stringPool[ sym->n_offset64 ];
			}
			else
			{
				//uint64_t addr = tabentry->_u._s64._addr64.e_paddr;
			}
			//catch_addrs_.push_back(new ExceptionBlock(addr));
		}
		else 
		{
			//char lang = tabentry->_u._s32._lang32;
			char reason = tabentry->_u._s32._reason32;
			if (reason == 0)
			{
				//  This case happens for the first exception entry
				//  for a given function.  If this is zero, the address
				//  field contains the symbol table entry of the 
				//  relevant function
				//int symindex = tabentry->_u._s32._addr32.e_symndx;
				//struct syment *sym = (struct syment *) (((char *)symbols) + symindex * SYMESZ);
				//const char *name =  &stringPool[ sym->n_offset32 ] ;
			}
			else
			{
				//uint32_t addr = tabentry->_u._s32._addr32.e_paddr;
			}
		}
	}
	return true;
}

void Object::parse_aout(int offset, bool /*is_aout*/, bool alloc_syms)
{
   // all these vrble declarations need to be up here due to the gotos,
   // which mustn't cross vrble initializations.  Too bad.
   long i,j;
   std::string name;
   unsigned long value;
   unsigned secno;
   unsigned textSecNo = 0, dataSecNo = 0, loaderSecNo = 0, exceptSecNo = 0;
   union auxent *aux = NULL;
   struct filehdr hdr;
   struct aouthdr aout;
   struct syment *sym = NULL;
   union auxent *csect = NULL;
   Symbol::SymbolType type; 
   stringPool = NULL;
   bool foundDebug = false;
   bool foundLoader = false;
   bool foundData = false;
   bool foundText = false;
       
   unsigned exceptab_size_ = 0;
#if 0
   struct exceptab * exceptab_ = NULL;
#endif

   stabs_ = NULL;
   nstabs_ = 0;
   stringpool_ = NULL;
   stabstr_ = NULL;
   linesptr_ = NULL;
   nlines_ = 0;
   linesfdptr_ = 0;

   struct syment *symbols = NULL;
   unsigned char *scnh_base = NULL;
   Symbol::SymbolLinkage linkage = Symbol::SL_UNKNOWN;
   Symbol::SymbolVisibility visibility = Symbol::SV_DEFAULT;
   unsigned toc_offset = 0;
   std::string modName;
   baseAddress_ = (Offset)fo_->getPtrAtOffset(offset);

   int linesfdptr=0;
   struct lineno* lines=NULL;

   // Get to the right place in the file (not necessarily 0)
   if (!fo_->set(offset))
      PARSE_AOUT_DIE("Seeking to correct offset", 49);

   // -------------------------------------------------------------------------
   // Read the 2 byte magic number.  Everything depends on it.
   //
   uint16_t magic;
   if (!fo_->read(&magic, 2))
      PARSE_AOUT_DIE("Reading magic number", 49);

   // Determine file address width.
   bool is64 = false;
   if (magic == 0x1EF /* XCOFF64 on AIX4.3 or earlier */ ||
       magic == 0x1F7 /* XCOFF64 on AIX5.1 or later   */) {
      is64 = true;
      addressWidth_nbytes = 8;

   } else if (magic != 0x1DF /* XCOFF32 */) {
      PARSE_AOUT_DIE("possible problem, invalid magic number", 49);
      //bperr("Possible problem, magic number is %x, should be %x\n",
      //       magic, 0x1df);
   }
   is64_ = is64;

   // -------------------------------------------------------------------------
   // Set up 32/64 bit offset variables.
   //
   uint64_t scn_size;
   uint64_t s_name_off, s_paddr_off, s_vaddr_off, s_size_off, s_scnptr_off;
   uint64_t s_relptr_off, s_lnnoptr_off;
   uint64_t s_nreloc_off, s_nlnno_off, s_flags_off;
   uint64_t word_shift, half_shift;

   if (is64) {
      scn_size      = SCNHSZ_64;
      s_name_off    = offsetof(struct scnhdr, s_name);
      s_paddr_off   = offsetof(struct scnhdr, s_paddr64);
      s_vaddr_off   = offsetof(struct scnhdr, s_vaddr64);
      s_size_off    = offsetof(struct scnhdr, s_size64);
      s_scnptr_off  = offsetof(struct scnhdr, s_scnptr64);
      s_relptr_off  = offsetof(struct scnhdr, s_relptr64);
      s_lnnoptr_off = offsetof(struct scnhdr, s_lnnoptr64);
      s_nreloc_off  = offsetof(struct scnhdr, s_nreloc64);
      s_nlnno_off   = offsetof(struct scnhdr, s_nlnno64);
      s_flags_off   = offsetof(struct scnhdr, s_flags64);
      word_shift    = 0;
      half_shift    = 0;

   } else {
      scn_size      = SCNHSZ_32;
      s_name_off    = offsetof(struct scnhdr, s_name);
      s_paddr_off   = offsetof(struct scnhdr, s_paddr32);
      s_vaddr_off   = offsetof(struct scnhdr, s_vaddr32);
      s_size_off    = offsetof(struct scnhdr, s_size32);
      s_scnptr_off  = offsetof(struct scnhdr, s_scnptr32);
      s_relptr_off  = offsetof(struct scnhdr, s_relptr32);
      s_lnnoptr_off = offsetof(struct scnhdr, s_lnnoptr32);
      s_nreloc_off  = offsetof(struct scnhdr, s_nreloc32);
      s_nlnno_off   = offsetof(struct scnhdr, s_nlnno32);
      s_flags_off   = offsetof(struct scnhdr, s_flags32);
      word_shift    = 32;
      half_shift    = 16;
   }

   // Begin file processing.  Re-seek to beginning of file.
   if (!fo_->set(offset))
      PARSE_AOUT_DIE("Seeking to correct offset", 49);

   // -------------------------------------------------------------------------
   // Read and process file header.
   //
   if (!fo_->read(&hdr, (is64 ? FILHSZ_64 : FILHSZ_32)))
      PARSE_AOUT_DIE("Reading file header", 49);

   uint64_t symptr, nsyms;
   if (is64) {
      symptr = hdr.f_symptr64;
      nsyms  = hdr.f_nsyms64;

   } else {
      symptr = hdr.f_symptr32;
      nsyms  = hdr.f_nsyms32;
   }
   is_aout_ = !(hdr.f_flags & F_SHROBJ);

#if 0
   if (hdr.f_opthdr == 0)
      cout << "no aout header" << endl;
   if (hdr.f_opthdr == _AOUTHSZ_SHORT)
      cout << "short header" << endl;
#endif

   // -------------------------------------------------------------------------
   // Read and process optional (a.out) header.
   //
   if (!fo_->read(&aout, hdr.f_opthdr))
      PARSE_AOUT_DIE("Reading a.out header", 49);

   uint64_t text_start, tsize;
   uint64_t data_start, dsize;
   uint64_t entry;

   if (is64) {
      text_start = aout.o_text_start64;
      data_start = aout.o_data_start64;
      tsize = aout.o_tsize64;
      dsize = aout.o_dsize64;
      entry = aout.o_entry64;

   } else {
      text_start = aout.o_text_start32;
      data_start = aout.o_data_start32;
      tsize = aout.o_tsize32;
      dsize = aout.o_dsize32;
      entry = aout.o_entry32;
   }

   scnh_base = (unsigned char *)fo_->ptr();
   if (!scnh_base)
      PARSE_AOUT_DIE("Reading section headers", 49);
   
   fo_->seek((is64 ? SCNHSZ_64 : SCNHSZ_32) * hdr.f_nscns);
   //if binary is not stripped 

   if ( symptr ) {
      fo_->set(offset + symptr);
      symbols = (struct syment *) fo_->ptr();
      fo_->seek(nsyms * SYMESZ);
       
      if (!symbols)
         PARSE_AOUT_DIE("Reading symbol table", 49);
   }

   // Consistency check
   if (hdr.f_opthdr == (is64 ? _AOUTHSZ_EXEC_64 : _AOUTHSZ_EXEC_32))
   {
      // complete aout header present
      if ( text_start != SCNH_PADDR(aout.o_sntext-1))
         PARSE_AOUT_DIE("Checking text address", 49);
      if ( data_start != SCNH_PADDR(aout.o_sndata-1))
         PARSE_AOUT_DIE("Checking data address", 49);
      if ( tsize != SCNH_SIZE(aout.o_sntext-1)) 
         PARSE_AOUT_DIE("Checking text size", 49);
      if ( dsize != SCNH_SIZE(aout.o_sndata-1))
         PARSE_AOUT_DIE("Checking data size", 49);
   }
   /* else if(hdr.f_opthdr == _AOUTHSZ_SHORT)
      {
      if ( text_start != SCNH_PADDR(aout.o_sntext-1))
      PARSE_AOUT_DIE("Checking text address", 49);
      if ( data_start != SCNH_PADDR(aout.o_sndata-1))
      PARSE_AOUT_DIE("Checking data address", 49);
      if ( tsize != SCNH_SIZE(aout.o_sntext-1))
      PARSE_AOUT_DIE("Checking text size", 49);
      if ( dsize != SCNH_SIZE(aout.o_sndata-1))
      PARSE_AOUT_DIE("Checking data size", 49);
      }*/
   if (hdr.f_opthdr !=  0)
      entryAddress_ = entry;

   /*
    * Get the string pool, if there is one
    */
   if ( nsyms ) 
   {
      // We want to jump past the symbol table...
      if (!fo_->set(offset + symptr + (nsyms*SYMESZ)))
         PARSE_AOUT_DIE("Could not seek to string pool", 49);

      stringPool = (char *)fo_->ptr();
      if (!stringPool)
         PARSE_AOUT_DIE("Getting pointer to string pool", 49);

      // First 4 bytes is the length; this is included in the string pool pointer
      Offset stringPoolSize;
      fo_->read(&stringPoolSize, 4);
   }

   /* find the text section such that we access the line information */
   nlines_ = 0;
   no_of_sections_ = hdr.f_nscns;
   foundLoader = false;
   foundData = false;
   
   for (i=0; i < hdr.f_nscns; i++)
   {
      if(SCNH_FLAGS(i) & STYP_DATA)
      {
         dataSecNo = i;
         foundData = true;
      }
      else if(SCNH_FLAGS(i) & STYP_LOADER)
      {
         loaderSecNo = i;
         foundLoader = true;
      }	   
      if (SCNH_FLAGS(i) & STYP_TEXT) {
         textSecNo = i;
         foundText = true;
         nlines_ = SCNH_NLNNO(i);

         /* if there is overflow in the number of lines */
         if (!is64 && nlines_ == 65535)
            for (j=0; j < hdr.f_nscns; j++)
               if ((SCNH_FLAGS(j) & STYP_OVRFLO) &&
                   ((long) SCNH_NLNNO(j) == (i+1))) {
                  nlines_ = (unsigned int)(SCNH_VADDR(j));
                  break;
               }

         /* There may not be any line information. */
         if (nlines_ == 0)
            continue;

         /* read the line information table */
         if (!fo_->set(offset + SCNH_LNNOPTR(i)))
            PARSE_AOUT_DIE("Seeking to line information table", 49);
         lines = (struct lineno *)fo_->ptr();
         if (!lines)
            PARSE_AOUT_DIE("Reading line information table", 49);
         fo_->seek(nlines_ * (is64 ? LINESZ_64 : LINESZ_32));
           
         linesfdptr = offset + SCNH_LNNOPTR(i);
         //break;
      }
      if (SCNH_FLAGS(i) & STYP_BSS) {
         bss_size_ = SCNH_SIZE(i);
      }
      if (SCNH_FLAGS(i) & STYP_EXCEPT) {
		  exceptSecNo = i;
         exceptab_size_ = SCNH_SIZE(i);
#if 0
         if (!fo_->set(offset + SCNH_SCNPTR(i)))
            PARSE_AOUT_DIE("Seeking to exception table", 49);
         exceptab_ = (struct exceptab *)fo_->ptr();
         if (!exceptab_)
            PARSE_AOUT_DIE("Reading line information table", 49);
		 if (!fillExceptionTable(exceptab_, exceptab_size_, is64, symbols))
		 {
			 fprintf(stderr, "%s[%d]:  WARNING:  failed to read exceptions\n", FILE__, __LINE__);
		 }
#endif
      }
   }

   if (alloc_syms)
      for (i=0; i < hdr.f_nscns; i++) {
          // the XCOFF spec says that only text and data sections are loaded
          Region::RegionType regionType = getRegionType(SCNH_FLAGS(i));
          bool isLoadable = regionType == Region::RT_TEXT || 
                            regionType == Region::RT_DATA;
          regions_.push_back(new Region(i, SCNH_NAME(i), SCNH_PADDR(i), SCNH_SIZE(i), SCNH_PADDR(i), SCNH_SIZE(i), (char *)fo_->getPtrAtOffset(offset+SCNH_SCNPTR(i)), getRegionPerms(SCNH_FLAGS(i)), regionType, isLoadable));
//         regions_.push_back(new Section(i, SCNH_NAME(i), SCNH_PADDR(i),
//                  fo_->getPtrAtOffset(offset+SCNH_SCNPTR(i)), SCNH_SIZE(i)));
         //fprintf(stderr, "%s[%d]:  section named %s\n", FILE__, __LINE__, SCNH_NAME(i));
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
   //Offset fileTextOffset = roundup4(SCNH_SCNPTR(aout.o_sntext-1));
   //Offset fileDataOffset = roundup4(SCNH_SCNPTR(aout.o_sndata-1));

   Offset fileTextOffset;
   Offset fileDataOffset;

   fileDataOffset = foundData ? roundup4(SCNH_SCNPTR(dataSecNo)) : (Offset) -1;
   fileTextOffset = foundText ? roundup4(SCNH_SCNPTR(textSecNo)) : (Offset) -1;
   code_off_ = foundText ? roundup4(SCNH_PADDR(textSecNo)) : (Offset) -1;
   data_off_ = foundData ? roundup4(SCNH_PADDR(dataSecNo)) : (Offset) -1;
   code_len_ = foundText ? SCNH_SIZE(textSecNo) : 0;
   data_len_ = foundData ? SCNH_SIZE(dataSecNo) : 0;
   text_reloc_ = fileTextOffset;
   data_reloc_ = fileDataOffset;

   if (foundText) 
   {
      if (!fo_->set(fileTextOffset + offset))
         PARSE_AOUT_DIE("Seeking to start of text segment", 49);
      code_ptr_ = (char *)fo_->ptr();
      if (!code_ptr_)
         PARSE_AOUT_DIE("Reading text segment", 49);
   }
   else
      code_ptr_ = NULL;
      
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

   if(foundLoader)
   {
   	//FIND LOADER INFO!
   	loader_off_ = SCNH_SCNPTR(loaderSecNo);
   	//loader_off_ = SCNH_SCNPTR(aout.o_snloader-1);
   	loadAddress_ = loader_off_;
   	loader_len_ = SCNH_SIZE(loaderSecNo); 
   	//loader_len_ = SCNH_SIZE(aout.o_snloader-1);
   }	

   // And some debug output
   object_printf("Text offset in file: 0x%x (0x%x + 0x%x), virtual address 0x%x, size %d (0x%x) bytes\n",
                 fileTextOffset + offset, fileTextOffset, offset, code_off_, code_len_, code_len_);
   object_printf("Data offset in file: 0x%x (0x%x + 0x%x), virtual address 0x%x, size %d (0x%x) bytes\n",
                 fileDataOffset + offset, fileDataOffset, offset, data_off_, data_len_, data_len_);
   
   foundDebug = false;

   // Find the debug symbol table.
   for (i=0; i < hdr.f_nscns; i++)
   {
      if (SCNH_FLAGS(i) & STYP_DEBUG) 
      {
         foundDebug = true;
         break;
      }
   }
   if ( foundDebug ) 
   {
      stabs_ = (void *) symbols;
      nstabs_ = nsyms;
      stringpool_ = (void *) stringPool;
      if( nsyms ) {
         if (!fo_->set(SCNH_SCNPTR(i) + offset))
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
   
   no_of_symbols_ = nsyms;
   // Now the symbol table itself:
   if (alloc_syms) {
       object_printf("%d symbols present in file\n", nsyms);
   for (i=0; i < (signed)nsyms; i++) 
   {
      /* do the pointer addition by hand since sizeof(struct syment)
       *   seems to be 20 not 18 as it should be. Mmm alignment. */
      sym = (struct syment *) (((char *)symbols) + i * SYMESZ);
      unsigned long sym_value = (is64 ? sym->n_value64 : sym->n_value32);

      if (sym->n_sclass & DBXMASK) {
          object_printf("Skipping DBXMASK symbol %s\n", name.c_str());
          continue;
      }

      secno = sym->n_scnum;
      if ((C_WEAKEXT && (sym->n_sclass == C_WEAKEXT)) ||
          (sym->n_sclass == C_HIDEXT) || 
          (sym->n_sclass == C_EXT) ||
          (sym->n_sclass == C_FILE)) {

         if (is64) {
            name = std::string( &stringPool[ sym->n_offset64 ] );
         } else if (!sym->n_zeroes32) {
            name = std::string( &stringPool[ sym->n_offset32 ] );
         } else {
            char tempName[9];
            memset(tempName, 0, 9);
            strncpy(tempName, sym->n_name32, 8);
            name = std::string(tempName);
         }
      }

      unsigned long size = 0;
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
            value = sym_value;
            /*
              fprintf(stderr, "Text symbol %s, at 0x%lx\n",
              name.c_str(), value);
            */
         } else {
            // bss or data
            csect = (union auxent *)
               ((char *) sym + sym->n_numaux * SYMESZ);

            // Bits 5-7 of x_smtyp (last three bits) hold symbol type.
            unsigned smtyp = csect->x_csect.x_smtyp & 0x3;
            if (smtyp == XTY_SD || smtyp == XTY_CM) {
               size = csect->x_csect.x_scnlen32;
               if (is64) size &= csect->x_csect.x_scnlen_hi64 * 0x10000;
            }

            if (csect->x_csect.x_smclas == XMC_TC0) 
            {
               //if (toc_offset);
               //logLine("Found more than one XMC_TC0 entry.");
               toc_offset = sym_value;
               object_printf("Found TOC offset: 0x%lx\n", toc_offset);
               continue;
            }

            if ((csect->x_csect.x_smclas == XMC_TC) ||
                (csect->x_csect.x_smclas == XMC_DS)) {
               // table of contents related entry not a real symbol.
               //dump << " toc entry -- ignoring" << endl;
                object_printf("TOC entry (%s/%d), ignoring (%s)\n", name.c_str(),sym_value,
                              (csect->x_csect.x_smclas == XMC_TC) ? "TC" : "DS");
                continue;
            }
            type = Symbol::ST_OBJECT;

            if (foundData && sym_value < SCNH_PADDR(dataSecNo)) {
               // Very strange; skip
                object_printf("Skipping odd case of data symbol not in data section (%s)\n", name.c_str());
                continue;
            }

            value = sym_value;

            // Shift it back down; data_org will be added back in later.

            //fprintf(stderr, "Sym %s, at 0x%lx\n",
            //      name.c_str(), value);
         }

         // skip .text entries
         if (name == ".text")  {
             object_printf("Skipping .text symbol\n");
             continue;
         }
       
         if (name[0] == '.' ) {
            // XXXX - Hack to make names match assumptions of symtab.C
            name = std::string(name.c_str()+1);
         }
         else if (type == Symbol::ST_FUNCTION) {
            // text segment without a leading . is a toc item
            //dump << " (no leading . so assuming toc item & ignoring)" << endl;
             object_printf("Skipping text symbol without leading .\n");
             continue;
         }
       
         if (type == Symbol::ST_FUNCTION) {
            // Find address of inst relative to code_ptr_, instead of code_off_
           
            size = 0;
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
         }

         Region *sec;
         if(secno >= 1 && secno <= regions_.size())
            sec = regions_[secno-1];
         else
            sec = NULL;

         //fprintf(stderr, "Creating new symbol: %s, %s, %d, %d, 0x%lx, %d, %d\n",
         //name.c_str(), modName.c_str(), type, linkage, value, sec, size);

         Symbol *sym = new Symbol(name, 
                                  type, 
                                  linkage, 
                                  Symbol::SV_DEFAULT, 
                                  value, 
                                  NULL,
                                  sec, 
                                  size);
       
         symbols_[name].push_back(sym);
         symsByOffset_[value].push_back(sym);
         symsToModules_[sym] = modName;

         if (symbols_.find(modName)!=symbols_.end()) {
            // Adjust module's address, if necessary, to ensure that it's <= the
            // address of this new symbol
           
            std::vector< Symbol *> & mod_symbols = symbols_[modName];
           
            Symbol *mod_symbol = mod_symbols[ 0 ];

            for (unsigned i = 0; i < mod_symbols.size(); i++) {
                if (mod_symbols[i]->getType() == Symbol::ST_MODULE) {
                    mod_symbol->offset_ = value;
                }
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
         std::string::size_type len = mf->pathname().length(); 
         if (((len>2)&&(mf->pathname().substr(len-2,2)==".a")) ||
             ((len>3)&&(mf->pathname().substr(len-3,3)==".so")) ||
             ((len>5)&&(mf->pathname().substr(len-5,5)==".so.1")))
            modName = mf->pathname();
         else if (name == "glink.s")
            modName = std::string("Global_Linkage");
         else {
            modName = name;
         }
         Symbol *modSym = new Symbol(modName, 
                                     Symbol::ST_MODULE, 
                                     linkage, 
                                     visibility,
                                     UINT_MAX); // dummy address for now!

         /* The old code always had the last module win. */
         if ( symbols_[modName].size() == 0 ) {
            symbols_[modName].push_back( modSym );
         } else {
            //  not sure if this is dangerous (anyone else have a pointer to this object?)
            //  but if we don't delete, or do something else, its a leak.
            delete (symbols_[modName][0]);
            symbols_[modName][0] = modSym;
         }
         
         continue;
      }
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
              mf->pathname().c_str(), errType); \
      err_func_(errType); \
      return; \
      }
   
void Object::load_archive(bool is_aout, bool alloc_syms) 
{
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
            parse_aout(archive->aout_offset, is_aout, alloc_syms);
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

void Object::load_object(bool alloc_syms)
{
   // Load in an object (archive, object, .so)

   assert(fo_);

   unsigned char magic_number[2];
   unsigned short f_flags;

   if (!fo_->set(0)) {
      sprintf(errorLine, "Error reading file %s\n", 
            mf->pathname().c_str());
      err_func_(errorLine); 
      //statusLine(errorLine);
      //showErrorCallback(49,(const char *) errorLine);
      return;
   }

   if (!fo_->read((void *)magic_number, 2)) {
      sprintf(errorLine, "Error reading file %s\n", 
            mf->pathname().c_str());
      err_func_(errorLine); 
      //statusLine(errorLine);
      //showErrorCallback(49,(const char *) errorLine);
      return;
   }

   if (!fo_->set(18)){
      sprintf(errorLine, "Error moving to the offset 18 file %s\n", 
            mf->pathname().c_str());
      err_func_(errorLine); 
      //statusLine(errorLine);
      //showErrorCallback(49,(const char *) errorLine);
   }   

   if (!fo_->read((void *)(&f_flags), 2)) {
      sprintf(errorLine, "Error reading file %s\n", 
            mf->pathname().c_str());
      err_func_(errorLine); 
      //statusLine(errorLine);
      //showErrorCallback(49,(const char *) errorLine);
      return;
   }

    if (magic_number[0] == 0x01) {
        if (magic_number[1] == 0xdf ||
	    magic_number[1] == 0xef ||
	    magic_number[1] == 0xf7)
	{
	    parse_aout(0, is_aout_, alloc_syms);
	}
    }
    else if (magic_number[0] == '<') // archive of some sort
    {
	// What?  Why aren't we calling load_archive here?
	// load_archive(true, alloc_syms);
	is_aout_ = !(f_flags & F_SHROBJ);
	parse_aout(offset_, true, alloc_syms);
    }	
    else {// Fallthrough
        sprintf(errorLine, "Bad magic number in file %s\n",
                mf->pathname().c_str());
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

Object::Object(const Object& obj) :
   AObject(obj) 
{
   fo_ = fileOpener::openFile((void *)obj.fo_->mem_image(), obj.fo_->size());
}


Object::Object(MappedFile *mf_, MappedFile *mfd, void (*err_func)(const char *), Offset offset, bool alloc_syms) :
   AObject(mf_, mfd, err_func), offset_(offset)
{    
   fo_ = fileOpener::openFile((void *)mf_->base_addr(), mf_->size());
   fo_->set_file(mf_->filename());
   load_object(alloc_syms); 
}

Object::Object(MappedFile *mf_, MappedFile *mfd, std::string &member_name, Offset offset, void (*err_func)(const char *), void *) :
   AObject(mf_, mfd, err_func), member_(member_name), offset_(offset)
{    
   fo_ = fileOpener::openFile((void *)mf_->base_addr(), mf_->size());
   fo_->set_file(member_name);
   load_object(true); 
}

Object& Object::operator=(const Object& obj) 
{
   (void) AObject::operator=(obj);
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
    bool is64 = (objPtr->getAddressWidth() == 8);
    SYMENT *syms;
    int stab_nsyms;
    char *stringPool;
    union auxent *aux;
    char *stabstr=NULL;
    void *syms_void = NULL;

    objPtr->get_stab_info(stabstr, stab_nsyms, syms_void, stringPool);
    syms = static_cast<SYMENT *>(syms_void);
    for (int i=0;i<stab_nsyms;i++) {
        SYMENT *sym = (SYMENT *) (((char *) syms) + i * SYMESZ);
        char tempName[15];
        char *compilerName = NULL;

        std::string name;
        if (sym->n_sclass == C_FILE) {
	    if (is64) {
		name = std::string( &stringPool[ sym->n_offset64 ] );

	    } else if (!sym->n_zeroes32) {
                name = std::string( &stringPool[ sym->n_offset32 ] );

            } else {
                char tempName[9];
                memset(tempName, 0, 9);
                strncpy(tempName, sym->n_name32, 8);
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
                const char * const compiler_strings[] = {
                   "IBM.*VisualAge.*C\\+\\+",
                   "IBM.* XL .*C\\+\\+",
                   NULL};

                for (const char * const *cpp = compiler_strings; *cpp != NULL; cpp++) {
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
void Object::getModuleLanguageInfo(dyn_hash_map<std::string, supportedLanguages> *)
{
}

ObjectType Object::objType() const {
   return is_aout() ? obj_Executable : obj_SharedLib;
}

bool Object::emitDriver(Symtab * /*obj*/, 
      string /*fName*/, 
      std::vector<Symbol *>&/*allSymbols*/, 
      unsigned /*flag*/) 
{
   return true;
}

bool AObject::getSegments(vector<Segment> &/*segs*/) const
{
   return true;
}

/* FIXME: hack. */
Offset trueBaseAddress = 0;

void Object::parseFileLineInfo(Symtab *, dyn_hash_map<std::string, LineInformation> &li)
{
   static set<std::string> haveParsedFileMap;

   //cerr << "parsing line info for file :" << mf->pathname() << endl;

   if ( haveParsedFileMap.count(mf->pathname()) != 0 ) { return; }

   // /* DEBUG */ fprintf( stderr, "%s[%d]: Considering image at 0x%lx\n", __FILE__, __LINE__, fileOnDisk );

   /* FIXME: hack.  Should be argument to parseLineInformation(), which should in turn be merged
      back into here so it can tell how far to extend the range of the last line information point. */

   //Offset baseAddress = obj()->codeBase();

   if (is_aout())
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

   for ( int i = 0; i < nstabs; i++ ) 
   {
      /* sizeof( SYMENT ) is 20, not 18, as it should be. */

      SYMENT * sym = (SYMENT *)( (char *)syms + (i * SYMESZ) );

      /* Get the name (period) */

      if (is64_) 
      {
         moduleName = &stringpool[ sym->n_offset64 ];
      } 
      else if (!sym->n_zeroes32) 
      {
         moduleName = &stringpool[ sym->n_offset32 ];
      } 
      else 
      {
         memset(temporaryName, 0, 9);
         strncpy(temporaryName, sym->n_name32, 8);
         moduleName = temporaryName;
      }

      /* Extract the current source file from the C_FILE entries. */

      if ( sym->n_sclass == C_FILE ) 
      {

         if (!strcmp(moduleName, ".file")) 
         {
            // The actual name is in an aux record.
            int j;

            /* has aux record with additional information. */

            for (j=1; j <= sym->n_numaux; j++) 
            {

               union auxent *aux = (union auxent *) ((char *) sym + j * SYMESZ);

               if (aux->x_file._x.x_ftype == XFT_FN) 
               {
                  // this aux record contains the file name.
                  if (!aux->x_file._x.x_zeroes) 
                  {
                     moduleName = &stringpool[aux->x_file._x.x_offset];
                  } 
                  else 
                  {
                     // x_fname is 14 bytes
                     memset(temporaryName, 0, 15);
                     strncpy(temporaryName, aux->x_file.x_fname, 14);
                     moduleName = temporaryName;
                  }
               }
            }
         }

         currentSourceFile = strrchr( moduleName, '/' );

         if ( currentSourceFile == NULL ) 
         {
            currentSourceFile = moduleName; 
         }
         else 
         {
            ++currentSourceFile; 
         }

         /* We're done with this entry. */
         continue;
      }/* end if C_FILE */

      /* This apparently compensates for a bug in the naming of certain entries. */

      char * nmPtr = NULL;

      if (!sym->n_zeroes32 && ( ( sym->n_sclass & DBXMASK  ) ||
               ( sym->n_sclass == C_BINCL ) ||
               ( sym->n_sclass == C_EINCL ) ) ) 
      {
         long sym_offset = (is64_ ? sym->n_offset64 : sym->n_offset32);

         if ( sym_offset < 3 ) 
         {
            if ( sym_offset == 2 && stabstr[ 0 ] ) 
            {
               nmPtr = & stabstr[ 0 ];
            } 
            else 
            {
               nmPtr = & stabstr[ sym_offset ];
            }
         } 
         else if( ! stabstr[ sym_offset - 3 ] ) 
         {
            nmPtr = & stabstr[ sym_offset ];
         } 
         else 
         {
            /* has off by two error */
            nmPtr = & stabstr[ sym_offset - 2 ];
         }
      } 
      else if (is64_) 
      {
         nmPtr = &stringpool[sym->n_offset64];

      } 
      else if (!sym->n_zeroes32) 
      {
         nmPtr = &stringpool[sym->n_offset32];
      } 
      else 
      {
         // names 8 or less chars on inline, not in stabstr
         memset( temporaryName, 0, 9 );
         strncpy( temporaryName, sym->n_name32, 8 );
         nmPtr = temporaryName;
      } /* end bug compensation */

      /* Now that we've compensated for buggy naming, actually
         parse the line information. */
      if ( ( sym->n_sclass == C_BINCL ) ||
            ( sym->n_sclass == C_EINCL ) ||
            ( sym->n_sclass == C_FUN ) ) 
      {

         if ( funcName ) 
         {
            free( funcName );
            funcName = NULL;
         }

         funcName = strdup( nmPtr );

         std::string pdCSF( currentSourceFile );
         parseLineInformation(li, & pdCSF, funcName, (SYMENT *)sym, linesfdptr, lines, nlines );
      } /* end if we're actually parsing line information */
   } /* end iteration over STAB entries. */

   if ( funcName != NULL ) 
   {
      free( funcName );
   }

   haveParsedFileMap.insert(mf->pathname());
} /* end parseFileLineInfo() */

void Object::parseLineInformation(dyn_hash_map<std::string,  LineInformation> &lineInfo_, 
      std::string * currentSourceFile,
      char * symbolName,
      SYMENT * sym,
      Offset linesfdptr,
      char * lines,
      int nlines ) 
{
   union auxent * aux;
   std::vector<IncludeFileInfo> includeFiles;
   unsigned long value = (is64_ ? sym->n_value64 : sym->n_value32);
   unsigned long LINESZ = (is64_ ? LINESZ_64 : LINESZ_32);

   /* if it is beginning of include files then update the data structure
      that keeps the beginning of the include files. If the include files contain
      information about the functions and lines we have to keep it */

   if ( sym->n_sclass == C_BINCL ) 
   {
      includeFiles.push_back( IncludeFileInfo( (value - linesfdptr)/LINESZ, symbolName ) );
   }

   /* similiarly if the include file contains function codes and line information
      we have to keep the last line information entry for this include file */

   else if (sym->n_sclass == C_EINCL) 
   {
      if (includeFiles.size() > 0) 
      {
         includeFiles[includeFiles.size()-1].end = (value - linesfdptr)/LINESZ;
      }
   }
   else if ( sym->n_sclass == C_FUN ) 
   {
      /* if the enrty is for a function than we have to collect all info
         about lines of the function */

      /* I have no idea what the old code did, except not work very well.
         Somebody who understands XCOFF should look at this. */

      int initialLine = 0;
      int initialLineIndex = 0;
      Offset funcStartAddress = 0;
      Offset funcEndAddress = 0;

      for (int j = -1; ; --j) 
      {
         SYMENT * extSym = (SYMENT *)( ((char *)sym) + (j * SYMESZ) );

         if ( extSym->n_sclass == C_EXT || extSym->n_sclass == C_HIDEXT ) 
         {
            aux = (union auxent *)( ((char *)extSym) + SYMESZ );

            initialLineIndex = (is64_ ? aux->x_fcn64.x_lnnoptr
                  : aux->x_sym32.x_fcnary.x_fcn.x_lnnoptr);
            initialLineIndex = (initialLineIndex - linesfdptr )/LINESZ;

            funcStartAddress = (is64_ ? extSym->n_value64 : extSym->n_value32);
            break;
         } /* end if C_EXT found */

      } /* end search for C_EXT */

      /* access the line information now using the C_FCN entry*/

      SYMENT * bfSym = (SYMENT *)( ((char *)sym) + SYMESZ );

      if ( bfSym->n_sclass != C_FCN ) 
      {
         //bperr("unable to process line info for %s\n", symbolName);
         return;
      }

      SYMENT * efSym = (SYMENT *)( ((char *)bfSym) + (2 * SYMESZ) );

      while (efSym->n_sclass != C_FCN)
      {
         efSym = (SYMENT *) ( ((Offset)efSym) + SYMESZ );
      }

      funcEndAddress = (is64_ ? efSym->n_value64 : efSym->n_value32);

      aux = (union auxent *)( ((char *)bfSym) + SYMESZ );

      initialLine = (is64_ ? aux->x_sym64.x_misc.x_lnsz.x_lnno
            : aux->x_sym32.x_misc.x_lnsz.x_lnno);

      std::string whichFile = *currentSourceFile;

      for (unsigned int j = 0; j < includeFiles.size(); j++) 
      {
         if ( (includeFiles[j].begin <= (unsigned)initialLineIndex) &&
               (includeFiles[j].end   >= (unsigned)initialLineIndex) ) 
         {
            whichFile = includeFiles[j].name;
            break;
         }
      } /* end iteration of include files */

      unsigned int previousLineNo = 0;
      Offset previousLineAddr = 0;
      bool isPreviousValid = false;

      /* Iterate over this entry's lines. */

      for (int j = initialLineIndex + 1; j < nlines; j++ ) 
      {
         LINENO * lptr = (LINENO *)( lines + (j * LINESZ) );
         unsigned long lnno = (is64_ ? lptr->l_lnno64 : lptr->l_lnno32);
         unsigned long paddr = (is64_ ? lptr->l_addr64.l_paddr : lptr->l_addr32.l_paddr);

         if (! lnno) 
         { 
            break; 
         }

         unsigned int lineNo = lnno + initialLine - 1;
         Offset lineAddr = paddr + trueBaseAddress;

         if (isPreviousValid) 
         {
            // /* DEBUG */ fprintf( stderr, "%s[%d]: adding %s:%d [0x%lx, 0x%lx).\n", __FILE__, __LINE__, whichFile.c_str(), previousLineNo, previousLineAddr, lineAddr );

            unsigned current_col = 0;

            //if (previousLineNo == 596 || previousLineNo == 597)
            //{
            //cerr << "FuncEndAddress: " <<setbase(16) << funcEndAddress << setbase(10) << ",totallines:" << nlines << ":" << endl;
            //cerr << __FILE__ <<"[" << __LINE__ << "]:inserted address range [" << setbase(16) << previousLineAddr << "," << lineAddr << ") for source " << whichFile << ":" << setbase(10) << previousLineNo << endl;
            //}	

            lineInfo_[whichFile].addLine(whichFile.c_str(), previousLineNo, current_col, previousLineAddr, lineAddr );

            //currentLineInformation.addLine( whichFile.c_str(), previousLineNo, current_col, previousLineAddr, lineAddr );
         }

         previousLineNo = lineNo;
         previousLineAddr = lineAddr;
         isPreviousValid = true;
      } /* end iteration over line information */

      if (isPreviousValid) 
      {
         /* Add the instruction (always 4 bytes on power) pointed at by the last entry.  We'd like to add a
            bigger range, but it's not clear how.  (If the function has inlined code, we won't know about
            it until we see the next section, so claiming "until the end of the function" will give bogus results.) */
         // /* DEBUG */ fprintf( stderr, "%s[%d]: adding %s:%d [0x%lx, 0x%lx).\n", __FILE__, __LINE__, whichFile.c_str(), previousLineNo, previousLineAddr, previousLineAddr + 4 );

         while (previousLineAddr < funcEndAddress) 
         {
            unsigned current_col = 0;

            //if(previousLineNo == 596 || previousLineNo == 597)
            //  cerr << __FILE__ <<"[" << __LINE__ << "]:inserted address range [" << setbase(16) << previousLineAddr << "," <<  previousLineAddr + 4 << ") for source " << whichFile << ":" << setbase(10) << previousLineNo << endl;

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

   for (i=0; i < nstabs; i++) {
      /* do the pointer addition by hand since sizeof(struct syment)
       *   seems to be 20 not 18 as it should be */
      SYMENT *sym = (SYMENT *) (((char *) syms) + i * SYMESZ);
      unsigned long sym_value = (is64_ ? sym->n_value64 : sym->n_value32);

      if (sym->n_sclass == C_FILE) {
         char *moduleName;
         if (is64_) {
            moduleName = &stringPool[sym->n_offset64];
         } else if (!sym->n_zeroes32) {
            moduleName = &stringPool[sym->n_offset32];
         } else {
            memset(tempName, 0, 9);
            strncpy(tempName, sym->n_name32, 8);
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
		 typeCollection *tc = typeCollection::getModTypeCollection(mod);
		 assert(tc);
         tc->clearNumberedTypes();
         if(!obj->findModuleByName(mod, currentSourceFile) && 
            !obj->findModuleByName(mod,extract_pathname_tail(currentSourceFile)))
         {            
               mod = NULL;
         }
      }

      if (!mod)
         mod = obj->getDefaultModule();

      //num_active++;
      char *nmPtr;
      if (!sym->n_zeroes32 && ((sym->n_sclass & DBXMASK) ||
                               (sym->n_sclass == C_BINCL) ||
                               (sym->n_sclass == C_EINCL))) {
         long sym_offset = (is64_ ? sym->n_offset64 : sym->n_offset32);

         // Symbol name stored in STABS, not string pool.
         if(sym_offset < 3) {
            if (sym_offset == 2 && stabstr[0]) {
               nmPtr = &stabstr[0];
            } else {
               nmPtr = &stabstr[sym_offset];
            }
         } else if (!stabstr[sym_offset-3]) {
            nmPtr = &stabstr[sym_offset];
         } else {
            /* has off by two error */
            nmPtr = &stabstr[sym_offset-2];
         }
#if 0	 
         bperr("using nmPtr = %s\n", nmPtr);
         bperr("got n_offset = (%d) %s\n", sym_offset, &stabstr[sym_offset]);
         if (sym_offset>=2)
            bperr("got n_offset-2 = %s\n", &stabstr[sym_offset-2]);
         if (sym_offset>=3)
            bperr("got n_offset-3 = %x\n", stabstr[sym_offset-3]);
         if (sym_offset>=4)
            bperr("got n_offset-4 = %x\n", stabstr[sym_offset-4]);
#endif
      } else if (is64_) {
         nmPtr = &stringPool[sym->n_offset64];
      } else if (!sym->n_zeroes32) {
         nmPtr = &stringPool[sym->n_offset32];
      } else {
         // names 8 or less chars on inline, not in stabstr
         memset(tempName, 0, 9);
         strncpy(tempName, sym->n_name32, 8);
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
            if(!obj->findSymbolByType(vars, commonBlockName, Symbol::ST_OBJECT, mangledName)) {
                if(!obj->findSymbolByType(vars, commonBlockName, Symbol::ST_OBJECT, prettyName))
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
				typeCollection *tc = typeCollection::getModTypeCollection(mod);
				assert(tc);
               commonBlock = dynamic_cast<typeCommon *>(tc->findVariableType(cbName));
               if (commonBlock == NULL) {
                  // its still the null type, create a new one for it
                  //TODO? ? ID for this typeCommon ?
                  commonBlock = new typeCommon(cbName);
                  tc->addGlobalVariable(cbName, commonBlock);
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
            if (obj->findSymbolByType(bpmv, funcName, Symbol::ST_FUNCTION, mangledName) || !bpmv.size()) {
               string newName = "." + funcName;
               if (obj->findSymbolByType(bpmv, newName, Symbol::ST_FUNCTION, mangledName) || !bpmv.size()) {
                  //bperr("unable to locate current function %s\n", funcName.c_str());
               }
               else{
                  Symbol *func = bpmv[0];
                  commonBlock->endCommonBlock(func, (void *)commonBlockVar->getAddr());
               }
            } else {
               Symbol *func = bpmv[0];
               commonBlock->endCommonBlock(func, (void *)commonBlockVar->getAddr());
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
            tsym = (SYMENT *) (((char *) syms) + sym_value * SYMESZ);

            // We can't lookup the value by name, because the name might have been
            // redefined later on (our lookup would then pick the last one)

            // Since this whole function is AIX only, we're ok to get this info

            staticBlockBaseAddr = (is64_ ? tsym->n_value64 : tsym->n_value32);

            /*
               char *staticName, tempName[9];
               if (is64_) {
               staticName = &stringPool[tsym->n_offset64];
               } else if (!tsym->n_zeroes32) {
               staticName = &stringPool[tsym->n_offset32];
               } else {
               memset(tempName, 0, 9);
               strncpy(tempName, tsym->n_name32, 8);
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
            if(!obj->findModuleByName(mod, fName))
            {
               modName = obj->file();
               if(!obj->findModuleByName(mod, modName))
                  continue;
            }
         }

         if (staticBlockBaseAddr && (sym->n_sclass == C_STSYM)) {
            parseStabString(mod, 0, nmPtr, 
                  sym_value+staticBlockBaseAddr, commonBlock);
         } else {
            parseStabString(mod, 0, nmPtr, sym_value, commonBlock);
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

bool Region::isStandardCode()
{
   return (getRegionPermissions() == RP_RX ||
           getRegionPermissions() == RP_RWX);
}

Dyninst::Architecture Object::getArch()
{
   if (getAddressWidth() == 4)
      return Dyninst::Arch_ppc32;
   return Dyninst::Arch_ppc64;
}
