import argparse
import capstone
import x86
import riscv64

"""
  Process and import Capstone instruction mnemonics
    
    Each architecture is in a separate module and implements things like mnemonic prefixes and
    instruction aliases. The latter must be manually updated as more instructions are added to
    the ISAs.
    
  Usage:
    The mnemonics are stored in the output file 'mnemonics'. This can be directly copied to
    the appropriate architecture file in Dyninst (e.g., common/mnemonics/x86_entryIDs.h).
    Examining changes is then most easily done using git.
"""

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Translate Capstone instruction data structures to Dyninst instruction data structures"
    )
    parser.add_argument(
        "--capstone",
        type=str,
        required=True,
        help="Capstone directory (e.g., /capstone-engine/capstone/)"
    )

    parser.add_argument("--arch", type=str, choices=["x86", "riscv64"], default="x86")
    args = parser.parse_args()

    arch = args.arch
    if arch == "x86":
        translator = x86.x86(args.capstone)
    elif arch == "riscv64":
        translator = riscv64.riscv64(args.capstone)

    mnemonics_orig = capstone.read_mnemonics(translator.mnemonics_file, arch)
    # Replace 'aqrl' with 'aq.rl'
    mnemonics_orig = list(map(lambda string: string.replace('aqrl', 'aq.rl'), mnemonics_orig))

    # RISC-V has some instructions that contains '.'
    # Replace it with '_'
    mnemonics = list(map(lambda string: string.replace('.', '_'), mnemonics_orig))
    
    with open("mnemonics", "w") as f:
        for p in translator.pseudo:
            f.write("{0:s}_{1:s}, /* pseudo mnemonic */\n".format(translator.dyninst_prefix))
    
        # Remove already-printed names
        mnemonics_scrubbed = [m for m in mnemonics if m not in translator.pseudo]
    
        for m in mnemonics_scrubbed:
          if m not in translator.aliases:
            f.write("{0:s}_{1:s},\n".format(translator.dyninst_prefix, m))
            continue
    
          if not translator.aliases[m]["seen"]:
            f.write("{0:s}_{1:s},\n".format(translator.dyninst_prefix, m))
            translator.aliases[m]["seen"] = True
            for a in translator.aliases[m]["values"]:
              f.write("{0:s}_{1:s} = {0:s}_{2:s},\n".format(translator.dyninst_prefix, a, m))
              translator.aliases[a]["seen"] = True

    for a in translator.aliases:
        translator.aliases[a]["seen"] = False;

    with open("mnemonics-translate", "w") as f:
        for m in mnemonics:
            f.write("case {0:s}_INS_{1:s}: return {2:s}_{3:s};\n".format(
                  translator.capstone_prefix, m.upper(), translator.dyninst_prefix, m
                )
            )

    with open("mnemonics-capstone", "w") as f:
        for m, m_orig in zip(mnemonics, mnemonics_orig):
            f.write("    opcode_str->emplace({0:s}_{1:s},\"{2:s}\");\n".format(
                  translator.dyninst_prefix, m, m_orig
                )
            )
