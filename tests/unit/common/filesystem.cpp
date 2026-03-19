#include "common/src/dyninst_filesystem.h"

#include <array>
#include <cstdlib>
#include <filesystem>
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
    namespace fs = std::filesystem;

    if(path.empty()) {
      return fs::path{};
    }

    auto has_terminal_slash = [&path]() {
      return path.find_last_of('/') == (path.length() - 1);
    };

    while(has_terminal_slash()) {
      path.erase(path.length() - 1);
    }

    // fs::canonical (see below) requires that the path exists.
    if(!fs::exists(path)) {
      return fs::path(path);
    }

    return fs::canonical(fs::path(path));
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

  namespace fs = std::filesystem;
  fs::remove(fs::path(test_file_path));

  return (failed) ? EXIT_FAILURE : EXIT_SUCCESS;
}
