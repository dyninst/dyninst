import os
import tuples
import utils

# Return the name of the mutator for this test
def test_mutator(testname, info):
   testobj = filter(lambda t: t['name'] == testname, info['tests'])
   if len(testobj) >= 1:
      testobj = testobj[0]
   else:
      # TODO Handle this case better
      testobj = None
   if testobj != None:
      mrname = testobj['mutator']
   else:
      mrname = None
   return mrname

# Builds a text label for a test based on the run group's information
# FIXME This is hardcoded to set grouped to false.  It needs to be fixed to
# support the group mutatee optimization
def build_label(test, mutator, rungroup, info):
   label = "{test: %s, mutator: %s, grouped: false" % (test, mutator)
   for n in rungroup:
      # We've already dealt with tests and we don't handle groupable yet
      if n not in ['tests', 'groupable']:
         label = label + ", " + n + ": " + rungroup[n]
   label = label + "}"
   return label

# Return the name of the mutatee executable for this rungroup
def mutatee_filename(rungroup, compilers, platform, info):
   if rungroup['mutatee'] == 'none':
      retval = ""
   else:
      mutatee = rungroup['mutatee']
      bto = utils.fullspec_bto_component(rungroup['compiler'],
                                         rungroup['abi'],
                                         rungroup['optimization'],
                                         rungroup['pic'])
      format = utils.mutatee_format(rungroup['format'])
      es = platform['filename_conventions']['executable_suffix']
      retval = "%s.%s%s%s" % (mutatee, format, bto, es)
   return retval

def print_initialize_mutatees(out, rungroups, compilers, info, platform):
	header = """
static unsigned int test_count = 0;
static unsigned int group_count = 0;
static std::vector<RunGroup *> *tests = NULL;

static void fini_group(RunGroup *rg) {
  rg->index = group_count++;
  tests->push_back(rg);
  test_count = 0;
}

static void add_test(RunGroup *rg, const char *ts) {
  rg->tests.push_back(new TestInfo(test_count++, "%s", ts));
}

// Now we insert the test lists into the run groups
void initialize_mutatees(std::vector<RunGroup *> &t) {
        tests = &t;
	RunGroup *rg;
"""
	LibSuffix = platform['filename_conventions']['library_suffix']
	out.write(header % (LibSuffix))

	# TODO Change these to get the string conversions from a tuple output
	for group in rungroups:
		compiler = info['compilers'][group['compiler']]
#		if compiler['presencevar'] != 'true':
#			out.write("#ifdef %s\n" % (compiler['presencevar']))
		mutateename = mutatee_filename(group, compilers, platform, info)
		out.write('  rg = new RunGroup("%s", ' % (mutateename))
		if group['start_state'] == 'stopped':
			out.write('STOPPED, ')
		elif group['start_state'] == 'running':
			out.write('RUNNING, ')
		elif group['start_state'] == 'selfattach':
			out.write('SELFATTACH, ')
                elif group['start_state'] == 'delayedattach':
                        out.write('DELAYEDATTACH, ')
		else: # Assuming 'selfstart'
			out.write('SELFSTART, ')
		if group['run_mode'] == 'createProcess':
			out.write('CREATE, ')
		elif group['run_mode'] == 'useAttach':
			out.write('USEATTACH, ')
		elif group['run_mode'] == 'deserialize':
			out.write('DESERIALIZE, ')
		else:
			out.write('DISK, ')
		if group['thread_mode'] == 'None':
			out.write('TNone, ')
		elif group['thread_mode'] == 'SingleThreaded':
			out.write('SingleThreaded, ')
		elif group['thread_mode'] == 'MultiThreaded':
			out.write('MultiThreaded, ')
		if group['process_mode'] == 'None':
			out.write('PNone, ')
		elif group['process_mode'] == 'SingleProcess':
			out.write('SingleProcess, ')
		elif group['process_mode'] == 'MultiProcess':
			out.write('MultiProcess, ')

		out.write(group['mutatorstart'])
		out.write(', ')
		out.write(group['mutateestart'])
		out.write(', ')
		out.write(group['mutateeruntime'])
		out.write(', ')

                if group['format'] == 'staticMutatee':
                        out.write('StaticLink, ')
                else:
                        out.write('DynamicLink, ')
		if group['groupable'] == 'true':
			out.write('false, ') # !groupable
		else:
			out.write('true, ') # !groupable
                if group['pic'] == 'pic':
                        out.write('PIC')
                else:
                        out.write('nonPIC')
		try:
			testobj = filter(lambda t: t['name'] == group['tests'][0], info['tests'])
			if len(testobj) < 1:
				raise TestNotFound, 'Test not found: ' + test
			else:
				module = testobj[0]['module']
		except KeyError:
			print "No module found! Test object: " 
			print testobj[0]
			raise
		out.write(', "%s", "%s", "%s", "%s", "%s"' % (module, group['compiler'], group['optimization'], group['abi'], group['platmode']))
		out.write(');\n')
		for test in group['tests']:
			# Set the tuple string for this test
			# (<test>, <mutatee compiler>, <mutatee optimization>, <create mode>)
			# I need to get the mutator that this test maps to..
			mutator = test_mutator(test, info)
			ts = build_label(test, mutator, group, info)
			if test in ['test_serializable']:
				serialize_enable = 'true'
			else:
				serialize_enable = 'false'
			out.write('  add_test(rg, "%s");\n' % (ts))
		out.write('  fini_group(rg);\n')
		# Close compiler presence #ifdef
#		if compiler['presencevar'] != 'true':
#			out.write("#endif // defined(%s)\n" % (compiler['presencevar']))
	out.write('}\n')


def write_test_info_new_gen(directory, info, platform):
   header = """/* This file automatically generated from test specifications.  See
 * specification/spec.pl and specification/makemake.py
 */

#include "test_info_new.h"


"""
   compilers = info['compilers']
   rungroups = info['rungroups']
   out = open(directory + '/test_info_new.gen.C', "w")
   out.write(header)
   print_initialize_mutatees(out, rungroups, compilers, info, platform)
   out.close()

