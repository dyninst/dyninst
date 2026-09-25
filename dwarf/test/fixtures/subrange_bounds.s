# Hand-crafted DWARF4 .debug_info/.debug_abbrev for test_dwarf_subrange.C.
#
# This is not compiled from source and is never linked or executed - it is
# assembled to an object file and its DWARF sections are read directly with
# libdw. It encodes one compile unit with four DW_TAG_subrange_type children
# (plus one DW_TAG_base_type child used only as a DW_AT_type reference
# target):
#
#   1. DW_AT_upper_bound as DW_FORM_exprloc (a location-expression block) -
#      this is how gfortran represents a Fortran assumed-shape/allocatable
#      array's runtime-determined bound; see dwarf_subrange.cpp.
#   2. DW_AT_upper_bound as DW_FORM_udata with value 9 - an ordinary
#      compile-time constant bound, for a regression check that the
#      dwarf_formblock() probe added for (1) doesn't disturb this path.
#   3. DW_AT_count with value 5 and no DW_AT_upper_bound/DW_AT_lower_bound -
#      exercises the lower_bound + count - 1 fallback arithmetic.
#   4. DW_AT_type pointing at a DW_TAG_base_type with DW_AT_encoding =
#      DW_ATE_signed, and DW_AT_upper_bound as DW_FORM_sdata == -1 -
#      exercises the is_signed()/dwarf_formsdata() decode path.

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

    # Abbrev 4: DW_TAG_base_type, no children, DW_AT_encoding/DW_FORM_data1
    .uleb128 4
    .uleb128 0x24               # DW_TAG_base_type
    .byte   0                   # DW_CHILDREN_no
    .uleb128 0x3e               # DW_AT_encoding
    .uleb128 0x0b               # DW_FORM_data1
    .uleb128 0
    .uleb128 0

    # Abbrev 5: DW_TAG_subrange_type, no children, DW_AT_type/DW_FORM_ref4 +
    # DW_AT_upper_bound/DW_FORM_sdata
    .uleb128 5
    .uleb128 0x21               # DW_TAG_subrange_type
    .byte   0                   # DW_CHILDREN_no
    .uleb128 0x49               # DW_AT_type
    .uleb128 0x13               # DW_FORM_ref4
    .uleb128 0x2f               # DW_AT_upper_bound
    .uleb128 0x0d               # DW_FORM_sdata
    .uleb128 0
    .uleb128 0

    # Abbrev 6: DW_TAG_subrange_type, no children, DW_AT_count/DW_FORM_udata only
    .uleb128 6
    .uleb128 0x21               # DW_TAG_subrange_type
    .byte   0                   # DW_CHILDREN_no
    .uleb128 0x37               # DW_AT_count
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

    # child DIE: subrange_type with only DW_AT_count (no explicit bounds)
    .uleb128 6                  # abbrev 6
    .uleb128 5                  # DW_AT_count = 5

    # child DIE: base_type (signed), referenced by the next subrange's DW_AT_type
.Lbase_type_signed:
    .uleb128 4                  # abbrev 4
    .byte 0x05                  # DW_ATE_signed

    # child DIE: subrange_type with a signed (DW_FORM_sdata) upper bound
    .uleb128 5                  # abbrev 5
    .4byte .Lbase_type_signed - .Lcu_start   # DW_AT_type (ref4, CU-relative)
    .sleb128 -1                              # DW_AT_upper_bound = -1

    .byte 0                     # end of compile_unit's children
.Lcu_end:
