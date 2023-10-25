class x86:

    def __init__(self):
        self.capstone_prefix = "X86"
        self.dyninst_prefix = "e"
        self.dyninst_register_prefix = "x86_64"
        
        # Mnemonics missing in Capstone
        self.missing = [ "faddp" ]
        
        # Pseudo-mnemonics used in Dyninst and Capstone
        #   The ones from Capstone are used for GNU assembler (gas) compatibility
        self.pseudo = [
            "No_Entry",  # needs to be first
            "3dnow_generic",
            "fp_generic",
            "int80",
            "jb_jnaej_j",
            "jcxz_jec",
            "jnb_jae_j",
            "movlps_movhlps",
            "movhps_movlhps",
            "movsd_sse",
            "pextrd_pextrq",
            "pinsrd_pinsrq",
            "prefetch_w",
            "ud2grp10 ",
            "ret_far",
            "ret_near",
            "lcall",  # From Capstone
            "clzero"  # From Capstone
         ]
        
        # x86 has a great many aliased opcodes. This table represents the mappings between
        # the various mnemonics associated with those opcodes. There are also opcodes with
        # distinct values that map to the same mnemonic (e.g., 0x14 and 0x15 are "ADC").
        # Those are left to Capstone to figure out. We just represent the single mnemonic
        # as defined in the SDM.
        #
        #   AT&T aliases (e.g., call vs callq) are not included.
        #
        #   Aliases that come from the LLVM tablegen files are marked as such. Otherwise,
        #   they come from the Volume 2 Instruction Set Referece in the Intel 64 and IA-32
        #   Architectures Software Developer’s Manual (SDM) June 2021 edition with a heading like
        #   X/Y (e.g., CBW/CWDE/CDQE).
        #
        #  All conditional operations are included.
        #    Aliases for the FCMOVcc or LOOPcc mnemonics do not exist. We also ignore jcxz b/c as it's
        #    a 16-bit instruction.
        
        self.aliases = {
            "bndcu" : { "seen" : False, "values" : ["bndcn"] },
            "bndcn" : { "seen" : False, "values" : ["bndcu"] },
            
            "cbw" : { "seen" : False, "values" : ["cdqe",] },
            "cdqe" : { "seen" : False, "values" : ["cbw",] },

            "cdq" : { "seen" : False, "values" : ["cwd", "cqo",] },
            "cqo" : { "seen" : False, "values" : ["cwd", "cdq",] },
            "cwd" : { "seen" : False, "values" : ["cdq", "cqo",] },

            "cmovb" : { "seen" : False, "values" : ["cmovc", "cmovnae",] },
            "cmovc" : { "seen" : False, "values" : ["cmovb", "cmovnae",] },
            "cmovnae" : { "seen" : False, "values" : ["cmovb", "cmovc",] },
            "cmovae" : { "seen" : False, "values" : ["cmovnb", "cmovnc",] },
            "cmovnb" : { "seen" : False, "values" : ["cmovae", "cmovnc",] },
            "cmovnc" : { "seen" : False, "values" : ["cmovae", "cmovnb",] },
            "cmove" : { "seen" : False, "values" : ["cmovz",] },
            "cmovz" : { "seen" : False, "values" : ["cmove",] },
            "cmovne" : { "seen" : False, "values" : ["cmovnz",] },
            "cmovnz" : { "seen" : False, "values" : ["cmovne",] },
            "cmovbe" : { "seen" : False, "values" : ["cmovna",] },
            "cmovna" : { "seen" : False, "values" : ["cmovbe",] },
            "cmova" : { "seen" : False, "values" : ["cmovnbe",] },
            "cmovnbe" : { "seen" : False, "values" : ["cmova",] },
            "cmovp" : { "seen" : False, "values" : ["cmovpe",] },
            "cmovpe" : { "seen" : False, "values" : ["cmovp",] },
            "cmovnp" : { "seen" : False, "values" : ["cmovpo",] },
            "cmovpo" : { "seen" : False, "values" : ["cmovnp",] },
            "cmovl" : { "seen" : False, "values" : ["cmovnge",] },
            "cmovnge" : { "seen" : False, "values" : ["cmovl",] },
            "cmovge" : { "seen" : False, "values" : ["cmovnl",] },
            "cmovnl" : { "seen" : False, "values" : ["cmovge",] },
            "cmovle" : { "seen" : False, "values" : ["cmovng",] },
            "cmovng" : { "seen" : False, "values" : ["cmovle",] },
            "cmovg" : { "seen" : False, "values" : ["cmovnle",] },
            "cmovnle" : { "seen" : False, "values" : ["cmovg",] },
            "fcompi" : { "seen" : False, "values" : ["fcomip",] }, # From LLVM, but present in bddisasm, binutils, and xed
            "fcomip" : { "seen" : False, "values" : ["fcompi",] },
            "fucomip" : { "seen" : False, "values" : ["fucompi",] },
            "fucompi" : { "seen" : False, "values" : ["fucomip",] }, # From LLVM
            "iret" : { "seen" : False, "values" : ["iretd", "iretq",] },
            "iretd" : { "seen" : False, "values" : ["iret", "iretq",] },
            "iretq" : { "seen" : False, "values" : ["iret", "iretd",] },
            "jb" : { "seen" : False, "values" : ["jc", "jnae",] },
            "jc" : { "seen" : False, "values" : ["jb", "jnae",] },
            "jnae" : { "seen" : False, "values" : ["jb", "jc",] },
            "jae" : { "seen" : False, "values" : ["jnb", "jnc",] },
            "jnb" : { "seen" : False, "values" : ["jae", "jnc",] },
            "jnc" : { "seen" : False, "values" : ["jae", "jnb",] },
            "je" : { "seen" : False, "values" : ["jz",] },
            "jz" : { "seen" : False, "values" : ["je",] },
            "jne" : { "seen" : False, "values" : ["jnz",] },
            "jnz" : { "seen" : False, "values" : ["jne",] },
            "jbe" : { "seen" : False, "values" : ["jna",] },
            "jna" : { "seen" : False, "values" : ["jbe",] },
            "ja" : { "seen" : False, "values" : ["jnbe",] },
            "jnbe" : { "seen" : False, "values" : ["ja",] },
            "js" : { "seen" : False, "values" : [] },
            "jns" : { "seen" : False, "values" : [] },
            "jp" : { "seen" : False, "values" : ["jpe",] },
            "jpe" : { "seen" : False, "values" : ["jp",] },
            "jnp" : { "seen" : False, "values" : ["jpo",] },
            "jpo" : { "seen" : False, "values" : ["jnp",] },
            "jl" : { "seen" : False, "values" : ["jnge",] },
            "jnge" : { "seen" : False, "values" : ["jl",] },
            "jge" : { "seen" : False, "values" : ["jnl",] },
            "jnl" : { "seen" : False, "values" : ["jge",] },
            "jle" : { "seen" : False, "values" : ["jng",] },
            "jng" : { "seen" : False, "values" : ["jle",] },
            "jg" : { "seen" : False, "values" : ["jnle",] },
            "jnle" : { "seen" : False, "values" : ["jg",] },
            "jcxz" : { "seen" : False, "values" : ["jecxz", "jrcxz",] },
            "jecxz" : { "seen" : False, "values" : ["jcxz", "jrcxz",] },
            "jrcxz" : { "seen" : False, "values" : ["jcxz", "jecxz",] },
            "lods" : { "seen" : False, "values" : ["lodsb",] },  # LLVM, bddisasm, binutils, and xed all agree
            "lodsb" : { "seen" : False, "values" : ["lods",] },
            "loope" : { "seen" : False, "values" : ["loopz",] },  # LLVM
            "loopz" : { "seen" : False, "values" : ["loope",] },
            "loopne" : { "seen" : False, "values" : ["loopnz",] },
            "loopnz" : { "seen" : False, "values" : ["loopne",] },  # Not a valid mnemonic for 0xE0, but used in LLVM, bddisasm, and binutils 
            "monitor" : { "seen" : False, "values" : ["monitorx"] },
            "monitorx" : { "seen" : False, "values" : ["monitor"] },
            "mwait" : { "seen" : False, "values" : ["mwaitx"] },
            "mwaitx" : { "seen" : False, "values" : ["mwait"] },
            # Capstone uses popal, but everyone else uses popa. popal isn't in the SDM.
            "popa" : { "seen" : False, "values" : ["popal",] },
            "popal" : { "seen" : False, "values" : ["popa",] },
            # Capstone uses popaw, but everyone else uses popad. popaw isn't in the SDM.
            "popad" : { "seen" : False, "values" : ["popaw",] },
            "popaw" : { "seen" : False, "values" : ["popad",] },
            # Capstone treats POP{F,D,Q} as separate mnemonics, even though they share an opcode (0x9D)
            # Capstone uses non-standard pusha{l,w}.
            "pusha" : { "seen" : False, "values" : ["pushad", "pushal", "pushaw",] },
            "pushad" : { "seen" : False, "values" : ["pusha", "pushal", "pushaw",] },
            "pushal" : { "seen" : False, "values" : ["pusha", "pushad", "pushaw",] },
            "pushaw" : { "seen" : False, "values" : ["pusha", "pushad", "pushal",] },
            "pushf" : { "seen" : False, "values" : ["pushfd", "pushfq",] },
            "pushfd" : { "seen" : False, "values" : ["pushf", "pushfq",] },
            "pushfq" : { "seen" : False, "values" : ["pushf", "pushfd",] },
            "retfq" : { "seen" : False, "values" : ["retf",] },
            "retf" : { "seen" : False, "values" : ["retfq",] },
            "setb" : { "seen" : False, "values" : ["setc", "setnae",] },
            "setc" : { "seen" : False, "values" : ["setb", "setnae",] },
            "setnae" : { "seen" : False, "values" : ["setb", "setc",] },
            "setae" : { "seen" : False, "values" : ["setnb", "setnc",] },
            "setnb" : { "seen" : False, "values" : ["setae", "setnc",] },
            "setnc" : { "seen" : False, "values" : ["setae", "setnb",] },
            "sete" : { "seen" : False, "values" : ["setz",] },
            "setz" : { "seen" : False, "values" : ["sete",] },
            "setne" : { "seen" : False, "values" : ["setnz",] },
            "setnz" : { "seen" : False, "values" : ["setne",] },
            "setbe" : { "seen" : False, "values" : ["setna",] },
            "setna" : { "seen" : False, "values" : ["setbe",] },
            "seta" : { "seen" : False, "values" : ["setnbe",] },
            "setnbe" : { "seen" : False, "values" : ["seta",] },
            "setp" : { "seen" : False, "values" : ["setpe",] },
            "setpe" : { "seen" : False, "values" : ["setp",] },
            "setnp" : { "seen" : False, "values" : ["setpo",] },
            "setpo" : { "seen" : False, "values" : ["setnp",] },
            "setl" : { "seen" : False, "values" : ["setnge",] },
            "setnge" : { "seen" : False, "values" : ["setl",] },
            "setge" : { "seen" : False, "values" : ["setnl",] },
            "setnl" : { "seen" : False, "values" : ["setge",] },
            "setle" : { "seen" : False, "values" : ["setng",] },
            "setng" : { "seen" : False, "values" : ["setle",] },
            "setg" : { "seen" : False, "values" : ["setnle",] },
            "setnle" : { "seen" : False, "values" : ["setg",] },
            "wait" : { "seen" : False, "values" : ["fwait",] },
            "fwait" : { "seen" : False, "values" : ["wait",] },
            "xlat" : { "seen" : False, "values" : ["xlatb",] },
            "xlatb" : { "seen" : False, "values" : ["xlat",] }
        }
