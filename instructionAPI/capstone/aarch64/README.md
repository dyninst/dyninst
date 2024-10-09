# Registers for Aarch64

Aarch64 has three different [profiles](https://www.arm.com/architecture/cpu): application (A), realtime (R), and microcontroller (M). We only support the A profile even though Capstone has a small amount of support for the other two because it ultimately derives its functionality from LLVM.

The core documentation used to generate the rules implemented herein is the April 2023 version of the [ARM](https://developer.arm.com/documentation/ddi0487/ka/).

## Base registers

These are general-purpose, special-purpose, floating-point, and vector registers and include the Scalable Vector Extension from Armv8,9 and the Scalable Matrix Extensions from Armv9.


## System registers

Aarch64 has a profound number of system registers across many extensions- numbering into the tens of thousands when including all of the numbered registers which are technically directly addressable but really serve as a register file. For example, there are sixteen variants of `amevcntvoff0<N>_el2`. Because of this, a lot of effort is included here to exclude as many unnecessary registers as possible.

The table below outlines which instructions can be used to address the included system registers.

### Instructions

| mnemonic(s) | description |
|--------|--------|
| MRS | Move System Register |
|MSRI, MSR[immediate] | Move immediate value to Special Register |
|MSRR, MSR[register] | Move general-purpose register to System Register |
|AT | Address Translate |
|ES | Error Synchronization |

| Category | Instruction | Section |
|--------|--------|--------|
| AD | AT | ARM C5.4.1 |
| AMR | MRS | ARM D19.6 |
| BAWS | MRS,MSR | ARM D19.3 |
| BRBE | MRS,MSR | ARM D19.8 |
| DVCR | MRS,MSR | ARM D19.3.10 |
| EXCP | MRS,MSR | ARM D19.2 |
| HYPER | MRS,MSR | ARM D19.2 |
| MPAM | MRS,MSR | MPAM 7.1 |
| OTHER |  |  |
| PHYSFAR | MRS,MSR | ARM D19.10 |
| PMU | MRS,MSR | ARM D12 |
| PSTATE | MRS,MSR | ARM D1.4 |
| RAS | ES | ARM C6.2 |
| RESET | MRS,MSR | ARM D19.2 |
| SEC | implicit | ARM D1.3 |
| STATPROF | MRS,MSR | ARM D19.7 |
| SYSCTL | MRS,MSR | ARM D19.2 |
| SYSFLOAT | MRS,MSR | ARM D19.2 |
| SYSID | MRS,MSR | ARM D19.2 |
| SYSMEMORY | MRS,MSR | ARM D19.2 |
| SYSMON | MRS,MSR | ARM D19.3 |
| SYSOS | MRS,MSR | ARM D19.3 |
| SYSPTR | MRS,MSR | ARM D19.2 |
| SYSSPR | MRS,MSR | ARM C5.2 |
| SYSTIMER | MRS,MSR | ARM C19.12 |
| THRD | MRS,MSR | ARM C19.2 |
| TRACE | MRS,MSR | ARM C19.2 |
| VIRT | MRS,MSR | ARM C19.2 |


We explicitly ignore Aarch32-only registers.


## Ignored registers

Many categories of registers (particularly system ones) are explicitly not included in Dyninst because they are either not in the A-profile architecture or are extremely unlikely to be enountered. They include:

- Pseudo-registers (those that don't represent physical registers)

- External Debug extension (EXTDBG* categories)

- Many internal categories in Capstone (see `parse_capstone` in sysregs.py)

- Cross-Trigger Interface (for embedded systems, CTI category)

- Generic Interrupt Controller Architecture Specification (GIC* categories)


## DWARF encodings

This generates the bidirectional mappings between DWARF register numbers and the Dyninst registers based on

DWARF for the Arm 64-bit Architecture
6th October 2023
4.1 DWARF register names
https://github.com/ARM-software/abi-aa/releases/download/2023Q3/aadwarf64.pdf



## Capstone to Dyninst register mappings

This is not yet implemented, but will be required in order to translate between Capstone and MachRegister. See the stub function `_capstone_to_dyninst` in registers.py for notes.

