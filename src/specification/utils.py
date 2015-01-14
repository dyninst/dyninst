import os
######################################################################
# Utility functions
######################################################################
#


def uniq(lst):
   return reduce(lambda l, i: ((i not in l) and l.append(i)) or l, lst, [])


def find_platform(pname, info):
   plist = filter(lambda p: p['name'] == pname, info['platforms'])
   if len(plist) == 0:
      return None
   else:
      return plist[0]

def find_language(lname, info):
   llist = filter(lambda l: l['name'] == lname, info['languages'])
   if len(llist) == 0:
      return None
   else:
      return llist[0]


def extension(filename):
   ext_ndx = filename.rfind('.')
   return filename[ext_ndx:]

def replace_extension(name, ext):
   dot_ndx = name.rfind('.')
   return name[:dot_ndx] + ext

# Get the name of the language for the given file.  If more than one
# is possible, I don't really know what to do...  Return a list of all
# the possibilities with compilers on the current platform?
# We should not ever get more than one language for a given extension.  We're
# enforcing that in the tuple generator.
def get_file_lang(filename, info):
   ext = extension(filename)
   langs = filter(lambda x: ext in x['extensions'], info['languages'])
   # We have a list of all languages which match the extension of the file
   # Filter out all of those languages for which there is no compiler on this
   # platform:
   #  Find the compilers available on this platform
   platform = os.environ.get('PLATFORM')
   comps = filter(lambda x: platform in info['compilers'][x]['platforms'],
               info['compilers'])
   #  Get a list of all the languages supported by those compilers
   supported_langs = map(lambda x: info['compilers'][x]['languages'],
                    comps)
   supported_langs = reduce(lambda x,y:x+y, supported_langs)
   supported_langs = uniq(supported_langs)
   #  Remove all languages from langs that aren't in supported_langs
   langs = filter(lambda x: x['name'] in supported_langs, langs)
   if len(langs) > 1:
      return langs
   else:
      return langs[0]

def compiler_count(lang, info):
   plat = os.environ.get('PLATFORM')
   return len(filter(lambda x: lang in info['compilers'][x]['languages']
                       and plat in info['compilers'][x]['platforms']
                 , info['compilers']))

# Returns a string containing the mutatee's compile-time options filename
# component: a string of the form _<compiler>_<ABI>_<optimization>
def mutatee_cto_component(mutatee, info):
   compiler = mutatee['compiler']
   return fullspec_cto_component(compiler,
                          mutatee['abi'],
                          mutatee['optimization'], mutatee['pic'])

# Returns a string containing the mutatee's compile-time options filename
# component for mutatees compiled with an auxilliary compiler
def auxcomp_cto_component(compiler, mutatee):
   return fullspec_cto_component(compiler,
                          mutatee['abi'],
                          mutatee['optimization'],
                          mutatee['pic'])

# Returns a string containing the mutatee's compile-time options filename
# component for a fully specified set of build-time options
def fullspec_cto_component(compiler, abi, optimization, pic):
   retval = "_%s_%s_%s_%s" % (compiler,
                     abi,
                     pic,
                     optimization)
   return retval

# Returns a string containing the link-time options component for a fully-
# specified set of link-time options
# NOTE: There are currently no link-time options specified
def fullspec_lto_component():
   return ""

# Returns a string containing the link-time options component for a mutatee's
# filename
# NOTE: There are currently no link-time options specified
def mutatee_lto_component(mutatee):
   return fullspec_lto_component()

# Returns a string containing the link-time options component for a mutatee's
# filename when the mutatee is compiled using an auxilliary compiler
# NOTE: I think this function is pointless
def auxcomp_lto_component(compiler, mutatee):
   return fullspec_lto_component()

# Returns a string containing the build-time options component for a fully-
# specified set of build-time options
def fullspec_bto_component(compiler, abi, optimization, pic):
   return "%s%s" % (fullspec_cto_component(compiler, abi, optimization, pic),
                fullspec_lto_component())

# Returns a string containing the build-time options component for a mutatee's
# filename
def mutatee_bto_component(mutatee,info):
   return "%s%s" % (mutatee_cto_component(mutatee,info),
                mutatee_lto_component(mutatee))

# Returns a string containing the build-time options component for a mutatee's
# filename, when the mutatee is built using an auxilliary compiler
# NOTE: I don't think this ever happens.
def auxcomp_bto_component(compiler, mutatee):
   return "%s%s" % (auxcomp_cto_component(compiler, mutatee),
                auxcomp_lto_component(compiler, mutatee))

def mutatee_format(formatSpec):
    if formatSpec == 'staticMutatee':
        format = 'stat'
    else:
        format = 'dyn'
    return format

def mutatee_binary(mutatee, platform,info):
   return "%s.%s" % (mutatee['name'], 
                     mutatee_suffix(mutatee, platform, info))

def mutatee_suffix(mutatee, platform, info):
   # Returns standard name for the solo mutatee binary for this mutatee
   format = mutatee_format(mutatee['format'])
   return "%s%s" % (format,
                       mutatee_bto_component(mutatee,info))


# Returns the command used to invoke the compiler
def compiler_command(compiler, platform, abi):
   return compiler['abiflags'][platform['name']][abi]['command']


def normalize_compiler(m):
   exe = m['compiler']
   if exe == '':
      exe = 'nil'
   return exe

#
######################################################################

def object_flag_string(platform, compiler, abi, optimization, pic):
   return "%s %s %s %s %s" % (compiler['flags']['std'],
                                 compiler['flags']['mutatee'],
                                 compiler['abiflags'][platform['name']][abi]['flags'],
                                 compiler['optimization'][optimization],
                                 compiler['pic'][pic])

