import re

def read_mnemonics(file:str, arch:str):
  if arch == "x86":
    # Format is '   "name", // X86_INS_NAME'
    reg = re.compile(r'\s+\"(.+)?\", \/\/')
  elif arch == "riscv64":
    # Format is '    { RISCV_INSN_NAME, "name" },'
    reg = re.compile(r'\s+{\s*.*,\s*"(.*)"')
  mnemonics = []
  with open(file, "r") as f:
    for line in f:
      matches = reg.search(line)
      if matches:
        mnemonics.append(matches.group(1))
  return sorted(mnemonics)
