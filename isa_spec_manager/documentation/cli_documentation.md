# ISA Spec Manager CLI
ISA Spec Manager CLI program is an example program that leverages IsaSpecApi. The main purpose of this program is to wrap the API and provide a platform to showcase the features of the API.

## Binary instruction decoding
To decode an instruction that is represented in the binary (machine code) format using CLI, run the commands below. 

```bash
cd IsaSpecManager-Win64/cli
./IsaSpecCli.exe -x ../../Specification/xml/gfx11.xml -d BE804814
```
As can be seen from the command, `-x` flag should be followed by the path to the machine-readable XML specification. Next, `-d` flag should be followed by the instruction in the binary form. Note that the provided instruction should be encoded in the specified architecture. After running the command, the CLI outputs the following:

```bash
Info: Parsing XML file...
Info: XML parsing completed successfully.
Info: Decoding...
Info: Decoding completed successfully.

Instruction: S_SETPC_B64 s20
Description:
Jump to a new location.  Argument is a byte address of the
instruction to jump to.

 Encoding: ENC_SOP1
    ENCODING     [31:23] = 17d
    OP           [15: 8] = 48
    SDST         [22:16] = 0
    SSRC0        [ 7: 0] = 14
```
If decoded successfully, CLI outputs the instruction in an assembly form, description of the instruction and followed by the breakdown of the instruction's encoding.

## Requesting instruction information (by name)
To retrieve information about a specific instruction from the spec, the name of the instruction could be provided to the CLI. Refer to the commands below.

```bash
cd IsaSpecManager-Win64/cli
./IsaSpecCli.exe -x ../../Specification/xml/gfx11.xml -i s_setpc_b64
```
As can be seen from the command, `-x` flag should be followed by the path to the machine-readable XML specification. Next, `-i` flag should be followed by the name of the instruction which we are interested in. Note, that the requested instruction should be present in the specified architecture. After running the command, CLI outputs the following:

```bash
Info: Parsing XML file...
Info: XML parsing completed successfully.
Info: Retrieving instruction information...
Info: Instruction information retrieved successfully.

Instruction: S_SETPC_B64
Description:
Jump to a new location.  Argument is a byte address of the instruction to jump to.
```
If data was retrieved successfully, CLI outputs the full description of the instruction.
