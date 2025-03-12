# AMD's Machine-Readable ISA Specification and Tools - Release Notes #

## Highlights of this release ##

AMD's machine-readable GPU Instruction Set Architecture specifications is a set of XML files that describe AMD's latest GPU ISA: instructions, encodings, operands, data formats and even human-readable description strings. 

The first release includes the specification XML files for the following GPU architectures:
* AMD CDNA™ 3 (Instinct™ MI300)
* AMD CDNA™ 2 (Instinct™ MI200)
* AMD CDNA™ 1 (Instinct™ MI100)
* AMD RDNA™ 3
* AMD RDNA™ 2
* AMD RDNA™ 1

The XML files can be downloaded from [GPUOpen.com](https://gpuopen.com/machine-readable-isa/).

This codebase includes the `IsaDecoder` API that can be used to decode, which can be used to decode AMD ISA assembly and disassembly using the specifications:
* Load XML specification files and automatically parse them, so you don't need to write your own parser.
* Decode single instructions and whole kernels and shaders in binary or text format.
* Handle multiple architectures in flight with the `DecodeManager` convenience API.

For usage examples and instructions on how to build the project, please see [source/examples subdirectory on the isa_spec_manager GitHub repository](https://github.com/GPUOpen-Tools/isa_spec_manager).

**Note:** while the `IsaDecoder` API is a good way to get started with parsing the XML files, nothing prevents you from parsing the files yourself and building your own custom workflow. To do that please refer to the XML schema documentation [XML schema documentation](https://github.com/GPUOpen-Tools/isa_spec_manager/main/spec_documentation.md).

## Known issues ##

### Specification ###
* Information about encoding modifiers is not provided in the specification.

### API and tools ###

* Decoding of MIMG instructions (such as IMAGE_STORE and IMAGE_LOAD) may produce the wrong register indices for source vector register operands.

