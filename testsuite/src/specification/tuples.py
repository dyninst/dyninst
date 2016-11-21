import gzip

# PLList -> []
# PLList -> [ List ]
# List -> Element
# List -> Element , List
# Element -> QString
# Element -> PLList
# QString -> ' string '
# QString -> string

def parse_pllist(pllist):
	if (pllist == '[]'):
		return []
	else:
		return parse_list(pllist[1:-1])

def find_matching_close(search_string):
	# Precondition: search_string starts with '['
	# Keep reading characters from search_string, incrementing a counter for
	# each '[' seen, and decrementing for each ']' seen.  Stop when the counter
	# reaches 0 and return the index of the ']' that decremented the counter
	count = 0
	index = 0
	while True:
		if (search_string[index:index + 1] == '['):
			count = count + 1
		elif (search_string[index:index + 1] == ']'):
			count = count - 1
		index = index + 1
		if (count == 0):
			return index

# FIXME This function assumes that string elements do not contain single quotes
def parse_list(list):
	elements = []
	start = 0
	while (list[start:] != ''):
		# does the first element start with a '['?
		if (list[start:start + 1] == '['):
			# We've got a list for the first element; find the matching close
			# bracket and parse it as a list
			matching_close = find_matching_close(list[start:])
			elements.append(parse_element(list[start:start+matching_close]))
                        start = start + matching_close + 1
		elif list[start:start + 1] == "'":
			# First element in the list is within quotes
			matching_quote = list[start+1:].find("'") + 1
			elements.append(parse_qstring(list[start:start+matching_quote+1]))
			start = start + matching_quote + 2
		else:
			# Find the comma that signifies the end of this element
			next_element = list[start:].find(',')
			if (next_element == -1):
				# This was the last element in the list
				elements.append(parse_element(list[start:]))
				start = start + len(list[start:])
			else:
				elements.append(parse_element(list[start:start+next_element]))
				start = start + next_element + 1
	return elements

def parse_element(element):
	if ((element[:1] == '[') and (element[-1:] == ']')):
		return parse_pllist(element)
	else:
		return parse_qstring(element)

def parse_qstring(qstring):
	if (qstring[:1] == qstring[-1:] == "'"):
		return qstring[1:-1]
	else:
		return qstring

# The following three functions take a list of lists ("tuples") as their
# arguments, and return a list of dictionaries.	 I figure that the dictionaries
# will make it easier to access the values of the tuples.

def parse_platforms(tuplestring):
	plat_list = parse_pllist(tuplestring)
	plat_labels = ('name', 'filename_conventions', 'linker', 'auxilliary_compilers',
				   'abis')
	def convert_tuple(ptup):
		[pl, os, es, lp, ls, l, ac, as1] = ptup
		conv_labels = ('object_suffix', 'executable_suffix',
					   'library_prefix', 'library_suffix')
		fnconv = dict(zip(conv_labels, [os, es, lp, ls]))
		return [pl, fnconv, l, dict(ac), as1]
	plat_map = map(lambda x: dict(zip(plat_labels, convert_tuple(x))),
				   plat_list)
	return plat_map

def parse_languages(tuplestring):
	lang_list = parse_pllist(tuplestring)
	return map(lambda x: dict(zip(('name', 'extensions'), x)), lang_list)

def parse_mutators(tuplestring):
	mutator_list = parse_pllist(tuplestring)
	return map(lambda x: dict(zip(('name','sources','libraries','platform','compiler'), x)), mutator_list)

def parse_mutatees(tuplestring):
	mutatee_labels = ('name', 'preprocessed_sources', 'raw_sources',
					  'libraries', 'platform', 'abi', 'compiler',
					  'optimization', 'groupable', 'module', 'format',
                                          'pic')
	mutatee_list = parse_pllist(tuplestring)
	return map(lambda x: dict(zip(mutatee_labels, x)), mutatee_list)

def parse_tests(tuplestring):
	test_list = parse_pllist(tuplestring)
	return map(lambda x: dict(zip(('name','mutator','mutatee','groupable', 'module'), x)), test_list)

def parse_compilers(tuplestring):
	compiler_tuple_labels = ('executable', 'defstring', 'platforms',
							 'presencevar', 'optimization', 'pic', 'parameters',
							 'languages', 'flags', 'abiflags', 'staticlink',
                                                         'dynamiclink', 'platform')
	compiler_list = parse_pllist(tuplestring)
	compiler_dict = dict(map(lambda x: (x[0], dict(zip(compiler_tuple_labels,x[1:]))), compiler_list))
	for c in compiler_dict:
		compiler_dict[c]['optimization'] = dict(compiler_dict[c]['optimization'])
                compiler_dict[c]['pic'] = dict(compiler_dict[c]['pic'])
		compiler_dict[c]['parameters'] = dict(compiler_dict[c]['parameters'])
		compiler_dict[c]['flags'] = dict(zip(('std', 'mutatee', 'link'), compiler_dict[c]['flags']))
		abiflags = {}
		for p in compiler_dict[c]['platforms']:
			abis = filter(lambda af: af[0] == p, compiler_dict[c]['abiflags'])
			abidict = {}
			for a in abis:
				abidict[a[1]] = dict(zip(('flags','command'),(a[2],a[3])))
			abiflags[p] = abidict
		compiler_dict[c]['abiflags'] = abiflags
	return compiler_dict

def parse_rungroups(tuplestring):
	rungroups_tuple_labels = ('mutatee', 'compiler', 'optimization',
                                  'run_mode', 'start_state', 'groupable', 'tests',
                                  'abi', 'thread_mode', 'process_mode', 'format',
                                  'mutatorstart', 'mutateestart', 'mutateeruntime', 'pic', 'platmode')
	rungroups_list = parse_pllist(tuplestring)
	return map(lambda x: dict(zip(rungroups_tuple_labels, x)), rungroups_list)

# I want this one to return a dictionary mapping the name of an exception type
# to a list containing the names of its parameters
# NOTE: This function is not used
def parse_exception_types(tuplestring):
	exception_type_list = parse_pllist(tuplestring)
	exceptions = map(lambda et: [et[0], et[2]], exception_type_list)
	return dict(exceptions)

# I want this to return a dictionary mapping the name of a file to a dictionary
# mapping each exception type applied to the file to that exception's
# parameters
# NOTE: This function is not used
def parse_exceptions(tuplestring):
	exception_list = parse_pllist(tuplestring)
	# Collect the names of the files with exceptions
	excepted_files = []
	for e in exception_list:
		if e[0] not in excepted_files:
			excepted_files += [e[0]]
	# TODO Build an exception map for each file in excepted_files
	exceptions = {}
	for f in excepted_files:
		es = filter(lambda e: e[0] == f, exception_list)
		em = {}
		for e in es:
			if e[1] in em:
				em[e[1]] += [dict(zip(e[2], e[3]))]
			else:
				em[e[1]] = [dict(zip(e[2], e[3]))]
		exceptions[f] = em
	return exceptions

def parse_object_files(tuplestring):
	object_list = parse_pllist(tuplestring)
	object_labels = ('object', 'compiler', 'sources', 'intermediate_sources',
					 'dependencies',
					 'flags')
	return map(lambda o: dict(zip(object_labels, o)), object_list)

def read_tuples(tuplefile, info):
   f = gzip.open(tuplefile)
   info['platforms'] = parse_platforms(f.readline())
   info['languages'] = parse_languages(f.readline())
   info['compilers'] = parse_compilers(f.readline())
   info['mutators'] = parse_mutators(f.readline())
   info['mutatees'] = parse_mutatees(f.readline())
   info['tests'] = parse_tests(f.readline())
   info['rungroups'] = parse_rungroups(f.readline())
#  info['exception_types'] = parse_exception_types(f.readline())
   info['exception_types'] = None
#  info['exceptions'] = parse_exceptions(f.readline())
   info['exceptions'] = None
   info['objects'] = parse_object_files(f.readline())
   f.close()
