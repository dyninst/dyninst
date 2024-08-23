class riscv64:

    def __init__(self, cap_dir):
        self.capstone_prefix = "RISCV"
        self.dyninst_prefix = "riscv64_op"
        self.dyninst_register_prefix = "riscv64"
        self.mnemonics_file = cap_dir + "/arch/RISCV/RISCVGenInsnNameMaps.inc"
        self.registers_file = cap_dir + "/include/capstone/riscv64.h"
        self.pseudo = []
        self.aliases = []
