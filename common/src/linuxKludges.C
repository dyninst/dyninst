/*
 * See the dyninst/COPYRIGHT file for copyright information.
 *
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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

#include "common/src/headers.h"
#include "common/src/parseauxv.h"
#include "common/src/linuxKludges.h"
#include <elf.h>

#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <sys/uio.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>


/**** process_vm_readv / process_vm_writev
 * Added in kernel 3.2 and some backports -- try it and check ENOSYS.
 * The wrappers are defined in glibc 2.15, otherwise make our own.
 */
#if !__GLIBC_PREREQ(2,15)

static ssize_t process_vm_readv(pid_t pid,
    const struct iovec *local_iov, unsigned long liovcnt,
    const struct iovec *remote_iov, unsigned long riovcnt,
    unsigned long flags)
{
#ifdef SYS_process_vm_readv
  return syscall(SYS_process_vm_readv,
      pid, local_iov, liovcnt, remote_iov, riovcnt, flags);
#else
  errno = ENOSYS;
  return -1;
#endif
}

static ssize_t process_vm_writev(pid_t pid,
    const struct iovec *local_iov, unsigned long liovcnt,
    const struct iovec *remote_iov, unsigned long riovcnt,
    unsigned long flags)
{
#ifdef SYS_process_vm_writev
  return syscall(SYS_process_vm_writev,
      pid, local_iov, liovcnt, remote_iov, riovcnt, flags);
#else
  errno = ENOSYS;
  return -1;
#endif
}

#endif /* !__GLIBC_PREREQ(2,15) */


int P_getopt(int argc, char *argv[], const char *optstring)
{
  /* On linux we prepend a + character */
  std::string newopt{"+"};
  newopt += optstring;
  return getopt(argc, argv, newopt.c_str());
}

int P_copy(const char *from, const char *to) {
    std::ifstream src(from, std::ios::binary);
    std::ofstream dst(to, std::ios::binary | std::ios::trunc);
    dst << src.rdbuf();
    dst.close();
    src.close();
    return (src && dst) ? 0 : -1;
}



#include "symbolDemangleWithCache.h"

std::string P_cplus_demangle( const std::string &symbol, bool includeTypes )
{
    return symbol_demangle_with_cache(symbol, includeTypes);
} /* end P_cplus_demangle() */


bool PtraceBulkRead(Dyninst::Address inTraced, unsigned size, void *inSelf, int pid)
{
   static bool have_process_vm_readv = true;

   const unsigned char *ap = (const unsigned char*) inTraced;
   unsigned char *dp = (unsigned char *) inSelf;
   Dyninst::Address w = 0x0;               /* ptrace I/O buffer */
   int len = sizeof(void *);
   unsigned cnt;

   if (0 == size) {
      return true;
   }

   /* If process_vm_readv is available, we may be able to read it all in one syscall. */
   if (have_process_vm_readv) {
      struct iovec local_iov = { inSelf, size };
      struct iovec remote_iov = { (void*)inTraced, size };
      ssize_t ret = process_vm_readv(pid, &local_iov, 1, &remote_iov, 1, 0);
      if (ret == -1) {
         if (errno == ENOSYS) {
            have_process_vm_readv = false;
         } else if (errno == EFAULT || errno == EPERM) {
            /* Could be a no-read page -- ptrace may be allowed to
             * peek anyway, so fallthrough and let ptrace try.
             * It may also be denied by kernel.yama.ptrace_scope=1 if we're
             * no longer a direct ancestor thanks to pid re-parenting.  */
         } else {
            return false;
         }
      } else if (ret < size) {
         /* partial reads won't split an iovec, but we only have one... huh?! */
         return false;
      } else {
         return true;
      }
   }

   cnt = inTraced % len;
   if (cnt) {
      /* Start of request is not aligned. */
      unsigned char *p = (unsigned char*) &w;

      /* Read the segment containing the unaligned portion, and
         copy what was requested to DP. */
      errno = 0;
      w = P_ptrace(PTRACE_PEEKDATA, pid, (Dyninst::Address) (ap-cnt), w, len);
      if (errno) {
         return false;
      }
      for (unsigned i = 0; i < len-cnt && i < size; i++)
         dp[i] = p[cnt+i];

      if (len-cnt >= size) {
         return true; /* done */
      }

      dp += len-cnt;
      ap += len-cnt;
      size -= len-cnt;
   }
   /* Copy aligned portion */
   while (size >= (u_int)len) {
      errno = 0;
      w = P_ptrace(PTRACE_PEEKTEXT, pid, (Dyninst::Address) ap, 0, len);
      if (errno) {
         return false;
      }
      memcpy(dp, &w, len);
      dp += len;
      ap += len;
      size -= len;
   }

   if (size > 0) {
      /* Some unaligned data remains */
      unsigned char *p = (unsigned char *) &w;

      /* Read the segment containing the unaligned portion, and
         copy what was requested to DP. */
      errno = 0;
      w = P_ptrace(PTRACE_PEEKTEXT, pid, (Dyninst::Address) ap, 0, len);
      if (errno) {
         return false;
      }
      for (unsigned i = 0; i < size; i++)
         dp[i] = p[i];
   }
   return true;

}

bool PtraceBulkWrite(Dyninst::Address inTraced, unsigned nbytes,
                     const void *inSelf, int pid)
{
   static bool have_process_vm_writev = true;

   unsigned char *ap = (unsigned char*) inTraced;
   const unsigned char *dp = (const unsigned char*) inSelf;
   Dyninst::Address w = 0x0;               /* ptrace I/O buffer */
   int len = sizeof(Dyninst::Address); /* address alignment of ptrace I/O requests */
   unsigned cnt;

   if (0 == nbytes) {
      return true;
   }

   /* If process_vm_writev is available, we may be able to write it all in one syscall. */
   if (have_process_vm_writev) {
      struct iovec local_iov = { const_cast<void*>(inSelf), nbytes };
      struct iovec remote_iov = { (void*)inTraced, nbytes };
      ssize_t ret = process_vm_writev(pid, &local_iov, 1, &remote_iov, 1, 0);
      if (ret == -1) {
         if (errno == ENOSYS) {
            have_process_vm_writev = false;
         } else if (errno == EFAULT || errno == EPERM) {
            /* Could be a read-only page -- ptrace may be allowed to
             * poke anyway, so fallthrough and let ptrace try.
             * It may also be denied by kernel.yama.ptrace_scope=1 if we're
             * no longer a direct ancestor thanks to pid re-parenting.  */
         } else {
            return false;
         }
      } else if (ret < nbytes) {
         /* partial writes won't split an iovec, but we only have one... huh?! */
         return false;
      } else {
         return true;
      }
   }

   if ((cnt = ((Dyninst::Address)ap) % len)) {
      /* Start of request is not aligned. */
      unsigned char *p = (unsigned char*) &w;

      /* Read the segment containing the unaligned portion, edit
         in the data from DP, and write the segment back. */
      errno = 0;
      w = P_ptrace(PTRACE_PEEKTEXT, pid, (Dyninst::Address) (ap-cnt), 0);

      if (errno) {
         return false;
      }

      for (unsigned i = 0; i < len-cnt && i < nbytes; i++)
         p[cnt+i] = dp[i];

      if (0 > P_ptrace(PTRACE_POKETEXT, pid, (Dyninst::Address) (ap-cnt), w)) {
         return false;
      }

      if (len-cnt >= nbytes) {
         return true; /* done */
      }

      dp += len-cnt;
      ap += len-cnt;
      nbytes -= len-cnt;
   }

   /* Copy aligned portion */
   while (nbytes >= (u_int)len) {
      assert(0 == ((Dyninst::Address)ap) % len);
      memcpy(&w, dp, len);
      int retval =  P_ptrace(PTRACE_POKETEXT, pid, (Dyninst::Address) ap, w);
      if (retval < 0) {
         return false;
      }

      // Check...
      dp += len;
      ap += len;
      nbytes -= len;
   }

   if (nbytes > 0) {
      /* Some unaligned data remains */
      unsigned char *p = (unsigned char *) &w;

      /* Read the segment containing the unaligned portion, edit
         in the data from DP, and write it back. */
      errno = 0;
      w = P_ptrace(PTRACE_PEEKTEXT, pid, (Dyninst::Address) ap, 0);

      if (errno) {
         return false;
      }


      for (unsigned i = 0; i < nbytes; i++)
         p[i] = dp[i];

      if (0 > P_ptrace(PTRACE_POKETEXT, pid, (Dyninst::Address) ap, w)) {
         return false;
      }
   }
   return true;
}

// These constants are not defined in all versions of elf.h
#ifndef AT_BASE
#define AT_BASE 7
#endif
#ifndef AT_NULL
#define AT_NULL 0
#endif
#ifndef AT_SYSINFO
#define AT_SYSINFO 32
#endif
#ifndef AT_SYSINFO_EHDR
#define AT_SYSINFO_EHDR 33
#endif

static bool couldBeVsyscallPage(map_entries *entry, bool strict, Dyninst::Address) {
   if (strict) {
       if (entry->prems != PREMS_PRIVATE)
         return false;
      if (entry->path[0] != '\0')
         return false;
   }
   if (entry->offset != 0)
      return false;
   if (entry->dev_major != 0 || entry->dev_minor != 0)
      return false;
   if (entry->inode != 0)
      return false;

   return true;
}

bool AuxvParser::readAuxvInfo()
{
  /**
   * The location of the vsyscall is stored in /proc/PID/auxv in Linux 2.6.
   * auxv consists of a list of name/value pairs, ending with the AT_NULL
   * name.  There isn't a direct way to get the vsyscall info on Linux 2.4
   **/
  uint32_t *buffer32 = NULL;
  uint64_t *buffer64 = NULL;
  unsigned pos = 0;
  Dyninst::Address dso_start = 0x0, text_start = 0x0;

  struct {
    unsigned long type;
    unsigned long value;
  } auxv_entry;

  /**
   * Try to read from /proc/%d/auxv.  On Linux 2.4 systems auxv
   * doesn't exist, which is okay because vsyscall isn't used.
   * On latter 2.6 kernels the AT_SYSINFO field isn't present,
   * so we have to resort to more "extreme" measures.
   **/
  buffer64 = (uint64_t *) readAuxvFromProc();
  if (!buffer64) {
     buffer64 = (uint64_t *) readAuxvFromStack();
  }
  if (!buffer64) {
     return false;
  }
  buffer32 = (uint32_t *) buffer64;
  do {
     /**Fill in the auxv_entry structure.  We may have to do different
      * size reads depending on the address space.  No matter which
      * size we read, we'll fill the data in to auxv_entry, which may
      * involve a size shift up.
      **/
     if (addr_size == 4) {
        auxv_entry.type = (unsigned long) buffer32[pos];
        pos++;
        auxv_entry.value = (unsigned long) buffer32[pos];
        pos++;
     }
     else {
        auxv_entry.type = (unsigned long) buffer64[pos];
        pos++;
        auxv_entry.value = (unsigned long) buffer64[pos];
        pos++;
     }

     switch(auxv_entry.type) {
        case AT_SYSINFO:
           text_start = auxv_entry.value;
           break;
        case AT_SYSINFO_EHDR:
           dso_start = auxv_entry.value;
           break;
        case AT_PAGESZ:
           page_size = auxv_entry.value;
           break;
        case AT_BASE:
           interpreter_base = auxv_entry.value;
           break;
        case AT_PHDR:
           phdr = auxv_entry.value;
           break;
     }

  } while (auxv_entry.type != AT_NULL);


  if (buffer64)
     free(buffer64);
  if (!page_size)
     page_size = getpagesize();
//#if !defined(arch_x86) && !defined(arch_x86_64)
#if !defined(arch_x86) && !defined(arch_x86_64) && !defined(arch_aarch64)
  //No vsyscall page needed or present
  return true;
#endif

  /**
   * Even if we found dso_start in /proc/pid/auxv, the vsyscall 'page'
   * can be larger than a single page.  Thus we look through /proc/pid/maps
   * for known, default, or guessed start address(es).
   **/
  std::vector<Dyninst::Address> guessed_addrs;

  /* The first thing to check is the auxvinfo, if we have any. */
  if( dso_start != 0x0 )
     guessed_addrs.push_back( dso_start );

  /**
   * We'll make several educatbed attempts at guessing an address
   * for the vsyscall page.  After deciding on a guess, we'll try to
   * verify that using /proc/pid/maps.
   **/

  // Guess some constants that we've seen before.
#if defined(arch_x86)
  guessed_addrs.push_back(0xffffe000); //Many early 2.6 systems
  guessed_addrs.push_back(0xffffd000); //RHEL4
#endif
#if defined(arch_x86_64)
  guessed_addrs.push_back(0xffffffffff600000);
#endif

  /**
   * Look through every entry in /proc/maps, and compare it to every
   * entry in guessed_addrs.  If a guessed_addr looks like the right
   * thing, then we'll go ahead and call it the vsyscall page.
   **/
  unsigned num_maps;
  map_entries *secondary_match = NULL;
  map_entries *maps = getVMMaps(pid, num_maps);
  for (unsigned i=0; i<guessed_addrs.size(); i++) {
     Dyninst::Address addr = guessed_addrs[i];
     for (unsigned j=0; j<num_maps; j++) {
        map_entries *entry = &(maps[j]);
        if (addr < entry->start || addr >= entry->end)
           continue;

        if (dso_start == entry->start ||
            couldBeVsyscallPage(entry, true, page_size)) {
           //We found a possible page using a strict check.
           // This is really likely to be it.
           vsyscall_base = entry->start;
           vsyscall_end = entry->end;
           vsyscall_text = text_start;
           found_vsyscall = true;
           free(maps);
           return true;
        }

        if (couldBeVsyscallPage(entry, false, page_size)) {
           //We found an entry that loosely looks like the
           // vsyscall page.  Let's hang onto this and return
           // it if we find nothing else.
           secondary_match = entry;
        }
     }
  }

  /**
   * There were no hits using our guessed_addrs scheme.  Let's
   * try to look at every entry in the maps table (not just the
   * guessed addresses), and see if any of those look like a vsyscall page.
   **/
  for (unsigned i=0; i<num_maps; i++) {
     if (couldBeVsyscallPage(&(maps[i]), true, page_size)) {
        vsyscall_base = maps[i].start;
        vsyscall_end = maps[i].end;
        vsyscall_text = text_start;
        found_vsyscall = true;
        free(maps);
        return true;
     }
  }

  /**
   * Return any secondary possiblitiy pages we found in our earlier search.
   **/
  if (secondary_match) {
     vsyscall_base = secondary_match->start;
     vsyscall_end = secondary_match->end;
     vsyscall_text = text_start;
     found_vsyscall = true;
     free(maps);
     return true;
  }

  /**
   * Time to give up.  Sigh.
   **/
  found_vsyscall = false;
  free(maps);
  return false;
}

#if 0
/**
 * get_word_at is a helper function for readAuxvFromStack.  It reads
 * a word out of the mutatee's stack via the debugger interface, and
 * it keeps the word cached for future reads.
 * The gwa_* global variables are basically parameters to get_word_at
 * and should be reset before every call
 *
 * gwa_buffer is a cache of data we've read before.  It's backwards
 * for convience, higher addresses are cached towards the base of gwa_buffer
 * and lower addresses are cached at the top.  This is because we read from
 * high addresses to low ones, but we want to start caching at the start of
 * gwa_buffer.
 **/
static unsigned long *gwa_buffer = NULL;
static unsigned gwa_size = 0;
static unsigned gwa_pos = 0;
static unsigned long gwa_base_addr = 0;

static unsigned long get_word_at(process *p, unsigned long addr, bool &err) {
   bool result;
   unsigned word_size = p->getAddressWidth();
   unsigned long word;

   /**
    * On AMD64 controlling 32-bit mutatee words are 32 bits long.
    * We don't want to deal with this now, so treat as a 64 bit read
    * (from aligned_addr) and then pick the correct 32 bits to return
    * at the end of this function.
    **/
   unsigned long aligned_addr = addr;
   if (word_size == 4 && sizeof(long) == 8 && addr % 8 == 4)
      aligned_addr -= 4;

   /**
    * Allocate gwa_buffer on first call
    **/
   if (gwa_buffer == NULL) {
      gwa_buffer = (unsigned long *) malloc(gwa_size);
   }

   /**
    * If gwa_buffer isn't big enough, grow it.
    **/
   if (gwa_base_addr - gwa_size >= aligned_addr) {
      while (gwa_base_addr - gwa_size >= aligned_addr)
         gwa_size = gwa_size * 2;
      gwa_buffer = (unsigned long *) realloc(gwa_buffer, gwa_size);
   }

   /**
    * Keep adding words to the cache (gwa_buffer) until we've cached
    * the word the user is interested in.
    **/
   while (gwa_base_addr - (gwa_pos * sizeof(long)) >= aligned_addr) {
      result = p->readDataSpace((void *) aligned_addr, sizeof(long), &word, false);
      if (!result) {
         err = true;
         return 0x0;
      }
      gwa_buffer[gwa_pos] = word;
      gwa_pos++;
   }

   /**
    * Return the word the user wants out of the cache.  'word' is the
    * long value we want to return.  On 64-bit mutator/32-bit mutatees
    * we may need to return a specific 32-bits of word.
    **/
   word = gwa_buffer[(gwa_base_addr - aligned_addr) / sizeof(long)];

   if (word_size == 4 && sizeof(long) == 8 && addr % 8 == 4) {
      //64-bit mutator, 32 bit mutatee, looking for unaligned word
      uint32_t *words = (uint32_t *) &word;
      return (long) words[1];
   }
   else if (word_size == 4 && sizeof(long) == 8)
   {
      //64-bit mutator, 32 bit mutatee, looking for aligned word
      uint32_t *words = (uint32_t *) &word;
      return (long) words[0];
   }
   else
   {
      //mutator and mutatee are same size
      return word;
   }
}


/**
 * Another helper function for readAuxvInfoFromStack.  We want to know
 * the top byte of the stack.  Unfortunately, if we're running this it's
 * probably because /proc/PID/ isn't reliable, so we can't use maps.
 * Check the machine's stack pointer, page align it, and start walking
 * back looking for an unaccessible page.
 **/
static Dyninst::Address getStackTop(AddrSpaceReader *proc, bool &err) {
   Dyninst::Address stack_pointer;
   Dyninst::Address pagesize = getpagesize();
   bool result;
   long word;
   err = false;


   stack_pointer = proc->readRegContents(PTRACE_REG_SP);
   dyn_lwp *init_lwp = proc->getInitialLwp();
   if (!init_lwp) {
      err = true;
      return 0x0;
   }

   Frame frame = init_lwp->getActiveFrame();
   stack_pointer = frame.getSP();
   if (!stack_pointer) {
      err = true;
      return 0x0;
   }

   //Align sp to pagesize
   stack_pointer = (stack_pointer & ~(pagesize - 1)) + pagesize;

   //Read pages until we get to an unmapped page
   for (;;) {
      result = proc->readDataSpace((void *) stack_pointer, sizeof(long), &word,
                                   false);
      if (!result) {
         break;
      }
      stack_pointer += pagesize;
   }

   //The vsyscall page sometimes hangs out above the stack.  Test if this
   // page is it, then move back down one if it is.
   char pagestart[4];
   result = proc->readDataSpace((void *) (stack_pointer - pagesize), 4, pagestart,
                                false);
   if (result) {
      if (pagestart[0] == 0x7F && pagestart[1] == 'E' &&
          pagestart[2] == 'L' &&  pagestart[3] == 'F')
      {
         stack_pointer -= pagesize;
      }
   }

   return stack_pointer;
}

/**
 * We can't read /proc/PID/auxv for some reason (BProc is a likely candidate).
 * We'll instead pull this data from the mutatee's stack.  On Linux the top of
 * the stack at process startup is arranged like the following:
 *          -------------------------------------
 * esp ->   |                argc               |
 *          |               argv[0]             |
 *          |                ...                |
 *          |               argv[n]             |
 *          |                                   |
 *          |               envp[0]             |
 *          |                ...                |
 *          |               envp[n]             |
 *          |                NULL               |
 *          |                                   |
 *          |  { auxv[0].type, auxv[0].value }  |
 *          |                ...                |
 *          |  { auxv[n].type, auxv[n].value }  |
 *          |  {      NULL   ,     NULL      }  |
 *          |                                   |
 *          |      Some number of NULL words    |
 *          |        Strings for argv[]         |
 *          |        Strings for envp[]         |
 *          |                NULL               |
 *          -------------------------------------
 *
 * We want to get at the name/value pairs of auxv.  Unfortunately,
 * if we're attaching the stack pointer has probably moved.  Instead
 * we'll try to read the from the bottom up, which is more difficult.
 * argv[] and envp[] are pointers to the strings at the bottom of
 * the stack.  We'll search backwards for these pointers, then move back
 * down until we think we have the auxv array.  Yea us.
 **/
void *AuxvParser::readAuxvFromStack(process *proc) {
   gwa_buffer = NULL;
   gwa_size = 1024 * 1024; //One megabyte default
   gwa_pos = 0;
   unsigned word_size = proc->getAddressWidth();
   bool err = false;

   // Get the base address of the mutatee's stack.  For example,
   //  on many standard linux/x86 machines this will return
   //  0xc0000000
   gwa_base_addr = getStackTop(proc, err);
   if (err)
      return NULL;
   gwa_base_addr -= word_size;

   unsigned long current = gwa_base_addr;
   unsigned long strings_start, strings_end;
   unsigned long l1, l2, auxv_start, word;
   unsigned char *buffer = NULL;
   unsigned bytes_to_read;

   // Go through initial NULL word
   while (get_word_at(proc, current, err) == 0x0) {
      if (err) goto done_err;
      current -= word_size;
   }

   // Go through the auxv[] and envp[] strings
   strings_end = current;
   while (get_word_at(proc, current, err) != 0x0) {
      if (err) goto done_err;
      current -= word_size;
   }
   strings_start = current + word_size;

   //Read until we find a pair of pointers into the strings
   // section, this should mean we're now above the auxv vector
   // and in envp or argv
   for (;;) {
      l1 = get_word_at(proc, current, err);
      if (err) goto done_err;
      l2 = get_word_at(proc, current - word_size, err);
      if (err) goto done_err;
      if (l1 >= strings_start && l1 < strings_end &&
          l2 >= strings_start && l2 < strings_end)
         break;
      current -= word_size;
   }

   //Read back down until we get to the end of envp[]
   while (get_word_at(proc, current, err) != 0x0) {
      if (err) goto done_err;
      current += word_size;
   }
   //Through the NULL byte before auxv..
   while (get_word_at(proc, current, err) == 0x0) {
      if (err) goto done_err;
      current += word_size;
   }

   //Success. Found the start of auxv.
   auxv_start = current;

   //Read auxv into buffer
   bytes_to_read = strings_start - auxv_start;
   buffer = (unsigned char *) malloc(bytes_to_read + word_size*2);
   if (!buffer)
      goto done_err;
   for (unsigned pos = 0; pos < bytes_to_read; pos += word_size) {
      word = get_word_at(proc, auxv_start + pos, err);
      if (err) goto done_err;
      if (word_size == 4)
         *((uint32_t *) (buffer + pos)) = (uint32_t) word;
      else
         *((unsigned long *) (buffer + pos)) = word;
   }

   goto done;

 done_err:
   if (buffer)
      free(buffer);
   buffer = NULL;
 done:
   if (gwa_buffer)
      free(gwa_buffer);
   return (void *) buffer;
}

#else

void *AuxvParser::readAuxvFromStack() {
   /**
    * Disabled, for now.  Re-enable if /proc/pid/auxv doesn't exist.
    **/
   return NULL;
}

#endif

#define READ_BLOCK_SIZE (1024 * 5)
void *AuxvParser::readAuxvFromProc() {
   char filename[64];
   unsigned char *buffer = NULL;
   unsigned char *temp;
   unsigned buffer_size = READ_BLOCK_SIZE;
   unsigned pos = 0;
   ssize_t result = 0;
   int fd = -1;

   sprintf(filename, "/proc/%d/auxv", pid);
   fd = open(filename, O_RDONLY, 0);
   if (fd == -1)
      goto done_err;

   buffer = (unsigned char *) malloc(buffer_size);
   if (!buffer) {
      goto done_err;
   }

   for (;;) {
      result = read(fd, buffer + pos, READ_BLOCK_SIZE);
      if (result == -1) {
         perror("Couldn't read auxv entry");
         goto done_err;
      }
      else if (!result && !pos) {
         //Didn't find any data to read
         perror("Could read auxv entry");
         goto done_err;
      }
      else if (result < READ_BLOCK_SIZE) {
         //Success
         goto done;
      }
      else if (result == READ_BLOCK_SIZE) {
         //WTF... 5k wasn't enough for auxv?
         buffer_size *= 2;
         temp = (unsigned char *) realloc(buffer, buffer_size);
         if (!temp)
            goto done_err;
         buffer = temp;
         pos += READ_BLOCK_SIZE;
      }
      else {
         fprintf(stderr, "[%s:%d] - Unknown error reading auxv\n",
                 __FILE__, __LINE__);
         goto done_err;
      }
   }

   done_err:
      if (buffer)
         free(buffer);
      buffer = NULL;
   done:
      if (fd != -1)
         close(fd);
      return buffer;
}


map_entries *getVMMaps(int pid, unsigned &maps_size) {
   std::ostringstream maps_filename;
   maps_filename << "/proc/" << pid << "/maps";
   std::ifstream maps_file(maps_filename.str());

   std::vector<map_entries> maps;
   while (maps_file.good()) {
      char delim;
      std::string prems;
      map_entries map;
      memset(&map, 0, sizeof(map));

      maps_file >> std::hex >> map.start >> delim >> map.end >> prems >> map.offset
         >> map.dev_major >> delim >> map.dev_minor >> std::dec >> map.inode;

      for (auto i = prems.begin(); i != prems.end(); ++i)
         switch (*i) {
            case 'r':
               map.prems |= PREMS_READ;
               break;
            case 'w':
               map.prems |= PREMS_WRITE;
               break;
            case 'x':
               map.prems |= PREMS_EXEC;
               break;
            case 'p':
               map.prems |= PREMS_PRIVATE;
               break;
            case 's':
               map.prems |= PREMS_EXEC;
               break;
         }

      std::string path;
      std::getline(maps_file, path);
      path.erase(0, path.find_first_not_of(" \t"));
      strncpy(map.path, path.c_str(), sizeof(map.path) - 1);

      if (maps_file.good())
         maps.push_back(map);
   }

   if (maps.empty()) {
      maps_size = 0;
      return NULL;
   }

   map_entries *cmaps = (map_entries *)calloc(maps.size() + 1, sizeof(map_entries));
   if (cmaps != NULL) {
      std::copy(maps.begin(), maps.end(), cmaps);
      maps_size = maps.size();
   }
   return cmaps;
}

bool findProcLWPs(pid_t pid, std::vector<pid_t> &lwps)
{
   char name[32];
   struct dirent *direntry;

   /**
    * Linux 2.6:
    **/
   snprintf(name, 32, "/proc/%d/task", pid);
   DIR *dirhandle = opendir(name);
   if (dirhandle)
   {
      //Only works on Linux 2.6
      while((direntry = readdir(dirhandle)) != NULL) {
         unsigned lwp_id = atoi(direntry->d_name);
         if (lwp_id)
            lwps.push_back(lwp_id);
      }
      closedir(dirhandle);
      return true;
   }

  return false;
}
