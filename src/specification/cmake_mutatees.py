import os
import tuples
import utils
from collections import defaultdict


def get_compiler_command(exe, platform, abi, info):
   compiler = info['compilers'][exe]
   c_compiler = utils.compiler_command(compiler, platform, abi)
   return c_compiler

def get_flags(platform, compiler, abi, opt, pic):
   c_flags = "${MUTATOR_DEFINES}  %s" % utils.object_flag_string(platform, 
                                                              compiler, 
                                                              abi, 
                                                              opt, 
                                                              pic)
   return c_flags


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


	 
def is_groupable(mutatee, info):
	 if(mutatee['groupable'] == 'false'):
		  return '0'
	 groups = info['rungroups']
	 mutatee_tests = filter(lambda g: g['mutatee'] == mutatee['name'], groups)
         tmp = map(lambda g: len(g['tests']), mutatee_tests)
         if not tmp:
            return '0'
	 if max(tmp) > 1:
		  return '1'
	 else:
		  return '0'

def get_all_mutatee_sources(groupable, module, info):
	return utils.uniq(reduce(lambda a, b: set(a) | set(b),
		(map(lambda m: m['preprocessed_sources'],
		filter(lambda m: m['name'] != 'none'
			and is_valid_test(m) == 'true' and is_groupable(m, info) == groupable and get_module(m) == module,
			info['mutatees']))),
		[]))
	 
def collect_mutatee_comps(mutatees):
   comps = []
   for m in mutatees:
      if m['compiler'] != '' and m['compiler'] not in comps:
         comps.append(m['compiler'])
   return comps

def print_one_cmakefile(exe, abi, stat_dyn, pic, opt, module, path, mlist, platform, cmakelists, info, directory):
   if len(mlist) == 0:
      return

   gen_path = '%s/%s' % (directory, path)

   mut = mlist[0]
   c_compiler = get_compiler_command(exe, platform, abi, info)
   compiler = info['compilers'][exe]

   include_path = '-I${PROJECT_SOURCE_DIR}/testsuite/src -I${PROJECT_SOURCE_DIR}/testsuite/src/%s' % module
   c_flags = "-g %s %s" % (include_path, get_flags(platform, info['compilers'][exe], abi, opt, pic))
   

   if not os.path.exists(gen_path):
      os.makedirs(gen_path)
      
   out = open('%s/CMakeLists.txt' % gen_path, 'w')
   out.write("# CMakeLists for %s\n" % path)
   
   # This is kinda ugly, but we need to force usage of a particular compiler. So 
   out.write("set (CMAKE_C_FLAGS \"%s\")\n" % c_flags)
   out.write("set (CMAKE_C_FLAGS_DEBUG \"\")\n")
   out.write("set (CMAKE_C_FLAGS_RELEASE \"\")\n")
   out.write("set (CMAKE_C_COMPILER \"${M_%s}\")\n" % c_compiler)

   out.write("set (CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})\n")
   out.write("set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})\n")
   out.write("set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})\n")
   out.write("set (CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})\n")

   if platform['name'] == 'i386-unknown-nt4.0':
       out.write("set (CMAKE_C_LINK_EXECUTABLE ${M_native_linker})\n")
       out.write("set (CMAKE_CXX_LINK_EXECUTABLE ${M_native_linker})\n")
   if stat_dyn == 'stat':
      linkage = compiler['staticlink']
   else:
      linkage = compiler['dynamiclink']
   if platform['name'] == 'i386-unknown-nt4.0' and module == 'proccontrol':
      linkage = "%s %s" % (linkage, "ws2_32.lib")
   out.write("set (CMAKE_EXE_LINKER_FLAGS \"%s %s %s\")\n" % ( compiler['flags']['link'],
                                                               compiler['abiflags'][platform['name']][mut['abi']]['flags'],
                                                               linkage))
   
   out.write("include (${PROJECT_SOURCE_DIR}/%s/srclists.cmake)\n" % platform['name'])
   
   # Add each mutatee executable
   for m in mlist:
      out.write("add_executable (%s ${SOURCE_LIST_%d})\n" % (utils.mutatee_binary(m, platform, info),
                                                             m['srclist_index']))
      # This lists all libraries
      if len(m['libraries']) > 0:
         out.write("target_link_libraries (%s" % utils.mutatee_binary(m, platform, info))
         for l in m['libraries']:
            lib_ext = ''
            if l == 'dl':
               l = '${CMAKE_DL_LIBS}'
            if l == 'pthread':
               l = '${CMAKE_THREAD_LIBS_INIT}'
            # This could be handled better....
            if m['abi'] == '32':
               if l == 'testA':
                  lib_ext = '_m32'

            out.write(" %s%s" % (l, lib_ext))
         out.write(")\n")
         
   # And install target
   out.write("\n\n")
   out.write("INSTALL (TARGETS\n")
   for m in mlist:
      out.write("\t\t%s\n" % utils.mutatee_binary(m, platform, info))
   out.write("\tDESTINATION ${INSTALL_DIR})\n")
   
   out.close()

   # And include it from the top-level test suite CMakeLists.txt
   cmakelists.write("\tadd_subdirectory (%s/%s)\n" % (directory, path))


def print_src_lists(mutatees, platform, info, directory):
   out = open("%s/srclists.cmake" % directory, "w")

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
   out.write("set (SRC ${PROJECT_SOURCE_DIR}/src)\n")

   srcs_to_mutatees = {}
   preproc_to_mutatees = {}
   for m in mutatees:
      collected_srcs = ['\t${SRC}/mutatee_driver.c']

      # If it's a group mutatee we need to add the generated group file
      if (is_groupable(m, info) == '1'):
         collected_srcs.append('\t${PROJECT_SOURCE_DIR}/%s/%s_group.c\n' % (platform['name'], m['name']))

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
      groupable = is_groupable(m, info)
      module = m['module']
      if ((utils.extension(s) == ".c") |
          (utils.extension(s) == ".C")):
         # Figure out the test name from the source file...
         ext = utils.extension(s)
         basename = s[0:-len('_mutatee') - len(ext)]

         out.write("set_property (SOURCE ${SRC}/%s/%s APPEND PROPERTY COMPILE_DEFINITIONS TEST_NAME=%s)\n" %
                   (module, 
                    s, 
                    basename))
         out.write("set_property (SOURCE ${SRC}/%s/%s APPEND PROPERTY COMPILE_DEFINITIONS GROUPABLE=%s)\n" %
                   (module, 
                    s, 
                    groupable))
      # Skip raw sources; they don't need the GROUPABLE and TEST_NAMEs set

   out.close()

def nested_dict():
   return defaultdict(nested_dict)

def print_compiler_cmakefiles(mutatees, platform, info, cmakelists, cmake_compilers, directory):
#
# Now that we have each executable defined (in CMake-speak), we
# need to specify the language so we don't get everything compiled
# in ${CC}. 
# YAI (yet another iteration)
# Again, build the list in Python and emit a set/foreach combo
# to CMake. This keeps the files smaller by templating the cut-and
# -paste code. 

   cmake_tree = defaultdict(nested_dict)

   directory_to_muts = {}

   # This must be the inverse of 'path', below

   for mut in mutatees:
      module = mut['module']
      exe = mut['compiler']
      if (exe == ''):
         continue
      abi = mut['abi']
      stat_dyn = utils.mutatee_format(mut['format'])
      pic = mut['pic']
      opt = mut['optimization']
      # REMOVE
      if opt != 'none':
         continue

      path = '%s/%s/%s/%s/%s/%s' % (module, exe, abi, stat_dyn, pic, opt)

      cmake_tree[exe][abi][stat_dyn][pic][opt][module]['path'] = path;
      cmake_tree[exe][abi][stat_dyn][pic][opt][module].setdefault('mutatees', []).append(mut)

   # What valid compiler combinations are there? Let's test that here. We put in CMake tests, 
   # rather than testing it in the python, because we need to do this per-test-build-system. So
   # we do it on the fly and set well-known variable names 

   for exe, tmp1 in cmake_tree.iteritems():
      for abi, tmp2 in tmp1.iteritems():
         for stat_dyn, tmp3 in tmp2.iteritems():
            # Assuming everything else _just works_
            compiler = info['compilers'][exe]
            c_compiler = get_compiler_command(exe, platform, abi, info)
            c_flags = get_flags(platform, info['compilers'][exe], abi, 'none', 'none')
            if stat_dyn == 'stat':
               linkage = compiler['staticlink']
            else:
               linkage = compiler['dynamiclink']
            # Manual hack: check for a present libdl...
            linkage = '%s -ldl' % linkage
            c_flags = '%s %s -ldl' % (c_flags, linkage)

            # You want crazy? Apparently three underscores breaks CMAKE's regexp parser. So no
            # underscores!
            # And we can't redefine a variable as part of the cache, hence the two-level system.
            varname = 'MUTATEE_%s%s%s' % (exe.replace('+','x'), abi, stat_dyn)
            cmake_compilers.write("IF (NOT ${M_%s} MATCHES \"NOTFOUND\")\n" % c_compiler)
            cmake_compilers.write("CHECK_MUTATEE_COMPILER (\"${M_%s}\"\n\t\"%s\"\n\t\"%s\"\n\tdummy%s)\n"
                             % (c_compiler, c_flags, linkage, varname))
            cmake_compilers.write("IF (dummy%s)\n" % varname)
            cmake_compilers.write("SET (%s 1 CACHE STRING \"Build mutatees: compiler %s, ABI %s-bit, %s linked\")\n"
                                  % (varname, exe, abi, stat_dyn))
            cmake_compilers.write("ELSE()\n")
            cmake_compilers.write("SET (%s 0 CACHE STRING \"Skip mutatees: compilers %s, ABI %s-bit, %s linked\")\n"
                                  % (varname, exe, abi, stat_dyn))
            cmake_compilers.write("ENDIF()\n")
            cmake_compilers.write("ENDIF()\n")

   for exe, tmp1 in cmake_tree.iteritems():
      for abi, tmp2 in tmp1.iteritems():
         for stat_dyn, tmp3 in tmp2.iteritems():
            cmakelists.write("if (${MUTATEE_%s%s%s})\n" % (exe.replace('+','x'), 
                                                           abi, 
                                                           stat_dyn))
            for pic, tmp4 in tmp3.iteritems():
               for opt, tmp5 in tmp4.iteritems():
                  for module, tmp6 in tmp5.iteritems():
                     path = tmp6['path']
                     mlist = tmp6['mutatees']
                     print_one_cmakefile(exe, abi, stat_dyn, pic, opt, module, path, mlist, platform, cmakelists, info, directory)
            cmakelists.write("endif()\n")

def write_mutatee_cmakelists(directory, info, platform):
   
   cmakelists = open(directory + "/cmake-mutatees.txt", "w")
   cmake_compilers = open(directory + "/cmake-compilers.txt", "w")

   compilers = info['compilers']
   mutatees = info['mutatees']
   comps = utils.uniq(map(lambda m: m['compiler'], mutatees))
   pname = os.environ.get('PLATFORM')
   modules = utils.uniq(map(lambda t: t['module'], info['tests']))

   print_src_lists(mutatees, platform, info, directory)
   print_compiler_cmakefiles(mutatees, platform, info, cmakelists, cmake_compilers, directory)
#

