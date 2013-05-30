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

#if !defined(SW_C_H_)
#define SW_C_H_

typedef struct {
   void *walker;
} walker_t;

typedef struct {
   void *frame;
} frame_t;
   
walker_t newWalkerLocal();
walker_t newWalkerAttach(int pid);
walker_t newWalkerCreate(char *executable, char **args);

int walkStack(walker_t walker, frame_t **out_frames, unsigned int *out_frames_size);
int walkStackFromFrame(walker_t walker, frame_t frame, frame_t **out_frames, 
                       unsigned int *out_frames_size);
int walkSingleFrame(walker_t walker, frame_t frame, frame_t *out_frame);
int getInitialFrame(walker_t walker, frame_t *out_frame);

unsigned long frameGetReturnAddress(frame_t frame);
unsigned long frameGetFramePointer(frame_t frame);
unsigned long frameGetStackPointer(frame_t frame);
int frameGetName(frame_t frame, char **buffer);
int frameGetLibOffset(frame_t frame, char **libname, unsigned long *offset);
int frameIsTopFrame(frame_t frame);
int frameIsBottomFrame(frame_t frame);

int freeStackwalk(frame_t *frames, unsigned int frames_size);
int freeFrame(frame_t *frame);
int freeName(char *name);

#endif
