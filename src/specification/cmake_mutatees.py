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

def print_add_executable(mutatees, platform, info):
   out = open("srclists.cmake", "w")

   # We want to build the list of sources for each mutatee; since each mutatee
   # gets compiled a bunch of different ways (32/64, no/low/high optimization, 
   # static/dynamic, per-compiler) but the sources stay the same, we can 
   # greatly simplify the output cmake file by making variables for each 
   # unique source list. However, the input "mutatees" exhaustively
   # enumerates the entire list of mutatees. So instead, we iterate over 
   # everything and build a map of sources -> mutatees that care about them.
   # ... ugh. 

   srcs_to_mutatees = {}
   for m in mutatees:
      collected_srcs = []
      for s in m['preprocessed_sources']:
         collected_srcs.append(s)
         collected_srcs.append(" ")
      for s in m['raw_sources']:
         collected_srcs.append(s)
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
      out.write("set (SOURCE_LIST_%d %s)\n" % (i, s))
      out.write("set (MUTATEE_LIST_%d " % i)
      for m in mlist:
         out.write("%s " % utils.mutatee_binary(m,platform,info))
         m['srclist_index'] = i
      out.write(")\n")

      srcs_to_vars[s] = i
      i += 1
   out.close()

def print_set_language(mutatees, platform, info, cmakelists):
#
# Now that we have each executable defined (in CMake-speak), we
# need to specify the language so we don't get everything compiled
# in ${CC}. 
# YAI (yet another iteration)
# Again, build the list in Python and emit a set/foreach combo
# to CMake. This keeps the files smaller by templating the cut-and
# -paste code. 

   compilers_to_muts = {}
   for m in mutatees:
      compilers_to_muts.setdefault(utils.mutatee_suffix(m, platform, info), []).append(m)

   for c, mlist in compilers_to_muts.iteritems():
      cmakelists.write("add_subdirectory (%s)\n" % c)

      # All the mutatees in this group have identical compiler info, so pick one
      mut = mlist[0]

      if mut['compiler'] == '':
         continue

      # Directory tree: compiler, 32/64, static/dynamic, pic/non-pic, optimization
      exe = mut['compiler']
      abi = mut['abi']
      stat_dyn = utils.mutatee_format(mut['format'])
      pic = mut['pic']
      opt = mut['optimization']
      tree = '%s/%s/%s/%s/%s' % (exe, abi, stat_dyn, pic, opt)

      print "Creating directory %s" % tree
      if not os.path.exists(tree):
         os.makedirs(tree)
      out = open('%s/CMakeLists.txt' % tree, 'w')
      out.write("# CMakeLists for the compiler/platform/opt string %s\n" % c)
      out.write("include (srclists.cmake)\n")
      
      # Override compiler/optimization/language/etc.
      for m in mlist:
         out.write("add_executable (%s ${SOURCE_LIST_%d})\n" % (utils.mutatee_binary(m, platform, info),
                                                                m['srclist_index']))
      out.close()


def write_mutatee_cmakelists(filename, tuplefile):
   cmakelists = open("CMakeLists.txt", "w")
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

   print_add_executable(mutatees, platform, info)
   print_set_language(mutatees, platform, info, cmakelists)
#

