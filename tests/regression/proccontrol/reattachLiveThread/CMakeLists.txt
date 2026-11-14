include_guard(GLOBAL)

# ProcControl drives a live process with ptrace; nothing is instrumented.
dyninst_add_test_executable(
  reattach_live_thread_mutatee
  SOURCE_FILES reattach_live_thread_mutatee.c
  DEPS Threads::Threads
  REQUIRES dynamic host:x86_64,aarch64,ppc64le
)

dyninst_add_test(
  proccontrol_reattach_live_thread
  TYPE regression
  SOURCE_FILES reattach_live_thread.cpp
  DEPS pcontrol
  ARGS $<TARGET_FILE:reattach_live_thread_mutatee>
  REQUIRES dynamic host:x86_64,aarch64,ppc64le
  PROPERTIES TIMEOUT 120
)
