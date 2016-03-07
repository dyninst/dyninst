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
ISA_dir = '/p/paradyn/arm/arm-download-1350222/AR100-DA-70000-r0p0-00rel10/AR100-DA-70000-r0p0-00rel10/ISA_xml/ISA_xml_v2_00rel11/'
files_dir = os.listdir(ISA_dir)

flagFieldsSet = set(['S', 'imm', 'option', 'opt', 'N', 'cond', 'sz', 'size', 'type'])
#forwardFieldsSet = set([ ])

insn_set = Set()
fp_insn_set = Set()

masksArray = list()
encodingsArray = list()

class Opcode:
    def __init__(self, ISA_dir, vec_FP=True):
        self.op_set = Set()
        self.insn_set = Set()
        self.fp_insn_set =Set()
        self.vec_FP = vec_FP
        self.base_insn_file = open(ISA_dir+'index.xml')
        if self.vec_FP == True:
            self.vec_FP_insn_file = open(ISA_dir+'fpsimdindex.xml')

    # to get op IDs
    def printP(self, op):
        print '  aarch64_op_'+op+','

    ##############################
    # parse xml files
    # get opcodes
    ##############################
    def getOpcodes(self):
        for lines in self.base_insn_file:
            if lines.startswith("    <iform"):
                self.op_set.add(lines.split('"')[1].split('.xml')[0].split('_')[0])
                self.insn_set.add(lines.split('"')[1].split('.xml')[0])

        if self.vec_FP == True:
            for lines in self.vec_FP_insn_file:
                if lines.startswith("    <iform"):
                    self.op_set.add(lines.split('"')[1].split('.xml')[0].split('_')[0])
                    self.insn_set.add(lines.split('"')[1].split('.xml')[0])
                    self.fp_insn_set.add(lines.split('"')[1].split('.xml')[0])

    def printOpcodes(self):
        self.getOpcodes()
        print('enum {')
        self.printP('INVALID')
        self.printP('extended')

        for ele in sorted(self.insn_set):
            self.printP(ele)

        print('}')
        print 'number of instructions: ', len(self.insn_set)

    def get_insn_set(self):
        return self.insn_set

    def get_fp_insn_set(self):
        return self.fp_insn_set

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

def ifNeedToSetFlags(line):
    if line.find('<iclass ') != -1 :
        for field in line.split(' '):
            if field.find('id=') !=-1:
                if field.split('\"')[1] == 's':
                    return True

    if line.find('<regdiagram') != -1:
        if line.find('float/compare') != -1:
            return True
    return False


class OpTable:
    global files_dir

    def __init__(self):
        self.masksArray = list()
        self.encodingsArray = list()
        self.insnArray = list()
        self.operandsArray = list()
        self.operandsSet = set()
        self.insn_unallocated = (2**28+2**27)

    def getTable(self):
        return self.masksArray, self.encodingsArray, self.insnArray, self.operandsArray

    def analyzeEncodeBit(self, encodeBit, maskBit, encodingArray, operands_pos_Insn, reserve_operand_pos, maskStartBit):
        if encodeBit == '1' or encodeBit == '0':
            maskBit[31-maskStartBit] = '1'
            encodingArray[31-maskStartBit] = encodeBit

        elif encodeBit == '(1)':
            maskBit[31-maskStartBit] = '1'
            encodingArray[31-maskStartBit] = '1'

        elif encodeBit == '(0)':
            maskBit[31-maskStartBit] = '1'
            encodingArray[31-maskStartBit] = '0'

        # if it is 'x', we set it as not a control field
        # and append the reserved operand to the list
        elif encodeBit == 'x':
            maskBit[31-maskStartBit] = '0'
            encodingArray[31-maskStartBit] = '0'

            if len(operands_pos_Insn) == 0:
                operands_pos_Insn.append(reserve_operand_pos)
            elif operands_pos_Insn[-1:][0] != reserve_operand_pos:
                operands_pos_Insn.append(reserve_operand_pos)
            if reserve_operand_pos[0] not in self.operandsSet:
                self.operandsSet.add(reserve_operand_pos[0])

        # if it is blank, same as 'x', do late operand appending
        elif encodeBit == '' or encodeBit.startswith('!=') != -1:
	    if encodeBit == '!= 0000':
		for i in range(0,4):
		    maskBit[31-maskStartBit+i] = '1'
		    encodingArray[31-maskStartBit+i] = '1'

            operands_pos_Insn.append(reserve_operand_pos)
            if reserve_operand_pos[0] not in self.operandsSet:
                self.operandsSet.add(reserve_operand_pos[0])

        else:
            #if not encodeBit.startswith('!='):
            print '[WARN] something not has been analyzed:'+ encodeBit


    # to get the encoding table
    def printOpTable(self ):

        self.masksArray.append(self.insn_unallocated)
        self.encodingsArray.append(int(0))
        self.insnArray.append('INVALID')
        self.operandsArray.append('')

        indexOfInsn = 1

        print 0, '%22s'%'INVALID',  '%34s'%bin(self.insn_unallocated), '(', hex(self.insn_unallocated), ')'

        for file in sorted(files_dir):

            if file.endswith('.xml'):
                instruction = file.split('.xml')[0]

                if instruction in insn_set:
                    #print file
                    curFile = open(ISA_dir+file)

                    startBox = False
		    hasZeroImmh = False

                    startDiagram = False
                    maskBit = list('0'*32)
                    encodingArray = list('0'*32)
                    operands_pos_Insn = list()

                    reserve_operand_pos = list()
                    maskStartBit = 31
                    isRnUp = False
                    needToSetFlags = False

                    # to analyze lines , do iterative passes#
                    for line in curFile:

                        if line.find('<iclass ') != -1 :
                            needToSetFlags = ifNeedToSetFlags(line)

                        # diagram starts #
                        if line.find('<regdiagram')!=-1:
                            isRnUp = isRnUpdated(line)
                            if needToSetFlags == False:
                                needToSetFlags = ifNeedToSetFlags(line)
                            startDiagram = True
                            continue

                        # end of diagram #
                        if line.find('</regdiagram')!=-1:
                            if needToSetFlags == True:
                                operands_pos_Insn.insert(0, ('setFlags',) )
                            self.printInstnEntry(maskBit, encodingArray, indexOfInsn, instruction, operands_pos_Insn, hasZeroImmh)

                            startDiagram = False
                            maskBit = list('0'*32)
                            encodingArray = list('0'*32)
                            operands_pos_Insn = list()

                            indexOfInsn +=1
                            continue

                        # analyze each box #
                        if startDiagram == True and line.find('<box') != -1:
                            #name, start bit, length
                            for x in line.split(' '):
                                if x.find('hibit') != -1:
                                    maskStartBit = int(x.split('\"')[1])
                                    break

                            # reserve the operand and position information
                            # it only will be appended if the encoding fields are not defined unique
                            reserve_operand_pos = getOperandValues(line, instruction, isRnUp)
                            startBox = True

                        # end of box #
                        if line.find('</box') != -1:
                            startBox = False
                            continue

                        # read box content #
                        if startBox == True:
                            # start of <c>
                            if line.find('<c') != -1:
                                encodeBit = line.split('>')[1].split('<')[0]
				if encodeBit == '!= 0000':
				    hasZeroImmh = True

                                self.analyzeEncodeBit(encodeBit, maskBit, encodingArray, operands_pos_Insn, reserve_operand_pos, maskStartBit)
                                maskStartBit = maskStartBit - 1


                    # end of each line #

    ####################################
    # generate instructions
    ####################################
    def printInstnEntry(self, maskBit, encodingArray, index, instruction, operands, hasZeroImmh):
	if hasZeroImmh == True and encodingArray[21] == '1' and 'Q' not in operands[0][0]:
	    for i in range(19, 23):
		maskBit[31-i] = '0'
		encodingArray[31-i] = '0'

        # print instruction and encoding mask per file
        maskBitInt = int(''.join(maskBit), 2)
        encodingBitInt = int( ''.join(encodingArray),2)

        self.masksArray.append(maskBitInt)
        self.encodingsArray.append(encodingBitInt)
        self.insnArray.append(instruction)
        self.operandsArray.append(operands)

        print index, "%22s"%instruction, '\t', ''.join(maskBit),'(', hex(maskBitInt),')', '\t', ''.join(encodingArray), '(', hex(encodingBitInt), ')', operands


    def isLDST(self, insn):
        if insn.startswith('ld') or insn.startswith('st'):
            return True
        return False

    def getRegWidth(self, insn):

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

    def isSIMD(self, insn):
        if insn.find('simd') != -1:
            return True
        return False


    ####################################
    # generate the c++ code
    # which builds the instruction table
    ####################################
    def buildInsnTable(self):
        assert len(self.masksArray) == len(self.encodingsArray) ==len(self.insnArray)
        print len(self.insnArray)
        print '*** instruction table ***'

        for i in range(0, len(self.insnArray)):
            instruction = self.insnArray[i]

            if len(self.operandsArray[i]) == 0:
                operands = 'operandSpec()'
            else:
                operands = 'list_of'

                # recognize FP and SIMD
                if instruction in fp_insn_set:
                    if self.isSIMD(instruction) == False:
                        self.operandsArray[i].insert(0, ('setFPMode',) )
                    else:
                        self.operandsArray[i].insert(0, ('setSIMDMode',) )

                if self.isLDST(instruction) == True:
                    if self.getRegWidth(instruction) == 32 or self.getRegWidth(instruction) == 64:
                        self.operandsArray[i].insert(0, ('setRegWidth',) )
                    else:
                        if self.getRegWidth(instruction) != 128:
                            print '[WARN] unknown width'

                for index, operand in enumerate(self.operandsArray[i]):
                    # this is solution to the compiler bug
                    # if OPRimm<x, y> appears in the first place of the list
                    if len(operand) != 1 and index == 0:
                        operands += '( (operandFactory) fn('
                    else:
                        operands += '( fn('

                    if len(operand) != 1:
                        operands += 'OPR'+operand[0]+'<'+ str(operand[1][0])+' COMMA ' + str(operand[1][1])+'>'
                    else:
                        curOperandName = operand[0]
                        if curOperandName.startswith('set'):
                            operands += curOperandName
                        else:
                            operands += 'OPR'+ curOperandName

                    operands += ') )'

            #print '\tmain_insn_table.push_back(aarch64_insn_entry(aarch64_op_'+ self.insnArray[i]+', \t\"'+ self.insnArray[i].split('_')[0]+'\",\t'+ operands +' ));'
            print '\tmain_insn_table.push_back(aarch64_insn_entry(aarch64_op_'+ self.insnArray[i]+', \t\"'+ self.insnArray[i].split('_')[0]+'\",\t'+ operands +', ' \
                + str(self.encodingsArray[i]) + ', ' + str(self.masksArray[i]) + ') );'

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

def printDecodertable_list(entryToPlace, curMask=0, entryList=list(), index=list() ):
    entries = 'map_list_of'
    if len(entryList) == 0:
        entries = 'branchMap()'
    else:
        for ent in entryList:
            entries += '('+str(ent[0])+','+str(ent[1])+')'

    index_list = str()
    for i in index:
        index_list += '('+ str(i) +')'

    print '\tmain_decoder_table['+str(entryToPlace)+']=aarch64_mask_entry('+str(hex(curMask))+', '+entries+', list_of'+ index_list+');'

def num1sInMask(x):
    mask = masksArray[x]
    num = 0
    while mask != 0:
        if mask &1 == 1:
            num +=1
        mask = mask>>1
    return int(num)

def alias_comparator( x, y ):
    return num1sInMask(x) - num1sInMask(y)

def alias_comparatorRev( x, y ):
    return num1sInMask(y) - num1sInMask(x)

class DecodeTable:
    global masksArray
    global encodingsArray
    global insnArray
    global operandsArray

    ##########################
    # generate decoding tree #
    ##########################
    def __init__(self):
        self.numOfLeafNodes=0
        self.processedIndex = Set()
        self.numNodes = 0
        ####################################
        # this is the init value that the instruction
        # entry should start from. 0 is reserved for
        # invalid insn.
        ####################################
        self.entryAvailable = 1
        self.debug = False
        self.aliasWeakSolution = True

    # weak solution to aliases
    def alias_weakSolution(self, inInsnIndex, entryToPlace):
        inInsnIndex.sort( cmp=alias_comparator )
        for i in inInsnIndex:
            self.processedIndex.add(i)
            #if self.debug == True:
            #print insnArray[i], '\t', bin( masksArray[i] ), '\t', bin(encodingsArray[i])

        printDecodertable(entryToPlace, 0, list(), inInsnIndex[0]);

    # strict solution to aliases
    def alias_strictSolution(self, inInsnIndex, entryToPlace):
        inInsnIndex.sort( cmp=alias_comparatorRev )

        # for debuggging
        for i in inInsnIndex:
            self.processedIndex.add(i)
            #if self.debug == True:
            print insnArray[i], '\t', bin( masksArray[i] ), '\t', bin(encodingsArray[i])

        printDecodertable_list(entryToPlace, 0, list(), inInsnIndex);



    ###########################################
    # arg-1 is self
    # arg0 is the range of indexes in the instruction array
    #   that you want to analyze
    # arg1 is the bit mask that has been processed
    # arg2 is the entry of the decoding table
    #   where the current call should place for one decision node
    ###########################################
    def buildDecodeTable(self, inInsnIndex , processedMask, entryToPlace):

        # guards, redundant
        if entryToPlace > 2**32:
            print '*** [WARN] should not reach here ***'
            return

        # terminated condition 1
        if len(inInsnIndex) < 1:
            print '*** [WARN] should not reach here ***'
            return

        # size of inInsnIndex is 1. This means we should generate a leaf node
        if len(inInsnIndex) ==1:
            if self.aliasWeakSolution == True:
                printDecodertable(entryToPlace, 0, list() , inInsnIndex[0]);
            else:
                printDecodertable_list(entryToPlace, 0, list() , inInsnIndex[0:1]);
            self.numNodes += 1
            if self.debug == True:
                print insnArray[inInsnIndex[0]]
            if inInsnIndex[0] in self.processedIndex:
                print '[WARN] index processed, repeated index is ', inInsnIndex[0]

            self.processedIndex.add(inInsnIndex[0])
            self.numOfLeafNodes +=1
            return

        validMaskBits = int(''.join('1'*32), 2)

        # find the minimum common mask bit field
        for i in inInsnIndex:
            validMaskBits = validMaskBits & masksArray[i]
	#print "Valid Mask : ", '\t', bin(validMaskBits)
	#print "Processed Mask : ", '\t', bin(processedMask)
        # eliminate the processed mask bit field
        validMaskBits = validMaskBits&(~processedMask)
        if self.debug == True:
            print hex(validMaskBits), bin(validMaskBits)

        #### ALIASING ####
        # terminated condition 2
        # if the mask we get is 0, this means we have a bunch of instructions
        # sharing the same opcode. They are aliases actually.
        # Group them together.
        addMask = 0
        if validMaskBits == 0:
            # weak solution to aliases
            self.alias_weakSolution( inInsnIndex, entryToPlace)
            # strict solution to aliases
            #self.alias_strictSolution(inInsnIndex, entryToPlace)

            self.numOfLeafNodes += len(inInsnIndex)
            self.numNodes += 1
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

        #print 'cur mask', hex(curMask)

        # should not be 0
        if curMask == 0:
            print "[WARN] mask is 0"
            if self.debug == True:
                print '%25s'%'processed mask'+'%35s'%bin(processedMask)
            for i in inInsnIndex:
                print '%3d'%i+'%22s'%insnArray[i]+'%35s'%bin(masksArray[i])+'%35s'%bin(encodingsArray[i])
            self.numOfLeafNodes+=len(inInsnIndex)
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
            entryList.append( (posToBrNo[i], self.entryAvailable + i) )

        # reserve room
        self.entryAvailable += numBranch

        if self.aliasWeakSolution == True:
            printDecodertable(entryToPlace, curMask, entryList, -1);
        else:
            printDecodertable_list(entryToPlace, curMask, entryList, [-1]);
        self.numNodes += 1

        """
        print '%34s'%bin(curMask)
        for i in zeroIndexes:
            print '%34s'%bin(encodingsArray[i])

        for i in oneIndexes:
            print '%34s'%bin(encodingsArray[i])
        """
        if self.debug == True:
            print validIndexList
        for i in range(0, numBranch):
            self.buildDecodeTable( validIndexList[i], processedMask, entryList[i][1])

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
        if instruction.startswith('ld'):
            name += 'L'

        if instruction.startswith('st'):
            name += 'S'

    if instruction.startswith('ld') or instruction.startswith('st'):
        if name.startswith('Rn'):
            if isRnUp == True:
                name += 'U'

    endbit = bit - (width-1)
    if name in multiOperandSet:
        return (name, [bit, endbit])
    else:
        return (name,)


def printOperandFuncs(operandsSet):
    print 'operand function declares'
    for operand in operandsSet:
        print 'void '+ 'OPR'+operand + '(){ }'


def main():

    ######################################
    # get opcodes of GP instructions
    # and FP/VEC instructions respectively
    ######################################
    opcodes = Opcode(ISA_dir, True)
    opcodes.printOpcodes()
    global insn_set
    global fp_insn_set
    insn_set = opcodes.get_insn_set()
    fp_insn_set = opcodes.get_fp_insn_set()

    #################################################################
    # get instructions, opcodes, lists of operands
    # each set is stored in an array
    # indexes in the arrays are used to find the instruction you want
    #################################################################
    opTable = OpTable()
    opTable.printOpTable()

    ###############################################
    # instruction table
    # generate C++ code to build instruction tables
    ###############################################
    opTable.buildInsnTable()

    global masksArray
    global encodingsArray
    global insnArray
    global operandsArray
    masksArray, encodingsArray, insnArray, operandsArray = opTable.getTable()

    #########################################
    # generate C++ code of operands functions
    #########################################
    print '*** operands ***'
    print sorted(opTable.operandsSet)
    printOperandFuncs(opTable.operandsSet)

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
    validInsnIndex = list( range(0, len(opTable.insnArray) ) )
    decodeTable = DecodeTable()
    decodeTable.buildDecodeTable(validInsnIndex, 0 , 0)

    ###############################
    # some statistics for debugging
    ###############################
    print 'num of vaild leaves', decodeTable.numOfLeafNodes
    allIndex = Set(range(0, len(opTable.insnArray)) )
    print 'processed indexex: ', decodeTable.processedIndex, len(decodeTable.processedIndex)
    print 'missing indexes:', sorted(allIndex - decodeTable.processedIndex), len(allIndex-decodeTable.processedIndex)
    print 'number of total nodes in the tree:', decodeTable.numNodes

if __name__ == '__main__':
    main()




