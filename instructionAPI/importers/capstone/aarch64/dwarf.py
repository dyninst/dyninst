class invalid:
  def __init__(self, regnum):
    self.regnum = regnum

class reserved:
  def __init__(self, lower, upper):
    self.regnums = [i for i in range(lower, upper+1)]

mapping = [
  {"number": [i for i in range(31)], "name": "x", "desc": "64-bit general-purpose registers"},
  {"number": 31, "name": "sp", "desc": "64-bit stack pointer"},
  {"number": 32, "name": "pc", "desc": "64-bit program counter"},

  # Modes are {1,2,3} depending on state.
  # We only use mode 1.  
  {"number": 33, "name": "elr_el1", "desc": "Current mode exception link register"},
  
  #  This is a proposed value in beta state.
  #  RA_SIGN_STATE is _not_ mentioned in the April 2023 Architecture
  #  Reference Manual (ARM), so marked it as 'invalid' for now.
  {"number": invalid(34), "desc": "RA_SIGN_STATE Return address signed state pseudo-register (beta)"},
  
  {"number": 35, "name": "tpidrro_el0", "desc": "EL0 Read-Only Software Thread ID register"},
  {"number": 36, "name": "tpidr_el0", "desc": "EL0 Read/Write Software Thread ID register"},
  {"number": 37, "name": "tpidr_el1", "desc": "EL1 Software Thread ID register"},
  {"number": 38, "name": "tpidr_el2", "desc": "EL2 Software Thread ID register"},
  {"number": 39, "name": "tpidr_el3", "desc": "EL3 Software Thread ID register"},
  {"number": reserved(40,45)},
  {"number": 46, "name": "vg", "desc": "64-bit SVE vector granule pseudo-register (beta state)"},
  {"number": 47, "name": "ffr", "desc": "VG × 8-bit SVE first fault register"},
  {"number": [i for i in range(48, 64)], "name": "p", "desc": "VG × 8-bit SVE predicate registers"},
  {"number": [i for i in range(64, 96)], "name": "q", "desc": "128-bit FP/Advanced SIMD registers"},
  {"number": [i for i in range(96, 128)], "name": "z", "desc": "VG × 64-bit SVE vector registers (beta state)"}
]

def dwarf2reg(file):
  for m in mapping:
    if isinstance(m["number"], list):
      regnums = m["number"]
      file.write("\n// {0:s}\n".format(m["desc"]))
      for num in regnums:
        index = num - regnums[0]
        casestmt = "case {0:d}: return Dyninst::aarch64::{1:s};"
        file.write(casestmt.format(num, "{0:s}{1:d}".format(m["name"], index)))
        file.write("\n")
  
    elif isinstance(m["number"], invalid):
      casestmt = "case {0:d}: return Dyninst::InvalidReg;  // {1:s}"
      file.write(casestmt.format(m["number"].regnum, m["desc"]))
      file.write("\n")
    
    elif isinstance(m["number"], reserved):
      file.write("\n// Reserved\n")
      regnums = m["number"].regnums
      for num in regnums[0:-1]:
        file.write(f"case {num}:")
        file.write("\n")
      casestmt = "case {0:d}: return Dyninst::InvalidReg;\n"
      file.write(casestmt.format(regnums[-1]))
      file.write("\n")
  
    else:
      casestmt = "case {0:d}: return Dyninst::aarch64::{1:s};  // {2:s}"
      file.write(casestmt.format(m["number"], m["name"], m["desc"]))
      file.write("\n")

def reg2dwarf(file):
  for m in mapping:
    if isinstance(m["number"], list):
      regnums = m["number"]
      file.write("\n// {0:s}\n".format(m["desc"]))
      for num in regnums:
        index = num - regnums[0]
        casestmt = "case Dyninst::aarch64::i{0:s}: return {1:d};"
        file.write(casestmt.format("{0:s}{1:d}".format(m["name"], index), num))
        file.write("\n")

    elif isinstance(m["number"], invalid):
      file.write("\n// {0:d} is invalid -- {1:s}\n\n".format(m["number"].regnum, m["desc"]))
    
    elif isinstance(m["number"], reserved):
      first = m["number"].regnums[0]
      last = m["number"].regnums[-1]
      file.write(f"\n// {first} to {last} are reserved\n\n")
  
    else:
      casestmt = "case Dyninst::aarch64::i{0:s}: return {1:d};  // {2:s}"
      file.write(casestmt.format(m["name"], m["number"], m["desc"]))
      file.write("\n")
