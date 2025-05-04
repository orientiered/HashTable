section .text

global fastCrc32u
global fastCrc32


;========================================================
; Crc32 hashing algorithm for C strings
; Args:
;   rdi - memory address
; Ret:
;   rax - crc32 hash
;========================================================
fastCrc32u:
    xor  rax, rax
    dec  rax        ; rax = all ones
    jmp  .loop_cmp
    .hash_loop:
        crc32 rax, sil
        .loop_cmp:
        mov   sil, BYTE [rdi]
        inc   rdi
        test  sil, sil
        jnz   .hash_loop

    ret


;========================================================
; Crc32 hashing algorithm for C strings
; Args:
;   rdi - memory address
;   rsi - length
; Ret:
;   rax - crc32 hash
;========================================================
fastCrc32:
    ; xor  rax, rax
    ; dec  rax        ; rax = all ones
    mov  rax, 0xFFFFFFFFFFFFFFFF
    mov  rcx, rsi

    .hash_loop:
        mov  sil, [rdi]
        inc  rdi
        crc32 rax, sil
        loop .hash_loop

    ret