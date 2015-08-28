#!/usr/bin/env python
# encoding: utf-8
import os, sys
from sets import Set

def printP(op):
    print '  aarch64_op_'+op+','

def getOpcodes():
    # dir to store aarch64 ISA xml files
    ISA_dir = "../../ISA_xml_v2_00rel11/"
    files_dir = os.listdir(ISA_dir)

    base_insn_file = open(ISA_dir+"index.xml")

    op_set = Set()
    for lines in base_insn_file:
        if lines.startswith("    <iform"):
            op_set.add(lines.split('"')[1].split('.xml')[0].split('_')[0])

    print('enum {')
    printP('INVALID')
    printP('extended')
    for ele in op_set:
        printP(ele)

    print('}')


getOpcodes()
