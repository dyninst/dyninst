# `IsaDecoder` documentation
The `IsaDecoder` API makes it easier to perform common tasks with the AMD GPU machine-readable ISA specifications, such as reading the XML files, decoding instructions, and retrieving information about decoded instructions.

This document describes the API.

# API functions
## `amdisa::IsaDecoder::Initialize`
```c++
bool Initialize(const std::string& input_xml_file_path, std::string& err_message);
```
This routine initializes an `IsaDecoder` instance. It reads in a machine-readable XML ISA Specification and populates the internal data structures.

###  Parameters
parameter name | parameter type | description | input/output
-|-|-|-
input_xml_file_path | `const std::string&` | Path to the XML ISA Specification | input
err_message | `std::string&` | Error message (set in case of a failure) | output

### Return value
A boolean value: true on success, false otherwise.

### Example
```c++
#include "isa_decoder.h"

int main ()
{
    amdisa::IsaDecoder spec_api;
    std::string err_message;
    bool is_init = spec_api.Initialize("C:/gfx11.xml", err_message);
    if (!is_init)
    {
        std::cerr << err_message << std::endl;
    }
    return (is_init ? 0 : -1);
}
```

## `amdisa::IsaDecoder::GetVersion`
  
```c++
std::string GetVersion() const
```
Gets the API version.

### Parameters
No parameters accepted.

### Return value
The function returns a std::string which contains the API version in <major.minor.patch> format.

### Example
```c++
#include "isa_decoder.h"

int main ()
{
    amdisa::IsaDecoder decoder_api;
    std::cout << "API version: ";
    std::cout << decoder_api.Major << ".";
    std::cout << decoder_api.Minor << ".";
    std::cout << decoder_api.Patch << ".";
    return 0;
}
```

## `amdisa::IsaDecoder::DecodeInstructionStream`
```c++
bool DecodeInstructionStream(
	const std::vector<uint32_t>& machine_code_stream,
    std::vector<InstructionInfoBundle>& instruction_info_stream,
    std::string& err_message) const;
```
Decodes a sequence of binary instructions. 

### Parameters
parameter name | type | description | input/output
-|-|-|-
machine_code_stream | `const std::vector<uint32_t>&` | Vector of encoded instructions in binary format. | input
instruction_info_stream | `std::vector<InstructionInfoBundle>&` | Vector of InstructionInfoBundle structures that provides detailed information about each decoded instruction. Refer to [API structures](#api-structures) section for the definition the structure. | output
err_message | `std::string&` | Error message (set in case of a failure) | output

### Return value
A boolean value: true on success, false otherwise.

### Example
```c++
#include "isa_decoder.h"

int main ()
{
    amdisa::IsaDecoder spec_api;
    std::string err_message;
    bool is_init = spec_api.Initialize("C:/gfx11.xml", err_message);
    if (is_init)
    {
        // Instructions in binary.
        std::vector<uint32_t> instructions = {0xb0804006, 0xbefe0017};

        // Structure to hold the result of the decode.
        std::vector<InstructionInfoBundle> instruction_infos;

        // Decode.
        bool is_decoded = amdisa::DecodeInstructionStream(instructions,
            instruction_infos, err_message);
        if (is_decoded)
        {
            // Do stuff...
        }
        else
        {
            std::cerr << err_message << std::endl;
        }
    }
    else
    {
	    std::cerr << err_message << std::endl;
    }
    
    return (is_init && is_decoded) ? 0 : -1;
}
```

## `amdisa::IsaDecoder::DecodeInstruction` - binary
```c++
bool DecodeInstruction(uint64_t machine_code, 
    InstructionInfoBundle& instruction_info,
    std::string& err_message) const;
```
Decodes a single instruction encoded in binary format. This API is limited to a 64-bit instruction. If the instruction of interest is longer than 64 bits, `amdisa::IsaDecoder::DecodeInstructionStream` should be used instead.

### Parameters
parameter name | type | description | input/output
-|-|-|-
machine_code_stream | `uint64_t` | A single instruction encoded in binary. | input
instruction_info_stream | `InstructionInfoBundle&` | A single `InstructionInfoBundle` structure that provides detailed information about the decoded instruction. Refer to [API structures](#api-structures) section for the definition the structure. | output
err_message | `std::string&` | Error message (set in case of a failure) | output

### Return value
A boolean value: true on success, false otherwise.

### Example
```c++
#include "isa_decoder.h"

int main ()
{
    amdisa::IsaDecoder spec_api;
    std::string err_message;
    bool is_init = spec_api.Initialize("C:/gfx11.xml", err_message);
    if (is_init)
    {
        // Instructions in binary.
        uint64_t single_instruction = 0xb0804006;;

        // Structure to hold the result of the decode.
        InstructionInfoBundle single_instruction_info;

        // Decode.
        bool is_decoded = amdisa::DecodeInstruction(
            single_instruction, single_instruction_info, err_message);
        if (is_decoded)
        {
            // Do stuff...
        }
        else
        {
            std::cerr << err_message << std::endl;
        }
    }
    else
    {
	    std::cerr << err_message << std::endl;
    }
    
    return (is_init && is_decoded) ? 0 : -1;
}
```

## `amdisa::IsaDecoder::DecodeInstruction` - textual
```c++
bool DecodeInstruction(const std::string& instruction_name,
    InstructionInfo& instruction_info,
    std::string& err_message) const;
```
Given the name of the instruction (also known as the opcode name), the function outputs the information about the instruction.

### Parameters
parameter name | type | description | input/output
-|-|-|-
instruction_name| `const std::string&` | An instruction name as defined in ISA | input
instruction_info | `InstructionInfo&` | A single `InstructionInfo` structure that give information about the instruction. Refer to [API structures](#api-structures) section for the definition the structure. | output
err_message | `std::string&` | Error message (set in case of a failure) | output

### Example
```c++
#include "isa_decoder.h"

int main ()
{
    amdisa::IsaDecoder spec_api;
    std::string err_message;
    bool is_init = spec_api.Initialize("C:/gfx11.xml", err_message);
    if (is_init)
    {
        // Structure to hold the result of the decode.
        InstructionInfo instruction_info;

        // Decode.
        bool is_decoded = amdisa::DecodeInstruction("S_MOV_B32",
            instruction_info, err_message);
        if (is_decoded)
        {
            // Do stuff...
        }
        else
        {
            std::cerr << err_message << std::endl;
        }
    }
    else
    {
	    std::cerr << err_message << std::endl;
    }
    
    return (is_init && is_decoded) ? 0 : -1;
}

```
# API structures
API defines various struct in `isa_decoder.h`.

## `amdisa::InstructionInfo`
Through this structure `IsaDecoder` communicates information about a single decoded instruction.

### Definition
```c++
struct InstructionInfo {
    // Name of the instruction (also known as the opcode).
    std::string instruction_name;

    // Description of an instruction.
    std::string instruction_description;

    // Encoding of an instruction.
    std::string encoding_name;

    // Description of the encoding an instruction belongs to.
    std::string encoding_description;

    // Human-readable information about all of the fields and corresponding
    // encoded field values as a single string.
    std::string encoding_layout;

    // Parsable information about all of the fields.
    std::vector<EncodingField> encoding_fields;

    // All of the operands involved in a given instruction.
    std::vector<InstructionOperand> instruction_operands;

    // Modifiers applied to the operands.
    std::vector<OperandModifer> operand_modifiers;

    // Semantic information of an instruction.
    InstructionSemanticInfo instruction_semantic_info;
};
```

This structure instantiates the hierarchy of other structures with self-explanatory struct names and member variable names. Please refer to `isa_decoder.h` for the definition of all structures.

## `amdisa::InstructionInfoBundle`
This structure holds information about several instructions (two or more `InstructionInfo`). The reason why this is a vector is to support encodings that pack multiple instructions into a single instruction (although in most cases you should expect only a single instruction in the bundle).

### Definition
```c++
struct InstructionInfoBundle {
    std::vector<InstructionInfo> bundle;
};
```
