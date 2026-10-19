// AddressLookup (AddressTranslate underneath) on a child in its own mount
// namespace names each library /proc/<pid>/root/<path>, the same file the
// loader mapped, and maps an address in a library to that name.
// AddrLookup.h uses these without including them.
#include "Symtab.h"
#include "Symbol.h"
#include "ProcReader.h"
#include "AddrLookup.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#ifdef __linux__
#include <sched.h>
#include <signal.h>
#include <sys/prctl.h>
#include <sys/ptrace.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

#ifdef __linux__
namespace st = Dyninst::SymtabAPI;

// A paused child, optionally in its own user and mount namespace, or -1.
static pid_t spawn(bool own_namespace) {
  int ready[2];
  if (pipe(ready) != 0)  {
    return -1;
  }
  pid_t const parent = getpid();
  pid_t const child = fork();
  if (child == 0)  {
    // Never outlive the test, even if it is killed before it reaps us.
    if (prctl(PR_SET_PDEATHSIG, SIGKILL) != 0 || getppid() != parent)  {
      _exit(1);
    }
    close(ready[0]);
    char ok = !own_namespace || unshare(CLONE_NEWUSER | CLONE_NEWNS) == 0 ? 1 : 0;
    if (write(ready[1], &ok, 1) != 1)  {
      _exit(1);
    }
    for (;;)  {
      pause();
    }
  }
  close(ready[1]);
  char ok = 0;
  bool const started = child > 0 && read(ready[0], &ok, 1) == 1 && ok;
  close(ready[0]);
  if (child > 0 && !started)  {
    kill(child, SIGKILL);
    waitpid(child, nullptr, 0);
    return -1;
  }
  return child;
}

// Whether this process may ptrace `pid`, as AddressLookup does.
static bool can_trace(pid_t pid) {
  if (ptrace(PTRACE_ATTACH, pid, nullptr, nullptr) != 0)  {
    return false;
  }
  waitpid(pid, nullptr, 0);
  ptrace(PTRACE_DETACH, pid, nullptr, nullptr);
  return true;
}

static void reap(pid_t child) {
  kill(child, SIGKILL);
  waitpid(child, nullptr, 0);
}

// An address inside libc's code in `pid`, from its maps, or 0.
static Dyninst::Address libc_code_address(pid_t pid) {
  std::ifstream maps("/proc/" + std::to_string(pid) + "/maps");
  for (std::string line; std::getline(maps, line);)  {
    auto const slash = line.rfind('/');
    if (slash == std::string::npos || line.find(" r-xp ") == std::string::npos)  {
      continue;
    }
    auto const file = line.substr(slash + 1);
    if (file.compare(0, 5, "libc.") == 0 || file.compare(0, 5, "libc-") == 0)  {
      return std::stoul(line.substr(0, line.find('-')), nullptr, 16) + 0x10;
    }
  }
  return 0;
}

static bool same_file(std::string const& a, std::string const& b) {
  struct stat sa{};
  struct stat sb{};
  return stat(a.c_str(), &sa) == 0 && stat(b.c_str(), &sb) == 0 && sa.st_dev == sb.st_dev &&
         sa.st_ino == sb.st_ino;
}

struct lookup_result {
  std::vector<st::LoadedLibrary> libs;
  std::string libc_name;
  Dyninst::Offset libc_offset = 0;
};

static bool lookup(pid_t pid, lookup_result& result) {
  auto* al = st::AddressLookup::createAddressLookup(pid);
  if (!al)  {
    return false;
  }
  bool ok = al->refresh() && al->getLoadAddresses(result.libs);
  st::LoadedLibrary lib;
  auto const addr = libc_code_address(pid);
  if (ok && addr && al->getOffset(addr, lib, result.libc_offset))  {
    result.libc_name = lib.name;
  }
  delete al;
  return ok;
}
#endif

int main() {
#ifdef __linux__
  // Fork both children before any lookup: a lookup can load libraries into
  // this process (libdw loads libdebuginfod), and the children must map the
  // same files at the same addresses.
  pid_t const plain = spawn(false);
  if (plain < 0)  {
    std::cerr << "fork failed\n";
    return EXIT_FAILURE;
  }
  pid_t const contained = spawn(true);
  if (!can_trace(plain))  {
    reap(plain);
    if (contained > 0)  {
      reap(contained);
    }
    std::cout << "skipping: cannot trace a child process here\n";
    return EXIT_SUCCESS;
  }
  if (contained < 0)  {
    reap(plain);
    std::cout << "skipping: unshare() is not permitted here\n";
    return EXIT_SUCCESS;
  }
  lookup_result host;
  bool const traced = lookup(plain, host);
  reap(plain);
  lookup_result target;
  bool const looked_up = lookup(contained, target);
  auto const root = "/proc/" + std::to_string(contained) + "/root/";

  bool failed = false;
  auto fail = [&](std::string const& what) {
    std::cerr << what << "\n";
    failed = true;
  };

  std::map<Dyninst::Address, std::string> host_names;
  for (auto const& lib : host.libs)  {
    if (lib.name.compare(0, 6, "/proc/") == 0)  {
      fail("ordinary process: library named '" + lib.name + "'");
    }
    host_names[lib.codeAddr] = lib.name;
  }
  if (!traced)  {
    fail("ordinary process: AddressLookup failed");
  }
  if (host.libc_name.empty())  {
    fail("ordinary process: no library at an address in libc");
  }

  if (!looked_up)  {
    fail("process in a mount namespace: AddressLookup failed");
  }
  if (target.libs.size() != host.libs.size())  {
    fail("process in a mount namespace: " + std::to_string(target.libs.size()) + " libraries, expected " +
         std::to_string(host.libs.size()));
  }
  for (auto const& lib : target.libs)  {
    auto const host_name = host_names.find(lib.codeAddr);
    if (lib.name.compare(0, root.size(), root) != 0)  {
      fail("process in a mount namespace: library named '" + lib.name + "', expected under " + root);
    } else if (host_name == host_names.end() || !same_file(lib.name, host_name->second))  {
      fail("process in a mount namespace: '" + lib.name + "' is not the file the ordinary process maps there");
    }
  }
  if (target.libc_name.compare(0, root.size(), root) != 0 || target.libc_offset != host.libc_offset ||
      !same_file(target.libc_name, host.libc_name))  {
    fail("process in a mount namespace: an address in libc maps to '" + target.libc_name + "' +" +
         std::to_string(target.libc_offset) + ", expected " + root + "... +" + std::to_string(host.libc_offset));
  }
  reap(contained);
  std::cout << "failed = " << std::boolalpha << failed << "\n";
  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
#else
  return EXIT_SUCCESS;
#endif
}
