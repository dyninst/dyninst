#include "BPatch.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_function.h"
#include "BPatch_image.h"
#include "BPatch_point.h"
#include "BPatch_snippet.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

// A string constant in a snippet (BPatch_constExpr(char const *)) is stored in
// the mutatee's data heap. Code generation runs more than once for the same
// snippet, so the string must be stored once per distinct string, however
// often and wherever it is used; otherwise the copies can push relocated code
// out of reach of its jump slots (issue #2613).
//
// The test calls probe(MARKER) several times at target's entry and exit,
// rewrites the mutatee, counts MARKER in the output, and runs the output to
// check that every call passes the right string.

namespace {

  char const marker[] = "dyninst-stringConstant-test-marker";
  int const calls_per_point = 4;

  BPatch_function *find_function(BPatch_image *image, char const *name) {
    std::vector<BPatch_function *> funcs;
    image->findFunction(name, funcs);
    if (funcs.size() != 1) {
      std::cerr << "Expected one function '" << name << "', found " << funcs.size() << '\n';
      return nullptr;
    }
    return funcs[0];
  }

  bool insert_calls(BPatch_binaryEdit *app, BPatch_function *target, BPatch_procedureLocation loc,
                    BPatch_snippet &call) {
    std::vector<BPatch_point *> *points = target->findPoint(loc);
    if (!points || points->empty()) {
      std::cerr << "No points of type " << loc << " in target\n";
      return false;
    }
    std::vector<BPatch_snippet *> seq(calls_per_point, &call);
    BPatch_sequence body{seq};
    if (!app->insertSnippet(body, *points)) {
      std::cerr << "insertSnippet failed\n";
      return false;
    }
    return true;
  }

  int count_marker(std::string const &path) {
    std::ifstream in{path, std::ios::binary};
    std::string const data{std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{}};
    std::string const needle{marker, sizeof marker}; // with the terminating NUL
    int count = 0;
    for (auto pos = data.find(needle); pos != std::string::npos; pos = data.find(needle, pos + 1)) {
      ++count;
    }
    return count;
  }

}

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <mutatee> <RT library directory>\n";
    return EXIT_FAILURE;
  }
  std::string const output = "stringConstant_mutatee.rewritten";

  BPatch bpatch;
  BPatch_binaryEdit *app = bpatch.openBinary(argv[1], false);
  if (!app) {
    std::cerr << "Unable to open '" << argv[1] << "'\n";
    return EXIT_FAILURE;
  }
  BPatch_image *image = app->getImage();
  BPatch_function *probe = find_function(image, "probe");
  BPatch_function *target = find_function(image, "target");
  if (!probe || !target) {
    return EXIT_FAILURE;
  }

  BPatch_constExpr name{marker};
  std::vector<BPatch_snippet *> args{&name};
  BPatch_funcCallExpr call{*probe, args};
  if (!insert_calls(app, target, BPatch_entry, call) ||
      !insert_calls(app, target, BPatch_exit, call)) {
    return EXIT_FAILURE;
  }
  if (!app->writeFile(output.c_str())) {
    std::cerr << "writeFile failed\n";
    return EXIT_FAILURE;
  }

  bool ok = true;
  int const copies = count_marker(output);
  if (copies != 1) {
    std::cerr << "Expected one copy of the string constant in the output, found " << copies << '\n';
    ok = false;
  }

  // The rewritten mutatee needs the RT library.
  std::string const command = "LD_LIBRARY_PATH='" + std::string{argv[2]} + "' ./" + output + " '" +
                              marker + "' " + std::to_string(2 * calls_per_point);
  int const status = std::system(command.c_str());
  if (status != 0) {
    std::cerr << "'" << command << "' failed with status " << status << '\n';
    ok = false;
  }
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
