include_guard(GLOBAL)

# The checker
add_executable(getSOName getSOName.cpp)
target_compile_options(getSOName PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(getSOName PRIVATE symtabAPI)

# 1. The ordinary case: CMake gives a shared library a SONAME equal to its file name.
add_library(soname_default SHARED test.cpp)

# 2. No SONAME at all.
add_library(soname_absent SHARED test.cpp)
set_target_properties(soname_absent PROPERTIES NO_SONAME TRUE)

# 3. The case ProcControl cares about: the SONAME and the file name disagree. This is the
#    shape glibc-hwcaps produces, where libpthread.so.0 resolves to a file called
#    libpthread-<version>.so under a microarchitecture directory.
add_library(soname_mismatch SHARED test.cpp)
set_target_properties(soname_mismatch PROPERTIES NO_SONAME TRUE OUTPUT_NAME
                                                                "soname_on_disk_name")
target_link_options(soname_mismatch PRIVATE "-Wl,-soname,libdyninst_declared_name.so.7")

add_test(NAME symtab_soname_default COMMAND getSOName $<TARGET_FILE:soname_default>
                                            $<TARGET_FILE_NAME:soname_default>)

add_test(NAME symtab_soname_absent COMMAND getSOName $<TARGET_FILE:soname_absent> "")

add_test(NAME symtab_soname_mismatch COMMAND getSOName $<TARGET_FILE:soname_mismatch>
                                             "libdyninst_declared_name.so.7")

# The mismatch case must be a real mismatch, or it proves nothing.
add_test(NAME symtab_soname_mismatch_is_a_mismatch
         COMMAND getSOName $<TARGET_FILE:soname_mismatch>
                 $<TARGET_FILE_NAME:soname_mismatch>)
set_tests_properties(symtab_soname_mismatch_is_a_mismatch PROPERTIES WILL_FAIL TRUE)

set_tests_properties(symtab_soname_default symtab_soname_absent symtab_soname_mismatch
                     symtab_soname_mismatch_is_a_mismatch PROPERTIES LABELS "regression")
