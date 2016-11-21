        TITLE   test6LS.s
        .686P
        .mmx
        .xmm
        .MODEL   flat
       
saymsg  MACRO   Msg
;write our message to console
;call a function that takes all arguments in registers
;so all those pushes won't get instrumented
;this generates the same EA sequence as linux
;write our message to stdout
        mov     edx,OFFSET _msg_&Msg&
        mov     ecx,OFFSET _len_&Msg&
        call    _cputs   ; our function
        ENDM              
        
PUBLIC _divarw
PUBLIC _dfvars
PUBLIC _dfvard
PUBLIC _dfvart
PUBLIC _dlarge        
        
_DATA   SEGMENT
ALIGN 8              
        _divarw         dd      1234, 5678, 9012, 3456
ALIGN 16
        _dfvars         dd      1.25, 1.5, 1.75, 2.0
        _dfvard         dq      0.125, 0.175
        _dfvart         dt      1.99
        _written        dd      0
        
        _msg_mmx        db      "Testing MMX instructions...", 0AH
        _len_mmx        equ     $ - _msg_mmx

        _msg_sse        db      "Testing SSE instructions...",0AH
        _len_sse        equ     $ - _msg_sse
        
        _msg_sse2       db      "Testing SSE2 instructions...",0AH
        _len_sse2       equ     $ - _msg_sse2

        _msg_amd3d      db      "Testing 3DNow! instructions...",0AH
        _len_amd3d      equ     $ - _msg_amd3d
    
        _dlarge         db      "keep the interface small and easy to understand."
                        db 512-($ - _divarw)  DUP(0)
_DATA   ENDS

PUBLIC _loadsnstores
PUBLIC _ia32features
PUBLIC _amd_features
EXTRN   _GetStdHandle@4:NEAR
EXTRN   _WriteConsoleA@20:NEAR                

_TEXT   SEGMENT
_cputs  PROC NEAR
	; eax = GetStdHandle(stdout=-11)
	push    -11
	call	_GetStdHandle@4

        ; WriteConsole(eax, edx, ecx, written, 0)
	push    0
        push    OFFSET _written
	push	ecx
	push	edx
	push	eax
	call	_WriteConsoleA@20
	ret
        
_cputs  ENDP        
     
_loadsnstores   PROC NEAR
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

    mov eax,OFFSET _divarw
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
    mov eax,[_divarw]
    mov eax,[esi]
    mov eax,[edi]

    mov eax,OFFSET _divarw

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
    mov eax,[eax+_divarw-1]      ; a20
    mov eax,[ecx+_divarw+3]
    mov eax,[edx+_divarw+7]
    mov eax,[ebx+_divarw+11]
    ;; sib case tested separately
    mov eax,[ebp+_divarw-1]
    mov eax,[esi+_divarw+3]
    mov eax,[edi+_divarw+7]

    mov eax, OFFSET _divarw

; sib tests

    mov ebx,eax
    mov ebp,eax

    xor edx,edx
    mov ecx,4
    mov esi,12

    mov eax,[esi+ebx]         ; l22 a28
    mov eax,[esp]
    mov eax,[ebx+2*ecx]
    mov eax,[_divarw+2*ecx] ; nosplit prevents nasm from optimizing this as ecx+ecx+divarw
    mov eax,[ecx+ebx+4]
    mov eax,[edx+edx*8+_divarw]
    mov ecx,1
    mov eax,[ebp+ecx*2+2]       ; l28
    mov eax,[ebx+ecx*4+4]
    mov ebp,0
    mov eax,[ebp+4*ecx+_divarw]

    pop ebp                     ; l31 a37

; other addressing modes not using modrm

    ;; X, Y tested in the next section
    ;; wierd instructions TBD

; == semantic classes tests ==

    ;; sNONE well tested anyhow
    ;; s1R is tested at prefetch
    ;; s1W is tested at stmxcsr(SSE)
    inc dword ptr [_divarw+4]        ; s1RW                                              - l32 s6 a38
    cmp eax, [_divarw+4]         ; s1R2R [one operand is register]
    mov edi, OFFSET _divarw
    mov esi, OFFSET _divarw+4
    cmpsb                       ; s1R2R [both operands are memory]                  - l34 a40&41
    mov [_divarw+4], edi         ; s1W2R                                             - s7
    add [_divarw], eax           ; s1RW2R                                            - l35 s8
    xchg [_divarw+4], edi        ; s1RW2RW [one operand is register]                 - l36 s9
    imul eax, [_divarw+8], 3     ; s1W2R3R [one operand is register, one immediate]  - l37
    xor eax,eax
    mov ax,ds
    mov edx, OFFSET _divarw
    mov [_divarw], edx           ; s10
    ;lds eax,[divarw+2]          ; s1W2W3R [two regs, one mem]                       - l38
    mul byte ptr [_divarw]           ; hope it gens mul edx:eax, [divarw] - s1W2RW3R     - l39
    ;; s1W2R3RW - push & pop we've got plenty
    shld [_divarw+4], eax, 3     ; s1RW2R3R                                          - l40 s11
    mov eax,4
    mov edx,5
    idiv dword ptr [_divarw]         ; s1RW2RW3R                                         - l41 a48

; == MMX/SSE/SSE2 instructions test ==

    call _ia32features
    test eax, 1 SHL 23
    jz   near ptr bail             ; assuming it cannot do sse/sse2
    saymsg mmx

; MMX test
    movd mm0, [_divarw]          ; l42 a49
    pmaddwd mm0, qword ptr [_divarw+8]
    psraw mm0, 2                ; just decoder test for MMX groups
    movntq qword ptr [_divarw], mm0        ; s12, non-temporal
    emms

; TODO:    test fxsr?

    call _ia32features
    test eax, 1 SHL 25
    jz   near ptr bail             ; assuming it cannot do sse2
    saymsg sse

; SSE test    
    movaps xmm0, [_dfvars]       ; book incorrectly tags this as sse2                 - l44 a52
    cmpeqss xmm0, [_dfvars]
    prefetcht0 [_divarw]         ; not sure about this, but seem safe only if CPU knows SSE

    call _ia32features
    test eax, 1 SHL 26
    jz   near ptr bail
    saymsg sse2

; SSE2 test
    movapd xmm1, [_dfvard]       ; l46 a55
    cmpsd xmm0, [_dfvard], 0
    psrldq xmm0, 2              ; just decoder test for SSE2 groups

bail:

; == 3DNow! instructions test ==

    call _amd_features
    test eax, 80000000H
    jz   near ptr bail2
    saymsg amd3d

; since registers are shared with mmx, there are no special loads
    movq mm0, qword ptr [_dfvars]          ; l48 a56
    pfmin mm0, qword ptr [_dfvars+8]       ; memory is only read by 3DNow! instructions        - l49
    prefetch   [_divarw]
    femms                       ; is this needed?
    
; Athlon extensions to 3DNow! not tested (feature bit 30) - most of them are
; a subset of SSE which is fully implemented on the Athlon XP/MP anyway...

bail2:
    
; == REP prefixed stuff test

    cld
    mov ecx, 12/4
    mov eax, 10
    mov edi, OFFSET _divarw
    rep stosd                   ; s13 a58 a56onP4 a54onP3

    std
    mov ecx, 16/4
    mov edi, OFFSET _divarw+16-4
    stosd
;    rep stosd                   ; s14

    cld
    mov ecx, 16/4
    mov esi, OFFSET _dfvars
    mov edi, OFFSET _divarw
;    movsd
    rep movsd                   ; l50 s15 

    mov eax, 0                  ; only al matters...
    mov ecx, 256
    mov edi, OFFSET _dlarge
    repne scasb

    mov ecx, 256
    mov edi, OFFSET _dlarge + 25
    mov esi, OFFSET _dlarge + 44
    repe cmpsb

; == addressing using 16-bit registers ==

    ;; TBD
    
; == x87 FP instructions test ==

    fld dword ptr [_dfvars]          ; l51 a60
    fld qword ptr [_dfvard]
    fld tbyte ptr [_dfvart]
    fild word ptr [_divarw]
    fild dword ptr [_divarw+4]
    fild qword ptr [_divarw+8]

    fst dword ptr [_dfvars]          ; s16 a66
    fstp qword ptr [_dfvard]
    fstp tbyte ptr [_dfvart]
    fist word ptr [_divarw+2]
    fist dword ptr [_divarw+4]
    fistp qword ptr [_divarw+8]
    
    fnstcw word ptr [_divarw]             ; s21
    fldcw word ptr [_divarw]              ; l57

    fnstenv [_dlarge]            ; s22 a74
    fldenv [_dlarge]             ; l58 a75
    
; == conditional moves ==

    mov eax, 3
    mov edx, 2
    mov ecx, 0
    cmp eax,edx
    cmova ecx, [_divarw]         ; executed     - a76
    cmove ebx, [_divarw+4]       ; not -"-      - a77
    mov ebx, [_divarw+8]         ; sync point   - a78
        
    pop ebx                     ; l59 a76
    pop esi
    pop edi

    leave                       ; FIXME:     this is load too...
    ret                         ; FIXME:     this is load too...
_loadsnstores   ENDP


_ia32features   PROC NEAR
    mov eax,1
    push ebx                    ; cpuid changes ebx as well, but ebx "belongs" to the caller
    cpuid
    mov eax,edx
    pop ebx
    ret
_ia32features   ENDP


_amd_features   PROC NEAR
    mov eax,80000000H
    push ebx                    ; cpuid changes ebx as well, but ebx "belongs" to the caller
    cpuid
    cmp eax,80000000H
    jbe noext
    mov eax,80000001H
    cpuid
    mov eax,edx
    jmp done
noext:
    mov eax, 0
done:
    pop ebx
    ret    
_amd_features   ENDP
                   
_TEXT   ENDS                
        
END        