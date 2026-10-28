#include "common/src/dyninst_filesystem.h"
#include "dyninst_test_skip.h"

#include <array>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <pwd.h>
#include <string>
#include <unistd.h>
#include <vector>

#ifdef os_linux
#include <sched.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>

// Printed when unshare() is refused
static char const userns_hint[] =
    "Unprivileged user namespaces are not available here (for example with\n"
    "user.max_user_namespaces=0, or under Docker's default seccomp profile).\n"
    "Where that is expected, configure with -DDYNINST_ENABLE_USERNS_TESTS=OFF.\n";

// Whether AppArmor restricts unprivileged user namespaces, as Ubuntu 23.10 and later
// do by default: they can be created, but the process has no capabilities in them.
static bool apparmor_restricts_userns() {
  std::ifstream setting("/proc/sys/kernel/apparmor_restrict_unprivileged_userns");
  int value = 0;
  return (setting >> value) && value == 1;
}
#endif

static int test_canonicalize();
static int test_canonicalize_user();
static int test_resolve_in_root_fs();
static int test_canonicalize_procfs();
static int test_canonicalize_procfs_namespace();
static int test_executable_path();
static int test_executable_path_namespace();
static int test_executable_path_chroot();
static int test_exists();
static int test_replace_extension();
static int test_append_filename_suffix();
static int test_strip_all_extensions();

// Each case is its own test: the program runs the one named by its argument. The cases
// that test the library's Linux implementation (os_linux) skip elsewhere, naming what
// they use.
int main(int argc, char** argv) {
  struct test_case {
    char const* name;
    int (*run)();
  };
  std::array<test_case, 12> const cases = {{
      {"canonicalize", test_canonicalize},
      {"canonicalize_user", test_canonicalize_user},
      {"resolve_in_root_fs", test_resolve_in_root_fs},
      {"canonicalize_procfs", test_canonicalize_procfs},
      {"canonicalize_procfs_namespace", test_canonicalize_procfs_namespace},
      {"executable_path", test_executable_path},
      {"executable_path_namespace", test_executable_path_namespace},
      {"executable_path_chroot", test_executable_path_chroot},
      {"exists", test_exists},
      {"replace_extension", test_replace_extension},
      {"append_filename_suffix", test_append_filename_suffix},
      {"strip_all_extensions", test_strip_all_extensions}
  }};

  if(argc == 2) {
    for(auto const& c : cases) {
      if(std::string(argv[1]) == c.name) {
        return c.run();
      }
    }
  }
  std::cerr << "usage: " << argv[0] << " <case>, one of:";
  for(auto const& c : cases) {
    std::cerr << " " << c.name;
  }
  std::cerr << "\n";
  return EXIT_FAILURE;
}

int test_canonicalize_procfs() {
#ifdef os_linux
  namespace bf = boost::filesystem;
  namespace df = Dyninst::filesystem;

  auto const test_path = bf::absolute("procfs-canonicalize-test.out").string();
  {
    std::ofstream fs{test_path};
    if(!fs) {
      std::cerr << "Failed to create '" << test_path << "'\n";
      return EXIT_FAILURE;
    }
  }
  auto const cleanup = [&](int ret) { bf::remove(test_path); return ret; };

  // A process sharing our filesystem view: names come back as canonicalize() gives them.
  auto const self = getpid();
  if(df::has_distinct_filesystem_view(self)) {
    std::cerr << "has_distinct_filesystem_view: true for this process\n";
    return cleanup(EXIT_FAILURE);
  }
  if(df::canonicalize(test_path, self) != df::canonicalize(test_path)) {
    std::cerr << "canonicalize: '" << test_path << "' changed for this process\n";
    return cleanup(EXIT_FAILURE);
  }
  // No such process (above the kernel's pid limit): the view cannot be
  // determined, so the name is opened through its /proc/<pid>/root.
  auto const no_pid = std::numeric_limits<Dyninst::PID>::max();
  if (!df::has_distinct_filesystem_view(no_pid))  {
    std::cerr << "has_distinct_filesystem_view: false for a pid that does not exist\n";
    return cleanup(EXIT_FAILURE);
  }
  auto const unknown = "/proc/" + std::to_string(no_pid) + "/root" + test_path;
  if (df::canonicalize(test_path, no_pid) != unknown)  {
    std::cerr << "canonicalize: expected '" << unknown << "' for a pid that does not exist, got '"
              << df::canonicalize(test_path, no_pid) << "'\n";
    return cleanup(EXIT_FAILURE);
  }
  auto const exe = "/proc/" + std::to_string(self) + "/exe";
  for (auto const& name : {exe, std::string("/proc/self/exe"), std::string("/proc/12a/exe"),
                           std::string("/procfs/1/exe"), std::string("/proc/"), test_path})  {
    if (df::is_rooted_target_path(name))  {
      std::cerr << "is_rooted_target_path: true for '" << name << "'\n";
      return cleanup(EXIT_FAILURE);
    }
  }
  if (!df::is_rooted_target_path("/proc/" + std::to_string(no_pid) + "/root/lib"))  {
    std::cerr << "is_rooted_target_path: false for a pid that does not exist\n";
    return cleanup(EXIT_FAILURE);
  }
  if(df::canonicalize(exe, self) != bf::canonical(exe).string()) {
    std::cerr << "canonicalize: '" << exe << "' did not resolve to the executable\n";
    return cleanup(EXIT_FAILURE);
  }
  return cleanup(EXIT_SUCCESS);
#else
  return skip_without_linux("/proc/<pid>/root, /proc/<pid>/ns/mnt");
#endif
}

// Needs unprivileged user namespaces; with DYNINST_ENABLE_USERNS_TESTS on, not
// having them is a failure.
int test_canonicalize_procfs_namespace() {
#ifdef os_linux
  namespace bf = boost::filesystem;
  namespace df = Dyninst::filesystem;

  // Not the file of canonicalize_procfs, which may run at the same time.
  auto const test_path = bf::absolute("procfs-canonicalize-namespace-test.out").string();
  {
    std::ofstream fs{test_path};
    if(!fs) {
      std::cerr << "Failed to create '" << test_path << "'\n";
      return EXIT_FAILURE;
    }
  }
  auto const cleanup = [&](int ret) { bf::remove(test_path); return ret; };

  // A process with its own mount namespace: names are opened through /proc/<pid>/root.
  int ready[2];
  if(pipe(ready) != 0) {
    perror("pipe");
    return cleanup(EXIT_FAILURE);
  }
  pid_t const child = fork();
  if(child < 0) {
    perror("fork");
    return cleanup(EXIT_FAILURE);
  }
  if(child == 0) {
    close(ready[0]);
    int err = unshare(CLONE_NEWUSER | CLONE_NEWNS) == 0 ? 0 : errno;
    if(write(ready[1], &err, sizeof err) != static_cast<ssize_t>(sizeof err)) {
      perror("write");
      _exit(1);
    }
    pause();
    _exit(0);
  }
  close(ready[1]);  // so read() sees EOF if the child dies first
  int err = -1;
  bool const unshared =
      read(ready[0], &err, sizeof err) == static_cast<ssize_t>(sizeof err) && err == 0;
  int ret = EXIT_SUCCESS;
  auto const proc = "/proc/" + std::to_string(child);
  if(!unshared) {
    std::cerr << "unshare(CLONE_NEWUSER | CLONE_NEWNS): "
              << (err > 0 ? std::strerror(err) : "no answer from the child") << "\n"
              << userns_hint;
    ret = EXIT_FAILURE;
  } else if(access((proc + "/root/").c_str(), R_OK) != 0) {
    std::cerr << "cannot read " << proc << "/root\n";
    ret = EXIT_FAILURE;
  } else if(!df::has_distinct_filesystem_view(child)) {
    std::cerr << "has_distinct_filesystem_view: false for a process in another mount namespace\n";
    ret = EXIT_FAILURE;
  } else {
    auto const rooted = df::canonicalize(test_path, child);
    if(rooted != proc + "/root" + test_path) {
      std::cerr << "canonicalize: expected '" << proc << "/root" << test_path << "', got '" << rooted << "'\n";
      ret = EXIT_FAILURE;
    }
    if(df::canonicalize(proc + "/exe", child) != proc + "/exe") {
      std::cerr << "canonicalize: changed '" << proc << "/exe' for a process in another mount namespace\n";
      ret = EXIT_FAILURE;
    }
    if (!df::is_rooted_target_path(rooted))  {
      std::cerr << "is_rooted_target_path: false for '" << rooted << "'\n";
      ret = EXIT_FAILURE;
    }
  }
  kill(child, SIGKILL);
  waitpid(child, nullptr, 0);
  close(ready[0]);
  return cleanup(ret);
#else
  return skip_without_linux("/proc/<pid>/root, unshare() with mount namespaces");
#endif
}

int test_resolve_in_root_fs() {
  namespace bf = boost::filesystem;
  auto const root = bf::temp_directory_path() / bf::unique_path("dyninst-root-%%%%-%%%%-%%%%");
  auto const library = root / "opt/pkg/lib.so";
  bf::create_directories(library.parent_path());
  bf::create_directories(root / "links");
  { std::ofstream file(library.string()); file << "fixture"; }
  if(!bf::is_regular_file(library)) {
    bf::remove_all(root);
    return EXIT_FAILURE;
  }

  // Model an alternatives chain: /alias -> /links/current -> /opt/pkg.
  bf::create_symlink("/links/current", root / "alias");
  bf::create_symlink("/opt/pkg", root / "links/current");
  bf::create_symlink("../opt/pkg", root / "links/relative");
  bf::create_symlink("loop", root / "loop");
  bf::create_symlink(library, root / "host-only"); // Exists on the host, not in this root.

  // The last allowed link resolves; one additional link exceeds the limit.
  bf::create_symlink("/opt/pkg/lib.so", root / "chain40");
  for(int i = 39; i >= 0; --i)
    bf::create_symlink("chain" + std::to_string(i + 1), root / ("chain" + std::to_string(i)));

  // Each row maps a path inside root to a host path; "" means reject it.
  struct test_case {
    const char *scenario, *target_path;
    std::string expected_host_path;
  } const cases[] = {
      {"absolute chain", "/alias/lib.so", library.string()},
      {"relative directory link", "/links/relative/lib.so", library.string()},
      {"clamp at root", "/../../alias/lib.so", library.string()},
      {"parent after link", "/alias/../pkg/lib.so", library.string()},
      {"40 links", "/chain1", library.string()},
      {"41 links", "/chain0", ""},
      {"loop", "/loop", ""},
      {"host-only target", "/host-only", ""},
      {"file as directory", "/opt/pkg/lib.so/..", ""},
      {"relative path", "opt/pkg/lib.so", ""}
  };
  bool failed = false;
  auto check = [&](const char *scenario, const char *target_path, bf::path const& target_root,
                   std::string const& expected_host_path) {
    auto result = Dyninst::filesystem::detail::resolve_in_root_fs(target_path, target_root.string());
    if(result != expected_host_path) {
      std::cerr << scenario << ": expected '" << expected_host_path << "', got '" << result << "'\n";
      failed = true;
    }
  };
  for(auto const& t : cases)
    check(t.scenario, t.target_path, root, t.expected_host_path);
  check("relative root", "/", "relative-root", "");
  check("file as root", "/", library, "");
  bf::remove_all(root);

  // ".." stops at the root however the root is spelled. Both outer/secret and
  // outer/root/secret exist, so a walk that climbs out returns outer/secret.
  auto const outer = bf::temp_directory_path() / bf::unique_path("dyninst-clamp-%%%%-%%%%-%%%%");
  auto const inner = outer / "root";
  bf::create_directories(inner / "a");
  { std::ofstream file((outer / "secret").string()); file << "outside"; }
  { std::ofstream file((inner / "secret").string()); file << "inside"; }
  bf::create_symlink("/a/..", inner / "lnk");
  auto const inside = (inner / "secret").string();
  for (auto const& spelling : {inner.string(), inner.string() + "/", inner.string() + "//",
                               inner.string() + "/.", (outer / "a/../root").string()})  {
    for (auto const* target_path : {"/../secret", "/a/../../secret", "/a/../a/../../secret",
                                    "/lnk/../secret"})  {
      auto const scenario = "root '" + spelling + "', " + target_path;
      check(scenario.c_str(), target_path, spelling, inside);
    }
  }
  // "/" as the root resolves a host path to itself.
  auto const host_path = bf::canonical(inner / "secret").string();
  for (auto const* spelling : {"/", "/."})  {
    auto const scenario = std::string("root '") + spelling + "'";
    check(scenario.c_str(), host_path.c_str(), spelling, host_path);
  }
  bf::remove_all(outer);
  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}

int test_executable_path() {
#ifdef os_linux
  namespace bf = boost::filesystem;
  namespace df = Dyninst::filesystem;

  // A process sharing our filesystem view: the executable's real path.
  auto const self = getpid();
  auto const self_exe = "/proc/" + std::to_string(self) + "/exe";
  if (df::executable_path(self) != bf::canonical(self_exe).string())  {
    std::cerr << "executable_path: '" << df::executable_path(self) << "' is not this program's path\n";
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
#else
  return skip_without_linux("/proc/<pid>/exe");
#endif
}

// Needs unprivileged user namespaces; with DYNINST_ENABLE_USERNS_TESTS on, not
// having them is a failure.
int test_executable_path_namespace() {
#ifdef os_linux
  namespace df = Dyninst::filesystem;

  // A process with its own mount namespace: a name inside /proc/<pid>/root
  // that reaches the running program, and changes when the process execs.
  int ready[2], go[2];
  if (pipe(ready) != 0)  {
    perror("pipe");
    return EXIT_FAILURE;
  }
  if (pipe(go) != 0)  {
    perror("pipe");
    return EXIT_FAILURE;
  }
  pid_t const child = fork();
  if (child < 0)  {
    perror("fork");
    return EXIT_FAILURE;
  }
  if (child == 0)  {
    close(ready[0]);
    close(go[1]);
    int err = unshare(CLONE_NEWUSER | CLONE_NEWNS) == 0 ? 0 : errno;
    if (write(ready[1], &err, sizeof err) != static_cast<ssize_t>(sizeof err))  {
      perror("write");
      _exit(1);
    }
    char c;
    if (read(go[0], &c, 1) == 1)  {
      execlp("sleep", "sleep", "60", nullptr);
    }
    _exit(1);
  }
  close(ready[1]);
  close(go[0]);
  int err = -1;
  bool const unshared =
      read(ready[0], &err, sizeof err) == static_cast<ssize_t>(sizeof err) && err == 0;
  auto const proc = "/proc/" + std::to_string(child);
  auto const exe = proc + "/exe";
  auto const same_file = [&exe](std::string const& name) {
    struct stat a{}, b{};
    return stat(name.c_str(), &a) == 0 && stat(exe.c_str(), &b) == 0 && a.st_dev == b.st_dev &&
           a.st_ino == b.st_ino;
  };
  auto const named_in_root = [&](char const* when, std::string const& name) {
    if (name.compare(0, proc.size() + 6, proc + "/root/") != 0 || !same_file(name))  {
      std::cerr << "executable_path " << when << " exec: '" << name << "' is not the program inside "
                << proc << "/root\n";
      return false;
    }
    return true;
  };
  int ret = EXIT_SUCCESS;
  if (!unshared)  {
    std::cerr << "unshare(CLONE_NEWUSER | CLONE_NEWNS): "
              << (err > 0 ? std::strerror(err) : "no answer from the child") << "\n"
              << userns_hint;
    ret = EXIT_FAILURE;
  } else if (access((proc + "/root/").c_str(), R_OK) != 0)  {
    std::cerr << "cannot read " << proc << "/root\n";
    ret = EXIT_FAILURE;
  } else {
    struct stat before{};
    stat(exe.c_str(), &before);
    auto const first = df::executable_path(child);
    if (!named_in_root("before", first))  {
      ret = EXIT_FAILURE;
    }
    if (write(go[1], "x", 1) != 1)  {
      ret = EXIT_FAILURE;
    }
    // Wait for the exec to replace the program.
    struct stat after{};
    for (int i = 0; i < 500; ++i)  {
      if (stat(exe.c_str(), &after) == 0 && after.st_ino != before.st_ino)  {
        break;
      }
      usleep(10000);
    }
    if (after.st_ino == before.st_ino)  {
      std::cerr << "executable_path: the child did not exec sleep\n";
      ret = EXIT_FAILURE;
    } else {
      auto const second = df::executable_path(child);
      if (!named_in_root("after", second))  {
        ret = EXIT_FAILURE;
      }
      if (second == first)  {
        std::cerr << "executable_path: '" << first << "' did not change across exec\n";
        ret = EXIT_FAILURE;
      }
    }
  }
  kill(child, SIGKILL);
  waitpid(child, nullptr, 0);
  close(ready[0]);
  close(go[1]);
  return ret;
#else
  return skip_without_linux("/proc/<pid>/exe, /proc/<pid>/root, unshare() with mount namespaces");
#endif
}

// Needs unprivileged user namespaces; with DYNINST_ENABLE_USERNS_TESTS on, not
// having them is a failure.
int test_executable_path_chroot() {
#ifdef os_linux
  namespace bf = boost::filesystem;
  namespace df = Dyninst::filesystem;

  // A process chrooted into this program's directory: our view reaches its
  // root, so the link's text is our own path to the program, but the name is
  // still the target's path under /proc/<pid>/root.
  auto const self_exe = "/proc/" + std::to_string(getpid()) + "/exe";
  auto const program = bf::canonical(self_exe);
  int ready[2];
  if (pipe(ready) != 0)  {
    perror("pipe");
    return EXIT_FAILURE;
  }
  pid_t const jailed = fork();
  if (jailed < 0)  {
    perror("fork");
    return EXIT_FAILURE;
  }
  if (jailed == 0)  {
    close(ready[0]);
    // Which step failed (0 if none), and its errno
    int result[2] = {0, 0};
    if (unshare(CLONE_NEWUSER | CLONE_NEWNS) != 0)  {
      result[0] = 1;
    } else if (chroot(program.parent_path().c_str()) != 0)  {
      result[0] = 2;
    } else if (chdir("/") != 0)  {
      result[0] = 3;
    }
    result[1] = result[0] ? errno : 0;
    if (write(ready[1], result, sizeof result) != static_cast<ssize_t>(sizeof result))  {
      perror("write");
      _exit(1);
    }
    pause();
    _exit(0);
  }
  close(ready[1]);
  int result[2] = {-1, 0};
  int ret = EXIT_SUCCESS;
  bool skip = false;
  bool const answered = read(ready[0], result, sizeof result) == static_cast<ssize_t>(sizeof result);
  auto const jail = "/proc/" + std::to_string(jailed);
  if (!answered)  {
    std::cerr << "no answer from the child about unshare() and chroot()\n";
    ret = EXIT_FAILURE;
  } else if (result[0] == 2 && result[1] == EPERM && apparmor_restricts_userns())  {
    skip = true;
  } else if (result[0] != 0)  {
    char const* const steps[] = {"", "unshare(CLONE_NEWUSER | CLONE_NEWNS)", "chroot()", "chdir(\"/\")"};
    std::cerr << steps[result[0]] << ": " << std::strerror(result[1]) << "\n";
    if (result[0] == 1)  {
      std::cerr << userns_hint;
    }
    ret = EXIT_FAILURE;
  } else if (access((jail + "/root/").c_str(), R_OK) != 0)  {
    std::cerr << "cannot read " << jail << "/root\n";
    ret = EXIT_FAILURE;
  } else {
    auto const expected = jail + "/root/" + program.filename().string();
    auto const name = df::executable_path(jailed);
    if (name != expected)  {
      std::cerr << "executable_path in a chroot: expected '" << expected << "', got '" << name << "'\n";
      ret = EXIT_FAILURE;
    }
  }
  kill(jailed, SIGKILL);
  waitpid(jailed, nullptr, 0);
  close(ready[0]);
  if (skip)  {
    return skip_message("AppArmor gives no capabilities in unprivileged user namespaces "
                        "(kernel.apparmor_restrict_unprivileged_userns=1), so chroot() is refused");
  }
  return ret;
#else
  return skip_without_linux("/proc/<pid>/root, unshare(), chroot()");
#endif
}

// Canonicalizes each input and compares it with the expected path, both without
// terminal slashes and, where the path exists, made canonical.
struct canonicalize_test {
  std::string input;
  std::string expected;
};

static int check_canonicalize(std::vector<canonicalize_test> const& tests) {
  // Remove terminal slashes
  auto simplify = [](std::string &path) {
    namespace bf = boost::filesystem;

    if(path.empty()) {
      return bf::path{};
    }

    auto has_terminal_slash = [&path]() {
      return path.find_last_of('/') == (path.length() - 1);
    };

    while(has_terminal_slash()) {
      path.erase(path.length() - 1);
    }

    // bf::canonical (see below) requires that the path exists.
    if(!bf::exists(path)) {
      return bf::path(path);
    }

    return bf::canonical(bf::path(path));
  };

  bool failed = false;
  auto test_id = 1;

  for(auto t : tests) {
    auto fp = Dyninst::filesystem::canonicalize(t.input);
    if(simplify(fp) != simplify(t.expected)) {
      std::cerr << "Test " << test_id << " '" << t.input << "' failed: expected '"
                << t.expected << "', got '" << fp << "'\n";
      failed = true;
    }
    test_id++;
  }

  return (failed) ? EXIT_FAILURE : EXIT_SUCCESS;
}

// canonicalize() expands "~" from HOME when it is set, and otherwise from the
// password database entry of the effective user.
int test_canonicalize() {
  char const* const env_home = getenv("HOME");
  passwd const* const pw = getpwuid(geteuid());
  if(!env_home && (!pw || !pw->pw_dir)) {
    return skip_message("neither HOME nor a password database entry for this user");
  }
  std::string const home = env_home ? env_home : pw->pw_dir;

  // A name no file in the home directory has, since the file is removed afterwards
  std::string const test_file{
      "/" + boost::filesystem::unique_path("dyninst-canonicalize-%%%%-%%%%").string()};
  std::string const test_file_path{home + test_file};

  // Make sure we have a test file
  {
    std::ofstream fout(test_file_path);
    fout << "\n";
  }

  auto const ret = check_canonicalize({
    {"~", home},
    {"~//", home},
    {"~/" + test_file, test_file_path},
    {home, home},
    {"~UNKNOWN###USER/dir", "~UNKNOWN###USER/dir"}
  });

  boost::filesystem::remove(test_file_path);
  return ret;
}

// canonicalize() expands "~NAME" from NAME's password database entry, never from
// HOME, so the expected name and directory come from there.
int test_canonicalize_user() {
  passwd const* const pw = getpwuid(geteuid());
  if(!pw || !pw->pw_name || !pw->pw_dir) {
    return skip_message("no password database entry for this user");
  }
  std::string const user = pw->pw_name;
  std::string const user_home = pw->pw_dir;

  // A name no file in the home directory has, since the file is removed afterwards
  std::string const test_file{
      "/" + boost::filesystem::unique_path("dyninst-canonicalize-%%%%-%%%%").string()};
  std::string const test_file_path{user_home + test_file};

  // Make sure we have a test file
  {
    std::ofstream fout(test_file_path);
    fout << "\n";
  }

  auto const ret = check_canonicalize({
    {"~" + user, user_home},
    {"~" + user + test_file, test_file_path}
  });

  boost::filesystem::remove(test_file_path);
  return ret;
}

int test_exists() {
  auto file = "test.out";
  std::ofstream fs{file};
  if(!fs) {
    std::cerr << "Failed to create '" << file << "'\n";
    return EXIT_FAILURE;
  }
  if(!Dyninst::filesystem::exists(file)) {
    std::cerr << "'" << file << "' doesn't exist, but should.\n";
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

int test_replace_extension() {
  namespace df = Dyninst::filesystem;

  struct name_test {
    std::string old, new_;
  };

  std::array<name_test, 7> files = {{
      {"foo/bar", "foo/bar.new"},
      {"/foo/bar", "/foo/bar.new"},
      {"foo.txt", "foo.new"},
      {"foo/bar.txt", "foo/bar.new"},
      {"foo/bar.txt.tar", "foo/bar.txt.new"},
      {"foo.bar/bar.txt.tar", "foo.bar/bar.txt.new"},
      {"", ".new"}
  }};

  auto ext = ".new";
  bool failed = false;

  for(auto const& f : files) {
    auto x = df::replace_extension(f.old, ext);
    if(x != f.new_) {
      std::cerr << "replace_extension: expected '" << f.new_
                << "', got '" << x << "'\n";
      failed = true;
    }
  }

  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}

int test_append_filename_suffix() {
  namespace df = Dyninst::filesystem;

  struct name_test {
    std::string old, new_;
  };

  std::array<name_test, 7> files = {{
      {"foo/bar", "foo/bar_new"},
      {"/foo/bar", "/foo/bar_new"},
      {"foo.txt", "foo_new.txt"},
      {"foo/bar.txt", "foo/bar_new.txt"},
      {"foo/bar.txt.tar", "foo/bar_new.txt.tar"},
      {"foo.bar/bar.txt.tar", "foo.bar/bar_new.txt.tar"},
      {"", "_new"}
  }};

  auto suffix = "_new";
  bool failed = false;

  for(auto const& f : files) {
    auto x = df::append_filename_suffix(f.old, suffix);
    if(x != f.new_) {
      std::cerr << "append_filename_suffix: expected '" << f.new_
                << "', got '" << x << "'\n";
      failed = true;
    }
  }

  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}

int test_strip_all_extensions() {
  namespace df = Dyninst::filesystem;

  struct name_test {
    std::string old, new_;
  };

  std::array<name_test, 8> files = {{
      {"foo/bar", "foo/bar"},
      {"foo.txt", "foo"},
      {"foo/bar.txt", "foo/bar"},
      {"foo/bar.txt.tar", "foo/bar"},
      {"foo.bar/bar.txt.tar", "foo.bar/bar"},
      {".", "."},
      {"..", ".."},
      {"", ""}
  }};

  bool failed = false;

  for(auto const& f : files) {
    auto x = df::strip_all_extensions(f.old);
    if(x != f.new_) {
      std::cerr << "strip_all_extensions: expected '" << f.new_
                << "', got '" << x << "'\n";
      failed = true;
    }
  }

  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
