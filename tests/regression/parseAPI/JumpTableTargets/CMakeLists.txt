include_guard(GLOBAL)

set(_targ parseAPI_jumpTableTargets)
dyninst_add_test_executable(
  ${_targ}
  SOURCE_FILES jumpTableTargets.cpp
  DEPS parseAPI
  REQUIRES test_binaries
)
set(_fpath "${DYNINST_TEST_BINARIES_DIR}/dyninstAPI/findMain")

# __vfprintf_internal's two-level table: a class byte (table at 0x458218 or 0x458220,
# depending on the path) selects an entry of the pointer table at 0x48da20; these are the
# entries the 91 class bytes of both tables select.
dyninst_add_test(
  ${_targ}_aarch64_vfprintf
  TYPE regression
  COMMAND
    ${_targ} "${_fpath}/aarch64/stat.nopie.nostripped" 409958 408f9c 409088 4090c0 4092cc
    40939c 409450 409568 4095a0 409790 4097d0 409800 409830 40983c 409848 409880 4098a4
    4098e4 40991c 40995c 4099a4 4099dc 409a1c 409a54 409a8c 409ac4 409b10 409b70 409bc4
    409c34
  REQUIRES test_binaries
)

# plural_eval's table of 11 offsets (cmplwi r9,10) right after the bctr, at 0x10001ba4;
# its base is built from the TOC by addis and addi.
dyninst_add_test(
  ${_targ}_ppc64le_plural_eval
  TYPE regression
  COMMAND
    ${_targ} "${_fpath}/ppc64le/stat.nopie.nostripped" 10001ba0 10001c6c 10001c84 10001c9c
    10001cb4 10001ccc 10001ce4 10001cf4 10001d04 10001d24 10001d3c 10001d54
  REQUIRES test_binaries
)
