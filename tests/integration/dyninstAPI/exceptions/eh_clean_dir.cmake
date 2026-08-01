# Clean + (re)create a single directory, passed as -DDIR=<path>.
#
# Used as a ctest FIXTURES_SETUP step so each exception test starts from an empty
# artifact directory: a rewrite that fails or partially writes can then never
# leave a stale binary behind for the catch phase to run (a false pass). Cleaning
# at SETUP (rather than a FIXTURES_CLEANUP that deletes afterward) means the
# artifacts survive the run and are available for post-mortem debugging until the
# next run re-cleans them.
if(NOT DEFINED DIR)
  message(FATAL_ERROR "eh_clean_dir.cmake requires -DDIR=<path>")
endif()
file(REMOVE_RECURSE "${DIR}")
file(MAKE_DIRECTORY "${DIR}")
