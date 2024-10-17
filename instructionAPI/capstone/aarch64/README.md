# Registers for Aarch64

Aarch64 has three different [profiles](https://www.arm.com/architecture/cpu): application (A), realtime (R), and microcontroller (M). We only support the A profile even though Capstone has a small amount of support for the other two because it ultimately derives its functionality from LLVM.

The core documentation used to generate the rules implemented herein is the April 2023 version of the [ARM](https://developer.arm.com/documentation/ddi0487/ka/).

## Base registers

These are general-purpose, special-purpose, floating-point, and vector registers and include the Scalable Vector Extension from Armv8,9 and the Scalable Matrix Extensions from Armv9.


## System registers

Aarch64 has a profound number of system registers across many extensions- numbering into the tens of thousands when including all of the numbered registers which are technically directly addressable but really serve as a register file. For example, there are sixteen variants of `amevcntvoff0<N>_el2`. Because of this, a lot of effort is included here to exclude as many unnecessary registers as possible.

The table below outlines which instructions can be used to address the included system registers.

### Instructions

Taken from Arm Architecture Reference Manual (ARM) for A-profile architecture
April 2023
ARM DDI 0487J.a (ID042523)
https://developer.arm.com/documentation/ddi0487/ka

| mnemonic(s) | description |
|--------|--------|
| MRS | Move System Register |
|MSRI, MSR[immediate] | Move immediate value to Special Register |
|MSRR, MSR[register] | Move general-purpose register to System Register |
|AT | Address Translate |
|ES | Error Synchronization |


| Category | Instruction | Section | Count
|--------|--------|--------|--------|
| system trace, (TRACE) | MRS, MSR | ARM D6 | 227 |
| performance monitors extension, (PMU) | MRS, MSR | ARM A2.13 | 202 |
| branch record buffer extension, (BRBE) | MRS, MSR | ARM D16 | 104 |
| activity monitors, (AMR) | MRS | ARM D19.6 | 81 |
| virtualization, (VIRT) | MRS, MSR | ARM D8 | 54 |
| system identification, (SYSID) | MRS, MSR |  | 48 |
| system memory, (SYSMEMORY) | MRS, MSR | Armv8.9 system registers | 48 |
| system control, (SYSCTL) | MRS, MSR | ARM D19.2 | 24 |
| system timers, (SYSTIMER) | MRS, MSR | ARM D11.1 | 24 |
| reliability, availability, and serviceability extension, (RAS) | ES | ARM C6.2 | 18 |
| memory partitioning and monitoring extension, (MPAM) | MRS, MSR | ARM A2.18 | 15 |
| statistical profiling extension, (STATPROF) | MRS, MSR | ARM A2.15 | 14 |
| system special-purpose, (SYSSPR) | MRS, MSR | | 13 |
| pointer authentication, (SYSPTR) | MRS, MSR | ARM D8.8 | 10 |
| process state, (PSTATE) | MRS, MSR | ARM D1.4 | 11 |
| hypervisor debug fine-grained, (HYPRDBG) | MRS, MSR | ARM C6.2.128 | 10 |
| other system control, (OTHER) |  |  | 9 |
| threading, (THRD) | MRS, MSR | ARM D19.2 | 9 |
| exception, (EXCP) | MRS, MSR | ARM D1.3 | 8 |
| system OS lock/access/data/control, (SYSOS) | MRS, MSR | ARM H3.6 | 6 |
| reset management, (RESET) | MRS, MSR | ARM D19.2 | 5 |
| security for access to exception levels, (SEC) | implicit | ARM D1.3 | 5 |
| system floating-point, (SYSFLOAT) | MRS, MSR | | 5 |
| system monitor, (SYSMON) | MRS, MSR | | 4 |
| physical fault address, (PHYSFAR) | MRS, MSR | ARM D19.10 | 2 |
| accelerator data, (AD) | AT | ARM C5.4.1 | 1 |
| breakpoint and watchpoint selection, (BAWS) | MRS, MSR | ARM D2 | 1 |
| debug vector catch, (DVCR) | MRS, MSR | ARM D19 | 1 |

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

