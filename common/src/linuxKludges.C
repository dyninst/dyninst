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

#include "common/h/headers.h"
#include "common/h/parseauxv.h"

#include <elf.h>

#include <vector>

typedef int (*intKludge)();

int P_getopt(int argc, char *argv[], const char *optstring)
{
  /* On linux we prepend a + character */
  char newopt[strlen(optstring)+5];
  strcpy(newopt, "+");
  strcat(newopt, optstring);
  return getopt(argc, argv, newopt);
}

int P_copy(const char *from, const char *to) {
    int from_fd = P_open(from, O_RDONLY, 0);
    if (from_fd == -1)  {
        perror("Opening from file in copy"); 
        return -1;
    }
    int to_fd = P_open(to, O_WRONLY | O_APPEND | O_CREAT | O_TRUNC, 0);
    if (to_fd == -1) {
        perror("Opening to file in copy");
        close(from_fd);
        return -1;
    }

    char buffer[1048576];
    while(true) {
        int amount = read(from_fd, buffer, 1048576);
        if (amount == -1) {
            perror("Reading in file copy");
            return -1;
        }
        write(to_fd, buffer, amount);
        if (amount < 1048576) break;
    }
    close(to_fd);
    close(from_fd);
    return 0;
}


unsigned long long PDYN_div1000(unsigned long long in) {
   /* Divides by 1000 without an integer division instruction or library call, both of
    * which are slow.
    * We do only shifts, adds, and subtracts.
    *
    * We divide by 1000 in this way:
    * multiply by 1/1000, or multiply by (1/1000)*2^30 and then right-shift by 30.
    * So what is 1/1000 * 2^30?
    * It is 1,073,742.   (actually this is rounded)
    * So we can multiply by 1,073,742 and then right-shift by 30 (neat, eh?)
    *
    * Now for multiplying by 1,073,742...
    * 1,073,742 = (1,048,576 + 16384 + 8192 + 512 + 64 + 8 + 4 + 2)
    * or, slightly optimized:
    * = (1,048,576 + 16384 + 8192 + 512 + 64 + 16 - 2)
    * for a total of 8 shifts and 6 add/subs, or 14 operations.
    *
    */

   unsigned long long temp = in << 20; // multiply by 1,048,576
      // beware of overflow; left shift by 20 is quite a lot.
      // If you know that the input fits in 32 bits (4 billion) then
      // no problem.  But if it's much bigger then start worrying...

   temp += in << 14; // 16384
   temp += in << 13; // 8192
   temp += in << 9;  // 512
   temp += in << 6;  // 64
   temp += in << 4;  // 16
   temp -= in >> 2;  // 2

   return (temp >> 30); // divide by 2^30
}

unsigned long long PDYN_divMillion(unsigned long long in) {
   /* Divides by 1,000,000 without an integer division instruction or library call,
    * both of which are slow.
    * We do only shifts, adds, and subtracts.
    *
    * We divide by 1,000,000 in this way:
    * multiply by 1/1,000,000, or multiply by (1/1,000,000)*2^30 and then right-shift
    * by 30.  So what is 1/1,000,000 * 2^30?
    * It is 1,074.   (actually this is rounded)
    * So we can multiply by 1,074 and then right-shift by 30 (neat, eh?)
    *
    * Now for multiplying by 1,074
    * 1,074 = (1024 + 32 + 16 + 2)
    * for a total of 4 shifts and 4 add/subs, or 8 operations.
    *
    * Note: compare with div1000 -- it's cheaper to divide by a million than
    *       by a thousand (!)
    *
    */

   unsigned long long temp = in << 10; // multiply by 1024
      // beware of overflow...if the input arg uses more than 52 bits
      // than start worrying about whether (in << 10) plus the smaller additions
      // we're gonna do next will fit in 64...

   temp += in << 5; // 32
   temp += in << 4; // 16
   temp += in << 1; // 2

   return (temp >> 30); // divide by 2^30
}

unsigned long long PDYN_mulMillion(unsigned long long in) {
   unsigned long long result = in;

   /* multiply by 125 by multiplying by 128 and subtracting 3x */
   result = (result << 7) - result - result - result;

   /* multiply by 125 again, for a total of 15625x */
   result = (result << 7) - result - result - result;

   /* multiply by 64, for a total of 1,000,000x */
   result <<= 6;

   /* cost was: 3 shifts and 6 subtracts
    * cost of calling mul1000(mul1000()) would be: 6 shifts and 4 subtracts
    *
    * Another algorithm is to multiply by 2^6 and then 5^6.
    * The former is super-cheap (one shift); the latter is more expensive.
    * 5^6 = 15625 = 16384 - 512 - 256 + 8 + 1
    * so multiplying by 5^6 means 4 shift operations and 4 add/sub ops
    * so multiplying by 1000000 means 5 shift operations and 4 add/sub ops.
    * That may or may not be cheaper than what we're doing (3 shifts; 6 subtracts);
    * I'm not sure.  --ari
    */

   return result;
}

bool PtraceBulkRead(Address inTraced, unsigned size, const void *inSelf, int pid)
{
   const unsigned char *ap = (const unsigned char*) inTraced; 
   unsigned char *dp = (unsigned char*) inSelf;
   Address w = 0x0;               /* ptrace I/O buffer */
   int len = sizeof(void *);
   unsigned cnt;
   
   if (0 == size) {
      return true;
   }
   
   if ((cnt = ((Address)ap) % len)) {
      /* Start of request is not aligned. */
      unsigned char *p = (unsigned char*) &w;
      
      /* Read the segment containing the unaligned portion, and
         copy what was requested to DP. */
      errno = 0;
      w = P_ptrace(PTRACE_PEEKTEXT, pid, (Address) (ap-cnt), w, len);
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
      w = P_ptrace(PTRACE_PEEKTEXT, pid, (Address) ap, 0, len);
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
      w = P_ptrace(PTRACE_PEEKTEXT, pid, (Address) ap, 0, len);
      if (errno) {
         return false;
      }
      for (unsigned i = 0; i < size; i++)
         dp[i] = p[i];
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

static bool couldBeVsyscallPage(map_entries *entry, bool strict, Address pagesize) {
   assert(pagesize != 0);
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
  char *buffer = NULL;
  unsigned pos = 0;
  Address dso_start = 0x0, text_start = 0x0;
  unsigned page_size = 0x0;

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
  buffer = (char *) readAuxvFromProc();
  if (!buffer) 
     buffer = (char *) readAuxvFromStack();
  if (!buffer)
     return false;
  do {
     /**Fill in the auxv_entry structure.  We may have to do different
      * size reads depending on the address space.  No matter which
      * size we read, we'll fill the data in to auxv_entry, which may
      * involve a size shift up.
      **/
     if (addr_size == 4) {
        auxv_entry.type = (unsigned long) *(uint32_t *) (buffer + pos);
        pos += sizeof(uint32_t);
        auxv_entry.value = (unsigned long) *(uint32_t *) (buffer + pos);
        pos += sizeof(uint32_t);
     }
     else {
        auxv_entry.type = *(unsigned long *) (buffer + pos);
        pos += sizeof(long);
        auxv_entry.value = *(unsigned long *) (buffer + pos);
        pos += sizeof(long);
     }
     
     if (auxv_entry.type == AT_SYSINFO)
        text_start = auxv_entry.value;
     else if (auxv_entry.type == AT_SYSINFO_EHDR)
        dso_start = auxv_entry.value;
     else if (auxv_entry.type == AT_PAGESZ)
        page_size = auxv_entry.value;
     else if (auxv_entry.type == AT_BASE)
        interpreter_base = auxv_entry.value;
  } while (auxv_entry.type != AT_NULL);

  if (buffer)
     free(buffer);
  if (!page_size)
     page_size = getpagesize();
  /**
   * Even if we found dso_start in /proc/pid/auxv, the vsyscall 'page'
   * can be larger than a single page.  Thus we look through /proc/pid/maps
   * for known, default, or guessed start address(es).
   **/
  std::vector<Address> guessed_addrs;
  
  /* The first thing to check is the auxvinfo, if we have any. */
  if( dso_start != 0x0 ) 
     guessed_addrs.push_back( dso_start );
    
  /**
   * We'll make several educated attempts at guessing an address
   * for the vsyscall page.  After deciding on a guess, we'll try to
   * verify that using /proc/pid/maps.
   **/
  
  // Guess some constants that we've seen before.
#if defined(arch_x86) 
  guessed_addrs.push_back(0xffffe000); //Many early 2.6 systems
  guessed_addrs.push_back(0xffffd000); //RHEL4
#elif defined(arch_ia64)
  guessed_addrs.push_back(0xa000000000000000); 
  guessed_addrs.push_back(0xa000000000010000); 
  guessed_addrs.push_back(0xa000000000020000); //Juniper & Hogan
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
  map_entries *maps = getLinuxMaps(pid, num_maps);
  for (unsigned i=0; i<guessed_addrs.size(); i++) {
     Address addr = guessed_addrs[i];
     for (unsigned j=0; j<num_maps; j++) {
        map_entries *entry = &(maps[j]);
        if (addr < entry->start || addr >= entry->end)
           continue;

        if (couldBeVsyscallPage(entry, true, page_size)) {
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
static Address getStackTop(AddrSpaceReader *proc, bool &err) {
   Address stack_pointer;
   Address pagesize = getpagesize();
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
         fprintf(stderr, "[%s:%u] - Unknown error reading auxv\n",
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


#define LINE_LEN 1024
map_entries *getLinuxMaps(int pid, unsigned &maps_size) {
   char line[LINE_LEN], prems[16], *s;
   int result;
   FILE *f;
   map_entries *maps;
   unsigned i, no_lines = 0;
   
  
   sprintf(line, "/proc/%d/maps", pid);
   f = fopen(line, "r");
   if (!f)
      return NULL;
   
   //Calc num of entries needed and allocate the buffer.  Assume the 
   //process is stopped.
   while (!feof(f)) {
      fgets(line, LINE_LEN, f);
      no_lines++;
   }
   maps = (map_entries *) malloc(sizeof(map_entries) * (no_lines+1));
   if (!maps)
      return NULL;
   result = fseek(f, 0, SEEK_SET);
   if (result == -1)
      return NULL;

   //Read all of the maps entries
   for (i = 0; i < no_lines; i++) {
      if (!fgets(line, LINE_LEN, f))
         break;
      line[LINE_LEN - 1] = '\0';
      maps[i].path[0] = '\0';
      sscanf(line, "%lx-%lx %16s %lx %x:%x %u %512s\n", 
             (Address *) &maps[i].start, (Address *) &maps[i].end, prems, 
             (Address *) &maps[i].offset, &maps[i].dev_major,
             &maps[i].dev_minor, &maps[i].inode, maps[i].path);
      maps[i].prems = 0;
      for (s = prems; *s != '\0'; s++) {
         switch (*s) {
            case 'r':
               maps[i].prems |= PREMS_READ;
               break;
            case 'w':
               maps[i].prems |= PREMS_WRITE;
               break;
            case 'x':
               maps[i].prems |= PREMS_EXEC;
               break;
            case 'p':
               maps[i].prems |= PREMS_PRIVATE;
               break;
            case 's':
               maps[i].prems |= PREMS_EXEC;
               break;
         }
      }
   }
   //Zero out the last entry
   memset(&(maps[i]), 0, sizeof(map_entries));
   maps_size = i;
   
   return maps;
}

