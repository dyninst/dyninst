# The x86/x86_64 register space is too complicated to automatically import from Capstone.
#
# The work here just creates the translation between Capstone and Dyninst

import re

def _read_capstone_registers(file:str, capstone_prefix):

  def _find_reg_defs(fd):
    # Capstone register enumerations start with a '*_INVALID' entry
    for line in fd:
      if f"{capstone_prefix}_INVALID" in line:
        return True
    return False

  registers = []
  with open(file, "r") as f:
    if not _find_reg_defs(f):
      raise Exception("Unable to find register definitions in '{0:s}'".format(file)) 

    for line in f:
      if f"{capstone_prefix}_ENDING" in line:
        break
      line = re.sub(r"\s","",line)
      for reg in line.split(','):
        if capstone_prefix not in reg:
          continue
        registers.append(reg.replace(f"{capstone_prefix}_","").strip().lower())

  return registers

def _read_dyninst_registers(file:str):
  regs = []
  
  with open(file, "r") as f:
    for line in f:
      
      # Don't include the internal pseudo-registers
      if "Pseudo-registers for internal use only" in line.lower():
        break
      
      if not "DEF_REGISTER(" in line:
        continue

      def _parse_name(prefix):
        name, _ = line.replace(' ', '').replace(prefix,"").split(",", 1)
        return name

      if "DEF_REGISTER_ALIAS" in line:
        # Format is DEF_REGISTER_ALIAS( X, Y, "XXX")
        name = _parse_name("DEF_REGISTER_ALIAS(")
      else:
        # Format is DEF_REGISTER( x0, 0 | FULL | GPR | Arch_XXX, "XXX")
        name = _parse_name("DEF_REGISTER(")

      regs.append(name.lower())

  return sorted(regs)

class registers:
  def __init__(self, cap_dir:str, dyn_dir:str):
    self.dyninst_prefix = "x86_64"
    self.capstone_prefix = "X86_REG"
    self.capstone = _read_capstone_registers(cap_dir+ "/include/capstone/x86.h", self.capstone_prefix)
    self.dyninst64 = _read_dyninst_registers(dyn_dir+ "/include/registers/x86_64_regs.h")
    self.dyninst = None
    self.all = None  # Don't generate MachRegister definitions in x86_{64}_regs.h
    self.aliases = None

  def export_xlat_tables(self):
    with open(f"{self.dyninst_prefix}.regs.xlat", "w") as f:
      for r in [r for r in self.capstone if r in self.dyninst64]:
        f.write(f"case {self.capstone_prefix}_{r.upper()}: return Dyninst::{self.dyninst_prefix}::{r};\n")

      f.write("\n\nIn Capstone, but not Dyninst\n\n")

      for r in [r for r in self.capstone if r not in self.dyninst64]:
        f.write(f"{self.capstone_prefix}_{r.upper()}\n")

  @staticmethod
  def export_lengths(f):
    pass
  
  @staticmethod
  def export_categories(f):
    pass
  
  @staticmethod
  def export_dwarf(f):
    pass
  
