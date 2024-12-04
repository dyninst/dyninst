import re

def _read_capstone_mnemonics(file:str):
  reg = re.compile(r'\s+{\s*.*,\s*"(.*)"')
  mnemonics = []
  with open(file, "r") as f:
    for line in f:
      matches = reg.search(line)
      if matches:
        mnemonic = matches.group(1).replace(".", "_").replace("aqrl", "aq_rl")
        mnemonics.append(mnemonic)
  return sorted(mnemonics)

def _read_dyninst_mnemonics(file:str, dyninst_prefix:str):
  mnemonics = []
  with open(file, "r") as f:
    for line in f:
      entry = line.split(",", 1)[0]
      if "=" in entry:
        name,alias = [e.strip() for e in entry.split("=")]
        for n in [name, alias]:
          if n.startswith(dyninst_prefix):
            n = n[len(dyninst_prefix) + 1:]
          if n != "INVALID":
            mnemonics.append(n)
      else:
        e = entry.strip()
        if e.startswith(dyninst_prefix):
          e = e[len(dyninst_prefix) + 1:]
        if e != "INVALID":
          mnemonics.append(e)

  return sorted(mnemonics)

class mnemonics:
  def __init__(self, cap_dir:str, dyn_dir:str):
    self.dyninst_prefix = "riscv64_op"
    self.missing = None
    self.pseudo = None
    self.capstone = _read_capstone_mnemonics(cap_dir + "/arch/RISCV/RISCVGenInsnNameMaps.inc")
    self.dyninst = _read_dyninst_mnemonics(dyn_dir + "/common/h/mnemonics/riscv64_entryIDs.h", self.dyninst_prefix)
    self.aliases = None
