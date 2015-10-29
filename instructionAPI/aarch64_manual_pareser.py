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

#######################################
# a flag for vector and floating points
#######################################
#VEC_SIMD_SWITCH = False
VEC_SIMD_SWITCH = True

####################################
# dir to store aarch64 ISA xml files
####################################
#ISA_dir = "../../ISA_xml_v2_00rel11/"
ISA_dir = '/p/paradyn/arm/arm-download-1350222/AR100-DA-70000-r0p0-00rel10/AR100-DA-70000-r0p0-00rel10/ISA_xml/ISA_xml_v2_00rel11/'
files_dir = os.listdir(ISA_dir)

flagFieldsSet = set(['S', 'imm', 'option', 'opt', 'N', 'cond', 'sz', 'size', 'type'])
forwardFieldsSet = set(['type', ])
##############################
# parse xml files
# get opcodes
##############################
op_set = Set()
insn_set = Set()
fp_insn_set =Set()

# to get op IDs
def printP(op):
    print '  aarch64_op_'+op+','

def getOpcodes():

    # for general purpose instructions
    base_insn_file = open(ISA_dir+'index.xml')
    for lines in base_insn_file:
        if lines.startswith("    <iform"):
            op_set.add(lines.split('"')[1].split('.xml')[0].split('_')[0])
            insn_set.add(lines.split('"')[1].split('.xml')[0])
        # else do nothing

    # for vector and fp instructions
    if VEC_SIMD_SWITCH == True:
        vec_FP_insn_file = open(ISA_dir+'fpsimdindex.xml')
        for lines in vec_FP_insn_file:
            if lines.startswith("    <iform"):
                op_set.add(lines.split('"')[1].split('.xml')[0].split('_')[0])
                insn_set.add(lines.split('"')[1].split('.xml')[0])
                fp_insn_set.add(lines.split('"')[1].split('.xml')[0])
            # else do nothing


    print('enum {')
    printP('INVALID')
    printP('extended')

    for ele in sorted(insn_set):
        printP(ele)

    print('}')
    print 'number of instructions: ', len(insn_set)

#################################
# to find all control field names
# discarded
#################################
'''
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
    '''


def shifting(bitlist):
    out = 0
    for bit in bitlist:
        out = (out<<1) | (bit=='1' )
    return out

##########################
# parse xml files and get
# opcodes and operands
##########################

def getOperand_Insn(line):
    '''
    analyze one <box> line </box> to get operand
    '''
    for field in line.split(' '):
        if field.find('name') != -1 and field.find('usename') == -1:
            opname = field.split('\"')[1]
            if opname.find('imm') !=-1:
                opname = 'imm'
            return opname
        # else continue do nothing

def isRnUpdated(line):
    if line.find('post-idx') != -1 or line.find('pre-idx') !=-1:
        return True;
    return False;


###########################
# to store all masks
# to store all encodings
# to store all instructions
###########################
masksArray = list()
encodingsArray = list()
insnArray = list()
operandsArray = list()
operandsSet = set()

# to get the encoding table
def getOpTable( filename = 'NULL' ):

    global masksArray
    global encodingsArray
    global insnArray
    global operandsSet

    # some init #
    insn_unallocated = (2**28+2**27)
    masksArray.append(insn_unallocated)
    encodingsArray.append(int(0))
    insnArray.append('INVALID')
    operandsArray.append('')
    indexOfInsn = 1

    print 0, '%22s'%'INVALID',  '%34s'%bin(insn_unallocated), '(', hex(insn_unallocated), ')'

    for file in sorted(files_dir):

        if file.endswith('.xml'):
            instruction = file.split('.xml')[0]

            if instruction in insn_set:
                #print file
                curFile = open(ISA_dir+file)

                startDiagram = False
                startBox = False
                maskBit = list('0'*32)
                encodingArray = list('0'*32)
                operands_pos_Insn = list()
                reserve_operand_pos = list()
                maskStartBit = 31
                isRnUp = False

                # to analyze lines #
                for line in curFile:

                    # diagram starts #
                    if line.find('<regdiagram')!=-1:
                        startDiagram = True
                        isRnUp = isRnUpdated(line)
                        continue

                    if line.find('</regdiagram')!=-1:
                        startDiagram = False
                        printInstnEntry(maskBit, encodingArray, indexOfInsn, instruction, operands_pos_Insn)

                        maskBit = list('0'*32)
                        encodingArray = list('0'*32)
                        operands_pos_Insn = list()

                        indexOfInsn +=1

                        continue
                    # end of diagram #

                    # analyze each box #
                    if startDiagram == True and line.find('<box') != -1:
                        #name, start bit, length
                        for x in line.split(' '):
                            if x.find('hibit') != -1:
                                maskStartBit = int(x.split('\"')[1])
                                break

                        reserve_operand_pos = getOperandValues(line, instruction, isRnUp)
                        startBox = True

                    if line.find('</box') != -1:
                        startBox = False
                        continue
                    # end of box #

                    # read box content #
                    if startBox == True:
                        # start of <c>
                        # if line.find('<c>') != -1:
                        if line.find('<c') != -1:
                            encodeBit = line.split('>')[1].split('<')[0]

                            # control field
                            #if encodeBit != '' and encodeBit != 'x':
                            if encodeBit == '1' or encodeBit == '0':
                                maskBit[31-maskStartBit] = '1'
                                encodingArray[31-maskStartBit] = encodeBit

                            elif encodeBit == '(1)':
                                maskBit[31-maskStartBit] = '1'
                                encodingArray[31-maskStartBit] = '1'

                            elif encodeBit == '(0)':
                                maskBit[31-maskStartBit] = '1'
                                encodingArray[31-maskStartBit] = '0'

                            elif encodeBit == 'x':
                                maskBit[31-maskStartBit] = '0'
                                encodingArray[31-maskStartBit] = '0'

                                if reserve_operand_pos[0] in forwardFieldsSet:
                                    operands_pos_Insn.insert(0, reserve_operand_pos)
                                else:
                                    operands_pos_Insn.append(reserve_operand_pos)

                                if reserve_operand_pos[0] not in operandsSet:
                                    operandsSet.add(reserve_operand_pos[0])

                            # if it is blank, do late operand adding
                            elif encodeBit == '':
                                if reserve_operand_pos[0] in forwardFieldsSet:
                                    operands_pos_Insn.insert(0, reserve_operand_pos)
                                else:
                                    operands_pos_Insn.append(reserve_operand_pos)

                                if reserve_operand_pos[0] not in operandsSet:
                                    operandsSet.add(reserve_operand_pos[0])

                            else:
                                if not encodeBit.startswith('!='):
                                    print '[WARN] something not has been analyzed:'+ encodeBit

                            maskStartBit = maskStartBit - 1
                    # end of <box line> #

                # end of each line #

####################################
# generate instructions
####################################
def printInstnEntry( maskBit, encodingArray, index, instruction, operands):
    global masksArray
    global encodingsArray
    global insnArray
    global operandsArray

     # print instruction and encoding mask per file
    maskBitInt = int(''.join(maskBit), 2)
    encodingBitInt = int( ''.join(encodingArray),2)

    masksArray.append(maskBitInt)
    encodingsArray.append(encodingBitInt)
    insnArray.append(instruction)
    operandsArray.append(operands)

    print index, "%22s"%instruction, '\t', ''.join(maskBit),'(', hex(maskBitInt),')', '\t', ''.join(encodingArray), '(', hex(encodingBitInt), ')', operands


def isLDST(insn):
    if insn.startswith('ld') or insn.startswith('st'):
        return True
    return False

def getRegWidth(insn):

    insnMnemonic = insn.split('_')[0]
    # ld/st register, do nothing
    if insnMnemonic.endswith('b') and not insnMnemonic.endswith('sb'):
        return 32
    elif insnMnemonic.endswith('h') and not insnMnemonic.endswith('sh'):
        return 32
    elif insnMnemonic.endswith('sb'):
        return 64
    elif insnMnemonic.endswith('sh'):
        return 64
    elif insnMnemonic.endswith('sw'):
        return 64
    elif insnMnemonic.endswith('r'):
        return 64
    elif insnMnemonic.endswith('p'):
        return 64
    else :
        return 128
        #print '[WARN] not recognized instruction:', insn
    return 64

def isSIMD(insn):
    if insn.find('simd') != -1:
        return True
    return False


####################################
# generate the c++ code
# which builds the instruction table
####################################
def buildInsnTable():
    assert len(masksArray) == len(encodingsArray) ==len(insnArray)
    print len(insnArray)
    print '*** instruction table ***'

    for i in range(0, len(insnArray)):
        instruction = insnArray[i]

        if len(operandsArray[i]) == 0:
            operands = 'operandSpec()'
        else:
            operands = 'list_of'
            if instruction in fp_insn_set:
                if isSIMD(instruction) == False:
                    operands += '( fn(setFPMode) )'
                else:
                    operands += '( fn(setSIMDMode) )'

            if isLDST(instruction) == True:
                if getRegWidth(instruction) == 32 or getRegWidth(instruction) == 64:
                    operands += '( fn(setRegWidth) )'
                else:
                    if getRegWidth(instruction) != 128:
                        print '[WARN] unknown width'

            for operand in operandsArray[i]:
                operands += '( fn('

                if len(operand) != 1:
                    operands += 'OPR'+operand[0]+'<'+ str(operand[1][0])+' COMMA ' + str(operand[1][1])+'>'
                else:
                    curOperandName = operand[0]

                    operands += 'OPR'+ curOperandName
                operands += ') )'

        print '\tmain_insn_table.push_back(aarch64_insn_entry(aarch64_op_'+insnArray[i]+', \t\"'+insnArray[i].split('_')[0]+'\",\t'+ operands +' ));'

##########################
# generate decoding tree #
##########################
numOfLeafNodes=0
processedIndex = Set()
notProcessedIndex = Set()
numNotProcNodes = 0
numNodes = 0

##################
# a helper function
# clapse(1001, 1101) => (101 & 111) => 101 => 5
##################
def clapseMask(encoding, curMask):
    ret = 0
    while curMask!=0:
        if curMask & 0x80000000 != 0:
            ret = (ret<<1)|((encoding & 0x80000000)>>31)
        curMask = (curMask << 1)&0xffffffff
        encoding = (encoding << 1)&0xffffffff
    return ret


####################################
# generate the c++ code
# which builds the decoding table
####################################
def printDecodertable(entryToPlace, curMask=0, entryList=list(), index=-1 ):
    entries = 'map_list_of'
    if len(entryList) == 0:
        entries = 'branchMap()'
    else:
        for ent in entryList:
            entries += '('+str(ent[0])+','+str(ent[1])+')'

    print '\tmain_decoder_table['+str(entryToPlace)+']=aarch64_mask_entry('+str(hex(curMask))+', '+entries+','+str(index)+');'

# global var
entryAvailable = 1

###########################################
# arg0 is the range of indexes in the instruction array
#   that you want to analyze
# arg1 is the bit mask that has been processed
# arg2 is the entry of the decoding table
#   where the current call should place for one decision node
###########################################
def buildDecodeTable(inInsnIndex , processedMask, entryToPlace):
    global numOfLeafNodes
    global numNotProcNodes
    global numNodes
    global entryAvailable

    # guards, redundant
    if entryToPlace > 2**32:
        print '*** [WARN] should not reach here ***'
        return

    # terminated condition 1
    if len(inInsnIndex) < 1:
        print '*** [WARN] should not reach here ***'
        #numOfLeafNodes +=1
        return

    # size of inInsnIndex is 1. This means we should generate a leaf node
    if len(inInsnIndex) ==1:
        printDecodertable(entryToPlace, 0, list() , inInsnIndex[0]);
        numNodes += 1
        #print insnArray[inInsnIndex[0]]
        if inInsnIndex[0] in processedIndex:
            print '[WARN] index processed, repeated index is ', inInsnIndex[0]

        processedIndex.add(inInsnIndex[0])
        numOfLeafNodes +=1
        return

    validMaskBits = int(''.join('1'*32), 2)

    # find the minimum common mask bit field
    for i in inInsnIndex:
        validMaskBits = validMaskBits & masksArray[i]

    # eliminate the processed mask bit field
    validMaskBits = validMaskBits&(~processedMask)
    #print hex(validMaskBits), bin(validMaskBits)

    # if the mask we get is 0, this means a bunch of instructions
    # share the same opcode. They are aliases actually.
    # Group them together.
    # terminated condition 2
    addMask = 0
    if validMaskBits == 0:
        #print '[WARN] all mask bits have been processed', len(inInsnIndex)

        # weak solution to aliases
        numOfLeafNodes += len(inInsnIndex)
        for i in inInsnIndex:
            processedIndex.add(i)
        printDecodertable(entryToPlace, 0, list(), inInsnIndex[0]);
        numNodes += 1
        return

        '''
        for i in inInsnIndex:
            #print '%3d'%i+'%22s'%insnArray[i]+'%34s'%bin(masksArray[i])+'%34s'%bin(encodingsArray[i])
            addMask |= masksArray[i] &(~processedMask)
            #addMask ^= encodingsArray[i] &(~processedMask)

        # handle alias, merge them into one node
        if addMask == 0:
            numOfLeafNodes += len(inInsnIndex)
            for i in inInsnIndex:
                processedIndex.add(i)
            printDecodertable(entryToPlace, 0, list(), inInsnIndex[0]);
            numNodes += 1
            return
        # handle more mask bits
        else:
            validMaskBits = addMask
            #print bin(addMask)
            '''

    # till here we should have got the mask bits
    MSBmask = 0x80000000
    curMask = 0
    numCurMaskBit = 0

    #print hex(validMaskBits)

    # get currrent valid mask
    # check bit one by one from MSB
    for bit in range(0, 32):
        if (MSBmask & validMaskBits) == 0 :
            validMaskBits = (validMaskBits << 1)
        else:
            curMask |= 1<<(31-bit)
            validMaskBits = (validMaskBits << 1)
            numCurMaskBit += 1
            #break

    #print 'cur mask', hex(curMask)

    # should not be 0
    if curMask == 0:
        print "[WARN] mask is 0"
        #print '%25s'%'processed mask'+'%35s'%bin(processedMask)
        for i in inInsnIndex:
            print '%3d'%i+'%22s'%insnArray[i]+'%35s'%bin(masksArray[i])+'%35s'%bin(encodingsArray[i])
        numOfLeafNodes+=len(inInsnIndex)
        return


    # update processed mask
    processedMask = processedMask | curMask

    indexList = [ ]
    for i in range(0, 2**numCurMaskBit):
        indexList.append([])

    # generate the branches
    # glue instructions to that branch
    for index in inInsnIndex:
        branchNo = clapseMask(encodingsArray[index] & curMask, curMask)
        #print 'branchNo' ,branchNo
        indexList[branchNo].append(index)

    # get number of branches
    # a trick to eliminate null branches
    numBranch = 0
    validIndexList = []
    brNoinIndexList = 0
    posToBrNo = list()
    for i in indexList:
        if len(i)!=0:
            numBranch+=1
            validIndexList.append(i)
            posToBrNo.append(brNoinIndexList)
        brNoinIndexList += 1

    # typedef mask_table vector<mask_entry>;
    # assign entry number to that branch
    entryList = []
    for i in range(0, numBranch):
        entryList.append( (posToBrNo[i], entryAvailable + i) )

    # reserve room
    entryAvailable += numBranch

    #TODO
    printDecodertable(entryToPlace, curMask, entryList, -1);
    numNodes += 1

    """
    print '%34s'%bin(curMask)
    for i in zeroIndexes:
        print '%34s'%bin(encodingsArray[i])

    for i in oneIndexes:
        print '%34s'%bin(encodingsArray[i])
    """
    #print validIndexList
    for i in range(0, numBranch):
        buildDecodeTable( validIndexList[i], processedMask, entryList[i][1])

###############################################################
# the following section is for parsing and generating operands.
###############################################################

def getOperandValues(line, instruction, isRnUp):
    multiOperandSet = flagFieldsSet

    if line.find(' name') != -1:
        width = 1
        tokens = line.split(' ')
        for token in tokens:
            if token.find('name')!=-1 and token.find('usename')== -1:
                name = token.split('\"')[1]
                if name.find('imm') != -1:
                    name = 'imm'
            if token.find('hibit')!=-1:
                bit = int(token.split('\"')[1])
            if token.find('width') != -1:
                width = int(token.split('\"')[1])
    else:
        #print '*** [WARN] Blank operand field ***'
        return ('', [0, 0])

    if name.find('Rt') != -1 or name.find('Rt2') != -1 or name.find('Rn') !=-1:
    #if name == 'Rt' or name == 'Rt2' or name == 'Rn':
        if instruction.startswith('ld'):
            name += 'L'

        if instruction.startswith('st'):
            name += 'S'

    # ld/st class
    if instruction.startswith('ld') or instruction.startswith('st'):
        if name.startswith('Rn'):
            if isRnUp == True:
                name += 'U'

            # the following commented section is useless
            '''
            insnMnemonic = instruction.split('_')[0]
            # ld/st register, do nothing
            if insnMnemonic.endswith('b') and not insnMnemonic.endswith('sb'):
                name += '<u8>'
            elif insnMnemonic.endswith('h') and not insnMnemonic.endswith('sh'):
                name += '<u16>'
            elif insnMnemonic.endswith('w') and not insnMnemonic.endswith('sw'):
                name += '<u32>'
            elif insnMnemonic.endswith('sb'):
                name += '<s8>'
            elif insnMnemonic.endswith('sh'):
                name += '<s16>'
            elif insnMnemonic.endswith('sw'):
                name += '<s32>'
            # TODO: need to handle ld/st pair
            #if instruction.endswith('p'):
            else :
                if not insnMnemonic.endswith('r') and not insnMnemonic.endswith('p'):
                    print '[WARN] not recognized instruction:', instruction
                    '''


    endbit = bit - (width-1)
    #return (name, [bit, width])
    if name in multiOperandSet:
        return (name, [bit, endbit])
    else:
        return (name,)


def printOperandFuncs(operandsSet):
    print 'operand function declares'
    for operand in operandsSet:
        print 'void '+ 'OPR'+operand + '(){ }'


#####################
####### main ########
#####################

######################################
# get opcodes of GP instructions
# and FP/VEC instructions respectively
######################################
getOpcodes()

#################################################################
# get instructions, opcodes, lists of operands
# each set is stored in an array
# indexes in the arrays are used to find the instruction you want
#################################################################
getOpTable()

###############################################
# instruction table
# generate C++ code to build instruction tables
###############################################
buildInsnTable()

#########################################
# generate C++ code of operands functions
#########################################
print '*** operands ***'
print sorted(operandsSet)
printOperandFuncs(operandsSet)

###########################################
# Decoder table.
# Generate C++ code to build decoding table.
# Basically, the generated stuff is a decision tree, which is represented in a map.
#
# Each entry stands for a decision node in the decision tree with a few or zero
#   branches. One branch contains a {Key, Value} pair. Key is used to index. Value is
#   the next entry number in the map, like a pointer.
#
# Binaries are passed to the root node first, and compared with the decision mask in
#   that node. Instrution binary does bit operation AND over the mask. The result of it
#   will be compared with the Key of the branches. Once matched to the Key, the instruction
#   will be passed to next decision node pointed by the branch.
#   Recursively do this, until to the leaf node.
#
# Each leaf node contains the index of the instruction array in the last field,
#   which holds the instruction entry.
###########################################
print '*** decoder table ***'
validInsnIndex = list( range(0, len(insnArray) ) )
buildDecodeTable(validInsnIndex, 0 , 0)

###############################
# some statistics for debugging
###############################
print 'num of vaild leaves', numOfLeafNodes
allIndex = Set(range(0, len(insnArray)) )
print 'processed indexex: ', processedIndex, len(processedIndex)
print 'missing indexes:', sorted(allIndex - processedIndex), len(allIndex-processedIndex)
print 'number of total nodes in the tree:', numNodes
