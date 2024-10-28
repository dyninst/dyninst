import re

# These are not real instructions, and don't need an alias
_ignored_dyninst_names = [
  "fxcpmadd", "fxcpmsub", "fxcpnmadd", "fxcpnmsub", "fxcpnpma", "fxcpnsma",
  "fxcsmadd", "fxcsmsub", "fxcsnmadd", "fxcsnmsub", "fxcsnpma", "fxcsnsma",
  "fxcxma", "fxcxnms", "fxcxnpma", "fxcxnsma", "fxmadd", "fxmr", "fxmsub",
  "fxmul", "fxnmadd", "fxnmsub", "fxpmul", "fxsmul", "abs", "clcs", "clf",
  "cli", "dclst", "div", "divs", "doz", "dozi", "lscbx", "maskg", "maskir",
  "mfsri", "mul", "nabs", "rac", "rfi", "rfsvc", "rlmi", "rrib", "sle",
  "sleq", "sliq", "slliq", "sllq", "slq", "sraiq", "sraq", "sre", "srea",
  "sreq", "sriq", "srliq", "srlq", "srq", "fsmfp", "fsmr", "fsmtp",
  "lfpdux", "lfpdx", "lfpsux", "lfpsx",
  "lfsdux", "lfsdx", "lfssux", "lfssx", "lfxdux", "lfxdx", "lfxsux", "lfxsx", 
  "stfdpx", "stfpdux", "stfpdx", "stfpiwx", "stfpsux", "stfpsx", "stfsdux",
  "stfsdx", "stfssux", "stfssx", "stfxdux", "stfxdx", "stfxsux", "stfxsx",
  "stvb16x", "lfq", "stfq", "lfqu", "stfqu", "lfqux", "stfqux", "lfqx", "stfqx",
  "stswx"
]

def _read_capstone_mnemonics(file:str):
  # Format is '   "name", // PPC_INS_NAME'
  reg = re.compile(r'\s+\"(.+)?\", \/\/')
  mnemonics = []
  with open(file, "r") as f:
    for line in f:
      matches = reg.search(line)
      if matches:
        mnemonics.append(matches.group(1))
  return sorted(mnemonics)

def _read_dyninst_mnemonics(file:str, prefix:str):
  def _add(val):
    m = val.replace(prefix+'_', '').strip()
    if m not in _ignored_dyninst_names:
      mnemonics.append(m)
    
  # Format is '<prefix>_name[ = alias],[comment]'
  mnemonics = []
  with open(file, "r") as f:
    for line in f:
      if line.startswith('//'):
        continue
      entry = line.split(',', 1)[0]
      if '=' in entry:
        name,alias = [e.strip() for e in entry.split('=')]
        for n in [name, alias]:
          _add(n)
      else:
        _add(entry)

  return sorted(mnemonics)


class mnemonics:
  def __init__(self, cap_dir:str, dyn_dir:str):
    self.dyninst_prefix = "power_op"
    self.capstone = _read_capstone_mnemonics(cap_dir + "/arch/PowerPC/PPCGenCSMappingInsnName.inc")
    self.dyninst = _read_dyninst_mnemonics(dyn_dir + "/common/h/mnemonics/ppc_entryIDs.h", self.dyninst_prefix)

    self.pseudo = [
      'INVALID',
      'extended'
    ]

    self.missing = [
      # 3.3.14 Binary Coded Decimal (BCD) Assist Instructions added in v2.06
      "addg6s", "cbcdtd", "cdtbcd",

      # 3.3.9 Fixed-Point Compare Instructions added in v2.02
      "cmp", "cmpi", "cmpl",  "cmpli",

      # 4.6.5 Floating-Point Move Instructions added in v2.07
      "fmrgew", "fmrgow",
      
      # 4.6.4 Floating-Point Load Store Doubleword Pair Instructions
      #  First mentioned in v2.05, but marked there as phased-out
      "lfdpx", "stfdpx",
      
      # 5.1.1 External Access Instructions (added before v2.01, removed in v3.0)
      "eciwx", "ecowx",
      
      # 5.6.1 DFP Arithmetic Instructions added in v2.05 (Decimal Floating-Point)
      "dadd", "daddq", "dcffix", "dcffixq", "dcffixqq", "dcmpo", "dcmpoq", "dcmpu", 
      "dcmpuq", "dctdp", "dctfix", "dctfixq", "dctqpq", "ddedpd", "ddedpdq", "ddiv", 
      "ddivq", "denbcd", "denbcdq", "diex", "diexq", "dmul", "dmulq", "dqua", 
      "dquai", "dquaiq", "dquaq", "drdpq", "drintn", "drintnq", "drintx", "drintxq", 
      "drrnd", "drrndq", "drsp", "dscli", "dscliq", "dscri", "dscriq", "dsub", 
      "dsubq", "dtstdc", "dtstdcq", "dtstdg", "dtstdgq", "dtstex", "dtstexq", "dtstsf", 
      "dtstsfi", "dtstsfiq", "dtstsfq", "dxex", "dxexq",
      
      # 3.3.7 Fixed-Point Move Assist Instructions from v3.1B (Sept2021)
      "lswx",
      
      # 3.3.17.1 from v2.07B (Category: Embedded)
      "mcrxr",
      
      # 9.3 Processor Control Instructions from v2.03
      "msgclr", "msgsnd",
      
      # 11.4 Processor Control Instructions from v2.07B
      "msgclrp", "msgsndp",
      
      # 3.3.12 Fixed-Point Logical Instructions from v2.06
      "prtyd", "prtyw",
      
      # 5.7.5.1 Segment Lookaside Buffer (SLB) from v2.03)
      "slbiag"
    ]

    self.aliases = {
        "addic" : { "seen" : False, "values" : ["addic_rc"] },
        "addic_rc" : { "seen" : False, "values" : ["addic"] },
        
        "andi" : { "seen" : False, "values" : ["andi_rc"] },
        "andi_rc" : { "seen" : False, "values" : ["andi"] },

        "andis" : { "seen" : False, "values" : ["andis_rc"] },
        "andis_rc" : { "seen" : False, "values" : ["andis"] },
                
        "stdcx" : { "seen" : False, "values" : ["stdcx_rc"] },
        "stdcx_rc" : { "seen" : False, "values" : ["stdcx"] },
        
        "stwcx" : { "seen" : False, "values" : ["stwcx_rc"] },
        "stwcx_rc" : { "seen" : False, "values" : ["stwcx"] },
        
        "cp_abort" : { "seen" : False, "values" : ["cpabort"] },
        "cpabort" : { "seen" : False, "values" : ["cp_abort"] },

        # Floating-point (correct prefix is just 'f')
        "fpabs" : { "seen" : False, "values" : ["fabs"] },
        "fabs" : { "seen" : False, "values" : ["fpabs"] },
        "fpadd" : { "seen" : False, "values" : ["fadd"] },
        "fadd" : { "seen" : False, "values" : ["fpadd"] },
        "fpctiw" : { "seen" : False, "values" : ["fctiw"] },
        "fctiw" : { "seen" : False, "values" : ["fpctiw"] },
        "fpctiwz" : { "seen" : False, "values" : ["fctiwz"] },
        "fctiwz" : { "seen" : False, "values" : ["fpctiwz"] },
        "fpmadd" : { "seen" : False, "values" : ["fmadd"] },
        "fmadd" : { "seen" : False, "values" : ["fpmadd"] },
        "fpmr" : { "seen" : False, "values" : ["fmr"] },
        "fmr" : { "seen" : False, "values" : ["fpmr"] },
        "fpmsub" : { "seen" : False, "values" : ["fmsub"] },
        "fmsub" : { "seen" : False, "values" : ["fpmsub"] },
        "fpmul" : { "seen" : False, "values" : ["fmul"] },
        "fmul" : { "seen" : False, "values" : ["fpmul"] },
        "fpnabs" : { "seen" : False, "values" : ["fnabs"] },
        "fnabs" : { "seen" : False, "values" : ["fpnabs"] },
        "fpneg" : { "seen" : False, "values" : ["fneg"] },
        "fneg" : { "seen" : False, "values" : ["fpneg"] },
        "fpnmadd" : { "seen" : False, "values" : ["fnmadd"] },
        "fnmadd" : { "seen" : False, "values" : ["fpnmadd"] },
        "fpnmsub" : { "seen" : False, "values" : ["fnmsub"] },
        "fnmsub" : { "seen" : False, "values" : ["fpnmsub"] },
        "fpre" : { "seen" : False, "values" : ["fre"] },
        "fre" : { "seen" : False, "values" : ["fpre"] },
        "fprsp" : { "seen" : False, "values" : ["frsp"] },
        "frsp" : { "seen" : False, "values" : ["fprsp"] },
        "fprsqrte" : { "seen" : False, "values" : ["frsqrte"] },
        "frsqrte" : { "seen" : False, "values" : ["fprsqrte"] },
        "fpsel" : { "seen" : False, "values" : ["fsel"] },
        "fsel" : { "seen" : False, "values" : ["fpsel"] },
        "fpsub" : { "seen" : False, "values" : ["fsub"] },
        "fsub" : { "seen" : False, "values" : ["fpsub"] },
        
        # Extend Sign Word and Shift Left Immediate
        "extswsli" : { "seen" : False, "values" : ["extswsl"] },
        "extswsl" : { "seen" : False, "values" : ["extswsli"] },

        # Embedded floating-point instructions (removed in v3.0)
        "fsabs" : { "seen" : False, "values" : ["evfsabs"] },
        "evfsabs" : { "seen" : False, "values" : ["fsabs"] },
        "fscmp" : { "seen" : False, "values" : ["evfscmpgt"] },
        "evfscmpgt" : { "seen" : False, "values" : ["fscmp"] },
        "fsnabs" : { "seen" : False, "values" : ["evfsnabs"] },
        "evfsnabs" : { "seen" : False, "values" : ["fsnabs"] },
        "fsneg" : { "seen" : False, "values" : ["evfsneg"] },
        "evfsneg" : { "seen" : False, "values" : ["fsneg"] },
        
        # Quad Processing eXtension
        "qvfaligni" : { "seen" : False, "values" : ["qvaligni"] },
        "qvaligni" : { "seen" : False, "values" : ["qvfaligni"] },
        "qvlstdux" : { "seen" : False, "values" : ["qvstfdux"] },
        "qvstfdux" : { "seen" : False, "values" : ["qvlstdux"] },
        "qvlstduxi" : { "seen" : False, "values" : ["qvstfduxi"] },
        "qvstfduxi" : { "seen" : False, "values" : ["qvlstduxi"] },
        
        # VSX extension
        "svcs" : { "seen" : False, "values" : ["sc"] },
        "sc" : { "seen" : False, "values" : ["svcs"] },
        "xscvhphp" : { "seen" : False, "values" : ["xscvhpdp"] },
        "xscvhpdp" : { "seen" : False, "values" : ["xscvhphp"] },
    }
