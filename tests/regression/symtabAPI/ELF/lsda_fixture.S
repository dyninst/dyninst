/* Never executed: a small ELF with a hand-written LSDA for the parser test. */
.text
.globl lsda_function, lsda_try, lsda_catch
.hidden lsda_function, lsda_try, lsda_catch
.type lsda_function, %function
lsda_function:
    .cfi_startproc
    .cfi_lsda 0x1b, .Llsda /* PC-relative signed four-byte offset. */
    .space 4
lsda_try:
    .space 4
.Ltry_end:
    .space 4
lsda_catch:
    .space 4
    .cfi_endproc
.size lsda_function, .-lsda_function

.section .gcc_except_table,"a",%progbits
.Llsda:
#ifdef OMIT_LPSTART
    .byte 0xff /* Omitted LPStart: use the function address. */
#else
    .byte 0x1b /* PC-relative signed four-byte LPStart. */
    .long lsda_function - 0x40 - . /* LPStart is before the function. */
#endif
    .byte 0xff /* No type table. */
#ifdef INVALID_FORMAT
    .byte 0x0e /* Unsupported call-site encoding. */
#else
    .byte 0x01 /* ULEB128 call-site entries. */
#endif
#ifdef OVERSIZED_TABLE
    .uleb128 0x100000 /* Exceeds the fixture's exception section. */
#else
    .uleb128 .Ltable_end - .Ltable
#endif
.Ltable:
    /* The region remains function-relative even with an explicit LPStart. */
    .uleb128 lsda_try - lsda_function
    .uleb128 .Ltry_end - lsda_try
#ifdef OMIT_LPSTART
    .uleb128 lsda_catch - lsda_function
#else
    .uleb128 lsda_catch - lsda_function + 0x40
#endif
    .uleb128 0
.Ltable_end:
.section .note.GNU-stack,"",%progbits
