// Rewrite a program with a call to another object, a shared library, placed
// more than 2 GiB from the jump slot it calls through, and run it.
//
// The static rewriter puts a call's jump slot and the instrumentation that
// calls through it in the same new region.  The test allocates 64 bytes in that
// region, then a gap of more than 2 GiB, and frees the 64 bytes.  The jump slot
// for the call then lands in the freed hole, and the instrumentation after the
// gap.  On x86_64, EmitterAMD64Stat::emitPLTCall reaches the slot with a 32-bit
// displacement and asserts when it is out of reach (issue #2613).
//
// The snippet calls the library's far_plt_call_exit(kExitCode) at the entry of
// the mutatee's work().  The unrewritten mutatee exits with 1, so exit status
// kExitCode shows the call through the far jump slot ran.
//
// Usage: far_plt_call <mutatee> <output directory>
// DYNINSTAPI_RT_LIB names the runtime library; the rewritten mutatee is run
// with the runtime library's directory in LD_LIBRARY_PATH.

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "BPatch.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_function.h"
#include "BPatch_image.h"
#include "BPatch_module.h"
#include "BPatch_object.h"
#include "BPatch_point.h"
#include "BPatch_snippet.h"

namespace {

// Must match EXIT_CODE in mutatee.c.
int const kExitCode{42};

// Just over 2 GiB, so a 32-bit displacement can't reach across it.
unsigned long const kGapMiB{2100};

std::string basename(std::string const &path)
{
  auto const slash = path.rfind('/');
  return slash == std::string::npos ? path : path.substr(slash + 1);
}

std::string dirname(std::string const &path)
{
  auto const slash = path.rfind('/');
  return slash == std::string::npos ? "." : path.substr(0, slash);
}

BPatch_function *find_function(BPatch_image *image, char const *name)
{
  std::vector<BPatch_function *> funcs;
  image->findFunction(name, funcs);
  if (funcs.empty())  {
    std::cerr << "No function " << name << '\n';
    return nullptr;
  }
  return funcs[0];
}

// Allocate a hole, then a gap of kGapMiB, and free the hole, so the next
// small allocation, the jump slot, lands before the gap.
bool make_gap(BPatch_binaryEdit *edit)
{
  BPatch_variableExpr *hole{edit->malloc(64)};
  if (!hole)  {
    std::cerr << "Cannot allocate the hole\n";
    return false;
  }
  // BPatch_addressSpace::malloc takes an int: allocate the gap in pieces.
  for (unsigned long left{kGapMiB}; left > 0;)  {
    unsigned long const piece{left < 1024 ? left : 1024};
    if (!edit->malloc(static_cast<int>(piece << 20)))  {
      std::cerr << "Cannot allocate " << piece << " MiB of the gap\n";
      return false;
    }
    left -= piece;
  }
  return edit->free(*hole);
}

// Run 'exe' with 'libpath' prepended to LD_LIBRARY_PATH; its exit status, or
// -1 if it did not exit.
int run(std::string const &exe, std::string const &libpath)
{
  std::string ldpath{libpath};
  char const *old{std::getenv("LD_LIBRARY_PATH")};
  if (old && *old)  {
    ldpath += ":" + std::string(old);
  }
  pid_t const pid{fork()};
  if (pid < 0)  {
    std::cerr << "fork: " << std::strerror(errno) << '\n';
    return -1;
  }
  if (pid == 0)  {
    setenv("LD_LIBRARY_PATH", ldpath.c_str(), 1);
    execl(exe.c_str(), exe.c_str(), static_cast<char *>(nullptr));
    std::fprintf(stderr, "execl %s: %s\n", exe.c_str(), std::strerror(errno));
    _exit(127);
  }
  int status{0};
  if (waitpid(pid, &status, 0) != pid)  {
    std::cerr << "waitpid: " << std::strerror(errno) << '\n';
    return -1;
  }
  if (WIFSIGNALED(status))  {
    std::cerr << "rewritten mutatee killed by signal " << WTERMSIG(status) << '\n';
    return -1;
  }
  return WEXITSTATUS(status);
}

}

int main(int argc, char **argv)
{
  if (argc != 3)  {
    std::cerr << "Usage: " << argv[0] << " <mutatee> <output directory>\n";
    return EXIT_FAILURE;
  }
  std::string const mutatee{argv[1]};
  std::string const outdir{argv[2]};

  char const *rt{std::getenv("DYNINSTAPI_RT_LIB")};
  if (!rt || !*rt)  {
    std::cerr << "DYNINSTAPI_RT_LIB is not set\n";
    return EXIT_FAILURE;
  }

  BPatch bpatch;
  // Open the library too, so the call is to another object.
  BPatch_binaryEdit *edit{bpatch.openBinary(mutatee.c_str(), true)};
  if (!edit)  {
    std::cerr << "Cannot open " << mutatee << '\n';
    return EXIT_FAILURE;
  }
  BPatch_image *image{edit->getImage()};

  BPatch_function *work{find_function(image, "work")};
  BPatch_function *exit_func{find_function(image, "far_plt_call_exit")};
  if (!work || !exit_func)  {
    return EXIT_FAILURE;
  }
  if (work->getModule()->getObject() == exit_func->getModule()->getObject())  {
    std::cerr << "far_plt_call_exit is in the mutatee, not in another object\n";
    return EXIT_FAILURE;
  }

  if (!make_gap(edit))  {
    return EXIT_FAILURE;
  }

  std::vector<BPatch_point *> *entry{work->findPoint(BPatch_entry)};
  if (!entry || entry->empty())  {
    std::cerr << "No entry point in work\n";
    return EXIT_FAILURE;
  }
  std::vector<BPatch_snippet *> args{new BPatch_constExpr(kExitCode)};
  BPatch_funcCallExpr call{*exit_func, args};
  if (!edit->insertSnippet(call, *entry))  {
    std::cerr << "Cannot insert the snippet at work's entry\n";
    return EXIT_FAILURE;
  }

  if (mkdir(outdir.c_str(), 0755) != 0 && errno != EEXIST)  {
    std::cerr << "mkdir " << outdir << ": " << std::strerror(errno) << '\n';
    return EXIT_FAILURE;
  }
  std::string const rewritten{outdir + "/" + basename(mutatee)};
  if (!edit->writeFile(rewritten.c_str()))  {
    std::cerr << "Cannot write " << rewritten << '\n';
    return EXIT_FAILURE;
  }

  int const status{run(rewritten, dirname(rt))};
  // The rewritten file is over 2 GB: don't leave it behind.
  std::remove(rewritten.c_str());
  if (status != kExitCode)  {
    std::cerr << "rewritten mutatee exited with " << status << ", expected "
              << kExitCode << '\n';
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
