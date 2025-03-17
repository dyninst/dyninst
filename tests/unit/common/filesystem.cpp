#include "common/src/dyninst_filesystem.h"

#include <array>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

int main() {
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
