#!/usr/bin/env python
# encoding: utf-8

"""
Author: Steve Song
Email:  songxi.buaa@gmail.com
Date:   Sep 2015
"""

import os, sys
from sets import Set
from array import *
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
    #for ele in op_set:
    for ele in sorted(insn_set):
        printP(ele)

    print('}')
    print 'number of instructions: ', len(insn_set)


# to find all control field names
control_field_set = Set()
non_ctl_field_set = Set(['sf'] )

def getDecodeFieldNames():

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
masksArray = list()
encodingsArray = list()
insnArray = list()

def getOpTable():

    # open all op files
    commonBits = int(str('1'*32), 2)
    for files in sorted(files_dir):
        if files.endswith('.xml'):
            instruction = files.split('.xml')[0]
            if instruction in insn_set:
                #print files
                curFile = open(ISA_dir+files)

                startDiagram = False
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
                        #maskStartBit = 31
                        for x in line.split(' '):
                            if x.find('hibit') != -1:
                                maskStartBit = int(x.split('\"')[1])
                                break

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
                maskBitInt = int(''.join(maskBitArrayNew), 2)
                encodingBitInt = int( ''.join(encodingArray),2)

                masksArray.append(maskBitInt)
                encodingsArray.append(encodingBitInt)
                insnArray.append(instruction)

                print "%22s"%instruction, '\t', ''.join(maskBitArrayNew),'(', hex(maskBitInt),')', '\t', ''.join(encodingArray), '(', hex(encodingBitInt), ')'

    insn_unallocated = (2**28+2**27)

    commonBits = commonBits & insn_unallocated
    masksArray.append(insn_unallocated)
    encodingsArray.append(int(0))
    insnArray.append('unallocated')

    print "%22s"%'unallocated', '\t', bin(insn_unallocated), '(', hex(insn_unallocated), ')'

def buildInsnTable():
    assert len(masksArray) == len(encodingsArray) ==len(insnArray)
    print len(insnArray)
    for i in range(1, len(insnArray)):
        print '\taarch64_insn_table.push_back(aarch64_entry(aarch64_op_'+insnArray[i]+', \t\"'+insnArray[i].split('_')[0]+'\t\", list_of() ));'


numOfLeafNodes=0
# need to initialize the mask_table with unallocated entries

def buildDecodeTable(inInsnIndex , processedMask, entryToPlace):
    global numOfLeafNodes

    # guards
    if entryToPlace > 2**32:
        #numOfLeafNodes +=1
        return

    if processedMask == 0xffffffff:
        #numOfLeafNodes +=1
        return

    # terminated condition 1
    if len(inInsnIndex) <=1:
        numOfLeafNodes +=1
        return

    validMaskBits = int(''.join('1'*32), 2)
    for i in inInsnIndex:
        validMaskBits = validMaskBits & masksArray[i]
    validMaskBits = validMaskBits&(~processedMask)
    #print hex(validMaskBits), bin(validMaskBits)

    # till here we should have got the mask bits
    MSBmask = 0x80000000
    curMask = 0
    for bit in range(0, 31):
        if (MSBmask & validMaskBits) == 0 :
            validMaskBits = validMaskBits << 1
        else:
            curMask = 1<<(31-bit)
            break

    # terminated condition 2
    if curMask == 0:
        numOfLeafNodes+=1
        return

    #print hex(curMask)
    zeroIndexes = list()
    oneIndexes = list()

    # typedef mask_table vector<mask_entry>;
    zeroEntryToPlace = entryToPlace*2+1;
    oneEntryToPlace = entryToPlace*2+2;

    #TODO
    print 'mask_table['+str(entryToPlace)+']=mask_entry('+str(hex(curMask))+', '+str(zeroEntryToPlace)+', '+str(oneEntryToPlace)+',0'+');'

    #print hex(curMask)
    for index in inInsnIndex:
        if encodingsArray[index] & curMask == 0:
            zeroIndexes.append(index)
            #print index, bin(encodingsArray[index]), encodingsArray[index]&curMask
        else:
            oneIndexes.append(index)
            #print '\t\t', index, bin(encodingsArray[index]), encodingsArray[index]&curMask

    """
    print '%34s'%bin(curMask)
    for i in zeroIndexes:
        print '%34s'%bin(encodingsArray[i])

    for i in oneIndexes:
        print '%34s'%bin(encodingsArray[i])
    """
    processedMask0 = processedMask | curMask
    processedMask1 = processedMask | curMask

    buildDecodeTable(zeroIndexes, processedMask0, zeroEntryToPlace)
    buildDecodeTable(oneIndexes, processedMask1, oneEntryToPlace)



getOpcodes()
#getDecodeFieldNames()
getOpTable()

buildInsnTable()
validInsnIndex = list( range(0, len(insnArray) ) )
#print validInsnIndex
buildDecodeTable(validInsnIndex, 0 , 0)
print numOfLeafNodes

