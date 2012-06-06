
#include "walker.h"
#include "frame.h"
#include "swk_errors.h"
#include "procstate.h"

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
  
  PID pid;
  int num_samples = 10;
  int sample_interval = 100;
  
  std::string exe;

  parse_args(argc, argv, pid, num_samples, sample_interval, exe);
  
  ProcDebug* p = ProcDebug::newProcDebug(pid, exe);
  Walker* w = Walker::newWalker(p);
  
  std::vector<THR_ID> threads;
  std::vector<Frame> stack;
  int num_good, num_attempted = 0;
  
  for(int i = 0; i < num_samples; i++)
  {
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
      p->pause(*t);
      
      if(!w->walkStack(stack, *t))
      {
	fprintf(stderr, "Failed to get stack on process %d/%d (%s)\n", pid, *t, exe.c_str());
	fprintf(stderr, "Error was: %s\n", getLastErrorMsg());
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
      p->resume(*t);
    }
    // continue proc
    usleep(sample_interval);
  }
  printf("%d good stackwalks of %d attempts\n", num_good, num_attempted);
  
  return 0;
}

