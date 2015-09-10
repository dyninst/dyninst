#!/usr/bin/env python
# encoding: utf-8
import os, sys
from sets import Set
#from bitarray import BAarray

#MACRO used as a flag for vector and floating points
VEC_SIMD_SWITCH = False
#VEC_SIMD_SWITCH = True

# dir to store aarch64 ISA xml files
ISA_dir = "../../ISA_xml_v2_00rel11/"
files_dir = os.listdir(ISA_dir)

# opcodes and instructions
op_set = Set()
insn_set = Set()

# to get op IDs
def printP(op):
    print '  aarch64_op_'+op+','

def getOpcodes():
    base_insn_file = open(ISA_dir+'index.xml')

    for lines in base_insn_file:
        if lines.startswith("    <iform"):
            op_set.add(lines.split('"')[1].split('.xml')[0].split('_')[0])
            insn_set.add(lines.split('"')[1].split('.xml')[0])

    if VEC_SIMD_SWITCH == True:
        vec_FP_insn_file = open(ISA_dir+'fpsimdindex.xml')
        for lines in vec_FP_insn_file:
            if lines.startswith("    <iform"):
                op_set.add(lines.split('"')[1].split('.xml')[0].split('_')[0])
                insn_set.add(lines.split('"')[1].split('.xml')[0])

    print('enum {')
    printP('INVALID')
    printP('extended')
    for ele in op_set:
        printP(ele)

    print('}')
    print 'number of instructions: ', len(op_set)


# to find all control field names
control_field_set = Set()
non_ctl_field_set = Set(['sf'] )

def getDecodeFieldNames():
    global  control_field_set

    decodeFile = open(ISA_dir + 'encodingindex.xml')

    startReadDecodeField = False

    for line in decodeFile:
        if startReadDecodeField == True:
            if line.find('\"bitfields\"') != -1:
                control_field_set.add(line.split('>')[1].split('<')[0])


        if line.find('Decode fields') !=-1:
            startReadDecodeField = True

        if line.find('</thread>')!=-1 and startReadDecodeField == True:
            startReadDecodeField = False

        if line.find('funcgrouphearder') !=-1 and line.find('simd-dp')!=-1:
            break

    control_field_set.difference_update(non_ctl_field_set)
    print sorted(control_field_set)


def shifting(bitlist):
    out = 0
    for bit in bitlist:
        out = (out<<1) | (bit=='1' )
    return out

# to get the encoding table
def getOpTable():

    # open all op files
    commonBits = int(str('1'*32), 2)
    for files in sorted(files_dir):
        if files.endswith('.xml'):
            if files.split('_')[0] in op_set:
                instruction = files.split('.xml')[0]
                #print files
                curFile = open(ISA_dir+files)

                startDiagram = False
                maskBitArray = list('0'*32)
                maskBitArrayNew = list('0'*32)
                encodingArray = list('0'*32)
                maskStartBit = 31
                startBox = False

                # to analyze lines
                for line in curFile:

                    if line.find('<regdiagram')!=-1:
                        startDiagram = True
                    if line.find('</regdiagram')!=-1:
                        break

                    #diagram starts
                    if startDiagram == True and line.find('<box')!=-1:

                        not_control_field = False
                        if line.find('name=') !=-1:

                            for field in line.split(' '):
                                if field.find('name') != -1 and field.find('usename')==-1:
                                    control_field = field.split('\"')[1]
                                    #print control_field

                                    # if this is control field
                                    if control_field not in control_field_set:
                                        not_control_field = True
                                    break

                        #maskStartBit = 31
                        for x in line.split(' '):
                            if x.find('hibit') != -1:
                                maskStartBit = int(x.split('\"')[1])
                                break

                        maskLen = 1
                        for i in line.split(' '):
                            if i.find('width')!=-1:
                                maskLen = int(i.split('\"')[1])
                                break

                        if not_control_field == False:
                            for s in range(1, maskLen+1):
                                maskBitArray[(31-maskStartBit)+s-1] = '1'

                        startBox = True

                    if line.find('</box') != -1:
                        startBox = False

                    # read box content
                    if startBox == True:
                        if line.find('<c>') != -1:
                            encodeBit = line.split('>')[1].split('<')[0]
                            if encodeBit == '1' or encodeBit == '0':
                                maskBitArrayNew[31-maskStartBit] = '1'
                                encodingArray[31-maskStartBit] = encodeBit
                            else:
                                maskBitArrayNew[31-maskStartBit] = '0'
                                encodingArray[31-maskStartBit] = '0'
                            maskStartBit = maskStartBit - 1
                    # end of <box line>

                # end of each line


                # print instruction and encoding mask per file
                maskBitInt = int(''.join(maskBitArray), 2)
                maskBitNewInt = int(''.join(maskBitArrayNew), 2)

                #print "%22s"%instruction, '\t\t', ''.join(maskBitArray), '\t', ''.join(maskBitArrayNew), '\t', ''.join(encodingArray)
                print "%22s"%instruction, '\t\t', ''.join(maskBitArrayNew), '\t', ''.join(encodingArray)

    insn_unallocated = (2**28+2**27)

    commonBits = commonBits & insn_unallocated
    print "%22s"%'unallocated', '\t\t', bin(insn_unallocated), '\t', hex(insn_unallocated)


getOpcodes()
getDecodeFieldNames()
getOpTable()

