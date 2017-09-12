#!/usr/bin/env python
# encoding: utf-8

 # See the dyninst/COPYRIGHT file for copyright information.
 #
 # We provide the Paradyn Tools (below described as "Paradyn")
 # on an AS IS basis, and do not warrant its validity or performance.
 # We reserve the right to update, modify, or discontinue this
 # software at any time.  We shall have no obligation to supply such
 # updates or modifications or any other form of support to you.
 #
 # By your use of Paradyn, you understand and agree that we (or any
 # other person or entity with proprietary rights in Paradyn) are
 # under no obligation to provide either maintenance services,
 # update services, notices of latent defects, or correction of
 # defects for Paradyn.
 #
 # This library is free software; you can redistribute it and/or
 # modify it under the terms of the GNU Lesser General Public
 # License as published by the Free Software Foundation; either
 # version 2.1 of the License, or (at your option) any later version.
 #
 # This library is distributed in the hope that it will be useful,
 # but WITHOUT ANY WARRANTY; without even the implied warranty of
 # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 # Lesser General Public License for more details.
 #
 # You should have received a copy of the GNU Lesser General Public
 # License along with this library; if not, write to the Free Software
 # Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

"""
Author: Steve Song
Email:  songxi.buaa@gmail.com
Date:   Sep 2015
"""

import os
import sys
import copy
import re
import string
import collections
import xml.etree.ElementTree

allDefs = {}
invalidCharTbl = string.maketrans('[<:>]', '_____')

def warn(*s):
    print('[WARN] ' + ' '.join([str(x) for x in s]))

def error(*s):
    message = '[ERROR] ' + ' '.join([str(x) for x in s])
    sys.exit(message)

def cmp_deps(a, b):
    if not a: a = ''
    if not b: b = ''
    c = cmp(b.count(':'), a.count(':'))
    return c if c else cmp(a, b)

def masked_string(bits, mask):
    bits = bin(bits)[2:].zfill(32)
    mask = bin(mask)[2:].zfill(32)
    return '[' + ''.join(bits[x] if mask[x] == '1' else ' ' for x in range(32)) + ']'

def indent_code(space, rawCode):
    indent = ' ' * space
    return os.linesep.join(indent + x if x else x
                           for x in rawCode.split(os.linesep))

#######################################
# a flag for vector and floating points
#######################################
#VEC_SIMD_SWITCH = False
VEC_SIMD_SWITCH = True

###############################################################
# First argument should be a directory containing ISA xml files
###############################################################
if len(sys.argv) <= 1:
    sys.stderr.write('Usage: '+sys.argv[0]+' [ISA_xml_dir]')
    sys.stderr.write(os.linesep * 2)
    sys.exit(-1)

ISA_dir = sys.argv[1] + os.sep
files_dir = os.listdir(ISA_dir)
archName = 'ARM_ARCH_UNKNOWN'

flagFieldsSet = set(['S', 'imm', 'option', 'opt', 'N', 'cond', 'sz', 'size', 'type'])
#forwardFieldsSet = set([ ])

insn_set = set()
fp_insn_set = set()

masksArray = []
encodingsArray = []

class Opcode:
    def __init__(self, ISA_dir):
        self.ifiles = set()
        self.op_set = set()
        self.insn_set = set()
        self.fp_insn_set = set()
        self.base_insn_file = ISA_dir+'index.xml'
        self.vec_FP_insn_file = ISA_dir+'fpsimdindex.xml'
        self.getOpcodes()

    # to get op IDs
    def printP(self, op):
        global archName
        print '  '+archName+'_op_'+op+','

    ##############################
    # parse xml files
    # get opcodes
    ##############################
    def getOpcodes(self):
        if os.path.isfile( self.base_insn_file ):
            index = xml.etree.ElementTree.parse( self.base_insn_file )
            assert(index.getroot().tag == 'alphaindex')

            title = index.find('toptitle')
            instructionset = title.get('instructionset')

            global archName
            if instructionset == 'A64':
                archName = 'aarch64'
            elif instructionset == 'AArch32':
                archName = 'aarch32'

            for iform in index.iter('iform'):
                iformfile = iform.get('iformfile')

                self.ifiles.add(iformfile)
                self.op_set.add(iformfile.split('_')[0])
                self.insn_set.add(iformfile.split('.xml')[0])

        if os.path.isfile( self.vec_FP_insn_file ):
            index = xml.etree.ElementTree.parse( self.vec_FP_insn_file )

            for iform in index.iter('iform'):
                iformfile = iform.get('iformfile')

                self.ifiles.add(iformfile)
                self.op_set.add(iformfile.split('_')[0])
                self.insn_set.add(iformfile.split('.xml')[0])
                self.fp_insn_set.add(iformfile.split('.xml')[0])

    def printOpcodes(self):
        print('enum {')
        self.printP('INVALID')
        self.printP('extended')

        for ele in sorted(self.insn_set):
            self.printP(ele)

        print('}')
        print 'number of instructions: ', len(self.insn_set)

    def get_ifiles(self):
        return self.ifiles

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

class OpTable_aarch32:
    global files_dir

    class FieldInfo:
        def __init__(self, name=''):
            self.name    = name
            self.highBit = -1
            self.lowBit  = -1
            self.bitsSet =  0

        def code_string(self):
            if self.highBit != -1:
                return 'OPR%s<%d COMMA %d>' % (
                    self.name, self.highBit, self.lowBit)
            else:
                return 'OPR' + self.name

        def __lt__(self, other):
            return self.highBit - other.highBit

        def __str__(self):
            if self.highBit != -1 and self.lowBit != -1:
                return '%s[%d:%d]' % (self.name, self.highBit, self.lowBit)
            else:
                return self.name
        #
        # End of class OpTable_aarch32.FieldInfo definition.
        #

    class BitList:
        def __init__(self, width=32, rawValue=None):
            if rawValue:
                binaryString = '{0:0' + str(width) + 'b}'
                self.bits = list(binaryString.format(rawValue))
                self.bits.reverse()
            elif width:
                self.bits = [None] * width
            else:
                self.bits = list()

        def clear(self, position):
            self.bits[position] = None

        def __getitem__(self, position):
            return self.bits[position]

        def __setitem__(self, position, value):
            self.bits[position] = value

        def __int__(self):
            return int(str(self), 2)

        def __str__(self):
            values = [(x if x else '0') for x in self.bits]
            return ''.join(reversed(values))

        def __len__(self):
            return len(self.bits)

        def get_mask(self):
            mask = OpTable_aarch32.BitList()
            mask.bits = [('1' if x else '0') for x in self.bits]
            return mask
        #
        # End of class OpTable_aarch32.BitList definition.
        #

    class Instruction:
        def __init__(self, rawValue=None):
            self.id         = 'INVALID'
            self.bitList    = OpTable_aarch32.BitList(32, rawValue)
            self.fieldList  = []
            self.fieldDict  = {}
            self.symList    = []
            self.funcSet    = set()
            self.info       = {'mnemonic': 'INVALID'}

        # Import an "<asmtemplate>" XML tag for operand information.
        #
        def import_asmtemplate(self, asmtemplate):
            alltext = [ x.text for x in asmtemplate.iter() ]
            for text in [ x.text for x in asmtemplate.iter() ]:
                if (not text): continue
                if (not text.startswith('<')): continue
                if (not text.endswith('>')): continue

                self.symList.append(text.strip('<>'))

        # Import a "<pstext>" XML tag for operand generation information.
        #
        def import_pstext(self, pstext):
            if (pstext is None): return

            pscode = ''.join(pstext.itertext())
            self.funcSet.update(re.findall(r'\S+\([^)]*imm.*?\)', pscode))

        # Return the field that corresponds to the given bit position.
        def find_field(self, position):
            fields = [x for x in self.fieldList if (x.highBit >= position and
                                                    x.lowBit  <= position)]
            if (len(fields) == 1):
                return fields[0]

            error(self.id, 'bit', position, 'is ambiguous')

        # Import a "<c>" XML tag for bitfield value information.
        #
        def import_bit_cell(self, cell, position, field):
            value = '' if cell.text is None else cell.text

            if value in ['1', '0']: #, '(1)', '(0)']:
                self.bitList[position] = value.strip('()')
                field.bitsSet += 1

            elif value in ['x', 'N', 'Z'] or value.startswith('!='):
                # These fields do not contain control bits.
                width = int(cell.get('colspan', '1'))
                for i in range(position, position - width, -1):
                    self.bitList.clear(i)

            elif value:
                warn('Unknown bit value:', value)

        # Import a "<regdiagram>" XML tag for bit field encoding information.
        #
        def import_boxes(self, regdiagram):
            for box in regdiagram.findall('box'):

                field = OpTable_aarch32.FieldInfo()
                if (box.get('usename') == '1'):
                    field.name = self.update_field_name(box.get('name'))
                field.highBit = int(box.get('hibit'))

                # ARM XML Bug: Sometimes, the <box> element's width
                # attribute is incorrectly specified as an empty
                # string.  Instead, discover the box width by counting
                # child <c> elements (and colspans).

                position = field.highBit
                for cell in box.findall('c'):
                    self.import_bit_cell(cell, position, field)
                    position -= int(cell.get('colspan', '1'))
                    field.lowBit = position + 1

                self.fieldList.append(field)
                if (field.name):
                    self.fieldDict[field.name] = field

            # Verify that all bits have been accounted for.
            bit = len(self.bitList) - 1
            for field in self.fieldList:
                if bit != field.highBit:
                    error(self.id, 'diagram missing bit', bit)
                bit = field.lowBit - 1
            if bit != -1:
                error(self.id, 'diagram ends before reaching bit', bit)

        # Update this instruction from an "<encoding>" XML tag.
        #
        def update_boxes(self, encoding):
            for box in encoding.findall('box'):
                position = int(box.get('hibit'))

                for cell in box.findall('c'):
                    field = self.find_field(position)
                    self.import_bit_cell(cell, position, field)
                    position -= int(cell.get('colspan', '1'))

            # Verify that unnamed fields do not contain ambiguous bits.
            for field in self.fieldList:
                if (not field.name
                    and (field.highBit - field.lowBit + 1) != field.bitsSet):
                    warn('Unnamed field contains ambiguous bits')

        # Import an XML element's docvars child for key/value metadata.
        #
        def import_docvars(self, xmlElem):
            for docvar in xmlElem.findall('./docvars/docvar'):
                self.info[docvar.get('key')] = docvar.get('value')

        def is_load(self):
            return self.info['mnemonic'].startswith('LD')

        def is_store(self):
            return self.info['mnemonic'].startswith('ST')

        def is_Rn_updated(self):
            addressForm = self.info.get('address-form')
            return addressForm in ['pre-indexed', 'post-indexed']

        def is_flags_updated(self):
            # XXX Must have a more comprehensive test here.
            return ('S' in self.fieldDict)

        def is_SIMD(self):
            return self.info.get('instr-class') == 'fpsimd'

        def get_reg_width(self):
            mnemonic = self.info['mnemonic']
            # ld/st register, do nothing
            if mnemonic.endswith('B') and not mnemonic.endswith('SB'):
                return 32
            elif mnemonic.endswith('H') and not mnemonic.endswith('SH'):
                return 32
            elif mnemonic.endswith('SB'):
                return 64
            elif mnemonic.endswith('SH'):
                return 64
            elif mnemonic.endswith('SW'):
                return 64
            elif mnemonic.endswith('R'):
                return 64
            elif mnemonic.endswith('P'):
                return 64
            else:
                return 128
                #print '[WARN] not recognized instruction:', insn
            return 64

        def update_field_name(self, originalName):
            return originalName

            newName = originalName if originalName else ''

            if newName.startswith('imm'):
                newName = 'imm'

            if newName in ['Rt', 'Rt2', 'Rn']:
                if self.is_load():  newName += 'L'
                if self.is_store(): newName += 'S'

            if newName.startswith('Rn') and self.is_Rn_updated():
                if self.is_load() or self.is_store():
                    newName += 'U'

            return newName

        def get_operand_code(self):
            operands = list()

            if self.is_load() or self.is_store():
                width = self.get_reg_width()
                if width in [32, 64]:
                    operands.append('setRegWidth')
                elif width != 128:
                    warn('Unknown width:', width)

            # recognize FP and SIMD
            if self.is_SIMD():
                operands.append('setSIMDMode')

            if self.is_flags_updated():
                operands.append('setFlags')

            # Any field with unspecified bit values is considered an operand.
            for field in self.fieldDict.values():
                # Always ignore these fields.
                if field.name in [None, 'cond']:
                    continue

                allBits = field.highBit - field.lowBit + 1
                if field.bitsSet < allBits:
                    operands.append(field.code_string())

            if len(operands) == 0:
                code = 'operandSpec()'
            else:
                codeStrings = ['( fn(' + x + ') )' for x in operands]
                code = '( (operandFactory) ' + ''.join(codeStrings) + ' )'

            return code

        def __str__(self):
            mask = self.bitList.get_mask()

            return '%10s %s ( 0x%08x ) %s ( 0x%08x ) %s' % (
                self.info['mnemonic'].lower(),
                str(mask), int(mask),
                str(self.bitList), int(self.bitList),
                [str(x) for x in self.fieldDict.values()])
        #
        # End of class OpTable_aarch32.Instruction definition.
        #

    class Decoder:
        class MaskEntry:
            def __init__(self, mask, valueIdx, length = -1):
                self.mask     = mask
                self.valueIdx = valueIdx
                self.length   = length

        class ValueEntry:
            def __init__(self, value = -1, maskIdx = -1):
                self.value   = value
                self.maskIdx = maskIdx

        def __init__(self, insnList):
            self.insnList = insnList
            self.maskTable = []
            self.valueTable = []
            self.build_mask_table(range(len(insnList)), 0x0)

        def build_mask_table(self, insnIdxList, usedBits):
            indexCount = len(insnIdxList)
            assert(indexCount > 0)

            mask = self.find_common_mask(insnIdxList)
            mask &= ~usedBits

            if (indexCount == 1 or mask == 0x0):
                insnIdxList.sort(key = self.alias_key)
                for idx in reversed(range(len(insnIdxList))):
                    entry = OpTable_aarch32.Decoder.MaskEntry(0x0,
                                                              insnIdxList[idx],
                                                              idx + 1)
                    self.maskTable.append(entry)
                return

            valueIdx = len(self.valueTable)
            entry = OpTable_aarch32.Decoder.MaskEntry(mask, valueIdx)
            self.maskTable.append(entry)
            entry.length = self.build_value_table(insnIdxList, usedBits, mask)

        def build_value_table(self, insnIdxList, usedBits, mask):
            valueGroups = collections.defaultdict(list)
            for idx in insnIdxList:
                value = mask & int(self.insnList[idx].bitList)
                valueGroups[value].append(idx)

            # Reserve space in the valueTable
            baseIdx = len(self.valueTable)
            for i in range(len(valueGroups)):
                self.valueTable.append(OpTable_aarch32.Decoder.ValueEntry())

            # Update reserved space entries
            for (i, value) in enumerate(sorted(valueGroups)):
                self.valueTable[baseIdx + i].value = value
                self.valueTable[baseIdx + i].maskIdx = len(self.maskTable)
                self.build_mask_table(valueGroups[value], mask | usedBits)

            return len(valueGroups)

        def find_common_mask(self, insnIdxList):
            mask = int(self.insnList[insnIdxList[0]].bitList.get_mask())
            for idx in insnIdxList:
                mask &= int(self.insnList[idx].bitList.get_mask())
            return mask

        def alias_key(self, idx):
            mask = int(self.insnList[idx].bitList.get_mask())
            num = 0
            while mask:
                num += (mask & 0x1)
                mask >>= 1
            return num
        #
        # End of class OpTable_aarch32.Decoder definition.
        #


    class CodeGen:
        #
        # The following structures are the heart of how this script
        # automatically generates decoding functions.  They must be
        # defined by hand.
        #

        symType = {
            None: None,
            'align': None,
            'amode': None,
            'amount': None,
            'banked_reg': None,
            'c': None,
            'const': 'Immediate',
            'coproc': None,
            'CRm': None,
            'CRn': None,
            'Dd': 'Register',
            'Ddm': 'Register',
            'Dd[x]': 'Register',
            'Dm': 'Register',
            'Dm[x]': 'Register',
            'Dn': 'Register',
            'Dn[x]': 'Register',
            'dreglist': None,
            'dt': None,
            'dt1': None,
            'dt2': None,
            'endian_specifier': None,
            'fbits': None,
            'iflags': None,
            'imm': None,
            'imm16': None,
            'imm4': None,
            'index': None,
            'label': 'Immediate',
            'list': None,
            'lsb': None,
            'mode': None,
            'opc1': None,
            'opc2': None,
            'option': None,
            'q': None,
            'Qd': 'Register',
            'Qm': 'Register',
            'Qn': 'Register',
            'Ra': 'Register',
            'Rd': 'Register',
            'RdHi': 'Register',
            'RdLo': 'Register',
            'registers': 'RegList',
            'registers_without_pc': 'RegList',
            'registers_with_pc': 'RegList',
            'Rm': 'Register',
            'Rn': 'Register',
            'Rs': 'Register',
            'Rt': 'Register',
            'Rt2': 'Register',
            'Sd': 'Register',
            'Sdm': 'Register',
            'shift': None,
            'single_register_list': 'SingleRegList',
            'size': None,
            'Sm': 'Register',
            'Sm1': 'Register',
            'Sn': 'Register',
            'spec_reg': None,
            'sreglist': None,
            'type': None,
            'width': None
        }

        symDeps = {
            'Dd': ['D:Vd'],
            'Ddm': ['D:Vd'],
            'Dd[x]': ['D:Vd'],
            'Dm': ['M:Vm'],
            'Dm[x]': ['M:Vm'],
            'Dn': ['N:Vn'],
            'Dn[x]': ['N:Vn'],
            'label': ['imm24:H', 'imm24',
                      'imm12:U', 'imm12',
                      'imm8:size', 'imm8',
                      'imm4H:imm4L'],
            'Qd': ['D:Vd'],
            'Qm': ['M:Vm'],
            'Qn': ['N:Vn'],
            'Ra': ['Ra'],
            'Rd': ['Rd'],
            'RdHi': ['RdHi'],
            'RdLo': ['RdLo'],
            'Rm': ['Rm'],
            'Rn': ['Rn'],
            'Rs': ['Rs'],
            'Rt': ['Rt'],
            'Rt2': ['Rt2', 'Rt'],
            'Sd': ['Vd:D'],
            'Sdm': ['Vd:D'],
            'Sm': ['Vm:M'],
            'Sm1': ['Vm:M'],
            'Sn': ['Vn:N'],
        }

        readRegisterSet = set(['Ddm', 'Dd[x]', 'Dm', 'Dm[x]', 'Dn', 'Dn[x]',
                               'Qm', 'Qn',
                               'Ra', 'RdHi', 'RdLo', 'Rm', 'Rn', 'Rs',
                               'Sdm', 'Sm', 'Sm1', 'Sn'])

        writeRegisterSet = set(['Ddm', 'Dd', 'Dd[x]',
                                'Qd',
                                'Rd', 'RdHi', 'RdLo', 'Rt', 'Rt2',
                                'Sd', 'Sdm'])

        def __init__(self, insnList):
            self.insnList = insnList
            self.insnOperands = collections.defaultdict(list)
            self.operandFunc = {}
            self.decoder = OpTable_aarch32.Decoder(insnList)

            self.generate_code()

        def add_operand_function(self, baseID, body):
            baseID = baseID.translate(invalidCharTbl)
            oprID = baseID
            count = 1
            while (oprID in self.operandFunc):
                if (self.operandFunc[oprID] == body):
                    return baseID

                count += 1
                oprID = baseID + '_' + str(count)

            self.operandFunc[oprID] = body
            return oprID

        def is_register_read(self, sym, insn):
            return (sym in self.readRegisterSet)

        def is_register_written(self, sym, insn):
            addrForm = insn.info.get('address-form')
            return ((sym in self.writeRegisterSet) or
                    (sym == 'Rn' and addrForm in set(['pre-indexed',
                                                      'post-indexed'])))

        def generate_variable(self, varName, fieldDesc, insn):
            parts = []
            position = 0

            for name in reversed(fieldDesc.split(':')):
                if (name in insn.fieldDict):
                    field = insn.fieldDict[name]
                    value = '(field<%2d, %2d>(rawInsn)' % (field.highBit,
                                                           field.lowBit)
                    length = field.highBit - field.lowBit + 1

                elif (name.count('1') + name.count('0') == len(name)):
                    if (name.count('1') > 0):
                        value += '%22s' % hex(int(name, 2) + 'U')
                    else:
                        value = None
                    length = len(name)

                if (value):
                    if (position):
                        parts.append(value + ' << %d)' % (position))
                    else:
                        parts.append(value + ')')
                position += length

            if (len(parts) == 1):
                bitManipCode = parts[0]
            else:
                bitManipCode = '  ' + (os.linesep + '| ').join(parts)

            code = '''
int %(varName)s = (
%(bitManipCode)s
);
            ''' % {
                'varName'      : varName,
                'bitManipCode' : indent_code(2, bitManipCode),
            }

            return code.strip()

        def generate_register_operand_function(self, sym, insn):
            deps      = self.get_dependencies(sym, insn)
            idxVar    = self.generate_variable('idx', deps, insn)
            isRead    = self.is_register_read(sym, insn)
            isWritten = self.is_register_written(sym, insn)

            # Special case: symbol <Rn>
            if (sym == 'Rn'):
                addrForm = insn.info.get('address-form')
                if (addrForm in set(['pre-indexed', 'post-indexed'])):
                    isWritten = True

            funcBody = '''
int base = aarch32::%(regBase)s0.val();
%(idxVariable)s

Expression::Ptr regExpr = makeRegisterExpression(MachRegister(base + idx));
decodedInsn->appendOperand(regExpr, %(read)s, %(written)s);
            ''' % {
                'regBase'     : sym[0].lower(),
                'idxVariable' : idxVar,
                'read'        : 'true' if isRead else 'false',
                'written'     : 'true' if isWritten else 'false',
            }

            # Special case: symbol <Sm1>
            if (sym == 'Sm1'):
                funcBody = funcBody.replace('(base + idx)', '(base + idx + 1)')

            oprID = self.add_operand_function('reg_' + sym, funcBody.strip())
            self.insnOperands[insn.id].append(oprID)

        def generate_reglist_operand_function(self, sym, insn):
            if ('register_list' in insn.fieldDict):
                if ('Rn' not in insn.fieldDict):
                    error(insn.id, 'contains register_list field, but not Rn')

                regListVar  = self.generate_variable('regList',
                                                     'register_list', insn);
                idxVar      = self.generate_variable('idx', 'Rn', insn);
                field       = insn.fieldDict['register_list']
                fieldWidth  = field.highBit - field.lowBit + 1
                isRead      = insn.info['mnemonic'].startswith('ST')
                isWritten   = insn.info['mnemonic'].startswith('LD')

                funcBody = '''
%(regListVariable)s
%(idxVariable)s

/*
MachRegister baseReg(aarch32::r0.val() + idx);
int off = 0;
for (int i = 0; i < %(fieldWidth)d; ++i) {
    int mask = 0x1 << i;
    if ( !(regList & mask))
        continue;

    Expression::Ptr regExpr   = makeRegisterExpression(baseReg);
    Expression::Ptr offExpr   = Immediate::makeImmediate(Result(s32, off));
    Expression::Ptr addrExpr  = makeAddExpression(regExpr, offExpr, u32);
    Expression::Ptr derefExpr = makeDereferenceExpression(addrExpr, s32);
    decodedInsn->appendOperand(derefExpr, !%(listRead)s, !%(listWritten)s);
    off += 4;
}
*/

for (int i = 0; i < %(fieldWidth)d; ++i) {
    int mask = 0x1 << i;
    if ( !(regList & mask))
        continue;

    MachRegister regID(aarch32::r0.val() + i);
    Expression::Ptr listRegExpr = makeRegisterExpression(regID);
    decodedInsn->appendOperand(listRegExpr, %(listRead)s, %(listWritten)s);
}
                ''' % {
                    'fieldWidth'      : fieldWidth,
                    'idxVariable'     : idxVar,
                    'listRead'        : 'true' if isRead else 'false',
                    'listWritten'     : 'true' if isWritten else 'false',
                    'regListVariable' : regListVar,
                }


            oprID = self.add_operand_function('reglist', funcBody.strip())
            self.insnOperands[insn.id].append(oprID)

        def generate_label_operand_code(self, insn):
            if ('imm24' in insn.fieldDict):
                if ('H' in insn.fieldDict):
                    immDesc = 'imm24:H:0'
                    finalizer = 'bool isCall = true;'
                else:
                    immDesc = 'imm24:00'
                    finalizer = 'bool isCall = field<24, 24>(rawInsn);'

                finalizer += '''
imm = SignExtend(imm, 24+2);
Expression::Ptr regPC    = make_pc_expr();
Expression::Ptr offset   = Immediate::makeImmediate(Result(s32, imm));
Expression::Ptr targAddr = makeAddExpression(regPC, offset, u32);
handle_branch_target(targAddr, isCall, false);
                '''

            elif ('imm12' in insn.fieldDict):
                immDesc = 'imm12'
                if ('U' in insn.fieldDict or 'imm4' in insn.fieldDict):
                    finalizer = '// imm = ZeroExtend(imm, 12);'
                else:
                    finalizer = 'imm = A32ExpandImm(imm);'

                finalizer += '''
Expression::Ptr regPC   = make_pc_expr();
Expression::Ptr offset  = Immediate::makeImmediate(Result(s32, imm));
Expression::Ptr memAddr = makeAddExpression(regPC, offset, u32);
decodedInsn->appendOperand(memAddr, true, false);
                '''

            elif ('imm8' in insn.fieldDict):
                immDesc = 'imm8:00'
                if ('size' in insn.fieldDict):
                    sizeVar   = self.generate_variable('size', 'size', insn)
                    finalizer = '''
%s
if (size == 0x1) {
    imm >>= 1;
}
Expression::Ptr regPC   = make_pc_expr();
Expression::Ptr offset  = Immediate::makeImmediate(Result(s32, imm));
Expression::Ptr memAddr = makeAddExpression(regPC, offset, u32);
decodedInsn->appendOperand(memAddr, true, false);
                    ''' % (sizeVar)
                else:
                    finalizer = '// imm = ZeroExtend(imm, 8+2);'

            elif ('imm4H' in insn.fieldDict and 'imm4L' in insn.fieldDict):
                immDesc   = 'imm4H:imm4L'
                finalizer = '// imm = ZeroExtend(imm, 8);'
            else:
                error('Cannot handle <label> oprand for', insn.id)

            immCode = self.generate_variable('imm', immDesc, insn)
            funcCode = '''
%(immVar)s
%(finalizer)s

            ''' % {
                'idstr':insn.id,
                'immVar'    : immCode,
                'finalizer' : finalizer.strip(),
            }
            return funcCode.strip()

        def generate_const_operand_code(self, insn):
            if ('imm12' in insn.fieldDict):
                immDesc = 'imm12'
                finalizer = 'imm = A32ExpandImm(imm);'

                finalizer += '''
Expression::Ptr immExpr  = Immediate::makeImmediate(Result(s32, imm));
decodedInsn->appendOperand(immExpr, true, false);
                '''

            else:
                error('Cannot handle <const> operand for', insn.id)

            immCode = self.generate_variable('imm', immDesc, insn)
            funcCode = '''
%(immVar)s
%(finalizer)s

            ''' % {
                'idstr':insn.id,
                'immVar'    : immCode,
                'finalizer' : finalizer.strip(),
            }
            return funcCode.strip()

        def generate_immediate_operand_function(self, sym, insn):
            if (sym == 'label'):
                funcBody = self.generate_label_operand_code(insn)

            elif (sym == 'const'):
                funcBody = self.generate_const_operand_code(insn)

            oprID = self.add_operand_function('imm_' + sym, funcBody.strip())
            self.insnOperands[insn.id].append(oprID)

        def get_dependencies(self, sym, insn):
            if (sym not in self.symDeps):
                return None

            fieldSet = set(insn.fieldDict)
            for dep in self.symDeps[sym]:
                if (fieldSet.issuperset(dep.split(':'))):
                    return dep

            error('No dependencies for symbol <' + sym +
                  '> matched instruction', insn.id, "::", fieldSet)

        def generate_code(self):
            self.headerCode = {}
            self.sourceCode = {}
            for insn in self.insnList:
                for sym in insn.symList:
                    if sym not in self.symType:
                        error('Cannot handle untyped symbol <' + sym + '>')

                    if self.symType[sym] == 'Register':
                        self.generate_register_operand_function(sym, insn)

                    elif self.symType[sym] == 'RegList':
                        self.generate_reglist_operand_function(sym, insn)

                    elif self.symType[sym] == 'SingleRegList':
                        self.generate_register_operand_function('Rn', insn)

                    elif self.symType[sym] == 'Immediate':
                        self.generate_immediate_operand_function(sym, insn)

        def find_operand_max_length(self):
            return max(len(x) for x in self.insnOperands.values())

        def join_opcode_ids(self):
            ids = ['INVALID'] + sorted(x.id for x in self.insnList)
            ids = ['  aarch32_op_%s,' % (x) for x in ids]
            return os.linesep.join(ids)

        def join_operand_ids(self):
            oprIds = ['empty'] + sorted(self.operandFunc)
            oprIds = ['    OPR_' + x for x in oprIds]
            return (',' + os.linesep).join(oprIds)

        def join_operand_function_headers(self):
            funcTmpl = 'void OPRfunc_%s(void);'
            funcHdrs = [funcTmpl % (x) for x in sorted(self.operandFunc)]
            return os.linesep.join(funcHdrs)

        def join_operand_case_statements(self):
            caseTmpl  = '      case OPR_%-15s: return OPRfunc_%s();'
            caseStmts = [caseTmpl % (x, x) for x in sorted(self.operandFunc)]
            caseHead  = '      case OPR_empty          : return;'
            caseTail  = '      default                 : assert(0);'
            return os.linesep.join([caseHead] + caseStmts + [caseTail])

        def join_operand_function_source(self):
            funcTmpl = '''
void InstructionDecoder_aarch32::OPRfunc_%s()
{
%s
}
            '''
            allFuncs = [funcTmpl.strip() % (key, indent_code(4, value))
                        for (key, value) in sorted(self.operandFunc.items())]
            return (os.linesep * 2).join(allFuncs)

        def join_instruction_operands(self, insn):
            oprList = ['OPR_' + x for x in self.insnOperands[insn.id]]
            return '{' + ', '.join(oprList) + '}'

        def make_mask_table(self):
            entryTmpl = '    {0x%(mask)08x, %(valueIdx)4d, %(length)3d},'
            entries = [entryTmpl % x.__dict__ for x in self.decoder.maskTable]
            return os.linesep.join(entries)

        def make_value_table(self):
            entryTmpl = '    {0x%(value)08x, %(maskIdx)4d},'
            entries = [entryTmpl % x.__dict__ for x in self.decoder.valueTable]
            return os.linesep.join(entries)

        def make_instruction_table(self):
            code = []
            for insn in self.insnList:
                mnemonic = insn.info.get('alias_mnemonic',
                                         insn.info['mnemonic'])
                entry = '    {aarch32_op_%s, "%s", %s, 0x%08x, 0x%08x}' % (
                    insn.id,
                    mnemonic.lower(), # + ' ' + insn.id,
                    self.join_instruction_operands(insn),
                    int(insn.bitList),
                    int(insn.bitList.get_mask())
                )
                code.append(entry)

            return (',' + os.linesep).join(code)

        def generate_opcode_enum(self):
            content = '''
// Begin auto-generated opcodes
%(opcodes)s
// End auto-generated opcodes
            ''' % {
                'opcodes' : self.join_opcode_ids(),
            }
            return content.strip()

        def generate_decode_header(self):
            content   = '''
Expression::Ptr make_pc_expr(void);
void handle_branch_target(Expression::Ptr targExpr,
                          bool isCall,
                          bool isIndirect);
void handle_operand(int oprID);
%(oprFuncHdrs)s
            ''' % {
                'oprFuncHdrs' : self.join_operand_function_headers(),
            }
            return content.strip()

        def generate_decode_source(self):
            content   = '''
/*
 * This file was automatically generated.  Any manual edits will be lost.
 */

#ifdef  __AARCH32_ISA_GEN_SRC__
#error Generated source file included multiple times.
#endif

#define __AARCH32_ISA_GEN_SRC__

namespace Dyninst {
namespace InstructionAPI {

typedef enum Operand_t {
%(oprIds)s
} Operand_t;

#define MAX_OPR_COUNT %(oprMax)d

typedef enum Condition_t {
    COND_eq = 0x0, // Z == 1
    COND_ne = 0x1, // Z == 0
    COND_cs = 0x2, // C == 1
    COND_cc = 0x3, // C == 0
    COND_mi = 0x4, // N == 1
    COND_pl = 0x5, // N == 0
    COND_vs = 0x6, // V == 1
    COND_vc = 0x7, // V == 0
    COND_hi = 0x8, // C == 1 and Z == 0
    COND_ls = 0x9, // C == 0 or  Z == 1
    COND_ge = 0xa, // N == V
    COND_lt = 0xb, // N != V
    COND_gt = 0xc, // Z == 0 and N == V
    COND_le = 0xd, // Z == 1 or N != V
    COND_al = 0xe, // Always true
    COND_xx = 0xf, // Not a conditional instruction
} Condition_t;

static const char insnSuffix[][3] = {
    "eq",
    "ne",
    "cs",
    "cc",
    "mi",
    "pl",
    "vs",
    "vc",
    "hi",
    "ls",
    "ge",
    "lt",
    "gt",
    "le",
    "",
    "??"
};

typedef struct Insn_Entry {
    entryID op;
    const char* mnemonic;
    Operand_t operand[MAX_OPR_COUNT];
    uint32_t code;
    uint32_t mask;
} Insn_Entry_t;

static Insn_Entry_t insnTable[] = {
%(insnTable)s
};

static Insn_Entry_t* INVALID_INSN = &insnTable[0];

typedef struct Mask_Entry {
    uint32_t mask;
    int valueIdx;
    int length;
} Mask_Entry_t;

static Mask_Entry_t maskTable[] = {
%(maskTable)s
};

typedef struct Value_Entry {
    uint32_t value;
    int maskIdx;
} Value_Entry_t;

static Value_Entry_t valueTable[] = {
%(valueTable)s
};

#ifdef DEBUG_AARCH32_DECODE

static const char* bit_rep[] = {
    "0000", "0001", "0010", "0011", "0100", "0101", "0110", "0111",
    "1000", "1001", "1010", "1011", "1100", "1101", "1110", "1111"
};

static void print_bin(FILE* fp, uint32_t insn)
{
    fprintf(fp, "%%s %%s %%s %%s %%s %%s %%s %%s",
           bit_rep[(insn >> 0x1c) & 0xf], bit_rep[(insn >> 0x18) & 0xf],
           bit_rep[(insn >> 0x14) & 0xf], bit_rep[(insn >> 0x10) & 0xf],
           bit_rep[(insn >> 0x0c) & 0xf], bit_rep[(insn >> 0x08) & 0xf],
           bit_rep[(insn >> 0x04) & 0xf], bit_rep[(insn >> 0x00) & 0xf]);
}

static void identify_trace(uint32_t rawInsn)
{
    int idx = 0, len, i;

    fprintf(stderr, "===========================\\n");
    fprintf(stderr, "Begin decode for %%08x:\\n", rawInsn);
    while (maskTable[idx].mask != 0x0) {
        uint32_t mask  = maskTable[idx].mask;
        uint32_t value = rawInsn & mask;

        print_bin(stderr, rawInsn);
        fprintf(stderr, ": raw instruction\\n");
        print_bin(stderr, maskTable[idx].mask);
        fprintf(stderr, ": Mask table -- entry #%%d\\n", idx);

        len = maskTable[idx].length;
        idx = maskTable[idx].valueIdx;

        for (i = 0; i < len; ++i) {
            print_bin(stderr, valueTable[idx + i].value);
            if (value == valueTable[idx + i].value) {
                fprintf(stderr, ": Match at value #%%d\\n", i);
                idx = valueTable[idx + i].maskIdx;
                break;
            }
            fprintf(stderr, ": Value #%%d does not match\\n", i);
        }
        if (i >= len) {
            fprintf(stderr, "Decode failure: No value found for mask.\\n");
            return;
        }
    }

    len = maskTable[idx].length;
    for (i = 0; i < len; ++i) {
        int insnIdx = maskTable[idx + i].valueIdx;
        uint32_t encoding = rawInsn & insnTable[insnIdx].mask;

        print_bin(stderr, insnTable[insnIdx].mask);
        fprintf(stderr, ": %%s mask\\n", insnTable[insnIdx].mnemonic);
        print_bin(stderr, insnTable[insnIdx].code);
        fprintf(stderr, ": %%s encoding\\n", insnTable[insnIdx].mnemonic);
        print_bin(stderr, encoding);
        if (encoding == insnTable[insnIdx].code) {
            idx = insnIdx;
            fprintf(stderr, ": Matched candidate #%%d (insnTable[%%d]).\\n",
                    i, insnIdx);
            break;
        }
        fprintf(stderr, ": Candidate #%%d (insnTable[%%d]) does not match.\\n",
                i, insnIdx);
    }
    if (i >= len)
        fprintf(stderr, "Decode failure: No instruction matched.\\n");
}
#endif

//
// Static functions for use in this file only.
//

static Insn_Entry_t* identify(uint32_t rawInsn)
{
    int idx = 0, len, i;

    while (maskTable[idx].mask != 0x0) {
        uint32_t value = rawInsn & maskTable[idx].mask;
        len = maskTable[idx].length;
        idx = maskTable[idx].valueIdx;

        for (i = 0; i < len; ++i) {
            if (value == valueTable[idx + i].value) {
                idx = valueTable[idx + i].maskIdx;
                break;
            }
        }
        if (i >= len) {
#if DEBUG_AARCH32_DECODE
            identify_trace(rawInsn);
#endif
            return INVALID_INSN;
        }
    }

    len = maskTable[idx].length;
    for (i = 0; i < len; ++i) {
        int insnIdx = maskTable[idx + i].valueIdx;
        uint32_t encoding = rawInsn & insnTable[insnIdx].mask;
        if (encoding == insnTable[insnIdx].code) {
            idx = insnIdx;
            break;
        }
    }
    if (i >= len) {
#if DEBUG_AARCH32_DECODE
        identify_trace(rawInsn);
#endif
        return INVALID_INSN;
    }

    return insnTable + idx;
}

static int SignExtend(int imm, int signBit)
{
    int mask  = ~0U >> (31 - signBit);
    int value = imm & mask;

    mask = 1 << (signBit - 1);
    if (imm & mask)
        value |= ~0U << signBit;

    return value;
}

static int A32ExpandImm(int imm)
{
    int value  = imm & 0xFF;
    int rotate = (imm & 0xF00) >> 7;

    return (value >> rotate) | (value << (32-rotate));
}

template<int highBit, int lowBit>
static int field(unsigned int raw)
{
    return (raw >> lowBit) & ~(~0UL << ((highBit - lowBit) + 1));
}

static Condition_t get_condition_field(uint32_t rawInsn)
{
    return static_cast<Condition_t>(field<31, 28>(rawInsn));
}

//
// Class InstructionDecoder_aarch32 member function implementations.
//

Expression::Ptr InstructionDecoder_aarch32::make_pc_expr()
{
    return makeRegisterExpression(MachRegister(aarch32::pc));
}

void InstructionDecoder_aarch32::handle_branch_target(Expression::Ptr targAddr,
                                                      bool isCall,
                                                      bool isIndirect)
{
    Condition_t condField = get_condition_field(rawInsn);
    bool isConditional = (condField == COND_al || condField == COND_xx);

    decodedInsn->addSuccessor(targAddr,
                              isCall,
                              isIndirect,
                              isConditional,
                              false // isFallthrough
                             );

    if (isCall || isConditional) {
        Expression::Ptr regPC = make_pc_expr();
        Expression::Ptr insnSize = Immediate::makeImmediate(Result(s32, 4));
        decodedInsn->addSuccessor(makeAddExpression(regPC, insnSize, u32),
                                  false, // isCall
                                  false, // isIndirect
                                  false, // isConditional
                                  true   // isFallthrough
                                 );
    }
}

void InstructionDecoder_aarch32::handle_operand(int oprID)
{
    switch ((Operand_t)oprID) {
%(oprCaseStmts)s
    }
}

%(oprFuncs)s

}} // End namespace Dyninst::InstructionAPI
            ''' % {
                'insnTable'    : self.make_instruction_table(),
                'maskTable'    : self.make_mask_table(),
                'oprCaseStmts' : self.join_operand_case_statements(),
                'oprFuncs'     : self.join_operand_function_source(),
                'oprIds'       : self.join_operand_ids(),
                'oprMax'       : self.find_operand_max_length() + 1,
                'valueTable'   : self.make_value_table(),
            }

            return content.strip()
        #
        # End of class OpTable_aarch32.CodeGen definition.
        #


    # Import individual instruction specification XML files.
    #
    def __init__(self, opcodes):
        self.insnList = []

        #invalidInsn = OpTable_aarch32.Instruction(rawValue=0x18000000)
        #self.insnList.append(invalidInsn)

        for ifile in sorted(opcodes.ifiles):
            # print 'Opening ', ifile
            doc = xml.etree.ElementTree.parse(ISA_dir + ifile)

            symbol = {}
            for explain in doc.iter('explanation'):
                text = ''.join(x.text for x in explain.iter('intro'))
                for encoding in explain.get('enclist').split(', '):
                    sym = explain.find('symbol').text
                    if encoding not in symbol:
                        symbol[encoding] = {}

                    symbol[encoding][sym] = text

            for iclass in doc.iter('iclass'):
                isa = iclass.get('isa')

                if (isa != 'A32'):
                    continue

                baseInsn = OpTable_aarch32.Instruction()
                baseInsn.import_docvars(iclass)
                baseInsn.import_boxes(iclass.find('regdiagram'))
                pstext = iclass.find('.//ps_section//pstext')

                for encoding in iclass.findall('encoding'):
                    thisInsn = copy.deepcopy(baseInsn)

                    thisInsn.id = encoding.get('name')
                    thisInsn.import_docvars(encoding)
                    thisInsn.import_asmtemplate(encoding.find('asmtemplate'))
                    thisInsn.import_pstext(pstext)
                    thisInsn.update_boxes(encoding)
                    if thisInsn.id not in symbol:
                        print '[DEBUG]:', thisInsn.id, 'missing. Choices are:', list(symbol)
                        thisInsn.explanation = None
                    else:
                        thisInsn.explanation = symbol[thisInsn.id]

                    print '*** ', ifile, isa, thisInsn.id
                    print 'field names:', [str(x) for x in thisInsn.fieldDict.values() if x.name]
                    print '  box names:', [x.get('name') for x
                                          in encoding.findall('box')
                                          if x.get('name')]

                    asmtemplate = encoding.find('asmtemplate')
                    alltext = [ x.text for x in asmtemplate.iter() ]
                    print 'asmtemplate:', [ x.strip('<>') for x in alltext
                                            if x
                                            and x.startswith('<')
                                            and x.endswith('>') ]
                    print 'explanation:', thisInsn.explanation

                    for exp in doc.findall('.//explanation'):
                        alltext = ' '.join(exp.itertext())
                        angles   = re.findall('<.*?>', alltext)
                        matches = re.findall('\w+ \w+ \w+ ".*?" \w+', alltext)

                        if angles:
                            name = angles[0]
                        else:
                            name = "<???>"

                        print 'XXX ', name, " ", matches, ' {', ' '.join(thisInsn.fieldDict), '}'

                    global allDefs
                    for sym in thisInsn.symList:
                        for field in thisInsn.fieldDict.values():
                            if not field.name: continue
                            if field.name != sym: continue

                            if sym not in allDefs:
                                allDefs[sym] = {}
                            if field.name not in allDefs[sym]:
                                allDefs[sym][field.name] = set()

                            allDefs[sym][field.name].add(str(field))

                    self.insnList.append(thisInsn)

                immfuncs = set()
                pstext = iclass.findall('.//ps_section//pstext')
                if len(pstext) == 0:
                    print '   immfuncs: [No <pstext>]'
                else:
                    immfunc = re.compile('\S+\([^)]*imm.*?\)')
                    for pstext in iclass.findall('.//ps_section//pstext'):
                        pscode = ''.join(pstext.itertext())

                    pscode = re.sub(r'NOT\((.*?)\)',
                                    r'NOT_\1', pscode)
                    pscode = re.sub(r'(\w+)<(\d+)>',
                                    r'\1_bits_\2_\2', pscode)
                    pscode = re.sub(r'(\w+)<(\d+):(\d+)>',
                                    r'\1_bits_\2_\3', pscode)
#                    pscode = re.sub(r'((\w+:)+\w+)', 'concat(\1)', pscode)
#                    pscode = re.sub(r'(\w+):', r'\1_', pscode)
                    concat  = re.compile('\w*:\S*')
#                    print '     concat:', re.findall(r'concat\(.*?\)', pscode)
                    print '     concat:', re.findall(r'(?:\w+:)+\w+', pscode)

                    print '   immfuncs:', re.findall(r'\S+\([^)]*imm.*?\)', pscode)
                    print '   allfuncs:', re.findall('\w+\(.*?\)', pscode)
            '''
            for exp in doc.findall('.//explanation'):
                alltext = ' '.join(exp.itertext())
                print 'XXX', ' '.join(alltext.split()), ' {'
            '''
            symSet = set()
            symList = []
            for sym in doc.iter('symbol'):
                if sym.text not in symSet:
                    symSet.add(sym.text)
                    symList.append(sym.text)
            print '       SYMS:', symList

        global masksArray
        masksArray = self.get_masks()

        global encodingsArray
        encodingsArray = self.get_encodings()

        print '***'
        cg = OpTable_aarch32.CodeGen(self.insnList)

        f = open('aarch32_opcode_autogen.h', 'w')
        f.write(cg.generate_opcode_enum() + os.linesep)
        f.close()

        f = open('aarch32_decoder_autogen.h', 'w')
        f.write(cg.generate_decode_header() + os.linesep)
        f.close()

        f = open('aarch32_decoder_autogen.C', 'w')
        f.write(cg.generate_decode_source() + os.linesep)
        f.close()
        print '***'

    ####################################
    # generate the c++ code
    # which builds the instruction table
    ####################################
    def generate_code(self):
        code = list()
        for insn in self.insnList:
            global archName
            code.append('main_insn_table.push_back(%s_insn_entry('
                        '%s_op_%s, "%s", %s, 0x%08x, 0x%08x);' % (
                            archName, archName,
                            insn.id, insn.info['mnemonic'].lower(),
                            insn.get_operand_code(),
                            int(insn.bitList), int(insn.bitList.get_mask())))
        return code

    def get_instructions(self):
        return [x.id for x in self.insnList]

    def get_masks(self):
        return [int(x.bitList.get_mask()) for x in self.insnList]

    def get_encodings(self):
        return [int(x.bitList) for x in self.insnList]

    def get_operands(self):
        operandSet = set()

        for insn in self.insnList:
            operandSet.update(insn.symList)

        return sorted(operandSet)


class OpTable_aarch64:
    global files_dir

    def __init__(self):
        self.masksArray = list()
        self.encodingsArray = list()
        self.insnArray = list()
        self.operandsArray = list()
        self.operandsSet = set()
        self.insn_unallocated = (2**28+2**27)

        global masksArray
        masksArray = self.get_masks()

        global encodingsArray
        encodingsArray = self.get_encodings()

    def getTable(self):
        return self.masksArray, self.encodingsArray, self.insnArray, self.operandsArray

    def isRnUpdated(self, line):
        if line.find('post-idx') != -1 or line.find('pre-idx') !=-1:
            return True;
        return False;

    def ifNeedToSetFlags(self, line):
        if line.find('<iclass ') != -1 :
            for field in line.split(' '):
                if field.find('id=') !=-1:
                    if field.split('\"')[1] == 's':
                        return True

        if line.find('<regdiagram') != -1:
            if line.find('float/compare') != -1:
                return True
        return False

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
                            needToSetFlags = self.ifNeedToSetFlags(line)

                        # diagram starts #
                        if line.find('<regdiagram')!=-1:
                            isRnUp = self.isRnUpdated(line)
                            if needToSetFlags == False:
                                needToSetFlags = self.ifNeedToSetFlags(line)
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

            global archName
            #print '\tmain_insn_table.push_back('+archName+'_insn_entry('+archName+'_op_'+ self.insnArray[i]+', \t\"'+ self.insnArray[i].split('_')[0]+'\",\t'+ operands +' ));'
            print '\tmain_insn_table.push_back('+archName+'_insn_entry('+archName+'_op_'+ self.insnArray[i]+', \t\"'+ self.insnArray[i].split('_')[0]+'\",\t'+ operands +', ' \
                + str(self.encodingsArray[i]) + ', ' + str(self.masksArray[i]) + ') );'

    # The following functions provide a common interface for
    # OpTable_aarchXX classes.  Eventually, these should be accessed
    # directly as common member variables.

    def get_instructions(self):
        return self.insnArray

    def get_masks(self):
        return self.masksArray

    def get_encodings(self):
        return self.encodingsArray

    def get_operands(self):
        return list(self.operandsSet)

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

    global archName
    print '\tmain_decoder_table['+str(entryToPlace)+']='+archName+'_mask_entry('+str(hex(curMask))+', '+entries+','+str(index)+');'

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

    global archName
    print '\tmain_decoder_table['+str(entryToPlace)+']='+archName+'_mask_entry('+str(hex(curMask))+', '+entries+', list_of'+ index_list+');'

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

class DeocdeTable:
    ##########################
    # generate decoding tree #
    ##########################
    def __init__(self, opTable):
        self.maskList = opTable.get_masks()
        self.encodingList = opTable.get_encodings()
        self.insnList = opTable.get_instructions()
        self.operandList = opTable.get_operands()

        self.numOfLeafNodes = 0
        self.processedIndex = set()
        self.numNodes = 0
        ####################################
        # this is the init value that the instruction
        # entry should start from. 0 is reserved for
        # invalid insn.
        ####################################
        self.entryAvailable = 1
        self.debug = False
        self.aliasWeakSolution = True

        self.build_decode_table(range(0, len(self.maskList)), 0, 0)

    # weak solution to aliases
    def alias_weakSolution(self, inInsnIndex, entryToPlace):
        inInsnIndex.sort( cmp=alias_comparator )
        for i in inInsnIndex:
            self.processedIndex.add(i)
            #if self.debug == True:
            #print self.insnList[i], '\t', bin( self.maskList[i] ), '\t', bin(self.encodingList[i])

        printDecodertable(entryToPlace, 0, list(), inInsnIndex[0]);

    # strict solution to aliases
    def alias_strictSolution(self, inInsnIndex, entryToPlace):
        inInsnIndex.sort( cmp=alias_comparatorRev )

        # for debuggging
        for i in inInsnIndex:
            self.processedIndex.add(i)
            #if self.debug == True:
            print self.insnList[i], '\t', bin( self.maskList[i] ), '\t', bin(self.encodingList[i])

        printDecodertable_list(entryToPlace, 0, list(), inInsnIndex);



    ###########################################
    # arg-1 is self
    # arg0 is the range of indexes in the instruction array
    #   that you want to analyze
    # arg1 is the bit mask that has been processed
    # arg2 is the entry of the decoding table
    #   where the current call should place for one decision node
    ###########################################
    def build_decode_table(self, inInsnIndex , processedMask, entryToPlace):

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
                print self.insnList[inInsnIndex[0]]
            if inInsnIndex[0] in self.processedIndex:
                print '[WARN] index processed, repeated index is ', inInsnIndex[0]

            self.processedIndex.add(inInsnIndex[0])
            self.numOfLeafNodes +=1
            return

        validMaskBits = int(''.join('1'*32), 2)

        # find the minimum common mask bit field
        for i in inInsnIndex:
            validMaskBits = validMaskBits & self.maskList[i]
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
                #print '%3d'%i+'%22s'%self.insnList[i]+'%34s'%bin(self.maskList[i])+'%34s'%bin(self.encodingList[i])
                addMask |= self.maskList[i] &(~processedMask)
                #addMask ^= self.encodingList[i] &(~processedMask)

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
                print '%3d'%i+'%22s'%self.insnList[i]+'%35s'%bin(self.maskList[i])+'%35s'%bin(self.encodingList[i])
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
            branchNo = clapseMask(self.encodingList[index] & curMask, curMask)
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
            print '%34s'%bin(self.encodingList[i])

        for i in oneIndexes:
            print '%34s'%bin(self.encodingList[i])
        """
        if self.debug == True:
            print validIndexList
        for i in range(0, numBranch):
            self.build_decode_table( validIndexList[i], processedMask, entryList[i][1])

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
            if token.find('hibit=')!=-1:
                bit = int(token.split('\"')[1])
            if token.find('width=') != -1:
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
        stmt = 'void OPR' + operand + '(){ }'
        print stmt.translate(invalidCharTbl)


def main():

    ######################################
    # get opcodes of GP instructions
    # and FP/VEC instructions respectively
    ######################################
    opcodes = Opcode(ISA_dir)
    global insn_set
    global fp_insn_set
    insn_set = opcodes.get_insn_set()
    fp_insn_set = opcodes.get_fp_insn_set()

    #################################################################
    # Get opcode (instruction) list
    #
    # Each instruction contains information such as mnemonic, opcode
    # bits, operands, read/write sets, etc.
    #################################################################
    if (archName == 'aarch32'):
        opTable = OpTable_aarch32(opcodes)

        print "Operand/Field Conflicts:"
        for x in sorted(allDefs):
            for y in sorted(allDefs[x]):
                deps = sorted(allDefs[x][y])
                #if len(deps) > 1:
                print x + ' depends on: ' + str(deps)
        print

        print('enum {')
        print('  ' + archName + '_op_INVALID,')
        print('  ' + archName + '_op_extended,')
        for insn in sorted([x.id for x in opTable.insnList]):
            if insn == 'INVALID': continue
            print('  ' + archName + '_op_' + insn + ',')
        print('}')

        print 'number of instructions: ', len(opTable.insnList)
        for (idx, insn) in enumerate(opTable.insnList):
            print("%3d %s" % (idx, insn))

        ###############################################
        # instruction table
        # generate C++ code to build instruction tables
        ###############################################
        print(len(opTable.insnList))
        print '*** instruction table ***'
        for line in opTable.generate_code():
            print('    ' + line)

    elif (archName == 'aarch64'):
        opcodes.printOpcodes()
        opTable = OpTable_aarch64()
        opTable.printOpTable()
        ###############################################
        # instruction table
        # generate C++ code to build instruction tables
        ###############################################
        opTable.buildInsnTable()

    #########################################
    # generate C++ code of operands functions
    #########################################
    print '*** operands ***'
    operandList = opTable.get_operands()
    print sorted(operandList)
    printOperandFuncs(operandList)

    ###########################################
    # Decoder table.
    # Generate C++ code to build decoding table.
    # Basically, the generated stuff is a decision tree, which is
    # represented in a map.
    #
    # Each entry stands for a decision node in the decision tree with
    # a few or zero branches. One branch contains a {Key, Value}
    # pair. Key is used to index. Value is the next entry number in
    # the map, like a pointer.
    #
    # Binaries are passed to the root node first, and compared with
    # the decision mask in that node. Instruction binary does bit
    # operation AND over the mask. The result of it will be compared
    # with the Key of the branches. Once matched to the Key, the
    # instruction will be passed to next decision node pointed by the
    # branch. Recursively do this, until to the leaf node.
    #
    # Each leaf node contains the index of the instruction array in
    # the last field, which holds the instruction entry.
    ###########################################
    if (archName != 'aarch32'):
        print '*** decoder table ***'
        decodeTable = DecodeTable(opTable)

        ###############################
        # some statistics for debugging
        ###############################
        print 'num of vaild leaves', decodeTable.numOfLeafNodes
        allIndex = set(range(0, len(opTable.get_instructions())) )
        print 'processed indexex: ', decodeTable.processedIndex, len(decodeTable.processedIndex)
        print 'missing indexes:', sorted(allIndex - decodeTable.processedIndex), len(allIndex-decodeTable.processedIndex)
        print 'number of total nodes in the tree:', decodeTable.numNodes

if __name__ == '__main__':
    main()
