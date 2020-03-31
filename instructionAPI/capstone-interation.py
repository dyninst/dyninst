import sys
import argparse
from subprocess import *

def getParameters():
    parser = argparse.ArgumentParser(description='Translating Capstone instruction data structures to Dyninst instruction data structures')
    parser.add_argument("--capstone_header", type=str, default="/home/xm13/capstone/install/include/capstone/x86.h")
    parser.add_argument("--dyninst_header", type=str, default="/home/xm13/dyninstapi/dyninst/common/h/entryIDs.h")
    parser.add_argument("--arch", type=str, default="x86")
    parser.add_argument("--outputfile", type=str, default="output.txt")
    args = parser.parse_args()
    return args

def RunGrep(op):
    dyninst_root = args.dyninst_header[:args.dyninst_header.find("common")]
    cmd = "cd {0}; grep {1} * -r".format(dyninst_root, op)
    p = Popen(cmd, shell=True, stdout=PIPE, stderr=PIPE)
    msg, err = p.communicate()
    if (len(err) > 0):
        print "Error message in running", cmd,":"
        print err
    return msg

class Translator():
    def __init__(self):
        self.arch = args.arch
        self.capstone = args.capstone_header
        self.dyninst = args.dyninst_header
        self.output_file = open(args.outputfile, "w")

        self.aliasMap = {}
        self.aliasMap["ja"] = "jnbe"
        self.aliasMap["jae"] = "jnb"
        self.aliasMap["je"] = "jz"
        self.aliasMap["jne"] = "jnz"
        self.aliasMap["jg"] = "jnle"
        self.aliasMap["jge"] = "jnl"
        self.aliasMap["ret"] = "ret_near"

    def CapstoneArchPrefix(self):
        # Capstone's opcode is in the format of
        # $(arch)_INS_$(opcode)
        if self.arch == "x86": 
            return "X86"
        elif self.arch == "power": 
            return "PPC"
        elif self.arch == "arm":
            return "ARM64"
        else:
            print "Not supported arch"
            return ""

    def AnalyzeCapstoneHeader(self):
        # We first extract Capstone's opcodes
        self.cap_opcode = []
        capstone_prefix = self.CapstoneArchPrefix()
        # Capstone's opcode enum for each architecture
        # starts with INVALID and ENDING. This makes it convenient to 
        # find the actual opcodes.
        cap_opcode_start = "{0}_INS_INVALID".format(capstone_prefix)
        cap_opcode_end = "{0}_INS_ENDING".format(capstone_prefix)
        capstone_header = open(self.capstone, "r").read()
        if capstone_header == "":
            return False
        start_index = capstone_header.find(cap_opcode_start) 
        if start_index == -1:
            print "Do not find capstone opcode enum start {0}".format(cap_opcode_start)
            return False
        end_index = capstone_header.find(cap_opcode_end) 
        if end_index == -1:
            print "Do not find capstone opcode enum end {0}".format(cap_opcode_end)
            return False
        # INVALID and ENDING are not actual opcodes
        for part in capstone_header[start_index : end_index].split(",")[1:]:
            parts = part.split("\n")
            last_line = parts[-1].strip()
            if len(last_line) == 0: continue
            self.cap_opcode.append(last_line)


        # We now extract Capston'e registers
        self.cap_reg = []
        cap_reg_start = "{0}_REG_INVALID".format(capstone_prefix)
        cap_reg_end = "{0}_REG_ENDING".format(capstone_prefix)
        start_index = capstone_header.find(cap_reg_start) 
        if start_index == -1:
            print "Do not find capstone opcode enum start {0}".format(cap_reg_start)
            return False
        end_index = capstone_header.find(cap_reg_end) 
        if end_index == -1:
            print "Do not find capstone opcode enum end {0}".format(cap_reg_end)
            return False
        # INVALID and ENDING are not actual regs
        for part in capstone_header[start_index : end_index].split(",")[1:]:
            parts = part.split("\n")
            last_line = parts[-1].strip()
            if len(last_line) == 0: continue
            self.cap_reg.append(last_line)

        return True

    def DyninstOpcodeRange(self):
        if self.arch == "x86": 
            return "e_jb", "e_getsec"
        elif self.arch == "power": 
            return "power_op_extended", "power_op_dxex"
        elif self.arch == "arm":
            return "aarch64_op_extended", "aarch64_op_zip2_advsimd"
        else:
            print "Not supported arch"
            return "", ""

    def DyninstOpcodePrefix(self):
        if self.arch == "x86": 
            return "e"
        elif self.arch == "power": 
            return "power_op"
        elif self.arch == "arm":
            return "aarch64_op"
        else:
            print "Not supported arch"
            return ""


    def DyninstRegisterPrefix(self):
        if self.arch == "x86": 
            return "x86_64"
        elif self.arch == "power": 
            return "ppc64"
        elif self.arch == "arm":
            return "aarch64"
        else:
            print "Not supported arch"
            return ""

    def AnalyzeDyninstHeader(self):
        self.dyninst_opcode = []
        # Dyninst's opcode enum does not have boundary marker.
        # So, we need to manually identify the first and last actual
        # opcodes for each architecture
        dyninst_opcode_start, dyninst_opcode_end = self.DyninstOpcodeRange()

        dyninst_header = open(self.dyninst, "r").read()
        start_index = dyninst_header.find(dyninst_opcode_start) 
        if start_index == -1:
            print "Do not find capstone opcode enum start {0}".format(dyninst_opcode_start)
            return False
        end_index = dyninst_header.find(dyninst_opcode_end) 
        if end_index == -1:
            print "Do not find capstone opcode enum end {0}".format(dyninst_opcode_end)
            return False
        # Dyninst's opcode does not contain boundary marker
        for part in dyninst_header[start_index : (end_index + len(dyninst_opcode_end))].split(","):
            # Dyninst's opcode enum list may contain many noise
            parts = part.split("\n")
            last_line = parts[-1].strip()
            # There may be commends
            if last_line.startswith("//"): continue
            # There can be value definitions
            if last_line.find("=") != -1:
                last_line = last_line.split("=")[0].strip()
            # There can be comma in comments. Every dyninst opcode should contain "_"
            if last_line.find("_") == -1: continue
            self.dyninst_opcode.append(last_line)
        return True

    def BuildTranslation(self):
        # In most cases, Capstone's opcode ($(ARCH)_INSN_$(OPCODE)) can be translated
        # into Dyninst's opcode with a stirng replacement.
        cap_map = {}
        for op in self.cap_opcode:
            opcode_suffix = op.split("_")[-1].lower()
            cap_map[opcode_suffix] = op

        
        if self.arch == "x86":
            # Dyninst's x86 opcodes are "e_xxx"
            dyninst_underscore_start = 1
        elif self.arch == "power":
            # Dyninst's power opcode are "power_op_xxx"
            dyninst_underscore_start = 2
        elif self.arch == "arm":
            # Dyninst's ARMv8 opcode are "aarch64_op_xxx"
            dyninst_underscore_start = 2
        else:
            print "Not supported arch"
            return

        # We now go over each Dyninst opcode to find a correspondce in Capstone
        count = 0
        useless = []
        for op in self.dyninst_opcode:
            # Dyninst's opcode sometimes has multiple "_"
            parts = op.split("_")
            actual_opcode = "_".join(parts[dyninst_underscore_start:]).lower()

            capstone_opcode = ""
            # String replacement
            if actual_opcode in cap_map:
                capstone_opcode = cap_map[actual_opcode]
                del(cap_map[actual_opcode])
            
#            if actual_opcode == "xlat":
#                capstone_opcode = cap_map["xlatb"]
#                del(cap_map["xlatb"])


            # Dyninst's e_No_Entry is a special entry
            if op == "e_No_Entry": continue

        print len(cap_map), "new opcodes"
        print "{0} no match, {1} useless".format(count, len(useless))
        for op, grep in useless:
            print op, "Grep:"
            print grep
        self.new_opcode = cap_map

    def WriteOutputFile(self):
        new_opcode_list = list(self.new_opcode)
        new_opcode_list.sort()
        self.output_file.write("To common/h/entryIDs.h (new opcode) \n\n")
        prefix = self.DyninstOpcodePrefix()
        for actual_op in new_opcode_list:
            self.output_file.write("  {0}_{1},\n".format(prefix, actual_op))

#        self.output_file.write("\nTo common/src/arch-{0}.C\n\n".format(self.arch))
#        for actual_op in new_opcode_list:
#            self.output_file.write('({0}_{1},"{1}")\n'.format(prefix, actual_op))

        self.output_file.write("\nOpcode translation. To instructionAPI/src/InstructionDecoder-Capstone.C (replace existing opcodeTranslation_arch function)\n\n");
        self.output_file.write("    switch (cap_id) {\n");
        for op in self.cap_opcode:
            self.output_file.write('        case {0}:\n'.format(op))
            actual_op = op.split("_")[-1].lower()
            actual_op = self.AliasNormalization(actual_op)
            self.output_file.write('            return {0}_{1};\n'.format(prefix, actual_op))
        self.output_file.write('        default:\n')
        self.output_file.write('            return e_No_Entry;\n')
        self.output_file.write('    }\n')

        self.output_file.write("\nOpcode to string map. To instructionAPI/src/InstructionDecoder-Capstone.C (replace existing opcode_str init code)\n\n");
        for op in self.cap_opcode:
            actual_op = op.split("_")[-1].lower()
            actual_op = self.AliasNormalization(actual_op)
            self.output_file.write('    ({0}_{1},"{1}")\n'.format(prefix, actual_op))
        self.output_file.write('    (e_No_Entry, "No_Entry");\n')

        prefix = self.DyninstRegisterPrefix()
        self.output_file.write("\nRegister translation. To instructionAPI/src/InstructionDecoder-Capstone.C (replace existing registerTranslation_arch function)\n\n");
        self.output_file.write("    switch (cap_reg) {\n");
        for op in self.cap_reg:
            self.output_file.write('        case {0}:\n'.format(op))
            actual_reg = op.split("_")[-1].lower()
            if actual_reg == "eflags":
                actual_reg = "flags"
            elif actual_reg == "ip":
                actual_reg = "rip"
            self.output_file.write('            return {0}::{1};\n'.format(prefix, actual_reg))
        self.output_file.write('        default:\n')
        self.output_file.write('            return InvalidReg;\n')
        self.output_file.write('    }\n')

        self.output_file.close()

    def AliasNormalization(self, op):
        if op in self.aliasMap:
            return self.aliasMap[op]
        else:
            return op

args = getParameters()

t = Translator()
t.AnalyzeCapstoneHeader()
t.AnalyzeDyninstHeader()
t.BuildTranslation()
t.WriteOutputFile()
