# isa_spec_manager
A set of tools for parsing and using AMD's machine-readable GPU ISA specifications.

The `IsaDecoder` API makes it easy to parse the specification XML files, decode instructions and even decode whole shaders.

For usage examples, see the [examples subfolder](https://github.amd.com/Developer-Solutions/isa_spec_manager/tree/amd-main/source/examples).

## Building isa_spec_manager
To build the project, use the build scripts located in the ./build subfolder. Please note that the build process requires CMake with minimum version of 3.0.

### Building on Linux
```
cd ./isa_spec_manager/build
./prebuild_linux.sh
cd linux
make
```

The above script will launch the cmake. The script will generate projects directory.

### Building on Windows
```
cd ./isa_spec_manager/build
./prebuild_windows.bat
```

The above script will create a windows directory and generate a solution for Visual Studio.

By default, a solution is generated for VS 2019. To generate a solution for a different VS version or to use a different MSVC toolchain use the `--vs` argument.
For example, to generate the solution for VS 2022 with the VS 2022 toolchain (MSVC 17), run:

``
./prebuild_windows.bat --vs 2022
``

## Using the API
The following example files can give you a quick overview of how to start using the XML ISA spec in your project:
* Basic usage example: [./source/examples/basic_decoder.cpp](./source/examples/basic_decoder.cpp). This sample requires a single command line argument which is a full path to the XML specification file.
* Usage example with multiple architectures in flight: [./source/examples/multi_arch_decoder.cpp](./source/examples/multi_arch_decoder.cpp). This sample requires one or more command line arguments which are the full paths to the XML specification files.

## Getting the ISA specification files
The Machine-Readable GPU ISA specification files can be downloaded from [AMD's Machine-Readable GPU ISA Specification page on GPUOpen.com](https://gpuopenstaging.wpengine.com/machine-readable-isa/).
