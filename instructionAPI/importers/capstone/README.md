# Capstone importer for registers and instructions

Imports for PowerPC are not yet supported.

## Usage

### Registers

Register processing is done with `import_registers.py`. Its options are described below.

>   --capstone-dir CAPSTONE_DIR
>                         Capstone source directory (e.g., /capstone-engine/capstone/)
>
>   --dyninst-dir DYNINST_DIR
>                         Dyninst source directory (e.g., /dyninst/src/)
>
>   --spec-dir SPEC_DIR   Location of XML specification files (required for aarch64)
>
>   --arch {x86,aarch64,ppc}

`--spec-dir` is only required for Aarch64.


### Instructions/Mnemonics

Mnemonic processing is done with `import_mnemonics.py`. Its options are described below.

>   --capstone-dir CAPSTONE_DIR
>                         Capstone source directory (e.g., /capstone-engine/capstone/)
>
>   --dyninst-dir DYNINST_DIR
>                         Dyninst source directory (e.g., /dyninst/src/)
>
>   --arch {x86,aarch64,ppc}

The x86 register processor doesn't do anything yet because they were previously manually imported.


## Adding a new architecture

### Registers

1. Add a new directory <arch>
2. Under this directory, add the files
  a. __init__.py  (can be empty)
  b. register.py
  c. mnemonics.py
3. Add `<arch>.registers` import at top of `import_registers.py`
4. In <arch>/register.py, implement a class called `registers`

This class has the following structure:

```
class registers:
  def __init__(self, cap_dir:str, dyn_dir:str)
  
    cap_dir - path to Capstone
    dyn_dir - path to Dyninst
  
  @staticmethod
  def export_lengths(file_handle)
  
    file_handle - handle as returned by `open`
    
    Writes the C++ declarations for the register sizes to `file_handle`.
  
  @staticmethod
  def export_categories(file_handle)

    file_handle - handle as returned by `open`
    
    Writes the C++ declarations for the register categories to `file_handle`.
  
  @staticmethod
  def export_dwarf(file_handle)
  
    file_handle - handle as returned by `open`
    
    Writes the C++ declarations for the DWARf declarations to `file_handle`.


  capstone : list of `str`
  
    non-system registers from Capstone
   
    These are the names of general-purpose, floating-point (vector and scalar), and control
    registers. They have Capstone names with the form Aarch64_REG_<name>.

  capstone_sysregs : list of `<Register>` type
  
    system registers from Capstone
  
    These either have a Capstone name of the form Aarch64_SYSREG_<name> or are
    derived from the system operand tablegen files (e.g., AArch64_DC_<name>).
    
    `<Register>` is a dict where the key is the register name and the value is a dict with
    a single key `categories` that is a list of the category names to which the register
    belongs.
  
  dyninst : list of `str`
  
    registers declared in Dyninst
  
    These are only used to determine which registers have been added after processing
    the new files from Capstone.
    
  spec_sysregs : list of `<Register>` type
  
    system registers from the ARM64 System Register Spec files
  
    These are from the files in
    https://developer.arm.com/Architectures/A-Profile%20Architecture#Software-Download
    
    `<Register>` is a dict where the key is the register name and the value is a dict with
    keys `categories` that is a list of the category names to which the register
    belongs and `size` that is the number of bits the register holds.
    
  all : list of `<Register>` type
  
    A list of registers to be imported into Dyninst.
    
    `<Register>` is a list of dicts with keys `name` that is the register name, `categories`
    that is a string of the category names to which the register belongs concatenated
    together using with the base string ' | ', and `size` that is a string containing the
    name of the bitmask for the size of the register (e.g., "FULL").
  
  aliases : list of dict
  
    Registers with different names for the same physical register
    
    The alias is stored in the "alias" key, the primary under "primary".
    
    The "primary" key is the one Capstone uses. For example, Capstone calls the Aarch64
    link register Aarch64_REG_LR, but the ISA specifies it as x30. The primary name would
    then be "LR" and the alias name "x30".
  
```

### Mnemonics


