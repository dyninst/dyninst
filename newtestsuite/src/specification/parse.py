#! /usr/bin/python

# parse.py
#
# This program is intended to parse the output from the prolog query,
# and output the Makefiles needed to build the tests in the specification.

from sys import argv, exit
import re

record_str = r"""
   \[
   \[(?P<M_name>.+?),(?P<M_src>.+?)\],
   \[(?P<m_name>.+?),(?P<m_src>.+?)\],
   (?P<M_comp>.+?),
   (?P<m_comp>.+?),
   (?P<plat>.+?),
   (?P<debug>.+?),
   (?P<option>.*?)
   \]$"""

prog_str = r"""
   \[
   (?P<name>.+?),\[(?P<src>.+?)\]
   \]
"""
record_re = re.compile(record_str, re.VERBOSE)
prog_re = re.compile(prog_str, re.VERBOSE)
records = []

def parse_flat_list(str):
   if ( len(str) < 2 ):
      print "error: ill formed list: " + str
      exit(1)
   return re.compile(",").split(str[1:-1])

# TODO: The program class is still a little preliminary,
#       it needs to incorporate the data from the Test class
#       like compiler names and options to actually instatiate
#       the Makefile template.
#      
#       Program is intended as a simplification of the Test class
#       to get the implementation right, before working on the
#       full thing
class Program:
   '''Stores Mutator Data'''

   so,aout = range(2)

   # soname, dep names, comp name, comp options
   make_so_template = '''
   %.so: %s
   	%s %s
   '''

   # Constructor:
   # Takes a string from the prolog output and parses
   def __init__(self,record):
      match = prog_re.match(record)

      if ( match == None ):
         print "failed match"
         print record
         exit(1)

      self.name = match.group('name')
      print "Parsing source: " + match.group('src')
      self.src = parse_flat_list(match.group('src'))
      self.type = self.so

      print self.name
      print self.src
      print self.type
   

class Test:
   "Stores the full test tuple"

   
   def __init__(self, record):
      match = record_re.match(record)

      if ( match == None ):
         print "failed match"
         print record
         exit(1)

      self.name = match.group('M_name')
      self.mutator_src = parse_flat_list(match.group('M_src'))
      self.m_name = match.group('m_name')
      self.mutatee_src = parse_flat_list(match.group('m_src'))
      self.M_comp = match.group('M_comp')
      self.m_comp = match.group('m_comp')
      self.plat = match.group('plat')
      self.debug = match.group('debug')
      self.option = match.group('option')

      '''
      print record
      print self.name
      print self.mutator_src
      print self.m_name
      print self.mutatee_src
      print self.M_comp
      print self.m_comp
      print self.plat
      print self.debug
      print self.option
      print
      '''
      
      

def parse_record(rec):
   records.append(Program(rec))
   

# Read in the data from 'filename' and parse each tuples in turn, and
# add them to the records list.
def read_data(filename):
   bracket_depth = 0

   file = open(filename, 'r')
   
   record = ""
   iter = 0
   
   # Read a character at a time and feed it through the simple bracket
   # counting state machine below to seperate the results into individual
   # tuples
   x = file.read(1)
   while ( x != ""  ):
      if ( bracket_depth == 0 ):
         if ( x == "[" ):
            bracket_depth += 1
         elif (x == "]" ):
            bracket_depth -= 1
         elif (x == "\n"):
	   pass
         elif (x == ","):
	   pass
         else:
            print "error: malformed file1: %s"%(iter)
            exit(1)
      elif ( bracket_depth == 1 ):
         if ( x == "[" ):
            bracket_depth += 1
            record = record + x 
         elif ( x == "]" ):
            bracket_depth -= 1
         elif ( x == "," ):
            pass
         else:
            print "error: malformed file2: %s"%(iter)
            exit(1)
      elif ( bracket_depth >= 2 ):
         record = record + x
         if ( x == "[" ):
            bracket_depth += 1
         if ( x == "]" ):
            bracket_depth -= 1
            if ( bracket_depth < 2 ):
               parse_record(record)
               record = ""

      x = file.read(1)
      iter += 1

#   if (bracket_depth != 0):
#      print "error: file ended prematurely"

   

def main():
   filename = argv[1]

   read_data(filename)


if __name__ == "__main__":
   main()
