# Tutorial: "Decoding RDNA Instructions with IsaDecoder" #

## Motivation ##

AMD recently released its machine-readable GPU ISA specification - a set of XML files describing its RDNA and CDNA Instruction Set Architectures. While you can parse the XML files yourself, the easiest way to consume the specification is using the `IsaDecoder` API: Given an XML specification file, the API can read and parse it for you and even decode single instructions or whole shaders.

This tutorial demonstrates how to use the `IsaDecoder` API to decode AMD GPU assembly provided in either binary or textual (disassembly) format.

## `IsaDecoder` ##

The API allows you to decode either a single instruction or a whole shader/kernel.

The source code for the API can be found on the [isa_spec_manager GitHub repository](https://github.com/GPUOpen-Tools/isa_spec_manager).

## Getting started - `IsaDecoder` ##

Step 1: Instantiate `IsaDecoder`.

`IsaDecoder` is defined under the namespace amdisa. For this tutorial, we define an `IsaDecoder` object named "api_decoder". 
```c
amdisa::IsaDecoder api_decoder;
```

Step 2: Initialize the `IsaDecoder` object with an input XML specification file. Make sure that the XML file you are using matches the GPU architecture that you are about to decode instructions for. For instance, use the MI-300 XML file to decode MI-300 kernels.

The `Initialize()` API function reads and parses the given XML specification file. Upon success, the `IsaDecoder` object is ready to decode instructions.
```c
bool is_success = api_decoder.Initialize(kPathToSpec, error_msg);
```

## 1. Decoding a single instruction ##

### 1.1 Decoding a single instruction in binary format ###

Consider the following binary representation of an RDNA instruction: `8BEA7E6A`. Let's use the API to decode it.

Step 1: Create a variable to store the binary representation.
```c
const std::string kStrSampleInstructionBinary = "8BEA7E6A"; 
```

Step 2: Convert the binary representation to its decimal equivalent.

We convert the hexadecimal value to its unsigned int (decimal) equivalent with the help of std::stringstream as shown below. This is, of course, only necessary for the sake of this example. In the wild, you will be able to use the API directly on binary machine code obtained from AMD GPU Code Objects.
```c
std::stringstream IsaInstructionStream;
IsaInstructionStream << kStrSampleInstructionBinary;
uint64_t instruction_binary = 0;
IsaInstructionStream >> std::hex >> instruction_binary;
```

**Note**: To use std::stringstream, we will have to ```#include <sstream>```

Step 3: Create an empty `InstructionInfoBundle`<sup>[2]</sup>.

To obtain the decoded information, we pass an empty data structure `amdisa::InstructionInfoBundle`.
```c
amdisa::InstructionInfoBundle instruction_info_bundle;
```

Step 4: Call the `DecodeInstruction()` API function.

We then proceed to provide the decimal equivalent `instruction_binary`, `instruction_info_bundle`, and `error_msg` to `DecodeInstruction()` which returns true on successful decode and false if the decoding fails.
```c    
bool is_success = api_decoder.DecodeInstruction(instruction_binary, instruction_info_bundle, error_msg);
```

**Note**: It is possible to provide a decimal equivalent directly to the DecodeInstruction() API. Skip 2 & 3.

Output:
    Instruction Name: S_AND_B64
    Instruction Description: Bitwise AND.
    Encoding Name: ENC_SOP2
    Encoding Description: SCALAR ALU OPERATIONS WITH ONE DESTINATION AND TWO SOURCES.
        ALLOWED PATTERNS:
        SOP2 (32 BITS)
        SOP2 + LITERAL (64 BITS)

`8BEA7E6A` decodes to a scalar ALU instruction (`S_AND_B64`) that performs bitwise AND operation. The instruction's encoding is `ENC_SOP2` implying a scalar ALU operation with one destination and two sources.

### 1.2 Decoding a single instruction in texual format ###

Now, let's learn how the API can be used to decode an instruction's disassembly to retrieve its human-readable description. Consider the following textual representation of an RDNA instruction: `v_mov_b32`.

Step 1: Create a variable to store the instruction name.
```c
const std::string kStrSampleInstruction = "v_mov_b32";
```

Step 2: Create an empty `InstructionInfo`<sup>[1]</sup> struct variable.
```c
amdisa::InstructionInfo instruction_info;
```

Step 3: Call the `DecodeInstruction()` API function.

We then proceed to provide the instruction name `kStrSampleInstruction`, `instruction_info1, and `error_msg` to `DecodeInstruction()`. Returns true on successful decode and false if the decoding fails. The failure reason is populated in the error_msg.
```c    
bool is_success = api_decoder.DecodeInstruction(kStrSampleInstruction, instruction_info, error_msg);
```

Output:
On a successful decode, we get the following information from instruction_info.
    
    Instruction Name: V_MOV_B32
    Instruction Description: Move data to a VGPR.

## 2. Decoding a whole shader ##

### 2.1 Decoding a whole shader in binary format ###

The API accepts and decodes a whole shader in the form of binary stream of instructions. Let's have a look at the following binary stream of instructions:
```
`8BEA7E6A D6130002 00884D02`
```

Step 1: Create a variable to store the sample instructions in binary format.
```c
std::string sample_instructions_binary = "8BEA7E6A D6130002 00884D02";
```

Step 2: Convert the sample instructions to their respective binary format and store in a std::vector\<uint32_t\>. This is, of course, only necessary for the sake of this example. In practice you can use this API call directly on sequences of AMD GPU assembly instructions. 
```c
std::vector<uint32_t> sample_instructions;
std::stringstream SampleInstructionStream(sample_instructions_binary);
uint32_t instruction_binary;
while (SampleInstructionStream >> std::hex >> instruction_binary) {
    sample_instructions.push_back(instruction_binary);
}
```

Step 3: Create a vector of `InstructionInfoBundle`<sup>[3]</sup>.
To obtain the decoded information, we pass an empty vector of `amdisa::InstructionInfoBundle`.
```c
std::vector<amdisa::InstructionInfoBundle> instruction_info_bundle;
```

Step 4: Call the `DecodeInstructionStream()` API

We then proceed to provide the vector of sample instructions in binary format `SampleInstructions`, `instruction_info_bundle`, and `error_msg` to `DecodeInstructionStream()` API. 
```c
bool is_success = api_decoder.DecodeInstructionStream(SampleInstructions, instruction_info_bundle, error_msg);
```

Output:
    Instruction Name: S_AND_B64
    Instruction Description: Bitwise AND.
    Encoding Name: ENC_SOP2

    Instruction Name: V_FMA_F32
    Instruction Description: Fused single precision multiply add.
    Encoding Name: ENC_VOP3

**Note**: The `V_FMA_F32` instruction is represented using two dwords (2 x 32-bit). Thus, from the given sample instructions of the shader, two instructions are decoded.

### 2.2 Decoding a whole shader in disassembly format ###

The API can also decode a whole shader in AMD's disassembly format. Consider decoding the shader disassembly file "example_shader_disassembly".

Step 1: Create a variable to store the shader disassembly file path.
```c
const std::string kPathToShaderFile = "/path/to/example_shader_disassembly";
```

Step 2: Create a vector of `InstructionInfoBundle`<sup>[3]</sup> variable.
To obtain the decoded information, we pass an empty data structure `amdisa::InstructionInfoBundle`.
```c
amdisa::InstructionInfoBundle instruction_info_stream;
```

Step 3: Call the DecodeInstruction() API function.

We then proceed to provide the path to the shader disassembly file, `instruction_info_strea`, `error_msg`, and `resolve_direct_branch_targets` to `DecodeShaderText()`.
```c
bool is_success = api_decoder.DecodeShaderText(kPathToShaderFile, instruction_info_stream, error_msg, resolve_direct_branch_targets);
```

**Note**: `resolve_direct_branch_targets` is an optional parameter. Setting it to `true` enables determining the direct branch's target instruction's index in the `instruction_info_bundle` vector which will be available in the `InstructionInfo.instruction_semantic_info.branch_info` struct's member `branch_target_index`. For example, in the following shader:
    
    s_branch      label_05F4                              // 000000000000: BFA00004
    label_05E4:
    v_mov_b32     v28, 0                                  // 000000000004: 7E380280
    label_05F4:
    v_fma_f32     v0, s10, s10, v0                        // 000000000008: D6130000 0400140A

The target instruction index of the s_branch instruction will be 2, i.e, the third instruction v_fma_f32. Indexing in the instruction_info_bundle starts from 0.

## Appendix ##

### 1. amdisa::InstructionInfo ###

This output data structure is used when we decode the instruction by name.

amdisa::InstructionInfo struct type that consists of the following information:
- Instruction
    - instruction_name: Instruction's name
    - instruction_description: Brief description about the instruction
- Encoding
    - encoding_name: Instruction's encoding
    - encoding_description: Brief description about the encoding
- encoding_fields: Information about the fields in the encoding
    - field_name
    - field_value
    - bit_count
    - bit_offset
- encoding_layout: Representation of the encoding's layout
- Operands (instruction_operands): Information about the instruction's operands
    - operand_name
    - operand_size
    - is_input
    - is_output
- Modifiers (operand_modifiers): Information about the instruction's operand's modifiers
    - modifier_name
    - value
- Semantics (instruction_semantic_info): Information about the instruction's semantics
    - is_program_terminator
    - is_immediately_executed
    - branch_info
        - is_branch
        - is_conditional
        - is_indirect
        - branch_offset
        - branch_target_PC
        - branch_target_index

**Note**: This hierarchy resembles the nested structures (struct in a struct).

For example, consider the instruction 'S_AND_B64', following information can be fetched on a successful decode.
```
Instruction Name: S_AND_B64
Instruction Description: Bitwise AND.
Encoding Name: ENC_SOP2
Encoding Description: Scalar ALU operations with one destination and two sources. Allowed patterns:
    SOP2 (32 bits)
    SOP2 + LITERAL (64 bits)
Encoding Layout:
    ENCODING     [31:30] = 2
    OP           [29:23] = 17
    SDST         [22:16] = 6a
    SSRC0        [ 7: 0] = 6a
    SSRC1        [15: 8] = 7e

Encoding Fields:
Field Name    Value         Bit Count     Bit Offset
ENCODING      2             2             30
OP            23            7             23
SDST          106           7             16
SSRC0         106           8             0
SSRC1         126           8             8
Instruction Operands:
Operand    Size    Is_Input   Is_Output
vcc_lo     64      no         yes
vcc_lo     64      yes        no
exec_lo    64      yes        no
Operand Modifier:
    None
Instruction Semantic Info:
    Is_Program_Terminator: no
    Is_Immediately_Executed: no
    Is_Branch: no
    Is_Conditional: no
    Is_Indirect: no
    Branch Offset: 65536
    Branch Target Index: -1
```

### 2. amdisa::InstructionInfoBundle ###

This output data structure is used when we decode the binary representation of the instruction.
`amdisa::InstructionInfoBundle` is a vector of `amdisa::InstructionInfo`. The reason why this is a vector is to support encodings that pack multiple instructions into a single instruction (although in most cases you should expect only a single instruction in the bundle). 

### 3. A vector of amdisa::InstructionInfoBundle ###

This output vector is used when a stream of instructions is provided to decode. On a successful decode, the decoded instruction infos are populated in this vector in the same order as provided. The first instruction's info index in the vector is 0.