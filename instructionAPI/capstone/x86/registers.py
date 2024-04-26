def _read_capstone_registers(file:str):
  pass

def _read_dyninst_registers(file:str):
  pass

class registers:
  def __init__(self, cap_dir:str, dyn_dir:str):
    self.dyninst_prefix = "x86_64"
