; $Id: test6LS-x86.asm,v 1.1 2002/08/04 17:29:54 gaburici Exp $
;
; This file must be assembled with nasm  - http://freshmeat.net/projects/nasm/

; TODO: use library call this will work on NT
%macro saymsg 1
;write our message to stdout
    mov edx,len_%1
    mov ecx,msg_%1
    mov ebx,1      ; stdout
    mov eax,4      ; sys_write
    int 0x80       ; call linux kernel
%endmacro

global divarw:data (4)
global dfvars:data (4)
global dfvard:data (8)
global dfvart:data (10)
global dlarge:data (512)

segment .data align=16          ; note: all aligns below are relative to this!!!

; x86 has dw do 16-bits, but we stick to 32-bit word for the sake of int==word
    align 8                     ; for MMX
    divarw dd 1234, 5678, 9012, 3456
    align 16                    ; for SSE
    dfvars dd 1.25, 1.5, 1.75, 2.0
    dfvard dq 0.125, 0.175   
; ten byte fp - x86 specific
    dfvart dt 1.99 ; nasm cannot assemble scientific notation (1e-99)

    features  dd 0x0
    features2 dd 0x0
    
    msg_mmx     db      "Testing MMX instructions...",0xa
    len_mmx     equ     $ - msg_mmx

    msg_sse     db      "Testing SSE instructions...",0xa
    len_sse     equ     $ - msg_sse

    msg_sse2    db      "Testing SSE2 instructions...",0xa
    len_sse2    equ     $ - msg_sse2

    msg_amd3d   db      "Testing 3DNow! instructions...",0xa
    len_amd3d   equ     $ - msg_amd3d

segment .bss align=16
    dlarge resb 512
    
segment .text

; nasm elf extension for type/size symbol info... there MUST be a blank after "function"!
global loadsnstores:function (loadsnstores.end - loadsnstores)
global ia32features:function (ia32features.end - ia32features)
global amd_features:function (amd_features.end - amd_features)

loadsnstores:
; IA32 System V ABI - ebp, ebx, edi, esi, esp "belong" to caller
;     rep movsd
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
; dyninst thinks function prologue has to be this big and won't let arbitrary inst points in it
    
    push ebp                    ; s1
    mov  ebp,esp

    push edi
    push esi
    push ebx

    mov eax,divarw
    mov ecx,eax
    mov ebx,eax
    mov edx,eax
    mov ebx,eax
    mov esi,eax
    mov edi,eax
   
; == addressing modes tests ==

; mod/rm decoding tests - 32 bit

; mod == 0
    mov eax,[eax]               ; a5
    mov eax,[ecx]
    mov eax,[edx]
    mov eax,[ebx]
    ;; sib case tested separately
    mov eax,[divarw]
    mov eax,[esi]
    mov eax,[edi]

    mov eax,divarw

; mod == 1
    mov eax,[eax+4]             ; a12
    mov eax,[ecx+8]
    mov eax,[edx+4]
    mov eax,[ebx+8]
    ;; sib case tested separately
    mov eax,[ebp+4]
    mov eax,[esi+8]
    mov eax,[edi+4]

    mov eax,1
    mov ebx,eax
    mov ecx,eax
    mov edx,eax
    mov esi,eax
    mov edi,eax

    push ebp                    ; temporarily destroy frame pointer; confuse debuggers - s5 a19
    mov ebp,eax

; mod == 2
    mov eax,[eax+divarw-1]      ; a20
    mov eax,[ecx+divarw+3]
    mov eax,[edx+divarw+7]
    mov eax,[ebx+divarw+11]
    ;; sib case tested separately
    mov eax,[ebp+divarw-1]
    mov eax,[esi+divarw+3]
    mov eax,[edi+divarw+7]

    mov eax,divarw

; sib tests

    mov ebx,eax
    mov ebp,eax

    xor edx,edx
    mov ecx,4
    mov esi,12

    mov eax,[ebx+esi]           ; l22 a28
    mov eax,[esp]
    mov eax,[ebx+2*ecx]
    mov eax,[nosplit divarw+2*ecx] ; nosplit prevents nasm from optimizing this as ecx+ecx+divarw
    mov eax,[ebx+ecx+4]
    mov eax,[edx+edx*8+divarw]
    mov ecx,1
    mov eax,[ebp+ecx*2+2]       ; l28
    mov eax,[ebx+ecx*4+4]
    mov ebp,0
    mov eax,[ebp+4*ecx+divarw]

    pop ebp                     ; l31 a37

; other addressing modes not using modrm

    ;; X, Y tested in the next section
    ;; wierd instructions TBD

; == semantic classes tests ==

    ;; sNONE well tested anyhow
    ;; s1R is tested at prefetch
    ;; s1W is tested at stmxcsr(SSE)
    inc dword [divarw+4]        ; s1RW                                              - l32 s6 a38
    cmp eax, [divarw+4]         ; s1R2R [one operand is register]
    mov edi, divarw
    mov esi, divarw+4
    cmpsb                       ; s1R2R [both operands are memory]                  - l34 a40&41
    mov [divarw+4], edi         ; s1W2R                                             - s7
    add [divarw], eax           ; s1RW2R                                            - l35 s8
    xchg [divarw+4], edi        ; s1RW2RW [one operand is register]                 - l36 s9
    imul eax, [divarw+8], 3     ; s1W2R3R [one operand is register, one immediate]  - l37
    xor eax,eax
    mov ax,ds
    mov edx,divarw
    mov [divarw], edx           ; s10
    ;lds eax,[divarw+2]          ; s1W2W3R [two regs, one mem]                       - l38
    mul byte [divarw]           ; hope it gens mul edx:eax, [divarw] - s1W2RW3R     - l39
    ;; s1W2R3RW - push & pop we've got plenty
    shld [divarw+4], eax, 3     ; s1RW2R3R                                          - l40 s11
    mov eax,4
    mov edx,5
    idiv dword [divarw]         ; s1RW2RW3R                                         - l41 a48

; == MMX/SSE/SSE2 instructions test ==

    call ia32features
    test eax, 1<<23
    jz   near .bail             ; assuming it cannot do sse/sse2
    saymsg mmx

; MMX test
    movd mm0, [divarw]          ; l42 a49
    pmaddwd mm0, [divarw+8]
    psraw mm0, 2                ; just decoder test for MMX groups
    movq [divarw], mm0          ; s12
    emms

; TODO:    test fxsr?

    call ia32features
    test eax, 1<<25
    jz   near .bail             ; assuming it cannot do sse2
    saymsg sse

; SSE test    
    movaps xmm0, [dfvars]       ; book incorrectly tags this as sse2                 - l44 a52
    cmpeqss xmm0, [dfvars]

    call ia32features
    test eax, 1<<26
    jz   near .bail
    saymsg sse2

; SSE2 test
    movapd xmm1, [dfvard]       ; l46 a54
    cmpeqsd xmm0, [dfvard]
    psrldq xmm0, 2              ; just decoder test for SSE2 groups

    ;; TODO:    prefetches & non-temporals!!!

.bail:

; == 3DNow! instructions test ==

    call amd_features
    test eax, 80000000H
    jz   near .bail2
    saymsg amd3d

; since registers are shared with mmx, there are no special loads
    movq mm0, [dfvars]          ; l48 a56
    pfmin mm0, [dfvars+8]       ; memory is only read by 3DNow! instructions        - l49
    femms                       ; is this needed?

;; TODO:    prefetches
    
; Athlon extensions to 3DNow! not tested (feature bit 30) - most of them are
; a subset of SSE which is fully implemented on the Athlon XP/MP anyway...

.bail2:
    
; == REP prefixed stuff test

    cld
    mov ecx, 16/4
    mov eax, 10
    mov edi, divarw
    stosd
;    rep stosd                   ; s13 a58 a56onP4 a54onP3

    std
    mov ecx, 16/4
    mov edi, divarw+16-4
    stosd
;    rep stosd                   ; s14

    cld
    mov ecx, 16/4
    mov esi, dfvars
    mov edi, divarw
    movsd
;    rep movsd                   ; l50 s15

; == addressing using 16-bit registers ==

    ;; TBD
    
; == x87 FP instructions test ==

    fld dword [dfvars]          ; l51 a60
    fld qword [dfvard]
    fld tword [dfvart]
    fild word [divarw]
    fild dword [divarw+4]
    fild qword [divarw+8]

    fst dword [dfvars]          ; s16 a66
    fstp qword [dfvard]
    fstp tword [dfvart]
    fist word [divarw+2]
    fist dword [divarw+4]
    fistp qword [divarw+8]
    
    fnstcw [divarw]             ; s21
    fldcw [divarw]              ; l57

    fnstenv [dlarge]            ; s22 a74
    fldenv [dlarge]             ; l58
    
; == conditional moves: TBD ==
    
    pop ebx                     ; l59 a76
    pop esi
    pop edi

    leave                       ; FIXME:     this is load too...
    ret                         ; FIXME:     this is load too...
.end:

  
ia32features:
    mov eax,1
    push ebx                    ; cpuid changes ebx as well, but ebx "belongs" to the caller
    cpuid
    mov eax,edx
    pop ebx
    ret
.end:


amd_features:
    mov eax,80000000H
    push ebx                    ; cpuid changes ebx as well, but ebx "belongs" to the caller
    cpuid
    cmp eax,80000000H
    jbe .noext
    mov eax,80000001H
    cpuid
    mov eax,edx
    jmp .done
.noext:
    mov eax, 0
.done:
    pop ebx
    ret
.end:
