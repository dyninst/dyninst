import re

def read_mnemonics(file:str):
  # Format is '   "name", // X86_INS_NAME'
  reg = re.compile(r'\s+\"(.+)?\", \/\/')
  mnemonics = []
  with open(file, "r") as f:
    for line in f:
      matches = reg.search(line)
      if matches:
        mnemonics.append(matches.group(1))
  return sorted(mnemonics)
