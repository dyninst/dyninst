import os
import tuples
import utils

info = {}

def get_module(mutatee):
   return mutatee["module"]

def is_valid_test(mutatee):
	 if(mutatee['groupable'] == 'false'):
		  return 'true'
	 groups = info['rungroups']
	 mutatee_tests = filter(lambda g: g['mutatee'] == mutatee['name'], groups)
	 if not mutatee_tests:
		  return 'false'
	 else:
	 	  return 'true'


	 
def is_groupable(mutatee):
	 if(mutatee['groupable'] == 'false'):
		  return 'false'
	 groups = info['rungroups']
	 mutatee_tests = filter(lambda g: g['mutatee'] == mutatee['name'], groups)
	 if(max(map(lambda g: len(g['tests']), mutatee_tests)) > 1):
		  return 'true'
	 else:
		  return 'false'

def get_all_mutatee_sources(groupable, module):
	return utils.uniq(reduce(lambda a, b: set(a) | set(b),
		(map(lambda m: m['preprocessed_sources'],
		filter(lambda m: m['name'] != 'none'
			and is_valid_test(m) == 'true' and is_groupable(m) == groupable and get_module(m) == module,
			info['mutatees']))),
		[]))
	 
def collect_mutatee_comps(mutatees):
   comps = []
   for m in mutatees:
      if m['compiler'] != '' and m['compiler'] not in comps:
         comps.append(m['compiler'])
   return comps

def print_src_lists(mutatees, platform, info):
   out = open("srclists.cmake", "w")

   # We want to build the list of sources for each mutatee; since each mutatee
   # gets compiled a bunch of different ways (32/64, no/low/high optimization, 
   # static/dynamic, per-compiler) but the sources stay the same, we can 
   # greatly simplify the output cmake file by making variables for each 
   # unique source list. However, the input "mutatees" exhaustively
   # enumerates the entire list of mutatees. So instead, we iterate over 
   # everything and build a map of sources -> mutatees that care about them.
   # ... ugh. 

   # Since CMake expects everything to be in its current directory, we have to 
   # output the sources with relative paths

   # Make sure this agrees with the subdirectory structure for CMakeLists 
   # as defined below in print_compiler_cmakefiles
   root = '../../../../../..' 
   to_src = '%s/../src' % root
   out.write("set (SRC %s)\n" % to_src)

   srcs_to_mutatees = {}
   preproc_to_mutatees = {}
   for m in mutatees:
      collected_srcs = []
      # Preprocessed == module specific, apparently
      for s in m['preprocessed_sources']:
         collected_srcs.append('\t${SRC}/%s/%s\n' % (m['module'], s))
         collected_srcs.append(" ")
         preproc_to_mutatees.setdefault(s, m)
      # Raw == generic
      for s in m['raw_sources']:
         collected_srcs.append('\t${SRC}/%s\n' % s)
         collected_srcs.append(" ")
      key = ''.join(collected_srcs)
      srcs_to_mutatees.setdefault(key, []).append(m)
      m['srclist'] = key

   # Now that we have this map of sources to mutatees that use them,
   # create CMake lists that mimic the structure. This way we can reference
   # the list instead of reiterating. This is actually an important step;
   # the old mutatee Makefile for x86_64-linux was ~7M. I'm aiming for 1M
   # for the CMake file. 

   srcs_to_vars = dict()
   i = 0
   for s, mlist in srcs_to_mutatees.iteritems():
      out.write("set (SOURCE_LIST_%d \n%s)\n" % (i, s))
      for m in mlist:
         m['srclist_index'] = i
      srcs_to_vars[s] = i
      i += 1

   for s, m in preproc_to_mutatees.iteritems():
      module = m['module']
      out.write("set_property (SOURCE %s APPEND PROPERTY COMPILE_FLAGS \"-DTEST_NAME=%s -DGROUPABLE=%d\")\n" %
                (s,m['name'], m['groupable'] == 'true'))
      # Skip raw sources; they don't need the GROUPABLE and TEST_NAMEs set

   out.close()

def print_compiler_cmakefiles(mutatees, platform, info, cmakelists):
#
# Now that we have each executable defined (in CMake-speak), we
# need to specify the language so we don't get everything compiled
# in ${CC}. 
# YAI (yet another iteration)
# Again, build the list in Python and emit a set/foreach combo
# to CMake. This keeps the files smaller by templating the cut-and
# -paste code. 

   compilers_to_muts = {}

   # This must be the inverse of 'tree', below
   root = '../../../../../..' 

   for mut in mutatees:
      module = mut['module']
      exe = mut['compiler']
      if (exe == ''):
         continue
      abi = mut['abi']
      stat_dyn = utils.mutatee_format(mut['format'])
      pic = mut['pic']
      opt = mut['optimization']
      tree = '%s/%s/%s/%s/%s/%s' % (module, exe, abi, stat_dyn, pic, opt)

      compilers_to_muts.setdefault(tree, []).append(mut)

   for tree, mlist in compilers_to_muts.iteritems():
      mut = mlist[0]

      module = mut['module']
      exe = mut['compiler']
      abi = mut['abi']
      stat_dyn = utils.mutatee_format(mut['format'])
      pic = mut['pic']
      opt = mut['optimization']

      compiler = info['compilers'][mlist[0]['compiler']]

      c_compiler = utils.compiler_command(compiler, platform, abi)
      c_flags = "-I../src/%s %s" % (module, 
                                    utils.object_flag_string(platform, 
                                                             compiler, 
                                                             abi, 
                                                             opt, 
                                                             pic))

      if not os.path.exists(tree):
         os.makedirs(tree)

      out = open('%s/CMakeLists.txt' % tree, 'w')
      out.write("# CMakeLists for %s\n" % tree)

      include_path = '-I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/%s' % module
      out.write("set (CMAKE_C_FLAGS \"%s %s\")\n" % (c_flags, include_path))
      out.write("set (CMAKE_C_FLAGS_DEBUG \"\")\n")
      out.write("set (CMAKE_C_FLAGS_RELEASE \"\")\n")
      out.write("set (CMAKE_C_COMPILE_OBJECT \"${M_%s} -c <FLAGS> -o <OBJECT> -c <SOURCE>\")\n" % c_compiler)

      # Directory tree: compiler, 32/64, static/dynamic, pic/non-pic, optimization

      # This needs to match the number of subdirectories created above
      out.write("include (%s/srclists.cmake)\n" % root)
       
      # Add each mutatee executable
      for m in mlist:
         out.write("add_executable (%s ${SOURCE_LIST_%d})\n" % (utils.mutatee_binary(m, platform, info),
                                                                m['srclist_index']))

      # Override compiler and flags
      # From observation, mutatees need the following flags:
      # -I../src/<module>
      # -DTEST_NAME=<test1_1> (e.g.)
      # -DGROUPABLE=[0,1]
      # <compiler string>
      # The TEST_NAME and GROUPABLE are taken care of in the srclist include file
      # -I../src/module is fixed

      out.close()

      # And include it from the top-level test suite CMakeLists.txt
      cmakelists.write("if (EXISTS ${M_%s})\n\tadd_subdirectory (%s)\nendif()\n" % (c_compiler, tree))


def write_mutatee_cmakelists(tuplefile):
   cmakelists = open("cmake-mutatees.txt", "w")
   tuples.read_tuples(tuplefile, info)
   compilers = info['compilers']
   mutatees = info['mutatees']
#   print_mutatee_comp_defs(out)
#   comps = collect_mutatee_comps(mutatees)
   comps = utils.uniq(map(lambda m: m['compiler'], mutatees))
   pname = os.environ.get('PLATFORM')
   platform = utils.find_platform(pname, info)
#   ObjSuffix = platform['filename_conventions']['object_suffix']
   modules = utils.uniq(map(lambda t: t['module'], info['tests']))

   print_src_lists(mutatees, platform, info)
   print_compiler_cmakefiles(mutatees, platform, info, cmakelists)
#

