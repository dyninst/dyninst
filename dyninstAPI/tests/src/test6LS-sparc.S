.file "test6LS-sparc.S"

#define SA(x) (((x)+15)&(~0x1F))
#define MINFRAME ((16+1+6)*8)
    
!!!!! Test 32-bit & 64-bit SPARCv9 mnemonics (without 64-bit constants)
!!!!! 64-bit mnemonics require -xarch=v8plus option, binary is still ELF32
!!!!! 64-constants (xword) require -xarch=v9, binary is ELF64 (unsupported)

.section ".data"
    .global divarw
    .global dfvars
    .global dfvard
    .global dfvarq
    .align 4
divarw:
    .word 1234, 5678
dfvars:
    .single 0r1.25
    .align 8
dfvard:
    .double 0r1.125
dfvarq:
    .quad 0r1e-99

.section ".bss"

.section ".text"
    .global loadsnstores
    .align 4

loadsnstores:
        
    save    %sp,-SA(MINFRAME),%sp

    set     divarw,%l1
    lduw    [%l1],%l2    
    nop

    mov     %g1, %l5
    mov     %l1, %g1
    ldsb    [%g1+3],%l2
    nop
    mov     %l5, %g1

    mov     %g2, %l5
    mov     %l1, %g2
    lduh    [%g2+2],%l2
    nop
    mov     %l5, %g2

    ldx     [%l1],%l2
    nop
    ldswa   [%l1]#ASI_P,%l2
    nop
    ldstub  [%l1+3],%l2
    nop
    ldub    [%l1+3],%l2

    set     1944,%l2
    cas     [%l1],%l2,%l3
    nop
    casx    [%l1],%l2,%l3
    nop
    swap    [%l1],%l2

    set     dfvars,%l1
    ld      [%l1],%f0
    nop
    ld      [%l1],%fsr      ! LDFSR
    set     dfvard,%l1
    ldd     [%l1],%f0
    nop
    ldd     [%l1],%fsr      ! LDXFSR
    set     dfvarq,%l1
    ldq     [%l1],%f0
    nop

    prefetch [%l1],2        ! #N_WRITES
    set     dfvarq,%l4
    prefetcha [%l4]#ASI_P,0 ! #N_READS 
    nop

    set     divarw,%l5
    stb     %i0,[%l5+7]
    nop
    sth     %i1,[%l5+6]
    nop
    stw     %i2,[%l5+4]
    nop
    stx     %i0,[%l5]

    set     dfvars,%l1
    st      %f0,[%l1]
    nop
    set     dfvard,%l1
    std     %f0,[%l1]
    nop
    set     dfvarq,%l2
    stq     %f0,[%l2]
    nop
    set     divarw,%l5
    st      %fsr,[%l5+4]    ! STFSR
    nop
    stx     %fsr,[%l5]      ! STXFSR    

    add     %i0,%i1,%l7
    add     %l7,%i2,%l7
    ret
    restore %l7,0,%o0

.type loadsnstores,#function
.size loadsnstores,.-loadsnstores
