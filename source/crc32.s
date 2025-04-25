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
    .hash_loop:
        mov   sil, BYTE [rdi]
        inc   rdi
        crc32 rax, sil
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
    xor  rax, rax
    dec  rax        ; rax = all ones
    
    jmp  .hash_loop8_cmp
    .hash_loop8:
        crc32 rax, QWORD [rdi]
        add   rdi, 8
        sub   rsi, 8
    .hash_loop8_cmp:
        cmp   rsi, 8
        jae   .hash_loop8

    jmp .hash_loop1_cmp
    .hash_loop1:
        crc32 rax, BYTE  [rdi]
        inc   rdi
        dec   rsi
    .hash_loop1_cmp:
        test  rsi, rsi
        jnz   .hash_loop1

    ret