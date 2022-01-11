# Change Log

## [12.0.1](https://github.com/dyninst/dyninst/tree/v12.0.0) (2021-11-23)
[Full Changelog](https://github.com/dyninst/dyninst/compare/v12.0.0...v12.0.1)

**Build Changes**

- Remove NVIDIA external line map configure check ([1162](https://github.com/dyninst/dyninst/issues/1162))
- Increase minimum elfutils version to 0.186 ([1161](https://github.com/dyninst/dyninst/issues/1161))
- Add conflict with CMake 3.19.0 ([1153](https://github.com/dyninst/dyninst/issues/1153))

**Enhancements**

- Refactor dwarfWalker::findConst ([1160](https://github.com/dyninst/dyninst/issues/1160))
- Add readable name for Symtab::typeRef ([1157](https://github.com/dyninst/dyninst/issues/1157))
- DwarfWalker: clean up interfaces for findDieName and findName ([1154](https://github.com/dyninst/dyninst/issues/1154))
- Added automated docker build for development and testing

## [12.0.0](https://github.com/dyninst/dyninst/tree/v12.0.0) (2021-11-11)
[Full Changelog](https://github.com/dyninst/dyninst/compare/v11.0.1...v12.0.0)

**GPU Support**

- Add CMake test to check if libdw supports NVIDIA extended line map
- Adjust interface changes in elfutils regarding NVIDIA extended line map
- Add compile-time checking to see if elfutils support nvidia extended line map when the user have specified ENABLE_NVIDIA_EXT_LINE_MAP
- Fix compilation warning and add cmake option ENABLE_NVIDIA_EXT_LINE_MAP
- 1. Handle unrelocated line map entries for CUBIN 2. Remove redundant addFunctionRange call to improve performance 3. Add some debug logging
- Inline context from nvidia extended line map identifies an inlined call path
- Start to construct inlining call chains using Nvidia's extended line map
- cleaning up code for ingesting nvidia extended linemaps
- first draft of support for nvidia enhanced line maps

**Enhancements**

- Load callee's address when the callee and caller are in the same module ([1056](https://github.com/dyninst/dyninst/issues/1056))
- Give global annotation objects internal linkage and file scope
- Summit fixes ([1108](https://github.com/dyninst/dyninst/issues/1108))
- Add x86 xsavec instruction ([1074](https://github.com/dyninst/dyninst/issues/1074))
- Convert TRAMP_\*_OFFSET macros to functions ([1073](https://github.com/dyninst/dyninst/issues/1073))
- Add x86_64 xrstor instruction ([1070](https://github.com/dyninst/dyninst/issues/1070))
- Fix insertion operators in BPatch and Symtab ([1069](https://github.com/dyninst/dyninst/issues/1069))
- Add DWARF4 base type entry encodings to symtabAPI::typeScalar ([1059](https://github.com/dyninst/dyninst/issues/1059))
- Add xsave instruction ([1055](https://github.com/dyninst/dyninst/issues/1055))
- Cleanup orphaned code ([1064](https://github.com/dyninst/dyninst/issues/1064))

**ABI Breakages**
- Remove AddressSpace::causeTemplateInstantiations ([1149](https://github.com/dyninst/dyninst/issues/1149))
- Remove unregisterTrapMapping from PCProcess
- Remove thread registration functions from PCProcess
- Remove PCProcess::getDeadCode
- Remove memory emulation ([1146](https://github.com/dyninst/dyninst/issues/1146))
- Remove unused generateSimple ([1122](https://github.com/dyninst/dyninst/issues/1122))
- Remove unused variables from Symtab
- Remove special Fortran debug handling
- Remove stabs from symbol demangling
- Remove stabs from BPatch
- Remove stabs from SymtabAPI
- Remove Module::getAllVariables ([1066](https://github.com/dyninst/dyninst/issues/1066))

**Documentation**

- Improve docs for lookup functions in CodeObject ([1147](https://github.com/dyninst/dyninst/issues/1147))
- Update copyright to 2022 ([1141](https://github.com/dyninst/dyninst/issues/1141))
- Remove stabs from documentation ([1120](https://github.com/dyninst/dyninst/issues/1120))

**Build Changes**
- Remove ppc32 from builds ([1145](https://github.com/dyninst/dyninst/issues/1145))
- Unify meaning of 'cap_32_64' macro ([1136](https://github.com/dyninst/dyninst/issues/1136))
- Remove support for Cray CNL ([1137](https://github.com/dyninst/dyninst/issues/1137))
- Remove xlc macros ([1132](https://github.com/dyninst/dyninst/issues/1132))
- Remove common/src/language.h ([1131](https://github.com/dyninst/dyninst/issues/1131))
- Remove usage of arch_ppc and arch_ppc64 ([1129](https://github.com/dyninst/dyninst/issues/1129))
- Remove usage of x86_64_cnl ([1130](https://github.com/dyninst/dyninst/issues/1130))
- Remove DynC tests ([1126](https://github.com/dyninst/dyninst/issues/1126))
- Remove NO_INITIALIZER_LIST_SUPPORT ([1125](https://github.com/dyninst/dyninst/issues/1125))
- Turn on STERILE_BUILD by default ([1118](https://github.com/dyninst/dyninst/issues/1118))
- update minimum boost version to 1.70.0 ([1117](https://github.com/dyninst/dyninst/issues/1117))
- Remove boost_system linking ([1112](https://github.com/dyninst/dyninst/issues/1112))
- Enforce detection of libiberty ([1099](https://github.com/dyninst/dyninst/issues/1099))
- fix compiler warnings to work with clang ([1092](https://github.com/dyninst/dyninst/issues/1092))
- update optimization (-Og) and debug flags (-g3) ([1084](https://github.com/dyninst/dyninst/issues/1084))
- use the C11 standard for C code in Dyninst ([1086](https://github.com/dyninst/dyninst/issues/1086))
- Make Dyninst buildable with Clang ([1021](https://github.com/dyninst/dyninst/issues/1021))
- Remove valueAdded subdirectory completely ([1065](https://github.com/dyninst/dyninst/issues/1065))
- Remove valueAdded subdirectory ([1063](https://github.com/dyninst/dyninst/issues/1063))

**Bug Fixes**
- fix statement-like macros ([1143](https://github.com/dyninst/dyninst/issues/1143))
- Don't overflow aarch64 float register vector when setting used regs. ([1127](https://github.com/dyninst/dyninst/issues/1127))
- fix unused const variable warnings
- fix pessimizing std::move warnings
- fix xor operator used as power operator
- fix misleading indentation warning
- fix uninitialized this and variable warnings
- fix float to double promotion warning
- fix unused const variable warnings
- Fix possible buffer overflow in BPatch::processCreate
- Fix uninitialized variable use in DispatcherARM64::iproc_init
- remove executable flag from .dyninst_heap section ([1096](https://github.com/dyninst/dyninst/issues/1096))
- fix broken cast of a char literal to pointer ([1090](https://github.com/dyninst/dyninst/issues/1090))
- fix possibly uninitialized variables ([1082](https://github.com/dyninst/dyninst/issues/1082))
- fix possible null 'this' pointer dereference ([1082](https://github.com/dyninst/dyninst/issues/1082))
- prevent maybe uninitialized warning ([1082](https://github.com/dyninst/dyninst/issues/1082))
- adjust large frame threshold for specific sources ([1082](https://github.com/dyninst/dyninst/issues/1082))
- fix deprecated implicit assignment operator ([1082](https://github.com/dyninst/dyninst/issues/1082))
- fix buffer overflow ([1082](https://github.com/dyninst/dyninst/issues/1082))
- fix duplicate branch condition by removing branch ([1082](https://github.com/dyninst/dyninst/issues/1082))
- fix out of bounds array access ([1082](https://github.com/dyninst/dyninst/issues/1082))
- fix potentially uninitialized variable warning ([1082](https://github.com/dyninst/dyninst/issues/1082))
- use unused variable to correct code ([1082](https://github.com/dyninst/dyninst/issues/1082))
- remove unused variables ([1082](https://github.com/dyninst/dyninst/issues/1082))
- make printf format and argument types match ([1082](https://github.com/dyninst/dyninst/issues/1082))
- fix broken bool expression that was always true ([1082](https://github.com/dyninst/dyninst/issues/1082))
- add missing initializer braces ([1082](https://github.com/dyninst/dyninst/issues/1082))
- make constructor public so class is usable ([1082](https://github.com/dyninst/dyninst/issues/1082))
- remove ';' after in-class method definitions ([1082](https://github.com/dyninst/dyninst/issues/1082))
- eliminate logical op warning ([1082](https://github.com/dyninst/dyninst/issues/1082))
- make implicit double promotions explicit ([1082](https://github.com/dyninst/dyninst/issues/1082))
- annotate malloc-like functions ([1082](https://github.com/dyninst/dyninst/issues/1082))
- make method noexcept, so noexcept expr can be true ([1082](https://github.com/dyninst/dyninst/issues/1082))
- add missing default to switch statement ([1082](https://github.com/dyninst/dyninst/issues/1082))
- fix int to void\* cast if sizeof(int)<sizeof(void\*) ([1082](https://github.com/dyninst/dyninst/issues/1082))
- eliminate conversion of NULL to non-pointer type ([1082](https://github.com/dyninst/dyninst/issues/1082))
- fix variable signedness ([1082](https://github.com/dyninst/dyninst/issues/1082))
- replace if stmt with identical branches with then stmt ([1082](https://github.com/dyninst/dyninst/issues/1082))
- fgetc returns an int not a char ([1082](https://github.com/dyninst/dyninst/issues/1082))
- do not discard volatile type qualifier in cast ([1082](https://github.com/dyninst/dyninst/issues/1082))
- add missing #include <assert.h> ([1082](https://github.com/dyninst/dyninst/issues/1082))
- fix unused vars/params/funcs on aarch64 ([1082](https://github.com/dyninst/dyninst/issues/1082))
- fix ambiguous type name warning ([1082](https://github.com/dyninst/dyninst/issues/1082))
- remove always true || sub-expression ([1082](https://github.com/dyninst/dyninst/issues/1082))
- fix possible sprintf buffer overflow ([1082](https://github.com/dyninst/dyninst/issues/1082))
- delete unnecessary ambiguous forward class decl ([1082](https://github.com/dyninst/dyninst/issues/1082))
- make destructor virtual if a virtual method exist ([1082](https://github.com/dyninst/dyninst/issues/1082))
- make printf format and argument signedness match ([1082](https://github.com/dyninst/dyninst/issues/1082))
- make printf format and argument types match ([1082](https://github.com/dyninst/dyninst/issues/1082))
- add compiler annotation to printf-like functions ([1082](https://github.com/dyninst/dyninst/issues/1082))
- fix var-tracking-assignments warnings ([1082](https://github.com/dyninst/dyninst/issues/1082))
- remove assert(this) as 'this' should never be null ([1082](https://github.com/dyninst/dyninst/issues/1082))
- remove obvious null pointer dereference ([1082](https://github.com/dyninst/dyninst/issues/1082))
- fix for C++20 removal of std::allocator methods ([1082](https://github.com/dyninst/dyninst/issues/1082))
- make cmp function object operator() a const func ([1082](https://github.com/dyninst/dyninst/issues/1082))
- make Boost and TBB include dirs be system includes ([1082](https://github.com/dyninst/dyninst/issues/1082))
- fix shadow variable warning, has other brokenness ([1082](https://github.com/dyninst/dyninst/issues/1082))
- fix duplicate branch warnings ([1082](https://github.com/dyninst/dyninst/issues/1082))
- eliminate switch case fall through warnings ([1082](https://github.com/dyninst/dyninst/issues/1082))
- explicit base class initialization in constructor ([1082](https://github.com/dyninst/dyninst/issues/1082))
- remove default argument from lambda ([1082](https://github.com/dyninst/dyninst/issues/1082))
- remove non-C++ compound literal ([1082](https://github.com/dyninst/dyninst/issues/1082))
- do not compile empty compilation units ([1082](https://github.com/dyninst/dyninst/issues/1082))
- fix deprecated implicit copy constructor if dtor ([1082](https://github.com/dyninst/dyninst/issues/1082))
- add missing copy assignment ([1082](https://github.com/dyninst/dyninst/issues/1082))
- fix illegal in C empty brace initialization ([1082](https://github.com/dyninst/dyninst/issues/1082))
- disable flexible array member warning in C++ ([1082](https://github.com/dyninst/dyninst/issues/1082))
- fix discard qualifiers: make char\* -> const char\* ([1082](https://github.com/dyninst/dyninst/issues/1082))
- fix non-standard use of \_\_VA_ARGS\_\_ ([1082](https://github.com/dyninst/dyninst/issues/1082))
- remove excess semicolons as reported by -pedantic ([1082](https://github.com/dyninst/dyninst/issues/1082))
- fix overflow warning for 0x90 assigned to a char ([1082](https://github.com/dyninst/dyninst/issues/1082))
- fix illegal function pointer to void\* compare ([1082](https://github.com/dyninst/dyninst/issues/1082))
- remove use of GNU binary operator ?: ([1082](https://github.com/dyninst/dyninst/issues/1082))
- remove non-C++ variable length arrays ([1082](https://github.com/dyninst/dyninst/issues/1082))
- make printf format and argument types match ([1082](https://github.com/dyninst/dyninst/issues/1082))
- fix shadow identifier warnings ([1082](https://github.com/dyninst/dyninst/issues/1082))
- enable more warnings and test compiler support ([1082](https://github.com/dyninst/dyninst/issues/1082))
- miscellaneous compiler warning cleanups ([1082](https://github.com/dyninst/dyninst/issues/1082))
- eliminate switch case fall through warnings ([1082](https://github.com/dyninst/dyninst/issues/1082))
- add header with compiler annotation macros ([1082](https://github.com/dyninst/dyninst/issues/1082))
- add missing break statements ([1082](https://github.com/dyninst/dyninst/issues/1082))
- compute num array elements instead of fixed values ([1082](https://github.com/dyninst/dyninst/issues/1082))
- remove dynamic_ and dynamic() from fileDescriptor ([1082](https://github.com/dyninst/dyninst/issues/1082))
- remove emptyString static members ([1082](https://github.com/dyninst/dyninst/issues/1082))
- delete unnecessary .DS_Store file ([1082](https://github.com/dyninst/dyninst/issues/1082))


## [11.0.1](https://github.com/dyninst/dyninst/tree/v11.0.1) (2021-06-14)
[Full Changelog](https://github.com/dyninst/dyninst/compare/v11.0.0...v11.0.1)

**Enhancements:**
- fix Position Independent Executable (PIE) handling ([1042](https://github.com/dyninst/dyninst/issues/1042))

**Bug Fixes:**
- Find function by address, not by entry ([1047](https://github.com/dyninst/dyninst/issues/1047))
- Fix implicit operand decoding of x86 instructions on non-x86 architectures ([1044](https://github.com/dyninst/dyninst/issues/1044))
- Fixing calling `dwarf_getabbrevcode` and refactoring debug logging in common ([1037](https://github.com/dyninst/dyninst/issues/1037))
- Search for 'version.h' when determining version for TBB ([1041](https://github.com/dyninst/dyninst/issues/1041))
- Fix data races and asserts discovered on Power  ([1038](https://github.com/dyninst/dyninst/issues/1038))
- Remove the use of C++17's structured binding ([1036](https://github.com/dyninst/dyninst/issues/1036))
- Skip parsing of blocks whose code buffer is null ([1033](https://github.com/dyninst/dyninst/issues/1033))
- Remove debug printing that causes large output ([1029](https://github.com/dyninst/dyninst/issues/1029))
- Fix catch block parsing ([1030](https://github.com/dyninst/dyninst/issues/1030))
- Do not treat symbols in any text sections as data (.text, .init, or .fini) ([1026](https://github.com/dyninst/dyninst/issues/1026))
- Allow usage of SIGILL for signal trampolines ([963](https://github.com/dyninst/dyninst/issues/963))
- Set -B and -S when configuring dyninstAPI_RT ([1020](https://github.com/dyninst/dyninst/issues/1020))

## [11.0.0](https://github.com/dyninst/dyninst/tree/v11.0.0) (2021-04-08)
[Full Changelog](https://github.com/dyninst/dyninst/compare/v10.2.1...v11.0.0)

**Minimum Version Changes:**

- Bump minimum Boost version to 1.67 ([993](https://github.com/dyninst/dyninst/issues/993))
- Require gcc >= 6.0 ([932](https://github.com/dyninst/dyninst/issues/932))
- Add note about minimum elfutils version ([912](https://github.com/dyninst/dyninst/issues/912))
- Require c++11 thread_local support ([860](https://github.com/dyninst/dyninst/issues/860))
- Require standards-compliant c++11 ABI ([882](https://github.com/dyninst/dyninst/issues/882))

**GPU Support:**

- Add support for instructions with largest opcode in each instruction family ([1014](https://github.com/dyninst/dyninst/issues/1014))
- Add Initial support for Analyzing Indirect Control Flow on Amdgpu Vega ([979](https://github.com/dyninst/dyninst/issues/979))
- Fix unhandled enum values for Intel GPUs ([955](https://github.com/dyninst/dyninst/issues/955))
- Fixing another bug related to amdgpu register names ([948](https://github.com/dyninst/dyninst/issues/948))
- Fix a bug in MachRegister::name for AMD GPUs ([928](https://github.com/dyninst/dyninst/issues/928))
- Add initial support for analyzing AMDGPU binaries ([900](https://github.com/dyninst/dyninst/issues/900))
- Nvidia GPU slicing and support for opening Intel GPU binaries ([865](https://github.com/dyninst/dyninst/issues/865))
- Add arch address for intel gen9 gpus ([874](https://github.com/dyninst/dyninst/issues/874))
- Some necessary changes for overlapping text regions ([855](https://github.com/dyninst/dyninst/issues/855))

**Enhancements:**

- Update copyrights to 2021 ([1015](https://github.com/dyninst/dyninst/issues/1015))
- Add a ParseAPI::Block constructor to allow external parsers to set correctly block end address ([1011](https://github.com/dyninst/dyninst/issues/1011))
- Clean up PLTFunction in dyninstAPI/Parse ([1003](https://github.com/dyninst/dyninst/issues/1003))
- add erase to 2 std::remove calls
- Aggregate variable symbols based on offset and size ([933](https://github.com/dyninst/dyninst/issues/933))
- make Symbol methods const ([936](https://github.com/dyninst/dyninst/issues/936))
- Add direct tail call case for Point::getCallee ([931](https://github.com/dyninst/dyninst/issues/931))
- Make assert expressions side effect free ([927](https://github.com/dyninst/dyninst/issues/927))
- Improve single thread's backward slicing speed by 40% ([917](https://github.com/dyninst/dyninst/issues/917))
- make symbol name demangling consistent (#872, #717)
- Ensure data is copied in memoryTracker::realloc ([886](https://github.com/dyninst/dyninst/issues/886))
- Include <iostream> in CodeTracker.C ([887](https://github.com/dyninst/dyninst/issues/887))
- clean up BoundFactCalculator constructor ([884](https://github.com/dyninst/dyninst/issues/884))
- CodeSource destructor cleanup ([883](https://github.com/dyninst/dyninst/issues/883))
- Fix memory leaks in BinaryEdit::openResolvedLibraryName ([879](https://github.com/dyninst/dyninst/issues/879))
- Add lookup by name in block_instance::callee ([875](https://github.com/dyninst/dyninst/issues/875))
- cleanup memoryTracker memory handling ([876](https://github.com/dyninst/dyninst/issues/876))
- Provide correct default constructor for parse_func class ([878](https://github.com/dyninst/dyninst/issues/878))
- Correctly destruct AddressSpace objects ([871](https://github.com/dyninst/dyninst/issues/871))
- Update documentation for ParseAPI::CodeObject::CodeObject ([870](https://github.com/dyninst/dyninst/issues/870))
- Remove include cycle in dyntypes.h ([868](https://github.com/dyninst/dyninst/issues/868))
- Cleanup dyn_hash_{set,map} ([861](https://github.com/dyninst/dyninst/issues/861))
- parseThat: remove autotools build files ([858](https://github.com/dyninst/dyninst/issues/858))
- Replace BPatch_vector internal implementation with std::vector ([844](https://github.com/dyninst/dyninst/issues/844))
- InstructionAPI docs: Update InsnCategory values returned from Instruction::getCategory ([851](https://github.com/dyninst/dyninst/issues/851))

**Bug Fixes:**

- Remove assert in block_instance::callee(std::string const&) ([999](https://github.com/dyninst/dyninst/issues/999))
- Fix breakage introduced by PR990 ([997](https://github.com/dyninst/dyninst/issues/997))
- Fix non-deterministic inline function lookup when bad DWARF is generated for OpenMP outlined code ([1012](https://github.com/dyninst/dyninst/issues/1012))
- Fix wrong return value in DwarfHandle::init_dbg ([939](https://github.com/dyninst/dyninst/issues/939))
- Fix memory leaks in emitElf ([895](https://github.com/dyninst/dyninst/issues/895))
- SymElf:  fix memory leak of cached demangled names
- properly check for empty string in parseStabString
- fix duplicate Windows demangle code
- Fix PLT function call lookup ([1001](https://github.com/dyninst/dyninst/issues/1001))
- Fix undefined behavior in usage of std::transform ([862](https://github.com/dyninst/dyninst/issues/862))

**DWARF Changes:**

- In DwarfWalker, start a new context dissociated from the current context ([1013](https://github.com/dyninst/dyninst/issues/1013))
- DWARF supplemental file and type parsing ([1002](https://github.com/dyninst/dyninst/issues/1002))
- Add debuginfod support ([736](https://github.com/dyninst/dyninst/issues/736))
- Suppress parallelism in dwarf parsing when an alternative debug file is present ([929](https://github.com/dyninst/dyninst/issues/929))

**Compiler Warning Cleanup:**

- Clean up "unused parameter" warnings on Aarch64 ([1005](https://github.com/dyninst/dyninst/issues/1005))
- Fix "unused parameter" warnings on PPC64 ([1004](https://github.com/dyninst/dyninst/issues/1004))
- Remove dead code from ia32_decode ([989](https://github.com/dyninst/dyninst/issues/989))
- Clean up sign-compare warnings ([991](https://github.com/dyninst/dyninst/issues/991))
- Add the default case in adhocMovementTransformer::isPCRelData to suppress compiler warning ([995](https://github.com/dyninst/dyninst/issues/995))
- Clean up "unused variable" warnings ([990](https://github.com/dyninst/dyninst/issues/990))
- Correctly declare Aggregate::operator<< ([988](https://github.com/dyninst/dyninst/issues/988))
- Fix unhandled switch case in Region::regionType2Str ([987](https://github.com/dyninst/dyninst/issues/987))
- Fix unused values ([978](https://github.com/dyninst/dyninst/issues/978))
- Remove unused parameter 'b' from BoundFactsCalculator::Meet ([983](https://github.com/dyninst/dyninst/issues/983))
- Fix string truncations in parseThat ([982](https://github.com/dyninst/dyninst/issues/982))
- Use std::locale when writing a timeStamp to a stream ([981](https://github.com/dyninst/dyninst/issues/981))
- Remove ignored cast qualifier in SnippetGenerator::findParameter ([980](https://github.com/dyninst/dyninst/issues/980))
- Fix shifts of negative values ([976](https://github.com/dyninst/dyninst/issues/976))
- Remove unused function 'InsertFrames' in parseAPI/Parser ([977](https://github.com/dyninst/dyninst/issues/977))
- Fix deprecated usage of boost::bind ([975](https://github.com/dyninst/dyninst/issues/975))
- Fix inclusion of boost deprecated headers ([974](https://github.com/dyninst/dyninst/issues/974))
- Fix constructor member intializer list reordering ([973](https://github.com/dyninst/dyninst/issues/973))
- Fix pointer arithmetic on 'void*' in codeGen::insert ([972](https://github.com/dyninst/dyninst/issues/972))
- Remove usage of designated initializer in dwarfHandle.C ([971](https://github.com/dyninst/dyninst/issues/971))
- Fix possibly uninitialized local variable in InstructionDecoder_aarch64 ([970](https://github.com/dyninst/dyninst/issues/970))
- Fix ignored qualifiers on some C-style casts in Object-elf::read_val_of_type ([969](https://github.com/dyninst/dyninst/issues/969))
- Fix C-string format specifier mismatch in Operand::getReadSet ([968](https://github.com/dyninst/dyninst/issues/968))
- Fix several C-string truncations in parseThat ([967](https://github.com/dyninst/dyninst/issues/967))
- Fix possible buffer overflow in parseThat::runHunt_binaryEdit ([966](https://github.com/dyninst/dyninst/issues/966))
- Clean up "unused parameter" warnings ([965](https://github.com/dyninst/dyninst/issues/965))
- Fix misleading indentation compiler warning in codeRangeTree::remove ([964](https://github.com/dyninst/dyninst/issues/964))
- Remove empty region_data constructor ([960](https://github.com/dyninst/dyninst/issues/960))
- Fix -Wreturn-type warning ([956](https://github.com/dyninst/dyninst/issues/956))
- Remove unused parameter 'elf' from Object::parse_all_relocations ([962](https://github.com/dyninst/dyninst/issues/962))
- Remove unused 'name' parameter from Collections::addGlobalVariable ([961](https://github.com/dyninst/dyninst/issues/961))
- Fix compile warnings for AMDGPU for Release 11.0 ([954](https://github.com/dyninst/dyninst/issues/954))

**Build Changes:**

- add missing libiberty include dir ([950](https://github.com/dyninst/dyninst/issues/950))
- Make libiberty detection more flexible ([922](https://github.com/dyninst/dyninst/issues/922))
- Correctly set up libiberty to be consumed by build system ([901](https://github.com/dyninst/dyninst/issues/901))

**Remove Deprecated Functionality:**

- Remove JumpTableIndexPred::FillInOutEdges ([959](https://github.com/dyninst/dyninst/issues/959))
- Remove BoundFactCalculator::CheckZeroExtend ([958](https://github.com/dyninst/dyninst/issues/958))
- Remove BoundsFactCalculator::ThunkBound ([957](https://github.com/dyninst/dyninst/issues/957))
- API-breaking changes for 11.0 release ([920](https://github.com/dyninst/dyninst/issues/920))
- Remove old InstrucIter class references ([890](https://github.com/dyninst/dyninst/issues/890))
- code cleanup integer funcs to * and / by constants
- Remove unused lineDict class ([880](https://github.com/dyninst/dyninst/issues/880))
- Remove bluegene support ([847](https://github.com/dyninst/dyninst/issues/847))
- Remove vxworks support ([859](https://github.com/dyninst/dyninst/issues/859))
- Replace pdvector with std::vector ([856](https://github.com/dyninst/dyninst/issues/856))
- Remove vectorSet ([857](https://github.com/dyninst/dyninst/issues/857))
- Remove vestiges of sparc ([850](https://github.com/dyninst/dyninst/issues/850))
- Remove vestiges of AIX ([849](https://github.com/dyninst/dyninst/issues/849))
- Remove binaryEdit::deleteBinaryEdit ([866](https://github.com/dyninst/dyninst/issues/866))


## [10.2.1](https://github.com/dyninst/dyninst/tree/v10.2.1) (2020-09-09)
[Full Changelog](https://github.com/dyninst/dyninst/compare/v10.2.0...v10.2.1)

**Implemented enhancements:**

- Remove exception specifications ([841](https://github.com/dyninst/dyninst/issues/841))
- Remove unused Doxygen files from instructionAPI ([829](https://github.com/dyninst/dyninst/issues/829))
- Remove deprecated members of BPatch_flowGraph ([828](https://github.com/dyninst/dyninst/issues/828))
- Undeprecate mapped_object::getBaseAddress ([827](https://github.com/dyninst/dyninst/issues/827))
- Remove unused symtabAPI/doc/symtab-text.txt ([826](https://github.com/dyninst/dyninst/issues/826))
- Make ~DynObject virtual ([813](https://github.com/dyninst/dyninst/issues/813))
- Remove cotire ([816](https://github.com/dyninst/dyninst/issues/816))
- Fix cmake configuration with -pie ([817](https://github.com/dyninst/dyninst/issues/817))
- Remove usage of miniTramp class ([801](https://github.com/dyninst/dyninst/issues/801))
- Add include for Elf_X in emitElf.h ([790](https://github.com/dyninst/dyninst/issues/790))
- Clean up some includes ([796](https://github.com/dyninst/dyninst/issues/796))

**Fixed bugs:**

- Fix memory leak in singleton_object_pool ([835](https://github.com/dyninst/dyninst/issues/835))
- Fix power instruction decoding regression
- Fix aarch64 instruction decoding regression
- Fix memory leak in singleton_object_pool
- Fix memory leak in SymEval::expandInsn ([793](https://github.com/dyninst/dyninst/issues/793))
- Fix aliasing bug of Region::buffer on copy ([791](https://github.com/dyninst/dyninst/issues/791))
- Cleanup memory handling when emitting Elf for static libraries ([789](https://github.com/dyninst/dyninst/issues/789))

## [10.2.0](https://github.com/dyninst/dyninst/tree/v10.2.0) (2020-07-30)
[Full Changelog](https://github.com/dyninst/dyninst/compare/v10.1.0...v10.2.0)

**Implemented enhancements:**

- Fix Variable location parsing ([781](https://github.com/dyninst/dyninst/issues/781))
- Fix proccontrol hangs in test_thread_X ([780](https://github.com/dyninst/dyninst/issues/780))
- proccontrol: Handle "ghost" threads ([742](https://github.com/dyninst/dyninst/issues/742))
- Elf extended numbering ([776](https://github.com/dyninst/dyninst/issues/776))
- Expand the list of non-returning functions in CodeSource.C. ([748](https://github.com/dyninst/dyninst/issues/748))
- Memory leak fixes ([758](https://github.com/dyninst/dyninst/issues/758))
- C++ cleanup ([610](https://github.com/dyninst/dyninst/issues/610))
- Libxul fixes ([755](https://github.com/dyninst/dyninst/issues/755))
- Potential fixes needed for relocating libraries ([754](https://github.com/dyninst/dyninst/issues/754))
- Add compiler-specific search directories in getResolvedLibraryPath ([732](https://github.com/dyninst/dyninst/issues/732))
- More parallel parsing optimization and fixes ([729](https://github.com/dyninst/dyninst/issues/729))
- Add search of libgcc.a to x86 and aarch64 ([731](https://github.com/dyninst/dyninst/issues/731))
- PIE/PIC aarch64 rewrite implementation ([698](https://github.com/dyninst/dyninst/issues/698))
- Fixes for AbsRegion and jump table index slicing involving global variable ([695](https://github.com/dyninst/dyninst/issues/695))
- Parallel DWARF parsing and improved parallel code parsing ([651](https://github.com/dyninst/dyninst/issues/651))
- Allow same address range to map to different source lines in DWARF ([643](https://github.com/dyninst/dyninst/issues/643))
- Parse dwarf variables with abstract origin attribute ([642](https://github.com/dyninst/dyninst/issues/642))
- Fix instrumentation regressions for libc-2.29 on ARM ([653](https://github.com/dyninst/dyninst/issues/653))
- Implement the check of ThreadDB at BPatch level ([667](https://github.com/dyninst/dyninst/issues/667))
- Use static AArch64 decoder tables. ([633](https://github.com/dyninst/dyninst/issues/633))
- Don't use software breakpoints when creating traps for springboards ([637](https://github.com/dyninst/dyninst/issues/637))
- ARMv8 initial work on rewriter ([612](https://github.com/dyninst/dyninst/issues/612))

**Fixed bugs:**

- Revert CUDA binaries openning to libdw ([787](https://github.com/dyninst/dyninst/issues/787))
- Updates for #780 ([783](https://github.com/dyninst/dyninst/issues/783))
- Improve detection of ghost threads ([784](https://github.com/dyninst/dyninst/issues/784))
- Remove Aggregate::setModule declaration ([779](https://github.com/dyninst/dyninst/issues/779))
- Remove examples ([764](https://github.com/dyninst/dyninst/issues/764))
- Improve logging in PCEventHandler::handleThreadCreate ([772](https://github.com/dyninst/dyninst/issues/772))
- Retain hint function from CodeSource even if we do not parse the code object. ([768](https://github.com/dyninst/dyninst/issues/768))
- remove unnecessary assertion for unknown phdr_type ([757](https://github.com/dyninst/dyninst/issues/757))
- Use register x30 (Link Register) to generate long branch ([720](https://github.com/dyninst/dyninst/issues/720))
- Do not use non-trivial types in varargs ([704](https://github.com/dyninst/dyninst/issues/704))
- Remove undefined behavior from Symtab::Type ([706](https://github.com/dyninst/dyninst/issues/706))
- Update Examples in dataflowAPI ([700](https://github.com/dyninst/dyninst/issues/700))
- Prevent corruption to rax during stack alignment on x86-64 ([670](https://github.com/dyninst/dyninst/issues/670))
- Ignore additional Eclipse file and spurious .gitignore ([681](https://github.com/dyninst/dyninst/issues/681))
- Add explicit ElfUtils dependency for ParseThat and examples ([678](https://github.com/dyninst/dyninst/issues/678))
- Add $INSTALL/lib/elfutils subdirectory to build paths ([680](https://github.com/dyninst/dyninst/issues/680))
- Allow sterile builds ([641](https://github.com/dyninst/dyninst/issues/641))
- Reorder includes to fix hidden build dependencies ([665](https://github.com/dyninst/dyninst/issues/665))
- Deprecate Blue Gene/Q support ([662](https://github.com/dyninst/dyninst/issues/662))
- Delete duplicate friend declaration of Parser in Block ([649](https://github.com/dyninst/dyninst/issues/649))
- Rename getType() -> getDataClass() in Section 8.1 of SymtabAPI docs ([661](https://github.com/dyninst/dyninst/issues/661))
- Fix spelling of getTypedefType in Section 8.1 of SymtabAPI docs ([660](https://github.com/dyninst/dyninst/issues/660))
- Update handling of TBB CMake file for clang ([654](https://github.com/dyninst/dyninst/issues/654))
- Fix typo in declaration of 'create' in Section 8.3 ([659](https://github.com/dyninst/dyninst/issues/659))
- Change dataTypeDefine -> DataTypedef in DataClass enum documentation ([650](https://github.com/dyninst/dyninst/issues/650))
- Use CMAKE_xx_COMPILER when building external dependencies ([636](https://github.com/dyninst/dyninst/issues/636))
- Allow CMake files for dependencies to be included multiple times ([639](https://github.com/dyninst/dyninst/issues/639))
- Fix overlapping bug of program header segments on fixPhdr. ([618](https://github.com/dyninst/dyninst/issues/618))
- Updates TLS descriptors values in .dynamic section (ARMv8, #614)
- Use private writable mmap when opening binaries and debug files ([624](https://github.com/dyninst/dyninst/issues/624))
- Convert snprintf with PRIx64 to std::to_string ([627](https://github.com/dyninst/dyninst/issues/627))
- Use PRIx64 macros for long long results for i686. ([517](https://github.com/dyninst/dyninst/issues/517))
- Delete .syntastic_cpp_config
- Add note on TBB usage when built from source

## [10.1.0](https://github.com/dyninst/dyninst/tree/v10.1.0) (2019-05-15)
[Full Changelog](https://github.com/dyninst/dyninst/compare/v10.0.0...10.1.0)

**Implemented enhancements:**

- Unset library variables when building elfutils from source [\#561](https://github.com/dyninst/dyninst/issues/561)
- Update LibIberty to new build system [\#601](https://github.com/dyninst/dyninst/pull/601) ([hainest](https://github.com/hainest))
- ARM effective address calculation [\#594](https://github.com/dyninst/dyninst/pull/594) ([mxz297](https://github.com/mxz297))
- Elfutils cleanup [\#591](https://github.com/dyninst/dyninst/pull/591) ([hainest](https://github.com/hainest))
- TBB Cleanup [\#584](https://github.com/dyninst/dyninst/pull/584) ([hainest](https://github.com/hainest))
- Boost cleanup [\#578](https://github.com/dyninst/dyninst/pull/578) ([hainest](https://github.com/hainest))
- Add initial support for decoding AMD FMA4 and XOP instructions [\#575](https://github.com/dyninst/dyninst/pull/575) ([mxz297](https://github.com/mxz297))
- Convert ThreadDB error message to info message [\#574](https://github.com/dyninst/dyninst/pull/574) ([hainest](https://github.com/hainest))
- Fix compilation of thread\_db\_process [\#571](https://github.com/dyninst/dyninst/pull/571) ([sashanicolas](https://github.com/sashanicolas))
- Fixing TBB dependencies during build. [\#570](https://github.com/dyninst/dyninst/pull/570) ([sashanicolas](https://github.com/sashanicolas))
- Support parsing cubin in dyninst-10.0 [\#556](https://github.com/dyninst/dyninst/pull/556) ([Jokeren](https://github.com/Jokeren))

**Fixed bugs:**

- InstructionAPI fails to decode some vector instructions on x86-64 [\#573](https://github.com/dyninst/dyninst/issues/573)
- Make boost available after 'make install' [\#563](https://github.com/dyninst/dyninst/issues/563)
- Assertion failure in LivenessAnalyzer::getLivenessIn\(Dyninst::ParseAPI::Block\*\) [\#560](https://github.com/dyninst/dyninst/issues/560)
- Homogenize cmake includes [\#557](https://github.com/dyninst/dyninst/issues/557)
- Legacy test failures [\#550](https://github.com/dyninst/dyninst/issues/550)
- Implicit ParseFunctionRanges is not thread-safe [\#360](https://github.com/dyninst/dyninst/issues/360)
- GCC7 issues: new warnings, building with cotire causes GCC crash [\#321](https://github.com/dyninst/dyninst/issues/321)
- \[ARMv8 Decoding\] SHA instruction should have 0s for bits 20 and 22 [\#264](https://github.com/dyninst/dyninst/issues/264)
- \[ARM Decoding\] Some compare instructions should be invalid [\#251](https://github.com/dyninst/dyninst/issues/251)
- \[ARM Decoding\] Reserved value for register shift field should create invalid insn [\#250](https://github.com/dyninst/dyninst/issues/250)
- \[ARM Syntax\] Should print zero immediate for compares [\#237](https://github.com/dyninst/dyninst/issues/237)
- \[ARM Decoding\] Compare instructions with zero ignore reserved bits [\#236](https://github.com/dyninst/dyninst/issues/236)
- \[ARM Decoding\] SQSHL instruction has invalid bits set [\#235](https://github.com/dyninst/dyninst/issues/235)
- \[ARM Decoding\] Invalid Subtract instruction [\#234](https://github.com/dyninst/dyninst/issues/234)
- Update LibIberty to new build system [\#601](https://github.com/dyninst/dyninst/pull/601) ([hainest](https://github.com/hainest))
- Update README to reflect new build system [\#597](https://github.com/dyninst/dyninst/pull/597) ([hainest](https://github.com/hainest))
- TBB spack fixes [\#595](https://github.com/dyninst/dyninst/pull/595) ([hainest](https://github.com/hainest))
- Fix BPatch\_effectiveAddress on ppc where only low 32-bit address is extracted  [\#593](https://github.com/dyninst/dyninst/pull/593) ([mxz297](https://github.com/mxz297))
- More fixes for parallel parsing, spring boards, and ARM [\#592](https://github.com/dyninst/dyninst/pull/592) ([mxz297](https://github.com/mxz297))
- Elfutils cleanup [\#591](https://github.com/dyninst/dyninst/pull/591) ([hainest](https://github.com/hainest))
- TBB Cleanup [\#584](https://github.com/dyninst/dyninst/pull/584) ([hainest](https://github.com/hainest))
- Boost cleanup [\#578](https://github.com/dyninst/dyninst/pull/578) ([hainest](https://github.com/hainest))
- A few fixes for parallel parsing [\#572](https://github.com/dyninst/dyninst/pull/572) ([mxz297](https://github.com/mxz297))
- Fix bad interactions between patchAPI and parseAPI [\#564](https://github.com/dyninst/dyninst/pull/564) ([mxz297](https://github.com/mxz297))
- Elfutil version check [\#558](https://github.com/dyninst/dyninst/pull/558) ([hainest](https://github.com/hainest))
- Add --enable-install-elfh when building elfutils from source [\#555](https://github.com/dyninst/dyninst/pull/555) ([hainest](https://github.com/hainest))
- Attempts to fix legacy test failures in Dyninst test suite [\#549](https://github.com/dyninst/dyninst/pull/549) ([mxz297](https://github.com/mxz297))

**Closed issues:**

- Altering input operand of an instruction [\#590](https://github.com/dyninst/dyninst/issues/590)
- BPatch\_addressSpace replaceFunctionCall does not set R12 on Power [\#589](https://github.com/dyninst/dyninst/issues/589)
- Trying to get a very simple PatchAPI example working [\#587](https://github.com/dyninst/dyninst/issues/587)
- Cobwebs on the Documentation and boost 1.70.0 issues. [\#585](https://github.com/dyninst/dyninst/issues/585)
- libboost\_system.so.1.58.0: error adding symbols: DSO missing from command line [\#579](https://github.com/dyninst/dyninst/issues/579)
- Symtab should not always demangle every function name [\#577](https://github.com/dyninst/dyninst/issues/577)
- Springboards can trample function data due to incorrect range [\#551](https://github.com/dyninst/dyninst/issues/551)
- lib/libdw.so.1 not copied on install [\#547](https://github.com/dyninst/dyninst/issues/547)
- linking old system libelf when found [\#546](https://github.com/dyninst/dyninst/issues/546)
- Spack Build Failed with errors about variable not declared in the scope [\#544](https://github.com/dyninst/dyninst/issues/544)
- separate debuginfo failures [\#542](https://github.com/dyninst/dyninst/issues/542)
- Any support for ARM32 and MIPS [\#538](https://github.com/dyninst/dyninst/issues/538)
- After instrument with dyninst binary exports functions from libc [\#529](https://github.com/dyninst/dyninst/issues/529)
- Dyninst errors when building with boost-1.69.0 [\#526](https://github.com/dyninst/dyninst/issues/526)
- Bpatch\_effectiveAddress truncates memory addresses to bottom 32 bits \(PPC Only\) [\#524](https://github.com/dyninst/dyninst/issues/524)
- segfault [\#523](https://github.com/dyninst/dyninst/issues/523)
- Abort in Dyninst 10.0.0 when trying to open file: libcublas.so.9.2.88 [\#508](https://github.com/dyninst/dyninst/issues/508)
- Dyninst-10.0.0 boost::shared\_ptr\<Dyninst::InstructionAPI::Instruction\> instead of Dyninst::InstructionAPI::Instruction [\#505](https://github.com/dyninst/dyninst/issues/505)
- Dyninst-10.0.0 undeclared EM\_AARCH64 [\#503](https://github.com/dyninst/dyninst/issues/503)
- dyninst 10 hangs on ppcle at bpatch.processAttach [\#502](https://github.com/dyninst/dyninst/issues/502)
- function return value destroyed when instrumented function access the input parameters at return statement only [\#501](https://github.com/dyninst/dyninst/issues/501)
- 'repfunc' error [\#499](https://github.com/dyninst/dyninst/issues/499)
- Mutator 'Aborted' when injecting BPatch\_while [\#494](https://github.com/dyninst/dyninst/issues/494)
- Segfault \(nullptr deref\) in getCalledFunction\(\) [\#489](https://github.com/dyninst/dyninst/issues/489)
- The return value at BPatch\_exit points [\#391](https://github.com/dyninst/dyninst/issues/391)
- Instrumenting indirect callsites and calltargets with labels [\#386](https://github.com/dyninst/dyninst/issues/386)
- libelf install not copying libelf-0.168.so [\#375](https://github.com/dyninst/dyninst/issues/375)

**Merged pull requests:**

- Compilation fix for boost 1.70 [\#600](https://github.com/dyninst/dyninst/pull/600) ([mxz297](https://github.com/mxz297))
- Bump minimum CMake version to 3.4.0 [\#598](https://github.com/dyninst/dyninst/pull/598) ([hainest](https://github.com/hainest))
- Fix DwarfFrameParser, decodeDwarfExpression and DwarfResult [\#596](https://github.com/dyninst/dyninst/pull/596) ([sashanicolas](https://github.com/sashanicolas))
- simple working hybrid example of patchAPI  [\#588](https://github.com/dyninst/dyninst/pull/588) ([ianamason](https://github.com/ianamason))
- Remove assertions for jump table analysis [\#576](https://github.com/dyninst/dyninst/pull/576) ([mxz297](https://github.com/mxz297))
- Should set errno to zero before calling ptrace [\#569](https://github.com/dyninst/dyninst/pull/569) ([mxz297](https://github.com/mxz297))
- Implement ARMv8 IMFC and Load/Store Shared [\#562](https://github.com/dyninst/dyninst/pull/562) ([LER0ever](https://github.com/LER0ever))
- Fix ARM stack walking [\#559](https://github.com/dyninst/dyninst/pull/559) ([mxz297](https://github.com/mxz297))
- Fix tests in PIC mode [\#553](https://github.com/dyninst/dyninst/pull/553) ([mxz297](https://github.com/mxz297))
- Do not put newly added functions to .dynsym, which may cause the load [\#548](https://github.com/dyninst/dyninst/pull/548) ([mxz297](https://github.com/mxz297))
- Ppcle rhel8 fix [\#543](https://github.com/dyninst/dyninst/pull/543) ([mxz297](https://github.com/mxz297))
- Add some explicit casts from boost::tribool to bool for some code in [\#541](https://github.com/dyninst/dyninst/pull/541) ([mwkrentel](https://github.com/mwkrentel))
- Ignore Eclipse settings files and folders [\#539](https://github.com/dyninst/dyninst/pull/539) ([hainest](https://github.com/hainest))
- Remove Travis and AppVeyor CI [\#537](https://github.com/dyninst/dyninst/pull/537) ([hainest](https://github.com/hainest))
- Remove undefined behavior from bit-shifting code [\#536](https://github.com/dyninst/dyninst/pull/536) ([hainest](https://github.com/hainest))
- fix missing installation of libdw when installing elfutils [\#531](https://github.com/dyninst/dyninst/pull/531) ([rafzi](https://github.com/rafzi))
- Adjust Dyninst to support injecting CFGs for CUBINs [\#530](https://github.com/dyninst/dyninst/pull/530) ([Jokeren](https://github.com/Jokeren))
- Ppc pc fix [\#528](https://github.com/dyninst/dyninst/pull/528) ([mxz297](https://github.com/mxz297))
- Fix several bugs in common/src/pathName.C [\#527](https://github.com/dyninst/dyninst/pull/527) ([hainest](https://github.com/hainest))
- Power abi v2 fixes [\#519](https://github.com/dyninst/dyninst/pull/519) ([mxz297](https://github.com/mxz297))
- Don't use system header \<\> syntax for dyninst includes [\#518](https://github.com/dyninst/dyninst/pull/518) ([stanfordcox](https://github.com/stanfordcox))
- Fix linemaps for CUBINs [\#516](https://github.com/dyninst/dyninst/pull/516) ([jmellorcrummey](https://github.com/jmellorcrummey))
- as needed, add -fopenmp flag when linking executables [\#513](https://github.com/dyninst/dyninst/pull/513) ([jmellorcrummey](https://github.com/jmellorcrummey))
- Fix whileExpr not generating the correct code and causing abort [\#510](https://github.com/dyninst/dyninst/pull/510) ([LER0ever](https://github.com/LER0ever))
- Fixes signed/unsigned operations for comparison, multiplication, [\#509](https://github.com/dyninst/dyninst/pull/509) ([mxz297](https://github.com/mxz297))
- Add .dir suffix to examples/{codeCoverage,unstrip} [\#507](https://github.com/dyninst/dyninst/pull/507) ([stanfordcox](https://github.com/stanfordcox))
- Install docs into target doc dirs [\#506](https://github.com/dyninst/dyninst/pull/506) ([stanfordcox](https://github.com/stanfordcox))

## [v10.0.0](https://github.com/dyninst/dyninst/tree/v10.0.0) (2018-11-09)
[Full Changelog](https://github.com/dyninst/dyninst/compare/v9.3.2...v10.0.0)

**Implemented enhancements:**

- Migrate Dyninst from libdwarf to libdw [\#328](https://github.com/dyninst/dyninst/issues/328)
- Auto-detect Cilk [\#326](https://github.com/dyninst/dyninst/issues/326)
- Make ParseAPI \(optionally\) multi-threaded [\#145](https://github.com/dyninst/dyninst/issues/145)

**Fixed bugs:**

- Misc. x86 Decoding Issues [\#372](https://github.com/dyninst/dyninst/issues/372)
- Segfault when singleton\_object\_pool reallocs [\#359](https://github.com/dyninst/dyninst/issues/359)
- assert fails at insnCodeGen::generateBranchViaTrap [\#356](https://github.com/dyninst/dyninst/issues/356)
- Incorrect function boundaries for functions sharing code [\#149](https://github.com/dyninst/dyninst/issues/149)

**Closed issues:**

- Intel TBB install [\#500](https://github.com/dyninst/dyninst/issues/500)
- Compilation Error [\#491](https://github.com/dyninst/dyninst/issues/491)
- Build fails on Arch Linux [\#486](https://github.com/dyninst/dyninst/issues/486)
- Power 8 Instrimentation stack frame generation destroys vector register values [\#484](https://github.com/dyninst/dyninst/issues/484)
- Missing Vector Instructions and Reused Opcodes in Power 8 [\#483](https://github.com/dyninst/dyninst/issues/483)
- Codegen gen.point\(\) fails in most cases on Power \(returns NULL\) [\#482](https://github.com/dyninst/dyninst/issues/482)
- Power support for code generation of long branch calls  \(i.e.  branch with link to SPR\) [\#481](https://github.com/dyninst/dyninst/issues/481)
- relocation of branch +0x4 causes erratic behaviors on PowerPC [\#480](https://github.com/dyninst/dyninst/issues/480)
- Handling Relocation of Power 8 Function Preamble  [\#479](https://github.com/dyninst/dyninst/issues/479)
- SymtabAPI dumps core when reading an exception table for a KNL \(provided\) binary [\#477](https://github.com/dyninst/dyninst/issues/477)
- PCWidget::PCtoReturnAddr sets LR unnecessarily on non-x86 architectures [\#474](https://github.com/dyninst/dyninst/issues/474)
- Spack Build Fails with missing dependency on libiberty [\#473](https://github.com/dyninst/dyninst/issues/473)
- how to print the instruction which contains “cmp” [\#465](https://github.com/dyninst/dyninst/issues/465)
- Instrumentation blocks not saving/restoring correct registers. [\#461](https://github.com/dyninst/dyninst/issues/461)
- Segfault [\#456](https://github.com/dyninst/dyninst/issues/456)
- InsertSnippet does not check if "when" parameter is legal [\#455](https://github.com/dyninst/dyninst/issues/455)
- Heuristics to determined prologues [\#454](https://github.com/dyninst/dyninst/issues/454)
- processCreate crashed on aarch64 [\#449](https://github.com/dyninst/dyninst/issues/449)
- virtual bool AstCallNode::initRegisters\(codeGen&\): Assertion `callee' failed. Aborted \(core dumped\) [\#442](https://github.com/dyninst/dyninst/issues/442)
- undefined reference to symbol '\_ZNK7Dyninst14InstructionAPI11Instruction4sizeEv [\#440](https://github.com/dyninst/dyninst/issues/440)
- cannot find -ldwarf?  [\#439](https://github.com/dyninst/dyninst/issues/439)
- xdrrec\_create\(\) type cast error: char\* vs. void\* [\#438](https://github.com/dyninst/dyninst/issues/438)
- Error in build boost c++ library during installing Dyninst in Linux Ubuntu [\#435](https://github.com/dyninst/dyninst/issues/435)
- dyninst not saving/restoring a register used in insertSnippet [\#434](https://github.com/dyninst/dyninst/issues/434)
- Non-returning function analysis involving tail calls [\#433](https://github.com/dyninst/dyninst/issues/433)
- Several problems for analyzing powerpc binarieson x86 [\#432](https://github.com/dyninst/dyninst/issues/432)
- Patch without libdyninstAPI\_RT.so [\#428](https://github.com/dyninst/dyninst/issues/428)
- testsuite failures with separate debuginfo [\#423](https://github.com/dyninst/dyninst/issues/423)
- make\[2\]: \*\*\* No rule to make target `libiberty/libiberty.a', needed by `common/libcommon.so.9.3.2'.  Stop. [\#420](https://github.com/dyninst/dyninst/issues/420)
- make failing in latest branch [\#419](https://github.com/dyninst/dyninst/issues/419)
- Serious problem introduced when libdw was adopted [\#415](https://github.com/dyninst/dyninst/issues/415)
- emitElf::createLoadableSections uses hard-coded sh\_info [\#405](https://github.com/dyninst/dyninst/issues/405)
- Memory corruption in ROSE memory pool allocator [\#400](https://github.com/dyninst/dyninst/issues/400)
- ebx should be callee-saved [\#399](https://github.com/dyninst/dyninst/issues/399)
- getFirstSymbol\(\)-\>getMangledName SIGSEGV in PLT stub processing [\#396](https://github.com/dyninst/dyninst/issues/396)
- disassembling issue [\#395](https://github.com/dyninst/dyninst/issues/395)
- parseAPI shouldn't segfault if c++filt cannot demangle a symbol [\#390](https://github.com/dyninst/dyninst/issues/390)
- The value of BPatch\_registerExpr\(BPatch\_register reg\) [\#388](https://github.com/dyninst/dyninst/issues/388)
- Dyninst doesn't instrument the binary when it is compiled with -O3 flag \(g++\) [\#384](https://github.com/dyninst/dyninst/issues/384)
- The address of instructions [\#380](https://github.com/dyninst/dyninst/issues/380)
- Thunk call judgement condition [\#379](https://github.com/dyninst/dyninst/issues/379)
- Is there any APIs that can be used for finding the indirect calls? [\#378](https://github.com/dyninst/dyninst/issues/378)
- force boost build and force boost install [\#374](https://github.com/dyninst/dyninst/issues/374)
- Compilation issue wrt to libdwarf an then zlib [\#373](https://github.com/dyninst/dyninst/issues/373)
- CFG of stripped binary is empty [\#371](https://github.com/dyninst/dyninst/issues/371)
- BPatch\_statement::fileName\(\) returns the empty string since somewhere between 9.2.0...9.3.0 [\#363](https://github.com/dyninst/dyninst/issues/363)
- Power ABI v2 abstractions [\#119](https://github.com/dyninst/dyninst/issues/119)

**Merged pull requests:**

- Vector instruction support on Power and recycled opcode [\#498](https://github.com/dyninst/dyninst/pull/498) ([mxz297](https://github.com/mxz297))
- Build fixes for parallel building and xdr-related issues [\#496](https://github.com/dyninst/dyninst/pull/496) ([LER0ever](https://github.com/LER0ever))
- Merge parallel code parsing [\#488](https://github.com/dyninst/dyninst/pull/488) ([mxz297](https://github.com/mxz297))
- Power8 instrumentation fix [\#485](https://github.com/dyninst/dyninst/pull/485) ([mxz297](https://github.com/mxz297))
- symtab: fix memory error in Statement::getFile [\#469](https://github.com/dyninst/dyninst/pull/469) ([rafzi](https://github.com/rafzi))
- Vector categories merge into master [\#463](https://github.com/dyninst/dyninst/pull/463) ([jgalarowicz](https://github.com/jgalarowicz))
- Fix for crashing on relocating at unistrumentable points [\#460](https://github.com/dyninst/dyninst/pull/460) ([bwelton](https://github.com/bwelton))
- New walker to walk out of Instrimentation Frames FP [\#452](https://github.com/dyninst/dyninst/pull/452) ([bwelton](https://github.com/bwelton))
- Fixes for testsuite failures on powerv7 and block boundary aligning for overlapping instructions [\#446](https://github.com/dyninst/dyninst/pull/446) ([mxz297](https://github.com/mxz297))
- Remove unused rpc/xdr references. [\#445](https://github.com/dyninst/dyninst/pull/445) ([stanfordcox](https://github.com/stanfordcox))
- Very minor clean-up a particualrly ugly piece of code. [\#441](https://github.com/dyninst/dyninst/pull/441) ([thomasdullien](https://github.com/thomasdullien))
- Fixes for non-returning functions, endianness for cross architecture parsing, and powerpc instruction decoding [\#437](https://github.com/dyninst/dyninst/pull/437) ([mxz297](https://github.com/mxz297))
- add missing initialization for flags when Elf\_X is a memory image [\#430](https://github.com/dyninst/dyninst/pull/430) ([jmellorcrummey](https://github.com/jmellorcrummey))
- Fix sh\_info for VERNEED section [\#427](https://github.com/dyninst/dyninst/pull/427) ([nedwill](https://github.com/nedwill))
- Bugfixes windows [\#418](https://github.com/dyninst/dyninst/pull/418) ([mitalirawat](https://github.com/mitalirawat))
- AArch32 ARM Parsing Support [\#417](https://github.com/dyninst/dyninst/pull/417) ([rchyena](https://github.com/rchyena))
- Parallel Parsing changes [\#416](https://github.com/dyninst/dyninst/pull/416) ([jmellorcrummey](https://github.com/jmellorcrummey))
- Add linux-vdso64.so.1 to the library blacklist. [\#414](https://github.com/dyninst/dyninst/pull/414) ([stanfordcox](https://github.com/stanfordcox))
- Handle R\_X86\_64\_IRELATIVE relocation. [\#413](https://github.com/dyninst/dyninst/pull/413) ([stanfordcox](https://github.com/stanfordcox))
- Add basic support for EM\_CUDA binary type [\#410](https://github.com/dyninst/dyninst/pull/410) ([jmellorcrummey](https://github.com/jmellorcrummey))
- Fix the crash issue of retee [\#408](https://github.com/dyninst/dyninst/pull/408) ([mxz297](https://github.com/mxz297))
- fix dwarf symbol frame [\#403](https://github.com/dyninst/dyninst/pull/403) ([sashanicolas](https://github.com/sashanicolas))
- Merging my jump table improvements, att\_syntax, and arm semantics [\#401](https://github.com/dyninst/dyninst/pull/401) ([mxz297](https://github.com/mxz297))
- Fixing the destruction of objects under process control api. [\#382](https://github.com/dyninst/dyninst/pull/382) ([sashanicolas](https://github.com/sashanicolas))
- Add FORCE\_BOOST CMake option for @lee218llnl [\#381](https://github.com/dyninst/dyninst/pull/381) ([wrwilliams](https://github.com/wrwilliams))
- fix a bug when generating relocation index [\#370](https://github.com/dyninst/dyninst/pull/370) ([fengharry](https://github.com/fengharry))
- Clean up and refactor reaching definitions for better readability [\#369](https://github.com/dyninst/dyninst/pull/369) ([morehouse](https://github.com/morehouse))
- Sfm/fixes/indirection fixes [\#368](https://github.com/dyninst/dyninst/pull/368) ([morehouse](https://github.com/morehouse))
- Pull request for arm64/feature/relocation into master [\#367](https://github.com/dyninst/dyninst/pull/367) ([ssunny7](https://github.com/ssunny7))
- Pull request for att\_syntax into master [\#366](https://github.com/dyninst/dyninst/pull/366) ([ssunny7](https://github.com/ssunny7))
- Prevent non-PIC thunks from being classified as such [\#365](https://github.com/dyninst/dyninst/pull/365) ([morehouse](https://github.com/morehouse))
- Sfm/feature/reaching defs [\#364](https://github.com/dyninst/dyninst/pull/364) ([morehouse](https://github.com/morehouse))

## [v9.3.2](https://github.com/dyninst/dyninst/tree/v9.3.2) (2017-04-17)
[Full Changelog](https://github.com/dyninst/dyninst/compare/v9.3.1...v9.3.2)

**Fixed bugs:**

- memcpy can fail with rewriter tests [\#355](https://github.com/dyninst/dyninst/issues/355)
- dyninst does not build on i386 [\#343](https://github.com/dyninst/dyninst/issues/343)
- \[ARMv8 Decoding\] SQ\* instructions need to check bits with specific values [\#268](https://github.com/dyninst/dyninst/issues/268)
- \[ARMv8 Decoding\] SMADDL and SMSUBL should have 32-bit register for operands 2 and 3 [\#266](https://github.com/dyninst/dyninst/issues/266)
- \[ARMv8 Decoding\] SHL and SLI should have 0 for bit 11 [\#265](https://github.com/dyninst/dyninst/issues/265)
- \[ARMv8 Decoding\] NEG instruction must have size = '11' [\#263](https://github.com/dyninst/dyninst/issues/263)
- \[ARMv8 Decoding\] FMUL instruction cannot have size:q = '10' [\#262](https://github.com/dyninst/dyninst/issues/262)
- \[ARM Decoding\] FMUL instructions cannot have 'size:L' == '11' [\#258](https://github.com/dyninst/dyninst/issues/258)
- \[ARM Decoding\] Convert instruction immediate has reserved values \(currently ignored\) [\#257](https://github.com/dyninst/dyninst/issues/257)
- \[ARM Decoding\] FCVTXN should be FCVTXN2 [\#255](https://github.com/dyninst/dyninst/issues/255)
- \[ARM Decoding\] FCVT 'type' field cannot equal 'opc' field [\#254](https://github.com/dyninst/dyninst/issues/254)
- \[ARM Decoding\] Reserved size value for some vector register instructions is ignored [\#249](https://github.com/dyninst/dyninst/issues/249)
- \[ARM Decoding\] Stack pointer used where zero register should be [\#248](https://github.com/dyninst/dyninst/issues/248)
- \[ARM Decoding\] Signed multiply instructions ignore size resitrictions [\#247](https://github.com/dyninst/dyninst/issues/247)
- \[ARM Decoding\] Paired memory accesses must access aligned memory [\#245](https://github.com/dyninst/dyninst/issues/245)
- \[ARM Decoding\] Convert instruction immediates appear incorrect at 64 [\#241](https://github.com/dyninst/dyninst/issues/241)
- \[ARM Decoding\] Convert instruction immediate should not be larger than the register size [\#240](https://github.com/dyninst/dyninst/issues/240)
- \[ARM Syntax\] Signed immediates should be shown as signed [\#239](https://github.com/dyninst/dyninst/issues/239)
- \[ARM Decoding\] We should print the full operands of PRFUM [\#238](https://github.com/dyninst/dyninst/issues/238)
- \[ARM Decoding\] Bad shift amounts. [\#233](https://github.com/dyninst/dyninst/issues/233)
- \[ARM Decoding\] SIMD load instruction should be valid [\#223](https://github.com/dyninst/dyninst/issues/223)
- \[ARM Decoding\] Decoding of MOVK instruction ignores restriction on combination of size and hw bits [\#222](https://github.com/dyninst/dyninst/issues/222)
- \[ARM Decoding\] Decoding of ADDHN ignore reserved size bits [\#221](https://github.com/dyninst/dyninst/issues/221)
- \[ARM Syntax\] Zero register should have sizing, either XZR or WZR [\#220](https://github.com/dyninst/dyninst/issues/220)
- \[ARM Decoding\] Invalid CCMP and CCMN decoded as valid [\#219](https://github.com/dyninst/dyninst/issues/219)
- \[ARM Syntax\] Shifted immediate for CCMP and CCMN [\#218](https://github.com/dyninst/dyninst/issues/218)
- \[ARM Syntax\] Immediate out of range for LDRSB [\#217](https://github.com/dyninst/dyninst/issues/217)
- \[ARM Syntax\] Repeated register number as constant [\#216](https://github.com/dyninst/dyninst/issues/216)

**Merged pull requests:**

- More 9.3.2 cleanup [\#362](https://github.com/dyninst/dyninst/pull/362) ([wrwilliams](https://github.com/wrwilliams))
- Final cleanup bits for 9.3.2 [\#361](https://github.com/dyninst/dyninst/pull/361) ([wrwilliams](https://github.com/wrwilliams))
- Support 32-bit builds in Jenkins [\#350](https://github.com/dyninst/dyninst/pull/350) ([cuviper](https://github.com/cuviper))
- CMake fixup [\#349](https://github.com/dyninst/dyninst/pull/349) ([wrwilliams](https://github.com/wrwilliams))
- Add a macro MSROp [\#348](https://github.com/dyninst/dyninst/pull/348) ([ikitayama](https://github.com/ikitayama))
- Fix up exception handling code so that we only consider call instructions for exception sensitivity and its attendant emulation [\#347](https://github.com/dyninst/dyninst/pull/347) ([wrwilliams](https://github.com/wrwilliams))
- Refactor BPatch\_type so it always has a reference to its underlying symtab type. [\#346](https://github.com/dyninst/dyninst/pull/346) ([wrwilliams](https://github.com/wrwilliams))
- v9.3.x [\#341](https://github.com/dyninst/dyninst/pull/341) ([wrwilliams](https://github.com/wrwilliams))

## [v9.3.1](https://github.com/dyninst/dyninst/tree/v9.3.1) (2017-03-02)
[Full Changelog](https://github.com/dyninst/dyninst/compare/v9.3.0...v9.3.1)

**Fixed bugs:**

- Abort on PPC64LE when trying to get line number info [\#337](https://github.com/dyninst/dyninst/issues/337)
- Seg Fault on PPC64LE during attach [\#336](https://github.com/dyninst/dyninst/issues/336)
- Memory leaks in indirect control flow analysis [\#322](https://github.com/dyninst/dyninst/issues/322)
- BPatch\_binaryEdit::writeFile\(\) fails for stack diversification [\#311](https://github.com/dyninst/dyninst/issues/311)
- Stackwalk issue on arm64  [\#303](https://github.com/dyninst/dyninst/issues/303)
- Decode returns null shared pointer [\#288](https://github.com/dyninst/dyninst/issues/288)
- Operands labelled "\[empty\]" with operand type mismatch \(all with 0x67 prefix\) [\#203](https://github.com/dyninst/dyninst/issues/203)

**Merged pull requests:**

- Replaced a bunch of asserts with graceful error handling. [\#340](https://github.com/dyninst/dyninst/pull/340) ([wrwilliams](https://github.com/wrwilliams))
- Fix jump table analysis for lulesh  [\#338](https://github.com/dyninst/dyninst/pull/338) ([mxz297](https://github.com/mxz297))
- Better handling of anonymous structs and unions [\#335](https://github.com/dyninst/dyninst/pull/335) ([wrwilliams](https://github.com/wrwilliams))
- Fix memory leaks found with lsan [\#333](https://github.com/dyninst/dyninst/pull/333) ([wrwilliams](https://github.com/wrwilliams))
- Suppress debug message when no vsyscall page was found on arm64 [\#332](https://github.com/dyninst/dyninst/pull/332) ([wrwilliams](https://github.com/wrwilliams))
- Use ifdef to guard x86 code [\#331](https://github.com/dyninst/dyninst/pull/331) ([wrwilliams](https://github.com/wrwilliams))
- Fix memory leaks in indirect control flow. [\#329](https://github.com/dyninst/dyninst/pull/329) ([wrwilliams](https://github.com/wrwilliams))
- Fixes for API and dependency issues in 9.3.0 [\#323](https://github.com/dyninst/dyninst/pull/323) ([wrwilliams](https://github.com/wrwilliams))
- Changing the URL of libelf to download elfutil. [\#318](https://github.com/dyninst/dyninst/pull/318) ([sashanicolas](https://github.com/sashanicolas))
- Displacement validation checks updated [\#314](https://github.com/dyninst/dyninst/pull/314) ([jdetter](https://github.com/jdetter))
- Release9.3/fixes/icc binaries [\#310](https://github.com/dyninst/dyninst/pull/310) ([mxz297](https://github.com/mxz297))
- Fix test\_basic test error [\#309](https://github.com/dyninst/dyninst/pull/309) ([ikitayama](https://github.com/ikitayama))
- Make dyninst compile on x64 windows [\#168](https://github.com/dyninst/dyninst/pull/168) ([pefoley2](https://github.com/pefoley2))

## [v9.3.0](https://github.com/dyninst/dyninst/tree/v9.3.0) (2016-12-22)
[Full Changelog](https://github.com/dyninst/dyninst/compare/v9.2.0...v9.3.0)

**Implemented enhancements:**

- Update build requirements: drop libelf.so.0 support [\#242](https://github.com/dyninst/dyninst/issues/242)
- Merge AT&T syntax for x86 and ARM [\#210](https://github.com/dyninst/dyninst/issues/210)
- Two options for opcode should print one, not both. [\#199](https://github.com/dyninst/dyninst/issues/199)
- document proccontrol "tracking" APIs [\#151](https://github.com/dyninst/dyninst/issues/151)
- Read access to SymtabAPI and ParseAPI should be thread-safe [\#144](https://github.com/dyninst/dyninst/issues/144)
- New format for decoding tables [\#128](https://github.com/dyninst/dyninst/issues/128)
- InstructionAPI disassembly should match AT&T syntax [\#4](https://github.com/dyninst/dyninst/issues/4)
- x86 decoding and syntax fixes -- tentative [\#271](https://github.com/dyninst/dyninst/pull/271) ([jdetter](https://github.com/jdetter))
- findMain improvements [\#142](https://github.com/dyninst/dyninst/pull/142) ([jdetter](https://github.com/jdetter))
- Added asserts in liveness.C to prevent buffer underreads for [\#141](https://github.com/dyninst/dyninst/pull/141) ([jdetter](https://github.com/jdetter))

**Fixed bugs:**

- arm64 building current master fails   [\#304](https://github.com/dyninst/dyninst/issues/304)
- CMake boost error [\#300](https://github.com/dyninst/dyninst/issues/300)
- arm64 pc\_irpc test failure [\#296](https://github.com/dyninst/dyninst/issues/296)
- arm64 pc\_tls Library TLS was not the expected value [\#295](https://github.com/dyninst/dyninst/issues/295)
- arm64 Problem with simple example code in the ProcControlAPI Programmer’s Guide [\#290](https://github.com/dyninst/dyninst/issues/290)
- Stackanalysis asserts when analyzing \_\_start\_context in libc [\#283](https://github.com/dyninst/dyninst/issues/283)
- test1\_30 test failure [\#281](https://github.com/dyninst/dyninst/issues/281)
- 'nullptr' not declared for GCC 4.4.7-17 compiler [\#278](https://github.com/dyninst/dyninst/issues/278)
- Test4\_4 seems to be in deadlock on amd64\_ubu14 [\#274](https://github.com/dyninst/dyninst/issues/274)
- Testsuite not building with branch att\_syntax\_formerge [\#272](https://github.com/dyninst/dyninst/issues/272)
- AppVeyor having issues downloading boost [\#270](https://github.com/dyninst/dyninst/issues/270)
- PGI line info regression [\#243](https://github.com/dyninst/dyninst/issues/243)
- att\_syntax not building after merge [\#230](https://github.com/dyninst/dyninst/issues/230)
- VEX3 and EVEX assert - decoding invalid should throw exception or return error [\#213](https://github.com/dyninst/dyninst/issues/213)
- Race conditions with transient threads [\#208](https://github.com/dyninst/dyninst/issues/208)
- Dynist returns error "\(bad\)" decoding for valid sal variants. [\#207](https://github.com/dyninst/dyninst/issues/207)
- No opcode suffix and no register operand creates ambiguous size. [\#204](https://github.com/dyninst/dyninst/issues/204)
- Instructions produce FIXME opcodes, but otherwise appear correct [\#202](https://github.com/dyninst/dyninst/issues/202)
- Invalid instructions produce FIXME opcodes [\#201](https://github.com/dyninst/dyninst/issues/201)
- Invalid register numbers for VEX3 instructions [\#200](https://github.com/dyninst/dyninst/issues/200)
- Opcode suffixes do not take into account prefix bytes [\#198](https://github.com/dyninst/dyninst/issues/198)
- Incorrect operand suffix for register size for string instruction [\#197](https://github.com/dyninst/dyninst/issues/197)
- Decode incorrect VEX3 as valid instruction [\#196](https://github.com/dyninst/dyninst/issues/196)
- Decode incorrect VEX2 as valid instruction [\#195](https://github.com/dyninst/dyninst/issues/195)
- Double printing first operand [\#193](https://github.com/dyninst/dyninst/issues/193)
- findMain failing on master under Jenkins [\#188](https://github.com/dyninst/dyninst/issues/188)
- Call emulation causing testsuite failures [\#187](https://github.com/dyninst/dyninst/issues/187)
- PPC64 generateBranchViaTrap: Assertion `isCall == false' failed. [\#175](https://github.com/dyninst/dyninst/issues/175)
- error: ‘class func\_instance’ has no member named ‘freeStackMod’ [\#165](https://github.com/dyninst/dyninst/issues/165)
- make install not working on latest master [\#160](https://github.com/dyninst/dyninst/issues/160)
- Add generated cotire directories to gitignore [\#158](https://github.com/dyninst/dyninst/issues/158)
- test\_pt\_ls failing on master \(RHEL6\) [\#157](https://github.com/dyninst/dyninst/issues/157)
- Assertion failed with a bad DYNINSTAPI\_RT\_LIB [\#153](https://github.com/dyninst/dyninst/issues/153)
- Assertion failure in DwarfWalker [\#152](https://github.com/dyninst/dyninst/issues/152)
- Segfault when a process is attached without specifying exe [\#146](https://github.com/dyninst/dyninst/issues/146)
- Indirect jumps that use jump tables are not relocated correctly [\#139](https://github.com/dyninst/dyninst/issues/139)
- PC-relative read in indirect jump was not modified during relocation [\#133](https://github.com/dyninst/dyninst/issues/133)
- stackanalysis assert while running in 32bit mode on master [\#131](https://github.com/dyninst/dyninst/issues/131)
- Assert in StackAnalysis on release9.2/fixes/test\_pt\_ls [\#130](https://github.com/dyninst/dyninst/issues/130)
- Cannot find malloc symbol in libc.so [\#126](https://github.com/dyninst/dyninst/issues/126)
- test\_pt\_ls fails with Dyninst master [\#123](https://github.com/dyninst/dyninst/issues/123)
- Line information fixes for HPCToolkit [\#122](https://github.com/dyninst/dyninst/issues/122)
- Rewrite exception handlers to adjust for relocated code [\#121](https://github.com/dyninst/dyninst/issues/121)
- Assertion failure during rewriting [\#116](https://github.com/dyninst/dyninst/issues/116)
- Crash during liveness analysis [\#114](https://github.com/dyninst/dyninst/issues/114)
- Segfault during traversal of slice generated in StackMod [\#113](https://github.com/dyninst/dyninst/issues/113)
- Segfault when parsing binary with no functions [\#53](https://github.com/dyninst/dyninst/issues/53)
- rewriter tests fail/crash on ppc64 [\#34](https://github.com/dyninst/dyninst/issues/34)
- Warnings not being properly disabled under Visual Studio [\#26](https://github.com/dyninst/dyninst/issues/26)
- Cleanup ppc \(and old gcc\) issues [\#277](https://github.com/dyninst/dyninst/pull/277) ([wrwilliams](https://github.com/wrwilliams))
- CMake fixes for Cotie and GCC 4.4 compatibility [\#164](https://github.com/dyninst/dyninst/pull/164) ([jdetter](https://github.com/jdetter))
- fix insnCodeGen::modifyData's 64-bit conversion [\#163](https://github.com/dyninst/dyninst/pull/163) ([cuviper](https://github.com/cuviper))
- Fixes for test\_pt\_ls [\#143](https://github.com/dyninst/dyninst/pull/143) ([jdetter](https://github.com/jdetter))

**Closed issues:**

- ABI changes from v9.2.0 to v9.2\_patches [\#136](https://github.com/dyninst/dyninst/issues/136)

**Merged pull requests:**

- ARM64 fixes for stack walking [\#307](https://github.com/dyninst/dyninst/pull/307) ([wrwilliams](https://github.com/wrwilliams))
- Update AssignmentConverter instantiation to make use of stack analysis explicit. [\#306](https://github.com/dyninst/dyninst/pull/306) ([wrwilliams](https://github.com/wrwilliams))
- Bug fix for PR\#294 [\#302](https://github.com/dyninst/dyninst/pull/302) ([wrwilliams](https://github.com/wrwilliams))
- Enhance DWARF parser to recognize \(and skip\) DW\_TAG\_member entries for static fields [\#299](https://github.com/dyninst/dyninst/pull/299) ([ma-neumann](https://github.com/ma-neumann))
- Fix omitting leading 0's when concatenating hex bytes in stringstream [\#298](https://github.com/dyninst/dyninst/pull/298) ([ma-neumann](https://github.com/ma-neumann))
- More manual fixes [\#297](https://github.com/dyninst/dyninst/pull/297) ([mxz297](https://github.com/mxz297))
- arm64: remove syscall tracing workaround [\#294](https://github.com/dyninst/dyninst/pull/294) ([ikitayama](https://github.com/ikitayama))
- Autodetect RTLib location [\#293](https://github.com/dyninst/dyninst/pull/293) ([wrwilliams](https://github.com/wrwilliams))
- Small fixes [\#292](https://github.com/dyninst/dyninst/pull/292) ([wrwilliams](https://github.com/wrwilliams))
- Update manual version numbers and minor fixes [\#291](https://github.com/dyninst/dyninst/pull/291) ([mxz297](https://github.com/mxz297))
- Remove stackanalysis assertions [\#289](https://github.com/dyninst/dyninst/pull/289) ([wrwilliams](https://github.com/wrwilliams))
- Line info cleanup [\#287](https://github.com/dyninst/dyninst/pull/287) ([wrwilliams](https://github.com/wrwilliams))
- Passing test\_pt\_ls \(at least on RHEL7\) [\#286](https://github.com/dyninst/dyninst/pull/286) ([mxz297](https://github.com/mxz297))
- Sfm/fixes/untouched blocks [\#285](https://github.com/dyninst/dyninst/pull/285) ([morehouse](https://github.com/morehouse))
- Use in-place translation [\#284](https://github.com/dyninst/dyninst/pull/284) ([wrwilliams](https://github.com/wrwilliams))
- common: use ptrace if yama blocked process\_vm\_readv/writev [\#280](https://github.com/dyninst/dyninst/pull/280) ([cuviper](https://github.com/cuviper))
- Fix dependency of LibDwarf [\#279](https://github.com/dyninst/dyninst/pull/279) ([rafzi](https://github.com/rafzi))
- Compiling DynInst v9.2.0 on arm64 [\#273](https://github.com/dyninst/dyninst/pull/273) ([ikitayama](https://github.com/ikitayama))
- proccontrol: fix double-increment while erasing a dead process [\#261](https://github.com/dyninst/dyninst/pull/261) ([cuviper](https://github.com/cuviper))
- Document some of the ProcControlAPI options in PlatFeatures.h. [\#260](https://github.com/dyninst/dyninst/pull/260) ([morehouse](https://github.com/morehouse))
- proccontrol: scrub newly created threads that fail to attach [\#259](https://github.com/dyninst/dyninst/pull/259) ([cuviper](https://github.com/cuviper))
- RT: trymmap should retry if the result is out of range [\#231](https://github.com/dyninst/dyninst/pull/231) ([cuviper](https://github.com/cuviper))
- Merge ARM instruction semantics and jump table parsing [\#228](https://github.com/dyninst/dyninst/pull/228) ([mxz297](https://github.com/mxz297))
- line info bugfixes [\#226](https://github.com/dyninst/dyninst/pull/226) ([wrwilliams](https://github.com/wrwilliams))
- LibraryTracker documentation [\#225](https://github.com/dyninst/dyninst/pull/225) ([wrwilliams](https://github.com/wrwilliams))
- proccontrol: Synchronize additional threads found during attach [\#214](https://github.com/dyninst/dyninst/pull/214) ([cuviper](https://github.com/cuviper))
- Fix errors when thread disappears during attach [\#212](https://github.com/dyninst/dyninst/pull/212) ([cuviper](https://github.com/cuviper))
- Added symbol linkage support for GNU unique linkage types [\#209](https://github.com/dyninst/dyninst/pull/209) ([bwelton](https://github.com/bwelton))
- Fix icc warning flags [\#206](https://github.com/dyninst/dyninst/pull/206) ([pefoley2](https://github.com/pefoley2))
- Make dynC work on windows [\#205](https://github.com/dyninst/dyninst/pull/205) ([pefoley2](https://github.com/pefoley2))
- pefoley2-boost\_win [\#194](https://github.com/dyninst/dyninst/pull/194) ([wrwilliams](https://github.com/wrwilliams))
- ElfX: add xlate funcs [\#192](https://github.com/dyninst/dyninst/pull/192) ([wrwilliams](https://github.com/wrwilliams))
- proccontrol: check thread handle before calculating TLS [\#191](https://github.com/dyninst/dyninst/pull/191) ([cuviper](https://github.com/cuviper))
- Improved the findMain analysis significantly [\#189](https://github.com/dyninst/dyninst/pull/189) ([jdetter](https://github.com/jdetter))
- Exception frame rewriting fixes [\#186](https://github.com/dyninst/dyninst/pull/186) ([jdetter](https://github.com/jdetter))
- String table now includes an entry at zero for "unknown", so don't subtract from the DWARF file number. [\#185](https://github.com/dyninst/dyninst/pull/185) ([wrwilliams](https://github.com/wrwilliams))
- cmake: Fix not building when libiberty is automatically installed [\#183](https://github.com/dyninst/dyninst/pull/183) ([rafzi](https://github.com/rafzi))
- Determine the architecture of an ELF by looking at the file header in… [\#182](https://github.com/dyninst/dyninst/pull/182) ([rafzi](https://github.com/rafzi))
- Documentation updates -- tentative [\#180](https://github.com/dyninst/dyninst/pull/180) ([jdetter](https://github.com/jdetter))
- Fix operand types for vpand [\#177](https://github.com/dyninst/dyninst/pull/177) ([BlairArchibald](https://github.com/BlairArchibald))
- symtabAPI: dont expect a data segment in elf; eliminate dead code [\#173](https://github.com/dyninst/dyninst/pull/173) ([rafzi](https://github.com/rafzi))
- elf: fix uninitialized isBigEndian for archives; duplicate code removal [\#172](https://github.com/dyninst/dyninst/pull/172) ([rafzi](https://github.com/rafzi))
- Support building with LTO [\#171](https://github.com/dyninst/dyninst/pull/171) ([pefoley2](https://github.com/pefoley2))
- Fix various warnings under Visual Studio [\#169](https://github.com/dyninst/dyninst/pull/169) ([pefoley2](https://github.com/pefoley2))
- Guard StackMod implementation from non-x86 architectures. [\#166](https://github.com/dyninst/dyninst/pull/166) ([morehouse](https://github.com/morehouse))
- Added generated cotire directories to gitignore [\#159](https://github.com/dyninst/dyninst/pull/159) ([jdetter](https://github.com/jdetter))
- symtabAPI: use the known type for new relocations [\#156](https://github.com/dyninst/dyninst/pull/156) ([cuviper](https://github.com/cuviper))
- Sfm/feature/interproc analysis [\#155](https://github.com/dyninst/dyninst/pull/155) ([morehouse](https://github.com/morehouse))
- Remove unnecessary assert from dwarfWalker.  Fixes \#152. [\#154](https://github.com/dyninst/dyninst/pull/154) ([morehouse](https://github.com/morehouse))
- proccontrol: fix process attachment without an exe [\#148](https://github.com/dyninst/dyninst/pull/148) ([cuviper](https://github.com/cuviper))
- proccontrol: fix process attachment without an exe [\#147](https://github.com/dyninst/dyninst/pull/147) ([cuviper](https://github.com/cuviper))
- Restored dyn\_regs.h to v9.2.0 version [\#140](https://github.com/dyninst/dyninst/pull/140) ([jdetter](https://github.com/jdetter))
- Fixes significant ABI issues on v9.2\_patches. [\#138](https://github.com/dyninst/dyninst/pull/138) ([jdetter](https://github.com/jdetter))
- Line info optimizations [\#135](https://github.com/dyninst/dyninst/pull/135) ([wrwilliams](https://github.com/wrwilliams))
- Matt's patch applied -- fixes test\_stack\_1 issue [\#134](https://github.com/dyninst/dyninst/pull/134) ([jdetter](https://github.com/jdetter))
- Rose build fixes [\#129](https://github.com/dyninst/dyninst/pull/129) ([pefoley2](https://github.com/pefoley2))
- Merge arm64/feature/semantics\_setup into master [\#127](https://github.com/dyninst/dyninst/pull/127) ([ssunny7](https://github.com/ssunny7))
- V9.2 patches [\#124](https://github.com/dyninst/dyninst/pull/124) ([jdetter](https://github.com/jdetter))
- Release9.2/fixes/liveness patch [\#118](https://github.com/dyninst/dyninst/pull/118) ([jdetter](https://github.com/jdetter))
- Add config to build using Appveyor [\#19](https://github.com/dyninst/dyninst/pull/19) ([pefoley2](https://github.com/pefoley2))
- Fix warnings produced when compiling with clang [\#14](https://github.com/dyninst/dyninst/pull/14) ([pefoley2](https://github.com/pefoley2))

## [v9.2.0](https://github.com/dyninst/dyninst/tree/v9.2.0) (2016-06-29)
[Full Changelog](https://github.com/dyninst/dyninst/compare/v9.1.0...v9.2.0)

**Implemented enhancements:**

- Dataflow documentation: Stack Analysis [\#31](https://github.com/dyninst/dyninst/issues/31)
- Dataflow documentation: Slicing and SymEval [\#30](https://github.com/dyninst/dyninst/issues/30)
- Dataflow documentation: Intro/Abstractions [\#29](https://github.com/dyninst/dyninst/issues/29)

**Fixed bugs:**

- Rewriting with StackMods broken [\#111](https://github.com/dyninst/dyninst/issues/111)
- Assertion during libxul PIE rewriting \(VEX/master\) [\#110](https://github.com/dyninst/dyninst/issues/110)
- Testsuite failures on master/VEX for 32 bit platform [\#104](https://github.com/dyninst/dyninst/issues/104)
- RHEL6 "cannot allocate memory in static TLS block" [\#101](https://github.com/dyninst/dyninst/issues/101)
- Infinite recursion in TLS tramp guard [\#98](https://github.com/dyninst/dyninst/issues/98)
- Rewritten binary dies with SIGILL [\#96](https://github.com/dyninst/dyninst/issues/96)
- pc\_fork\_exec failure on master and VEX [\#94](https://github.com/dyninst/dyninst/issues/94)
- Rewritten libc.so is not usable [\#93](https://github.com/dyninst/dyninst/issues/93)
- dyninstAPI\_RT build failure on Windows [\#92](https://github.com/dyninst/dyninst/issues/92)
- amd64\_7\_arg\_call passing, then segfaulting from shared pointer on VEX [\#90](https://github.com/dyninst/dyninst/issues/90)
- Multiple testsuite failures on VEX [\#89](https://github.com/dyninst/dyninst/issues/89)
- New instruction decoding problem in master branch [\#88](https://github.com/dyninst/dyninst/issues/88)
- Build failure on windows [\#86](https://github.com/dyninst/dyninst/issues/86)
- Dyninst parsing part of function multiple times [\#83](https://github.com/dyninst/dyninst/issues/83)
- runTest -test pc\_addlibrary fails/dumps core \(actually, none of the proccontrol tests run\) [\#81](https://github.com/dyninst/dyninst/issues/81)
- Problems with Instruction API parsing x86-64 binaries: xhpl executable [\#80](https://github.com/dyninst/dyninst/issues/80)
- Problems with Instruction API parsing x86-64 binaries: sqrtsd [\#79](https://github.com/dyninst/dyninst/issues/79)
- parseThat not outputting executable binary \(Exec format error\) [\#71](https://github.com/dyninst/dyninst/issues/71)
- symtabAPI fails to link on 32bit linux [\#70](https://github.com/dyninst/dyninst/issues/70)
- Dyndwarf assert thrown on latest master [\#67](https://github.com/dyninst/dyninst/issues/67)
- decodeOneOperand\(\) called with unknown addressing method 18 [\#66](https://github.com/dyninst/dyninst/issues/66)
- Segfault during PIE rewriting [\#65](https://github.com/dyninst/dyninst/issues/65)
- walkSingleFrame run against local process on WIndows crashes [\#64](https://github.com/dyninst/dyninst/issues/64)
- Symtab can't find any functions without libc [\#58](https://github.com/dyninst/dyninst/issues/58)
- Rewriting of binaries with GNU\_RELRO segment fails on master [\#52](https://github.com/dyninst/dyninst/issues/52)
- Master timeout on test4\_2 and test4\_4 on Fedora23 [\#50](https://github.com/dyninst/dyninst/issues/50)
- ERROR: failed bind/eval [\#48](https://github.com/dyninst/dyninst/issues/48)
- Possible slicing/frame issue [\#44](https://github.com/dyninst/dyninst/issues/44)
- dyninstAPI documentation typo [\#41](https://github.com/dyninst/dyninst/issues/41)
- MachRegister::getReturnAddress not implemented on x86/x86\_64 [\#40](https://github.com/dyninst/dyninst/issues/40)
- Master not building with boost 1.58.0 \(undefined references\) [\#38](https://github.com/dyninst/dyninst/issues/38)
- ptrace\_peektext failing and producing spam in thread tests [\#36](https://github.com/dyninst/dyninst/issues/36)
- Test 4\_1, 4\_2, and 4\_4 fail on ppc64 [\#35](https://github.com/dyninst/dyninst/issues/35)
- test1\_33 fails on ppc64 [\#33](https://github.com/dyninst/dyninst/issues/33)
- PLT entries misparsed on ARM [\#32](https://github.com/dyninst/dyninst/issues/32)
- BPatch\_function.C.o build failure on PPC64le [\#23](https://github.com/dyninst/dyninst/issues/23)
- getABIVersion\(\) not defined in Object-nt.h [\#21](https://github.com/dyninst/dyninst/issues/21)
- Enable build only if .travis.yml is present option for repo [\#20](https://github.com/dyninst/dyninst/issues/20)
- Bundling cvconst.h [\#17](https://github.com/dyninst/dyninst/issues/17)
- Missing htobe32 function under Visual Studio [\#16](https://github.com/dyninst/dyninst/issues/16)
- Line info gets misfiled into incorrect Modules [\#15](https://github.com/dyninst/dyninst/issues/15)
- ./runTests -test test1\_1 fails on ppc64 platform [\#8](https://github.com/dyninst/dyninst/issues/8)
- LivenessAnalyzer::isMMX assertion failure [\#7](https://github.com/dyninst/dyninst/issues/7)
- "Bad addressing mode!" in F23 libm.so [\#6](https://github.com/dyninst/dyninst/issues/6)
- pc\_tls fails [\#3](https://github.com/dyninst/dyninst/issues/3)
- pc\_add\_library fails in attach mode on 9.2.x [\#2](https://github.com/dyninst/dyninst/issues/2)
- test\_mem\_\* fails on 9.2.x [\#1](https://github.com/dyninst/dyninst/issues/1)

**Merged pull requests:**

- Disabled condition decoding in stack rewriting. [\#112](https://github.com/dyninst/dyninst/pull/112) ([jdetter](https://github.com/jdetter))
- symtabAPI: Apply corrections for the library\_adjust page offset [\#109](https://github.com/dyninst/dyninst/pull/109) ([cuviper](https://github.com/cuviper))
- fix dll linkage on windows [\#108](https://github.com/dyninst/dyninst/pull/108) ([pefoley2](https://github.com/pefoley2))
- Rewriter fixes, instruction decoding fixes. [\#107](https://github.com/dyninst/dyninst/pull/107) ([jdetter](https://github.com/jdetter))
- Don't use broken llvm apt mirror [\#105](https://github.com/dyninst/dyninst/pull/105) ([pefoley2](https://github.com/pefoley2))
- Fixes for jump table, instruction decoding, windows proccontrol [\#102](https://github.com/dyninst/dyninst/pull/102) ([mxz297](https://github.com/mxz297))
- RTlib: use static TLS for the tramp guard [\#99](https://github.com/dyninst/dyninst/pull/99) ([cuviper](https://github.com/cuviper))
- Visual Studio Build Fixes [\#97](https://github.com/dyninst/dyninst/pull/97) ([pefoley2](https://github.com/pefoley2))
- Refactor RTheap to avoid doing arithmetic with void\* [\#95](https://github.com/dyninst/dyninst/pull/95) ([pefoley2](https://github.com/pefoley2))
- Latest fix for rice folks [\#91](https://github.com/dyninst/dyninst/pull/91) ([mxz297](https://github.com/mxz297))
- Build fix for addrtranslate [\#87](https://github.com/dyninst/dyninst/pull/87) ([pefoley2](https://github.com/pefoley2))
- Temporarily disable broken clang build [\#85](https://github.com/dyninst/dyninst/pull/85) ([pefoley2](https://github.com/pefoley2))
- Simplify INTERP logic for better consistency [\#84](https://github.com/dyninst/dyninst/pull/84) ([cuviper](https://github.com/cuviper))
- Fix for square root floating point instructions [\#82](https://github.com/dyninst/dyninst/pull/82) ([jdetter](https://github.com/jdetter))
- Set defaults for Windows first-party stack walking: library tracker [\#78](https://github.com/dyninst/dyninst/pull/78) ([wrwilliams](https://github.com/wrwilliams))
- Define htobe on Windows/MSVC as a wrapper for \_byteswap\_ulong [\#77](https://github.com/dyninst/dyninst/pull/77) ([wrwilliams](https://github.com/wrwilliams))
- release9.2/bugs/rtheap\_mmap\_only [\#76](https://github.com/dyninst/dyninst/pull/76) ([wrwilliams](https://github.com/wrwilliams))
- Fix 32-bit build; rename emitElf64 to emitElf [\#73](https://github.com/dyninst/dyninst/pull/73) ([wrwilliams](https://github.com/wrwilliams))
- Truncate PTRACE\_GETEVENTMSG exit status to int [\#69](https://github.com/dyninst/dyninst/pull/69) ([cuviper](https://github.com/cuviper))
- Fix Function/Module mapping [\#61](https://github.com/dyninst/dyninst/pull/61) ([wrwilliams](https://github.com/wrwilliams))
- Fix rewriting interp sections and debug symbols [\#57](https://github.com/dyninst/dyninst/pull/57) ([cuviper](https://github.com/cuviper))
- Fix uninitialized data in rewriter elf\_update [\#54](https://github.com/dyninst/dyninst/pull/54) ([cuviper](https://github.com/cuviper))
- Merge 9.2 branch back to master [\#51](https://github.com/dyninst/dyninst/pull/51) ([wrwilliams](https://github.com/wrwilliams))
- fix\#48 [\#49](https://github.com/dyninst/dyninst/pull/49) ([wrwilliams](https://github.com/wrwilliams))
- symtabAPI: don't free cuDIE in parseLineInfoForCU [\#47](https://github.com/dyninst/dyninst/pull/47) ([cuviper](https://github.com/cuviper))
- symtabAPI: comment out some debug chatter [\#46](https://github.com/dyninst/dyninst/pull/46) ([cuviper](https://github.com/cuviper))
- Remove low-level warnings from ptrace read/write failures [\#43](https://github.com/dyninst/dyninst/pull/43) ([cuviper](https://github.com/cuviper))
- Recover from a bad force push. [\#42](https://github.com/dyninst/dyninst/pull/42) ([wrwilliams](https://github.com/wrwilliams))
- Remove low-level warnings from ptrace read/write failures [\#39](https://github.com/dyninst/dyninst/pull/39) ([cuviper](https://github.com/cuviper))
- Fix \#23, build failure on PPC64le [\#25](https://github.com/dyninst/dyninst/pull/25) ([pefoley2](https://github.com/pefoley2))
- Provided base class virtual for getABIVersion\(\) that returns false when not implemented [\#24](https://github.com/dyninst/dyninst/pull/24) ([mcfadden8](https://github.com/mcfadden8))
- Allow dyninst to be compiled using clang [\#13](https://github.com/dyninst/dyninst/pull/13) ([pefoley2](https://github.com/pefoley2))
- Add initial file for travis [\#12](https://github.com/dyninst/dyninst/pull/12) ([pefoley2](https://github.com/pefoley2))
- Misc fixes and improvements [\#11](https://github.com/dyninst/dyninst/pull/11) ([pefoley2](https://github.com/pefoley2))
- ProcControl and Symtab support for ppc64le [\#10](https://github.com/dyninst/dyninst/pull/10) ([mcfadden8](https://github.com/mcfadden8))

## [v9.1.0](https://github.com/dyninst/dyninst/tree/v9.1.0) (2015-12-16)
[Full Changelog](https://github.com/dyninst/dyninst/compare/v9.0.3...v9.1.0)

## [v9.0.3](https://github.com/dyninst/dyninst/tree/v9.0.3) (2015-08-26)
[Full Changelog](https://github.com/dyninst/dyninst/compare/v9.0.2...v9.0.3)

## [v9.0.2](https://github.com/dyninst/dyninst/tree/v9.0.2) (2015-08-24)
[Full Changelog](https://github.com/dyninst/dyninst/compare/v9.0.1...v9.0.2)

## [v9.0.1](https://github.com/dyninst/dyninst/tree/v9.0.1) (2015-08-21)
[Full Changelog](https://github.com/dyninst/dyninst/compare/v9.0.0...v9.0.1)

## [v9.0.0](https://github.com/dyninst/dyninst/tree/v9.0.0) (2015-08-20)
[Full Changelog](https://github.com/dyninst/dyninst/compare/milestone_5...v9.0.0)

## [milestone_5](https://github.com/dyninst/dyninst/tree/milestone_5) (2015-01-15)
[Full Changelog](https://github.com/dyninst/dyninst/compare/milestone_4...milestone_5)

## [milestone_4](https://github.com/dyninst/dyninst/tree/milestone_4) (2015-01-14)
[Full Changelog](https://github.com/dyninst/dyninst/compare/milestone_3...milestone_4)

## [milestone_3](https://github.com/dyninst/dyninst/tree/milestone_3) (2015-01-12)
[Full Changelog](https://github.com/dyninst/dyninst/compare/v8.2.1...milestone_3)

## [v8.2.1](https://github.com/dyninst/dyninst/tree/v8.2.1) (2014-10-30)
[Full Changelog](https://github.com/dyninst/dyninst/compare/v8.2.0.1...v8.2.1)

## [v8.2.0.1](https://github.com/dyninst/dyninst/tree/v8.2.0.1) (2014-08-19)
[Full Changelog](https://github.com/dyninst/dyninst/compare/v8.2.0...v8.2.0.1)

## [v8.2.0](https://github.com/dyninst/dyninst/tree/v8.2.0) (2014-08-19)
[Full Changelog](https://github.com/dyninst/dyninst/compare/v8.1.2...v8.2.0)

## [v8.1.2](https://github.com/dyninst/dyninst/tree/v8.1.2) (2013-06-18)
[Full Changelog](https://github.com/dyninst/dyninst/compare/pre8.1.2RC3...v8.1.2)

## [pre8.1.2RC3](https://github.com/dyninst/dyninst/tree/pre8.1.2RC3) (2013-06-07)
[Full Changelog](https://github.com/dyninst/dyninst/compare/pre8.1.2RC2...pre8.1.2RC3)

## [pre8.1.2RC2](https://github.com/dyninst/dyninst/tree/pre8.1.2RC2) (2013-06-04)
[Full Changelog](https://github.com/dyninst/dyninst/compare/pre8.1.2RC1...pre8.1.2RC2)

## [pre8.1.2RC1](https://github.com/dyninst/dyninst/tree/pre8.1.2RC1) (2013-05-29)
[Full Changelog](https://github.com/dyninst/dyninst/compare/v8.1.1...pre8.1.2RC1)

## [v8.1.1](https://github.com/dyninst/dyninst/tree/v8.1.1) (2013-03-14)
[Full Changelog](https://github.com/dyninst/dyninst/compare/pre-8.1RC1...v8.1.1)

## [pre-8.1RC1](https://github.com/dyninst/dyninst/tree/pre-8.1RC1) (2013-03-01)
[Full Changelog](https://github.com/dyninst/dyninst/compare/pre-8.1...pre-8.1RC1)

## [pre-8.1](https://github.com/dyninst/dyninst/tree/pre-8.1) (2013-02-22)
[Full Changelog](https://github.com/dyninst/dyninst/compare/v8.0...pre-8.1)

## [v8.0](https://github.com/dyninst/dyninst/tree/v8.0) (2012-11-19)
[Full Changelog](https://github.com/dyninst/dyninst/compare/SW8.0CrayRC3...v8.0)

## [SW8.0CrayRC3](https://github.com/dyninst/dyninst/tree/SW8.0CrayRC3) (2012-10-15)
[Full Changelog](https://github.com/dyninst/dyninst/compare/SW8.0RC2...SW8.0CrayRC3)

## [SW8.0RC2](https://github.com/dyninst/dyninst/tree/SW8.0RC2) (2012-10-15)
[Full Changelog](https://github.com/dyninst/dyninst/compare/SW8.0RC1...SW8.0RC2)

## [SW8.0RC1](https://github.com/dyninst/dyninst/tree/SW8.0RC1) (2012-10-15)
[Full Changelog](https://github.com/dyninst/dyninst/compare/kevin-final...SW8.0RC1)

## [kevin-final](https://github.com/dyninst/dyninst/tree/kevin-final) (2012-01-11)
[Full Changelog](https://github.com/dyninst/dyninst/compare/Release7_0...kevin-final)

## [Release7_0](https://github.com/dyninst/dyninst/tree/Release7_0) (2011-03-23)
[Full Changelog](https://github.com/dyninst/dyninst/compare/Release6_1...Release7_0)

## [Release6_1](https://github.com/dyninst/dyninst/tree/Release6_1) (2009-12-04)
[Full Changelog](https://github.com/dyninst/dyninst/compare/Release6_0...Release6_1)

## [Release6_0](https://github.com/dyninst/dyninst/tree/Release6_0) (2009-06-30)
[Full Changelog](https://github.com/dyninst/dyninst/compare/SanDiegoDistro...Release6_0)

## [SanDiegoDistro](https://github.com/dyninst/dyninst/tree/SanDiegoDistro) (2007-11-21)
[Full Changelog](https://github.com/dyninst/dyninst/compare/Release5_1...SanDiegoDistro)

## [Release5_1](https://github.com/dyninst/dyninst/tree/Release5_1) (2007-05-31)
[Full Changelog](https://github.com/dyninst/dyninst/compare/release5_1_beta...Release5_1)

## [release5_1_beta](https://github.com/dyninst/dyninst/tree/release5_1_beta) (2007-01-04)
[Full Changelog](https://github.com/dyninst/dyninst/compare/Release5_0...release5_1_beta)

## [Release5_0](https://github.com/dyninst/dyninst/tree/Release5_0) (2006-07-05)
[Full Changelog](https://github.com/dyninst/dyninst/compare/pre_multitramp...Release5_0)

## [pre_multitramp](https://github.com/dyninst/dyninst/tree/pre_multitramp) (2005-07-19)
[Full Changelog](https://github.com/dyninst/dyninst/compare/Release4_2_1...pre_multitramp)

## [Release4_2_1](https://github.com/dyninst/dyninst/tree/Release4_2_1) (2005-04-12)
[Full Changelog](https://github.com/dyninst/dyninst/compare/mrnet-1_1...Release4_2_1)

## [mrnet-1_1](https://github.com/dyninst/dyninst/tree/mrnet-1_1) (2005-04-04)
[Full Changelog](https://github.com/dyninst/dyninst/compare/Release4_2...mrnet-1_1)

## [Release4_2](https://github.com/dyninst/dyninst/tree/Release4_2) (2005-03-23)
[Full Changelog](https://github.com/dyninst/dyninst/compare/Dyninst4_1...Release4_2)

## [Dyninst4_1](https://github.com/dyninst/dyninst/tree/Dyninst4_1) (2004-04-28)
[Full Changelog](https://github.com/dyninst/dyninst/compare/mrnet-1-0...Dyninst4_1)

## [mrnet-1-0](https://github.com/dyninst/dyninst/tree/mrnet-1-0) (2003-09-11)
[Full Changelog](https://github.com/dyninst/dyninst/compare/Before_PVM_Removal...mrnet-1-0)

## [Before_PVM_Removal](https://github.com/dyninst/dyninst/tree/Before_PVM_Removal) (2003-07-30)
[Full Changelog](https://github.com/dyninst/dyninst/compare/Dyninst4_0...Before_PVM_Removal)

## [Dyninst4_0](https://github.com/dyninst/dyninst/tree/Dyninst4_0) (2003-05-30)
[Full Changelog](https://github.com/dyninst/dyninst/compare/snapshot_20020513...Dyninst4_0)

## [snapshot_20020513](https://github.com/dyninst/dyninst/tree/snapshot_20020513) (2002-05-10)
[Full Changelog](https://github.com/dyninst/dyninst/compare/Dyninst3_0...snapshot_20020513)

## [Dyninst3_0](https://github.com/dyninst/dyninst/tree/Dyninst3_0) (2002-01-17)
[Full Changelog](https://github.com/dyninst/dyninst/compare/Release3_2...Dyninst3_0)

## [Release3_2](https://github.com/dyninst/dyninst/tree/Release3_2) (2001-03-14)
[Full Changelog](https://github.com/dyninst/dyninst/compare/Release3_1...Release3_2)

## [Release3_1](https://github.com/dyninst/dyninst/tree/Release3_1) (2000-08-24)
[Full Changelog](https://github.com/dyninst/dyninst/compare/Release3_0...Release3_1)

## [Release3_0](https://github.com/dyninst/dyninst/tree/Release3_0) (2000-05-16)
[Full Changelog](https://github.com/dyninst/dyninst/compare/Dyninst2_0...Release3_0)

## [Dyninst2_0](https://github.com/dyninst/dyninst/tree/Dyninst2_0) (2000-04-11)
[Full Changelog](https://github.com/dyninst/dyninst/compare/Release2_1...Dyninst2_0)

## [Release2_1](https://github.com/dyninst/dyninst/tree/Release2_1) (1998-05-06)
[Full Changelog](https://github.com/dyninst/dyninst/compare/Release2_0...Release2_1)

## [Release2_0](https://github.com/dyninst/dyninst/tree/Release2_0) (1997-09-19)
[Full Changelog](https://github.com/dyninst/dyninst/compare/Release1_1...Release2_0)

## [Release1_1](https://github.com/dyninst/dyninst/tree/Release1_1) (1996-08-16)
[Full Changelog](https://github.com/dyninst/dyninst/compare/Release1_0...Release1_1)

## [Release1_0](https://github.com/dyninst/dyninst/tree/Release1_0) (1996-05-17)
[Full Changelog](https://github.com/dyninst/dyninst/compare/v0_0...Release1_0)

## [v0_0](https://github.com/dyninst/dyninst/tree/v0_0) (1993-09-03)
[Full Changelog](https://github.com/dyninst/dyninst/compare/10.0.0...v0_0)



\* *This Change Log was automatically generated by [github_changelog_generator](https://github.com/skywinder/Github-Changelog-Generator)*
