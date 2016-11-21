import os
import tuples
import utils

def format_test_defines(test):
   line = 'extern int ' + test + '_mutatee();\n'
   return line

def format_test_info(test):
   line = '  {"' + test + '", ' + test + '_mutatee, GROUPED, "' + test + '"}'
   return line

def accumulate_tests_by_mutatee(acc, g):
   if g['mutatee'] in acc:
      acc[g['mutatee']] = acc[g['mutatee']] | set(g['tests']);
   else:
      acc.update([(g['mutatee'], set(g['tests']))])
   return acc

def write_group_mutatee_boilerplate_file(filename, tests, info, platform):
   out = open(filename, "w")
   out.write("#ifdef __cplusplus\n")
   out.write('extern "C" {\n')
   out.write("#endif\n")
   out.write('#include "../src/mutatee_call_info.h"\n\n')
   map(lambda t: out.write(format_test_defines(t)), tests)
   out.write("\n")
   out.write("mutatee_call_info_t mutatee_funcs[] = {\n")
   out.write(reduce(lambda s, t: s + ',\n' + t, map(format_test_info, tests)))
   out.write("\n")
   out.write("};\n")
   out.write("\n")
   out.write("int max_tests = %d;\n" % (len(tests)))
   out.write("int runTest[%d];\n" % (len(tests)))
   out.write("int passedTest[%d];\n" % (len(tests)))
   out.write("#ifdef __cplusplus\n")
   out.write("}\n")
   out.write("#endif\n")
   out.close()

def write_group_mutatee_boilerplate(directory, info, platform):
   groups = filter(lambda g: len(g['tests']) >= 2, info['rungroups'])
   tests_by_group = reduce(accumulate_tests_by_mutatee, groups, {})
   for mutatee, tests in tests_by_group.iteritems():
      write_group_mutatee_boilerplate_file(directory + '/' + mutatee + '_group.c', tests, info, platform)

