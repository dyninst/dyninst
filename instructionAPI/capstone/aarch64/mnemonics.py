def _read_capstone_mnemonics(file:str):
  return []

def _read_dyninst_mnemonics(file:str):
  return []

class mnemonics:
  def __init__(self, cap_dir:str, dyn_dir:str):
    self.dyninst_prefix = "aarch64_op"
    self.missing = None
    self.pseudo = None
    self.capstone = _read_capstone_mnemonics(cap_dir + "/arch/AArch64/AArch64GenCSMappingInsnName.inc")
    self.dyninst = _read_dyninst_mnemonics(dyn_dir + "/common/h/mnemonics/aarch64_entryIDs.h")
    self.aliases = None
