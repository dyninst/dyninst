import re

# These are not real instructions, and don't need an alias
_ignored_dyninst_names = [
  "abs", "clcs", "clf",
  "cli", "dclst", "div", "divs", "doz", "dozi", "lscbx", "maskg", "maskir",
  "mfsri", "mul", "nabs", "rac", "rfi", "rfsvc", "rlmi", "rrib", "sle",
  "sleq", "sliq", "slliq", "sllq", "slq", "sraiq", "sraq", "sre", "srea",
  "sreq", "sriq", "srliq", "srlq", "srq"
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
    self.pseudo = ['INVALID']
    self.capstone = _read_capstone_mnemonics(cap_dir + "/arch/PowerPC/PPCGenCSMappingInsnName.inc")
    self.dyninst = _read_dyninst_mnemonics(dyn_dir + "/common/h/mnemonics/ppc_entryIDs.h", self.dyninst_prefix)

    self.missing = [
      # 3.3.14 Binary Coded Decimal (BCD) Assist Instructions added in v2.06
      "addg6s", "cbcdtd", "cdtbcd",

      # 3.3.9 Fixed-Point Compare Instructions added in v2.02
      "cmp", "cmpi", "cmpl",  "cmpli",
      
      # 5.6.1 DFP Arithmetic Instructions added in v2.05 (Decimal Floating-Point)
      "dadd", "daddq", "dcffix", "dcffixq", "dcffixqq", "dcmpo", "dcmpoq", "dcmpu", 
      "dcmpuq", "dctdp", "dctfix", "dctfixq", "dctqpq", "ddedpd", "ddedpdq", "ddiv", 
      "ddivq", "denbcd", "denbcdq", "diex", "diexq", "dmul", "dmulq", "dqua", 
      "dquai", "dquaiq", "dquaq", "drdpq", "drintn", "drintnq", "drintx", "drintxq", 
      "drrnd", "drrndq", "drsp", "dscli", "dscliq", "dscri", "dscriq", "dsub", 
      "dsubq", "dtstdc", "dtstdcq", "dtstdg", "dtstdgq", "dtstex", "dtstexq", "dtstsf", 
      "dtstsfi", "dtstsfiq", "dtstsfq", "dxex", "dxexq"
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
    }
