import re
import aarch64.sysregs
import aarch64.dwarf


class registers:
  """
  capstone - non-system registers from Capstone
   
   These are the general-purpose, floating-point (vector and scalar), and control
   registers. They have Capstone names with the form Aarch64_REG_<name>.

  capstone_sysregs - system registers from Capstone
  
    These either have a Capstone name of the form Aarch64_SYSREG_<name> or are
    derived from the system operand tablegen files (e.g., AArch64_DC_<name>).
  
  dyninst - registers declared in Dyninst
  
    These are only used to determine which registers have been added after processing
    the new files from Capstone.
    
  spec_sysregs - system registers from the ARM64 System Register Spec files
  
    These are from the files in
    https://developer.arm.com/Architectures/A-Profile%20Architecture#Software-Download
    
  all - the set of registers to be imported into Dyninst
  
  aliases - Registers with different names for the same physical register
  
    The "primary" name is taken to be the one Capstone uses. For example, Capstone
    calls the link register Aarch64_REG_LR, but the ISA specifies it as x30. The primary
    name would then be "LR" and the alias name "x30". 
    
  """

  def __init__(self, cap_dir:str, dyn_dir:str, spec_dir:str):
    self.dyninst_suffix = "aarch64"
    self.capstone = _read_capstone_registers(cap_dir + "/include/capstone/aarch64.h")
    self.capstone_sysregs = aarch64.sysregs.parse_capstone(cap_dir + "/include/capstone/aarch64.h")
    self.dyninst = _read_dyninst_registers(dyn_dir + "/common/h/registers/aarch64_regs.h")
    spec_sysregs = aarch64.sysregs.parse_xml_specs(spec_dir)

    # Architecture aliases
    _aliases = {
      "fp": "x29",    # Frame Pointer
      "lr": "x30",    # Link Register
      "Ip0": "x16",   # First intra-procedure-call scratch register (capitalized to avoid conflict with DEF_REGISTER(p0))
      "Ip1": "x17"    # Second intra-procedure-call scratch register
    }
    self.aliases = [{"alias":a, "primary":_aliases[a]} for a in _aliases]
    
    # These Capstone aliases can't be parsed via _read_capstone_registers
    for r in ["x29", "x30"]:
      if not r in self.capstone: 
        self.capstone.append(r)

    # Internal Capstone aliases
    _capstone_aliases = {
      "pn0": "p0",    # SVE predicate registers: PN<N> is an LLVM alias of P<N>
      "pn1": "p1",
      "pn2": "p2",
      "pn3": "p3",
      "pn4": "p4",
      "pn5": "p5",
      "pn6": "p6",
      "pn7": "p7",
      "pn8": "p8",
      "pn9": "p9",
      "pn10": "p10",
      "pn11": "p11",
      "pn12": "p12",
      "pn13": "p13",
      "pn14": "p14",
      "pn15": "p15",
    }
    
    self.all = _process_regs(self.capstone, self.capstone_sysregs, spec_sysregs, _aliases | _capstone_aliases)


  @staticmethod
  def export_lengths(f):
    max_len = max([len(l) for l in _lengths])
    id = 0
    decl = "const int32_t %-{0:d}s = 0x0000%02X00;  // %s\n".format(max_len)
    
    for l in sorted(_lengths.keys(), key=lambda x: _lengths[x]["sortid"]):
      f.write(decl % (l, id, _lengths[l]["desc"]))
      id += 1

  @staticmethod
  def export_categories(f):
    max_len = max([len(d) for d in _basic_category_descriptions])
    id = 0
    decl = "const int32_t %-{0:d}s = 0x00%02X0000;  // %s\n".format(max_len)
    for d in _basic_category_descriptions:
      f.write(decl % (d, id, _basic_category_descriptions[d]))
      id += 1

    f.write("\n\n-- System Registers --\n\n")
    aarch64.sysregs.export_categories(f)

  @staticmethod
  def export_dwarf(f):
    aarch64.dwarf.dwarf2reg(f)
    f.write("\n\n")
    aarch64.dwarf.reg2dwarf(f)


def _capstone_to_dyninst():
  """ Map Capstone register enum values to Dyninst MachRegister objects """
  
  # NOTE: Be sure to include extra values for entries in `_capstone_sysreg_dupes`
  pass

def _read_capstone_registers(file:str):

  ignore = [
    "x28_fp",   # nonsense alias of x28 to frame pointer (real alias is x29)
    "lr_xzr",   # Unused
    "fpmr",     # Floating-point mode register; not in ARM
  ]
  
  def _find_reg_defs(fd):
    # Capstone register enumerations start with a '*_INVALID' entry
    for line in fd:
      if "AARCH64_REG_INVALID" in line:
        return True
    return False

  registers = []
  with open(file, "r") as f:
    if not _find_reg_defs(f):
      raise Exception("Unable to find register definitions in '{0:s}'".format(file)) 

    marker = "AARCH64_REG_"
    for line in f:
      if marker in line:
        # Format: AARCH64_REG_NAME = NUMBER,
        line = line.strip().replace(",", "")
        name = line[len(marker):line.find(' ')].lower()
        if name == "ending":
          break
        if name in ignore:
          continue
        # <R><1-9>_<Rn>... are internal registers
        # e.g., X10_X11_X12_X13_X14_X15_X16_X17
        if name[0] in _capstone_by_prefix and name[1].isdigit() and "_" in name:
          continue

        registers.append(name)

  return registers


def _read_dyninst_registers(file:str):
  regs = []
  
  with open(file, "r") as f:
    for line in f:
      
      # Don't include the internal pseudo-registers
      if "pseudo-registers" in line.lower():
        break
      
      if not "DEF_REGISTER(" in line:
        continue

      if "DEF_REGISTER_ALIAS" in line:
        # Format is DEF_REGISTER_ALIAS( X, Y, "aarch64")
        name, _ = line[line.index("DEF_REGISTER_ALIAS(") + 19:].split(",", 1)
        name = name.strip()
      else:
        # Format is DEF_REGISTER( x0, 0 | FULL | GPR | Arch_aarch64, "aarch64")
        name, _ = line[line.index("DEF_REGISTER(") + 13:].split(",", 1)
        name = name.strip()

      regs.append(name.lower())

  return sorted(regs)


def _process_regs(capstone, capstone_sysregs, spec_sysregs, aliases):
  regs = []
  
  for r in capstone:
    if r[0] in _capstone_by_prefix and r[1].isdigit():
      # <R><1-9> is a gpr, fpr, or SVE predicate
      details = _capstone_by_prefix[r[0]]
    elif r[:2] in _capstone_by_prefix:
      # ZA, ZT Scalable Matrix Extension
      details = _capstone_by_prefix[r[:2]]
    elif r in _capstone_by_name:
      details = _capstone_by_name[r]
    elif r in aliases:
      continue
    else:
      raise Exception("Unknown register: '{0:s}'".format(r))
    
    regs.append({
      "name": r,
      "size": details["size"],
      "categories": ' | '.join(sorted(details["categories"])),
      "is_sysreg": False
    })
    
  # Capstone doesn't have a PC register representation because it's not writable
  # We add one for completeness
  regs.append({
    "name": "pc",
    "size": "FULL",
    "categories": "SPR",
    "is_sysreg": False
  })
  
  # External Debug registers
  #  These are for use with a JTAG-style external debugger
  _ignored = [
    "DBGAUTH",
    "DBGBRK",
    "DBGCT",
    "DBGDTR",
    "DBGLR",
    "DBGPCR",
    "DBGSPSR",
    "DBGW"
  ]

  for reg_name in capstone_sysregs:
    # Check for duplicates first    
    if reg_name in _capstone_sysreg_dupes:
      # These have already been processed as normal registers
      continue
    elif reg_name in _capstone_by_name:
      details = _capstone_by_name[reg_name]
    elif reg_name[-1].isdigit() and reg_name[:-1] in _capstone_by_name:
      # The Capstone registers have the sequence number listed explicitly
      # e.g., trcdvaff{0,1,2} is trcdvaff<N> in the spec
      details = _capstone_by_name[reg_name[:-1]]
    elif reg_name.startswith("icc_"):
      # Capstone has partial support for Generic Interrupt Controller Architecture Specification
      # registers. However, these are for embedded systems only. To reduce the number of registers
      # in Dyninst, we ignore them for now.
      continue
    elif reg_name not in spec_sysregs:
      print("capstone sysreg not in spec: {0:s}".format(reg_name))
      continue
    else:
      details = spec_sysregs[reg_name]
    
    if details["categories"] in _ignored:
      continue
    
    size = details["size"]
    if size not in _lengths:
      size = _encode_sysreg_size(size)
      if size is None:
        raise Exception("'{0:s}': unknown sysreg size '{1:s}'".format(reg_name, details["size"]))
  
    regs.append({
      "name": reg_name,
      "size": size,
      "categories": ' | '.join(sorted(details["categories"])),
      "is_sysreg": True
    })

  regs.sort(key=_sort_reg_key)
  
  return regs


def _encode_sysreg_size(size):
    _sizes = {
      "32": "D_REG",
      "64": "FULL",
      "128": "Q_REG"
    }
    if size in _sizes:
      return _sizes[size]


def _sort_reg_key(reg):
  """
  Some register names encode complex information, so we
  expand that data to allow for more logical sorting.
  
  The formats are one of two forms:
  
    First form
    ----------
    {mnemonic}{tag}{ID}_el{index}
    
    For the register 'foo01_el0', mnemonic is 'foo0', tag is '0',
    ID is '1', and index is '0'. Note that the tag is optional
    and is appended to the mnemonic if the ID is present. An
    example of a register having no ID would be 'foo0_el1'.
  
    Example:
      The sequence of register names
  
        foo00_el1
        foo01_el0
        foo00_el0
        foo10_el0
        foo010_el0
  
      would be sorted as
  
        foo00_el0
        foo01_el0
        foo010_el0
        foo10_el0
        foo00_el1
  
  
    Second form
    -----------
    {size}{ID}
    
    These are the basic x, w, q, etc. registers. They are sorted in
    ascending size/width of the register and then by their ID.
    
    Example:
      The sequence of register names
  
        za
        x0
        w0
        q1
  
      would be sorted as
  
        w0
        x0
        q1
        za
  
  """
  el_fmt = re.compile(r"(\w+?)(\d+)_el(\d)")
  basic_fmt = re.compile(r"([{0:s}]+?)(\d+)".format(''.join(_capstone_by_prefix.keys())))
  indexed_basic_fmt = re.compile(r"^(\w+)(\d+)$")

  sort_priority_by_category = {
    "SPR"  : 0,
    "FLAG" : 1,
    "GPR"  : 2,
    "FPR"  : 3,
    "SVE"  : 4,
    "SVE2" : 5,
    "SME"  : 6
  }
  def _make_sort_key(mnemonic, el_index, ID):
    # Sort by category, then length, then name
    # Put the system registers at the end
    basic_priority = 1000
    for c in sort_priority_by_category:
      if c in reg["categories"]:
        basic_priority = sort_priority_by_category[c]
        break
    return (reg["is_sysreg"], basic_priority, mnemonic, int(el_index), int(ID), reg["categories"], reg["size"])

  name = reg["name"]
  mnemonic = name
  ID = '0'
  el_index = '0'
  
  matches = el_fmt.search(name)
  if matches is not None:
    mnemonic = matches.group(1)
    ID = matches.group(2)
    el_index = matches.group(3)
    if len(ID) > 1:
      mnemonic += ID[0]
      ID = ID[1:]
    return _make_sort_key(mnemonic, el_index, ID)

  matches = basic_fmt.search(name)
  if matches is not None:
    mnemonic = matches.group(1)
    ID = matches.group(2)
    return _make_sort_key(mnemonic, el_index, ID)

  matches = indexed_basic_fmt.search(name)
  if matches is not None:
    mnemonic = matches.group(1)
    ID = matches.group(2)
    return _make_sort_key(mnemonic, el_index, ID)

  return _make_sort_key(mnemonic, el_index, ID)

_basic_category_descriptions = {
  "GPR": "General-purpose",
  "FPR": "Floating-point",
  "SPR": "Special-purpose",
  "FLAG": "Control/Status flag",
  "SVE": "Scalable Vector Extension",
  "SVE2": "Scalable Vector Extension, version 2",
  "SME": "Scalable Matrix Extension"
}

_lengths = {
  "FULL": {"desc": "64-bit double-word", "sortid": -1},  # Must be first
  "D_REG": {"desc": "32-bit single-word", "sortid": 0},
  "W_REG": {"desc": "16-bit half-word", "sortid": 1},
  "B_REG": {"desc": "8-bit byte", "sortid": 2},
  "BIT": {"desc": "1 bit", "sortid": 3},
  "Q_REG": {"desc": "128-bit vector", "sortid": 4},
  "SVES": {"desc": "2048-bit Scalable Vector Extension (SVE) vector length", "sortid": 5},
  "PREDS": {"desc": "SVE predicate register", "sortid": 6},
  "SVE2S": {"desc": "512-bit Scalable Vector Extension", "sortid": 7},
  "SVLS": {"desc": "2048-bit SME Effective Streaming SVE vector length (SVL)", "sortid": 8},
  "SMEZAS": {"desc": "Scalable Matrix Extension ZA array", "sortid": 9}
}

_capstone_by_prefix = {
  "x": {"size":"FULL" , "categories":["GPR"]},
  "w": {"size":"D_REG", "categories":["GPR"]},
  "q": {"size":"Q_REG", "categories":["FPR"]},
  "d": {"size":"FULL" , "categories":["FPR"]},
  "s": {"size":"D_REG", "categories":["FPR"]},
  "h": {"size":"W_REG", "categories":["FPR"]},
  "b": {"size":"B_REG", "categories":["FPR"]},
  "z": {"size":"SVES", "categories":["SVE"]},    # Scalable Vector Extension version 1
  "p": {"size":"PREDS", "categories":["SVE"]},   # Predicate registers
  "za": {"size":"SMEZAS", "categories":["SME"]}, # Scalable Matrix Extension ZA array
  "zt": {"size":"SVE2S", "categories":["SVE2"]}  # Scalable Vector Extension version 2
}

_capstone_by_name = {
  "ffr": _capstone_by_prefix["p"],                         # First Fault Register, same size/category as predicate registers
  "fpcr": {"size":"D_REG", "categories":["SPR"]},          # Floating-Point Control Register
  "nzcv": {"size":"BIT", "categories":["SPR"]},            # Condition flag bits
  "vg": {"size":"FULL", "categories":["SVE"]},             # 64-bit SVE vector granule pseudo-register (needed for DWARF mappings)
  
  "sp": {"size":"FULL", "categories":["SPR"]},             # 64-bit stack pointer
  "wsp": {"size":"D_REG", "categories":["SPR"]},           # 32-bit stack pointer
  "xzr": {"size":"FULL", "categories":["SPR"]},            # 64-bit zero register
  "wzr": {"size":"D_REG", "categories":["SPR"]},           # 32-bit zero register
  
  "trcdvcmr": {"size": "FULL", "categories":["TRACE"]},    # Embedded Trace
  "trcdevaff": {"size": "FULL", "categories":["TRACE"]},
  "trcdvcmr": {"size": "FULL", "categories":["TRACE"]},
  "trcdvcvr": {"size": "FULL", "categories":["TRACE"]},
  "trcextinselr": {"size": "FULL", "categories":["TRACE"]},
  "trcoslar": {"size": "FULL", "categories":["TRACE"]},
  "trcprocselr": {"size": "FULL", "categories":["TRACE"]},
  "trcvdarcctlr": {"size": "FULL", "categories":["TRACE"]},
  "trcvdctlr": {"size": "FULL", "categories":["TRACE"]},
  "trcvdsacctlr": {"size": "FULL", "categories":["TRACE"]},
  
  # Counter-timer Access Control Registers for Realm Management Extension (aka, Trusted Execution Environment)
  # real name is cntacr0
  "teecr32_el1": {"size": "D_REG", "categories":["SYSTIMER"]},
  "teehbr32_el1": {"size": "D_REG", "categories":["SYSTIMER"]},
  
  # Armv8.9 Extended Translation Control Register (EL2)
  # real name is tcr_el2
  "vsctlr_el2": {"size": "D_REG", "categories":["SYSMEMORY"]}
}


# Capstone has both a SYSREG_<name> and a REG_<name> for these
_capstone_sysreg_dupes = [
  "nzcv", # NVCZ condition flags
  "fpcr"  # Floating-Point Control Register
]
