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

def print_mutatee_rules(out, mutatees, compiler, module, platform):
	if(len(mutatees) == 0):
		return
	mut_names = map(lambda x: utils.mutatee_binary(x, platform,info), mutatees)
	out.write("######################################################################\n")
	out.write("# Mutatees compiled with %s for %s\n" % (mutatees[0]['compiler'], module))
	out.write("######################################################################\n\n")
        ifdef_comp = (compiler['presencevar'] != 'true')
	if ifdef_comp:
           out.write("# Only build if this compiler exists\n")
           out.write("if (%s)\n" % compiler['presencevar'])
	pname = os.environ.get('PLATFORM')
	platform = utils.find_platform(pname,info)
	ObjSuffix = platform['filename_conventions']['object_suffix']
	groups = info['rungroups']


def write_mutatee_cmakelists(filename, tuplefile):
   tuples.read_tuples(tuplefile, info)
   compilers = info['compilers']
   mutatees = info['mutatees']
   out = open(filename, "w")
#   print_mutatee_comp_defs(out)
#   comps = collect_mutatee_comps(mutatees)
   comps = utils.uniq(map(lambda m: m['compiler'], mutatees))
   pname = os.environ.get('PLATFORM')
   platform = utils.find_platform(pname, info)
#   ObjSuffix = platform['filename_conventions']['object_suffix']
   modules = utils.uniq(map(lambda t: t['module'], info['tests']))

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

   # For each unique set of sources, create a variable in the cmake file. 
   # For lack of anything better, name it numerically. 

   srcs_to_vars = {}
   i = 0
   for s, m in srcs_to_mutatees.iteritems():
      out.write("set (SOURCE_LIST_%d %s)\n" % (i, s))
      out.write("set (MUTATEE_LIST_%d %s)\n" % (i, utils.mutatee_binary(m,platform,info)))
      srcs_to_vars[s] = i
      i += 1
      
   out.close()
#

