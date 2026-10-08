#include "common/src/dyninst_filesystem.h"

#include <array>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#ifdef __linux__  // os_linux is defined for the libraries, not the unit tests
#include <sched.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

static int test_canonicalize();
static int test_canonicalize_procfs();
static int test_exists();
static int test_replace_extension();
static int test_append_filename_suffix();
static int test_strip_all_extensions();

int main() {
  std::array<int(*)(), 6> tests = {{
      test_canonicalize,
      test_canonicalize_procfs,
      test_exists,
      test_replace_extension,
      test_append_filename_suffix,
      test_strip_all_extensions
  }};

  bool failed = false;
  for(auto t : tests) {
    if(t() == EXIT_FAILURE) {
      failed = true;
    }
  }
  std::cout << "failed = " << std::boolalpha << failed << "\n";
  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}

int test_canonicalize_procfs() {
#ifdef __linux__
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
  auto const exe = "/proc/" + std::to_string(self) + "/exe";
  if(df::canonicalize(exe, self) != bf::canonical(exe).string()) {
    std::cerr << "canonicalize: '" << exe << "' did not resolve to the executable\n";
    return cleanup(EXIT_FAILURE);
  }

  // A process with its own mount namespace: names are opened through /proc/<pid>/root.
  int ready[2];
  if(pipe(ready) != 0) {
    return cleanup(EXIT_FAILURE);
  }
  pid_t const child = fork();
  if(child < 0) {
    return cleanup(EXIT_FAILURE);
  }
  if(child == 0) {
    close(ready[0]);
    char ok = unshare(CLONE_NEWUSER | CLONE_NEWNS) == 0 ? 1 : 0;
    if(write(ready[1], &ok, 1) != 1) {
      _exit(1);
    }
    pause();
    _exit(0);
  }
  close(ready[1]);  // so read() sees EOF if the child dies first
  char ok = 0;
  bool const unshared = read(ready[0], &ok, 1) == 1 && ok;
  int ret = EXIT_SUCCESS;
  auto const proc = "/proc/" + std::to_string(child);
  if(!unshared) {
    std::cerr << "skipping the mount-namespace case: unshare() is not permitted here\n";
  } else if(access((proc + "/root/").c_str(), R_OK) != 0) {
    std::cerr << "skipping the mount-namespace case: cannot read " << proc << "/root\n";
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
  }
  kill(child, SIGKILL);
  waitpid(child, nullptr, 0);
  close(ready[0]);
  return cleanup(ret);
#else
  return EXIT_SUCCESS;
#endif
}

int test_canonicalize() {
  auto home = []() -> std::string {
    auto* h = getenv("HOME");
    if(!h) {
      return {};
    }
    return h;
  }();

  if(home.empty()) {
    std::cerr << "'HOME' not defined in environment\n";
    return EXIT_FAILURE;
  }

  auto user = []() -> std::string {
    auto* h = getenv("USER");
    if(!h) {
      return {};
    }
    return h;
  }();

  if(user.empty()) {
    std::cerr << "'USER' not defined in environment\n";
    return EXIT_FAILURE;
  }

  struct test {
    std::string input;
    std::string expected;
  };

  std::string const test_file{"/test1"};
  std::string const test_file_path{home + test_file};

  // Make sure we have a test file
  {
    std::ofstream fout(test_file_path);
    fout << "\n";
  }

  // clang-format off
  const std::array<test, 7> tests = {{
    {"~", home},
    {"~//", home},
    {"~/" + test_file, test_file_path},
    {home, home},
    {"~" + user, home},
    {"~" + user + test_file, test_file_path},
    {"~UNKNOWN###USER/dir", "~UNKNOWN###USER/dir"}
  }};
  // clang-format on

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

  namespace bf = boost::filesystem;
  bf::remove(bf::path(test_file_path));

  return (failed) ? EXIT_FAILURE : EXIT_SUCCESS;
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
