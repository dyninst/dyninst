#!/usr/bin/python
# quick hack: is a located library static?
# false if shared, true otherwise
import os
import sys
lib_name = sys.argv[1]
lib_dir = sys.argv[2]
dir = os.listdir(lib_dir)
for file in dir:
	has_name = file.find(lib_name)
	has_dot_so = file.find('.so')
	if(has_name > -1 and has_dot_so > -1):
		print "false"
		sys.exit(0)
print "true"
sys.exit(0)
