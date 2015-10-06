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
VEC_SIMD_SWITCH = False
#VEC_SIMD_SWITCH = True

####################################
# dir to store aarch64 ISA xml files
####################################
ISA_dir = "../../ISA_xml_v2_00rel11/"
files_dir = os.listdir(ISA_dir)

##############################
# parse xml files
# get opcodes
##############################
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

    #### some init ####
    insn_unallocated = (2**28+2**27)
    masksArray.append(insn_unallocated)
    encodingsArray.append(int(0))
    insnArray.append('unallocated')
    operandsArray.append('')
    indexOfInsn = 1

    print 0, '%22s'%'unallocated',  '%34s'%bin(insn_unallocated), '(', hex(insn_unallocated), ')'

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
                operands_Insn = list()
                operands_pos_Insn = list()
                reserve_operand = list()
                reserve_operand_pos = list()
                maskStartBit = 31

                foundOneEncoding = False

                # to analyze lines
                for line in curFile:

                    #diagram starts ##
                    if line.find('<regdiagram')!=-1:
                        startDiagram = True
                        continue

                    if line.find('</regdiagram')!=-1:
                        startDiagram = False
                        foundOneEncoding = True
                        continue
                        #break
                    # end of diagram

                    #analyze each box ##
                    if startDiagram == True and line.find('<box') != -1:
                        #name, start bit, length
                        for x in line.split(' '):
                            if x.find('hibit') != -1:
                                maskStartBit = int(x.split('\"')[1])
                                break

                        reserve_operand = getOperand_Insn(line)
                        reserve_operand_pos = getOperandValues(line)
                        startBox = True

                    if line.find('</box') != -1:
                        startBox = False
                        continue
                    # end of box #######

                    # read box content #################################
                    if startBox == True:
                        # start of <c>
                        #if line.find('<c>') != -1:
                        if line.find('<c') != -1:
                            encodeBit = line.split('>')[1].split('<')[0]

                            # control field
                            if encodeBit != '' and encodeBit != 'x':
                                if encodeBit == '1' or encodeBit == '0':
                                    maskBit[31-maskStartBit] = '1'
                                    encodingArray[31-maskStartBit] = encodeBit
                                else:
                                    maskBit[31-maskStartBit] = '0'
                                    encodingArray[31-maskStartBit] = '0'

                            if encodeBit == '':
                                operands_Insn.append(reserve_operand)
                                operands_pos_Insn.append(reserve_operand_pos)

                                if reserve_operand not in operandsSet:
                                    operandsSet.add(reserve_operand)

                            maskStartBit = maskStartBit - 1
                    # end of <box line> ################################

                    #print instruction in file. Might be multiples in one file
                    if foundOneEncoding == True:
                        #printInstnEntry(maskBit, encodingArray, indexOfInsn, instruction, operands_Insn)
                        printInstnEntry(maskBit, encodingArray, indexOfInsn, instruction, operands_pos_Insn)

                        maskBit = list('0'*32)
                        encodingArray = list('0'*32)
                        operands_Insn = list()
                        operands_pos_Insn = list()

                        foundOneEncoding = False
                        indexOfInsn +=1

                    #end of print instruction

                # end of each line

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


####################################
# generate the c++ code
# which builds the instruction table
####################################
def buildInsnTable():
    assert len(masksArray) == len(encodingsArray) ==len(insnArray)
    print len(insnArray)
    print '*** instruction table ***'

    for i in range(0, len(insnArray)):

        if len(operandsArray[i]) == 0:
            operands = 'operandSpec()'
        else:
            operands = 'list_of'
            for operand in operandsArray[i]:
                if len(operand) != 1:
                    operands += '(fn('+operand[0]+'<'+ str(operand[1][0])+',' + str(operand[1][1])+'>))'
                else:
                    operands += '(fn('+operand[0]+'))'

        print '\tmain_insn_table.push_back(aarch64_insn_entry(aarch64_op_'+insnArray[i]+', \t\"'+insnArray[i].split('_')[0]+'\",\t'+ operands +' ));'


# generate decoding tree ####
numOfLeafNodes=0
processedIndex = Set()
notProcessedIndex = Set()
numNotProcNodes = 0
numNodes = 0

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

entryAvailable = 1

def buildDecodeTable(inInsnIndex , processedMask, entryToPlace):
    global numOfLeafNodes
    global numNotProcNodes
    global numNodes
    global entryAvailable

    # guards, redundant
    '''
    if entryToPlace > 2**32:
        #numOfLeafNodes +=1
        return

    if processedMask == 0xffffffff:
        #numOfLeafNodes +=1
        return
    '''

    # terminated condition 1
    if len(inInsnIndex) < 1:
        #numOfLeafNodes +=1
        return

    if len(inInsnIndex) ==1:
        #printDecodertable(entryToPlace, 0, -1, -1, inInsnIndex[0]);
        printDecodertable(entryToPlace, 0, list() , inInsnIndex[0]);
        numNodes += 1
        #print insnArray[inInsnIndex[0]]
        if inInsnIndex[0] in processedIndex:
            print '[WARN] index processed, repeated index is ', inInsnIndex[0]

        processedIndex.add(inInsnIndex[0])
        numOfLeafNodes +=1
        return

    validMaskBits = int(''.join('1'*32), 2)

    for i in inInsnIndex:
        validMaskBits = validMaskBits & masksArray[i]

    validMaskBits = validMaskBits&(~processedMask)
    #print hex(validMaskBits), bin(validMaskBits)

    # terminated condition 2
    addMask = 0
    if validMaskBits == 0:
        #print '[WARN] all mask bits have been processed', len(inInsnIndex)

        # relaxed solution to alias
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

    for bit in range(0, 32):
        if (MSBmask & validMaskBits) == 0 :
            validMaskBits = (validMaskBits << 1)
        else:
            curMask |= 1<<(31-bit)
            validMaskBits = (validMaskBits << 1)
            numCurMaskBit += 1
            #break

    #print 'cur mask', hex(curMask)

    if curMask == 0:
        #print '%25s'%'processed mask'+'%35s'%bin(processedMask)
        for i in inInsnIndex:
            print '%3d'%i+'%22s'%insnArray[i]+'%35s'%bin(masksArray[i])+'%35s'%bin(encodingsArray[i])
        numOfLeafNodes+=len(inInsnIndex)
        return


    processedMask = processedMask | curMask

    indexList = [ ]
    for i in range(0, 2**numCurMaskBit):
        indexList.append([])

    #print hex(curMask)
    #print indexList
    for index in inInsnIndex:
        branchNo = clapseMask(encodingsArray[index] & curMask, curMask)
        #print 'branchNo' ,branchNo
        indexList[branchNo].append(index)

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
    entryList = []
    for i in range(0, numBranch):
        entryList.append( (posToBrNo[i], entryAvailable + i) )

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

"""
the following section is for parsing and generating operands.
"""

def getOperandValues(line):
    multiOperandSet = set(['S', 'imm', 'option', 'opt', 'N', 'cond'])

    #operandValues #= {'name':'', 'bit':32, 'width':0}
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
        return ('', [0, 0])

    endbit = bit - (width-1)
    #return (name, [bit, width])
    if name in multiOperandSet:
        return (name, [bit, endbit])
    else:
        return (name,)

'''
operandList = list()
operandDict = {}

def analyzeOperands():
    for files in sorted(files_dir):
        if files.endswith('.xml'):
            instruction = files.split('.xml')[0]
            if instruction in insn_set:
                #print files
                curFile = open(ISA_dir+files)

                startDiagram = False
                startBox = False

                # to analyze lines
                for line in curFile:

                    #diagram starts
                    if line.find('<regdiagram')!=-1:
                        startDiagram = True
                    if line.find('</regdiagram')!=-1:
                        break

                    #analyze each box
                    if startDiagram == True and line.find('<box')!=-1:
                        #name, start bit, length
                        operandValues = getOperandValues(line)
                        startBox = True

                    if line.find('</box') != -1:
                        startBox = False

                    # read box content
                    if startBox == True:
                        if line.find('<c') != -1:
                            if line.split('>')[1].split('<')[0] =='':
                                #print type(operandDict)
                                print operandValues
                                #operandDict[operandValues[0]].append(operandValues[1])
                                #operandList.append(operandValues)
                                continue
                            '''

def printOperandFuncs(operandsSet):
    print 'operand function declares'
    for operand in operandsSet:
        print 'void '+operand + '(){ }'


#####################
####### main ########
#####################

getOpcodes()
#getDecodeFieldNames()
getOpTable()

#instruction table
buildInsnTable()

#operands
print '*** operands ***'
print operandsSet
printOperandFuncs(operandsSet)

#decoder table
print '*** decoder table ***'
validInsnIndex = list( range(0, len(insnArray) ) )
buildDecodeTable(validInsnIndex, 0 , 0)
print 'num of vaild leaves', numOfLeafNodes
allIndex = Set(range(0, len(insnArray)) )

#print allIndex
print 'processed indexex: ', processedIndex, len(processedIndex)
print 'missing indexes:', sorted(allIndex - processedIndex), len(allIndex-processedIndex)
print 'number of total nodes in the tree:', numNodes

'''
analyzeOperands()
print 'operands:', operandDict
'''
