# The warning levels.  Each level adds flags to the ones below it, and
# DyninstWarnings.cmake requests every flag up to DYNINST_WARNING_LEVEL.  Flags are
# written without the leading '-'.
#
# Levels are append-only, so that a repository that pins a level keeps the same flags:
#
# - Never add a flag to an existing level, or move or remove one.
# - To add warnings, add a level after the last one, with the next number.  The
#   module finds the highest level itself; nothing else needs to change.
# - Build Dyninst and the repositories that copy cmake/common at the new level, with
#   DYNINST_WARNINGS_AS_ERRORS=ON, and fix the new warnings or suppress them before
#   the level is added.
# - Add the level to the table in README.md.

set(_dyninst_warning_level_1 Wall)

set(_dyninst_warning_level_2 Wextra)

set(_dyninst_warning_level_3 Wpedantic)

set(
  _dyninst_warning_level_4
  Walloca
  Wcast-align
  Wcast-qual
  Wcomma-subscript
  Wctor-dtor-privacy
  Wdeprecated-copy-dtor
  Wdouble-promotion
  Wduplicated-branches
  Wduplicated-cond
  Wenum-conversion
  Wextra-semi
  Wfloat-equal
  Wformat-overflow=2
  Wformat-signedness
  Wformat=2
  Wframe-larger-than=${defaultMaxFrameSize}
  Wjump-misses-init
  Wlogical-op
  Wmismatched-tags
  Wmissing-braces
  Wmultichar
  Wnoexcept
  Wnon-virtual-dtor
  Woverloaded-virtual
  Wpointer-arith
  Wrange-loop-construct
  Wrestrict
  Wshadow
  Wstrict-null-sentinel
  Wsuggest-attribute=format
  Wsuggest-attribute=malloc
  Wuninitialized
  Wvla
  Wvolatile
  Wwrite-strings
)

set(
  _dyninst_warning_level_5
  Wcomma
  Wdate-time
  Wdeprecated-enum-enum-conversion
  Wdeprecated-enum-float-conversion
  Winconsistent-missing-override
)

# Candidates for later levels, not yet clean in Dyninst.  Each group is
# about the work to make it clean; a level gets its number when it is added.
#
# Small, a few dozen sites:
#
# set(_dyninst_warning_level_N
#   Wundef Wfloat-conversion Wredundant-decls
#   Wimplicit-fallthrough       # Clang; GCC has it in Wextra
#   Wunreachable-code Wmissing-noreturn Wshadow-all   # Clang
# )
#
# Medium, a few hundred sites:
#
# set(_dyninst_warning_level_N
#   Wzero-as-null-pointer-constant Wuseless-cast Wsuggest-attribute=noreturn
#   Wmissing-declarations Wmissing-prototypes
# )
#
# Large, a project each:
#
# set(_dyninst_warning_level_N
#   Wsuggest-override Wold-style-cast Wextra-semi-stmt Wconversion
#   Wredundant-tags Wsign-promo Wsuggest-final-types Wsuggest-final-methods
#   Walloc-zero
# )
#
# Not worth pursuing:
#
#   Wnull-dereference   false positives that change with optimization
#   Wswitch-enum        floods on the large enums that have a default:
#   Weffc++             outdated advice
#   Wpadded Wweak-vtables Wunused-macros   mostly noise
#   Wstrict-overflow    changes with optimization level
#   Weverything         (Clang) changes with each release
