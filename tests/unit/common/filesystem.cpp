#include "common/src/dyninst_filesystem.h"

#include <array>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

static int test_canonicalize();
static int test_exists();
static int test_replace_extension();
static int test_append_filename_suffix();

int main() {
  std::array<int(*)(), 4> tests = {{
      test_canonicalize,
      test_exists,
      test_replace_extension,
      test_append_filename_suffix
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

  std::array<name_test, 4> files = {{
      {"foo/bar", "foo/bar.new"},
      {"foo.txt", "foo.new"},
      {"foo/bar.txt", "foo/bar.new"},
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

  std::array<name_test, 4> files = {{
      {"foo/bar", "foo/bar_new"},
      {"foo.txt", "foo_new.txt"},
      {"foo/bar.txt", "foo/bar_new.txt"},
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
