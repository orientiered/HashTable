section .text

global fastCrc32u


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