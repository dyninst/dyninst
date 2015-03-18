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

#include <common/src/headers.h>

// taken from LinuxKludges.C.
// TODO: put this in some common place?  It's used by at least 2 platforms.
char * P_cplus_demangle( const char * symbol, bool nativeCompiler, bool includeTypes ) 
{
  int opts = 0;
  opts |= includeTypes ? DMGL_PARAMS | DMGL_ANSI : 0;
  opts |= nativeCompiler ? DMGL_ARM : 0;
  
  char * demangled = cplus_demangle( const_cast< char *>(symbol), opts);
  if( demangled == NULL ) { return NULL; }
  
  if( ! includeTypes ) {
    /* de-demangling never increases the length */   
    char * dedemangled = strdup( demangled );   
    assert( dedemangled != NULL );
    dedemangle( demangled, dedemangled );
    assert( dedemangled != NULL );
    
    free( demangled );
    return dedemangled;
  }
  
  return demangled;
} /* end P_cplus_demangle() */


#if !defined(os_bg_compute)

#define getVMMaps getLinuxMaps
COMMON_EXPORT map_entries *getLinuxMaps(int pid, unsigned &maps_size);

#include "parseauxv.h"
#include "auxvtypes.h"

bool AuxvParser::readAuxvInfo() {
  assert(0); //
}

#define LINE_LEN 1024
map_entries *getLinuxMaps(int pid, unsigned &maps_size) {
   char line[LINE_LEN], prems[16], *s;
   int result;
   int fd = -1;
   map_entries *maps = NULL;
   unsigned i, no_lines = 0, cur_pos = 0, cur_size = 4096;
   unsigned file_size = 0;
   char *buffer = NULL;
  
   sprintf(line, "/proc/%d/maps", pid);
   fd = open(line, O_RDONLY);
   if (fd == -1)
      goto done_err;
   
   cur_pos = 0;
   buffer = (char *) malloc(cur_size);
   if (!buffer) {
      goto done_err;
   }
   for (;;) {
      result = read(fd, buffer+cur_pos, cur_size - cur_pos);
      if (result == -1) {
         goto done_err;
      }
      cur_pos += result;
      if (result == 0) {
         break;
      }
      assert(cur_pos <= cur_size);
      if (cur_size == cur_pos) {
         cur_size *= 2;
         buffer = (char *) realloc(buffer, cur_size);
         if (!buffer) {
            goto done_err;
         }
      }
   }
   file_size = cur_pos;

   close(fd);
   fd = -1;
   //Calc num of entries needed and allocate the buffer.  Assume the 
   //process is stopped.
   no_lines = file_size ? 1 : 0;
   for (i = 0; i < file_size; i++) {
      if (buffer[i] == '\n')
         no_lines++;
   } 

   maps = (map_entries *) malloc(sizeof(map_entries) * (no_lines+1));
   memset(maps, 0, sizeof(map_entries) * (no_lines+1));
   if (!maps)
      goto done_err;

   //Read all of the maps entries
   cur_pos = 0;
   for (i = 0; i < no_lines; i++) {
      if (cur_pos >= file_size)
         break;
      unsigned next_end = cur_pos;
      while (buffer[next_end] != '\n' && next_end < file_size) next_end++;
      unsigned int line_size = (next_end - cur_pos) > LINE_LEN ? LINE_LEN : (next_end - cur_pos);
      memcpy(line, buffer+cur_pos, line_size);
      line[line_size] = '\0';
      line[LINE_LEN - 1] = '\0';
      cur_pos = next_end+1;

      sscanf(line, "%lx-%lx %16s %lx %x:%x %lu %" MAPENTRIES_PATH_SIZE_STR "s\n", 
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

   free(buffer);
   return maps;

 done_err:
   if (fd != -1)
      close(fd);
   if (buffer)
      free(buffer);
   return NULL;
}


#endif
