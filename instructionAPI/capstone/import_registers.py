import argparse
import x86.registers
import aarch64.registers
import ppc.registers

parser = argparse.ArgumentParser(
  description="Translate Capstone's registers into Dyninst registers",
)
parser.add_argument(
  "--capstone-dir",
  type=str,
  required=True,
  help="Capstone source directory (e.g., /capstone-engine/capstone/)"
)
parser.add_argument(
  "--dyninst-dir",
  type=str,
  required=True,
  help="Dyninst source directory (e.g., /dyninst/src/)"
)
parser.add_argument(
  "--spec-dir",
  type=str,
  required=False,
  help="Location of XML specification files (required for aarch64)"
)
parser.add_argument("--arch", type=str, choices=["x86", "aarch64", "ppc"], default="x86")
args = parser.parse_args()

if args.arch == "aarch64" and args.spec_dir is None:
  raise Exception("Must specify location of XML specification files for ARM")

print("Processing registers for {0:s}\n".format(args.arch))

if args.arch == "x86":
  registers = x86.registers.registers(args.capstone_dir, args.dyninst_dir, args.spec_dir)
elif args.arch == "aarch64":
  registers = aarch64.registers.registers(args.capstone_dir, args.dyninst_dir, args.spec_dir)
elif args.arch == "ppc":
  registers = ppc.registers.registers(args.capstone_dir, args.dyninst_dir, args.spec_dir)

# Check for duplicate register names
def _check_uniqueness():
  seen = {}
  for r in registers.all:
    if r["name"] in seen:
      print("Duplicate register: \n  old: {0:s}\n  new: {1:s}".format(str(r), str(seen[r["name"]])))
    else:
      seen[r["name"]] = r
_check_uniqueness()


# Display added registers
def _print_new_regs():
  print("Adding new registers:")
  all = set([r["name"] for r in registers.all])
  dyn = set(registers.dyninst)
  new_regs = list(all - dyn)
  seen = False
  for r in sorted(new_regs):
    print("   {0:s}".format(r))
    seen = True
  if not seen:
    print("  none added")
_print_new_regs()


# length/size fields
with open("{0:s}.lengths".format(args.arch), "w") as f:
  registers.export_lengths(f)

# categories
with open("{0:s}.categories".format(args.arch), "w") as f:
  registers.export_categories(f)

# DWARF mappings
with open("{0:s}.dwarf".format(args.arch), "w") as file:
  registers.export_dwarf(file)

# Register definitions
unique_ids_by_category = {}
with open("{0:s}.registers".format(args.arch), "w") as file:
  max_name_len = max([len(r["name"]) for r in registers.all])
  max_size_len = max([len(r["size"]) for r in registers.all])
  max_cat_len = max([len(r["categories"]) for r in registers.all])

  for r in registers.all:
    categories = r["categories"]
    
    base_category = categories
    if '|' in categories:
      base_category = categories.split("|")[0].strip()
    
    if base_category not in unique_ids_by_category:
      unique_ids_by_category[base_category] = 0

    #                  (   name,  ID |    size |     cat |       arch,  "arch"
    fmt = "DEF_REGISTER(%{0:d}s, %3d | %{1:d}s | %{2:d}s | Arch_{3:s}, \"{3:s}\");\n".format(
      max_name_len+2,
      max_size_len,
      max_cat_len,
      registers.dyninst_suffix
    )
    file.write(fmt % (r["name"], unique_ids_by_category[base_category], r["size"], r["categories"]))
    unique_ids_by_category[base_category] += 1

    # The Dyninst register fields are only 8 bits wide
    if unique_ids_by_category[base_category] > 255:
      raise Exception("There are more than 255 '{0:s}' registers.".format(base_category))


# Aliased registers
with open("{0:s}.aliases".format(args.arch), "w") as file:
  max_alias_len = max([len(r["alias"]) for r in registers.aliases])
  max_primary_len = max([len(r["primary"]) for r in registers.aliases])
  
  fmt = "DEF_REGISTER_ALIAS(%{0:d}s, %{1:d}s, \"{2:s}\");\n".format(max_alias_len, max_primary_len, registers.dyninst_suffix) 

  for reg in registers.aliases:
    file.write(fmt % (reg["alias"], reg["primary"]))


