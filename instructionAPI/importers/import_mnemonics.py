import argparse
import zydis.mnemonics
import capstone.aarch64.mnemonics
import capstone.ppc.mnemonics
import capstone.riscv64.mnemonics

parser = argparse.ArgumentParser(
  description="Import third-party instruction decoder entries into Dyninst",
  epilog="""
  The mnemonics are stored in the output file 'mnemonics.<arch>'. This can be directly copied to
  the appropriate architecture file in Dyninst (e.g., common/mnemonics/x86_entryIDs.h).
"""
)
parser.add_argument(
  "--decoder-dir",
  type=str,
  required=True,
  help="Source directory for the third-party decoder (e.g., /capstone-engine/capstone/)"
)
parser.add_argument(
  "--dyninst-dir",
  type=str,
  required=True,
  help="Dyninst source directory (e.g., /dyninst/src/)"
)

parser.add_argument("--arch", type=str, choices=["x86","aarch64","ppc","riscv64"], default="x86")
args = parser.parse_args()

print("Processing mnemonics for {0:s}".format(args.arch))

if args.arch == "x86":
  mnemonics = zydis.mnemonics.mnemonics(args.decoder_dir, args.dyninst_dir)
elif args.arch == "aarch64":
  mnemonics = capstone.aarch64.mnemonics.mnemonics(args.decoder_dir, args.dyninst_dir)
elif args.arch == "ppc":
  mnemonics = capstone.ppc.mnemonics.mnemonics(args.decoder_dir, args.dyninst_dir)
elif args.arch == "riscv64":
  mnemonics = capstone.riscv64.mnemonics.mnemonics(args.decoder_dir, args.dyninst_dir)

with open("mnemonics.{0:s}".format(args.arch), "w") as f:
  if mnemonics.pseudo is not None:
    for p in mnemonics.pseudo:
      f.write("{0:s}_{1:s}, /* pseudo mnemonic */\n".format(mnemonics.dyninst_prefix, p))

  if mnemonics.aliases is not None:
    for m in mnemonics.external:
      if m not in mnemonics.aliases:
        f.write("{0:s}_{1:s},\n".format(mnemonics.dyninst_prefix, m))
        continue

      if not mnemonics.aliases[m]["seen"]:
        f.write("{0:s}_{1:s},\n".format(mnemonics.dyninst_prefix, m))
        mnemonics.aliases[m]["seen"] = True
        for a in mnemonics.aliases[m]["values"]:
          f.write("{0:s}_{1:s} = {0:s}_{2:s},\n".format(mnemonics.dyninst_prefix, a, m))
          mnemonics.aliases[a]["seen"] = True

# New mnemonics added from third-party decoder
print("New mnemonics added from third-party decoder: ")
externalset = set(mnemonics.external)
dynset = set(mnemonics.dyninst)
new_mnemonics = externalset - dynset
if len(new_mnemonics) > 0:
  for m in sorted(new_mnemonics):
    print("\t{0:s}".format(m))
else:
  print("None")

# Missing aliases
if mnemonics.aliases is not None:
  print("Known aliases not in third-party decoder: ")
  unseen_aliases = [a for a in mnemonics.aliases if not mnemonics.aliases[a]["seen"]]
  if len(unseen_aliases) > 0:
    for a in sorted(unseen_aliases):
      print("\tDidn't find '{0:s}', aliased with ".format(a), mnemonics.aliases[a]["values"])
  else:
    print("None")

# Known mnemonics missing from third-party decoder
if mnemonics.missing is not None:
  print("Mnemonics known to be missing from third-party decoder: ")
  if len(mnemonics.missing) > 0:
    print()
    for m in sorted(mnemonics.missing):
      print("\t{0:s}".format(m))
  else:
    print("None")

# In Dyninst, but not third-party decoder
print("Mnemonics found in Dyninst, but not third-party decoder (likely an error!!): ")
missing = dynset - externalset # implicitly ignores pseudo registers

if mnemonics.aliases is not None:
  # Ignore seen aliases
  missing -= set([a for a in mnemonics.aliases if mnemonics.aliases[a]["seen"]])  

if mnemonics.missing is not None:
  # Ignore known missing
  missing -= set(mnemonics.missing)

if len(missing) > 0:
  print()
  for m in sorted(missing):
    print("\t{0:s}".format(m))
else:
  print("None")
