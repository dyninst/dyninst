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

extern "C" {
#include "stackwalk/h/sw_c.h"
}

#include "stackwalk/h/walker.h"
#include "stackwalk/h/frame.h"

using namespace Dyninst;
using namespace Stackwalker;
using namespace std;

walker_t newWalkerLocal()
{
   walker_t ret;
   ret.walker = (void *) Walker::newWalker();
   return ret;
}

walker_t newWalkerAttach(int pid)
{
   walker_t ret;
   ret.walker = (void *) Walker::newWalker(pid);
   return ret;
}

walker_t newWalkerCreate(char *executable, char **args)
{
   vector<string> cpp_args;
   while (*args) {
      cpp_args.push_back(*args);
      args++;
   }
   string cpp_executable(executable);

   walker_t ret;
   ret.walker = (void *) Walker::newWalker(cpp_executable, cpp_args);
   return ret;
}

int walkStack(walker_t walker, frame_t **out_frames, unsigned int *out_frames_size)
{
   Walker *w = (Walker *) walker.walker;
   
   vector<Frame> frames;
   bool result = w->walkStack(frames);
   if (!frames.size())
      return -1;
   
   *out_frames = (frame_t *) malloc(sizeof(frame_t) * frames.size());
   if (!*out_frames)
      return -1;
   
   unsigned j=0;
   for (vector<Frame>::iterator i = frames.begin(); i != frames.end(); i++, j++) {
      (*out_frames)[j].frame = (void *) new Frame(*i);
   }
   *out_frames_size = frames.size();

   return result ? 0 : -1;
}

int walkStackFromFrame(walker_t walker, frame_t frame, frame_t **out_frames,
                       unsigned int *out_frames_size)
{
   Walker *w = (Walker *) walker.walker;
   Frame *f = (Frame *) frame.frame;
   
   vector<Frame> frames;
   bool result = w->walkStackFromFrame(frames, *f);
   if (!frames.size())
      return -1;
   
   *out_frames = (frame_t *) malloc(sizeof(frame_t) * frames.size());
   if (!*out_frames)
      return -1;
   
   unsigned j=0;
   for (vector<Frame>::iterator i = frames.begin(); i != frames.end(); i++, j++) {
      (*out_frames)[j].frame = (void *) new Frame(*i);
   }
   *out_frames_size = frames.size();

   return result ? 0 : -1;
}


int walkSingleFrame(walker_t walker, frame_t frame, frame_t *out_frame)
{
   Walker *w = (Walker *) walker.walker;
   Frame *f = (Frame *) frame.frame;

   Frame oframe(w);
   bool result = w->walkSingleFrame(*f, oframe);
   if (!result) 
      return -1;

   out_frame->frame = (void *) new Frame(oframe);
   return 0;
}

int getInitialFrame(walker_t walker, frame_t *out_frame)
{   
   Walker *w = (Walker *) walker.walker;

   Frame oframe(w);
   bool result = w->getInitialFrame(oframe);
   if (!result) 
      return -1;
   
   out_frame->frame = (void *) new Frame(oframe);
   return 0;
}

unsigned long frameGetReturnAddress(frame_t frame)
{
   Frame *f = (Frame *) frame.frame;
   return f->getRA();
}

unsigned long frameGetFramePointer(frame_t frame)
{
   Frame *f = (Frame *) frame.frame;
   return f->getFP();
}

unsigned long frameGetStackPointer(frame_t frame)
{
   Frame *f = (Frame *) frame.frame;
   return f->getSP();
}

int frameGetName(frame_t frame, char **buffer)
{
   Frame *f = (Frame *) frame.frame;
   std::string name;
   bool result = f->getName(name);
   if (!result)
      return -1;

   unsigned int name_length = (unsigned int) name.length();
   *buffer = (char *) malloc(name_length+1);
   strncpy(*buffer, name.c_str(), name_length);
   (*buffer)[name_length] = '\0';

   return 0;
}

int frameGetLibOffset(frame_t frame, char **libname, unsigned long *offset)
{
   Frame *f = (Frame *) frame.frame;

   Dyninst::Offset dyn_offset;
   std::string name;
   void *extra_handle = NULL;
   bool result = f->getLibOffset(name, dyn_offset, extra_handle);
   if (!result)
      return -1;

   if (*libname) {
      unsigned int name_length = (unsigned int) name.length();
      *libname = (char *) malloc(name_length+1);
      strncpy(*libname, name.c_str(), name_length);
      (*libname)[name_length] = '\0';
   }

   if (*offset) {
      *offset = (unsigned long) dyn_offset;
   }

   return 0;
}

int frameIsTopFrame(frame_t frame)
{
   Frame *f = (Frame *) frame.frame;
   return f->isTopFrame() ? 1 : 0;
}

int frameIsBottomFrame(frame_t frame)
{
   Frame *f = (Frame *) frame.frame;
   return f->isBottomFrame() ? 1 : 0;
}

int freeStackwalk(frame_t *frames, unsigned int frames_size)
{
   for (unsigned i=0; i<frames_size; i++) {
      freeFrame(frames+i);
   }
   free(frames);
   return 0;
}

int freeFrame(frame_t *frame)
{
   Frame *f = (Frame *) frame->frame;
   if (!f)
      return -1;
   delete f;
   frame->frame = NULL;
   return 0;
}

int freeName(char *name)
{
   free(name);
   return 0;
}
