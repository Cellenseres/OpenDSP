// SPDX-License-Identifier: BSD-2-Clause
.section .vorbis_shared,"ax",@progbits
.global ogg_cached_bits
.type ogg_cached_bits,@function
ogg_cached_bits:
    push r0
    push r1
    push r2
    push sv
    push mod0
    rst 0x0c80, mod0
    set 0x0003, mod0
    mov a0l, r0
    mov a1l, r1
    mov b0l, r2
    mov r1, a1
    and 0xffff, a1
    add r2, a1
    mov a1, b1
    mov [r0++], a0
    and 0xffff, a0
    sub 0x10, a1
    br .Lcached_shift, le
    mov [r0++], a1
    and 0xffff, a1
    shfi a1, a1, +16
    or a1, a0
.Lcached_shift:
    mov r1, a1
    and 0xffff, a1
    neg a1, always
    mov a1l, sv
    shfc a0, a0, always
    mov b1, a1
    sub 0x20, a1
    br .Lcached_done, le
    mov [r0], a1
    and 0xffff, a1
    mov a1, b0
    mov 0x20, a1
    sub r1, a1
    mov a1l, sv
    mov b0, a1
    shfc a1, a1, always
    or a1, a0
.Lcached_done:
    mov 0x28, a1
    sub r2, a1
    mov a1l, sv
    shfc a0, a0, always
    neg a1, always
    mov a1l, sv
    set 0x80, mod0
    shfc a0, a0, always
    rst 0x80, mod0
    shfi a0, a0, +8
    shfi a0, a0, -8
    pop mod0
    pop sv
    pop r2
    pop r1
    pop r0
    ret always
.size ogg_cached_bits, .-ogg_cached_bits
