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

#include "walker.h"
#include "frame.h"
#include "swk_errors.h"
#include "PCProcess.h"

void parse_args(int argc, char** argv, Dyninst::PID& pid, int& num_samples, int& sample_interval, std::string& exe)
{
  for(int i = 1; i < argc; ++i)
  {
    if(strcmp(argv[i], "--pid") == 0)
    {
      i++;
      pid = atoi(argv[i]);
      i++;
    }
    else if(strcmp(argv[i], "--samples") == 0)
    {
      i++;
      num_samples = atoi(argv[i]);
      i++;
    }
    else if(strcmp(argv[i], "--interval") == 0)
    {
      i++;
      sample_interval = atoi(argv[i]);
      i++;
    }
  }
  exe = argv[argc-1];
}


int main(int argc, char** argv)
{
  using namespace Dyninst;
  using namespace Stackwalker;
  using namespace ProcControlAPI;

  PID pid = 0;
  int num_samples = 10;
  int sample_interval = 100;

  std::string exe;

  parse_args(argc, argv, pid, num_samples, sample_interval, exe);

  Process::ptr p;
  if(pid != 0)
  {
    p = Process::attachProcess(pid, exe);
  }
  else
  {
    fprintf(stderr, "exe: %s\n", exe.c_str());
    std::vector<std::string> argv;
    p = Process::createProcess(exe, argv);
  }

  Address breakpoint_addr = 0x804843e;
  Breakpoint::ptr b = Breakpoint::newBreakpoint();
  fprintf(stderr, "before add bp...\n");
  p->addBreakpoint(breakpoint_addr, b);
  fprintf(stderr, "after add bp...\n");


  Walker* w = Walker::newWalker(p);

  std::vector<THR_ID> threads;
  std::vector<Frame> stack;
  int num_good, num_attempted = 0;

  //for(int i = 0; i < num_samples; i++)
  {
    p->continueProc();
    while(!p->allThreadsStopped() && Process::handleEvents(true))
    {
      // handle events until stopped by breakpoint
    }

    threads.clear();
    if(!w->getAvailableThreads(threads))
    {
      fprintf(stderr, "Failed to get threads on process %d (%s)\n", pid, exe.c_str());
      return -1;
    }
    // collect traces
    for(std::vector<THR_ID>::const_iterator t = threads.begin();
	t != threads.end();
	++t)
    {
      stack.clear();
      num_attempted++;

      if(!w->walkStack(stack, *t))
      {
	fprintf(stderr, "Failed to get stack on process %d/%d (%s)\n", pid, *t, exe.c_str());
	fprintf(stderr, "Error was: %s\n", Stackwalker::getLastErrorMsg());
	continue;
      }
      num_good++;

      for(std::vector<Frame>::const_iterator f = stack.begin();
	  f != stack.end();
	  ++f)
      {
	std::string name;
	f->getName(name);
        printf("%s\n", name.c_str());
      }
    }
  }
  printf("%d good stackwalks of %d attempts\n", num_good, num_attempted);

  return 0;
}

