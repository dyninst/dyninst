import re
import sys
import glob
import xml.dom.minidom as minidom


def parse_xml_specs(spec_dir):
  registers = {}
  
  def _insert(name, categories, size):
    if name not in registers:
      registers[name] = {"categories": categories, "size":size}

  for f in glob.glob(spec_dir + "/*.xml"):
    if f.lower().startswith("aarch32"):
      continue
    
    content = minidom.parse(f)

    for reg in content.getElementsByTagName("registers"):
      reg_tag = reg.getElementsByTagName("register")[0]

      # Not every spec corresponds to a real register
      if reg_tag.getAttribute("is_register") == "False":
        continue

      # Some of the extension files aren't for AArch64
      state = reg_tag.getAttribute("execution_state")
      if state is None or state == "AArch32":
        continue

      categories = _get_categories(content)
      if categories is None or len(categories) == 0:
        # No valid categories were found; ignore the register
        continue

      name = _get_name(content)
      if name is None:
        raise Exception("{0:s}: Unable to find a name".format(f))
      
      size = _get_size(content)
      if size is None:
        raise Exception("{0:s}: Unable to find size for '{1:s}'".format(f, str(name)))

      # Some registers have sequential numberings that are actually different registers
      array_tag = reg.getElementsByTagName("reg_array")
      if len(array_tag) > 0:
        start = array_tag[0].getElementsByTagName("reg_array_start")[0].childNodes[0].data
        end = array_tag[0].getElementsByTagName("reg_array_end")[0].childNodes[0].data
        for i in range(int(start), int(end) + 1):
          _insert(name.replace("<n>", str(i)).lower(), categories, size)
      elif "<" in name and "SYSREG_IMPL" in categories:
        # Some strange implementation-defined registers aren't really registers
        # even though 'is_register' above is true.
        print("{0:s}: possibly bad register or namespaced dummy '{1:s}'. Skipping.".format(f, name))
        continue
      else:
        _insert(name.lower(), categories, size)

  return registers


def parse_capstone(file:str):
  # Capstone has some categories that don't correspond to registers
  ignored_categories = [
    "AT",                 # internal capstone stuff for address translation
    "BTI",                # Branch Target Identification assembler symbols
    "DB",                 # Data Synchronization Barrier assembler symbols
    "DBnXS",              # Data Synchronization Barrier (XS attribute) assembler symbols
    "DC",                 # predictor maintenance instructions
    "DMB",                # Data Memory Barrier assembler symbols
    "ExactFPImm",         # internal capstone stuff
    "IC",                 # Cache maintenance instructions
    "ISB",                # internal capstone stuff
    "PRFB",               # Prefetch byte assembler symbols
    "PRFM",               # Prefetch memory assembler symbols
    "PSB",                # Profiling Synchronization Barrier assembler symbols
    "PStateImm0_1",       # internal capstone stuff
    "PStateImm0_15",      # internal capstone stuff
    "RPRFM",              # Prefetch memory assembler symbols
    "SVEPRFM",            # Prefetch memory assembler symbols for SVE
    "SVEPREDPAT",         # SVE predicate assembler symbols
    "SVEVECLENSPECIFIER", # SVE vector length encoding assembler symbols
    "SVCR",               # Streaming Vector Control Register instruction assembler symbols
    "TLBI",               # Translation Lookaside Buffer Maintenance Instructions assembler symbols
    "TSB"                 # Trace Synchronization Barrier assembler symbols
  ]

  def _find_next_category(file):
    # Format: generated content <AArch64GenCSSystemOperandsEnum.inc:GET_ENUM_VALUES_XXXX> begin
    line_fmt = re.compile(r"AArch64GenCSSystemOperandsEnum.inc\:GET_ENUM_VALUES_(.+)?\>\s*(.+)$")
    for line in file:
      if "AArch64GenCSSystemOperandsEnum" in line:
        category = line_fmt.search(line).group(1)
        if category in ignored_categories:
          continue
        return category

  _ignored_registers = [
    # Armv8-m profile (microcontrollers) Memory Processing Unit (MPU) Protection Region registers
    # We can ignore these for now as we're unlikely to encounter microcontroller code. This helps
    # reduce the number of registers in Dyninst.
    "prbar",
    "prenr",
    "prselr",
    "prlar",
    "mpuir",
    
    # These appear to be internal assembler symbols for the Armv8.6 Enhanced Counter Virtualization extension
    "cntiscale",
    "cntscale",
    "cntvfrq",
    
    # Capstone marks this as FeatureAppleA7SysReg. I can't find it in the Armv8 spec.
    "cpm_ioacc_ctl",

    # D8.10.3 System and Special-purpose register aliasing
    "afsr0_el12",        # afsr0_el1
    "afsr1_el12",        # afsr1_el1
    "amair2_el12",       # amair2_el1
    "amair_el12",        # amair_el1
    "brbcr_el12",        # brbcr_el1
    "cntkctl_el12",      # cntkctl_el1
    "contextidr_el12",   # contextidr_el1
    "cpacr_el12",        # cpacr_el1
    "elr_el12",          # elr_el1
    "esr_el12",          # esr_el1
    "far_el12",          # far_el1
    "gcscr_el12",        # gcscr_el1
    "gcspr_el12",        # gcspr_el1
    "mair2_el12",        # mair2_el1
    "mair_el12",         # mair_el1
    "mpam1_el12",        # mpam1_el1
    "pfar_el12",         # pfar_el1
    "pire0_el12",        # pire0_el1
    "pir_el12",          # pir_el1
    "pmscr_el12",        # pmscr_el1
    "por_el12",          # por_el1
    "sctlr2_el12",       # sctlr2_el1
    "sctlr_el12",        # sctlr_el1
    "scxtnum_el12",      # scxtnum_el1
    "smcr_el12",         # smcr_el1
    "spmaccessr_el12",   # spmaccessr_el1
    "spsr_el12",         # spsr_el1
    "tcr2_el12",         # tcr2_el1
    "tcr_el12",          # tcr_el1
    "tfsr_el12",         # tfsr_el1
    "trcitecr_el12",     # trcitecr_el1
    "trfcr_el12",        # trfcr_el1
    "ttbr0_el12",        # ttbr0_el1
    "ttbr1_el12",        # ttbr1_el1
    "vbar_el12",         # vbar_el1
    "zcr_el12",          # zcr_el1
    "cntp_ctl_el02",     # cntp_ctl_el0
    "cntp_cval_el02",    # cntp_cval_el0
    "cntp_tval_el02",    # cntp_tval_el0
    "cntv_ctl_el02",     # cntv_ctl_el0
    "cntv_cval_el02",    # cntv_cval_el0
    "cntv_tval_el02",    # cntv_tval_el0
  ]

  def _excluded(name):
    if "_" in name:
      if name.lower() in _ignored_registers:
        return True
      
      # Transform names like 'prlar5_el1' to 'prlar'
      import string
      prefix = name[:name.rindex("_")].strip(string.digits)
      if prefix.lower() in _ignored_registers:
        return True
    return False

  def _read_category(file, category):
    fmt = "AARCH64_{0:s}_".format(category.upper())
    for line in file:
      if not fmt in line:
        continue
      # Format:  AARCH64_CATEGORY_NAME = 0xNUMBER,
      line = line.strip().replace(",", "")
      if "ENDING" in line:
        return
      name = line[len(fmt):line.find(' ')]
      if _excluded(name):
        continue
      yield name
  
  sysregs = {}
  with open(file, "r") as f:
    while True:
      category = _find_next_category(f)
      if category is None:
        break
      for reg in _read_category(f, category):
        sysregs[reg.lower()] = {"categories": [category]}
  
  return sysregs


def export_categories(f):
  names = sorted(_category_descriptions.keys())
  max_len = max([len(n) for n in names])

  # Special case for the mask
  decl = "const int32_t %-{0:d}s =          0x00800000;  // Base mask\n".format(max_len)
  f.write(decl % "SYSREG")
  
  decl = "const int32_t %-{0:d}s = SYSREG | 0x00%02X0000;  // %s\n".format(max_len)
  id = 0
  for n in names:
    # This causes a collision
    if id == 0x1A:
      id += 1
    f.write(decl % (n, id, _category_descriptions[n]))
    id += 1


def _get_name(content):
  return content.getElementsByTagName("reg_short_name")[0].childNodes[0].data


def _get_groups(content):
  groups = []
  groups_section = content.getElementsByTagName("reg_groups")[0].childNodes
  for s in groups_section:
    for g in s.childNodes:
      groups.append(g.data.lower())
  return groups


def _get_size(content):
  fieldsets = content.getElementsByTagName("reg_fieldsets")[0]
  fields = fieldsets.getElementsByTagName("fields")
  
  for f in fields:
    if f.getAttribute("id") == "fieldset_0":
      return f.getAttribute("length")

  # Try parsing it from the attribute text  
  attributes = content.getElementsByTagName("reg_attributes")[0]
  text = attributes.getElementsByTagName("attributes_text")[0]
  para = text.getElementsByTagName("para")[0].childNodes[0].data
  fmt = re.compile(r"is a (\d{2})-bit")
  matches = fmt.search(para)
  if matches:
    return matches.group(1)


def _get_categories(content):
  long_name = content.getElementsByTagName("reg_long_name")[0].childNodes[0].data.lower()
  
  def _unknown(group, long_name):
    raise Exception(
      "Unknown group '{0:s}' (long name '{1:s}'). Create a new category for it.".format(group, long_name)
    )

  categories = []
  for group in _get_groups(content):
    if group in _ignored_groups:
      continue
    cat = _categories_by_group[group]

    # There are more than 255 ungrouped and debug registers, so break them down further
    if cat in ("DBG", "UNKNOWN"):
      if long_name in _category_from_long_name:
        cat = _category_from_long_name[long_name]
      elif long_name[0:15] in _category_from_long_name:
        cat = _category_from_long_name[long_name[0:15]]
      else:
        (name, *_) = long_name.split(maxsplit=1)
        if name in _category_from_long_name:
          cat = _category_from_long_name[name]
        else:
          _unknown(group, long_name)
    elif cat == "OTHER":
      if long_name.lower().startswith("system control register"):
        cat = "SYSCTL"

    categories.append(cat)

  _merge_categories = [
    "SYSMEMORY",
    "IMPLDEF",
    "VIRT"
  ]
  for c in _merge_categories:
    if c in categories:
      categories = [c]
      break
  
  return categories


_ignored_groups = [
  # There are some classes of registers that are either not in Capstone
  # or we are highly unlikely to encounter. To reduce the overall number
  # of registers in Dyninst, ignore these.

  "cti",                # Cross-Trigger Interface (for embedded systems)
  "gic",                # Generic Interrupt Controller Architecture Specification
  "gic control registers",
  "gic host interface control registers",
  "gicc",
  "gicd",
  "gich",
  "gicr",
  "gicv",
  "gicv control",
  "gits",
]

_categories_by_group = {
  # Groups taken from the <reg_group> fields in the documentation
  "address": "ADDR",
  "amu": "AMR",
  "brbe": "BRBE",
  "brbe instructions": "BRBE",
  "debug": "DBG",
  "debug secondary": "DBG",
  "exception": "EXCP",
  "float": "SYSFLOAT",
  "generic system control": "SYSCTL",
  "guarded control stack registers": "GCSR",
  "identification registers": "SYSID",
  "imp def": "IMPLDEF",
  "memory": "SYSMEMORY",
  "mpam": "MPAM",
  "other": "OTHER",
  "pmu": "PMU",
  "pointer authentication": "SYSPTR",
  "pstate": "PSTATE",
  "ras": "RAS",
  "reset management": "RESET",
  "root": "SYSROOT",
  "secure": "SEC",
  "spe": "STATPROF",
  "special": "SYSSPR",
  "thread": "THRD",
  "timer": "SYSTIMER",
  "trace": "TRACE",
  "trace management": "TRACE",
  "trace unit instructions": "TRACE",
  "trbe": "TRACE",
  "unknown": "UNKNOWN",
  "virt": "VIRT",
}

_category_from_long_name = {
  'accelerator data': 'AD',
  'activity monitors registers': 'AMR',
  'amr': 'AMR',
  'auxiliary': 'SYSAUX',
  'branch record b': 'BRBE',
  'breakpoint and ': 'BAWS',
  'debug': 'DBG',
  'debug authentication status register': 'DBGAUTH',
  'debug breakpoint control registers': 'DBGBRK',
  'debug breakpoint value registers': 'DBGBRK',
  'debug claim tag clear register': 'DBGCT',
  'debug claim tag set register': 'DBGCT',
  'debug data transfer register, half-duplex': 'DBGDTR',
  'debug data transfer register, receive': 'DBGDTR',
  'debug data transfer register, transmit': 'DBGDTR',
  'debug link register': 'DBGLR',
  'debug power control register': 'DBGPCR',
  'debug registers': 'DBG',
  'debug saved program status register': 'DBGSPSR',
  'debug vector catch register': 'DVCR',
  'debug watchpoint control registers': 'DBGW',
  'debug watchpoint value registers': 'DBGW',
  'external debug ': 'EXTDBG',
  'fine-grained write traps el3': 'FWTE',
  'float': 'SYSFPR',
  'generic timer registers': 'SYSTIMER',
  'hypervisor': 'HYPRDBG',
  'management registers': 'MGMT',
  'monitor': 'SYSMON',
  'mpam virtual partid mapping register 3': 'MPAM',
  'os': 'SYSOS',
  'permission indirection register 0 (el2)': 'PIR',
  'performance mon': 'PMU',
  'physical fault ': 'PHYSFAR',
  'process state registers': 'PSTATE',
  'ras registers': 'RAS',
  'reset management registers': 'RESET',
  'root security state registers': 'SYSROOT',
  'security registers': 'SEC',
  'selected error record group status register': 'SERGS',
  'special-purpose registers': 'SYSSPR',
  'statistical profiling extension registers': 'STATPROF',
  'system performa': 'PMU',
  'system pmu counter group configuration register <n>': 'PMU',
  'system pmu implementation identification register': 'PMU',
  'thread and process id registers': 'THRD',
  'trace buffer extension registers': 'TRACE',
  'trace filter control register (el1)': 'TRACE',
  'trace filter control register (el2)': 'TRACE',
  'virtual memory control registers': 'VIRT',
  'virtualization registers': 'VIRT'
}

_category_descriptions = {
  "AD": "accelerator data",
  "ADDR": "address",
  "AMR": "activity monitors",
  "BAWS": "breakpoint and watchpoint selection",
  "BRBE": "branch record buffer extension",
  "DBG": "debug",
  "DBGAUTH": "debug authentication",
  "DBGBRK": "debug breakpoint management",
  "DBGCT": "debug claim tag",
  "DBGDTR": "debug data transfer",
  "DBGLR": "debug link register",
  "DBGPCR": "debug power control",
  "DBGSPSR": "debug saved program status",
  "DBGW": "debug watchpoint",
  "DVCR": "debug vector catch",
  "EXCP": "exception",
  "FWTE": "fine-grained write traps el3",
  "GCSR": "guarded control stack registers",
  "HYPRDBG": "hypervisor debug fine-grained",
  "IMPLDEF": "implementation defined",
  "MPAM": "memory partitioning and monitoring extension",
  "OTHER": "other system control",
  "PHYSFAR": "physical fault address",
  "PMU": "performance monitors extension",
  "PSTATE": "process state",
  "RAS": "reliability, availability, and serviceability extension (RAS)",
  "RESET": "reset management",
  "SEC": "security for access to exception levels",
  "STATPROF": "statistical profiling extension",
  "SYSCTL": "system control",
  "SYSFLOAT": "system floating-point",
  "SYSID": "system identification",
  "SYSMEMORY": "system memory",
  "SYSMON": "system monitor",
  "SYSOS": "system OS lock/access/data/control",
  "SYSPTR": "pointer authentication",
  "SYSSPR": "system special-purpose",
  "SYSTIMER": "system timers",
  "THRD": "threading",
  "TRACE": "system trace",
  "VIRT": "virtualization",
}
