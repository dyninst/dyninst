def _read_capstone_mnemonics(file:str):
  return []

def _read_dyninst_mnemonics(file:str):
  return []

class mnemonics:
  def __init__(self, cap_dir:str, dyn_dir:str):
    self.dyninst_prefix = "power_op"
    self.missing = None
    self.pseudo = None
    self.capstone = _read_capstone_mnemonics(cap_dir + "/arch/PowerPC/PPCGenCSMappingInsnName.inc")
    self.dyninst = _read_dyninst_mnemonics(dyn_dir + "/common/h/mnemonics/ppc_entryIDs.h")
    self.aliases = None
