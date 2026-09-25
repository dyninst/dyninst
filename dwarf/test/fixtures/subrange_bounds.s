# Hand-crafted DWARF4 .debug_info/.debug_abbrev for test_dwarf_subrange.C.
#
# This is not compiled from source and is never linked or executed - it is
# assembled to an object file and its DWARF sections are read directly with
# libdw. It encodes one compile unit with two DW_TAG_subrange_type children:
#
#   1. DW_AT_upper_bound as DW_FORM_exprloc (a location-expression block) -
#      this is how gfortran represents a Fortran assumed-shape/allocatable
#      array's runtime-determined bound; see dwarf_subrange.cpp.
#   2. DW_AT_upper_bound as DW_FORM_udata with value 9 - an ordinary
#      compile-time constant bound, for a regression check that the
#      dwarf_formblock() probe added for (1) doesn't disturb this path.

    .section .debug_abbrev,"",@progbits
    # Abbrev 1: DW_TAG_compile_unit, has children, no attributes
    .uleb128 1
    .uleb128 0x11               # DW_TAG_compile_unit
    .byte   1                   # DW_CHILDREN_yes
    .uleb128 0
    .uleb128 0

    # Abbrev 2: DW_TAG_subrange_type, no children, DW_AT_upper_bound/DW_FORM_exprloc
    .uleb128 2
    .uleb128 0x21               # DW_TAG_subrange_type
    .byte   0                   # DW_CHILDREN_no
    .uleb128 0x2f               # DW_AT_upper_bound
    .uleb128 0x18               # DW_FORM_exprloc
    .uleb128 0
    .uleb128 0

    # Abbrev 3: DW_TAG_subrange_type, no children, DW_AT_upper_bound/DW_FORM_udata
    .uleb128 3
    .uleb128 0x21               # DW_TAG_subrange_type
    .byte   0                   # DW_CHILDREN_no
    .uleb128 0x2f               # DW_AT_upper_bound
    .uleb128 0x0f               # DW_FORM_udata
    .uleb128 0
    .uleb128 0

    .byte 0                     # end of abbreviation table

    .section .debug_info,"",@progbits
.Lcu_start:
    .4byte .Lcu_end - .Lcu_version   # unit_length
.Lcu_version:
    .2byte 4                    # DWARF version 4
    .4byte 0                    # debug_abbrev_offset
    .byte 8                     # address_size

    .uleb128 1                  # DIE: compile_unit (abbrev 1)

    # child DIE: subrange_type with a dynamic (exprloc) upper bound
    .uleb128 2                  # abbrev 2
    .uleb128 1                  # exprloc length = 1 byte
    .byte 0x9f                  # DW_OP_stack_value (content unused by the fix
                                 # under test - it must not be evaluated)

    # child DIE: subrange_type with an ordinary constant upper bound
    .uleb128 3                  # abbrev 3
    .uleb128 9                  # DW_AT_upper_bound = 9

    .byte 0                     # end of compile_unit's children
.Lcu_end:
